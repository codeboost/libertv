#include "common.iss"

[Setup]
LicenseFile=license.en.txt
OutputDir=..\setup
OutputBaseFilename=setup_en

[Files]
Source: "vlc\*"; DestDir: "{app}\vlc"; Flags: ignoreversion recursesubdirs createallsubdirs

[Registry]
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "ChannelGuideURL"; ValueData: "http://channelguide.libertv.tv/guide/"
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "SiteURL"; ValueData: "http://www.libertv.tv/"
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "CheckConnectURL"; ValueData: "http://www.libertv.tv/connectable.php"
Root: HKCU; Subkey: "Software\LiberTV\PlayerSettings"; ValueType: string; ValueName: "DefSubLang"; ValueData: "EN"

