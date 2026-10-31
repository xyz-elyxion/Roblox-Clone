# RBLX Clone

RBLX Clone is an early prototype of a Roblox-style client, game server, Studio editor, and web authentication/avatar service. It is usable for local experimentation, but it is not production-ready.

## Current Status

- Client, Server, and Studio are C++17 targets.
- Client and Server build on Windows and Linux. Socket calls go through `include/Network.h` and website calls through `include/HttpClient.h`, each of which has a WinSock/WinHTTP-facing Windows branch and a POSIX branch, so the game code itself has no platform split. The anti-cheat library is Windows-only and is skipped on other platforms.
- Client.exe embeds **RBLXAntiCheat**, a usermode anti-cheat compiled into the client process (vendored from [UltimateAntiCheat](https://github.com/AlSch092/UltimateAntiCheat) and renamed, GNU AGPL v3 — see `RBLXAntiCheat/LICENSE`). It starts with the client, writes `RBLXAntiCheat.log`, and a failed start is logged instead of stopping the game. Configure with `-DRBLX_BUILD_ANTICHEAT=OFF` to leave it out of the build. Its identifying strings (log name, startup messages, accepted parent processes) are stored XOR-masked at compile time through `include/Obfuscated.h` so they do not show up in `strings`, and MSVC Release builds link the client with control-flow guard, full ASLR and identical-code folding. That raises the cost of finding and patching the checks; it is not a security boundary, which is why the rules that matter (movement, world contents) are enforced by the server.
- Health and respawning are decided by the game server, not the client. The server keeps each player's health, kills anyone who falls below the world or takes damage, and returns them to the world's `SpawnLocation` after a three-second countdown that every client sees. The client draws the health bar and a "You died" overlay and stops taking movement input while dead; `Reset Character` asks the server rather than respawning locally. Scripts damage players with `player:TakeDamage(amount)`.
- **Turbo Circuit** is the bundled driving game. A world part named `Car` is the vehicle: the game server integrates it from client throttle and steer input (`include/Vehicle.h`) and broadcasts its position, so a modified client cannot teleport the car any more than a character. Walk up to it and press `E` to get in or out; the driver's character rides in the car, the camera follows behind and the HUD shows km/h. A `.rbx` script counts laps at the finish line.
- Studio uses GLFW/OpenGL/ImGui/ImGuizmo and has a basic world editing path.
- The web backend is `WebApp.exe`, a native C++ service (`src/WebApp.cpp`) that serves `web/public` and owns accounts (PBKDF2-SHA256), JWT sessions, avatars, friends, the public game catalog, and local game-server launch orchestration. There is no Node.js service any more.
- Networking is a compact binary protocol carried over UDP. Positions, rotations, and world updates travel as plain datagrams, so a lost update costs one frame instead of stalling the connection; the join handshake, world snapshot, chat, and join/leave events ride a reliability layer with acknowledgements, retransmission, and fragmentation (`include/UdpTransport.h`). It is still a prototype protocol.

## Repository Layout

- `CMakeLists.txt` - C++ build configuration.
- `CMakePresets.json` - Visual Studio debug and release presets for Windows.
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

Client, Server and Studio build on Windows and Linux. Sockets go through `include/Network.h` and website calls through `include/HttpClient.h`, each with a WinSock branch and a POSIX branch, so the game code itself has no platform split.

On Windows:

- Visual Studio 2022 with "Desktop development with C++".
- CMake 3.21 or newer if using `CMakePresets.json`; CMake 3.14 or newer for manual configure.
- OpenGL-capable GPU/drivers.

On Linux:

- GCC 11+ or Clang 14+ (anything with C++17), CMake 3.14+, Ninja or Make.
- `build-essential cmake ninja-build pkg-config` plus the GLFW development packages: `libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libwayland-dev libxkbcommon-dev`.
- OpenGL-capable GPU/drivers.

RBLXAntiCheat is Windows-only and is skipped automatically on other platforms; `-DRBLX_BUILD_ANTICHEAT=OFF` leaves it out on Windows too.

### Web Backend

`WebApp.exe` is built from `src/WebApp.cpp` with the rest of the C++ targets and needs nothing
beyond them: no Node.js, npm or SQLite.

## Build C++

From a Developer PowerShell or Developer Command Prompt:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

Manual configure also works:

```powershell
cmake -S . -B build/windows-debug -G "Visual Studio 17 2022" -DRBLX_BUILD_TESTS=ON
cmake --build build/windows-debug --config Debug
ctest --test-dir build/windows-debug -C Debug
```

The project has also been verified with a portable MinGW/Ninja toolchain:

```powershell
cmake -S . -B build/ninja -G Ninja -DCMAKE_C_COMPILER=C:\path\to\gcc.exe -DCMAKE_CXX_COMPILER=C:\path\to\g++.exe -DRBLX_BUILD_TESTS=ON
cmake --build build/ninja
ctest --test-dir build/ninja --output-on-failure
```

On Linux:

```bash
cmake -S . -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --parallel
ctest --test-dir build/linux --output-on-failure
```

Useful CMake options:

- `RBLX_BUILD_CLIENT=ON|OFF`
- `RBLX_BUILD_SERVER=ON|OFF`
- `RBLX_BUILD_STUDIO=ON|OFF`
- `RBLX_BUILD_TESTS=ON|OFF`
- `RBLX_BUILD_ANTICHEAT=ON|OFF`
- `RBLX_ENABLE_WARNINGS=ON|OFF`

Windows builds also compile the vendored RBLXAntiCheat into the client (see `RBLXAntiCheat/`); on other platforms that target is skipped.

## Run C++ Targets

After building Debug with the preset:

```powershell
.\build\windows-debug\Debug\Studio.exe
.\build\windows-debug\Debug\Server.exe
.\build\windows-debug\Debug\Client.exe
.\build\windows-debug\Debug\WebApp.exe
```

Start the web backend before using authenticated Client login.

## Prebuilt Windows Package

The `Build` workflow runs on every push and packages both configurations into folders you
can unzip and run. Open the latest run under **Actions** and download `rblx-windows-release`
(or `rblx-windows-debug` for the assertion-enabled build with `.pdb` files):

```
rblx-windows-release/
  WebApp.exe  Server.exe  Client.exe  Studio.exe
  web/public/  web/seed-worlds/
  shaders/  ServerWorld.world  face*.png  face_hex.txt
  start-website.bat  start-studio.bat  start-client.bat
  README.txt
```

Double-click `start-website.bat`: it starts `WebApp.exe` from that folder, points it at the
`Server.exe` and `Client.exe` beside it, and opens <http://localhost:3000> in your browser.
Pressing Play on a game page then launches the client from the same folder.

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
.\build\windows-debug\Debug\WebApp.exe
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
- `GAME_WORLDS_DIR` - where published `.world` files are written.
- `GAME_SERVER_HOST` / `GAME_SERVER_BASE_PORT` - local game-server address, default `127.0.0.1:7777`.
- `GAME_INSTANCE_EMPTY_GRACE_MS` - how long an empty game instance stays up, default `30000`.
- `SERVER_EXECUTABLE_PATH` / `CLIENT_EXECUTABLE_PATH` - binaries launched when a game is played.
- `PUBLIC_BASE_URL` - URL handed to the native server/client for web auth.
- `ADMIN_USERNAMES` - comma-separated accounts allowed into `/admin`, default `RBLXCreator`.

Passwords are stored as PBKDF2-SHA256 hashes and sessions are HS256 JWTs.

## Admin Dashboard

`/admin` shows the local install at a glance: account, game, live server, launch and playtime totals,
the full user and game lists, the running game-server instances, and a Delete action that removes a
game together with its published `.world` file (stopping any instance for it first).

Admin access is by username: set `ADMIN_USERNAMES` to a comma-separated list, or leave it unset to
make the seeded `RBLXCreator` account the only admin. The list is matched case-sensitively against the
signed-in account, so the flag follows the account rather than the browser. Non-admins get `403` from
`GET /api/admin/overview` and `DELETE /api/admin/games/:id`, and the Admin link on the dashboard stays
hidden for them.

## Web API

Authentication:

- `POST /api/signup` with JSON `{ "username": "Player_1", "password": "long-password" }`
- `POST /api/login` with JSON `{ "username": "Player_1", "password": "long-password" }`
- `POST /api/verify` with `Authorization: Bearer <token>`

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
- `GET /api/games/mine` with `Authorization: Bearer <token>` lists your published games.
- `POST /api/games` with `Authorization: Bearer <token>` and `{ "title": "...", "description": "...", "isPublic": true, "worldText": "..." }` publishes a world file.
- `POST /api/games/:id/play` with `Authorization: Bearer <token>` starts or reuses a local game server and returns native Player launch metadata.
- `GET /api/admin/overview` with an admin token returns site totals plus the user, game and instance lists.
- `DELETE /api/admin/games/:id` with an admin token deletes a game and its world file.

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
- Part properties: `Name`, `Position`, `Size`, `Color`, `Transparency`, `Anchored`, `CanCollide`.
- `part:Teleport(vector)` and `part:Destroy()`.
- `part.Touched:Connect(function(hit) ... end)`.
- `RunService.Heartbeat:Connect(function(dt) ... end)` for per-tick logic.
- `Players:GetPlayers()`, `Players.PlayerCount`, `player.Name`, `player:Teleport(vector)` and `player:TakeDamage(amount)`, which the server applies and answers with a death and respawn.
- `Vector3.new(x, y, z)`.

Run a script against a world with `Server.exe --world games/sky-platformer.world --script games/sky-platformer.rbx`.
Studio has a **Script** panel for loading, editing, saving, running and stopping scripts; the
world view updates live while a script runs. The seeded **Sky Platformer** game uses
`games/sky-platformer.world` and `games/sky-platformer.rbx`, and published starter games get
their script copied next to the world file and passed to the server with `--script`. The seeded
**Turbo Circuit** driving game works the same way with `games/turbo-circuit.world` and
`games/turbo-circuit.rbx`.

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
- The driving game's car is a kinematic arcade body: it accelerates, brakes, coasts and steers, but it does not collide with world geometry, so it will drive through a wall, and there is no client-side prediction, so steering over a high-latency connection feels delayed.
- The HTTP client speaks plain `http://` only. There is no TLS implementation, so an `https://` backend URL fails with an explanation instead of working.
- The binary protocol depends on matching C++ struct layouts, so Client and Server must be built from the same source. The handshake now carries a protocol revision and a layout signature derived from the packet sizes; a mismatched pair refuses the join and prints which build to rebuild instead of misreading packets.
- The web service keeps its data in a JSON file and sessions in browser storage; it is not hardened for internet deployment. Every response now carries `X-Content-Type-Options: nosniff`, `X-Frame-Options: DENY` and `Referrer-Policy: same-origin`, but that is baseline browser hygiene, not a security review.
- The client is split by concern: `src/Client.cpp` (startup, argument handling, main loop), `src/ClientNet.cpp` (game socket, reliable session, joining and leaving) and `src/ClientUi.cpp` (menus, HUD, chat, settings), sharing state through `src/ClientShared.h`. The remaining files could be split further.
- C++ build/test is verified with CMake, Ninja, MinGW GCC and GCC on Linux. Client, Server, and Studio runtime behavior still need manual testing on a graphics-capable desktop.

## Troubleshooting

- `cmake is not recognized`: install CMake or add it to PATH.
- `cl not found`: run from a Visual Studio Developer shell or install the C++ workload.
- `JWT_SECRET must be set`: set a strong secret in production or use development mode locally.
- Client login fails: start the web backend first and confirm `http://localhost:3000` is reachable.
- Client cannot connect: start `Server.exe`, allow firewall access, and connect to `127.0.0.1` for local testing.
- Play reports that the game server did not start: read `web/game-server-logs/game-server-<port>.log`,
  which the error message also quotes. A leftover `Server.exe` holding the port is the usual cause;
  close it and press Play again.
