#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	// {EBE1FB08-3957-47ca-AF13-5827E5442E56}
	DEFINE_GUID(IID_IDirectVobSub, 
	0xebe1fb08, 0x3957, 0x47ca, 0xaf, 0x13, 0x58, 0x27, 0xe5, 0x44, 0x2e, 0x56);

    MIDL_INTERFACE("EBE1FB08-3957-47ca-AF13-5827E5442E56")
	IDirectVobSub : public IUnknown 
    {
        STDMETHOD(get_FileName) (THIS_
                    WCHAR* fn	// fn should point to a buffer allocated to at least the length of MAX_PATH (=260)
                 ) PURE;

        STDMETHOD(put_FileName) (THIS_
                    WCHAR* fn
                 ) PURE;

		STDMETHOD(get_LanguageCount) (THIS_
					int* nLangs
                 ) PURE;

		STDMETHOD(get_LanguageName) (THIS_
					int iLanguage, 
					WCHAR** ppName	// the returned *ppName is allocated with CoTaskMemAlloc
                 ) PURE;

		STDMETHOD(get_SelectedLanguage) (THIS_
					int* iSelected
                 ) PURE;

        STDMETHOD(put_SelectedLanguage) (THIS_
					int iSelected
                 ) PURE;

        STDMETHOD(get_HideSubtitles) (THIS_
                    bool* fHideSubtitles
                 ) PURE;

        STDMETHOD(put_HideSubtitles) (THIS_
                    bool fHideSubtitles
                 ) PURE;

        STDMETHOD(get_PreBuffering) (THIS_
					bool* fDoPreBuffering
                 ) PURE;

        STDMETHOD(put_PreBuffering) (THIS_
					bool fDoPreBuffering
                 ) PURE;

        STDMETHOD(get_Placement) (THIS_
					bool* fOverridePlacement,
					int* xperc,
					int* yperc
                 ) PURE;

        STDMETHOD(put_Placement) (THIS_
					bool fOverridePlacement,
					int xperc,
					int yperc
                 ) PURE;

        STDMETHOD(get_VobSubSettings) (THIS_
					bool* fBuffer,
					bool* fOnlyShowForcedSubs,
					bool* fPolygonize
                 ) PURE;

        STDMETHOD(put_VobSubSettings) (THIS_
					bool fBuffer,
					bool fOnlyShowForcedSubs,
					bool fPolygonize
                 ) PURE;

        STDMETHOD(get_TextSettings) (THIS_
					void* lf,
					int lflen, // depending on lflen, lf must point to LOGFONTA or LOGFONTW
					COLORREF* color,
					bool* fShadow,
					bool* fOutline,
					bool* fAdvancedRenderer
                 ) PURE;

        STDMETHOD(put_TextSettings) (THIS_
					void* lf,
					int lflen,
					COLORREF color,
					bool fShadow,
					bool fOutline,
					bool fAdvancedRenderer
                 ) PURE;

        STDMETHOD(get_Flip) (THIS_
                    bool* fPicture,
                    bool* fSubtitles
                 ) PURE;

        STDMETHOD(put_Flip) (THIS_
                    bool fPicture,
                    bool fSubtitles
                 ) PURE;

        STDMETHOD(get_OSD) (THIS_
					bool* fOSD
                 ) PURE;

        STDMETHOD(put_OSD) (THIS_
					bool fOSD
                 ) PURE;

        STDMETHOD(get_SaveFullPath) (THIS_
					bool* fSaveFullPath
                 ) PURE;

        STDMETHOD(put_SaveFullPath) (THIS_
					bool fSaveFullPath
                 ) PURE;

        STDMETHOD(get_SubtitleTiming) (THIS_
					int* delay,
					int* speedmul,
					int* speeddiv
                 ) PURE;

        STDMETHOD(put_SubtitleTiming) (THIS_
					int delay,
					int speedmul,
					int speeddiv
                 ) PURE;

        STDMETHOD(get_MediaFPS) (THIS_
					bool* fEnabled,
					double* fps
                 ) PURE;

        STDMETHOD(put_MediaFPS) (THIS_
					bool fEnabled,
					double fps
                 ) PURE;

		//

        STDMETHOD(get_ColorFormat) (THIS_
					int* iPosition
                 ) PURE;

        STDMETHOD(put_ColorFormat) (THIS_
					int iPosition
                 ) PURE;

		//

        STDMETHOD(get_ZoomRect) (THIS_
					NORMALIZEDRECT* rect
                 ) PURE;

        STDMETHOD(put_ZoomRect) (THIS_
					NORMALIZEDRECT* rect
                 ) PURE;

		//

        STDMETHOD(UpdateRegistry) (THIS_
                 ) PURE;

		//

		STDMETHOD(HasConfigDialog) (THIS_
					int iSelected
				) PURE;

		STDMETHOD(ShowConfigDialog) (THIS_	// if available, this will popup a child dialog allowing the user to edit the style options
					int iSelected, 
					HWND hWndParent
				) PURE; 

		//

        STDMETHOD(IsSubtitleReloaderLocked) (THIS_
					bool* fLocked
                 ) PURE;

        STDMETHOD(LockSubtitleReloader) (THIS_
					bool fLock
                 ) PURE;

		STDMETHOD(get_SubtitleReloader) (THIS_
					bool* fDisabled
                 ) PURE;

        STDMETHOD(put_SubtitleReloader) (THIS_
					bool fDisable
                 ) PURE;

		//

        STDMETHOD(get_ExtendPicture) (THIS_
					int* horizontal, // 0 - disabled, 1 - mod32 extension (width = (width+31)&~31)
					int* vertical, // 0 - disabled, 1 - 16:9, 2 - 4:3, 0x80 - crop (use crop together with 16:9 or 4:3, eg 0x81 will crop to 16:9 if the picture was taller)
					int* resx2, // 0 - disabled, 1 - enabled, 2 - depends on the original resolution
					int* resx2minw, // resolution doubler will be used if width*height <= resx2minw*resx2minh (resx2minw*resx2minh equals to 384*288 by default)
					int* resx2minh 
                 ) PURE;

        STDMETHOD(put_ExtendPicture) (THIS_
					int horizontal,
					int vertical,
					int resx2,
					int resx2minw,
					int resx2minh
                 ) PURE;

        STDMETHOD(get_LoadSettings) (THIS_
					int* level, // 0 - when needed, 1 - always, 2 - disabled
					bool* fExternalLoad,
					bool* fWebLoad, 
					bool* fEmbeddedLoad
                 ) PURE;

        STDMETHOD(put_LoadSettings) (THIS_
					int level,
					bool fExternalLoad,
					bool fWebLoad, 
					bool fEmbeddedLoad
				) PURE;
	};

	// {FE6EC6A0-21CA-4970-9EF0-B296F7F38AF0}
	DEFINE_GUID(IID_ISubClock, 
	0xfe6ec6a0, 0x21ca, 0x4970, 0x9e, 0xf0, 0xb2, 0x96, 0xf7, 0xf3, 0x8a, 0xf0);

    MIDL_INTERFACE("FE6EC6A0-21CA-4970-9EF0-B296F7F38AF0")
	ISubClock : public IUnknown
	{
        STDMETHOD (SetTime) (THIS_
					REFERENCE_TIME rt
				) PURE;
        STDMETHOD_(REFERENCE_TIME, GetTime) (THIS_
				) PURE;
	};

	// {AB52FC9C-2415-4dca-BC1C-8DCC2EAE8150}
	DEFINE_GUID(IID_IDirectVobSub2, 
	0xab52fc9c, 0x2415, 0x4dca, 0xbc, 0x1c, 0x8d, 0xcc, 0x2e, 0xae, 0x81, 0x50);

    MIDL_INTERFACE("AB52FC9C-2415-4dca-BC1C-8DCC2EAE8150")
	IDirectVobSub2 : public IDirectVobSub
	{
        STDMETHOD(AdviseSubClock) (THIS_
                    ISubClock* pSubClock
				) PURE;

		STDMETHOD_(bool, get_Forced) (THIS_
				);

        STDMETHOD(put_Forced) (THIS_
                    bool fForced
				) PURE;
	};


#ifdef __cplusplus
}
#endif
