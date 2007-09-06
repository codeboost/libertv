#include "stdafx.h"
#include "FDownload.h"
#include "vfw.h"

int GetMediaType(FMediaFile* mf)
{
	//assert(mf);
	unsigned char sigbuf[32];
	FILE* fp = fopen(mf->m_FileName, "rb");
	if (!fp) 
	{ 
		_DBGAlert("Cannot open file %s", mf->m_FileName); 
		return 1; 
	}

	if (fread(sigbuf, 1, 16, fp) < 11) 
	{ 
		fclose(fp); 
		return 1; 
	}
	fclose(fp);

	if (sigbuf[0] == 'R' && sigbuf[1] == 'I' && sigbuf[2] == 'F' && sigbuf[3] == 'F')
	{
		AVIFileInit();

		PAVIFILE vid;
		PAVISTREAM videostream;
		PAVISTREAM audiostream; 
		AVIFILEINFO vidInfo;
		AVISTREAMINFO streamInfo;

		HRESULT hr = AVIFileOpen(&vid, mf->m_FileName, OF_READ, NULL);
		if (hr != 0) 
		{
			return 1;
		}

		if (AVIFileInfo(vid, &vidInfo, sizeof(vidInfo)))
		{
			AVIFileClose(vid);
			return 1;
		}

		if (AVIFileGetStream(vid, &videostream, streamtypeVIDEO, 0))
		{
			AVIFileClose(vid);
			return 1;
		}

		AVIStreamInfo(videostream, &streamInfo, sizeof(AVISTREAMINFO));
		double fps = (double)streamInfo.dwRate / (double)streamInfo.dwScale;

		BITMAPINFOHEADER bih;
		LONG fmtSize = sizeof(BITMAPINFOHEADER);
		AVIStreamReadFormat(videostream, 0, &bih, &fmtSize);
		char fcc[5] = {0};
		memcpy(fcc, &bih.biCompression, sizeof(bih.biCompression));
		FString audioFormat = "none";

		if (AVIFileGetStream(vid, &audiostream, streamtypeAUDIO, 0) == 0)
		{
			WAVEFORMATEX ah;
			fmtSize = sizeof(WAVEFORMATEX);
			AVIStreamReadFormat(audiostream, 0, &ah, &fmtSize);

			switch (ah.wFormatTag)
			{
			case 0x0000: audioFormat = "???"; break;
			case 0x0001: audioFormat = "PCM"; break;
			case 0x0002: audioFormat = "ADPCM"; break;
			case 0x0003: audioFormat = "IEEE"; break;
			case 0x0050: audioFormat = "MPEG"; break;
			case 0x0055: audioFormat = "MP3";break;
			case 0x0092: audioFormat = "AC3"; break;
			case 0x0160: audioFormat = "MSAUDIO1/DIVX"; break;
			case 0x0161: audioFormat = "MSAUDIO2/DIVX"; break;
			case 0x0162: audioFormat = "WMA9"; break;
			case 0x0163: audioFormat = "WMA9"; break;
			case 0x2000: audioFormat = "AC3"; break;
			}
		}


		if (audiostream != NULL)
			AVIStreamClose(audiostream);
		
		if (videostream != NULL)
			AVIStreamClose(videostream);
		
		if (vid != NULL)
			AVIFileClose(vid);

		if (fps > 0.0)
		{
			mf->m_FPS = fps;
			mf->m_DurationMS = (duration_type)(1000.0 * streamInfo.dwLength / fps);
			mf->m_MediaType.Format("avi.%s.%s", fcc, audioFormat);
		}
		return 0;
	}
	else
	if (sigbuf[0] == 0x00 && sigbuf[1] == 0x00 && sigbuf[2] == 0x01)
	{
		mf->m_MediaType = "mpeg.mpeg";
		mf->m_FPS = 0;
		mf->m_DurationMS = 0;

		return 0;
	}
	else
	if (sigbuf[4] == 'm' && 
		sigbuf[5] == 'o' && 
		sigbuf[6] == 'o' && 
		sigbuf[7] == 'v')
	{
		mf->m_MediaType = "mov";
		mf->m_FPS = 0; 
		mf->m_DurationMS = 0; 
		return 0;
	}
	else
	if (sigbuf[4] == 'f' && 
		sigbuf[5] == 't' && 
		sigbuf[6] == 'y' && 
		sigbuf[7] == 'p' && 
		sigbuf[8] == 'q' && 
		sigbuf[9] == 't')
	{
		mf->m_MediaType = "mov";
		mf->m_FPS = 0; 
		mf->m_DurationMS = 0; 
		return 0; 
	}
	else
	if (sigbuf[4] == 'f' && 
		sigbuf[5] == 't' && 
		sigbuf[6] == 'y' && 
		sigbuf[7] == 'p' && 
		sigbuf[8] == 'M' && 
		sigbuf[9] == '4' && 
		sigbuf[10] == 'V')
	{
		mf->m_MediaType = "m4v";
		mf->m_FPS = 0;
		mf->m_DurationMS = 0; 
		return 0; 
	}
	else
	if (strcmp((const char*)&sigbuf[4], "ftypmp4") == 0 || strcmp((const char*)&sigbuf[4], "ftypisom") == 0)
	{
		mf->m_MediaType = "mp4";
		mf->m_FPS = 0;
		mf->m_DurationMS = 0; 
		return 0; 
	}
	else
	if (sigbuf[0] == 0xFF && sigbuf[1] == 0xD8 && sigbuf[2] == 0xFF && sigbuf[3] == 0xE0 && 
		sigbuf[6] == 0x4A && sigbuf[7] == 0x46 && sigbuf[8] == 0x49 && sigbuf[9] == 0x46)
	{
		mf->m_MediaType = "jpg";
		mf->m_FPS = 0;
		mf->m_DurationMS = 0;
		return 0;
	}
	else
	if (sigbuf[0] == 'C' && sigbuf[1] == 'W' && sigbuf[2] == 'S')
	{
		mf->m_MediaType = "swf";
		mf->m_FPS = 0;
		mf->m_DurationMS = 0;
		return 0;
	}
	else
	if (sigbuf[0] == 'F' && sigbuf[1] == 'L' && sigbuf[2] == 'V')
	{
		mf->m_MediaType = "flv";
		mf->m_FPS = 0;
		mf->m_DurationMS = 0;
		return 0;
	}
	else
	if (sigbuf[0] == 0x89 && sigbuf[1] == 0x50 && sigbuf[2] == 0x4E && sigbuf[3] == 0x47 && sigbuf[4] == 0x0D && sigbuf[5] == 0x0A && sigbuf[6] == 0x1A && sigbuf[7] == 0x0A)
	{
		mf->m_MediaType = "png";
		mf->m_FPS = 0;
		mf->m_DurationMS = 0;
		return 0;
	}
	else
	if (sigbuf[0] == 'B' && sigbuf[1] == 'M')
	{
		mf->m_MediaType = "bmp";
		mf->m_FPS = 0;
		mf->m_DurationMS = 0;
		return 0;
	}
	else
	if (sigbuf[0] == 0x30 && sigbuf[1] == 0x26 && sigbuf[2] == 0xb2 && sigbuf[3] == 0x75 && sigbuf[4] == 0x8e &&
		sigbuf[5] == 0x66 && sigbuf[6] == 0xcf)
	{
		mf->m_MediaType = "asf";
		mf->m_FPS = 0; 
		mf->m_DurationMS = 0; 
		return 0; 
	}
	else
	{
		mf->m_MediaType = "none";
		mf->m_FPS = 0; 
		mf->m_DurationMS = 0; 
	}
	return 0;
}