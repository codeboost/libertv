#include "stdafx.h"
#include "Metatorrent.h"
#include <iostream>
#include <fstream>

const int pieceSize = 512 * 1024;


UINT64 TorrentInfo::GetTotalSize()
{
	UINT64 Sum = 0; 
	for (std::vector<ClipInfo>::iterator it = clips.begin(); it != clips.end(); it++)
		Sum += it->filesize; 
	return Sum; 
}

UINT64 MTinfo::GetTotalSize()
{
	UINT64 Sum = 0; 
	for (std::vector<TorrentInfo>::iterator it = torrents.begin(); it != torrents.end(); it++)
		Sum += it->GetTotalSize(); 
	return Sum; 
}

UINT64 Metatorrent::GetVideoDuration()
{
	return GetMT().duration; 
}

void Metatorrent::Create(const char* name, UINT64 length)
{
	mTorrent.mtName = name;
	mTorrent.duration = length;
}

TorrentInfo* Metatorrent::AddTorrent()
{
	TorrentInfo ti;
	ti.t.add_tracker("");
	ti.duration = 0;
//	const char* creator_str = "libertv";
//	ti.t.set_creator(creator_str); 
	mTorrent.torrents.push_back(ti);
	TorrentInfo* pti = &mTorrent.torrents[mTorrent.torrents.size() - 1];
	
	return pti;
}

int Metatorrent::AddClipToTorrent(TorrentInfo* ti, const char* clipname, UINT64 duration)
{
	static int clipID = 0;

	path truePath = path(clipname, native);
	if (truePath.branch_path().empty())
		truePath = initial_path() / truePath;

	path videoPath(truePath.leaf(), native); 
	
	std::ifstream fi(truePath.string().c_str(), std::ios_base::binary);
	fi.seekg(0, std::ios_base::end);
	int size = fi.tellg();
	fi.close();

	path dir = truePath.branch_path().leaf();
	
	ti->t.add_file(dir / videoPath, size);

	ti->directory = truePath.branch_path().branch_path().string(); 

	//std::cout << "METAT=FILE=" << videoPath.string() << std::endl;

	ClipInfo info;

	info.duration = duration;
	info.clipID = clipID++;
	info.clipname = videoPath.string();
	info.filesize = size;
	ti->clips.push_back(info);

	ti->duration += duration;
	mTorrent.duration += duration;
	
	return MT_OK;
}

int Metatorrent::BuildHashes()
{
#if 0
	int numPieces = 0;
	try
	{
		for (size_t k = 0; k < mTorrent.torrents.size(); k++)
		{
			TorrentInfo* ti = &mTorrent.torrents[k];
			numPieces += ti->t.num_pieces();
		}

		//divide by 2, don't ask why
		//std::cout << "METAT=Pieces=" << numPieces/2 << std::endl;

		numPieces = 0;
		for (size_t k = 0; k < mTorrent.torrents.size(); k++)
		{
			TorrentInfo* ti = &mTorrent.torrents[k];
			ti->t.set_piece_size(pieceSize);

			libtorrent::file_pool fp; 
			storage st(ti->t, ti->directory, fp);
			int num = ti->t.num_pieces();
			std::vector<char> buf(pieceSize);

			for (int j = 0; j < num; ++j)
			{		
				//std::cout << "METAT=Hashing=" << numPieces << std::endl;		
				st.read(&buf[0], j, 0, (int)ti->t.piece_size(j));
				hasher h(&buf[0], (int)ti->t.piece_size(j));
				sha1_hash ahash = h.final();
				ti->t.set_hash(j, ahash);
				numPieces++;
			}
		}
	}

	catch (std::exception& e)
	{
		std::cout << "ERROR=Exception: " << e.what() << std::endl; 
		return MT_ERR; 
	}
#endif
	return MT_OK; 
}


int Metatorrent::DelTorrent(int index)
{
	mTorrent.duration -= mTorrent.torrents[index].duration;
	mTorrent.torrents.erase(mTorrent.torrents.begin() + index);
	return MT_OK;
}

