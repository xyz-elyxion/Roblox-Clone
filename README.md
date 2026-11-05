# RBLX Clone

RBLX Clone is an early prototype of a Roblox-style client, game server, Studio editor, and web authentication/avatar service. It is usable for local experimentation, but it is not production-ready.

## Current Status

- Client, Server, Studio and RBLXCode are C++17 targets.
- Client and Server build on Windows, Linux and macOS. Socket calls go through `include/Network.h` and website calls through `include/HttpClient.h`, each of which has a WinSock/WinHTTP-facing Windows branch and a POSIX branch, so the game code itself has no platform split. The anti-cheat library is Windows-only and is skipped on other platforms.
- Client.exe embeds **RBLXAntiCheat**, a usermode anti-cheat compiled into the client process (vendored from [UltimateAntiCheat](https://github.com/AlSch092/UltimateAntiCheat) and renamed, GNU AGPL v3 — see `RBLXAntiCheat/LICENSE`). It starts with the client, writes `RBLXAntiCheat.log`, and a failed start is logged instead of stopping the game. Configure with `-DRBLX_BUILD_ANTICHEAT=OFF` to leave it out of the build. Its identifying strings (log name, startup messages, accepted parent processes) are stored XOR-masked at compile time through `include/Obfuscated.h` so they do not show up in `strings`, and MSVC Release builds link the client with control-flow guard, full ASLR and identical-code folding. That raises the cost of finding and patching the checks; it is not a security boundary, which is why the rules that matter (movement, world contents) are enforced by the server.
- Cheating costs the account, not just the packet. The game server's movement guard is the only thing that can prove it, because the client's anti-cheat runs inside the client process and a modified client can simply not run it; a confirmed speed hack or fly hack is reported to the website, which keeps the account's strikes. The first two strikes are warnings shown to the player in game, the next three are suspensions of a day, three days and a week, and the strike after that is a ban. A detection is not acted on where it is reported: it is held, and a wave gives every account whose oldest held detection has waited out `BAN_WAVE_WINDOW_MS` (five minutes by default) exactly one more step, so everyone caught in the same window is answered together, a burst of detections on one account is one step rather than a climb, and nothing about the timing tells a cheater which check it failed. A suspended or banned account keeps its place on the site: it stays signed in and is told where it stands, because a door that will not open is not something a player can argue with. Every page carries the standing - warned, suspended until a time, or banned - and Settings has an Account standing panel listing every entry that led there: what it cost, when, and when a suspension ends. It does not repeat what was detected, because that names the check that fired and is a map for the next attempt; the reason and the number of detections behind an action are read in the admin dashboard only. A server detection carries its proof with it: the frame the guard measured - the claimed position, the position the server had accepted, the gap between the packets, and the horizontal travel and climb claimed in that gap - beside the gameplay it was held against, which is the walk and jump speeds, the slack they get and the climb budget left. Both are stored with the detection, the wave copies them onto the action it files, and the dashboard shows them with the appeal, because a correction an admin cannot check is a guess. The game server also keeps the last ninety packets of every player - where the client claimed to be and where the server put it - and sends that clip with a report; it is stored with the detection beside the numbers, and an admin can draw it from the appeal it led to, with the claimed path and the accepted path from above. A server-side repositioning, such as a spawn or a respawn, starts the clip again, because a teleport the server chose is not something the player did. From that panel an account appeals one action at a time and only once per action: every warning, suspension and ban in the record carries its own Appeal button, the appeal names the action it is about, and a second appeal about the same action is refused. The dashboard lists appeals with Accept and Deny, and with the action and reason behind each; accepting one is the same pardon the dashboard already offers, and the record keeps who lifted the restriction and why. Only joining a game is still refused, and the game server reads that refusal from the login check it already makes. The ladder itself is `include/Moderation.h`, so an install tunes it in one place, and the admin dashboard lists each account's strikes and can pardon or ban by hand. The client's own anti-cheat is wired into the same ladder: what it detects inside the process is sent to `POST /api/client-detections` with the player's token, and a report that carries a valid token is attributable to that account and climbs the ladder like a server detection would. Without a token there is nobody to punish, so a guest's client has its log and nothing else, and the endpoint only ever files against the caller's own account. The dashboard counts those client detections beside the strikes. Two things keep that honest: a handle detection naming one of this project's own executables is dropped before it is reported, because the site launcher starts the client with `CreateProcess` and keeps the handle it waits on, so a clean launch always leaves one; and the three lines a single open-handle scan writes (the handle, the process behind it, the scan's summary) are filed as the one detection they describe, so one scan cannot climb the whole ladder on its own, and the module scan is filed the same way, one report however many modules it names, because a single start-up loads dozens of them. Every loaded module still has to be signed by Microsoft, but that rule is applied by the module scan and not by the loader: the loader-level `MicrosoftSignedOnly` policy applies to the whole process, cannot except one publisher, and answers a refused DLL with a `Bad Image` dialog that hangs the client mid-run. `Detections::IsAllowedModuleSignature` holds each module to a Microsoft signature, read from the file's embedded signature or from the catalog Windows signs it through, whichever holds it; Discord's publisher is the one named exception, and a module under `%SystemRoot%` is held to a valid signature from its own vendor instead, since the graphics, audio and chipset drivers install their libraries there and sign them with AMD's, Intel's or NVIDIA's name; Discord's `DiscordHook64.dll` is loaded into every game running beside Discord and hooks the frame the client presents; the client also drops detections that name that module, the way it drops handle detections about its own launcher.
- Health and respawning are decided by the game server, not the client. The server keeps each player's health, kills anyone who falls below the world or takes damage, and returns them to the world's `SpawnLocation` after a three-second countdown that every client sees. The client draws the health bar and a "You died" overlay and stops taking movement input while dead; `Reset Character` asks the server rather than respawning locally. Scripts damage players with `player:TakeDamage(amount)`.
- **Turbo Circuit** is the bundled driving game. A world part named `Car` is the vehicle: the game server integrates it from client throttle and steer input (`include/Vehicle.h`) and broadcasts its position, so a modified client cannot teleport the car any more than a character. Walk up to it and press `E` to get in or out; the driver's character rides in the car, the camera follows behind and the HUD shows km/h. A `.rbx` script counts laps at the finish line. The car stops at the world's collidable parts and slides along a wall it only clips, so a lap cannot be finished by driving through the edge of the track; the finish line and the checkpoints are non-colliding triggers, because a car that bumps into a checkpoint flag would never score the lap.
- **Doors Hotel** is the bundled horror game, in the style of Roblox's *Doors*: a lobby and then 100 rooms down a corridor that turns, all of them generated by `games/doors-hotel.rbx` rather than painted into the world file, which keeps only the spawn, the sign and the entity's body. The rooms are seeded: each one is a plain room, a room split by a wall you have to walk around, a room with a pillar you pick a side of, or a corner where the corridor turns a quarter turn and the next door is in the side wall; the wardrobe side, the lighting, the paint and what furniture is stacked in a room all come from a seed and the room's number, so a run is the same every time and a different seed builds a different hotel. They are streamed as well - the script builds two rooms ahead of the deepest player and destroys the two behind them, so a hundred-door hotel only ever holds the handful of rooms someone can see, and the way back closes behind you. Each room gets a door that slides up when you walk into the invisible pad in front of it, a wardrobe on one side or the other that you step into to hide, and a lamp; every dozen seconds or so an entity starts behind the deepest player and runs past them, killing anyone still standing in the open, and a few seconds before it comes the sign and the lobby lamp above the door turn red. The whole hotel is under fog - the colour and density sit on the world file's header line, and the server sends them to every client as it joins, because a client is only ever handed the parts and never the file - so a room fades into the dark a few doors down instead of ending in a hard edge. A death sends you back to the last door you opened instead of to the lobby, and reaching the elevator past door 100 ends the run. Building a room the world file never had is what `workspace:CreatePart` is for, so the game is still only those two files.
- Studio uses GLFW/OpenGL/ImGui/ImGuizmo and has a basic world editing path.
- The web backend is `WebApp.exe`, a native C++ service (`src/WebApp.cpp`) that serves `web/public` and owns accounts (PBKDF2-SHA256), JWT sessions, avatars, friends, the public game catalog, and local game-server launch orchestration. There is no Node.js service any more.
- `rblxbuild/` is **RBLXBuild**, this project's build-tool source: a renamed fork of the CMake source tree, with its own `.git` removed so this repository tracks the files - `RBLXBuildLists.txt`, `.rblxbuild` modules and `RBLXBUILD_*` variables throughout, with only `rblxbuild/Licenses/` keeping upstream attribution. It stands in place of the project's own `RBLXCC` compiler, which is gone; the targets above are configured by the `RBLXBuildLists.txt` at the repository root and built by that tool, not by an installed CMake. See [RBLXBUILD.md](RBLXBUILD.md).
- Networking is a compact binary protocol carried over UDP. Positions, rotations, and world updates travel as plain datagrams, so a lost update costs one frame instead of stalling the connection; the join handshake, world snapshot, chat, and join/leave events ride a reliability layer with acknowledgements, retransmission, and fragmentation (`include/UdpTransport.h`). It is still a prototype protocol.

## Repository Layout

- `RBLXBuildLists.txt` - C++ build configuration, written in RBLXBuild's dialect.
- `RBLXBuildPresets.json` - A Visual Studio Release preset for Windows and a Ninja Release preset for macOS.
- `scripts/package-build.ps1` - Assembles a runnable folder from a build tree.
- `include/` - C++ headers.
- `src/` - C++ Client, Server, Studio, rendering, physics, auth, and world loading code.
- `shaders/` - OpenGL shader files used by Client and Studio.
- `textures/` - Tileable part textures a world can name.
- `tests/` - C++ protocol tests.
- `web/` - public HTML pages, starter worlds and published `.world` files served by `WebApp.exe`.
- `ServerWorld.world` - Default world file.
- `GameRelease/` - Existing release asset folders. Do not commit generated binaries here.

## Prerequisites

### C++ Targets

Client, Server and Studio build on Windows, Linux and macOS. Sockets go through `include/Network.h` and website calls through `include/HttpClient.h`, each with a WinSock branch and a POSIX branch, so the game code itself has no platform split.

On Windows:

- Visual Studio 2022 with "Desktop development with C++".
- No CMake install: the build tool is vendored in `rblxbuild/` and bootstrapped from source (see Build C++ below).
- OpenGL-capable GPU/drivers.

On Linux:

- GCC 11+ or Clang 14+ (anything with C++17), Ninja or Make.
- `build-essential ninja-build pkg-config` plus the GLFW development packages: `libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libwayland-dev libxkbcommon-dev`.
- OpenGL-capable GPU/drivers.

On macOS:

- Xcode command line tools (Clang with C++17).
- Ninja, either from Homebrew (`brew install ninja`) or the preinstalled copy on the CI runners.
- GLFW, GLM, ImGui, ImGuizmo, ReactPhysics3D and stb are fetched at configure time, so no Homebrew packages are needed for them.

RBLXAntiCheat is Windows-only and is skipped automatically on other platforms; `-DRBLX_BUILD_ANTICHEAT=OFF` leaves it out on Windows too.

### Web Backend

`WebApp.exe` is built from `src/WebApp.cpp` with the rest of the C++ targets and needs nothing
beyond them: no Node.js, npm or SQLite.

## Build C++

The build tool is vendored, so nothing has to be installed: bootstrap it once and use
the copy that comes out of `rblxbuild/`.

```bash
mkdir -p build/rblxbuild-tool
cd build/rblxbuild-tool
../../rblxbuild/bootstrap --parallel 4
cd ../..
```

That leaves `build/rblxbuild-tool/bin/rblxbuild`; every command below is that binary.

From a Developer PowerShell or Developer Command Prompt:

```powershell
build\rblxbuild-tool\bin\rblxbuild --preset windows-release
build\rblxbuild-tool\bin\rblxbuild --build --preset windows-release
build\rblxbuild-tool\bin\rblxtest --preset windows-release
```

Manual configure also works:

```powershell
build\rblxbuild-tool\bin\rblxbuild -S . -B build/windows-release -G "Visual Studio 17 2022" -DRBLX_BUILD_TESTS=ON
build\rblxbuild-tool\bin\rblxbuild --build build/windows-release --config Release
build\rblxbuild-tool\bin\rblxtest --test-dir build/windows-release -C Release
```

The project has also been verified with a portable MinGW/Ninja toolchain:

```powershell
build\rblxbuild-tool\bin\rblxbuild -S . -B build/ninja -G Ninja -DRBLXBUILD_C_COMPILER=C:\path\to\gcc.exe -DRBLXBUILD_CXX_COMPILER=C:\path\to\g++.exe -DRBLX_BUILD_TESTS=ON
build\rblxbuild-tool\bin\rblxbuild --build build/ninja
build\rblxbuild-tool\bin\rblxtest --test-dir build/ninja --output-on-failure
```

On Linux:

```bash
build/rblxbuild-tool/bin/rblxbuild -S . -B build/linux-release -G Ninja -DRBLXBUILD_BUILD_TYPE=Release
build/rblxbuild-tool/bin/rblxbuild --build build/linux-release --parallel
build/rblxbuild-tool/bin/rblxtest --test-dir build/linux-release --output-on-failure
```

On macOS, with the Xcode command line tools and Ninja installed (`brew install ninja`):

```bash
build/rblxbuild-tool/bin/rblxbuild --preset macos-release
build/rblxbuild-tool/bin/rblxbuild --build --preset macos-release
build/rblxbuild-tool/bin/rblxtest --preset macos-release
```

The same configure works by hand as `build/rblxbuild-tool/bin/rblxbuild -S . -B
build/macos-release -G Ninja -DRBLXBUILD_BUILD_TYPE=Release`. The anti-cheat library is
Windows-only, so it is left out of the Linux and macOS builds.

Useful build options:

- `RBLX_BUILD_CLIENT=ON|OFF`
- `RBLX_BUILD_SERVER=ON|OFF`
- `RBLX_BUILD_STUDIO=ON|OFF`
- `RBLX_BUILD_TESTS=ON|OFF`
- `RBLX_BUILD_ANTICHEAT=ON|OFF`
- `RBLX_ENABLE_WARNINGS=ON|OFF`

Windows builds also compile the vendored RBLXAntiCheat into the client (see `RBLXAntiCheat/`); on other platforms that target is skipped.

## RBLXBuild (`rblxbuild/`)

The project's own C++ compiler is gone: `RBLXCC` (`src/Cxx*.cpp`,
`include/CxxCompiler.h`, its test suite and `examples/rblxcc/`) has been removed
and replaced by RBLXBuild: the CMake source tree, rebranded and vendored as a plain directory.

