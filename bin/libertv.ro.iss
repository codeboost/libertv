#include "common.iss"

[Setup]
LicenseFile=license.txt
OutputDir=..\setup
OutputBaseFilename=setup_ro

[Files]
Source: "vlc\*"; DestDir: "{app}\vlc"; Flags: ignoreversion recursesubdirs createallsubdirs

[Registry]
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "ChannelGuideURL"; ValueData: "http://channelguide.libertv.ro/guide/"
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "SiteURL"; ValueData: "http://www.libertv.ro/"
Root: HKCU; Subkey: "Software\LiberTV"; ValueType: string; ValueName: "CheckConnectURL"; ValueData: "http://storage1.libertv.ro/connectable.php"
Root: HKCU; Subkey: "Software\LiberTV\PlayerSettings"; ValueType: string; ValueName: "DefSubLang"; ValueData: "RO"