void Metatorrent::Clear()
{
	for (size_t i = 0; i < mTorrent.torrents.size(); ++i)
	{
		mTorrent.torrents[i].clips.clear();
		mTorrent.torrents[i].duration = 0;
	}
	mTorrent.torrents.clear();
	mTorrent.mtName = "";
	mTorrent.videoID = 0;
	mTorrent.duration = 0;
	mTorrent.mtTracker = "";
}

int Metatorrent::CreateTorrents()
{
	for (unsigned i = 0; i < mTorrent.torrents.size(); ++i)
	{
		std::string fname = mTorrent.mtName + boost::lexical_cast<std::string>(i) + ".torrent";
		//std::cout << "METAT=Creating torrent file " << fname << std::endl;
		

		torrent_info info = mTorrent.torrents[i].t; //copy this		
//		info.remove_trackers();
		info.add_tracker(mTorrent.mtTracker);
		entry newe = info.create_torrent();

		std::ofstream trr(fname.c_str(), std::ios_base::binary);
		try
		{
			libtorrent::bencode(std::ostream_iterator<char>(trr) , newe);
		}
		catch (std::exception&)
		{
			trr.close();
			return MT_ERR;
		}
		trr.close();
	}
	return MT_OK;
}

int Metatorrent::Save(const char* fname)
{
	size_t numTorrents = mTorrent.torrents.size();

	std::ofstream fo(fname, std::ios_base::binary);
	unsigned magic = MT_MAGIC;
	PutInt(fo, magic);
	PutString(fo, mTorrent.mtName);
	PutString(fo, mTorrent.mtTracker);
	PutInt64(fo, mTorrent.duration);
	PutInt(fo, mTorrent.videoID);
	PutInt(fo, (int)numTorrents);
	
	for (size_t i = 0; i < numTorrents; ++i)
	{			
		std::vector<char> buf;
		size_t numClips = mTorrent.torrents[i].clips.size();
		
		PutInt(fo, (int)numClips);
		PutInt64(fo, mTorrent.torrents[i].duration); 

		for (size_t j = 0; j < numClips; ++j)
		{
			PutString(fo, mTorrent.torrents[i].clips[j].clipname);
			PutInt64(fo, mTorrent.torrents[i].clips[j].duration);
			PutInt64(fo, mTorrent.torrents[i].clips[j].filesize);
			PutInt(fo, mTorrent.torrents[i].clips[j].clipID);
		}
		
		try
		{
			entry e = mTorrent.torrents[i].t.create_torrent();
			torrent_info ti = e;			
			//std::cout << "METAT=INFOHASH=" << ti.info_hash() << std::endl;
			libtorrent::bencode(std::back_inserter(buf), e);
		}
		catch (std::exception&)
		{
			if (fo.is_open())
				fo.close();

			return MT_ERR;
		}
		
		PutBuffer(fo, buf);		
	}
	
	fo.close();

	return MT_OK;
}

UINT64 Metatorrent::GetTotalSize()
{
	UINT64 totalSize = 0;
	for (size_t i = 0; i < mTorrent.torrents.size(); ++i)
	{
		for (size_t j = 0; j < mTorrent.torrents[i].clips.size(); ++j)
		{
			totalSize += mTorrent.torrents[i].clips[j].filesize;
		}
	}
	return totalSize;
}