```bash
git clone --depth 1 https://github.com/Kitware/CMake.git rblxbuild
rm -rf rblxbuild/.git
```

The directory, the vendored README and every identifier inside it carry the RBLXBuild name: the command is `rblxbuild`, build files are `RBLXBuildLists.txt`, modules end in `.rblxbuild`, variables are `RBLXBUILD_*` and the module directory for Fortran interface checks is `RBLXBuildAddFortranSubdirectory`. The one place the upstream name survives is `rblxbuild/Licenses/`, because the BSD 3-clause terms require the copyright notice and disclaimer to travel with the source. Removing its `.git` is what lets the outer repository track the files instead of carrying a nested repository. What is kept is the source that matters: `Source/`,
`Modules/`, `Utilities/`, `bootstrap`, the top-level CMake scripts and the
licences. What was pruned, because none of it is read here:

- `Tests/` - CMake's own test suite, 25,000 files and 113 MB of a 183 MB tree
- `Help/` - the reStructuredText sources the manual is generated from
- `Auxiliary/`, `Packaging/` - editor autocompletion data and packaging templates
- `.github/`, `.gitlab/`, `.gitlab-ci.yml`, `.devcontainer/` and the dotfile
  tooling config - CI and editor metadata for the upstream repository

So this is a source snapshot to read, not a tree that configures and builds as
upstream CMake: `add_subdirectory(Tests)` and the manual both expect the pruned
directories back. CI and the Build C++ instructions build it with `rblxbuild/bootstrap`; the root
build file does not add it as a subdirectory. See [RBLXBUILD.md](RBLXBUILD.md) for what it is for, what was pruned, the renamed
commands and how it relates to the tool this project builds with.
Commit `62b91b06` still has the complete
tree if any of it has to come back.
## Run C++ Targets

