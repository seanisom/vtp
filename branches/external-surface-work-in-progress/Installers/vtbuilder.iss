; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=VTBuilder
AppVerName=2009.08.26
AppPublisher=Virtual Terrain Project
AppPublisherURL=http://vterrain.org/
AppSupportURL=http://vterrain.org/
AppUpdatesURL=http://vterrain.org/
DefaultDirName={pf}\VTP
DefaultGroupName=VTP
AllowNoIcons=yes
LicenseFile=C:\VTP\license.txt
OutputBaseFilename=VTBuilder_090826
OutputDir=C:\Distrib
; We need the following because some Windows machines won't turn Registry settings into Environment variables w/o a reboot
AlwaysRestart=yes

[Types]
Name: "standard"; Description: "Standard installation"; Flags: iscustom

[Components]
Name: "main"; Description: "The VTBuilder application"; Types: standard
Name: "data"; Description: "Data used by the application"; Types: standard
Name: "docs"; Description: "Documentation for the application"; Types: standard
Name: "proj"; Description: "Data files for coordinate systems (GDAL/PROJ.4)"; Types: standard
Name: "dlls"; Description: "Third-party DLL files (wxWindows, etc.)"; Types: standard

[Tasks]
Name: env; Description: "Set environment variables for coordinate system data files"; GroupDescription: "Environment variables:"; Components: proj

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "GDAL_DATA"; ValueData: "{app}\GDAL-data"; Components: proj
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "PROJ_LIB"; ValueData: "{app}\PROJ4-data"; Components: proj

[Files]
Source: "C:\VTP\license.txt"; DestDir: "{app}"; Flags: ignoreversion; Components: main

Source: "C:\VTP\TerrainApps\VTBuilder\Release-Unicode-vc9\VTBuilder.exe"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\VTBuilder.xml"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\Docs\*"; DestDir: "{app}\Docs"; Flags: ignoreversion recursesubdirs; Components: docs

; Translation files
Source: "C:\VTP\TerrainApps\VTBuilder\ar\VTBuilder.mo"; DestDir: "{app}\Apps\ar"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\de\VTBuilder.mo"; DestDir: "{app}\Apps\de"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\fr\VTBuilder.mo"; DestDir: "{app}\Apps\fr"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\ro\VTBuilder.mo"; DestDir: "{app}\Apps\ro"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\zh\VTBuilder.mo"; DestDir: "{app}\Apps\zh"; Flags: ignoreversion; Components: main

; Core Data
Source: "G:\Data-Distro\WorldMap\gnv19.*"; DestDir: "{app}/Data/WorldMap"; Flags: ignoreversion; Components: data
Source: "G:\Data-Distro\Culture\materials.xml"; DestDir: "{app}/Data/Culture"; Flags: ignoreversion; Components: data

; Projection Stuff
Source: "C:\APIs\gdal-1.6.0\data\*"; DestDir: "{app}\GDAL-data"; Flags: ignoreversion; Components: proj
Source: "C:\APIs\proj-4.5.0\nad\*"; DestDir: "{app}\PROJ4-data"; Flags: ignoreversion; Components: proj

; DLLs
Source: "C:\APIs\bzip2-1.0.3-bin-vc9\bzip2.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\gdal160-vc9\bin\gdal16-vc9.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\gdal160-vc9\bin\proj.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\gdal160-vc9\bin\*.exe"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\libcurl-7.15.0\libcurl.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\libpng-1.2.32\libpng13-vc9.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\libpng-1.2.32\zlib1-vc9.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\netcdf-3.5.0.win32bin\bin\*.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\wx2.8.7-bin-vc9\lib\vc_dll\wxbase28u_vc_custom.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\wx2.8.7-bin-vc9\lib\vc_dll\wxmsw28u_core_vc_custom.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\wx2.8.7-bin-vc9\lib\vc_dll\wxmsw28u_gl_vc_custom.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\wx2.8.7-bin-vc9\lib\vc_dll\wxbase28u_net_vc_custom.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\wx2.8.7-bin-vc9\lib\vc_dll\wxmsw28u_aui_vc_custom.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\Program Files (x86)\Expat 2.0.1\Bin\libexpat.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls

; Microsoft DLLs (not useful, since vc8 they don't work if installed this way)
;Source: "C:\Program Files\VisStudio8\VC\REDIST\x86\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest"; DestDir: "{app}"; Flags: ignoreversion; Components: dlls
;Source: "C:\Program Files\VisStudio8\VC\REDIST\x86\Microsoft.VC80.CRT\msvcp80.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: dlls
;Source: "C:\Program Files\VisStudio8\VC\REDIST\x86\Microsoft.VC80.CRT\msvcr80.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: dlls

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Documentation"; Filename: "{app}\Docs\en\index.html"
Name: "{group}\VTBuilder"; Filename: "{app}\Apps\VTBuilder.exe"; WorkingDir: "{app}/Apps"
Name: "{group}\Uninstall VTBuilder"; Filename: "{uninstallexe}"
