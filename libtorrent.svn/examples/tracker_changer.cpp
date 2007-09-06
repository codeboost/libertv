#include "libtorrent/pch.hpp"
#include <iostream>
#include <fstream>
#include <iterator>
#include <exception>

#include "libtorrent/config.hpp"

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "libtorrent/extensions/metadata_transfer.hpp"
#include "libtorrent/extensions/ut_pex.hpp"

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/identify_client.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/ip_filter.hpp"
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/file.hpp"
#include "libtorrent/storage.hpp"
#include "libtorrent/hasher.hpp"
#include "libtorrent/file_pool.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace boost::filesystem;
using namespace libtorrent;


using namespace libtorrent; 
int main(int argc, char** argv)
{

	boost::filesystem::path::default_name_check(boost::filesystem::native); 

	if (argc < 4)
	{
		printf("Adds or replaces the tracker in a torrent file\n");
		printf("Parameters: TorrentFile Tracker OutputFile [-replace]\n");

		return -1; 
	}
	
	std::string torrent_file = argv[1];
	std::string tracker = argv[2];
	std::string outfile = argv[3];
	std::string replace;
	if (argc > 4)
		replace = argv[4];


	std::ifstream in_t(torrent_file.c_str(), std::ios_base::binary);

	if (!in_t.is_open())
	{
		printf("Cannot open torrent file: %s\n", torrent_file.c_str());
		return -1;
	}

	try{
		in_t.unsetf(std::ios_base::skipws);
		entry e = bdecode(std::istream_iterator<char>(in_t), std::istream_iterator<char>()); 
		in_t.close();
		torrent_info t(e);

		if (replace == "-replace")
		{
			t.remove_trackers();
		}
		else
			t.add_tracker(""); //for uTorrent

		printf("Adding tracker %s to the torrent.\n", tracker.c_str()); 
		
		t.add_tracker(tracker);


		printf("Saving to %s\n", outfile.c_str()); 
		ofstream out(outfile.c_str(), std::ios_base::binary);
		e = t.create_torrent();
		libtorrent::bencode(std::ostream_iterator<char>(out), e);

	} catch (std::exception& ex)
	{
		printf("Exception while processing: %s\n", ex.what());
		return -1; 
	}

	
	printf("Done.\n");
	return 0; 
}