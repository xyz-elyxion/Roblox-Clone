# Assembles a folder that can be unzipped and run on a clean Windows machine.
#
# The executables load their data from the working directory (shaders/,
# ServerWorld.world, face*.png) and WebApp.exe locates the website by walking up
# from its own directory looking for web/public/index.html. So the package has
# to keep the executables, assets and website folder together. Nothing here is a
# script: Explorer starts a program in its own folder, which is what they expect.
#
# Usage: ./scripts/package-build.ps1 -Configuration Release

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration,

    [string]$BuildDir = '',

    [string]$Output = '',

    # What this build calls itself, and where the copy it installs should look
    # for the next one. Both go into the package: manifest.json describes it,
    # update.json tells RBLXUpdater.exe where to ask. Leave it empty and this
    # build's own date and time become the version; see below.
    [string]$Version = '',

    [string]$UpdateUrl = 'http://127.0.0.1:3000/updates/manifest.json'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# A package with no -Version is stamped with the moment it was built, such as
# 1.0.20261109.143205: two packages never share a number, and each sorts above
# the one before it because the updater compares numeric components. UTC is used
# so the clock cannot go backwards when daylight saving does, which would look
# like a downgrade and be refused. A deliberate -Version 1.1.0 still outranks
# every automatic build, since only the components after the minor carry the
# stamp.
if ([string]::IsNullOrWhiteSpace($Version)) {
    $stamp = (Get-Date).ToUniversalTime().ToString('yyyyMMdd.HHmmss')
    $Version = "1.0.$stamp"
}

if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = "build/windows-$($Configuration.ToLowerInvariant())"
}
if ([string]::IsNullOrWhiteSpace($Output)) {
    $Output = "dist/rblx-windows-$($Configuration.ToLowerInvariant())"
}

$repoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $repoRoot
try {
    # .NET file APIs resolve relative paths against the process directory rather
    # than the PowerShell location, so work from an absolute path.
    if (-not [System.IO.Path]::IsPathRooted($Output)) {
        $Output = Join-Path (Get-Location).Path $Output
    }
    if (Test-Path -LiteralPath $Output) {
        Remove-Item -LiteralPath $Output -Recurse -Force
    }
    New-Item -ItemType Directory -Path $Output -Force | Out-Null

    $exeDir = Join-Path $BuildDir $Configuration
    $found = @()
    $missing = @()
    foreach ($name in @('WebApp', 'Server', 'Client', 'Studio', 'RBLXCode', 'AiTrainer', 'RBLXUpdater', 'RBLXSetup', 'RBLXHosts', 'RBLXProxy', 'RBLXGit', 'RBLXHub')) {
        $exePath = Join-Path $exeDir "$name.exe"
        if (Test-Path -LiteralPath $exePath) {
            Copy-Item -LiteralPath $exePath -Destination $Output
            $found += "$name.exe"
        }
        else {
            $missing += "$name.exe"
            Write-Warning "No $name.exe at $exePath."
        }
    }
    # A link failure can leave some targets built and one missing. Packaging that
    # folder would hand out a release that cannot run the website, so stop here
    # instead of uploading a partial package.
    if ($missing.Count -gt 0) {
        throw "The build is incomplete: missing $($missing -join ', ') in $exeDir."
    }
    if ($found.Count -eq 0) {
        throw "No executables were built in $exeDir; nothing to package."
    }

    # Debug symbols stay next to the binaries that produced them.
    if ($Configuration -eq 'Debug') {
        Get-ChildItem -LiteralPath $exeDir -Filter '*.pdb' -File -ErrorAction SilentlyContinue |
            Copy-Item -Destination $Output
    }

    # Data the executables read from their working directory.
    Copy-Item -LiteralPath 'shaders' -Destination (Join-Path $Output 'shaders') -Recurse
    # Part textures: world files name their images relative to this folder.
    if (Test-Path -LiteralPath 'textures') {
        Copy-Item -LiteralPath 'textures' -Destination (Join-Path $Output 'textures') -Recurse
    }
    Copy-Item -LiteralPath 'ServerWorld.world' -Destination $Output
    Get-ChildItem -LiteralPath '.' -File |
        Where-Object {
            $_.Extension -in @('.png', '.jpg', '.jpeg') -or $_.Name -eq 'face_hex.txt'
        } |
        ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $Output }

    # Website assets. public/ holds the pages and API scripts; seed-worlds/ is
    # read the first time WebApp.exe publishes the starter games.
    $webTarget = Join-Path $Output 'web'
    New-Item -ItemType Directory -Path $webTarget -Force | Out-Null
    Copy-Item -LiteralPath 'web/public' -Destination $webTarget -Recurse
    Copy-Item -LiteralPath 'web/seed-worlds' -Destination $webTarget -Recurse
    # The RBLXHosts control panel. RBLXHosts.exe looks for web/host next to
    # itself, and shows a plain page that says so if the folder is missing.
    Copy-Item -LiteralPath 'web/host' -Destination $webTarget -Recurse
    # The RBLXHub stylesheet, found the same way.
    Copy-Item -LiteralPath 'web/hub' -Destination $webTarget -Recurse

    # Scripted example games: the .world files and the .rbx scripts WebApp.exe
    # publishes as starter games on first start.
    if (Test-Path -LiteralPath 'games') {
        Copy-Item -LiteralPath 'games' -Destination (Join-Path $Output 'games') -Recurse
    }

    $crlf = "`r`n"
    $encoding = [System.Text.Encoding]::ASCII

    # There are no launcher scripts in the package. Every program finds its data
    # in the folder it runs from, so Explorer can start it directly: WebApp.exe
    # locates web/ and the Server.exe and Client.exe beside it, and RBLXCode.exe
    # opens the folder it was started in. Double-clicking a .bat only added a
    # console window and a wrapper around that.

    $readmeLines = @(
        "RBLX Clone - Windows $Configuration build"
        ''
        'WHAT IS IN THIS FOLDER'
        '  WebApp.exe    website backend: pages, accounts, avatars, games, game-server launch'
        '  Server.exe    game server, started by WebApp.exe when a game is played'
        '  Client.exe    game client, started by WebApp.exe when you press Play'
        '  Studio.exe    world editor for levels and their scripts'
        '  RBLXCode.exe  source editor for the files in this folder: project tree, tabs,'
        '                C++/GLSL/RBLX script highlighting, search, save'
        '  AiTrainer.exe headless trainer: signs in with an AI token, then learns and replays'
        '  RBLXUpdater.exe auto updater: checks manifest.json, downloads only the files that'
        '                changed, then starts the game'
        '  RBLXSetup.exe   installer: copies this folder into your account, adds the shortcuts'
        '                and the Add/Remove Programs entry, and it is what that entry runs'
        '  RBLXHosts.exe   server supervisor: keeps a pool of Server.exe processes alive, one'
        '                per port with its own log, restarting any that exit'
        '  RBLXProxy.exe   reverse proxy: puts the website on another address - port 80 on the'
        '                LAN, say - and forwards every request to WebApp.exe'
        '  RBLXGit.exe     git command line: init, add, commit, log, branch and show, over a'
        '                repository that git itself can read (loose objects, refs, index)'
        '  RBLXHub.exe     repository browser: files at HEAD, history, a commit page and'
        '                issues on http://127.0.0.1:3100, rendered from the repository'
        '  web/          website assets (public/ pages and API scripts, seed-worlds/)'
        '  web/host/     the RBLXHosts control panel page'
        '  web/hub/      the RBLXHub browser stylesheet'
        '  games/        example worlds and their .rbx scripts (Sky Platformer)'
        '  shaders/      OpenGL shaders used by Client and Studio'
        '  ServerWorld.world, face*.png, face_hex.txt  data loaded from the working directory'
        ''
        'HOW TO RUN'
        '  WebApp.exe          starts the website on http://localhost:3000; press Open site'
        '                      in its window to open it in the browser'
        '  RBLXProxy.exe       forwards http://127.0.0.1:8080 to WebApp.exe; add'
        '                      --listen 0.0.0.0:80 to serve other machines'
        '  Studio.exe          opens the Studio editor'
        '  RBLXCode.exe        opens the source editor on this folder'
        '  Client.exe          opens the client directly (the website normally does this)'
        '  RBLXUpdater.exe     checks for an update, installs it, then starts the client'
        '  RBLXSetup.exe       installs this folder for the current user (no unzip needed after)'
        '  AiTrainer.exe       headless trainer, run it from this folder with the flags below'
        '  RBLXHosts.exe up    keeps a pool of servers running on their own ports (HOSTING below)'
        '  RBLXGit.exe init .  makes a repository in this folder; add .,'
        '                      commit -m "first" and log are the rest of it'
        '  RBLXHub.exe         browses the repository this folder is inside on port 3100'
        ''
        'FIRST RUN: THE GUIDED TOUR'
        '  A fresh unzip has no account data yet. Start WebApp.exe, press Open site, and'
        '  the tour is at http://localhost:3000/welcome: nine steps through the website,'
        '  the client and game servers, Studio, RBLXCode and the trainer. Steps the site'
        '  can see for itself tick themselves off; the rest you mark as done. It is'
        '  linked as "Tour" in the site navigation, so /welcome works at any time.'
        ''
        'SOURCE EDITOR (RBLXCode.exe)'
        '  RBLXCode.exe                edits this folder: shaders/, web/public/, games/'
        '  RBLXCode.exe D:\Roblox-Clone  edits a source clone, including the C++ files'
        '  Ctrl+P opens a file by name, Ctrl+F searches the folder, Ctrl+G goes to a line,'
        '  Ctrl+S saves. It writes the file back with the line endings it already had.'
        '  This package does not contain the C++ sources; for those, point it at a clone.'
        ''
        'AI TRAINER (AiTrainer.exe)'
        '  The trainer signs in to the website before it runs. Start WebApp.exe, sign in,'
        '  open /developers, create an AI token, and tick "AI compatible" on the game you'
        '  want it to play. A game that is not marked is refused for every token.'
        '  AiTrainer.exe --token rblxai_... --game 4 --generations 60'
        '      trains the course the game names: models\doors-hotel.policy for a doors game,'
        '      or models\<world>.policy for a walk-to-the-goal game'
        '  AiTrainer.exe --token rblxai_... --game 4 --replay --watch'
        '      replays the saved policy; --watch draws the run as a top-down map in the terminal'
        '  AiTrainer.exe --token rblxai_... --game 4 --gui --agents 4'
        '      trains in a window: the course from above with the roof off, one dot per agent'
        '  AiTrainer.exe --token rblxai_... --game 4 --play --port 7777'
        '      plays the saved policy on a running Server.exe instead of training here'
        '  --site http://127.0.0.1:3000 points it at a WebApp.exe elsewhere.'
        '  Run it from this folder so web\ and models\ resolve. CPU only, no GPU; --gui needs a'
        '  desktop session, and every other run works headless.'
        ''
        'HOSTING YOUR OWN SERVERS (RBLXHosts.exe)'
        '  RBLXHosts.exe init --config hosting.conf  writes a commented default config file'
        '  RBLXHosts.exe up --config hosting.conf    keeps the servers alive until Ctrl+C'
        '  With no config it runs two servers on 7777 and 7778 from this folder, each with'
        '  its log in logs\host-<port>.log. A server that exits is started again after a'
        '  growing delay; one that keeps dying is marked failed and shown as such instead'
        '  of looping. Ctrl+C or closing the window stops the servers it started.'
        '  The control panel prints a URL such as'
        '  http://127.0.0.1:7900/?token=... at startup: open it to see every slot, read a'
        '  log, and start, stop or restart one server or the whole pool. It listens on'
        '  loopback only and that link is the way in; --dashboard-port 0 turns it off.'
        '  HOSTING.md in the source repository documents every key the config accepts.'
        ''
        'NOTES'
        '  - Keep this folder layout intact. The executables look for shaders/ and web/'
        '    relative to the folder they run from, which is where Explorer starts them.'
        '  - WebApp.exe uses the Server.exe and Client.exe beside it, so Play launches'
        '    the copies that ship with this package.'
        '  - WebApp.exe opens a server console window: the live log with level filters and'
        '    search, every running game server with its port, players and output, plus Stop,'
        '    Open site, Copy log and Clear. Closing it stops the website and its servers.'
        '    Start it with --nogui (or set RBLX_NO_GUI=1) to keep the plain terminal log.'
        '  - Website data lives in web/users.json and web/game-worlds/. Game server output'
        '    is written to web/game-server-logs/game-server-<port>.log, which is where a'
        '    failed Play reports the reason. Delete these to start over; they are'
        '    recreated on the next launch.'
        '  - The C++ runtime is linked statically, so Visual Studio and the VC++'
        '    redistributable are not required. Visual Studio is only needed to build'
        '    from source.'
        '  - WebApp.exe accepts connections on every interface and a game server binds'
        '    every interface too, so anything on your network can reach them if the'
        '    firewall allows it. This is a local setup, not a public deployment: keep the'
        '    Windows firewall on, and read SECURITY.md in the repository before exposing'
        '    it to anything else.'
        ''
        'ENVIRONMENT VARIABLES (all optional)'
        '  PORT                        website port, default 3000'
        '  NODE_ENV                    set to "production" to require a real JWT_SECRET'
        '  JWT_SECRET                  token signing secret, 32+ characters in production'
        '  WEB_DIR                     folder containing public/index.html'
        '  DATABASE_PATH               account database, default web/users.json'
        '  GAME_WORLDS_DIR             published world files, default web/game-worlds'
        '  PUBLIC_BASE_URL             origin game servers report back to'
        '  SERVER_EXECUTABLE_PATH      Server.exe to launch for a game'
        '  CLIENT_EXECUTABLE_PATH      Client.exe to launch when you press Play'
        '  GAME_SERVER_HOST            interface game servers bind to, default 127.0.0.1'
        '  GAME_SERVER_BASE_PORT       first port tried for a game server, default 7777'
        '  GAME_INSTANCE_EMPTY_GRACE_MS  delay before an empty server is shut down'
        '  GAME_INSTANCE_LAUNCH_GRACE_MS delay before a server nobody has joined yet is shut down'
    ) -join $crlf
    [System.IO.File]::WriteAllText((Join-Path $Output 'README.txt'), $readmeLines + $crlf, $encoding)

    # The updater ships beside the game: it is what the installed Play shortcut
    # runs, and what a double-click of it runs in this folder. update.json is the
    # machine's own setting, so it is written here and never comes from a release.
    $updateConfig = @(
        '{'
        '  "manifestUrl": "' + $UpdateUrl + '",'
        '  "launch": "Client.exe"'
        '}'
    ) -join $crlf
    [System.IO.File]::WriteAllText((Join-Path $Output 'update.json'), $updateConfig + $crlf, $encoding)

    # The release description, generated last so every file above is in it. The
    # updater and its settings are left out on purpose: it cannot replace the copy
    # that is running it, and the URL belongs to whoever installed this folder.
    & (Join-Path $Output 'RBLXUpdater.exe') manifest --dir $Output --version $Version
    if ($LASTEXITCODE -ne 0) {
        throw "Could not write manifest.json for $Output (RBLXUpdater exit $LASTEXITCODE)."
    }

    $files = Get-ChildItem -LiteralPath $Output -Recurse -File
    $totalBytes = ($files | Measure-Object -Property Length -Sum).Sum
    Write-Host ("Packaged {0} version {1} with {2} files into {3} ({4:N1} MB)" -f `
        ($found -join ', '), $Version, $files.Count, $Output, ($totalBytes / 1MB))
}
finally {
    Pop-Location
}
