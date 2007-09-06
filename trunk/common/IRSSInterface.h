#pragma once

#define FEED_REFRESH_STARTED 1
#define FEED_REFRESH_ENDED 0


class IRSSInterface{
public:
	//Called when channel is being refreshed or when channel has been refreshed.
	//dwState = 0 - Channel has been refreshed
	//dwState = 1 - Channel refresh started
	virtual void	OnChannelRefresh(DWORD dwChannelID, DWORD dwState, DWORD dwNewItems) = 0; 
	//Called before a channel is set to Auto Download. 
	//Interface should check with the user where to save the downloaded files.
	//Return: TRUE - set channel's auto download property; FALSE - cancel operation
	virtual BOOL	OnSetAutoDownload() = 0; 
	virtual ~IRSSInterface(){}
};

//Synchronized Guid Map
//Caller must Lock() and Unlock() for thread safety.

class IRSSGuidMap{

public:
	virtual ~IRSSGuidMap(){}
	virtual void	Lock() = 0; 
	virtual void	Unlock() = 0; 
	virtual void	SetGuid(const char* pszGuid, DWORD videoID) = 0;
	virtual DWORD FindGuid(const tchar* pszGuid) = 0; 
};