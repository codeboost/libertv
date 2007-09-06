#ifndef _METATORRENT_H
#define _METATORRENT_H

#include "libtorrent/torrent_handle.hpp"
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/file.hpp"
#include "libtorrent/storage.hpp"
#include "libtorrent/hasher.hpp"

#include "boost/filesystem/operations.hpp"
using namespace boost::filesystem;

using namespace libtorrent;

#define MT_OK	1
#define MT_ERR	0

#define MT_MAGIC 0x4043

typedef unsigned long vidtype; //videoID data type
typedef unsigned long dword;
struct ClipInfo
{
	UINT64 duration;
	UINT64 filesize;
	std::string clipname;
	int clipID;
	ClipInfo()
	{
		duration = 0; 
		clipID = 0; 
	}
};

struct TorrentInfo
{
	UINT64 duration;
	std::vector<ClipInfo> clips;
	torrent_info t;
	std::string directory; 
	UINT64 GetTotalSize(); 

	//fields used during download
	dword			dwFlags; 
	torrent_handle	handle;			
};

struct MTinfo
{
	UINT64 duration; //in milliseconds, 64 bits
	std::vector<TorrentInfo> torrents;
	std::string mtName;
	std::string mtTracker;
	vidtype videoID;
	UINT64 GetTotalSize(); 
	MTinfo()
	{
		videoID = 0; 
	}
};

class Metatorrent  
{
public:
	Metatorrent() { Clear(); paused = false; }
	Metatorrent(const char* name, UINT64 length) 
	{ 
		paused = false;
		Create(name, length);
		//This is  DISABLED
		//mTorrent.duration = length; 
	}

	void Create(const char* name, UINT64 length);
	int  Load(const char* fname, bool verbose = true);
	TorrentInfo*  AddTorrent();
	int	 AddClipToTorrent(TorrentInfo* t, const char* fname, UINT64 duration);
	int  DelTorrent(int index);
    int  Save(const char* fname);
	int  CreateTorrents();
	void SetName(const char* name) { mTorrent.mtName = name; }
	void SetTracker(const char* addr) { mTorrent.mtTracker = addr; }
	void SetVideoID(int id) { mTorrent.videoID = id; }
	void Clear();
	MTinfo& GetMT() { return mTorrent; }   
	int	 BuildHashes(); 
	UINT64 GetTotalSize();

	bool IsPaused() { return paused; }
	void Pause() { paused = true; }
	void Resume() { paused = false; }
	UINT64 GetVideoDuration();

private:
	
	MTinfo mTorrent;
	bool paused;

	int GetString(std::ifstream& f, std::string* s);
	int GetBuffer(std::ifstream& f, std::vector<char>* buf);
	int GetInt(std::ifstream& f, int* val);
	int GetInt64(std::ifstream& f, UINT64* val);

	void PutString(std::ofstream& f, std::string& s);
	void PutInt64(std::ofstream& f, UINT64 val);
	void PutInt(std::ofstream& f, int val);
	void PutBuffer(std::ofstream& f, std::vector<char>& buf);

};

#endif
