// FHtmlEventDispatcher.h : Declaration of the CFHtmlEventDispatcher

#pragma once
#include "resource.h"       // main symbols

#include "Liber2Player.h"



// CFHtmlEventDispatcher

class ATL_NO_VTABLE CFHtmlEventDispatcher : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFHtmlEventDispatcher, &CLSID_FHtmlEventDispatcher>,
	public IDispatchImpl<IFHtmlEventDispatcher, &IID_IFHtmlEventDispatcher, &LIBID_Liber2PlayerLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CFHtmlEventDispatcher()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FHTMLEVENTDISPATCHER)


BEGIN_COM_MAP(CFHtmlEventDispatcher)
	COM_INTERFACE_ENTRY2(IDispatch, IFHtmlEventDispatcher)
	COM_INTERFACE_ENTRY(IFHtmlEventDispatcher)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:

	STDMETHOD(DownloadMTT)(BSTR Address);
	STDMETHOD(MF_SetActiveSection)(LONG lSection);
	STDMETHOD(PlayVideo)(VARIANT varVideoID);
	STDMETHOD(OnToolbarCommand)(LONG idiControl, LONG wParam, LONG lParam);
	STDMETHOD(RemoveVideo)(VARIANT varVideoID, LONG* pvResult);
	STDMETHOD(OnButtonClicked)(LONG dialogID, LONG buttonID);
	STDMETHOD(Conf_OnLoad)(LONG lPageID);
	STDMETHOD(Conf_OnUnload)(LONG lSectionID);
	STDMETHOD(Conf_OnCancel)(LONG lSectionID);
	STDMETHOD(Conf_OnSave)(void);
	STDMETHOD(ControlBar_SetVolume)(LONG lVolume);
	STDMETHOD(ControlBar_PlayButton)(LONG lPlaying);
	STDMETHOD(OutputDebugString)(VARIANT vStr);
	STDMETHOD(Seekbar_SetPosition)(LONGLONG lPosition);
	STDMETHOD(PauseResume)(VARIANT videoID);
	STDMETHOD(DebugPrintPeerList)(VARIANT videoID);
	STDMETHOD(RestoreFullScreen)(void);
	STDMETHOD(PauseCurrentVideo)(void);
	STDMETHOD(ReannounceAll)(void);
	STDMETHOD(ReannounceVideo)(VARIANT videoID);
	STDMETHOD(BrowseForFolder)(VARIANT vtFolder, BSTR* vtOutFolder);
	STDMETHOD(OnAdComplete)(void);
    STDMETHOD(OpenSettings)(void);
    STDMETHOD(Search)(BSTR searchString, LONG flag);
	STDMETHOD(SelectSubtitle)(BSTR subLang, ULONG videoID);
	STDMETHOD(GetPlayingVideoID)(ULONG* videoID);
	STDMETHOD(GetVideoInfo)(ULONG videoID, IDispatch* pDispScript, IDispatch* aOutArray);
	STDMETHOD(GetPlayerVersion)(BSTR* version);
	STDMETHOD(AddEpisodeComment)(ULONG videoID, ULONG episodeID);
	STDMETHOD(EpisodeDetails)(ULONG ulEpisodeID);
	STDMETHOD(GetSearchString)(BSTR* bstrSearchStr);
	STDMETHOD(PlayNextClip)(void);
	STDMETHOD(PlayNextClipEx)(LONG lClip);
	STDMETHOD(ShowVideoOptions)(ULONG videoID);
	STDMETHOD(ToggleSubtitles)(ULONG videoID);
	STDMETHOD(ToggleInfobar)(void);
	STDMETHOD(VideoDetails)(ULONG videoID);
	STDMETHOD(PauseVideo)(void);
	STDMETHOD(NavigateGuide)(BSTR bstrURL, ULONG ulFlags);
	STDMETHOD(PlayPrevClip)(void);
	STDMETHOD(SetLabel)(VARIANT videoID, BSTR bstrLabel);
	STDMETHOD(AddGlobalLabel)(BSTR bstrLabel);
	STDMETHOD(RemoveGlobalLabel)(BSTR bstrLabel);
	STDMETHOD(GetGlobalLabels)(IDispatch* pLabelsArray);
	STDMETHOD(ShowContextMenu)(VARIANT wParam, VARIANT lParam, VARIANT* ppRes);
	STDMETHOD(ShowAddLabelDlg)(BSTR bstrLabel, BSTR* bstrRetVal);
	STDMETHOD(PostPlayerMessage)(ULONG uMsg, ULONG wParam, ULONG lParam);
	STDMETHOD(AddLabel)(VARIANT varVideoID, BSTR bstrNewLabel);
	STDMETHOD(RemoveLabel)(VARIANT varVideoID, BSTR bstrLabel);
	STDMETHOD(GetDownloadStatus)(IDispatch* pDispScript, IDispatch* pArray, IDispatch* pOptions);
	STDMETHOD(_SendMessage)(BSTR bstrWndName, ULONG ulMessage, ULONG wParam, ULONG lParam);
	STDMETHOD(GetSessionStatus)(IDispatch* pStatus);
	STDMETHOD(GetPeerInfo)(VARIANT varVideoID, IDispatch* pScript, IDispatch* pPeerInfo);
	STDMETHOD(SetMainFrameFocus)(void);
	STDMETHOD(SetEngineSetting)(BSTR bstrName, BSTR bstrValue);
	STDMETHOD(IsClientConnectible)(VARIANT_BOOL* bConnectible);
	STDMETHOD(OpenSiteURL)(BSTR bstrURL);
	STDMETHOD(SetServer)(BSTR bstrURL, BSTR bstrGuideURL, BSTR bstrLang, BSTR bstrExtra);
	STDMETHOD(RSSGetChannels)(IDispatch* pDispScript, BSTR bstrFilter, IDispatch* pChannels);
	STDMETHOD(RSSGetItems)(IDispatch* pDispScript, IDispatch* pArray, IDispatch* pDispOptions);
	STDMETHOD(RSSDeleteChannel)(VARIANT vtChannelID);
	STDMETHOD(RSSDownloadItem)(UINT uiItemGuid);
	STDMETHOD(RSSAddFeed)(IDispatch* pDisp, ULONG* pulResult);
	STDMETHOD(RSSRefreshChannel)(VARIANT vtChannelID);
	STDMETHOD(RSSAddFeedEx)(BSTR bstrURL, BSTR bstrName, BSTR bstrOptions, ULONG* ulChannelID);
	STDMETHOD(GetCollectionCounts)(BSTR* bstrCounts);

	STDMETHOD(GetDownloadStatusArray)(IDispatch* pScript, IDispatch* pArray, BSTR bstrNameFilter, BSTR bstrLabel, BSTR bstrSort, BSTR bstrSortDir, ULONG dwFlags);
public:
	STDMETHOD(ExecCmd)(BSTR bstrCmd, VARIANT vtParam1, VARIANT vtParam2, BSTR* bstrRes);
	};

//OBJECT_ENTRY_AUTO(__uuidof(FHtmlEventDispatcher), CFHtmlEventDispatcher)
