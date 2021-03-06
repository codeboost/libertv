//
// win_iocp_socket_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_IOCP_SOCKET_SERVICE_HPP
#define ASIO_DETAIL_WIN_IOCP_SOCKET_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/push_options.hpp"

#include "asio/detail/win_iocp_io_service_fwd.hpp"

#if defined(ASIO_HAS_IOCP)

#include "asio/detail/push_options.hpp"
#include <cstring>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "asio/detail/pop_options.hpp"

#include "asio/buffer.hpp"
#include "asio/error.hpp"
#include "asio/io_service.hpp"
#include "asio/socket_base.hpp"
#include "asio/detail/bind_handler.hpp"
#include "asio/detail/handler_alloc_helpers.hpp"
#include "asio/detail/handler_invoke_helpers.hpp"
#include "asio/detail/mutex.hpp"
#include "asio/detail/select_reactor.hpp"
#include "asio/detail/socket_holder.hpp"
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/socket_types.hpp"
#include "asio/detail/win_iocp_io_service.hpp"

namespace asio {
namespace detail {

template <typename Protocol>
class win_iocp_socket_service
  : public asio::detail::service_base<win_iocp_socket_service<Protocol> >
{
public:
  // The protocol type.
  typedef Protocol protocol_type;

  // The endpoint type.
  typedef typename Protocol::endpoint endpoint_type;

  // Base class for all operations.
  typedef win_iocp_operation operation;

  struct noop_deleter { void operator()(void*) {} };
  typedef boost::shared_ptr<void> shared_cancel_token_type;
  typedef boost::weak_ptr<void> weak_cancel_token_type;

  // The native type of a socket.
  class native_type
  {
  public:
    native_type(socket_type s)
      : socket_(s),
        have_remote_endpoint_(false)
    {
    }

    native_type(socket_type s, const endpoint_type& ep)
      : socket_(s),
        have_remote_endpoint_(true),
        remote_endpoint_(ep)
    {
    }

    void operator=(socket_type s)
    {
      socket_ = s;
      have_remote_endpoint_ = false;
      remote_endpoint_ = endpoint_type();
    }

    operator socket_type() const
    {
      return socket_;
    }

    HANDLE as_handle() const
    {
      return reinterpret_cast<HANDLE>(socket_);
    }

    bool have_remote_endpoint() const
    {
      return have_remote_endpoint_;
    }

    endpoint_type remote_endpoint() const
    {
      return remote_endpoint_;
    }

  private:
    socket_type socket_;
    bool have_remote_endpoint_;
    endpoint_type remote_endpoint_;
  };

  // The implementation type of the socket.
  class implementation_type
  {
  public:
    // Default constructor.
    implementation_type()
      : socket_(invalid_socket),
        flags_(0),
        cancel_token_(),
        protocol_(endpoint_type().protocol()),
        next_(0),
        prev_(0)
    {
    }

  private:
    // Only this service will have access to the internal values.
    friend class win_iocp_socket_service;

    // The native socket representation.
    native_type socket_;

    enum
    {
      enable_connection_aborted = 1, // User wants connection_aborted errors.
      user_set_linger = 2, // The user set the linger option.
      user_set_non_blocking = 4 // The user wants a non-blocking socket.
    };

    // Flags indicating the current state of the socket.
    unsigned char flags_;

    // We use a shared pointer as a cancellation token here to work around the
    // broken Windows support for cancellation. MSDN says that when you call
    // closesocket any outstanding WSARecv or WSASend operations will complete
    // with the error ERROR_OPERATION_ABORTED. In practice they complete with
    // ERROR_NETNAME_DELETED, which means you can't tell the difference between
    // a local cancellation and the socket being hard-closed by the peer.
    shared_cancel_token_type cancel_token_;

    // The protocol associated with the socket.
    protocol_type protocol_;

    // The ID of the thread from which it is safe to cancel asynchronous
    // operations. 0 means no asynchronous operations have been started yet.
    // ~0 means asynchronous operations have been started from more than one
    // thread, and cancellation is not supported for the socket.
    DWORD safe_cancellation_thread_id_;

    // Pointers to adjacent socket implementations in linked list.
    implementation_type* next_;
    implementation_type* prev_;
  };

  // The type of the reactor used for connect operations.
  typedef detail::select_reactor<true> reactor_type;

  // The maximum number of buffers to support in a single operation.
  enum { max_buffers = 16 };

  // Constructor.
  win_iocp_socket_service(asio::io_service& io_service)
    : asio::detail::service_base<
        win_iocp_socket_service<Protocol> >(io_service),
      iocp_service_(asio::use_service<win_iocp_io_service>(io_service)),
      reactor_(0),
      mutex_(),
      impl_list_(0)
  {
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
    // Close all implementations, causing all operations to complete.
    asio::detail::mutex::scoped_lock lock(mutex_);
    implementation_type* impl = impl_list_;
    while (impl)
    {
      asio::error_code ignored_ec;
      close(*impl, ignored_ec);
      impl = impl->next_;
    }
  }

  // Construct a new socket implementation.
  void construct(implementation_type& impl)
  {
    impl.socket_ = invalid_socket;
    impl.flags_ = 0;
    impl.cancel_token_.reset();
    impl.safe_cancellation_thread_id_ = 0;

    // Insert implementation into linked list of all implementations.
    asio::detail::mutex::scoped_lock lock(mutex_);
    impl.next_ = impl_list_;
    impl.prev_ = 0;
    if (impl_list_)
      impl_list_->prev_ = &impl;
    impl_list_ = &impl;
  }

  // Destroy a socket implementation.
  void destroy(implementation_type& impl)
  {
    if (impl.socket_ != invalid_socket)
    {
      // Check if the reactor was created, in which case we need to close the
      // socket on the reactor as well to cancel any operations that might be
      // running there.
      reactor_type* reactor = static_cast<reactor_type*>(
            interlocked_compare_exchange_pointer(
              reinterpret_cast<void**>(&reactor_), 0, 0));
      if (reactor)
        reactor->close_descriptor(impl.socket_);

      if (impl.flags_ & implementation_type::user_set_linger)
      {
        ::linger opt;
        opt.l_onoff = 0;
        opt.l_linger = 0;
        asio::error_code ignored_ec;
        socket_ops::setsockopt(impl.socket_,
            SOL_SOCKET, SO_LINGER, &opt, sizeof(opt), ignored_ec);
      }

      asio::error_code ignored_ec;
      socket_ops::close(impl.socket_, ignored_ec);
      impl.socket_ = invalid_socket;
      impl.flags_ = 0;
      impl.cancel_token_.reset();
      impl.safe_cancellation_thread_id_ = 0;
    }

    // Remove implementation from linked list of all implementations.
    asio::detail::mutex::scoped_lock lock(mutex_);
    if (impl_list_ == &impl)
      impl_list_ = impl.next_;
    if (impl.prev_)
      impl.prev_->next_ = impl.next_;
    if (impl.next_)
      impl.next_->prev_= impl.prev_;
    impl.next_ = 0;
    impl.prev_ = 0;
  }

  // Open a new socket implementation.
  asio::error_code open(implementation_type& impl,
      const protocol_type& protocol, asio::error_code& ec)
  {
    if (is_open(impl))
    {
      ec = asio::error::already_open;
      return ec;
    }

    socket_holder sock(socket_ops::socket(protocol.family(), protocol.type(),
          protocol.protocol(), ec));
    if (sock.get() == invalid_socket)
      return ec;

    HANDLE sock_as_handle = reinterpret_cast<HANDLE>(sock.get());
    iocp_service_.register_handle(sock_as_handle);

    impl.socket_ = sock.release();
    impl.flags_ = 0;
    impl.cancel_token_.reset(static_cast<void*>(0), noop_deleter());
    impl.protocol_ = protocol;
    ec = asio::error_code();
    return ec;
  }

  // Assign a native socket to a socket implementation.
  asio::error_code assign(implementation_type& impl,
      const protocol_type& protocol, const native_type& native_socket,
      asio::error_code& ec)
  {
    if (is_open(impl))
    {
      ec = asio::error::already_open;
      return ec;
    }

    iocp_service_.register_handle(native_socket.as_handle());

    impl.socket_ = native_socket;
    impl.flags_ = 0;
    impl.cancel_token_.reset(static_cast<void*>(0), noop_deleter());
    impl.protocol_ = protocol;
    ec = asio::error_code();
    return ec;
  }

  // Determine whether the socket is open.
  bool is_open(const implementation_type& impl) const
  {
    return impl.socket_ != invalid_socket;
  }

  // Destroy a socket implementation.
  asio::error_code close(implementation_type& impl,
      asio::error_code& ec)
  {
    if (is_open(impl))
    {
      // Check if the reactor was created, in which case we need to close the
      // socket on the reactor as well to cancel any operations that might be
      // running there.
      reactor_type* reactor = static_cast<reactor_type*>(
            interlocked_compare_exchange_pointer(
              reinterpret_cast<void**>(&reactor_), 0, 0));
      if (reactor)
        reactor->close_descriptor(impl.socket_);

      if (socket_ops::close(impl.socket_, ec) == socket_error_retval)
        return ec;

      impl.socket_ = invalid_socket;
      impl.flags_ = 0;
      impl.cancel_token_.reset();
      impl.safe_cancellation_thread_id_ = 0;
    }

    ec = asio::error_code();
    return ec;
  }

  // Get the native socket representation.
  native_type native(implementation_type& impl)
  {
    return impl.socket_;
  }

  // Cancel all operations associated with the socket.
  asio::error_code cancel(implementation_type& impl,
      asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
    }
    else if (impl.safe_cancellation_thread_id_ == 0)
    {
      // No operations have been started, so there's nothing to cancel.
      ec = asio::error_code();
    }
    else if (impl.safe_cancellation_thread_id_ == ::GetCurrentThreadId())
    {
      // Asynchronous operations have been started from the current thread only,
      // so it is safe to try to cancel them using CancelIo.
      socket_type sock = impl.socket_;
      HANDLE sock_as_handle = reinterpret_cast<HANDLE>(sock);
      if (!::CancelIo(sock_as_handle))
      {
        DWORD last_error = ::GetLastError();
        ec = asio::error_code(last_error, asio::native_ecat);
      }
      else
      {
        ec = asio::error_code();
      }
    }
    else
    {
      // Asynchronous operations have been started from more than one thread,
      // so cancellation is not safe.
      ec = asio::error::operation_not_supported;
    }

    return ec;
  }

