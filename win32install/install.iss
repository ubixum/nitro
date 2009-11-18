; This examples demonstrates how libusb's drivers
; can be installed automatically along with your application using an installer.
;
; Requirements: Inno Setup (http://www.jrsoftware.org/isdl.php)
;
; To use this script, do the following:
; - copy libusb's driver (libusb0.sys, libusb0.dll) to this folder
; - create an .inf and .cab file using libusb's 'inf-wiward.exe'
;   and save the generated files in this folder.
; - in this script replace <your_inf_file.inf> with the name of your .inf file
; - customize other settings (strings)
; - open this script with Inno Setup
; - compile and run

[Setup]
AppName = Ubixum Nitrogen Data Acquisition
AppVerName = Nitro 1.0.0.0
AppPublisher = Ubixum, Inc
AppPublisherURL = http://ubixum.com/
AppVersion = 1.0.0.0
DefaultDirName = {pf}\Ubixum\Nitro
DefaultGroupName = Nitro
Compression = lzma
SolidCompression = yes
; Win2000 or higher
MinVersion = 5,5
PrivilegesRequired = admin

[Files]
; core
; copy the file to the App folder
Source: "..\win32\libusb\install\*"; DestDir: "{app}\usb"
; also copy the DLL to the system folder so that rundll32.exe will find it
Source: "..\win32\libusb\install\*.dll"; DestDir: "{win}\system32"; Flags: replacesameversion restartreplace uninsneveruninstall
Source: "c:\WINDOWS\system32\Python26.dll"; DestDir: "{win}\system32"; Flags: onlyifdoesntexist sharedfile
Source: "..\win32\BinRelease\nitro.exe"; DestDir: "{app}\bin";
Source: "..\README"; Flags: isreadme; DestDir: "{app}"; DestName: "readme.txt";
Source: "..\CHANGELOG"; DestDir: "{app}"; DestName: "changelog.txt";
Source: "..\test\deviceinterface.xsd"; DestDir: "{app}\xml";
; install dll regardless, py bindings require it
Source: "..\win32\DllRelease\nitro.dll"; DestDir: "{win}\system32"; Flags: restartreplace replacesameversion
Source: "..\python\build\lib.win32-2.6\_nitro.pyd"; DestDir: "{app}\nitro_py";
Source: "..\python\build\lib.win32-2.6\nitro\*"; Flags: recursesubdirs; DestDir: "{app}\nitro_py\nitro";
; python site-packaeges
Source: "c:\Python26\Lib\*.py"; DestDir: "{app}\nitro_py";
Source: "c:\Python26\Lib\logging\*"; Flags: recursesubdirs; DestDir: "{app}\nitro_py\logging";
Source: "c:\Python26\Lib\encodings\*"; Flags: recursesubdirs; DestDir: "{app}\nitro_py\encodings";

; dll/bin all need the 2008 redistributables
Source: "vcredist_x86.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

; docs
Source: "..\docs\html\*"; DestDir: "{app}\html"; Components: docs; Flags: recursesubdirs

; headers
Source:  "..\include\*" ; DestDir: "{app}\include"; Components: c; Flags: recursesubdirs

; dll
Source: "readme.libs.txt"; Components: "c"; DestDir: "{app}\lib"; DestName: "readme.txt"

Source: "..\win32\DllRelease\nitro.lib"; Components: "c"; DestDir: "{app}\lib"

; csharp
Source: "..\csharp\Release\CSNitro.dll"; Components: "csharp"; DestDir: "{app}\csharp";

[Registry]
Root: HKLM; SubKey: "Software\Ubixum\Nitro\InstDir"; ValueType: string; ValueData: "{app}"; Flags: uninsdeletekey;
Root: HKLM; SubKey: "SYSTEM\CurrentControlSet\Control\UsbFlags"; ValueName: "IgnoreHWSerNum1fe11212"; ValueType: binary; ValueData: "01";
Root: HKLM; SubKey: "SYSTEM\CurrentControlSet\Control\UsbFlags"; ValueName: "IgnoreHWSerNum1fe12030"; ValueType: binary; ValueData: "01";

[Components]
Name: "driver"; Description: "Nitro USB Driver"; Types: full custom; Flags: fixed
Name: "docs"; Description: "HTML Driver Documentation"; Types: full
Name: "c"; Description: "C++ Bindings"; Types: full
Name: "csharp"; Description: "C# Bindings"; Types: full

[Icons]
Name: "{group}\Uninstall Nitro Drivers"; Filename: "{uninstallexe}"
Name: "{group}\Documentation\Documentation Index"; FileName: "{app}\html\index.html"; Components: docs
Name: "{group}\View Readme"; Filename: "{app}\readme.txt";

[Run]

; touch the HID .inf file to break its digital signature
; this is only required if the device is a mouse or a keyboard !!
;Filename: "rundll32"; Parameters: "libusb0.dll,usb_touch_inf_file_np_rundll {win}\inf\input.inf"

; invoke libusb's DLL to install the .inf file
Filename: "rundll32"; Parameters: "libusb0.dll,usb_install_driver_np_rundll {app}\usb\ubixum.inf"; StatusMsg: "Installing driver (this may take a few seconds) ..."

; bin/dll have to have the 2008 redistributables
Filename: "{tmp}\vcredist_x86.exe"; Parameters: "/q:a"; StatusMsg: "Install MS VC 2008 Runtime Libraries ...";


[Code]
// Delete all the OemXX.inf and OemXX.PNF files associated with previous driver installs.
procedure CleanupOEMFiles();
var
 RegKeys: TArrayOfString;
 I: Integer;
 RegPath: String;
 CurKey: String;
 CurValue: String;
 DeletePath: String;
begin
    RegPath := 'SYSTEM\CurrentControlSet\Control\Class\{EB781AAF-9C70-4523-A5DF-642A87ECA567}';
    if RegGetSubkeyNames ( HKEY_LOCAL_MACHINE, RegPath , RegKeys ) then
    begin
      for I := 0 to GetArrayLength (RegKeys)-1 do
      begin
        // remove OemXX.inf any file associated with Ubixum, Inc. LibUSB devices
        CurKey := RegPath + '\' + RegKeys[I];
        if RegQueryStringValue ( HKEY_LOCAL_MACHINE, CurKey, 'ProviderName', CurValue ) then
        begin
          if CurValue = 'Ubixum, Inc.' then
          begin
            if  RegQueryStringValue ( HKEY_LOCAL_MACHINE, CurKey, 'InfPath', CurValue ) then
            begin
              DeletePath := GetWinDir() + '\inf\' + CurValue;
              DeleteFile ( DeletePath );
              CurValue := ChangeFileExt ( CurValue, '.PNF' );
              DeletePath := GetWinDir() + '\inf\' + CurValue;
              DeleteFile ( DeletePath );
            end // end successful query of InfPath
          end // end CurValue = Ubixum
        end // end successful queried Provider Name
      end // ned processing current key
    end //
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
    // actual install is about to begin
    CleanupOEMFiles();
end;


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  case CurUninstallStep of
    usPostUninstall:
      begin
        // uninstall just finished
        CleanupOEMFiles();
      end;
  end; // end case
end; // end CurUninstallStepChanged



