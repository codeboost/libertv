;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"
  !include "Library.nsh"

;--------------------------------
;General

	RequestExecutionLevel admin

  ;Name and file
  Name "LiberTV"
  OutFile "..\setup\ltv_setup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\LiberTV"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\LiberTV" ""

;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING
	!define MUI_HEADERIMAGE
	!define MUI_HEADERIMAGE_BITMAP "v2.bmp"

;--------------------------------
;Pages

	!insertmacro MUI_PAGE_LICENSE "License.txt"	
	!insertmacro MUI_PAGE_DIRECTORY	
	!insertmacro MUI_PAGE_INSTFILES
	Page custom LTVStartPage
  
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
   
;--------------------------------
;Languages
 
	!insertmacro MUI_LANGUAGE "English"

	ReserveFile "ltvs.ini" ;Your own InstallOptions INI files
	!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS ;InstallOptions plug-in
	!insertmacro MUI_RESERVEFILE_LANGDLL ;Language selection dialog	

;--------------------------------
;Variables

    Var INI_VALUE
    
;--------------------------------
;Installer Sections

Section "LiberTV Program files" SecDummy
 	
	SetOutPath "$INSTDIR"

	File "LiberTV.exe"
	File "LiberTV.tlb"
	File "xvid.ax"
	File "x264.ax"
	File "divxdec.ax"
	;File "/oname=$SYSDIR\xvidltv.dll" "xvidltv.dll" 
	File "xvidltv.dll"
	File "ac3filter.ax"
	File /r "data"
 
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV" "DisplayName" "LiberTV"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV" "NoModify" 1
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV" "NoRepair" 1
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV" "EstimatedSize" 800000
 	WriteUninstaller "uninstall.exe"
;	Store installation folder
	WriteRegStr HKCU "Software\LiberTV" "" "$INSTDIR"
	WriteRegStr HKCU "Software\LiberTV" "Version" "1.0.2.3"
  
;	Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
 
;	Register everything
;	RegDll "$INSTDIR\ovtool.dll"
	ExecWait "$INSTDIR\LiberTV.exe /RegServer"
	!insertmacro InstallLib TLB NOTSHARED NOREBOOT_PROTECTED LiberTV.tlb "$INSTDIR\LiberTV.tlb" $TEMP
SectionEnd

Section "Start Menu Shortcuts"
	CreateDirectory "$SMPROGRAMS\LiberTV"
	CreateShortCut "$SMPROGRAMS\LiberTV\LiberTV.lnk" "$INSTDIR\LiberTV.exe" "" "$INSTDIR\LiberTV.exe" 0
	CreateShortCut "$SMPROGRAMS\LiberTV\Uninstall LiberTV.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
	CreateShortCut "$DESKTOP\LiberTV.lnk" "$INSTDIR\LiberTV.exe" "" "$INSTDIR\LiberTV.exe" 0       
SectionEnd

Section LTVStart
SectionEnd

;---------------------------------
; Functions

Function .onInit
	!insertmacro MUI_INSTALLOPTIONS_EXTRACT "ltvs.ini"
	IfSilent 0 again
	Sleep 3000 ; short sleep to make sure that previous version has exited.
again:
	FindWindow $0 "LiberTV_Class"
	IntCmp $0 0 not_running
	MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "LiberTV is running, please close it to begin installation" IDRETRY again
	Quit
	
not_running:
	goto start_install
	
	ReadRegStr $0 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV\" "UninstallString"
	StrCmp $0 "" start_install
	MessageBox MB_YESNO|MB_ICONQUESTION "It is recommended that you uninstall the previous version of LiberTV. Uninstall now?" IDNO start_install
	ExecWait "$0" $1	
	MessageBox MB_OKCANCEL|MB_ICONINFORMATION "Press OK to continue installation or Cancel to exit" IDOK retry_again
	Quit
	
retry_again:
	ReadRegStr $0 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV\" "UninstallString"
	StrCmp $0 "" start_install
	MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "The uninstaller is still running or has failed to remove LiberTV. Would you like to retry or cancel the installation?" IDRETRY retry_again
	Quit
	
start_install:
	
FunctionEnd

Function .onInstSuccess
IfSilent 0 is_end
	DeleteRegValue HKCU "Software\LiberTV" "UpdateFile"
	DeleteRegValue HKCU "Software\LiberTV" "UpdateFileSize"
	MessageBox MB_OK "The update has been successfully installed. LiberTV will now start." 
	Exec "$INSTDIR\LiberTV.exe"
is_end:
FunctionEnd

Function LTVStartPage
	!insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE)" "$(TEXT_IO_SUBTITLE)"
	!insertmacro MUI_INSTALLOPTIONS_DISPLAY "ltvs.ini"    
 	!insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "ltvs.ini" "Field 1" "State"  	
	StrCmp $INI_VALUE "1" "" +2

	Exec "$INSTDIR\LiberTV.exe"		
FunctionEnd

;--------------------------------
;Descriptions

;	Language strings
	LangString DESC_SecDummy ${LANG_ENGLISH} "LiberTV Installation"

;	Assign language strings to sections
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
	!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

again:
	FindWindow $0 "LiberTV_Class"
	IntCmp $0 0 not_running
	MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "LiberTV is running, please close it." IDRETRY again
	Quit
not_running:

;	Determine storage path - not needed
	ReadRegStr $0 HKCU "SOFTWARE\LiberTV" "StoragePath"
	DetailPrint "Storage detected at $0"

	MessageBox MB_YESNO|MB_ICONQUESTION "Would you like to delete downloaded videos ?" IDNO leave_storage	
	ExecWait "$INSTDIR\LiberTV.exe /RemoveStorage"
leave_storage:

; 	Remove registry keys
	DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV"
	DeleteRegKey HKCU "Software\LiberTV"
	DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "LiberTV"

;	Remove files and uninstaller

	ExecWait "$INSTDIR\LiberTV.exe /UnregServer"
	!insertmacro UninstallLib TLB NOTSHARED NOREBOOT_PROTECTED "$INSTDIR\LiberTV.tlb" 
	
	Delete "$INSTDIR\LiberTV.exe"
	Delete "$INSTDIR\LiberTV.tlb"
	Delete "$INSTDIR\xvid.ax"	
	Delete "$INSTDIR\xvidltv.dll" 

	Delete "$INSTDIR\ac3filter.ax" 
	Delete "$INSTDIR\x264.ax"
	Delete "$INSTDIR\divxdec.ax"
	Delete "$INSTDIR\license.txt"
	Delete "$INSTDIR\readme.txt"
	Delete "$DESKTOP\LiberTV.lnk"
  
	RmDir  /r "$INSTDIR\data"
	
; 	Remove shortcuts, if any
	Delete "$SMPROGRAMS\LiberTV\*.*"

; 	Remove directories used
	RMDir "$SMPROGRAMS\LiberTV"
	RMDir "$INSTDIR"

	Delete "$INSTDIR\Uninstall.exe"
	DeleteRegKey HKCU "Software\LiberTV"

SectionEnd