  // Determine whether the socket is at the out-of-band data mark.
  bool at_mark(const implementation_type& impl,
      asio::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return false;
    }

    asio::detail::ioctl_arg_type value = 0;
    socket_ops::ioctl(impl.socket_, SIOCATMARK, &value, ec);
    return ec ? false : value != 0;
  }

  // Determine the number of bytes available for reading.
  std::size_t available(const implementation_type& impl,
      asio::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return 0;
    }

    asio::detail::ioctl_arg_type value = 0;
    socket_ops::ioctl(impl.socket_, FIONREAD, &value, ec);
    return ec ? static_cast<std::size_t>(0) : static_cast<std::size_t>(value);
  }

  // Bind the socket to the specified local endpoint.
  asio::error_code bind(implementation_type& impl,
      const endpoint_type& endpoint, asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return ec;
    }

    socket_ops::bind(impl.socket_, endpoint.data(), endpoint.size(), ec);
    return ec;
  }

  // Place the socket into the state where it will listen for new connections.
  asio::error_code listen(implementation_type& impl, int backlog,
      asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return ec;
    }

    socket_ops::listen(impl.socket_, backlog, ec);
    return ec;
  }

  // Set a socket option.
  template <typename Option>
  asio::error_code set_option(implementation_type& impl,
      const Option& option, asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return ec;
    }

    if (option.level(impl.protocol_) == custom_socket_option_level
        && option.name(impl.protocol_) == enable_connection_aborted_option)
    {
      if (option.size(impl.protocol_) != sizeof(int))
      {
        ec = asio::error::invalid_argument;
      }
      else
      {
        if (*reinterpret_cast<const int*>(option.data(impl.protocol_)))
          impl.flags_ |= implementation_type::enable_connection_aborted;
        else
          impl.flags_ &= ~implementation_type::enable_connection_aborted;
        ec = asio::error_code();
      }
      return ec;
    }
    else
    {
      if (option.level(impl.protocol_) == SOL_SOCKET
          && option.name(impl.protocol_) == SO_LINGER)
      {
        impl.flags_ |= implementation_type::user_set_linger;
      }

      socket_ops::setsockopt(impl.socket_,
          option.level(impl.protocol_), option.name(impl.protocol_),
          option.data(impl.protocol_), option.size(impl.protocol_), ec);
      return ec;
    }
  }

  // Set a socket option.
  template <typename Option>
  asio::error_code get_option(const implementation_type& impl,
      Option& option, asio::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return ec;
    }

    if (option.level(impl.protocol_) == custom_socket_option_level
        && option.name(impl.protocol_) == enable_connection_aborted_option)
    {
      if (option.size(impl.protocol_) != sizeof(int))
      {
        ec = asio::error::invalid_argument;
      }
      else
      {
        int* target = reinterpret_cast<int*>(option.data(impl.protocol_));
        if (impl.flags_ & implementation_type::enable_connection_aborted)
          *target = 1;
        else
          *target = 0;
        option.resize(impl.protocol_, sizeof(int));
        ec = asio::error_code();
      }
      return ec;
    }
    else
    {
      size_t size = option.size(impl.protocol_);
      socket_ops::getsockopt(impl.socket_,
          option.level(impl.protocol_), option.name(impl.protocol_),
          option.data(impl.protocol_), &size, ec);
      if (!ec)
        option.resize(impl.protocol_, size);
      return ec;
    }
  }

  // Perform an IO control command on the socket.
  template <typename IO_Control_Command>
  asio::error_code io_control(implementation_type& impl,
      IO_Control_Command& command, asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return ec;
    }

    socket_ops::ioctl(impl.socket_, command.name(),
        static_cast<ioctl_arg_type*>(command.data()), ec);

    if (!ec && command.name() == static_cast<int>(FIONBIO))
    {
      if (command.get())
        impl.flags_ |= implementation_type::user_set_non_blocking;
      else
        impl.flags_ &= ~implementation_type::user_set_non_blocking;
    }

    return ec;
  }

  // Get the local endpoint.
  endpoint_type local_endpoint(const implementation_type& impl,
      asio::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return endpoint_type();
    }

    endpoint_type endpoint;
    socket_addr_len_type addr_len = endpoint.capacity();
    if (socket_ops::getsockname(impl.socket_, endpoint.data(), &addr_len, ec))
      return endpoint_type();
    endpoint.resize(addr_len);
    return endpoint;
  }

  // Get the remote endpoint.
  endpoint_type remote_endpoint(const implementation_type& impl,
      asio::error_code& ec) const
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return endpoint_type();
    }

    if (impl.socket_.have_remote_endpoint())
    {
      // Check if socket is still connected.
      DWORD connect_time = 0;
      size_t connect_time_len = sizeof(connect_time);
      if (socket_ops::getsockopt(impl.socket_, SOL_SOCKET, SO_CONNECT_TIME,
            &connect_time, &connect_time_len, ec) == socket_error_retval)
      {
        return endpoint_type();
      }
      if (connect_time == 0xFFFFFFFF)
      {
        ec = asio::error::not_connected;
        return endpoint_type();
      }

      ec = asio::error_code();
      return impl.socket_.remote_endpoint();
    }
    else
    {
      endpoint_type endpoint;
      socket_addr_len_type addr_len = endpoint.capacity();
      if (socket_ops::getpeername(impl.socket_, endpoint.data(), &addr_len, ec))
        return endpoint_type();
      endpoint.resize(addr_len);
      return endpoint;
    }
  }

  /// Disable sends or receives on the socket.
  asio::error_code shutdown(implementation_type& impl,
      socket_base::shutdown_type what, asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return ec;
    }

    socket_ops::shutdown(impl.socket_, what, ec);
    return ec;
  }

  // Send the given data to the peer. Returns the number of bytes sent.
  template <typename ConstBufferSequence>
  size_t send(implementation_type& impl, const ConstBufferSequence& buffers,
      socket_base::message_flags flags, asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      asio::const_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(asio::buffer_size(buffer));
      bufs[i].buf = const_cast<char*>(
          asio::buffer_cast<const char*>(buffer));
      total_buffer_size += asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      ec = asio::error_code();
      return 0;
    }

    // Send the data.
    DWORD bytes_transferred = 0;
    int result = ::WSASend(impl.socket_, bufs,
        i, &bytes_transferred, flags, 0, 0);
    if (result != 0)
    {
      DWORD last_error = ::WSAGetLastError();
      if (last_error == ERROR_NETNAME_DELETED)
        last_error = WSAECONNRESET;
      else if (last_error == ERROR_PORT_UNREACHABLE)
        last_error = WSAECONNREFUSED;
      ec = asio::error_code(last_error, asio::native_ecat);
      return 0;
    }

    ec = asio::error_code();
    return bytes_transferred;
  }

  template <typename ConstBufferSequence, typename Handler>
  class send_operation
    : public operation
  {
  public:
    send_operation(asio::io_service& io_service,
        weak_cancel_token_type cancel_token,
        const ConstBufferSequence& buffers, Handler handler)
      : operation(
          &send_operation<ConstBufferSequence, Handler>::do_completion_impl,
          &send_operation<ConstBufferSequence, Handler>::destroy_impl),
        work_(io_service),
        cancel_token_(cancel_token),
        buffers_(buffers),
        handler_(handler)
    {
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef send_operation<ConstBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

#if defined(ASIO_ENABLE_BUFFER_DEBUGGING)
      // Check whether buffers are still valid.
      typename ConstBufferSequence::const_iterator iter
        = handler_op->buffers_.begin();
      typename ConstBufferSequence::const_iterator end
        = handler_op->buffers_.end();
      while (iter != end)
      {
        asio::const_buffer buffer(*iter);
        asio::buffer_cast<const char*>(buffer);
        ++iter;
      }
#endif // defined(ASIO_ENABLE_BUFFER_DEBUGGING)

      // Map non-portable errors to their portable counterparts.
      asio::error_code ec(last_error, asio::native_ecat);
      if (ec.value() == ERROR_NETNAME_DELETED)
      {
        if (handler_op->cancel_token_.expired())
          ec = asio::error::operation_aborted;
        else
          ec = asio::error::connection_reset;
      }
      else if (ec.value() == ERROR_PORT_UNREACHABLE)
      {
        ec = asio::error::connection_refused;
      }

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef send_operation<ConstBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);
    }

    asio::io_service::work work_;
    weak_cancel_token_type cancel_token_;
    ConstBufferSequence buffers_;
    Handler handler_;
  };

  // Start an asynchronous send. The data being sent must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_send(implementation_type& impl, const ConstBufferSequence& buffers,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->io_service().post(bind_handler(handler,
            asio::error::bad_descriptor, 0));
      return;
    }

    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);

    // Allocate and construct an operation to wrap the handler.
    typedef send_operation<ConstBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr,
        this->io_service(), impl.cancel_token_, buffers, handler);

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      asio::const_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(asio::buffer_size(buffer));
      bufs[i].buf = const_cast<char*>(
          asio::buffer_cast<const char*>(buffer));
      total_buffer_size += asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      asio::io_service::work work(this->io_service());
      ptr.reset();
      asio::error_code error;
      iocp_service_.post(bind_handler(handler, error, 0));
      return;
    }

    // Send the data.
    DWORD bytes_transferred = 0;
    int result = ::WSASend(impl.socket_, bufs, i,
        &bytes_transferred, flags, ptr.get(), 0);
    DWORD last_error = ::WSAGetLastError();

    // Check if the operation completed immediately.
    if (result != 0 && last_error != WSA_IO_PENDING)
    {
      asio::io_service::work work(this->io_service());
      ptr.reset();
      asio::error_code ec(last_error, asio::native_ecat);
      iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
    }
    else
    {
      ptr.release();
    }
  }

  // Send a datagram to the specified endpoint. Returns the number of bytes
  // sent.
  template <typename ConstBufferSequence>
  size_t send_to(implementation_type& impl, const ConstBufferSequence& buffers,
      const endpoint_type& destination, socket_base::message_flags flags,
      asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      asio::const_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(asio::buffer_size(buffer));
      bufs[i].buf = const_cast<char*>(
          asio::buffer_cast<const char*>(buffer));
    }

    // Send the data.
    DWORD bytes_transferred = 0;
    int result = ::WSASendTo(impl.socket_, bufs, i, &bytes_transferred,
        flags, destination.data(), destination.size(), 0, 0);
    if (result != 0)
    {
      DWORD last_error = ::WSAGetLastError();
      if (last_error == ERROR_PORT_UNREACHABLE)
        last_error = WSAECONNREFUSED;
      ec = asio::error_code(last_error, asio::native_ecat);
      return 0;
    }

    ec = asio::error_code();
    return bytes_transferred;
  }

  template <typename ConstBufferSequence, typename Handler>
  class send_to_operation
    : public operation
  {
  public:
    send_to_operation(asio::io_service& io_service,
        const ConstBufferSequence& buffers, Handler handler)
      : operation(
          &send_to_operation<ConstBufferSequence, Handler>::do_completion_impl,
          &send_to_operation<ConstBufferSequence, Handler>::destroy_impl),
        work_(io_service),
        buffers_(buffers),
        handler_(handler)
    {
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef send_to_operation<ConstBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

#if defined(ASIO_ENABLE_BUFFER_DEBUGGING)
      // Check whether buffers are still valid.
      typename ConstBufferSequence::const_iterator iter
        = handler_op->buffers_.begin();
      typename ConstBufferSequence::const_iterator end
        = handler_op->buffers_.end();
      while (iter != end)
      {
        asio::const_buffer buffer(*iter);
        asio::buffer_cast<const char*>(buffer);
        ++iter;
      }
#endif // defined(ASIO_ENABLE_BUFFER_DEBUGGING)

      // Map non-portable errors to their portable counterparts.
      asio::error_code ec(last_error, asio::native_ecat);
      if (ec.value() == ERROR_PORT_UNREACHABLE)
      {
        ec = asio::error::connection_refused;
      }

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef send_to_operation<ConstBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);
    }

    asio::io_service::work work_;
    ConstBufferSequence buffers_;
    Handler handler_;
  };

  // Start an asynchronous send. The data being sent must be valid for the
  // lifetime of the asynchronous operation.
  template <typename ConstBufferSequence, typename Handler>
  void async_send_to(implementation_type& impl,
      const ConstBufferSequence& buffers, const endpoint_type& destination,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->io_service().post(bind_handler(handler,
            asio::error::bad_descriptor, 0));
      return;
    }

    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);

    // Allocate and construct an operation to wrap the handler.
    typedef send_to_operation<ConstBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr,
        this->io_service(), buffers, handler);

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename ConstBufferSequence::const_iterator iter = buffers.begin();
    typename ConstBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      asio::const_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(asio::buffer_size(buffer));
      bufs[i].buf = const_cast<char*>(
          asio::buffer_cast<const char*>(buffer));
    }

    // Send the data.
    DWORD bytes_transferred = 0;
    int result = ::WSASendTo(impl.socket_, bufs, i, &bytes_transferred,
        flags, destination.data(), destination.size(), ptr.get(), 0);
    DWORD last_error = ::WSAGetLastError();

    // Check if the operation completed immediately.
    if (result != 0 && last_error != WSA_IO_PENDING)
    {
      asio::io_service::work work(this->io_service());
      ptr.reset();
      asio::error_code ec(last_error, asio::native_ecat);
      iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
    }
    else
    {
      ptr.release();
    }
  }

  // Receive some data from the peer. Returns the number of bytes received.
  template <typename MutableBufferSequence>
  size_t receive(implementation_type& impl,
      const MutableBufferSequence& buffers,
      socket_base::message_flags flags, asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      asio::mutable_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(asio::buffer_size(buffer));
      bufs[i].buf = asio::buffer_cast<char*>(buffer);
      total_buffer_size += asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      ec = asio::error_code();
      return 0;
    }

    // Receive some data.
    DWORD bytes_transferred = 0;
    DWORD recv_flags = flags;
    int result = ::WSARecv(impl.socket_, bufs, i,
        &bytes_transferred, &recv_flags, 0, 0);
    if (result != 0)
    {
      DWORD last_error = ::WSAGetLastError();
      if (last_error == ERROR_NETNAME_DELETED)
        last_error = WSAECONNRESET;
      else if (last_error == ERROR_PORT_UNREACHABLE)
        last_error = WSAECONNREFUSED;
      ec = asio::error_code(last_error, asio::native_ecat);
      return 0;
    }
    if (bytes_transferred == 0)
    {
      ec = asio::error::eof;
      return 0;
    }

    ec = asio::error_code();
    return bytes_transferred;
  }

  template <typename MutableBufferSequence, typename Handler>
  class receive_operation
    : public operation
  {
  public:
    receive_operation(asio::io_service& io_service,
        weak_cancel_token_type cancel_token,
        const MutableBufferSequence& buffers, Handler handler)
      : operation(
          &receive_operation<
            MutableBufferSequence, Handler>::do_completion_impl,
          &receive_operation<
            MutableBufferSequence, Handler>::destroy_impl),
        work_(io_service),
        cancel_token_(cancel_token),
        buffers_(buffers),
        handler_(handler)
    {
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef receive_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

#if defined(ASIO_ENABLE_BUFFER_DEBUGGING)
      // Check whether buffers are still valid.
      typename MutableBufferSequence::const_iterator iter
        = handler_op->buffers_.begin();
      typename MutableBufferSequence::const_iterator end
        = handler_op->buffers_.end();
      while (iter != end)
      {
        asio::mutable_buffer buffer(*iter);
        asio::buffer_cast<char*>(buffer);
        ++iter;
      }
#endif // defined(ASIO_ENABLE_BUFFER_DEBUGGING)

      // Map non-portable errors to their portable counterparts.
      asio::error_code ec(last_error, asio::native_ecat);
      if (ec.value() == ERROR_NETNAME_DELETED)
      {
        if (handler_op->cancel_token_.expired())
          ec = asio::error::operation_aborted;
        else
          ec = asio::error::connection_reset;
      }
      else if (ec.value() == ERROR_PORT_UNREACHABLE)
      {
        ec = asio::error::connection_refused;
      }

      // Check for connection closed.
      else if (!ec && bytes_transferred == 0)
      {
        ec = asio::error::eof;
      }

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef receive_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);
    }

    asio::io_service::work work_;
    weak_cancel_token_type cancel_token_;
    MutableBufferSequence buffers_;
    Handler handler_;
  };

  // Start an asynchronous receive. The buffer for the data being received
  // must be valid for the lifetime of the asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_receive(implementation_type& impl,
      const MutableBufferSequence& buffers,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->io_service().post(bind_handler(handler,
            asio::error::bad_descriptor, 0));
      return;
    }

    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);

    // Allocate and construct an operation to wrap the handler.
    typedef receive_operation<MutableBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr,
        this->io_service(), impl.cancel_token_, buffers, handler);

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    size_t total_buffer_size = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      asio::mutable_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(asio::buffer_size(buffer));
      bufs[i].buf = asio::buffer_cast<char*>(buffer);
      total_buffer_size += asio::buffer_size(buffer);
    }

    // A request to receive 0 bytes on a stream socket is a no-op.
    if (impl.protocol_.type() == SOCK_STREAM && total_buffer_size == 0)
    {
      asio::io_service::work work(this->io_service());
      ptr.reset();
      asio::error_code error;
      iocp_service_.post(bind_handler(handler, error, 0));
      return;
    }

    // Receive some data.
    DWORD bytes_transferred = 0;
    DWORD recv_flags = flags;
    int result = ::WSARecv(impl.socket_, bufs, i,
        &bytes_transferred, &recv_flags, ptr.get(), 0);
    DWORD last_error = ::WSAGetLastError();
    if (result != 0 && last_error != WSA_IO_PENDING)
    {
      asio::io_service::work work(this->io_service());
      ptr.reset();
      asio::error_code ec(last_error, asio::native_ecat);
      iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
    }
    else
    {
      ptr.release();
    }
  }

  // Receive a datagram with the endpoint of the sender. Returns the number of
  // bytes received.
  template <typename MutableBufferSequence>
  size_t receive_from(implementation_type& impl,
      const MutableBufferSequence& buffers,
      endpoint_type& sender_endpoint, socket_base::message_flags flags,
      asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return 0;
    }

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      asio::mutable_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(asio::buffer_size(buffer));
      bufs[i].buf = asio::buffer_cast<char*>(buffer);
    }

    // Receive some data.
    DWORD bytes_transferred = 0;
    DWORD recv_flags = flags;
    int endpoint_size = sender_endpoint.capacity();
    int result = ::WSARecvFrom(impl.socket_, bufs, i, &bytes_transferred,
        &recv_flags, sender_endpoint.data(), &endpoint_size, 0, 0);
    if (result != 0)
    {
      DWORD last_error = ::WSAGetLastError();
      if (last_error == ERROR_PORT_UNREACHABLE)
        last_error = WSAECONNREFUSED;
      ec = asio::error_code(last_error, asio::native_ecat);
      return 0;
    }
    if (bytes_transferred == 0)
    {
      ec = asio::error::eof;
      return 0;
    }

    sender_endpoint.resize(endpoint_size);

    ec = asio::error_code();
    return bytes_transferred;
  }

  template <typename MutableBufferSequence, typename Handler>
  class receive_from_operation
    : public operation
  {
  public:
    receive_from_operation(asio::io_service& io_service,
        endpoint_type& endpoint, const MutableBufferSequence& buffers,
        Handler handler)
      : operation(
          &receive_from_operation<
            MutableBufferSequence, Handler>::do_completion_impl,
          &receive_from_operation<
            MutableBufferSequence, Handler>::destroy_impl),
        endpoint_(endpoint),
        endpoint_size_(endpoint.capacity()),
        work_(io_service),
        buffers_(buffers),
        handler_(handler)
    {
    }

    int& endpoint_size()
    {
      return endpoint_size_;
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef receive_from_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

#if defined(ASIO_ENABLE_BUFFER_DEBUGGING)
      // Check whether buffers are still valid.
      typename MutableBufferSequence::const_iterator iter
        = handler_op->buffers_.begin();
      typename MutableBufferSequence::const_iterator end
        = handler_op->buffers_.end();
      while (iter != end)
      {
        asio::mutable_buffer buffer(*iter);
        asio::buffer_cast<char*>(buffer);
        ++iter;
      }
#endif // defined(ASIO_ENABLE_BUFFER_DEBUGGING)

      // Map non-portable errors to their portable counterparts.
      asio::error_code ec(last_error, asio::native_ecat);
      if (ec.value() == ERROR_PORT_UNREACHABLE)
      {
        ec = asio::error::connection_refused;
      }

      // Check for connection closed.
      if (!ec && bytes_transferred == 0)
      {
        ec = asio::error::eof;
      }

      // Record the size of the endpoint returned by the operation.
      handler_op->endpoint_.resize(handler_op->endpoint_size_);

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec, bytes_transferred), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef receive_from_operation<MutableBufferSequence, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);
    }

    endpoint_type& endpoint_;
    int endpoint_size_;
    asio::io_service::work work_;
    MutableBufferSequence buffers_;
    Handler handler_;
  };

  // Start an asynchronous receive. The buffer for the data being received and
  // the sender_endpoint object must both be valid for the lifetime of the
  // asynchronous operation.
  template <typename MutableBufferSequence, typename Handler>
  void async_receive_from(implementation_type& impl,
      const MutableBufferSequence& buffers, endpoint_type& sender_endp,
      socket_base::message_flags flags, Handler handler)
  {
    if (!is_open(impl))
    {
      this->io_service().post(bind_handler(handler,
            asio::error::bad_descriptor, 0));
      return;
    }

    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);

    // Allocate and construct an operation to wrap the handler.
    typedef receive_from_operation<MutableBufferSequence, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr,
        this->io_service(), sender_endp, buffers, handler);

    // Copy buffers into WSABUF array.
    ::WSABUF bufs[max_buffers];
    typename MutableBufferSequence::const_iterator iter = buffers.begin();
    typename MutableBufferSequence::const_iterator end = buffers.end();
    DWORD i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
    {
      asio::mutable_buffer buffer(*iter);
      bufs[i].len = static_cast<u_long>(asio::buffer_size(buffer));
      bufs[i].buf = asio::buffer_cast<char*>(buffer);
    }

    // Receive some data.
    DWORD bytes_transferred = 0;
    DWORD recv_flags = flags;
    int result = ::WSARecvFrom(impl.socket_, bufs, i, &bytes_transferred,
        &recv_flags, sender_endp.data(), &ptr.get()->endpoint_size(),
        ptr.get(), 0);
    DWORD last_error = ::WSAGetLastError();
    if (result != 0 && last_error != WSA_IO_PENDING)
    {
      asio::io_service::work work(this->io_service());
      ptr.reset();
      asio::error_code ec(last_error, asio::native_ecat);
      iocp_service_.post(bind_handler(handler, ec, bytes_transferred));
    }
    else
    {
      ptr.release();
    }
  }

  // Accept a new connection.
  template <typename Socket>
  asio::error_code accept(implementation_type& impl, Socket& peer,
      endpoint_type* peer_endpoint, asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return ec;
    }

    // We cannot accept a socket that is already open.
    if (peer.is_open())
    {
      ec = asio::error::already_open;
      return ec;
    }

    for (;;)
    {
      asio::error_code ec;
      socket_holder new_socket;
      socket_addr_len_type addr_len = 0;
      if (peer_endpoint)
      {
        addr_len = peer_endpoint->capacity();
        new_socket.reset(socket_ops::accept(impl.socket_,
              peer_endpoint->data(), &addr_len, ec));
      }
      else
      {
        new_socket.reset(socket_ops::accept(impl.socket_, 0, 0, ec));
      }

      if (ec)
      {
        if (ec == asio::error::connection_aborted
            && !(impl.flags_ & implementation_type::enable_connection_aborted))
        {
          // Retry accept operation.
          continue;
        }
        else
        {
          return ec;
        }
      }

      if (peer_endpoint)
        peer_endpoint->resize(addr_len);

      peer.assign(impl.protocol_, new_socket.get(), ec);
      if (!ec)
        new_socket.release();
      return ec;
    }
  }

  template <typename Socket, typename Handler>
  class accept_operation
    : public operation
  {
  public:
    accept_operation(win_iocp_io_service& io_service,
        socket_type socket, socket_type new_socket, Socket& peer,
        const protocol_type& protocol, endpoint_type* peer_endpoint,
        bool enable_connection_aborted, Handler handler)
      : operation(
          &accept_operation<Socket, Handler>::do_completion_impl,
          &accept_operation<Socket, Handler>::destroy_impl),
        io_service_(io_service),
        socket_(socket),
        new_socket_(new_socket),
        peer_(peer),
        protocol_(protocol),
        peer_endpoint_(peer_endpoint),
        work_(io_service.io_service()),
        enable_connection_aborted_(enable_connection_aborted),
        handler_(handler)
    {
    }

    socket_type new_socket()
    {
      return new_socket_.get();
    }

    void* output_buffer()
    {
      return output_buffer_;
    }

    DWORD address_length()
    {
      return sizeof(sockaddr_storage_type) + 16;
    }

  private:
    static void do_completion_impl(operation* op,
        DWORD last_error, size_t bytes_transferred)
    {
      // Take ownership of the operation object.
      typedef accept_operation<Socket, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);

      // Map Windows error ERROR_NETNAME_DELETED to connection_aborted.
      if (last_error == ERROR_NETNAME_DELETED)
      {
        last_error = WSAECONNABORTED;
      }

      // Restart the accept operation if we got the connection_aborted error
      // and the enable_connection_aborted socket option is not set.
      if (last_error == WSAECONNABORTED
          && !ptr.get()->enable_connection_aborted_)
      {
        // Reset OVERLAPPED structure.
        ptr.get()->Internal = 0;
        ptr.get()->InternalHigh = 0;
        ptr.get()->Offset = 0;
        ptr.get()->OffsetHigh = 0;
        ptr.get()->hEvent = 0;

        // Create a new socket for the next connection, since the AcceptEx call
        // fails with WSAEINVAL if we try to reuse the same socket.
        asio::error_code ec;
        ptr.get()->new_socket_.reset();
        ptr.get()->new_socket_.reset(socket_ops::socket(
              ptr.get()->protocol_.family(), ptr.get()->protocol_.type(),
              ptr.get()->protocol_.protocol(), ec));
        if (ptr.get()->new_socket() != invalid_socket)
        {
          // Accept a connection.
          DWORD bytes_read = 0;
          BOOL result = ::AcceptEx(ptr.get()->socket_, ptr.get()->new_socket(),
              ptr.get()->output_buffer(), 0, ptr.get()->address_length(),
              ptr.get()->address_length(), &bytes_read, ptr.get());
          last_error = ::WSAGetLastError();

          // Check if the operation completed immediately.
          if (!result && last_error != WSA_IO_PENDING)
          {
            if (last_error == ERROR_NETNAME_DELETED
                || last_error == WSAECONNABORTED)
            {
              // Post this handler so that operation will be restarted again.
              ptr.get()->io_service_.post_completion(ptr.get(), last_error, 0);
              ptr.release();
              return;
            }
            else
            {
              // Operation already complete. Continue with rest of this handler.
            }
          }
          else
          {
            // Asynchronous operation has been successfully restarted.
            ptr.release();
            return;
          }
        }
      }

      // Get the address of the peer.
      endpoint_type peer_endpoint;
      if (last_error == 0)
      {
        LPSOCKADDR local_addr = 0;
        int local_addr_length = 0;
        LPSOCKADDR remote_addr = 0;
        int remote_addr_length = 0;
        GetAcceptExSockaddrs(handler_op->output_buffer(), 0,
            handler_op->address_length(), handler_op->address_length(),
            &local_addr, &local_addr_length, &remote_addr, &remote_addr_length);
        if (remote_addr_length > peer_endpoint.capacity())
        {
          last_error = WSAEINVAL;
        }
        else
        {
          using namespace std; // For memcpy.
          memcpy(peer_endpoint.data(), remote_addr, remote_addr_length);
          peer_endpoint.resize(remote_addr_length);
        }
      }

      // Need to set the SO_UPDATE_ACCEPT_CONTEXT option so that getsockname
      // and getpeername will work on the accepted socket.
      if (last_error == 0)
      {
        SOCKET update_ctx_param = handler_op->socket_;
        asio::error_code ec;
        if (socket_ops::setsockopt(handler_op->new_socket_.get(),
              SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
              &update_ctx_param, sizeof(SOCKET), ec) != 0)
        {
          last_error = ec.value();
        }
      }

      // If the socket was successfully accepted, transfer ownership of the
      // socket to the peer object.
      if (last_error == 0)
      {
        asio::error_code ec;
        handler_op->peer_.assign(handler_op->protocol_,
            native_type(handler_op->new_socket_.get(), peer_endpoint), ec);
        if (ec)
          last_error = ec.value();
        else
          handler_op->new_socket_.release();
      }

      // Pass endpoint back to caller.
      if (handler_op->peer_endpoint_)
        *handler_op->peer_endpoint_ = peer_endpoint;

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made.
      Handler handler(handler_op->handler_);

      // Free the memory associated with the handler.
      ptr.reset();

      // Call the handler.
      asio::error_code ec(last_error, asio::native_ecat);
      asio_handler_invoke_helpers::invoke(
          detail::bind_handler(handler, ec), &handler);
    }

    static void destroy_impl(operation* op)
    {
      // Take ownership of the operation object.
      typedef accept_operation<Socket, Handler> op_type;
      op_type* handler_op(static_cast<op_type*>(op));
      typedef handler_alloc_traits<Handler, op_type> alloc_traits;
      handler_ptr<alloc_traits> ptr(handler_op->handler_, handler_op);
    }

    win_iocp_io_service& io_service_;
    socket_type socket_;
    socket_holder new_socket_;
    Socket& peer_;
    protocol_type protocol_;
    endpoint_type* peer_endpoint_;
    asio::io_service::work work_;
    unsigned char output_buffer_[(sizeof(sockaddr_storage_type) + 16) * 2];
    bool enable_connection_aborted_;
    Handler handler_;
  };

  // Start an asynchronous accept. The peer and peer_endpoint objects
  // must be valid until the accept's handler is invoked.
  template <typename Socket, typename Handler>
  void async_accept(implementation_type& impl, Socket& peer,
      endpoint_type* peer_endpoint, Handler handler)
  {
    // Check whether acceptor has been initialised.
    if (!is_open(impl))
    {
      this->io_service().post(bind_handler(handler,
            asio::error::bad_descriptor));
      return;
    }

    // Check that peer socket has not already been opened.
    if (peer.is_open())
    {
      this->io_service().post(bind_handler(handler,
            asio::error::already_open));
      return;
    }

    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);

    // Create a new socket for the connection.
    asio::error_code ec;
    socket_holder sock(socket_ops::socket(impl.protocol_.family(),
          impl.protocol_.type(), impl.protocol_.protocol(), ec));
    if (sock.get() == invalid_socket)
    {
      this->io_service().post(bind_handler(handler, ec));
      return;
    }

    // Allocate and construct an operation to wrap the handler.
    typedef accept_operation<Socket, Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    socket_type new_socket = sock.get();
    bool enable_connection_aborted =
      (impl.flags_ & implementation_type::enable_connection_aborted);
    handler_ptr<alloc_traits> ptr(raw_ptr,
        iocp_service_, impl.socket_, new_socket, peer, impl.protocol_,
        peer_endpoint, enable_connection_aborted, handler);
    sock.release();

    // Accept a connection.
    DWORD bytes_read = 0;
    BOOL result = ::AcceptEx(impl.socket_, ptr.get()->new_socket(),
        ptr.get()->output_buffer(), 0, ptr.get()->address_length(),
        ptr.get()->address_length(), &bytes_read, ptr.get());
    DWORD last_error = ::WSAGetLastError();

    // Check if the operation completed immediately.
    if (!result && last_error != WSA_IO_PENDING)
    {
      if (!enable_connection_aborted
          && (last_error == ERROR_NETNAME_DELETED
            || last_error == WSAECONNABORTED))
      {
        // Post handler so that operation will be restarted again. We do not
        // perform the AcceptEx again here to avoid the possibility of starving
        // other handlers.
        iocp_service_.post_completion(ptr.get(), last_error, 0);
        ptr.release();
      }
      else
      {
        asio::io_service::work work(this->io_service());
        ptr.reset();
        asio::error_code ec(last_error, asio::native_ecat);
        iocp_service_.post(bind_handler(handler, ec));
      }
    }
    else
    {
      ptr.release();
    }
  }

  // Connect the socket to the specified endpoint.
  asio::error_code connect(implementation_type& impl,
      const endpoint_type& peer_endpoint, asio::error_code& ec)
  {
    if (!is_open(impl))
    {
      ec = asio::error::bad_descriptor;
      return ec;
    }

    // Perform the connect operation.
    socket_ops::connect(impl.socket_,
        peer_endpoint.data(), peer_endpoint.size(), ec);
    return ec;
  }

  template <typename Handler>
  class connect_handler
  {
  public:
    connect_handler(socket_type socket, bool user_set_non_blocking,
        boost::shared_ptr<bool> completed,
        asio::io_service& io_service,
        reactor_type& reactor, Handler handler)
      : socket_(socket),
        user_set_non_blocking_(user_set_non_blocking),
        completed_(completed),
        io_service_(io_service),
        reactor_(reactor),
        work_(io_service),
        handler_(handler)
    {
    }

    bool operator()(const asio::error_code& result)
    {
      // Check whether a handler has already been called for the connection.
      // If it has, then we don't want to do anything in this handler.
      if (*completed_)
        return true;

      // Cancel the other reactor operation for the connection.
      *completed_ = true;
      reactor_.enqueue_cancel_ops_unlocked(socket_);

      // Check whether the operation was successful.
      if (result)
      {
        io_service_.post(bind_handler(handler_, result));
        return true;
      }

      // Get the error code from the connect operation.
      int connect_error = 0;
      size_t connect_error_len = sizeof(connect_error);
      asio::error_code ec;
      if (socket_ops::getsockopt(socket_, SOL_SOCKET, SO_ERROR,
            &connect_error, &connect_error_len, ec) == socket_error_retval)
      {
        io_service_.post(bind_handler(handler_, ec));
        return true;
      }

      // If connection failed then post the handler with the error code.
      if (connect_error)
      {
        ec = asio::error_code(
            connect_error, asio::native_ecat);
        io_service_.post(bind_handler(handler_, ec));
        return true;
      }

      // Revert socket to blocking mode unless the user requested otherwise.
      if (!user_set_non_blocking_)
      {
        ioctl_arg_type non_blocking = 0;
        if (socket_ops::ioctl(socket_, FIONBIO, &non_blocking, ec))
        {
          io_service_.post(bind_handler(handler_, ec));
          return true;
        }
      }

      // Post the result of the successful connection operation.
      ec = asio::error_code();
      io_service_.post(bind_handler(handler_, ec));
      return true;
    }

  private:
    socket_type socket_;
    bool user_set_non_blocking_;
    boost::shared_ptr<bool> completed_;
    asio::io_service& io_service_;
    reactor_type& reactor_;
    asio::io_service::work work_;
    Handler handler_;
  };

  // Start an asynchronous connect.
  template <typename Handler>
  void async_connect(implementation_type& impl,
      const endpoint_type& peer_endpoint, Handler handler)
  {
    if (!is_open(impl))
    {
      this->io_service().post(bind_handler(handler,
            asio::error::bad_descriptor));
      return;
    }

    // Update the ID of the thread from which cancellation is safe.
    if (impl.safe_cancellation_thread_id_ == 0)
      impl.safe_cancellation_thread_id_ = ::GetCurrentThreadId();
    else if (impl.safe_cancellation_thread_id_ != ::GetCurrentThreadId())
      impl.safe_cancellation_thread_id_ = ~DWORD(0);

    // Check if the reactor was already obtained from the io_service.
    reactor_type* reactor = static_cast<reactor_type*>(
          interlocked_compare_exchange_pointer(
            reinterpret_cast<void**>(&reactor_), 0, 0));
    if (!reactor)
    {
      reactor = &(asio::use_service<reactor_type>(this->io_service()));
      interlocked_exchange_pointer(
          reinterpret_cast<void**>(&reactor_), reactor);
    }

    // Mark the socket as non-blocking so that the connection will take place
    // asynchronously.
    ioctl_arg_type non_blocking = 1;
    asio::error_code ec;
    if (socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec))
    {
      this->io_service().post(bind_handler(handler, ec));
      return;
    }

    // Start the connect operation.
    if (socket_ops::connect(impl.socket_, peer_endpoint.data(),
          peer_endpoint.size(), ec) == 0)
    {
      // Revert socket to blocking mode unless the user requested otherwise.
      if (!(impl.flags_ & implementation_type::user_set_non_blocking))
      {
        non_blocking = 0;
        socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ec);
      }

      // The connect operation has finished successfully so we need to post the
      // handler immediately.
      this->io_service().post(bind_handler(handler, ec));
    }
    else if (ec == asio::error::in_progress
        || ec == asio::error::would_block)
    {
      // The connection is happening in the background, and we need to wait
      // until the socket becomes writeable.
      boost::shared_ptr<bool> completed(new bool(false));
      reactor->start_write_and_except_ops(impl.socket_,
          connect_handler<Handler>(
            impl.socket_,
            (impl.flags_ & implementation_type::user_set_non_blocking) != 0,
            completed, this->io_service(), *reactor, handler));
    }
    else
    {
      // Revert socket to blocking mode unless the user requested otherwise.
      if (!(impl.flags_ & implementation_type::user_set_non_blocking))
      {
        non_blocking = 0;
        asio::error_code ignored_ec;
        socket_ops::ioctl(impl.socket_, FIONBIO, &non_blocking, ignored_ec);
      }

      // The connect operation has failed, so post the handler immediately.
      this->io_service().post(bind_handler(handler, ec));
    }
  }

