# Assembles a folder that can be unzipped and run on a clean Windows machine.
#
# The executables load their data from the working directory (shaders/,
# ServerWorld.world, face*.png) and WebApp.exe locates the website by walking up
# from its own directory looking for web/public/index.html. So the package has
# to keep the executables, assets and website folder together, and the launch
# scripts have to change into the package root before starting anything.
#
# Usage: ./scripts/package-build.ps1 -Configuration Release

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration,

    [string]$BuildDir = '',

    [string]$Output = ''
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

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
    foreach ($name in @('WebApp', 'Server', 'Client', 'Studio', 'RBLXCode', 'AiTrainer')) {
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

    # Scripted example games: the .world files and the .rbx scripts WebApp.exe
    # publishes as starter games on first start.
    if (Test-Path -LiteralPath 'games') {
        Copy-Item -LiteralPath 'games' -Destination (Join-Path $Output 'games') -Recurse
    }

    $crlf = "`r`n"
    $encoding = [System.Text.Encoding]::ASCII

    $websiteBat = @(
        '@echo off'
        'rem Starts the RBLX website backend and opens it in the default browser.'
        'setlocal'
        'cd /d "%~dp0"'
        'if not exist "WebApp.exe" ('
        '    echo WebApp.exe is not in this package.'
        '    pause'
        '    exit /b 1'
        ')'
        'rem Point the website at the copies shipping in this folder.'
        'set "SERVER_EXECUTABLE_PATH=%~dp0Server.exe"'
        'set "CLIENT_EXECUTABLE_PATH=%~dp0Client.exe"'
        'if not defined PORT set "PORT=3000"'
        'echo Starting the RBLX website on http://localhost:%PORT%'
        'start "RBLX Website" "%~dp0WebApp.exe"'
        'timeout /t 2 /nobreak >nul'
        'start "" "http://localhost:%PORT%"'
    ) -join $crlf
    [System.IO.File]::WriteAllText((Join-Path $Output 'start-website.bat'), $websiteBat + $crlf, $encoding)

    $studioBat = @(
        '@echo off'
        'rem Opens the Studio world editor.'
        'setlocal'
        'cd /d "%~dp0"'
        'if not exist "Studio.exe" ('
        '    echo Studio.exe is not in this package.'
        '    pause'
        '    exit /b 1'
        ')'
        'Studio.exe'
        'if errorlevel 1 pause'
    ) -join $crlf
    [System.IO.File]::WriteAllText((Join-Path $Output 'start-studio.bat'), $studioBat + $crlf, $encoding)

    $clientBat = @(
        '@echo off'
        'rem Opens the game client on its own. Normally the website starts it for you'
        'rem when you press Play on a game page.'
        'setlocal'
        'cd /d "%~dp0"'
        'if not exist "Client.exe" ('
        '    echo Client.exe is not in this package.'
        '    pause'
        '    exit /b 1'
        ')'
        'Client.exe'
        'if errorlevel 1 pause'
    ) -join $crlf
    [System.IO.File]::WriteAllText((Join-Path $Output 'start-client.bat'), $clientBat + $crlf, $encoding)

    # The source editor opens the folder it is started in, so the launcher runs it
    # from this folder: shaders/, web/public and games/ are all editable here, and
    # pointing it at a source checkout is a command-line argument away.
    $codeBat = @(
        '@echo off'
        'rem Opens the RBLXCode source editor on this folder.'
        'setlocal'
        'cd /d "%~dp0"'
        'if not exist "RBLXCode.exe" ('
        '    echo RBLXCode.exe is not in this package.'
        '    pause'
        '    exit /b 1'
        ')'
        'rem A folder given on the command line wins over this one.'
        'if "%~1"=="" ('
        '    RBLXCode.exe "%~dp0"'
        ') else ('
        '    RBLXCode.exe "%~1"'
        ')'
        'if errorlevel 1 pause'
    ) -join $crlf
    [System.IO.File]::WriteAllText((Join-Path $Output 'start-code.bat'), $codeBat + $crlf, $encoding)

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
        '  AiTrainer.exe headless trainer: learns and replays a policy for a world, no window'
        '  web/          website assets (public/ pages and API scripts, seed-worlds/)'
        '  games/        example worlds and their .rbx scripts (Sky Platformer)'
        '  shaders/      OpenGL shaders used by Client and Studio'
        '  ServerWorld.world, face*.png, face_hex.txt  data loaded from the working directory'
        ''
        'HOW TO RUN'
        '  start-website.bat   starts WebApp.exe, then opens http://localhost:3000'
        '  start-studio.bat    opens the Studio editor'
        '  start-code.bat      opens the RBLXCode source editor on this folder'
        '  start-client.bat    opens the client directly (the website normally does this)'
        '  AiTrainer.exe       headless trainer, run it from this folder with the flags below'
        ''
        'SOURCE EDITOR (RBLXCode.exe)'
        '  RBLXCode.exe                edits this folder: shaders/, web/public/, games/'
        '  RBLXCode.exe D:\Roblox-Clone  edits a source clone, including the C++ files'
        '  Ctrl+P opens a file by name, Ctrl+F searches the folder, Ctrl+G goes to a line,'
        '  Ctrl+S saves. It writes the file back with the line endings it already had.'
        '  This package does not contain the C++ sources; for those, point it at a clone.'
        ''
        'AI TRAINER (AiTrainer.exe)'
        '  AiTrainer.exe --world games\turbo-circuit.world --goal FinishLine --generations 60'
        '      trains a policy and writes models\<world>.policy'
        '  AiTrainer.exe --world games\turbo-circuit.world --goal FinishLine --replay --watch'
        '      replays the saved policy; --watch draws the run as a top-down map in the terminal'
        '  Run it from this folder so games\ and models\ resolve. CPU only, no GPU, no window.'
        ''
        'NOTES'
        '  - Keep this folder layout intact. The executables look for shaders/ and web/'
        '    relative to the folder they run from, and every .bat changes into it first.'
        '  - start-website.bat tells WebApp.exe where Server.exe and Client.exe are, so'
        '    Play launches the copies that ship with this package.'
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
        '  - The game server listens on 127.0.0.1 only. This is a local setup, not a'
        '    public deployment.'
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

    $files = Get-ChildItem -LiteralPath $Output -Recurse -File
    $totalBytes = ($files | Measure-Object -Property Length -Sum).Sum
    Write-Host ("Packaged {0} with {1} files into {2} ({3:N1} MB)" -f `
        ($found -join ', '), $files.Count, $Output, ($totalBytes / 1MB))
}
finally {
    Pop-Location
}