After building Release with the preset:

```powershell
.\build\windows-release\Release\Studio.exe
.\build\windows-release\Release\RBLXCode.exe
.\build\windows-release\Release\Server.exe
.\build\windows-release\Release\Client.exe
.\build\windows-release\Release\WebApp.exe
```

On Linux and macOS the same executables land directly in `build/linux-release` and
`build/macos-release`, and `./build/linux-release/RBLXCode .` opens the source editor on the
checkout it is launched from.

Start the web backend before using authenticated Client login.

## Prebuilt Windows Package

The `Build` workflow runs on every push and packages the Release build into a folder you can
unzip and run. Open the latest run under **Actions** and download `rblx-windows-release`; the
Linux and macOS jobs build, run the tests and upload their five binaries as
`rblx-linux-release` and `rblx-macos-release`. Only the Windows artifact is a package folder
ready to unzip and start; the other two are the bare executables, which expect to be run from
the checkout's build directory beside their assets:

```
rblx-windows-release/
  WebApp.exe  Server.exe  Client.exe  Studio.exe  RBLXCode.exe
  web/public/  web/seed-worlds/
  shaders/  ServerWorld.world  face*.png  face_hex.txt
  start-website.bat  start-studio.bat  start-client.bat  start-code.bat
  README.txt
```

