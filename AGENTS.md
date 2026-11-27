# Working in this repository

Linear is a Windows-only sandbox-platform prototype in C++17: a game client, a game
server, a world editor (Studio), a source editor (LinearCode), a browser
(`LinearBrowser.exe`), a native website backend (`WebApp.exe`), an installer, an updater, a
headless trainer and a server supervisor (`LinearHosts.exe`). Everything — including the
website — is C++ built from this tree, and the build tool itself is vendored. There is no
Node.js, npm, Python, CMake install or package manager involved.

This file is the short version for anyone (person or agent) making a change. The
[README](README.md) is the reference for what the project is; [HOSTING.md](HOSTING.md) and
[LINEARBUILD.md](LINEARBUILD.md) cover hosting and the build tool; [SECURITY.md](SECURITY.md)
covers the security model.

## Build and test

The build tool is bootstrapped from `linearbuild/` once:

```powershell
mkdir -p build/linearbuild-tool
cd build/linearbuild-tool
../../linearbuild/bootstrap --parallel 4
cd ../..
```

Then, from the repository root:

```powershell
build\linearbuild-tool\bin\linearbuild --preset windows-release
build\linearbuild-tool\bin\linearbuild --build --preset windows-release
build\linearbuild-tool\bin\lineartest --preset windows-release
```

`LinearBuildPresets.json` has one preset, `windows-release` (Visual Studio 17 2022,
`build/windows-release`, Release). Binaries land in `build\windows-release\Release\`, and so
does `LinearGame.dll`, which the client, server, Studio and trainer load from beside themselves.
Visual Studio 2022 with the C++ workload, or MinGW + Ninja, is required; the vendored
`linearbuild` is the only build tool.

The release folder is assembled by:

```powershell
./scripts/package-build.ps1 -Configuration Release
```

It copies a fixed list of executables, refuses to package a folder that is missing any of
them, and writes `README.txt`, `update.json` and `manifest.json` into `dist/linear-windows-release`.

## Rules that are not negotiable

1. **Windows only.** There is no POSIX branch: sockets go through `include/Network.h`
   (WinSock), HTTP through `include/HttpClient.h` (plain sockets, plus the operating
   system's SChannel for `https://` — that is why the targets that compile it link
   `secur32` and `crypt32`; there is still no TLS library in the tree). Do not reintroduce
   `#ifdef _WIN32` forks or a "portable" path; the project was deliberately cut down to one
   platform.
2. **No new dependencies.** Do not add a dependency, a package manager, a submodule or a
   generated blob. The eight library trees at the top of the repository (`linearwin`,
   `linearmath`, `linearui`, `lineargizmo`, `lineartextedit`, `linearphysics`, `linearstb`, `lineargl`),
   `linearbuild/` and `LinearAntiCheat/` are vendored snapshots at pinned versions, renamed into
   this project's scheme. Do not reformat or "clean up" them, keep their licences
   (`LinearAntiCheat/LICENSE`, `linearbuild/Licenses/` and the `LICENSE`/`copying.txt` in each
   tree) intact, and do not rename identifiers piecemeal: a change there is a fork-wide
   rename or nothing, because the trees must stay internally consistent. The one
   exception is a defect the compiler is right about: those have been corrected in
   place rather than silenced - the 64-bit length counter in
   `LinearAntiCheat/Common/SHA256.{hpp,cpp}`, the missing return in
   `linearphysics/.../containers/Deque.h`, the unsigned `INVALID_INDEX` in its
   `Map.h`/`Set.h`, `toLocalTime` in `DefaultLogger.h`, the safe CRT calls in
   `linearui`, the signed sentinels in `lineargizmo/src/LinearGizmo.cpp`, and the narrowing
   casts in the anti-cheat's `Detections.cpp` and `Process.cpp`; the winsock2.h-before-windows.h
   order in `linearbuild`'s Windows units and the headers they pull in, which MinGW warns about
   because those files reach windows.h before libuv includes winsock2.h; the TLS callback in
   `Core/AntiCheatLib.cpp`, which scanned threads and logged through the C runtime while the
   loader still held its lock and before the static runtime existed, hanging the client
   before `main()` on an empty console; and the module-trust rule in `Core/Detections.cpp`, which
   accepts a DLL signed by the same publisher as the running executable so `LinearGame.dll` is not
   reported as an injection; and the process mitigations in `Core/Preventions.cpp`, which
   applied `DisallowWin32kSystemCalls` to a process that draws a window, after which the
   window was created and never shown; and the hardware-debugger scan in
   `AntiDebug/AntiDebugger.cpp`, which logged through the logger's mutex while every other
   thread in the process was suspended, freezing the client mid-line with its window white,
   and read the leftover Dr6/Dr7 values as proof of a debugger; and the DLL-load notification in
   `Core/Detections.cpp`, which logged from the loader callback while the loader lock was held, so a
   thread waiting on the logger's mutex inside an anti-virus file hook stopped every later DLL load
   and left the client unkillable; and the module-signature pass beside it, which re-verified modules
   already judged and so fed its own signature lookups (and the AV provider DLLs they load) back into
   the queue as new modules; and the same file's publisher lookup, which used
   `GetModuleFileNameW` after `RandomizeModuleName` had rewritten the loader entry, so it read
   the bare randomized name instead of the image path and could never match the game's own
   publisher; and the `logfw` calls that passed a narrow `std::string` to a wide `%s` in
   `Core/Detections.cpp` and `AntiTamper/Integrity.cpp`; and the periodic integrity check in
   `AntiTamper/Integrity.cpp`, which threw from its worker thread when the (runtime-renamed)
   module name did not resolve, so `std::terminate` closed the game with no message at all; and the
   open-handle check in `Core/Detections.cpp`, which counted the launcher that started the client
   as an intrusion even though `WebApp.exe` holds that handle by design; and the enumeration behind
   it in `Process/Handles.cpp`, which did that for every handle on the machine, one `OpenProcess`
   per entry, every five seconds, and which read the system handle table through its oldest class,
   where the process id and the handle value are 16 bits each, so the value it handed to
   `DuplicateHandle` had been truncated and it asked each process about a handle that process did
   not have; and the strict handle check in `Core/Preventions.cpp`, which makes every invalid
   handle reference in the process a fatal error - not an exception, so no handler runs and
   nothing is written to a log - which is what closed the client a second into a match, leaving
   `crash.txt` and both logs empty; and the certificate check in `AntiTamper/NAuthenticode.cpp`,
   which filed a cheat detection, with no file name in it, for any module whose chain Windows would
   not vouch for - which is every module of a self-signed build on a machine where `LinearCert` has
   not run, and every one of them again once the signing certificate passes its date - instead of
   leaving the judgement to the caller that knows the module's path and its publisher.
   Keep those fixes;
   they are corrections, not a reformat. Their generated
   documentation, examples, own test suites and CI files have been pruned; the code, build
   files and licences are upstream as pinned.
