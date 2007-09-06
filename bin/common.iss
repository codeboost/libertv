; Designed for with Inno Setup Compiler 5.1.11
[Setup]
AppName=LiberTV
AppPublisher=LiberTV Media
AppPublisherURL=http://www.libertv.ro/
AppSupportURL=http://www.libertv.ro/
AppUpdatesURL=http://www.libertv.ro/
DefaultDirName={reg:HKCU\Software\LiberTV,|{pf}\LiberTV}
DefaultGroupName=LiberTV
AllowNoIcons=yes
SetupIconFile=LiberTVIcon.ico
Compression=lzma
SolidCompression=yes
;AppMutex=LiberTV_Mutex
AppVerName=LiberTV
;####################### DO NOT FORGET TO CHANGE VERSION IN THE REGISTRY SECTION ##############################
                                        VersionInfoVersion=1.4.0.0
;;####################### DO NOT FORGET TO CHANGE VERSION IN THE REGISTRY SECTION ##############################
WizardImageFile = SetupImage_Large.bmp
WizardSmallImageFile = SetupImage_Small.bmp
DirExistsWarning = no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
Source: "LiberTV.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "xvidltv.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "ac3filter.ax"; DestDir: "{app}"; Flags: ignoreversion
Source: "libertv.tlb"; DestDir: "{app}"; Flags: ignoreversion regtypelib
Source: "LTVUtil.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "msvcirt.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "x264.ax"; DestDir: "{app}"; Flags: ignoreversion
Source: "xvid.ax"; DestDir: "{app}"; Flags: ignoreversion
Source: "divxdec.ax"; DestDir: "{app}"; Flags: ignoreversion
Source: "libvlc.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "rss.ini"; DestDir: "{app}"; Flags: ignoreversion

Source: "data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "skins\*"; DestDir: "{app}\skins"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
; VLC patch - in axvlcl.dll - Replace Software\VideoLAN\VLC with Software\LiberTV\VLCP (widestrings)
Root: HKLM; SubKey: "Software\LiberTV\VLCP"; ValueType: string; ValueName: "InstallDir"; ValueData: "{app}"
Root: HKLM; SubKey: "Software\LiberTV\VLCP"; ValueType: string; ValueName: "Version"; ValueData: "0.8.6c"
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "Version"; ValueData: "1.4.0.0"
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "Skin"; ValueData: "piele"
Root: HKCU; Subkey: "Software\LiberTV"; Flags: uninsdeletekey
Root: HKCU; SubKey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "LiberTV"; Flags: uninsdeletevalue;
;DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV"
Root: HKCU; SubKey: "Software\Microsoft\Windows\CurrentVersion\Uninstall\LiberTV"; Flags: deletekey;

[Run]
Filename: "{app}\LiberTV.exe"; Parameters: "/RegServer"
;LTVCMDLINE is used during the automatic updates; it may be /LTVCMDLINE="/silent"; in this case, we pass /silent to LiberTV
Filename: "{app}\LiberTV.exe"; Description: "{cm:LaunchProgram,LiberTV}"; Parameters: "{param:LTVCMDLINE}"; Flags: nowait postinstall

[UninstallDelete]
;Make sure we remove the files from older versions in the /data directory.
Type: filesandordirs; Name: "{app}\data"

[Icons]
Name: "{group}\LiberTV"; Filename: "{app}\LiberTV.exe"
Name: "{group}\{cm:ProgramOnTheWeb,LiberTV}"; Filename: "http://www.libertv.ro/"
Name: "{group}\{cm:UninstallProgram,LiberTV}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\LiberTV"; Filename: "{app}\LiberTV.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\LiberTV"; Filename: "{app}\LiberTV.exe"; Tasks: quicklaunchicon


[UninstallRun]
Filename: "{app}\LiberTV.EXE"; Parameters: "/UnregServer"

[Code]

function InitializeSetup(): Boolean;
begin
    Result := True;
    while (True) do
    begin
        if (CheckForMutexes('LiberTV_Mutex')) then
        begin
            Sleep(2000);
            if (CheckForMutexes('LiberTV_Mutex')) then
            begin
                if MsgBox('LiberTV is running. Please close it to continue maintenance.', mbInformation, MB_OKCANCEL) = IDCANCEL then
                begin
                    Result := False;
                    break;
                end
            end else break; //!CheckForMutexes
        end else break; //!CheckForMutexes
    end //while
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var ErrorCode: Integer;
begin
    if curUninstallStep = usUninstall then begin
      if MsgBox('Would you like to remove downloaded videos ?', mbInformation, MB_YESNO) = IDYES then
      begin
          if not ShellExec('open', ExpandConstant('{app}\LiberTV.exe'), '/RemoveStorage', '', SW_SHOW, ewWaitUntilTerminated, ErrorCode) then
          begin
            MsgBox('Error executing LiberTV.exe. Storage not removed.', mbError, MB_OK);
          end;
      end;
    end;
end;