Double-click `start-website.bat`: it starts `WebApp.exe` from that folder, points it at the
`Server.exe` and `Client.exe` beside it, and opens <http://localhost:3000> in your browser.
Pressing Play on a game page then launches the client from the same folder. On a freshly unzipped copy the first launch opens the guided tour at
<http://localhost:3000/welcome> instead: nine steps through the website, the client and
game servers, Studio, RBLXCode and the trainer, each one checking itself against the
running site where it can. **Tour** in the site navigation reopens it. `start-studio.bat`
opens the world editor and `start-code.bat` the source editor.

Nothing else needs to be installed. The executables link the C++ runtime statically, so
neither Visual Studio nor the VC++ redistributable is required at runtime; keep the folder
layout intact because the executables load `shaders/`, `ServerWorld.world` and `web/`
relative to the working directory. The same folder can be produced locally with
`./scripts/package-build.ps1 -Configuration Release` after building that configuration.

## Native Web Backend

`WebApp.exe` is the website backend, written in C++ under `src/` (`WebApp`, `WebHttp`, `WebJson`,
`WebCrypto`, `WebStore`). It serves `web/public`, owns accounts, avatars, friends, games and local
game-server instances, and needs no npm, SQLite or third-party libraries.

```powershell
.\build\windows-release\Release\WebApp.exe
```

It listens on `http://localhost:3000` by default. Set `WEB_DIR` if the executable cannot find
`web/public` from its working directory.

`WebApp.exe` opens a **server console window** instead of relying on a terminal. The window shows
the live server log with timestamps and levels, a filter box and level toggles, the address the
site is served on, and every game server it has running (port, pid, players, uptime) with a Stop
button and that server's captured output. Buttons cover opening the site in the browser, copying
the log and clearing it; closing the window stops the website and every game server it started.
The text is still written to the terminal, so run it with `--nogui` (or set `RBLX_NO_GUI=1`) on a
headless host or in CI. If no window can be created, it falls back to the plain console on its own.

Configuration comes from the environment:

- `PORT` - HTTP port, default `3000`.
- `WEB_DIR` - directory containing `public/index.html`, default `web`.
- `DATABASE_PATH` - JSON data file, default `web/users.json`.
- `JWT_SECRET` - token signing secret; required and at least 32 characters in production.
- `JWT_EXPIRES_IN` - token lifetime, default `7d`.
- `PASSWORD_ITERATIONS` - PBKDF2-SHA256 rounds, default `120000`.
- `CORS_ORIGIN` - comma-separated allowed origins; same-origin requests are always allowed.
- `AUTH_RATE_LIMIT_WINDOW_MS` / `AUTH_RATE_LIMIT_MAX` - auth throttling.
- `RATE_LIMIT_WINDOW_MS` / `RATE_LIMIT_MAX` - API throttling per client address, default `60000` / `600`.
- `BAN_WAVE_WINDOW_MS` - how long a detection is held before the wave acts on it, default `300000`.
- `GAME_WORLDS_DIR` - where published `.world` files are written.
- `GAME_SERVER_HOST` / `GAME_SERVER_BASE_PORT` - local game-server address, default `127.0.0.1:7777`.
- `GAME_INSTANCE_EMPTY_GRACE_MS` - how long an empty game instance stays up, default `30000`.
- `SERVER_EXECUTABLE_PATH` / `CLIENT_EXECUTABLE_PATH` - binaries launched when a game is played.
- `PUBLIC_BASE_URL` - URL handed to the native server/client for web auth.
- `ADMIN_USERNAMES` - comma-separated extra accounts allowed into `/admin`; the install owner is always allowed.

Every `/api/` request is counted per client address in a fixed window (`RATE_LIMIT_WINDOW_MS`, default one minute) with a budget of `RATE_LIMIT_MAX` calls (default `600`). Over budget the site answers `429` with a `Retry-After` header in seconds, and the refusal is not counted as another call, so waiting out the window is enough to come back. The budget is loose on purpose: one page load makes a handful of calls, and a game server running beside the website calls the same endpoints over loopback. The account endpoints keep their own tighter limit on top, and pages and assets are read from disk without being counted.

Passwords are stored as PBKDF2-SHA256 hashes and sessions are HS256 JWTs.

A fresh install seeds one account, `defaultuser0`, with the password `local-starter-account`. It owns
the starter games and is the default admin, so `POST /api/login` with
`{"username":"defaultuser0","password":"local-starter-account"}` signs into it. Installs from
earlier builds seeded the same account as `RBLXCreator`; it is renamed to `defaultuser0` on startup
with its id, password and games unchanged.

## Accounts, recovery and deletion

There is no email address on an account and no mail service, so recovery works with a one-time code
instead. `POST /api/signup` returns a `recoveryCode` exactly once and stores only its PBKDF2-SHA256 hash;
`POST /api/account/recover` with `{ "username": "...", "recoveryCode": "...", "newPassword": "..." }`
sets a new password and rotates the code, so a code that was used stops working. Codes are 20 hex
characters shown in groups of five, and the comparison strips the dashes and the case. An account created
before this existed - including the seeded `defaultuser0` - has no code until it generates one with
`POST /api/account/recovery`, which needs the current password. The login page carries the recovery form
under **Forgot your password?**, and Settings has a button to issue a new code.

`POST /api/account/username` renames the signed-in account after checking its password, applies the same
3-20 character rule as signup, and returns a fresh token because the username is a claim inside it.
`POST /api/account/delete` removes the account with its avatar, stats, friendships, blocks, the reports it
filed and its games - world files deleted, running instances stopped - after checking the password. The
install owner is refused with `400`, because the install cannot be recovered without that account.
`GET /api/account/export` returns the same data as JSON and leaves the password and recovery hashes out.
Both live in Settings, next to the password form.

Changing the password raises the account's token version, and every token carries the version it
was issued with, so sessions from before the change are refused with `401` instead of staying valid
until they expire. The form that changed the password is handed a fresh token and carries on; a
recovery revokes everything, because there is no session to hand one to. Renaming is not a credential
change, so other sessions survive it.

## Captchas

Signing up, logging in and resetting a forgotten password all need a solved captcha. There are two kinds and either one satisfies an endpoint:

