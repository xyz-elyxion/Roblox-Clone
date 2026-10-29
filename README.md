# RBLX Clone

RBLX Clone is an early prototype of a Roblox-style client, game server, Studio editor, and web authentication/avatar service. It is usable for local experimentation, but it is not production-ready.

## Current Status

- Client, Server, and Studio are C++17 targets.
- The Client and Server currently require Windows because they use WinSock and WinHTTP.
- Studio uses GLFW/OpenGL/ImGui/ImGuizmo and has a basic world editing path.
- The web backend is a Node.js/Express service with SQLite, bcrypt password hashing, JWT authentication, avatar storage, public game catalog pages, and local game-server launch orchestration.
- Networking is still a compact binary protocol. It now has shared framing, payload limits, partial send handling, and receive buffering, but it is still a prototype protocol.

## Repository Layout

- `CMakeLists.txt` - C++ build configuration.
- `CMakePresets.json` - Visual Studio debug and release presets for Windows.
- `scripts/package-build.ps1` - Assembles a runnable folder from a build tree.
- `include/` - C++ headers.
- `src/` - C++ Client, Server, Studio, rendering, physics, auth, and world loading code.
- `shaders/` - OpenGL shader files used by Client and Studio.
- `tests/` - C++ protocol tests.
- `web/` - Express backend and public HTML pages.
- `ServerWorld.world` - Default world file.
- `GameRelease/` - Existing release asset folders. Do not commit generated binaries here.

## Prerequisites

### C++ Targets

Windows is currently required for Client and Server.

- Visual Studio 2022 with "Desktop development with C++".
- CMake 3.21 or newer if using `CMakePresets.json`; CMake 3.14 or newer for manual configure.
- OpenGL-capable GPU/drivers.

### Web Backend

- Node.js 20 or newer. The project was tested with Node.js 24.
- npm.

PowerShell may block `npm.ps1` on some machines. Use `npm.cmd` if that happens.

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

Useful CMake options:

- `RBLX_BUILD_CLIENT=ON|OFF`
- `RBLX_BUILD_SERVER=ON|OFF`
- `RBLX_BUILD_STUDIO=ON|OFF`
- `RBLX_BUILD_TESTS=ON|OFF`
- `RBLX_ENABLE_WARNINGS=ON|OFF`

On non-Windows hosts, CMake disables Client and Server because those targets depend on WinSock/WinHTTP.

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
`web/public` from its working directory. Configuration comes from the environment:

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

Passwords are stored as PBKDF2-SHA256 hashes and sessions are HS256 JWTs. The old `web/users.db`
SQLite file and its bcrypt hashes are not read by this backend.

## Admin Dashboard

`/admin` shows the local install at a glance: account, game, live server, launch and playtime totals,
the full user and game lists, the running game-server instances, and a Delete action that removes a
game together with its published `.world` file (stopping any instance for it first).

Admin access is by username: set `ADMIN_USERNAMES` to a comma-separated list, or leave it unset to
make the seeded `RBLXCreator` account the only admin. The list is matched case-sensitively against the
signed-in account, so the flag follows the account rather than the browser. Non-admins get `403` from
`GET /api/admin/overview` and `DELETE /api/admin/games/:id`, and the Admin link on the dashboard stays
hidden for them.

## Legacy Node.js Backend

`web/server.js` is the original Express implementation. It is no longer required, but still runs:

```powershell
cd web
npm.cmd install
npm.cmd test
npm.cmd start
```

The backend listens on `http://localhost:3000` by default.

Copy `web/.env.example` to `web/.env` for local overrides. Do not commit `.env`.

Environment variables:

- `NODE_ENV` - `development`, `test`, or `production`.
- `PORT` - HTTP port, default `3000`.
- `DATABASE_PATH` - SQLite database path, default `web/users.db`.
- `JWT_SECRET` - required and must be strong in production.
- `JWT_EXPIRES_IN` - JWT lifetime, default `7d`.
- `CORS_ORIGIN` - comma-separated allowed origins. Development defaults to localhost origins.
- `JSON_BODY_LIMIT` - Express JSON body limit, default `2mb`.
- `AUTH_RATE_LIMIT_WINDOW_MS` - auth rate limit window.
- `AUTH_RATE_LIMIT_MAX` - auth requests allowed per window.

Production startup fails if `JWT_SECRET` is missing, too short, or left as an unsafe placeholder.

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
- Properties: edit selected part fields.
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
- `Players:GetPlayers()`, `Players.PlayerCount`, `player.Name` and `player:Teleport(vector)`.
- `Vector3.new(x, y, z)`.

Run a script against a world with `Server.exe --world games/sky-platformer.world --script games/sky-platformer.rbx`.
Studio has a **Script** panel for loading, editing, saving, running and stopping scripts; the
world view updates live while a script runs. The seeded **Sky Platformer** game uses
`games/sky-platformer.world` and `games/sky-platformer.rbx`, and published starter games get
their script copied next to the world file and passed to the server with `--script`.

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
- Client and Server are Windows-only until socket and HTTP abstractions are made portable.
- The binary protocol still depends on matching C++ struct layouts across the same build family.
- The web service uses SQLite and local JWT storage in the browser; it is not hardened for internet deployment.
- Client UI/networking code remains large and should be split further.
- C++ build/test is verified with CMake, Ninja, and MinGW GCC. Client, Server, and Studio runtime behavior still need manual testing with a graphics-capable Windows desktop.

## Troubleshooting

- `cmake is not recognized`: install CMake or add it to PATH.
- `cl not found`: run from a Visual Studio Developer shell or install the C++ workload.
- `npm.ps1 cannot be loaded`: use `npm.cmd` from PowerShell.
- `JWT_SECRET must be set`: set a strong secret in production or use development mode locally.
- Client login fails: start the web backend first and confirm `http://localhost:3000` is reachable.
- Client cannot connect: start `Server.exe`, allow firewall access, and connect to `127.0.0.1` for local testing.
- Play reports that the game server did not start: read `web/game-server-logs/game-server-<port>.log`,
  which the error message also quotes. A leftover `Server.exe` holding the port is the usual cause;
  close it and press Play again.
