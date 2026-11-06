; Installer for the player-facing Windows build.
;
; Build it after the package exists:
;   scripts\package-build.ps1 -Configuration Release -Version 1.2.0 -UpdateUrl http://host/rblx
;   iscc deploy\RBLXClone.iss /DAppVersion=1.2.0
;
; Inno Setup 6 is the only tool needed; it is not vendored because it is only
; used to produce this artifact, never to build the game. The payload is the
; folder package-build.ps1 wrote, so the installer ships exactly the files the
; updater will later compare against its manifest.
;
; The shortcut players use points at RBLXUpdater.exe rather than Client.exe:
; the updater checks for a newer release, applies it, and then starts the game,
; which is how a copy installed here keeps itself current without anybody
; opening a separate updater.

#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif

#ifndef SourceDir
  #define SourceDir "..\dist\rblx-windows-release"
#endif

[Setup]
; A stable AppId is what makes a second setup upgrade the first one instead of
; installing beside it.
AppId={{8D6C1B57-2F1A-4C64-9B4E-0C2E7A1F53D1}
AppName=RBLX Clone
AppVersion={#AppVersion}
AppPublisher=RBLX Clone
AppPublisherURL=https://github.com/xyz-elyxion/Roblox-Clone
DefaultDirName={localappdata}\RBLXClone
DefaultGroupName=RBLX Clone
DisableProgramGroupPage=yes
AllowNoIcons=yes
OutputDir=..\dist
OutputBaseFilename=rblx-clone-{#AppVersion}-setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
; The game is 64-bit only: it links the 64-bit physics library and the
; anti-cheat is compiled into the client.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\Client.exe
; Accounts, worlds and logs live under the install folder, so the uninstaller
; must not quietly delete them; it asks below instead.
PrivilegesRequired=lowest

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Shortcuts:"

[Files]
; Everything the package script produced, layout intact: the executables load
; shaders/ and web/ relative to the folder they run from.
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\Play RBLX Clone"; Filename: "{app}\RBLXUpdater.exe"; Parameters: "--launch"; WorkingDir: "{app}"; Comment: "Checks for updates, then starts the game"
Name: "{group}\RBLX Clone website"; Filename: "{app}\start-website.bat"; WorkingDir: "{app}"
Name: "{group}\Studio world editor"; Filename: "{app}\start-studio.bat"; WorkingDir: "{app}"
Name: "{group}\RBLXCode source editor"; Filename: "{app}\start-code.bat"; WorkingDir: "{app}"
Name: "{group}\Uninstall RBLX Clone"; Filename: "{uninstallexe}"
Name: "{autodesktop}\RBLX Clone"; Filename: "{app}\RBLXUpdater.exe"; Parameters: "--launch"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\RBLXUpdater.exe"; Parameters: "--launch"; Description: "Start RBLX Clone"; Flags: nowait postinstall skipifsilent
Filename: "{app}\start-website.bat"; Description: "Open the RBLX website"; Flags: nowait postinstall skipifsilent unchecked

[UninstallDelete]
; The updater's staging folder never survives a successful run, but a failed one
; can leave it behind.
Type: filesandordirs; Name: "{app}\.update-staging"

[Code]
// Accounts and worlds are in the install folder (web/users.json, web/game-worlds,
// games/). Deleting a player's worlds because they uninstalled the game would be
// a worse surprise than leaving a folder behind, so ask, and default to keeping.
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    if MsgBox('Also delete your worlds, account data and game logs in ' + ExpandConstant('{app}') + '?',
              mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then
      DelTree(ExpandConstant('{app}'), True, True, True);
  end;
end;