- **Human.** `GET /api/captcha?kind=human` returns `id`, `kind`, `image` and `prompt`. The `image` is a self-contained `data:image/svg+xml` URI of four digits drawn from a 5x7 bitmap font, with jittered and rotated glyphs, noise lines and dots. The digits exist only as pixels, so the answer cannot be read back out of the markup the way a `<text>` element would allow. The signup, login and recovery forms send `captchaId` with `captchaAnswer`; spacing and dashes are ignored.
- **Machine.** `GET /api/captcha?kind=machine` returns `id`, `kind`, `difficulty`, `algorithm` and `input`. Find a decimal `nonce` where `sha256("<id>:<nonce>")` starts with `difficulty` hex zeros, then send `captchaId` with `captchaNonce`. This is what `Auth::login` does, so the desktop client's publish login keeps working: a program cannot read the image, but it can hash, and the work is useless to a person.

A challenge is single use and expires after five minutes. A wrong answer spends it too, so every attempt needs a fresh one, and the existing per-address rate limit still applies on top. Challenges live in the website process's memory, so restarting the backend invalidates the ones in flight.

```bash
# Ask for a machine challenge, then hash until the digest starts with zeros.
curl -s "http://localhost:3000/api/captcha?kind=machine"
```

## Blocks, reports and presence

`POST /api/blocks` with `{ "userId": 3, "blocked": true }` blocks an account and drops any friendship
between the pair; `false` removes the block. A block hides both accounts from each other in
`/api/users/search` and shows up on the profile as `"blocked": true`. It does not reach the native game
server, so it cannot stop someone joining a game you are in. `POST /api/reports` files a report against a
`game` or a `user`, which the admin dashboard lists newest first under **Reports** and counts in its totals.

`/api/verify` and `/api/me/social` record the time of the request against the account, and a friend counts
as online while that time is under two minutes old, which is what the green dot next to a name in the
friend list means. The dashboard reloads `/api/me/social` every 45 seconds while it is open, so presence
stays current without a second connection.

## 404 page, CSP and theme

An address that matches no page now answers with `404.html` and a real `404` status instead of a JSON error.
Every response also carries a `Content-Security-Policy` that allows only same-origin scripts, styles and
images plus the vendored three.js build, with `'unsafe-inline'` for styles because the avatar preview sets
colours through the `style` attribute. No page uses an inline script or `on...` handler, so nothing relies
on that being relaxed further.

Settings offers **Follow the system**, **Light** and **Dark**. The choice is stored in `localStorage` beside
Reduce motion and applied as `data-theme` on `<html>`; with the default, `prefers-color-scheme` decides.
Every colour is a CSS custom property, so the theme is one variable block rather than a second stylesheet.

## Privacy and Terms

`/privacy` and `/terms` are the policy pages (`web/public/privacy.html` and `terms.html`), both public and
linked from a footer that every page carries next to the menu bar. They describe what this build actually
does rather than boilerplate: the account record in the JSON data file, passwords kept only as PBKDF2-SHA256
hashes, the JWT session and display preference in `localStorage`, no cookies and no third-party analytics,
public profiles and public games, game-server logs under `web/game-server-logs`, and coins and hats as
prototype-only value with no cash equivalent. The terms cover accounts, acceptable use, published worlds,
availability, liability and the fact that the person running the install is the party responsible for it.

Both are a starting point for a local prototype, not legal advice: have them reviewed before pointing an
install at the open internet, and adjust the contact and governing-law wording to whoever operates it.

## Settings

`/settings` is the account page. It shows the signed-in username, user id and coin balance, changes
the password through `POST /api/account/password` (`{"currentPassword": "...", "newPassword": "..."}`,
8 to 128 characters, the current password is required, a wrong one is refused with `403` and the new
one must differ from the old), and has a Reduce motion switch.

The switch is a browser preference rather than an account setting: it is stored in `localStorage` and
applies the same `reduce-motion` rules as the `prefers-reduced-motion` media query, so it holds for
every page in that browser without touching the account.

## Avatar Items

`/avatar` also sells hats, paid for with the coins the game server already awards for playtime: a
minute played is one coin. `GET /api/avatar` returns the catalog with `owned` and `equipped` flags
plus your balance, `POST /api/avatar/buy` charges the price, and `POST /api/avatar/equip` wears a hat
or takes it off (`{"itemId": ""}`). A hat has to be owned before it can be worn, buying one twice is
refused, and saving body colours through `POST /api/avatar` leaves the equipped hat alone.

The catalog is `include/AvatarItems.h`, so the shop, the web preview and the 3D client all draw a hat
from the same size, colour and offset. In the client the hat is a posed part that follows the head
every frame, so it moves with the walk and breathing animation. The hat travels in the player list the game server sends, so the other players in your game see it too; an id this build's catalog does not know is dropped rather than drawn.

## Script Editor

`/editor` is the website's code editor for a game's `.rbx` script - the file the game server loads
beside the world, which is what makes a game's parts move. It is for game code rather than the engine's
own sources, and it is not a second Studio: running and debugging a script still happen in the desktop
Studio, while this page opens the file a published game will run. It has a game picker, line numbers,
`Tab` indentation, `Enter` that keeps the current indentation, `Ctrl+S` (or `Cmd+S`) to save, the line
and column under the cursor, and the size limit; an unsaved buffer is marked and leaving the page with
one asks first. The creator page links straight to it, with the game already selected.

`GET /api/games/:id/script` returns the owner's script text and the limit, and
`PUT /api/games/:id/script` with `{ "text": "..." }` writes it back in place. The first save names the
file after the game (`game-<id>.rbx` in the game-worlds directory) and stores that path on the game;
later saves rewrite the same path, so the file a game server was launched with never changes under it.
A script is capped at 256 KB, both routes are owner-only (anybody else gets `404`, so a private game's
existence is not revealed through them), and a game seeded from a starter game keeps its own copy of
the script, so editing it never touches the bundled original.

## Admin Dashboard