int Metatorrent::Load(const char* fname, bool verbose)
{
	int numTorrents = 0;
	Clear();
	unsigned magic;
	if (strlen(fname) == 0) return MT_ERR;
	std::ifstream fi(fname, std::ios_base::binary);

	if (GetInt(fi,(int*)&magic) == -1 ) return MT_ERR;
	if (magic != MT_MAGIC) return MT_ERR;

	if (GetString(fi, &mTorrent.mtName)  == -1) return MT_ERR;
	if (verbose) std::cout << "METAT=NAME=" << mTorrent.mtName << std::endl;

	if (GetString(fi, &mTorrent.mtTracker) == -1) return MT_ERR;
	if (verbose) std::cout << "METAT=TRACKER=" << mTorrent.mtTracker << std::endl;

	if (GetInt64(fi, &mTorrent.duration) == -1) return MT_ERR;

	if (GetInt(fi, (int*)&mTorrent.videoID) == -1) return MT_ERR;
	if (verbose) std::cout << "METAT=VIDEOID=" << mTorrent.videoID << std::endl;

	if (GetInt(fi, &numTorrents) == -1) return MT_ERR;

	for (int i = 0; i < numTorrents; ++i)
	{
		TorrentInfo* info = AddTorrent();
		if (verbose) std::cout << "METAT=TORRENT" << std::endl;
		int numClips = 0;
		if (GetInt(fi, &numClips) == -1) return MT_ERR;
		if (GetInt64(fi, &info->duration) == -1) return MT_ERR; 
		
		for (int j = 0; j < numClips; ++j)
		{
			ClipInfo clip;
			if (GetString(fi, &clip.clipname) == -1) return MT_ERR;
			if (verbose) std::cout << "METAT=Loading " << clip.clipname << std::endl;
			if (GetInt64(fi, &clip.duration) == -1) return MT_ERR;
			if (GetInt64(fi, &clip.filesize) == -1) return MT_ERR;
			if (GetInt(fi, &clip.clipID) == -1) return MT_ERR;
			info->clips.push_back(clip);
		}

		std::vector<char> buffer;
		if (GetBuffer(fi, &buffer) == -1) return MT_ERR;
		try
		{
			//torrent_info ti();				
			//info->t = ti;
			info->t = libtorrent::bdecode(buffer.begin(), buffer.end());
		}
		catch (std::exception&)
		{
			return MT_ERR;
		}				
	}
	fi.close();
	return MT_OK;
}

void Metatorrent::PutString(std::ofstream& f, std::string& s)
{
	std::streamsize len = (std::streamsize) s.length(); //no trailing 0 in the file
	f.write((const char*)&len, sizeof(std::streamsize));
	f.write((const char*)s.c_str(), len);
}

int Metatorrent::GetString(std::ifstream& f, std::string* s)
{
	assert(s != NULL);

	unsigned len = 0;
	f.read((char*)&len, sizeof(len));

	if (f.fail()) return -1;
	if (len > 4096) return -1;

	char* buf = new char[len + 1]; //allow space for NULL-termination		
	if (!buf) return -1;
	f.read((char*)buf, len);
	if (f.fail()) 
	{
		delete [] buf;
		return -1;
	}
	buf[len] = 0; //add trailing 0 which is not in thefile

	s->reserve(len + 1);
	s->assign(buf, len );
	//*s=buf; 

	delete [] buf;
	return len;
}

void Metatorrent::PutInt(std::ofstream& f, int val)
{
	f.write((const char*)&val, sizeof(int));
}

int Metatorrent::GetInt(std::ifstream& f, int* val)
{
	assert(val != NULL);
	f.read((char*)val, sizeof(int));
	if (f.fail()) return -1;
	return sizeof(int);
}

void Metatorrent::PutInt64(std::ofstream& f, UINT64 val)
{
	f.write((const char*)&val, sizeof(UINT64));
}

int Metatorrent::GetInt64(std::ifstream& f, UINT64* val)
{
	assert(val != NULL);
	f.read((char*)val, sizeof(UINT64));
	if (f.fail()) return -1;
	return sizeof(UINT64);
}

void Metatorrent::PutBuffer(std::ofstream& f, std::vector<char>& buf)
{
	std::streamsize len = (std::streamsize) buf.size();
	f.write((const char*)&len, sizeof(std::streamsize));
	f.write(&buf[0], len);
}

int Metatorrent::GetBuffer(std::ifstream& f, std::vector<char>* buf)
{
	assert(buf != NULL);
	unsigned len = 0;
	f.read((char*)&len, sizeof(len));
	if (f.fail()) return -1;
	//if (len > 16384) return -1; -- WTF was this supposed to prevent?!!!!!
	buf->resize(len);
	f.read((char*)&(*buf)[0], len);
	if (f.fail()) return -1;
	return len;
}