3. **One palette, in `include/UiTheme.h`.** Every window — client, Studio, LinearCode, the
   server console, the installer, the updater — reads its colours from there. There are
   two palettes, dark and light, and which one is used is the `theme` key in
   `%APPDATA%\Linear\ui.settings` (`LINEAR_THEME` overrides it for one process,
   `LINEAR_UI_SETTINGS` moves the file). Never hard-code a colour in a window: add a named
   role to `Colors` and give it a value in both palettes.
4. **No launcher scripts.** There are no `.bat`/`.cmd` files in the repository or in the
   package, on purpose: every program finds its data in the folder it runs from, which is
   where Explorer starts it. Do not add a wrapper to "help".
5. **Nothing ships unless the packager knows about it.** A new executable or DLL must be
   added to the file list in `scripts/package-build.ps1`, to the folder listing in
   `README.md` and to the generated `README.txt` inside that script.
6. **Security decisions belong in `SECURITY.md`.** Anything that touches authentication,
   tokens, the data file, rate limits or moderation is described there; keep it true when
   you change the behaviour.

## Where things live

| Area | Files |
| --- | --- |
| Shared game simulation (`LinearGame.dll`) | `src/PhysicsWorld.cpp`, `src/Character.cpp`, `src/Avatar.cpp`, `src/WorldLoader.cpp`, `src/Camera.cpp`, `src/Raycaster.cpp`; the export boundary is `include/LinearGameApi.h` |
| Game client | `src/Client.cpp` (startup, arguments, loop), `src/ClientNet.cpp` (socket, session, join), `src/ClientUi.cpp` (menus, HUD, chat, settings), `src/ClientShared.h` |
| Game server | `src/Server.cpp`, `include/MovementGuard.h`, `include/Moderation.h`, `src/Player.cpp` |
| Studio | `src/Editor.cpp`, `src/WorldLoader.cpp`, `src/WorldScript.cpp` |
| LinearCode | `src/CodeMain.cpp`, `src/CodeProject.cpp` (tree, search, outline, docs), `src/CodeBuild.cpp` + `src/CodeBuildRun.cpp` (the build panel), `src/CodeGit.cpp` (the Source control panel) |
| Website backend | `src/WebApp.cpp`, `src/WebHttp.cpp`, `src/WebJson.cpp`, `src/WebStore.cpp`, `src/WebCrypto.cpp`, `src/WebCaptcha.h` |
| Website pages | `web/public` (plain HTML/CSS/JS, no build step), `web/seed-worlds` |
| Server supervisor | `src/ServerHost.cpp`, `src/HostMain.cpp`, `include/ServerHost.h` |
| Supervisor control panel | `src/HostDashboard.cpp`, `include/HostDashboard.h`, `web/host/` |
| Installer / updater | `src/SetupMain.cpp`, `src/InstallUi.cpp`, `src/UpdaterMain.cpp`, `src/Updater.cpp` |
| The mark | `include/LogoMark.h` draws it in every window; `web/public/assets/linear-mark.svg` is the site's copy and the top bar's; `scripts/make-logo.py` rasterises it into `src/app.ico` and `web/public/favicon.ico`; `src/linear.rc` is the resource the executables load it from. All three hold the same geometry, so a change goes in all three or in none |
| Certificate trust | `src/CertMain.cpp`, `src/CertificateTrust.cpp`, `scripts/make-certificate.ps1`, `scripts/sign-binaries.ps1` |
| Documentation window | `src/DocsMain.cpp`, `src/DocsLibrary.cpp` (finding and reading `docs/`), `include/DocsLayout.h` + `src/DocsLayout.cpp` (Markdown and wrapping, no window) |
| Browser | `src/BrowserMain.cpp` (toolbar, page, footer, link hit-testing), `include/BrowserPage.h` + `src/BrowserPage.cpp` (HTML reader, link resolution, address normalising and history; no window) |
| Server setup | `src/ServerSetupMain.cpp`, `src/ServerConfig.cpp` (the `server.conf` the website reads) |
| Trainer | `src/AiTrainer.cpp` |
| Scripting | `src/LinearScript.cpp`, `include/LinearScript.h` |
| Anti-cheat integration | `include/ClientCheatReport.h`, `include/SystemSnapshot.h` + `src/SystemSnapshot.cpp` (what a detection carries to staff), `include/Obfuscated.h`, `LinearAntiCheat/` |
| Guard service (anti-cheat outside the client) | `src/GuardService.cpp` (service manager, cycle), `include/GuardChecks.h` + `src/GuardChecks.cpp` (the judgements, testable without Windows), `include/GuardStatus.h` + `src/GuardStatus.cpp` (log and heartbeat paths the client reads) |
| Build | `LinearBuildLists.txt`, `LinearBuildPresets.json`, `scripts/package-build.ps1` |
| Vendored libraries | `linearwin/`, `linearmath/`, `linearui/`, `lineargizmo/`, `lineartextedit/`, `linearphysics/`, `linearstb/`, `lineargl/` |
| Tests | `tests/<area>_tests.cpp` |