`/admin` shows the local install at a glance: account, game, live server, launch and playtime totals,
the full user and game lists, the running game-server instances, and a Delete action that removes a
game together with its published `.world` file (stopping any instance for it first). Each account row
carries the balance it earned by playing and a Give box beside it, so an admin can hand out coins - a
prize, a refund, or making a run the server dropped good - without touching the playtime that earned
the rest. A grant is one account at a time and only ever adds, 1 to 100000 coins, and it lands in the
same balance Settings and the avatar shop read.

Admin access is by account, and the account that owns the install is always an admin. That is the
seeded `defaultuser0` account, and it stays the admin after a rename or a differently-spelled seed,
because the owner is matched by id rather than by name. `ADMIN_USERNAMES` adds other accounts to the
list; it never replaces the owner, so naming someone else there cannot lock the install's own owner
out of `/admin`. Usernames are compared case-sensitively, so the flag follows the account rather than the browser.
Non-admins get `403` from `GET /api/admin/overview` and `DELETE /api/admin/games/:id`, the Admin link on
the dashboard stays hidden for them, and the refusal names the account that is signed in and the names
that are allowed, so a mismatch is visible instead of silent.

## AI Trainer

`AiTrainer` learns to play any `.world` without opening a window. It loads the world, creates the
same `Character` and `PhysicsWorld` the game runs, and improves a small policy with the
cross-entropy method, so walk speed, jumping and falling behave exactly as they do in game.

```
AiTrainer --world games/turbo-circuit.world --goal FinishLine --generations 60
AiTrainer --world games/turbo-circuit.world --goal FinishLine --replay
AiTrainer --world games/turbo-circuit.world --goal FinishLine --replay --watch
```

The policy sees eight numbers (direction to the goal on X and Z, distance, height above the spawn,
a bias, and how far each of three feeler rays reaches before it hits a wall on `-Z`, `-X` and `+X`)
and picks one of six actions (forward, backward, left, right, jump, idle) on fixed world axes, where
forward is `-Z`. Reward is progress toward the goal part, plus a bonus for reaching it
and a penalty for falling off the map. `--goal` names the part to walk to; a name the world does not
have falls back to the part furthest from the spawn.

The trained weights are written to `models/<world>.policy` as plain text (one row per action) unless
`--out` says otherwise. `--replay` loads that file and runs one episode, exiting 0 only if the goal
was reached, which makes it usable as a check. Training is CPU only and needs no GPU.

`--watch` draws the episode as a top-down map in the terminal while it happens: `@` is the
character, `G` the goal, `S` the spawn, `.` the path already walked, `#` a wall and `_` floor. It
works with `--replay`, and during training it shows the best policy once training finishes. The view
uses ANSI colours and cursor control, so run it in a terminal rather than piping it to a file. Parts
are drawn from their axis-aligned size, so a rotated part covers a little more than it really does.

## Web API

Authentication:

- `POST /api/signup` with JSON `{ "username": "Player_1", "password": "long-password" }`
- `POST /api/login` with JSON `{ "username": "Player_1", "password": "long-password" }`
- `POST /api/verify` with `Authorization: Bearer <token>`
- `POST /api/account/password` with the current and new password
- `POST /api/account/recover` with `{ "username": "...", "recoveryCode": "...", "newPassword": "..." }`
- `POST /api/account/recovery` with the current password, to issue a fresh recovery code
- `POST /api/account/username` with the new name and the current password
- `POST /api/account/delete` with the password
- `GET /api/account/export` returns everything kept about the account as JSON
- `POST /api/blocks` with `{ "userId": 2, "blocked": true }`
- `POST /api/reports` with `{ "targetType": "game", "targetId": 1, "reason": "..." }`

Avatar:

- `GET /api/avatar` with `Authorization: Bearer <token>`
- `POST /api/avatar` with `Authorization: Bearer <token>` and JSON `{ "avatar": { ...six RGB arrays..., "faceId": "wink" } }`

Valid avatar `faceId` values are `classic`, `happy`, `surprised`, `smirk`, and `wink`. The web previews and native Client use the same transparent face PNG assets.

Profiles and friends:

- `GET /api/me/social` returns the signed-in profile, friends, and friend requests.
- `POST /api/me/playtime` with `{ "seconds": 120 }` records authenticated playtime.
- `GET /api/users/search?q=Player` searches users by username or exact user ID.
- `GET /api/users/:id` returns a public profile, avatar, stats, and friends.
- `POST /api/friends/request` with `{ "userId": 2 }` sends or accepts a friend request.
- `POST /api/friends/respond` with `{ "userId": 2, "action": "accept" }` accepts or declines an incoming request.
- `POST /api/friends/remove` with `{ "userId": 2 }` removes a friend or cancels a pending request.

Games:

- `GET /api/games` lists public games for the games page.
- `GET /api/games/:id` returns game details, world stats, and active local instances.
- `GET /api/games/:id/script` with the owner's token returns the game's `.rbx` script and the 256 KB limit.
- `PUT /api/games/:id/script` with `{ "text": "..." }` (owner only) saves the script to the file the game server loads.
- `GET /api/games/mine` with `Authorization: Bearer <token>` lists your published games.
- `POST /api/games` with `Authorization: Bearer <token>` and `{ "title": "...", "description": "...", "isPublic": true, "worldText": "..." }` publishes a world file.
- `GET /api/account/standing` returns the caller's own standing, its infraction record and its appeals; `POST /api/account/appeal` with `{ "message": "..." }` files one while the account is restricted.
- `POST /api/admin/appeals/:id` with `{ "action": "accept" }` or `{ "action": "deny" }` (admin only) answers an appeal; accepting it pardons the account.
- `POST /api/games/:id/play` with `Authorization: Bearer <token>` starts or reuses a local game server and returns native Player launch metadata.
- `GET /api/admin/overview` with an admin token returns site totals plus the user, game and instance lists.
- `DELETE /api/admin/games/:id` with an admin token deletes a game and its world file.
- `POST /api/admin/users/:id/coins` with `{ "amount": 100 }` (admin only) gives an account coins; the balance it writes is the one Settings and the avatar shop read.

Tokens are no longer accepted in avatar URLs. `/api/verify` still accepts a body token as a deprecated compatibility fallback, but new callers should use the Authorization header.