private:
  // Helper function to provide InterlockedCompareExchangePointer functionality
  // on very old Platform SDKs.
  void* interlocked_compare_exchange_pointer(void** dest, void* exch, void* cmp)
  {
#if defined(_WIN32_WINNT) && (_WIN32_WINNT <= 0x400) && (_M_IX86)
    return reinterpret_cast<void*>(InterlockedCompareExchange(
          reinterpret_cast<LONG*>(dest), reinterpret_cast<LONG>(exch),
          reinterpret_cast<LONG>(cmp)));
#else
    return InterlockedCompareExchangePointer(dest, exch, cmp);
#endif
  }

  // Helper function to provide InterlockedExchangePointer functionality on very
  // old Platform SDKs.
  void* interlocked_exchange_pointer(void** dest, void* val)
  {
#if defined(_WIN32_WINNT) && (_WIN32_WINNT <= 0x400) && (_M_IX86)
    return reinterpret_cast<void*>(InterlockedExchange(
          reinterpret_cast<LONG*>(dest), reinterpret_cast<LONG>(val)));
#else
    return InterlockedExchangePointer(dest, val);
#endif
  }

  // The IOCP service used for running asynchronous operations and dispatching
  // handlers.
  win_iocp_io_service& iocp_service_;

  // The reactor used for performing connect operations. This object is created
  // only if needed.
  reactor_type* reactor_;

  // Mutex to protect access to the linked list of implementations. 
  asio::detail::mutex mutex_;

  // The head of a linked list of all implementations.
  implementation_type* impl_list_;
};

} // namespace detail
} // namespace asio

#endif // defined(ASIO_HAS_IOCP)

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_WIN_IOCP_SOCKET_SERVICE_HPP
