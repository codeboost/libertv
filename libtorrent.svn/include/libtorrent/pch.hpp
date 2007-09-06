#ifdef _MSC_VER
#pragma once

#define _WINVER			0x0500
#define _WIN32_WINDOWS	0x0500
#define _WIN32_WINNT	0x0500
#ifndef NDEBUG
#define NDEBUG
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#pragma warning(disable: 4503) //'identifier' : decorated name length exceeded, name was truncated
#pragma warning(disable: 4797)
#pragma warning(disable: 4311) //'variable' : pointer truncation from 'type' to 'type'
#pragma warning(disable: 4312) //'operation' : conversion from 'type1' to 'type2' of greater size
#pragma warning(disable: 4101) //'identifier' : unreferenced local variable
#pragma warning(disable: 4099) //'identifier' : type name first seen using 'objecttype1' now seen using 'objecttype2'
#pragma warning(disable: 4996) //was declared deprecated
#pragma warning(disable: 4355) //this used in base member initializer list
#pragma warning(disable: 4307) //integral constant overflow
#pragma warning(disable: 4244)	//conversion from 'small-type' to 'large-type': possible loss of data

#include <asio/detail/socket_types.hpp>	//must be included before windows.h/winsock.h
#include "Ws2tcpip.h"
#include "Wspiapi.h"
#include <WinSock2.h>
#include <Windows.h>

#endif //#ifdef _MSC_VER

#include <vector>
#include <iostream>
#include <cctype>
#include <iomanip>
#include <sstream>

#define TORRENT_DISABLE_ENCRYPTION
#define BOOST_BUILD_PCH_ENABLED
#define OPENSSL_NO_SHA1
#define OPENSSL_NO_SHA
#define HEADER_SHA_H	//prevent OpenSSL/sha.h from being included - collision with our SHA1 implementation

//#define TORRENT_DHT_VERBOSE_LOGGING

#ifdef BOOST_BUILD_PCH_ENABLED

#include <algorithm>
#include <asio/ip/host_name.hpp>
#include <assert.h>
#include <bitset>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/function.hpp>
#include <boost/integer_traits.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator_adaptors.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/limits.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/next_prior.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/preprocessor/repetition/enum_shifted_params.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <boost/version.hpp>
#include <boost/weak_ptr.hpp>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>
#include <zlib.h>

#ifdef __OBJC__
#define Protocol Protocol_
#endif

#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>
#include <asio/io_service.hpp>
#include <asio/deadline_timer.hpp>
#include <asio/write.hpp>
#include <asio/strand.hpp>

#ifdef __OBJC__ 
#undef Protocol
#endif

#endif