The game server reports authenticated player sessions to `/api/me/playtime` while a player is connected and once more when the player disconnects.

## Client Controls

- Right mouse: hold to look around.
- `W/A/S/D`: move, Space: jump.
- `Browse games` in the player lists public games straight from the website and joins them without a browser.
- `Shift`: toggle shift lock, which captures the mouse, keeps the character facing the camera and moves the view over the shoulder.
- `E`: get into or out of the car when you are standing next to it. While driving, `W`/`S` are throttle and brake/reverse and `A`/`D` steer.
- `Esc`: open the in-game menu with Resume, Reset Character, Settings and Leave Game; `Esc` again resumes.
- Settings holds shift lock, mouse sensitivity, invert mouse Y, field of view, chat and player list toggles, the debug overlay and a reset to defaults button.
- `F3`: toggle the debug overlay without opening the menu.

## Source Editor

`RBLXCode` is the code editor for this repository, the way VS Code is the editor for its own
sources. It is not the Studio: Studio edits a world and runs a script, this edits the C++,
shaders, website and `.rbx` scripts that make up RBLX Clone itself, and saves them in place.
Launch it from the checkout (`RBLXCode.exe` on Windows, `./RBLXCode .` elsewhere), or name the
folder to open, and it walks that folder for source files.

It has a project tree on the left, an outline of the file in front on the right, tabs across the
top (a `*` marks unsaved work), the ImGui colour text editor Studio's Script panel uses in the
middle, and a status bar under it with the line and column, the language, the line count, whether
the file is saved and the branch of the checkout. C++ and headers are coloured as C++, `.glsl`,
`.vert` and `.frag` as GLSL, `.rbx` as RBLX Script (a Lua-shaped language, so the Lua definition
is the right one), and anything the widget has no definition for opens as plain text with its
real name in the status bar instead of being coloured wrongly.

- `Ctrl+P` quick open: type an abbreviation of a path (`cmain` finds `src/CodeMain.cpp`, not the
  other files that merely contain those letters) and `Enter` opens the best match.
- `Ctrl+Shift+O` goes to a symbol in the open file. The outline panel on the right lists what the
  file declares — functions, classes, structs, enums and namespaces in C++ and GLSL, functions and
  handlers in RBLX Script and JavaScript, rules in CSS, headings in Markdown and keys in JSON —
  and a click jumps to it. Declarations without a body are left out, and the outline is rebuilt
  only when the buffer changes.
- `Ctrl+F` finds in the file in front, with match case and whole word, a `3 of 12` count and
  `Enter`/`Shift+Enter` or `F3`/`Shift+F3` to walk the matches. `Ctrl+H` adds the replace row, where
  `Replace` takes one match and `Replace all` rewrites the buffer.
- `Ctrl+Shift+F` focuses the project search: a case-insensitive substring across the project, one
  row per matching line, and clicking a row opens the file on that line.
- `Ctrl+G` goes to a line, `Ctrl+/` comments or uncomments the line under the caret (with the
  marker the language uses: `//` or `--`), `Ctrl+]` jumps to the bracket that pairs with the one at
  the caret — brackets inside strings and comments are ignored.
- `Alt+Left` and `Alt+Right` walk back and forward through the places you jumped from, and
  `Ctrl+Tab`/`Ctrl+Shift+Tab` switch tabs.
- `Ctrl+S` saves the file in front, `Ctrl+Shift+S` saves every open file, `Ctrl+R` reloads it from
  disk, `Ctrl+W` closes it. Undo, redo, cut, copy and paste belong to the text widget and work as
  usual; `Ctrl+Z` also takes back a single replace and a comment toggle.
- The View menu sets the tab width (2, 4 or 8), shows whitespace, switches to the light palette and
  hides the outline panel; the change applies to the already open files too.
- Closing a tab with unsaved changes asks whether to save it first.

A file that changes on disk while it is open is picked up on its own when the buffer has no edits.
When it does have edits the editor leaves them alone and shows a warning row with a choice between
reloading from disk and keeping what is in the buffer, instead of overwriting either side.

Line endings are preserved: a file is shown with plain newlines but saved back with the CRLF or LF
it arrived with, because this repository is not uniform (the anti-cheat sources are CRLF) and an
editor that rewrote every line would bury a one-line fix in a whole-file diff. Build trees,
`.git`, `node_modules`, vendored dependencies and the packaged `GameRelease` folder are left out
of the tree, images and models are never offered, and a file with a NUL byte or one larger than
4 MB is refused with the reason in the status bar rather than opened.

The Windows package ships it as `start-code.bat`, which opens it on the package folder (the
shaders, website and example games); the C++ sources are not in that package, so for those run
`RBLXCode.exe` from a checkout or pass the clone's path.

## Studio Controls

- Right mouse drag: look around.
- `W/A/S/D`: move camera.
- `E/Q`: move camera up/down.
- Shift: faster camera movement.
- Left click: select part.
- Toolbox: add parts, switch move/rotate/scale tools, enable snap.
- Properties: edit selected part fields, including the surface material (plastic, smooth plastic, neon, metal, wood, grass, concrete, brick) and an optional texture image from `textures/`, tiled across the part.
- Publish: sign in against the web server, save the current world, and publish it to the site as a public or private game.

## Scripting

The engine has its own scripting language, **RBLX Script**, a Lua-style subset used by Studio,
the game server and the seeded platformer. Scripts drive gameplay by moving parts and
reacting to events.

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

The language supports `local`, assignment, `if`/`elseif`/`else`, `while`, `repeat ... until`,
numeric `for`, `break`, named and anonymous `function`s with closures, tables (`{1, 2}`,
`t.field`, `t[1]`), strings, `..` concatenation, arithmetic/comparison/logic and `--`
comments. Builtins are `print`, `warn`, `type`, `tostring`, `tonumber`, `error` and `math`
(`sin`, `cos`, `abs`, `floor`, `ceil`, `sqrt`, `min`, `max`, `random`, `pi`).

World API available to scripts:

