#ifndef __FVIDEOCONV_H__
#define __FVIDEOCONV_H__


#include <fstream>
#include <iosfwd>
#include "FDownload.h"

enum FileTypes
{
	ftInvalid, ftMtt, ftMtt2, ftMtt3, ftTorrent, ftUnknown
};

class FVideoConv
{
public:
	
	static FileTypes GetMetafileType(const tchar* pszFileName);
	static BOOL MTT2Video(const tchar* pszMttFileName, FDownloadEx* pVideo); 
	static BOOL T2Video(const tchar* pszTorrent, FDownloadEx* pVideo, const tchar* pszOutDir); 

	static FileTypes  ConvertMetafile(const tchar* pszFileName, FDownloadEx* pVideo, const tchar* pszOutDir); 

	
};
#endif //__FVIDEOCONV_H__