`include/` mirrors `src/`: a unit lives in a header there so Studio, the client, the
server and the tests can share it.

## Adding things

- **A source file** goes into the right `add_executable` in `LinearBuildLists.txt`. Sources
  are listed explicitly per target — there is no globbing, so a new file that is not
  listed simply does not compile into anything. A game-simulation unit belongs in
  `GAME_SOURCES` (the DLL); anything that draws belongs in `SHARED_SOURCES` (each
  executable), because it needs that executable's GL context.
- **A test** is `tests/<area>_tests.cpp`: a plain `main()` that exercises the unit, prints
  what failed and returns nonzero. There is no test framework and none should be added.
  Register it with `add_executable` + `add_test` inside the `if(LINEAR_BUILD_TESTS)` block
  of `LinearBuildLists.txt`. Tests link the source file they cover directly (or a small
  library target), not the whole game.
- **Configuration** is an environment variable read with `parsePositiveInt` / `envString`
  in `src/WebApp.cpp`, with a default in the same place and a row in the README's
  configuration table. Never read `getenv` at a call site: `include/Environment.h` has
  `linear::environment::get`, which asks Windows rather than the CRT, because MSVC
  deprecates `getenv` as C4996.
- **A feature users can see** gets prose in `README.md` in the relevant section, not a
  changelog entry. Longer topics get their own file (`HOSTING.md`, `LINEARBUILD.md`) linked
  from the README.

## Style

- C++17, four-space indent, roughly 100 columns, `#pragma once` in headers, namespaces
  under `linear::`.
- Failures are returned (`bool` plus `std::string* error`) rather than thrown; the console
  programs print the error and return nonzero. Keep new code in that shape.
- Comments explain *why* a thing is the way it is, especially where a rule looks odd
  (the packaged-build README, the anti-cheat exceptions, the moderation wave). Do not
  add comments that restate the line below them.
- The website has no build step and no framework: `web/public` is served as it sits, so
  a change there is HTML/CSS/JS plus whatever route and JSON it talks to in `src/WebApp.cpp`.
- No formatter or linter is configured. Match the surrounding file and keep the existing
  compiler warning set (`LINEAR_ENABLE_WARNINGS`) clean.

## Before you call it done

- Build the configuration you changed and run `lineartest` if a test covers it.
- Run `./scripts/package-build.ps1 -Configuration Release` if you added an executable, a
  data folder the programs read (such as `textures/`), or a file that must ship.
- Re-read the README paragraph your change makes wrong. Documentation is part of the
  change here: the project's answer to "how does this work" is the README, not the code.