- `workspace:FindFirstChild("Name")` and `workspace.Name`.
- `workspace:CreatePart(name, position, size, color)`, which builds a part the world file never had, anchors it, gives it a physics body and returns it so the script can move and listen to it. The Doors Hotel builds its rooms with it as the player walks and takes them away again with `part:Destroy()`; a world is still capped at 4096 parts, and both of those reach the clients as their own packets because the join snapshot only describes the world as it was at join time. A part that is only repainted is announced again the same way: a colour is not a position, so the world update that carries movement would never show it.
- Part properties: `Name`, `Position`, `Size`, `Color`, `Transparency`, `Anchored`, `CanCollide`.
- `part:Teleport(vector)` and `part:Destroy()`.
- `part.Touched:Connect(function(hit) ... end)`.
- `RunService.Heartbeat:Connect(function(dt) ... end)` for per-tick logic.
- `Players:GetPlayers()`, `Players.PlayerCount`, `player.Name`, `player:Teleport(vector)` and `player:TakeDamage(amount)`, which the server applies and answers with a death and respawn.
- `Vector3.new(x, y, z)`.

Run a script against a world with `Server.exe --world games/sky-platformer.world --script games/sky-platformer.rbx`.
Studio has a **Script** panel for loading, editing, saving, running and stopping scripts in a VS Code-style editor with line numbers, syntax highlighting, undo and a cursor status bar; the
world view updates live while a script runs. The seeded **Sky Platformer** game uses
`games/sky-platformer.world` and `games/sky-platformer.rbx`, and published starter games get
their script copied next to the world file and passed to the server with `--script`. The seeded
**Turbo Circuit** driving game works the same way with `games/turbo-circuit.world` and
`games/turbo-circuit.rbx`, and **Doors Hotel** is the same shape again: `games/doors-hotel.world`
with `games/doors-hotel.rbx`, where the script builds the corridor, the hundred rooms and their
doors with `workspace:CreatePart` and then owns the whole game - the sweep, the damage check, the
hiding rule and the doors are script state rather than engine features.

Starter games are checked on every start, not just on an empty database: any starter game the
data file is missing is published then, so an install created before a game existed picks it up
after restarting the website. Starter games that are already present are never duplicated. The
starter assets are read from the repo root during development and from next to `WebApp.exe` in a
packaged build (`games/`, `web/seed-worlds/`, `ServerWorld.world`).

## Multi-Game Local Hosting

The website stores published `.world` files under `web/game-worlds` by default, along with any `.rbx` script that belongs to the game. When a signed-in user presses Play on a game page, the web backend finds an open local port, starts `Server.exe --port <port> --world <published-world> --script <published-script> --web <site-url>`, and can launch `Client.exe --server <host> --port <port> --web <site-url> --token <jwt> --connect` when `CLIENT_EXECUTABLE_PATH` points to a built Player.

Play waits for the server to own its port before launching the client, and each game server
writes its output to `web/game-server-logs/game-server-<port>.log`. When a server dies during
startup, for example because its port is still held by a leftover `Server.exe`, the game page
shows that output and the log path instead of a bare connection error.

Useful launch environment variables:

- `GAME_WORLDS_DIR` - directory for published world files.
- `GAME_SERVER_BASE_PORT` - first port to try for game instances, default `7777`.
- `SERVER_EXECUTABLE_PATH` - local game server executable.
- `CLIENT_EXECUTABLE_PATH` - local Player executable.
- `PUBLIC_BASE_URL` - URL passed to native server/client for web auth, default `http://localhost:3000`.

Scale operations assign the decomposed absolute scale from ImGuizmo and clamp it to a safe positive range. They do not multiply scale every frame.

## Known Limitations

- This is a prototype and should be treated as local-only.
- The Linux build of Client and Server compiles and passes its tests in CI, but nobody has played a match on it yet; the world file picker exists only on Windows (elsewhere pass the `.world` path on the command line).
- The driving game's car is a kinematic arcade body: it accelerates, brakes, coasts, steers and stops at collidable parts, but there is no client-side prediction, so steering over a high-latency connection feels delayed. Collision uses the axis-aligned box around a rotated part, so a diagonal wall blocks a little more ground than it looks like it does.
- The HTTP client speaks plain `http://` only. There is no TLS implementation, so an `https://` backend URL fails with an explanation instead of working.
- The binary protocol depends on matching C++ struct layouts, so Client and Server must be built from the same source. The handshake now carries a protocol revision and a layout signature derived from the packet sizes; a mismatched pair refuses the join and prints which build to rebuild instead of misreading packets.
- The web service keeps its data in a JSON file and sessions in browser storage; it is not hardened for internet deployment. Every response carries `X-Content-Type-Options: nosniff`, `X-Frame-Options: DENY`, `Referrer-Policy: same-origin` and a strict `Content-Security-Policy`, but that is baseline browser hygiene, not a security review.
- Account recovery depends on a recovery code the player saved, not on email, and presence is "an authenticated request in the last two minutes" rather than a game-server connection. Blocks and reports stop at the website: the native game server is not told about either.
- The client is split by concern: `src/Client.cpp` (startup, argument handling, main loop), `src/ClientNet.cpp` (game socket, reliable session, joining and leaving) and `src/ClientUi.cpp` (menus, HUD, chat, settings), sharing state through `src/ClientShared.h`. The remaining files could be split further.
- C++ build/test is verified with the vendored RBLXBuild using Ninja, MinGW GCC and GCC on Linux. Client, Server, and Studio runtime behavior still need manual testing on a graphics-capable desktop.

## Troubleshooting

- `rblxbuild is not recognized`: bootstrap the vendored tool first (see Build C++).
- `cl not found`: run from a Visual Studio Developer shell or install the C++ workload.
- `JWT_SECRET must be set`: set a strong secret in production or use development mode locally.
- Client login fails: start the web backend first and confirm `http://localhost:3000` is reachable.
- Client cannot connect: start `Server.exe`, allow firewall access, and connect to `127.0.0.1` for local testing.
- Play reports that the game server did not start: read `web/game-server-logs/game-server-<port>.log`,
  which the error message also quotes. A leftover `Server.exe` holding the port is the usual cause;
  close it and press Play again.
