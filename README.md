# RBLX Clone

A Roblox-style platform built from scratch in C++17: a player client, an authoritative game
server with its own scripting language, a Studio world editor, a source editor, and a website
that owns accounts, avatars, games and game-server launch.

Everything runs on one machine and needs no runtime to install. It is a prototype meant for
local experimentation, not a production service — see [Known limitations](#known-limitations)
before pointing it at the internet.

| Executable | What it is |
| --- | --- |
| `Client.exe` | The game: player list, chat, shift lock, health and respawn, driving, the bundled games |
| `Server.exe` | The authoritative game server: world, physics, scripts, players, movement guard |
| `Studio.exe` | World editor: build, colour, texture, script and publish |
| `RBLXCode.exe` | Source editor for this repository, the way VS Code is the editor for its own sources |
| `WebApp.exe` | The website: accounts, sessions, avatars, friends, games, coins, moderation, launching servers |
| `AiTrainer.exe` | Signs in with a developer token, learns to play an AI-compatible game, and can draw the run in a window while it trains |
| `RBLXHosts.exe` | Keeps N game servers running on one machine — ports, logs, restarts, control panel — see [HOSTING.md](HOSTING.md) |

```powershell
build\rblxbuild-tool\bin\rblxbuild --preset windows-release
build\rblxbuild-tool\bin\rblxbuild --build --preset windows-release
build\windows-release\Release\WebApp.exe        # then open http://localhost:3000
```

Sign in as `defaultuser0` with the password `local-starter-account`, pick a game, press **Play**.
The website starts a game server and launches the client for you.

## Contents

- [Quick start](#quick-start) · [What's in the box](#whats-in-the-box) · [Requirements](#requirements)
- [Building](#building) · [Running](#running) · [How it fits together](#how-it-fits-together)
- [Anti-cheat and moderation](#anti-cheat-and-moderation) — the ladder, machine bans, appeals
- [The website](#the-website) — configuration, accounts, captchas, coins, admin, themes
- [Web API](#web-api) — every route, grouped
- [Scripting](#scripting) — RBLX Script and the world API
- [Controls](#controls) — client, Studio, RBLXCode
- [AI Trainer](#ai-trainer) · [Repository layout](#repository-layout)
- [RBLXBuild and vendored dependencies](#rblxbuild-and-vendored-dependencies)
- [CI and prebuilt packages](#ci-and-prebuilt-packages)
- [Known limitations](#known-limitations) · [Troubleshooting](#troubleshooting) · [Credits](#credits)
- [SECURITY.md](SECURITY.md) — the security model, and how to report a problem

## Quick start

### The unzip-and-run package

The `Build` workflow packages every push into `rblx-windows-release` (see
[CI and prebuilt packages](#ci-and-prebuilt-packages)). Unzip it and double-click `WebApp.exe`: it
serves the site on <http://localhost:3000>, uses the `Server.exe` and `Client.exe` beside it, and
**Open site** in its window opens the page in the browser.

The guided tour is at <http://localhost:3000/welcome>: nine steps through the website, the client and
game servers, Studio, RBLXCode and the trainer, each one checking itself against the running site
where it can. **Tour** in the site navigation reopens it. `Studio.exe` opens the world editor,
`RBLXCode.exe` the source editor, `Client.exe` the game on its own, and `RBLXUpdater.exe` checks for
an update before starting it. `RBLXHosts.exe` is in the same folder: `RBLXHosts.exe up` keeps a pool
of `Server.exe` processes alive on their own ports and prints a loopback address for its control
panel, where you can watch the pool and start, stop or restart a server ([HOSTING.md](HOSTING.md)).

There are no `.bat` files anywhere, in the package or in an install: every program looks for its data
in the folder it runs from, which is where Explorer starts it, so a console wrapper added nothing.

Nothing else needs installing. The executables link the C++ runtime statically, so neither Visual
Studio nor the VC++ redistributable is required at runtime. Keep the folder layout intact: the
executables load `shaders/`, `ServerWorld.world` and `web/` relative to the working directory.

### The installer, and how a copy updates itself

`RBLXSetup.exe` is the installer, and it is this project's own code: no installer tool to download,
nothing vendored that never ships. It is built with everything else and travels inside the package,
where a player runs it from the folder they unpacked:

```powershell
RBLXSetup.exe                                  # install into %LOCALAPPDATA%\RBLXClone
RBLXSetup.exe --dir D:\Games\RBLXClone --launch
RBLXSetup.exe --uninstall                      # keeps worlds and account data
RBLXSetup.exe --uninstall --purge              # removes those too
```

It copies the folder with its layout intact and leaves anything already identical alone, so running
it over an existing install is an upgrade rather than a clobber. It records what it installed as
`manifest.json` — the same document the updater diffs against — which means an installed copy can
update and uninstall itself file by file. Then it adds **Play RBLX Clone** to the Start menu and the
desktop, lists the program in Settings → Apps → Installed apps and in Control Panel → Programs and
Features with its own `UninstallString`, and copies itself into the install so that entry works.
Shortcuts are COM's `IShellLink` and the entry is the per-user
`HKCU\Software\Microsoft\Windows\CurrentVersion\Uninstall\RBLXClone`, carrying the version,
publisher, install date, size and icon those two pages display. Uninstalling removes exactly the
recorded files and leaves worlds, saves and account data unless `--purge` is given.

The shortcut points at `RBLXUpdater.exe` rather than `Client.exe`, so an update is checked every
time somebody plays and nobody has to run a second program. The updater asks the URL in `update.json`
for `manifest.json`, compares it with the manifest it saved in the install folder, and downloads only
the files whose SHA-256 differs. Each download is checked against its hash before it is written, and
each replacement goes through a rename with the old file moved aside first, so an interrupted update
leaves the previous file in place; the manifest is written last, which is what marks the update
complete. A machine that is offline, whose release server has published nothing yet (the usual state
of a fresh copy, and of every build that has never been put on a server), or whose build is newer
than the published one, still starts the game; a 404 for `manifest.json` is reported as "nothing is
published yet" rather than as a failure.

Every window this project opens — the installer and updater, the server console, Studio, RBLXCode and
the client's menus — takes its palette and spacing from `include/UiTheme.h`, so they read as one
program rather than five. There are two palettes, a dark one and a light one built on the same mint
accent, and a single settings file chooses between them for every window:

```ini
# %APPDATA%\RBLXClone\ui.settings
theme = light
```

Nothing is written until somebody makes that file, and without it every window draws the dark
palette. `RBLX_THEME=light` or `RBLX_THEME=dark` overrides the file for one launch, and
`RBLX_UI_SETTINGS` points at a different file, which is how the tests below run without touching the
user's settings. Lines starting with `#` or `;` are comments, both the key and the value are
case-insensitive, and an unknown key or value is ignored rather than treated as an error, so a
settings file written for a later version still loads. RBLXCode's own editor palette starts out
matching the window and stays switchable from its View menu.

The release side is one command on the machine that produced the package:

```powershell
RBLXUpdater.exe manifest --dir dist\rblx-windows-release --version 1.2.0
```

`RBLXUpdater.exe install --manifest <url> --dir <folder>` fills a folder from scratch, and `check`
(exit 10 when an update exists, 0 when current) is there for a scheduled task or a monitor. The rules the updater applies are covered by
`UpdaterTests`: version order, manifest parsing, paths that try to leave the install folder, which
files a plan touches, and the rename that replaces a file.

### From source

```bash
mkdir -p build/rblxbuild-tool && cd build/rblxbuild-tool
../../rblxbuild/bootstrap --parallel 4       # once; builds the vendored build tool
cd ../..
build/rblxbuild-tool/bin/rblxbuild --preset windows-release
build/rblxbuild-tool/bin/rblxbuild --build --preset windows-release
build/rblxbuild-tool/bin/rblxtest --preset windows-release
```

The MinGW and manual configure forms are under [Building](#building).

## What's in the box

**The client** is a third-person game: WASD and mouse, shift lock, a player list, chat, a debug
overlay, health and a death overlay, and sound. Health and respawning are decided by the game server, not the
client: the server keeps each player's health, kills anyone who falls below the world or takes
damage, and returns them to the world's `SpawnLocation` after a three-second countdown every
client sees. The client draws the bar and stops taking movement input while dead; **Reset
Character** asks the server rather than respawning locally. Scripts damage players with
`player:TakeDamage(amount)`.

**The game server** owns the world and everything that matters in it — positions it accepts,
collisions, scripts, damage, spawns — and is the only side whose word counts. It loads a `.world`
file plus an optional `.rbx` script, runs the script's `Heartbeat`, streams part changes to
clients, and reports detections to the website.

**Studio** uses GLFW/OpenGL/ImGui/ImGuizmo and has a basic world editing path: a toolbox of parts,
move/rotate/scale tools with snapping, a properties panel (surface material, optional tiled
texture), a script panel with a VS Code-style editor, and publish. Scale operations assign the
decomposed absolute scale from ImGuizmo and clamp it to a positive range; they do not multiply
scale every frame.

**Three games ship with the build**, each a `.world` and a `.rbx`:

- **Sky Platformer** (`games/sky-platformer.world` and `games/sky-platformer.rbx`) — the scripting introduction:
  moving platforms, lava that teleports you back to the spawn.
- **Turbo Circuit** (`games/turbo-circuit.world` and `games/turbo-circuit.rbx`) — a driving game. A world part named
  `Car` is the vehicle: the server integrates it from client throttle and steer input
  (`include/Vehicle.h`) and broadcasts its position, so a modified client cannot teleport the car
  any more than a character. Walk up to it and press `E` to get in or out; the driver's character
  rides in the car, the camera follows behind and the HUD shows km/h. A `.rbx` script counts laps at
  the finish line. The car stops at the world's collidable parts and slides along a wall it only
  clips, so a lap cannot be finished by driving through the edge of the track; the finish line and
  the checkpoints are non-colliding triggers, because a car that bumps into a checkpoint flag would
  never score the lap.
- **Doors Hotel** (`games/doors-hotel.world` and `games/doors-hotel.rbx`) — a horror game in the style of Roblox's
  *Doors*: a lobby and then 100 rooms down a corridor that turns, all generated by the script
  rather than painted into the world file, which keeps only the spawn, the sign and the entity's
  body. The rooms are seeded — each one is a plain room, a room split by a wall you have to walk
  around, a room with a pillar you pick a side of, or a corner where the corridor turns a quarter
  turn and the next door is in the side wall; the wardrobe side, the lighting, the paint and what
  furniture is stacked in a room all come from a seed and the room's number, so a run is the same
  every time and a different seed builds a different hotel. They are streamed as well: the script
  builds two rooms ahead of the deepest player and destroys the two behind them, so a hundred-door
  hotel only ever holds the handful of rooms someone can see, and the way back closes behind you.
  Each room gets a door that slides up when you walk into the invisible pad in front of it, a
  wardrobe on one side or the other that you step into to hide, and a lamp; every dozen seconds or
  so an entity starts behind the deepest player and runs past them, killing anyone still standing
  in the open, and a few seconds before it comes the sign and the lobby lamp above the door turn
  red. The whole hotel is under fog — the colour and density sit on the world file's header line,
  and the server sends them to every client as it joins, because a client is only ever handed the
  parts and never the file — so a room fades into the dark a few doors down instead of ending in a
  hard edge. A death sends you back to the last door you opened instead of to the lobby, and
  reaching the elevator past door 100 ends the run. Building a room the world file never had is
  what `workspace:CreatePart` is for, so the game is still only those two files.

Starter games are checked on every start, not just on an empty database: any starter game the data
file is missing is published then, so an install created before a game existed picks it up after
restarting the website. Starter games already present are never duplicated. The assets are read
from the repo root during development and from next to `WebApp.exe` in a packaged build (`games/`,
`web/seed-worlds/`, `ServerWorld.world`).

## Requirements

Client, Server, Studio, the website and the tools build on Windows only. Socket calls go through
`include/Network.h` and website calls through `include/HttpClient.h`, both of them WinSock now that
the POSIX branch is gone, so there is no platform split left in the game code. The anti-cheat library
is compiled into the client.

**Windows**

- Visual Studio 2022 with "Desktop development with C++", or MinGW + Ninja.
- No CMake install: the build tool is vendored in `rblxbuild/` and bootstrapped from source.
- An OpenGL-capable GPU and drivers.
- GLFW, GLM, ImGui, ImGuizmo, ImGuiColorTextEdit, ReactPhysics3D, stb and glad are vendored under
  `third_party/`, so configuring needs no network.

**The website** is built from the same tree and needs nothing beyond it: no Node.js, npm or SQLite.

`RBLXAntiCheat` is compiled into the client; `-DRBLX_BUILD_ANTICHEAT=OFF` leaves it out.

## Building

### Bootstrap the build tool once

```bash
mkdir -p build/rblxbuild-tool
cd build/rblxbuild-tool
../../rblxbuild/bootstrap --parallel 4
cd ../..
```

That leaves `build/rblxbuild-tool/bin/rblxbuild`, which every command below uses.

### Windows (Developer PowerShell or Command Prompt)

```powershell
build\rblxbuild-tool\bin\rblxbuild --preset windows-release
build\rblxbuild-tool\bin\rblxbuild --build --preset windows-release
build\rblxbuild-tool\bin\rblxtest --preset windows-release
```

Manual configure works too, with Visual Studio or a portable MinGW/Ninja toolchain:

```powershell
# Visual Studio
build\rblxbuild-tool\bin\rblxbuild -S . -B build/windows-release -G "Visual Studio 17 2022" -DRBLX_BUILD_TESTS=ON
build\rblxbuild-tool\bin\rblxbuild --build build/windows-release --config Release
build\rblxbuild-tool\bin\rblxtest --test-dir build/windows-release -C Release

# MinGW + Ninja
build\rblxbuild-tool\bin\rblxbuild -S . -B build/ninja -G Ninja `
  -DRBLXBUILD_C_COMPILER=C:\path\to\gcc.exe -DRBLXBUILD_CXX_COMPILER=C:\path\to\g++.exe -DRBLX_BUILD_TESTS=ON
build\rblxbuild-tool\bin\rblxbuild --build build/ninja
build\rblxbuild-tool\bin\rblxtest --test-dir build/ninja --output-on-failure
```

### Build options

| Option | Effect |
| --- | --- |
| `RBLX_BUILD_CLIENT` | Build the game client |
| `RBLX_BUILD_SERVER` | Build the game server |
| `RBLX_BUILD_STUDIO` | Build the world editor |
| `RBLX_BUILD_TESTS` | Build the C++ test suite (`rblxtest` runs it) |
| `RBLX_BUILD_ANTICHEAT` | Compile RBLXAntiCheat into the client |
| `RBLX_BUILD_HOSTS` | Build the game server supervisor (`RBLXHosts`) |
| `RBLX_ENABLE_WARNINGS` | Turn on the warning set |

Windows builds compile the vendored RBLXAntiCheat into the client (see `RBLXAntiCheat/`); on other
platforms that target is skipped.

## Running

After a Release build with the preset:

```powershell
.\build\windows-release\Release\Studio.exe
.\build\windows-release\Release\RBLXCode.exe
.\build\windows-release\Release\Server.exe
.\build\windows-release\Release\Client.exe
.\build\windows-release\Release\WebApp.exe
```

`RBLXCode.exe .` opens the source editor on the checkout it is launched from.

Start the website before using authenticated client login. A game server can also be started by
hand, with or without a script:

```powershell
.\build\windows-release\Release\Server.exe --world games/sky-platformer.world --script games/sky-platformer.rbx
```

## How it fits together

```
                 ┌──────────────┐        HTTP        ┌──────────────┐
   Client.exe ───┤  WebApp.exe  ├────────────────────┤  web/public  │
        │        │ accounts,    │                    └──────────────┘
        │        │ games, coins │
        │ UDP    │ moderation   │  starts when you press Play
        │        └──────┬───────┘
        │               │ CreateProcess
        ▼               ▼
   Server.exe ◄───────  Server.exe --port … --world … --script … --web …
   (join, snapshots, positions, chat — over one UDP socket)
```

- **The server is authoritative.** Positions it accepts, damage, spawns and world contents come from
  it, not from the client. That is what makes cheating cost an account rather than a packet.
- **The protocol is compact binary over UDP.** Positions, rotations and world updates travel as
  plain datagrams, so a lost update costs one frame instead of stalling the connection; the join
  handshake, world snapshot, chat, and join/leave events ride a reliability layer with
  acknowledgements, retransmission and fragmentation (`include/UdpTransport.h`). It is still a
  prototype protocol: the binary layout depends on matching C++ structs, so a client and server
  must be built from the same source. The handshake carries a protocol revision and a layout
  signature derived from the packet sizes; a mismatched pair refuses the join and prints which build
  to rebuild instead of misreading packets.
- **The website is native.** `WebApp.exe` is a C++ service (`WebApp`, `WebHttp`, `WebJson`,
  `WebCrypto`, `WebStore`) that serves `web/public`, owns accounts and sessions, and orchestrates
  local game servers. There is no Node.js service.
- **One shared copy of each asset set.** Shaders, textures, world files and the script the server
  runs all live beside the checkout, and a packaged build carries them next to the executables.

## Anti-cheat and moderation

Cheating costs the account, not just the packet.

### What the client can prove

`Client.exe` embeds **RBLXAntiCheat**, a usermode anti-cheat compiled into the client process
(vendored from [UltimateAntiCheat](https://github.com/AlSch092/UltimateAntiCheat) and renamed,
GNU AGPL v3 — see `RBLXAntiCheat/LICENSE`). It starts with the client, writes
`RBLXAntiCheat.log`, and a failed start is logged instead of stopping the game.

Its identifying strings (log name, startup messages, accepted parent processes) are stored
XOR-masked at compile time through `include/Obfuscated.h` so they do not show up in `strings`, and
MSVC Release builds link the client with control-flow guard, full ASLR and identical-code folding.
That raises the cost of finding and patching the checks; it is not a security boundary, which is
why the rules that matter (movement, world contents) are enforced by the server.

### The ladder

The game server's movement guard is the only thing that can prove a speed hack or a fly hack,
because the client's anti-cheat runs inside the client process and a modified client can simply not
run it. It measures speed against the time the player moved rather than the moment a datagram was
read: every `PLAYER_STATE` carries the client's own millisecond clock, and the server believes it
only while it agrees with when the packet arrived. That is what separates a burst of packets
delivered in one tick (measured over the time the player actually walked, so it is not a speed
hack), a stall both clocks agree on (a second of walking is a second of walking), and a client
claiming a quarter of a second in one datagram (held to the millisecond it took to arrive). Climbing
is budgeted rather than timed, and landing pays back what a jump spent, so a player who keeps
jumping or walks up a staircase is not reported as flying for it. A confirmed detection is reported
to the website, which keeps the account's strikes:

| Strike | Result |
| --- | --- |
| 1st, 2nd | Warning, shown to the player in game |
| 3rd | Suspension, 1 day |
| 4th | Suspension, 3 days |
| 5th | Suspension, 7 days |
| 6th | Ban |

A detection is not acted on where it is reported: it is held, and a wave gives every account whose
oldest held detection has waited out `BAN_WAVE_WINDOW_MS` (five minutes by default) exactly one
more step. So everyone caught in the same window is answered together, a burst of detections on one
account is one step rather than a climb, and nothing about the timing tells a cheater which check
it failed. The ladder itself is `include/Moderation.h`, so an install tunes it in one place, and
the admin dashboard lists each account's strikes and can pardon or ban by hand.

### Standing, and what a player is told

A suspended or banned account keeps its place on the site: it stays signed in and is told where it
stands, because a door that will not open is not something a player can argue with. Every page
carries the standing — warned, suspended until a time, or banned — and Settings has an **Account
standing** panel listing every entry that led there: what it cost, when, and when a suspension ends.

It does not repeat what was detected, because that names the check that fired and is a map for the
next attempt. The reason and the number of detections behind an action are read in the admin
dashboard only. Only joining a game is still refused, and the game server reads that refusal from
the login check it already makes.

### The proof a detection carries

A server detection carries its evidence with it:

- the frame the guard measured — the claimed position, the position the server had accepted, the
  gap between the packets, and the horizontal travel and climb claimed in that gap;
- the gameplay it was held against — walk and jump speeds, the slack they get, and the climb budget
  left;
- the last ninety packets of the player — where the client claimed to be and where the server put
  it — which the server keeps and sends with the report, so an admin can draw the claimed path and
  the accepted path from above.

All of it is stored with the detection, the wave copies it onto the action it files, and the
dashboard shows it with the appeal, because a correction an admin cannot check is a guess. A
server-side repositioning, such as a spawn or a respawn, starts the clip again, because a teleport
the server chose is not something the player did.

### Appeals

From the standing panel an account appeals one action at a time and only once per action: every
warning, suspension and ban in the record carries its own **Appeal** button, the appeal names the
action it is about, and a second appeal about the same action is refused. The dashboard lists
appeals with **Accept** and **Deny**, beside the action and reason behind each; accepting one is
the same pardon the dashboard already offers, and the record keeps who lifted the restriction and
why.

### Bans follow the machine

An account ban only costs a player the minute it takes to make another account, so a ban also
covers the computer it was earned on.

The client hashes what identifies the machine — the system volume's serial number and the computer
name on Windows, `/etc/machine-id` elsewhere — and sends only the 32-character hash in its join.
The disk serial and hostname never leave the process. `POST /api/verify` takes that hash as
`hardwareId`, records it on the account, and refuses the join when the machine is barred; a guest
join is checked through `POST /api/hardware-check`, so signing out is not a way back in.

Bans from the detection ladder and bans an admin applies both bar the machine the account was last
seen playing from. A pardon — the admin action or an accepted appeal — lifts the machine bans that
account created, so a false positive does not leave the player locked out of their own computer.
Machines can also be listed and released directly: `GET /api/admin/hardware-bans` and
`POST /api/admin/hardware-bans/lift`.

The hash is not a security boundary: a modified client can send any value it likes, so this raises
the cost of coming back rather than making it impossible.

### Client detections

The client's own anti-cheat is wired into the same ladder: what it detects inside the process is
sent to `POST /api/client-detections` with the player's token, and a report carrying a valid token
is attributable to that account and climbs the ladder like a server detection would. Without a
token there is nobody to punish, so a guest's client has its log and nothing else, and the endpoint
only ever files against the caller's own account. The dashboard counts client detections beside the
strikes.

Two things keep that honest:

- **Its own launcher is not suspicious.** A handle detection naming one of this project's own
  executables is dropped before it is reported, because the site launcher starts the client with
  `CreateProcess` and keeps the handle it waits on, so a clean launch always leaves one.
- **One scan is one detection.** The three lines a single open-handle scan writes (the handle, the
  process behind it, the scan's summary) are filed as the one detection they describe, so one scan
  cannot climb the whole ladder on its own. The module scan is filed the same way — one report
  however many modules it names — because a single start-up loads dozens of them.

Every loaded module still has to be signed by Microsoft, but that rule is applied by the module
scan and not by the loader: the loader-level `MicrosoftSignedOnly` policy applies to the whole
process, cannot except one publisher, and answers a refused DLL with a `Bad Image` dialog that
hangs the client mid-run. `Detections::IsAllowedModuleSignature` holds each module to a Microsoft
signature, read from the file's embedded signature or from the catalog Windows signs it through,
whichever holds it. Discord's publisher is the one named exception, and a module under
`%SystemRoot%` is held to a valid signature from its own vendor instead, since the graphics, audio
and chipset drivers install their libraries there and sign them with AMD's, Intel's or NVIDIA's
name; Discord's `DiscordHook64.dll` is loaded into every game running beside Discord and hooks the
frame the client presents. The client also drops detections that name that module, the way it
drops handle detections about its own launcher.
## The website

`WebApp.exe` is the website backend, written in C++ under `src/` (`WebApp`, `WebHttp`, `WebJson`,
`WebCrypto`, `WebStore`). It serves `web/public`, owns accounts, avatars, friends, games and local
game-server instances, and needs no npm, SQLite or third-party libraries.

```powershell
.\build\windows-release\Release\WebApp.exe
```

It listens on <http://localhost:3000> by default. Set `WEB_DIR` if it cannot find `web/public` from
its working directory.

### The server console

`WebApp.exe` opens a **server console window** instead of relying on a terminal. The window shows:

- the live server log with timestamps and levels, a filter box and level toggles;
- the address the site is served on;
- every game server it has running — port, pid, players, uptime — each with a **Stop** button and
  that server's captured output;
- buttons to open the site in the browser, copy the log and clear it. Closing the window stops the
  website and every game server it started.

The text is still written to the terminal, so run it with `--nogui` (or set `RBLX_NO_GUI=1`) on a
headless host or in CI. If no window can be created, it falls back to the plain console on its own.

### Configuration

Configuration comes from the environment:

| Variable | Default | Meaning |
| --- | --- | --- |
| `PORT` | `3000` | HTTP port |
| `WEB_DIR` | `web` | Directory containing `public/index.html` |
| `DATABASE_PATH` | `web/users.json` | JSON data file |
| `DATA_KEY` | — | Key the data file is sealed with: 64 hex characters, or any text hashed into 32 bytes. Without it the key is derived from `JWT_SECRET` |
| `JWT_SECRET` | — | Token signing secret; required, 32+ characters in production |
| `JWT_EXPIRES_IN` | `7d` | Token lifetime |
| `PASSWORD_ITERATIONS` | `120000` | PBKDF2-SHA256 rounds |
| `CORS_ORIGIN` | — | Comma-separated allowed origins; same-origin is always allowed |
| `AUTH_RATE_LIMIT_WINDOW_MS` / `AUTH_RATE_LIMIT_MAX` | — | Extra throttling for account endpoints |
| `RATE_LIMIT_WINDOW_MS` / `RATE_LIMIT_MAX` | `60000` / `600` | API budget per client address |
| `BAN_WAVE_WINDOW_MS` | `300000` | How long a detection is held before the wave acts |
| `GAME_WORLDS_DIR` | `web/game-worlds` | Where published `.world` files are written |
| `GAME_SERVER_HOST` / `GAME_SERVER_BASE_PORT` | `127.0.0.1` / `7777` | Local game-server address |
| `GAME_INSTANCE_EMPTY_GRACE_MS` | `30000` | How long an empty game instance stays up |
| `SERVER_EXECUTABLE_PATH` / `CLIENT_EXECUTABLE_PATH` | — | Binaries launched when a game is played |
| `PUBLIC_BASE_URL` | `http://localhost:3000` | URL handed to the native server and client for web auth |
| `ADMIN_USERNAMES` | — | Comma-separated extra accounts allowed into `/admin` |

Every `/api/` request is counted per client address in a fixed window with a budget of
`RATE_LIMIT_MAX` calls. Over budget the site answers `429` with `Retry-After` in seconds, and the
refusal is not counted as another call, so waiting out the window is enough to come back. The
budget is loose on purpose: one page load makes a handful of calls, and a game server running
beside the website calls the same endpoints over loopback. The account endpoints keep their own
tighter limit on top, and pages and assets are read from disk without being counted.

Passwords are stored as PBKDF2-SHA256 hashes and sessions are HS256 JWTs.

A fresh install seeds one account, `defaultuser0`, with the password `local-starter-account`. It
owns the starter games and is the default admin, so `POST /api/login` with
`{"username":"defaultuser0","password":"local-starter-account"}` signs into it. Installs from
earlier builds seeded the same account as `RBLXCreator`; it is renamed to `defaultuser0` on startup
with its id, password and games unchanged.

### The data file

`web/users.json` holds every account, so it is sealed rather than left as JSON
that anyone holding the file can read. Each account record is encrypted on its
own with **AES-256-GCM** under a fresh 12-byte nonce per save. What stays
readable is what the site already shows: the account id, the name it is looked up
by, timestamps, coin and playtime counts, and whether it is suspended. What does
not is the password hash, the recovery hash, the machine id, the moderation record
and the evidence attached to a detection.

Nothing is decrypted at startup. A record stays sealed until a request needs that
one account: signing in opens the account being signed into, reading a standing
opens that account's record, and the ban sweep asks the store which accounts hold
a queued detection instead of opening every account to find out. A record that was
opened stays open in memory for the life of the process, and the next save re-seals
it with a new nonce -- so a save never re-encrypts a record it did not have to
read, and no ciphertext on disk is ever written twice.

Each sealed value is bound to its account id as associated data, so a value moved
into another row fails to authenticate rather than decrypting into the wrong
account. A value that does not open (wrong key, edited file) leaves that account
with no password hash, which refuses every sign-in instead of letting one through,
and says so on stderr.

The key is `DATA_KEY`: 64 hex characters, or any other text, which is hashed with
SHA-256 into one. Set it. Without it the key is derived from `JWT_SECRET`, so an
upgrade from an older build keeps reading its own data and the startup log says
which key source is in use -- but changing `JWT_SECRET` after that locks the data
file, so set `DATA_KEY` first if you plan to rotate it. An install that has never
had a key keeps writing the old readable layout, which is what a file from an
earlier build looks like; the first save with a key seals every record in it.

### Accounts, recovery and deletion

There is no email address on an account and no mail service, so recovery works with a one-time code
instead. `POST /api/signup` returns a `recoveryCode` exactly once and stores only its
PBKDF2-SHA256 hash; `POST /api/account/recover` with
`{ "username": "...", "recoveryCode": "...", "newPassword": "..." }` sets a new password and rotates
the code, so a code that was used stops working. Codes are 20 hex characters shown in groups of
five, and the comparison strips the dashes and the case.

An account created before this existed — including the seeded `defaultuser0` — has no code until it
generates one with `POST /api/account/recovery`, which needs the current password. The login page
carries the recovery form under **Forgot your password?**, and Settings has a button to issue a new
code.

- `POST /api/account/username` renames the signed-in account after checking its password, applies
  the same 3–20 character rule as signup, and returns a fresh token because the username is a claim
  inside it.
- `POST /api/account/delete` removes the account with its avatar, stats, friendships, blocks, the
  reports it filed and its games — world files deleted, running instances stopped — after checking
  the password. The install owner is refused with `400`, because the install cannot be recovered
  without that account.
- `GET /api/account/export` returns the same data as JSON and leaves the password and recovery
  hashes out.

All three live in Settings, next to the password form.

Changing the password raises the account's token version, and every token carries the version it
was issued with, so sessions from before the change are refused with `401` instead of staying valid
until they expire. The form that changed the password is handed a fresh token and carries on; a
recovery revokes everything, because there is no session to hand one to. Renaming is not a
credential change, so other sessions survive it.

### Captchas

Signing up, logging in and resetting a forgotten password all need a solved captcha. There are two
kinds, and either one satisfies an endpoint:

- **Human.** `GET /api/captcha?kind=human` returns `id`, `kind`, `image` and `prompt`. The `image`
  is a self-contained `data:image/svg+xml` URI of four digits drawn from a 5x7 bitmap font, with
  jittered and rotated glyphs, noise lines and dots. The digits exist only as pixels, so the answer
  cannot be read back out of the markup the way a `<text>` element would allow. The signup, login
  and recovery forms send `captchaId` with `captchaAnswer`; spacing and dashes are ignored.
- **Machine.** `GET /api/captcha?kind=machine` returns `id`, `kind`, `difficulty`, `algorithm` and
  `input`. Find a decimal `nonce` where `sha256("<id>:<nonce>")` starts with `difficulty` hex
  zeros, then send `captchaId` with `captchaNonce`. This is what `Auth::login` does, so the desktop
  client's publish login keeps working: a program cannot read the image, but it can hash, and the
  work is useless to a person.

A challenge is single use and expires after five minutes. A wrong answer spends it too, so every
attempt needs a fresh one, and the existing per-address rate limit still applies on top. Challenges
live in the website process's memory, so restarting the backend invalidates the ones in flight.

```bash
# Ask for a machine challenge, then hash until the digest starts with zeros.
curl -s "http://localhost:3000/api/captcha?kind=machine"
```
### Blocks, reports and presence

`POST /api/blocks` with `{ "userId": 3, "blocked": true }` blocks an account and drops any
friendship between the pair; `false` removes the block. A block hides both accounts from each other
in `/api/users/search` and shows up on the profile as `"blocked": true`. It does not reach the
native game server, so it cannot stop someone joining a game you are in.

`POST /api/reports` files a report against a `game` or a `user`, which the admin dashboard lists
newest first under **Reports** and counts in its totals.

`/api/verify` and `/api/me/social` record the time of the request against the account, and a friend
counts as online while that time is under two minutes old — that is what the green dot next to a
name in the friend list means. The dashboard reloads `/api/me/social` every 45 seconds while it is
open, so presence stays current without a second connection.

### Coins and avatar items

`/avatar` sells hats, paid for with the coins the game server already awards for playtime: a minute
played is one coin. `GET /api/avatar` returns the catalog with `owned` and `equipped` flags plus
your balance, `POST /api/avatar/buy` charges the price, and `POST /api/avatar/equip` wears a hat or
takes it off (`{"itemId": ""}`). A hat has to be owned before it can be worn, buying one twice is
refused, and saving body colours through `POST /api/avatar` leaves the equipped hat alone.

The catalog is `include/AvatarItems.h`, so the shop, the web preview and the 3D client all draw a
hat from the same size, colour and offset. In the client the hat is a posed part that follows the
head every frame, so it moves with the walk and breathing animation. The hat travels in the player
list the game server sends, so the other players in your game see it too; an id this build's
catalog does not know is dropped rather than drawn.

Valid avatar `faceId` values are `classic`, `happy`, `surprised`, `smirk` and `wink`. The web
previews and the native client use the same transparent face PNG assets.

### Script editor

`/editor` is the website's code editor for a game's `.rbx` script — the file the game server loads
beside the world, which is what makes a game's parts move. It is for game code rather than the
engine's own sources, and it is not a second Studio: running and debugging a script still happen in
the desktop Studio, while this page opens the file a published game will run.

The page has a game picker, line numbers, `Tab` indentation, `Enter` that keeps the current
indentation, `Ctrl+S` (or `Cmd+S`) to save, the line and column under the cursor, and the size
limit. An unsaved buffer is marked, and leaving the page with one asks first. The creator page
links straight to it, with the game already selected.

`GET /api/games/:id/script` returns the owner's script text and the limit, and
`PUT /api/games/:id/script` with `{ "text": "..." }` writes it back in place. The first save names
the file after the game (`game-<id>.rbx` in the game-worlds directory) and stores that path on the
game; later saves rewrite the same path, so the file a game server was launched with never changes
under it. A script is capped at 256 KB, both routes are owner-only (anybody else gets `404`, so a
private game's existence is not revealed through them), and a game seeded from a starter game keeps
its own copy of the script, so editing it never touches the bundled original.

### Chat

`/chat` is a Discord-shaped chat view for the site: a server rail down the left, the channel list,
the message log and the member list, filling the window instead of sitting in the page column. The
four channels and the conversation in them are seed data that shows the shape of the page; the
composer works, groups consecutive messages by the same author, and keeps what you send, but it
keeps it in the tab -- there is no chat API behind this page, and the game's own chat is in the
client, inside a running instance. It is a preview of the layout, not a second chat network.

The layout folds down with the window: the member list becomes a drawer below 1180px and the
channel column below 900px, both closed with Escape or a click outside. Every colour comes from the
existing custom properties, so light and dark both work, and pages carry a **Chat** link in the
menu once you are signed in.

### Admin dashboard

`/admin` shows the local install at a glance: account, game, live server, launch and playtime
totals, the full user and game lists, the running game-server instances, and a Delete action that
removes a game together with its published `.world` file (stopping any instance for it first).

Each account row carries the balance it earned by playing and a Give box beside it, so an admin can
hand out coins — a prize, a refund, or making a run the server dropped good — without touching the
playtime that earned the rest. A grant is one account at a time and only ever adds, 1 to 100000
coins, and it lands in the same balance Settings and the avatar shop read.

Admin access is by account, and the account that owns the install is always an admin. That is the
seeded `defaultuser0` account, and it stays the admin after a rename or a differently-spelled seed,
because the owner is matched by id rather than by name. `ADMIN_USERNAMES` adds other accounts to the
list; it never replaces the owner, so naming someone else there cannot lock the install's own owner
out of `/admin`. Usernames are compared case-sensitively, so the flag follows the account rather
than the browser. Non-admins get `403` from `GET /api/admin/overview` and
`DELETE /api/admin/games/:id`, the Admin link on the dashboard stays hidden for them, and the
refusal names the account that is signed in and the names that are allowed, so a mismatch is
visible instead of silent.

### 404, CSP and themes

An address that matches no page answers with `404.html` and a real `404` status instead of a JSON
error. Every response carries a `Content-Security-Policy` that allows only same-origin scripts,
styles and images plus the vendored three.js build, with `'unsafe-inline'` for styles because the
avatar preview sets colours through the `style` attribute. No page uses an inline script or an
`on...` handler, so nothing relies on that being relaxed further.

Settings offers **Follow the system**, **Light** and **Dark**. The choice is stored in
`localStorage` beside Reduce motion and applied as `data-theme` on `<html>`; with the default,
`prefers-color-scheme` decides. Every colour is a CSS custom property, so the theme is one variable
block rather than a second stylesheet.

### Privacy and terms

`/privacy` and `/terms` are the policy pages (`web/public/privacy.html` and `terms.html`), both
public and linked from a footer that every page carries next to the menu bar. They describe what
this build actually does rather than boilerplate: the account record in the JSON data file,
passwords kept only as PBKDF2-SHA256 hashes, the JWT session and display preference in
`localStorage`, no cookies and no third-party analytics, public profiles and public games,
game-server logs under `web/game-server-logs`, and coins and hats as prototype-only value with no
cash equivalent. The terms cover accounts, acceptable use, published worlds, availability,
liability and the fact that the person running the install is the party responsible for it.

Both are a starting point for a local prototype, not legal advice: have them reviewed before
pointing an install at the open internet, and adjust the contact and governing-law wording to
whoever operates it.

### Settings

`/settings` is the account page. It shows the signed-in username, user id and coin balance, changes
the password through `POST /api/account/password` (`{"currentPassword": "...", "newPassword":
"..."}`, 8 to 128 characters, the current password is required, a wrong one is refused with `403`
and the new one must differ from the old), and has a Reduce motion switch.

The switch is a browser preference rather than an account setting: it is stored in `localStorage`
and applies the same `reduce-motion` rules as the `prefers-reduced-motion` media query, so it holds
for every page in that browser without touching the account.

## Web API

All routes take and return JSON; authenticated routes need `Authorization: Bearer <token>`. Tokens
are no longer accepted in avatar URLs. `/api/verify` still accepts a body token as a deprecated
compatibility fallback, but new callers should use the header.

### Authentication and the account

| Route | What it does |
| --- | --- |
| `POST /api/signup` | Creates the account; returns a one-time `recoveryCode` |
| `POST /api/login` | `{ "username": …, "password": … }` — returns the JWT |
| `POST /api/verify` | Checks a token, the account's standing and the machine id |
| `POST /api/hardware-check` | Answers whether a machine may join; used for guest joins |
| `POST /api/account/password` | Current and new password |
| `POST /api/account/recover` | `{ "username", "recoveryCode", "newPassword" }` — resets and rotates the code |
| `POST /api/account/recovery` | Current password; issues a fresh recovery code |
| `POST /api/account/username` | New name and current password; returns a fresh token |
| `POST /api/account/delete` | Current password; removes the account and everything it owns |
| `GET /api/account/export` | Everything kept about the account, as JSON |
| `GET /api/account/standing` | The caller's standing, infraction record and appeals |
| `POST /api/account/appeal` | `{ "message": "..." }` — files an appeal while restricted |

### Avatars, profiles and friends

| Route | What it does |
| --- | --- |
| `GET /api/avatar` | Catalog with `owned`/`equipped` flags and the balance |
| `POST /api/avatar` | Saves colours and `faceId` |
| `POST /api/avatar/buy` | Buys a hat |
| `POST /api/avatar/equip` | Wears or removes a hat (`{"itemId": ""}` removes) |
| `GET /api/me/social` | Own profile, friends and requests; also marks presence |
| `POST /api/me/playtime` | `{ "seconds": 120 }` — records authenticated playtime |
| `GET /api/users/search?q=Player` | Search by username or exact id |
| `GET /api/users/:id` | Public profile, avatar, stats and friends |
| `POST /api/friends/request` | `{ "userId": 2 }` — sends or accepts a request |
| `POST /api/friends/respond` | `{ "userId": 2, "action": "accept" }` |
| `POST /api/friends/remove` | Removes a friend or cancels a pending request |
| `POST /api/blocks` | `{ "userId": 2, "blocked": true }` |
| `POST /api/reports` | `{ "targetType": "game", "targetId": 1, "reason": "..." }` |

### Games

| Route | What it does |
| --- | --- |
| `GET /api/games` | Public games for the games page |
| `GET /api/games/:id` | Details, world stats and active local instances |
| `GET /api/games/mine` | Your published games |
| `POST /api/games` | `{ "title", "description", "isPublic", "worldText" }` — publishes a world file |
| `POST /api/games/:id/play` | Starts or reuses a local server; returns native launch metadata |
| `GET /api/games/:id/script` | Owner only: the script text and the 256 KB limit |
| `PUT /api/games/:id/script` | Owner only: `{ "text": "..." }` |

### Detection, moderation and admin

| Route | What it does |
| --- | --- |
| `POST /api/client-detections` | What the client's anti-cheat found; files against the caller's account |
| `POST /api/instance-detections` | What a game server's movement guard caught, with its proof |
| `GET /api/admin/overview` | Site totals plus user, game and instance lists |
| `DELETE /api/admin/games/:id` | Deletes a game and its world file |
| `POST /api/admin/users/:id/coins` | `{ "amount": 100 }` — gives an account coins |
| `POST /api/admin/users/:id/moderate` | Pardons or bans an account by hand |
| `POST /api/admin/appeals/:id` | `{ "action": "accept" }` or `deny`; accepting pardons |
| `GET /api/admin/hardware-bans` | Lists the machines barred from joining |
| `POST /api/admin/hardware-bans/lift` | `{ "hardwareId": "…" }` — lets one machine back in |

The game server reports authenticated player sessions to `/api/me/playtime` while a player is
connected and once more when the player disconnects.

### Trying it with curl

```bash
# Sign in as the seeded admin and keep the token.
TOKEN=$(curl -s localhost:3000/api/login -H 'Content-Type: application/json' \
  -d '{"username":"defaultuser0","password":"local-starter-account"}' | sed 's/.*"token":"\([^"]*\)".*/\1/')

curl -s localhost:3000/api/games
curl -s localhost:3000/api/avatar -H "Authorization: Bearer $TOKEN"
curl -s localhost:3000/api/account/standing -H "Authorization: Bearer $TOKEN"
```
## Scripting

The engine has its own scripting language, **RBLX Script**, a Lua-style subset used by Studio, the
game server and the seeded games. Scripts drive gameplay by moving parts and reacting to events.

```lua
local platform = workspace:FindFirstChild("MoveA")
local t = 0

RunService.Heartbeat:Connect(function(dt)
    t = t + dt
    platform.Position = Vector3.new(math.sin(t) * 6, 5, 0)
end)

workspace.Lava.Touched:Connect(function(hit)
    hit:Teleport(workspace.SpawnPoint.Position)
end)
```

### The language

- `local`, assignment, `if`/`elseif`/`else`, `while`, `repeat ... until`, numeric `for`, `break`
- named and anonymous functions with closures
- tables (`{1, 2}`, `t.field`, `t[1]`), strings, `..` concatenation
- arithmetic, comparison and logic, and `--` comments
- builtins: `print`, `warn`, `type`, `tostring`, `tonumber`, `error`, and `math` (`sin`, `cos`,
  `abs`, `floor`, `ceil`, `sqrt`, `min`, `max`, `random`, `pi`)

### The world API

| Call | What it does |
| --- | --- |
| `workspace:FindFirstChild("Name")`, `workspace.Name` | Find a part |
| `workspace:CreatePart(name, position, size, color)` | Builds a part the world file never had: anchored, given a physics body, returned so the script can move it and listen to it. A world is still capped at 4096 parts, and the new part and its later changes reach clients as their own packets, because the join snapshot only describes the world as it was at join time |
| `part:Destroy()` | Removes a created part; Doors Hotel builds rooms this way and takes them away again |
| `Name`, `Position`, `Size`, `Color`, `Transparency`, `Anchored`, `CanCollide` | Part properties. A repaint is announced to clients too, because a colour is not a position and the movement update would never show it |
| `part:Teleport(vector)` | Moves a part |
| `part.Touched:Connect(function(hit) ... end)` | Touch events |
| `RunService.Heartbeat:Connect(function(dt) ... end)` | Per-tick logic |
| `Players:GetPlayers()`, `Players.PlayerCount` | The players in the game |
| `player.Name`, `player:Teleport(vector)`, `player:TakeDamage(amount)` | Player access; damage is applied by the server and answered with a death and respawn |
| `workspace:PlaySound(name[, position[, volume]])` | Plays one of the build's sounds, at a place in the world when a position is given |
| `Vector3.new(x, y, z)` | A vector |

### Sound

A script makes a noise with `workspace:PlaySound("door")`, or with a `position` for one that comes
from somewhere in the world and an optional `volume` from 0 to 1. The server checks the name against
the build's list and forwards it to every client, which mixes it and fades it by how far the player
is from the position, so a door down the corridor is a whisper and the one in front of you is not. A
name the build does not have fails at the line that used it, with the list of what it does have,
rather than becoming a sound that never comes.

The sounds are synthesised by the client instead of shipped as files, which keeps the package a
folder of executables and keeps a sound from ever drifting out of step with the code that names it.
The set is `footstep`, `jump`, `door`, `wardrobe`, `entity`, `death`, `lap`, `lava`, `chime` and
`elevator`; the client plays footsteps and jumps itself, because a character's own feet should not
wait for a round trip. Three ships with the build: the lava and the goal in Sky Platformer, the lap
bell in Turbo Circuit, and the doors, wardrobes, entity, deaths and the elevator in Doors Hotel.

### Scripts in Studio and on the server

Run a script against a world with
`Server.exe --world games/sky-platformer.world --script games/sky-platformer.rbx`.

Studio has a **Script** panel for loading, editing, saving, running and stopping scripts in a
VS Code-style editor with line numbers, syntax highlighting, undo and a cursor status bar; the
world view updates live while a script runs. Published starter games get their script copied next
to the world file and passed to the server with `--script`.

## Controls

### Client

| Input | Action |
| --- | --- |
| Right mouse (hold) | Look around |
| `W` `A` `S` `D` | Move |
| `Space` | Jump |
| `Shift` | Toggle shift lock: captures the mouse, keeps the character facing the camera, moves the view over the shoulder |
| `E` | Get into or out of the car when standing next to it. While driving, `W`/`S` are throttle and brake/reverse, `A`/`D` steer |
| `Esc` | In-game menu: Resume, Reset Character, Settings, Leave Game; `Esc` again resumes |
| `F3` | Toggle the debug overlay |

**Browse games** in the player list shows public games straight from the website and joins them
without a browser. Settings holds shift lock, mouse sensitivity, invert mouse Y, field of view,
chat and player list toggles, the debug overlay, and a reset to defaults button.

### Studio

| Input | Action |
| --- | --- |
| Right mouse drag | Look around |
| `W` `A` `S` `D` | Move the camera |
| `E` / `Q` | Move the camera up / down |
| `Shift` | Faster camera movement |
| Left click | Select a part |

The **Toolbox** adds parts and switches the move/rotate/scale tools and snapping. **Properties**
edits the selected part, including the surface material (plastic, smooth plastic, neon, metal,
wood, grass, concrete, brick) and an optional texture image from `textures/`, tiled across the part.
**Publish** signs in against the web server, saves the current world, and publishes it to the site
as a public or private game.

### RBLXCode, the source editor

`RBLXCode` is the code editor for this repository, the way VS Code is the editor for its own
sources. It is not the Studio: Studio edits a world and runs a script, this edits the C++, shaders,
website and `.rbx` scripts that make up RBLX Clone itself, and saves them in place. Launch it from
the checkout (`RBLXCode.exe` on Windows, `./RBLXCode .` elsewhere), or name the folder to open, and
it walks that folder for source files.

It has a project tree on the left, an outline of the file in front on the right, tabs across the top
(a `*` marks unsaved work), the ImGui colour text editor Studio's Script panel uses in the middle,
and a status bar under it with the line and column, the language, the line count, whether the file
is saved and the branch of the checkout. C++ and headers are coloured as C++, `.glsl`, `.vert` and
`.frag` as GLSL, `.rbx` as RBLX Script (a Lua-shaped language, so the Lua definition is the right
one), and anything the widget has no definition for opens as plain text with its real name in the
status bar instead of being coloured wrongly.

| Shortcut | Action |
| --- | --- |
| `Ctrl+P` | Quick open: type an abbreviation of a path (`cmain` finds `src/CodeMain.cpp`, not the files that merely contain those letters) and `Enter` opens the best match |
| `Ctrl+Shift+O` | Go to a symbol in the open file. The outline lists functions, classes, structs, enums and namespaces in C++ and GLSL, functions and handlers in RBLX Script and JavaScript, rules in CSS, headings in Markdown and keys in JSON; declarations without a body are left out, and the outline is rebuilt only when the buffer changes |
| `Ctrl+F` | Find in the file in front, with match case and whole word, a `3 of 12` count and `Enter`/`Shift+Enter` or `F3`/`Shift+F3` to walk the matches |
| `Ctrl+H` | Adds the replace row: `Replace` takes one match, `Replace all` rewrites the buffer |
| `Ctrl+Shift+F` | Project search: a case-insensitive substring across the project, one row per matching line; clicking a row opens the file on that line |
| `Ctrl+G` | Go to a line |
| `Ctrl+/` | Comment or uncomment the line under the caret, with the marker the language uses (`//` or `--`) |
| `Ctrl+Space` | Suggest what fits the caret, ranked by what has been typed; after a `.` or `:` the list is the members of what stands before it |
| `F1` | The documentation page for the name under the caret, in a window that also holds every other page |
| `Ctrl+]` | Jump to the bracket pairing the one at the caret; brackets inside strings and comments are ignored |
| `Alt+Left`, `Alt+Right` | Walk back and forward through the places you jumped from |
| `Ctrl+Tab`, `Ctrl+Shift+Tab` | Switch tabs |
| `Ctrl+S`, `Ctrl+Shift+S` | Save the file in front / every open file |
| `Ctrl+R`, `Ctrl+W` | Reload from disk / close the tab |

Undo, redo, cut, copy and paste belong to the text widget and work as usual; `Ctrl+Z` also takes
back a single replace and a comment toggle.

#### Suggestions and documentation

Typing a name raises a list of what fits: the members of `workspace:` or `math.` after a dot, the
keywords of the language, the symbols this file declares, and the names the file already uses. A
prefix match ranks above a match in the middle, which ranks above an abbreviation, and the name
being typed is left out of its own list. `Up` and `Down` walk the suggestions, `Enter` or `Tab`
takes the highlighted one as a single edit (so one `Ctrl+Z` takes it back), `Escape` closes the
list, and `Ctrl+Space` asks for it without typing first.

Hovering a name shows how it is written and one sentence about what it does. Hovering a file in the
tree or a tab shows what that file says it does, taken from the comment it opens with. Either
tooltip has a button that opens the whole page.

`F1` opens the documentation window on whatever the caret is on, and the window reaches every page
there is: the RBLX Script API this build actually implements (`print`, `warn`, `math`,
`workspace:FindFirstChild`, `Vector3.new` and the rest of what the interpreter and the world runtime
install), the keywords and built-ins of every other language the editor colours, the symbols the
open file declares, the page for the file itself, and the editor's own pages for its project tree,
find and replace, outline, suggestions and this documentation. The filter box matches names by
abbreviation, and the related pages at the bottom of each one are links.

Those pages are built from the code instead of being kept beside it, which is why hovering a
function in this repository shows the comment already written above it: a symbol's documentation
cannot drift from the symbol it documents. A name the reference and the file both know -- a
keyword, say -- is described by the reference, which is the only place that knows what it means.

 The View menu sets the tab width (2, 4 or 8), shows
whitespace, switches to the light palette and hides the outline panel; the change applies to
already open files too. Closing a tab with unsaved changes asks whether to save it first.

A file that changes on disk while it is open is picked up on its own when the buffer has no edits.
When it does have edits the editor leaves them alone and shows a warning row with a choice between
reloading from disk and keeping what is in the buffer, instead of overwriting either side.

Line endings are preserved: a file is shown with plain newlines but saved back with the CRLF or LF
it arrived with, because this repository is not uniform (the anti-cheat sources are CRLF) and an
editor that rewrote every line would bury a one-line fix in a whole-file diff. Build trees, `.git`,
`node_modules`, vendored dependencies and the packaged `GameRelease` folder are left out of the
tree, images and models are never offered, and a file with a NUL byte or one larger than 4 MB is
refused with the reason in the status bar rather than opened.

In the Windows package, double-click `RBLXCode.exe`: it opens the folder it was started in (the
shaders, website and example games); the C++ sources are not in that package, so for those run it
from a checkout, where it opens the clone, or pass another folder's path.

## AI Trainer

`AiTrainer` learns to play a published game without opening a window. It signs in to the site with
a developer token, the site names the map and the course, and the trainer loads the same
`Character` and `PhysicsWorld` the game runs, so walk speed, jumping and falling behave exactly as
they do in game. The policy itself is a plain table of weights, improved with the cross-entropy
method.

Before the first run, sign in to the site and open `/developers`. Create an AI token there (it is
shown once; only its hash is stored) and tick **AI compatible** on the game the trainer should
play. A game its owner has not marked is refused for every token, including the owner's own. The
two starter games the trainer is meant for, Turbo Circuit and Doors Hotel, are switched on when a
fresh install seeds them. `WebApp.exe` has to be running, and `--site` points the trainer at it
when it is not on `http://127.0.0.1:3000`.

```
AiTrainer --token rblxai_... --game 4 --generations 60
AiTrainer --token rblxai_... --game 4 --replay
AiTrainer --token rblxai_... --game 4 --replay --watch
AiTrainer --token rblxai_... --game 4 --gui --agents 4
AiTrainer --token rblxai_... --game 4 --play --port 7777
```

A game's AI course is one of two, set beside the switch in the portal:

- **Walk to a part** trains the policy to reach the part named in the goal box, or the part
  furthest from the spawn when that is empty. Turbo Circuit is this course with the goal
  `FinishLine`.
- **Doors hotel to door 100** builds the hundred-door hotel the Doors script generates, from the
  same seed and the same measurements, and trains the run through all hundred doors to the
  elevator. The hotel lives in the script rather than in the `.world` file, so the trainer builds
  it instead of loading it. Progress is the door count, printed each generation as
  `doors 43/100`. The door pads and the wardrobe are not simulated: the trainer learns the walk,
  not the key presses.

The policy sees eight numbers (direction to the checkpoint it is walking to on X and Z, distance,
height above the spawn, a bias, and how far each of three feeler rays reaches before it hits a
wall on `-Z`, `-X` and `+X`) and picks one of six actions (forward, backward, left, right, jump,
idle) on fixed world axes, where forward is `-Z`. Reward is progress toward that checkpoint, a
bonus for every door and for the elevator, and a penalty for falling off the map.

The trained weights are written to `models/doors-hotel.policy` for a doors game, or
`models/<world>.policy` for a map game, as plain text with one row per action and a `course`
line, unless `--out` says otherwise. `--replay` loads that file and runs one episode, exiting 0
only if the goal was reached, which makes it usable as a check. The hotel's default budget is
16,000 steps because its corridor is nearly 3,000 studs long, and `--steps` overrides either
default. Training is CPU only and needs no GPU.

`--watch` draws the episode as a top-down map in the terminal while it happens: `@` is the
character, `G` the goal, `S` the spawn, `.` the path already walked, `#` a wall and `_` floor. It
works with `--replay`, and during training it shows the best policy once training finishes. The
view uses ANSI colours and cursor control, so run it in a terminal rather than piping it to a file.
Parts are drawn from their axis-aligned size, so a rotated part covers a little more than it really
does.

`--gui` opens a window instead of a terminal map: the course from above, one rectangle per part,
with the roof left off so the inside of a building shows. A part is taken off the picture when its
underside sits above the roof cut, so a ceiling disappears and the walls below it stay; the
window's **Roof cut** slider moves that height, and **Hide roof** turns the rule off to compare.
Each agent is a dot in its own colour with a trail of where it has been and a line to the
checkpoint it is walking towards, and the header counts its doors. `--agents 4` walks four policies
in the same world at once, which also settles four candidates per generation headless. Closing the
window ends training and saves the best policy found so far, so it is safe to stop and look. The
window takes its colours from the shared palette in `include/UiTheme.h`, like every other window
here.

`--play` puts the trained policy on the game server itself: the trainer joins a running
`Server.exe` as a guest, walks the same world with the same observations, and sends one position
packet per simulated step, so the run is the game's rather than this process's copy of it. Other
players in that game see the AI walk the course, and the server's movement guard checks its steps
like any player's, correcting it if it claims a speed a character cannot reach. `--server` and
`--port` name the server (default `127.0.0.1:7777`), `--name` is the name it joins under, and
`--gui` still opens the window so the same run can be watched from above. It plays the policy in
`--out`, so train or `--replay` first; the server has to be running the same game and be built
from the same source, or the handshake refuses the join with a reason in chat.

A live run starts by asking the server where it thinks the player stands, because the server
places a joining player at the world's `SpawnLocation` and measures every later step from there;
when that is not the course's own start it says so once and walks from the server's position. The
summary counts the steps the server refused to believe. A run that is corrected sixty steps in a
row stops and says the two sides are running different worlds, and a run whose AI the server
reports dead stops too, because the server drops a dead player's movement rather than arguing
about it.

## Repository layout

```
RBLXBuildLists.txt      C++ build configuration, in RBLXBuild's dialect
RBLXBuildPresets.json   Visual Studio Release preset (windows-release)
rblxbuild/              RBLXBuild itself: the vendored build tool's source
include/                C++ headers
src/                    Client, Server, Studio, rendering, physics, auth, web app, world loading
tests/                  C++ protocol, moderation and web tests
shaders/                OpenGL shaders used by Client and Studio
textures/               Tileable part textures a world can name
games/                  The bundled worlds and their .rbx scripts
web/                    Public pages, starter worlds, published worlds, game-server logs
web/host/               The RBLXHosts control panel page (served by RBLXHosts.exe)
ServerWorld.world       Default world file
scripts/                package-build.ps1 and other tooling
GameRelease/            Existing release asset folders (do not commit generated binaries here)
third_party/            Pinned dependencies, converted to this build dialect
RBLXAntiCheat/          The vendored anti-cheat library (Windows only, AGPL v3)
AGENTS.md               How to work in this repository: build, rules, conventions
SECURITY.md             Security model, known limitations, how to report a problem
```

## RBLXBuild and vendored dependencies

The project's own C++ compiler is gone: `RBLXCC` (`src/Cxx*.cpp`, `include/CxxCompiler.h`, its test
suite and `examples/rblxcc/`) has been removed and replaced by RBLXBuild — the CMake source tree,
rebranded and vendored as a plain directory.

```bash
git clone --depth 1 https://github.com/Kitware/CMake.git rblxbuild
rm -rf rblxbuild/.git
```

The directory, the vendored README and every identifier inside it carry the RBLXBuild name: the
command is `rblxbuild`, build files are `RBLXBuildLists.txt`, modules end in `.rblxbuild`,
variables are `RBLXBUILD_*`, and the module directory for Fortran interface checks is
`RBLXBuildAddFortranSubdirectory`. The one place the upstream name survives is
`rblxbuild/Licenses/`, because the BSD 3-clause terms require the copyright notice and disclaimer
to travel with the source. Removing its `.git` is what lets the outer repository track the files
instead of carrying a nested repository.

What is kept is the source that matters: `Source/`, `Modules/`, `Utilities/`, `bootstrap`, the
top-level build scripts and the licences. What was pruned, because none of it is read here:

- `Tests/` — the build tool's own test suite, 25,000 files and 113 MB of a 183 MB tree
- `Help/` — the reStructuredText sources the manual is generated from
- `Auxiliary/`, `Packaging/` — editor autocompletion data and packaging templates
- `.github/`, `.gitlab/`, `.gitlab-ci.yml`, `.devcontainer/` and the dotfile tooling config

So this is a source snapshot to read, not a tree that configures and builds as upstream CMake:
`add_subdirectory(Tests)` and the manual both expect the pruned directories back. CI and the build
instructions here use `rblxbuild/bootstrap`, and the root build file does not add it as a
subdirectory. See [RBLXBUILD.md](RBLXBUILD.md) for what it is for, what was pruned and the renamed
commands. Commit `62b91b06` still has the complete tree if any of it has to come back.

The libraries the game links are vendored the same way under `third_party/`: cloned at the pinned
commits, `.git` removed, and every build file converted to this dialect (glfw, glm, imgui,
ImGuizmo, ImGuiColorTextEdit, reactphysics3d, stb, glad). Configuring therefore downloads nothing
and needs no compatibility mode; `rblxbuild/Source/cmRBLXBuildCompat.h` remains only as a fallback
for code that has not been converted.

## CI and prebuilt packages

The `Build` workflow runs on every push: one Windows job with MSVC (its build tool is bootstrapped
with MinGW) configures, builds and runs the test suite, then packages the Release build into a folder
you can unzip and run. Open the latest run under **Actions** and download `rblx-windows-release`.

```
rblx-windows-release/
  WebApp.exe  Server.exe  Client.exe  Studio.exe  RBLXCode.exe
  web/public/  web/seed-worlds/
  shaders/  ServerWorld.world  face*.png  face_hex.txt
  RBLXUpdater.exe  RBLXSetup.exe  AiTrainer.exe  RBLXHosts.exe  update.json  manifest.json
  README.txt
```

The same folder can be produced locally with `./scripts/package-build.ps1 -Configuration Release`
after building that configuration.

## Multi-game local hosting

The website stores published `.world` files under `web/game-worlds` by default, along with any
`.rbx` script that belongs to the game. When a signed-in user presses Play on a game page, the web
backend finds an open local port, starts
`Server.exe --port <port> --world <published-world> --script <published-script> --web <site-url>`,
and can launch
`Client.exe --server <host> --port <port> --web <site-url> --token <jwt> --connect` when
`CLIENT_EXECUTABLE_PATH` points to a built Player.

Play waits for the server to own its port before launching the client, and each game server writes
its output to `web/game-server-logs/game-server-<port>.log`. When a server dies during startup — for
example because its port is still held by a leftover `Server.exe` — the game page shows that output
and the log path instead of a bare connection error.

## Known limitations

- This is a prototype and should be treated as local-only.
- The driving game's car is a kinematic arcade body: it accelerates, brakes, coasts, steers and
  stops at collidable parts, but there is no client-side prediction, so steering over a high-latency
  connection feels delayed. Collision uses the axis-aligned box around a rotated part, so a diagonal
  wall blocks a little more ground than it looks like it does.
- Sound is mono and synthesised. A sound fades with distance from the player instead of panning
  left or right, there is no volume control beyond the system mixer, and no audio file is loaded:
  the ten built-in sounds are the whole set, so a game that wants its own noise needs a code change.
- The HTTP client speaks plain `http://` only. There is no TLS implementation, so an `https://`
  backend URL fails with an explanation instead of working.
- The binary protocol depends on matching C++ struct layouts, so Client and Server must be built
  from the same source. The handshake carries a protocol revision and a layout signature derived
  from the packet sizes; a mismatched pair refuses the join and prints which build to rebuild
  instead of misreading packets.
- The web service keeps its data in a JSON file and sessions in browser storage; it is not hardened
  for internet deployment. Every response carries `X-Content-Type-Options: nosniff`,
  `X-Frame-Options: DENY`, `Referrer-Policy: same-origin` and a strict `Content-Security-Policy`,
  but that is baseline browser hygiene, not a security review.
- Account recovery depends on a recovery code the player saved, not on email, and presence is "an
  authenticated request in the last two minutes" rather than a game-server connection. Blocks and
  reports stop at the website: the native game server is not told about either.
- Machine bans raise the cost of returning, not the possibility: the client sends a hash, and a
  modified client can send any hash it likes (see
  [Bans follow the machine](#bans-follow-the-machine)).
- The client is split by concern: `src/Client.cpp` (startup, argument handling, main loop),
  `src/ClientNet.cpp` (game socket, reliable session, joining and leaving) and `src/ClientUi.cpp`
  (menus, HUD, chat, settings), sharing state through `src/ClientShared.h`. The remaining files
  could be split further.
- The C++ build and test suite are verified with the vendored RBLXBuild using Ninja and MinGW GCC,
  and the game links with MSVC in CI. Client, Server and Studio runtime behavior still need manual
  testing on a graphics-capable desktop.

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| `rblxbuild is not recognized` | Bootstrap the vendored tool first (see [Building](#building)) |
| `cl not found` | Run from a Visual Studio Developer shell, or install the C++ workload |
| `JWT_SECRET must be set` | Set a strong secret in production, or use development mode locally |
| Client login fails | Start the web backend first and confirm <http://localhost:3000> is reachable |
| Client cannot connect | Start `Server.exe`, allow firewall access, and connect to `127.0.0.1` for local testing |
| Play says the game server did not start | Read `web/game-server-logs/game-server-<port>.log`, which the error also quotes; a leftover `Server.exe` holding the port is the usual cause |

## Credits

- **RBLXAntiCheat** is a renamed fork of
  [UltimateAntiCheat](https://github.com/AlSch092/UltimateAntiCheat), GNU AGPL v3; the terms
  travel in `RBLXAntiCheat/LICENSE`.
- **RBLXBuild** is a renamed, pruned snapshot of [CMake](https://github.com/Kitware/CMake), BSD
  3-clause, with attribution kept in `rblxbuild/Licenses/`.
- The libraries under `third_party/` keep their own licences and pinned commits.
