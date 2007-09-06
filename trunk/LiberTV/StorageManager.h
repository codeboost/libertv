#pragma once

#include "Utils.h"
#include "Metatorrent.h"

#define MEGABYTE 1024 * 1024
#define GIGABYTE 1024 * MEGABYTE



class FStorageManager 
{
	FString						m_StorageRoot; 

	//Returns a relative path
	bool GetFileName(FString& pFileName, vidtype nShowID, fsize_type nRequiredSize);
	bool CreateNewFileName(FString& pFileName, vidtype nShowID, fsize_type nRequiredSize, bool CreateFileName=true); 

public:
	FStorageManager()
	{
	}
	bool	InitStorage(const char* pStrStorageRoot); 

	//Returns full path to a file located in storage
	//FString FileName(const char* pRelFile);

	bool	ReserveStorage(vidtype VideoID, fsize_type nReqSize, FString& pOutFileName);
	bool	CleanupStorage(fsize_type nSizeRequired); 
	bool	DiscardStorage(vidtype videoID); 

	fsize_type	GetDiskFreeSpace(); 

	const FString &GetStorageRoot()
	{
		return m_StorageRoot; 
	}
};