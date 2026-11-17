# Working in this repository

Linear is a Windows-only sandbox-platform prototype in C++17: a game client, a game
server, a world editor (Studio), a source editor (LinearCode), a native website backend
(`WebApp.exe`), an installer, an updater, a headless trainer and a server supervisor
(`LinearHosts.exe`). Everything — including the website — is C++ built from this tree, and
the build tool itself is vendored. There is no Node.js, npm, Python, CMake install or
package manager involved.

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
`build/windows-release`, Release). Binaries land in `build\windows-release\Release\`.
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
   (WinSock), HTTP through `include/HttpClient.h`. Do not reintroduce `#ifdef _WIN32`
   forks or a "portable" path; the project was deliberately cut down to one platform.
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
   casts in the anti-cheat's `Detections.cpp` and `Process.cpp`; the TLS callback in
   `Core/AntiCheatLib.cpp`, which scanned threads and logged through the C runtime while the
   loader still held its lock and before the static runtime existed, hanging the client
   before `main()` on an empty console. Keep those fixes;
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
5. **Nothing ships unless the packager knows about it.** A new executable must be added
   to the executable list in `scripts/package-build.ps1`, to the folder listing in
   `README.md` and to the generated `README.txt` inside that script.
6. **Security decisions belong in `SECURITY.md`.** Anything that touches authentication,
   tokens, the data file, rate limits or moderation is described there; keep it true when
   you change the behaviour.

## Where things live

| Area | Files |
| --- | --- |
| Game client | `src/Client.cpp` (startup, arguments, loop), `src/ClientNet.cpp` (socket, session, join), `src/ClientUi.cpp` (menus, HUD, chat, settings), `src/ClientShared.h` |
| Game server | `src/Server.cpp`, `include/MovementGuard.h`, `include/Moderation.h`, `src/Player.cpp` |
| Studio | `src/Editor.cpp`, `src/WorldLoader.cpp`, `src/WorldScript.cpp` |
| LinearCode | `src/CodeMain.cpp`, `src/CodeProject.cpp` (tree, search, outline, docs), `src/CodeBuild.cpp` + `src/CodeBuildRun.cpp` (the build panel), `src/CodeGit.cpp` (the Source control panel) |
| Website backend | `src/WebApp.cpp`, `src/WebHttp.cpp`, `src/WebJson.cpp`, `src/WebStore.cpp`, `src/WebCrypto.cpp`, `src/WebCaptcha.h` |
| Website pages | `web/public` (plain HTML/CSS/JS, no build step), `web/seed-worlds` |
| Server supervisor | `src/ServerHost.cpp`, `src/HostMain.cpp`, `include/ServerHost.h` |
| Supervisor control panel | `src/HostDashboard.cpp`, `include/HostDashboard.h`, `web/host/` |
| Installer / updater | `src/SetupMain.cpp`, `src/InstallUi.cpp`, `src/UpdaterMain.cpp`, `src/Updater.cpp` |
| Server setup | `src/ServerSetupMain.cpp`, `src/ServerConfig.cpp` (the `server.conf` the website reads) |
| Trainer | `src/AiTrainer.cpp` |
| Scripting | `src/LinearScript.cpp`, `include/LinearScript.h` |
| Anti-cheat integration | `include/ClientCheatReport.h`, `include/Obfuscated.h`, `LinearAntiCheat/` |
| Build | `LinearBuildLists.txt`, `LinearBuildPresets.json`, `scripts/package-build.ps1` |
| Vendored libraries | `linearwin/`, `linearmath/`, `linearui/`, `lineargizmo/`, `lineartextedit/`, `linearphysics/`, `linearstb/`, `lineargl/` |
| Tests | `tests/<area>_tests.cpp` |

`include/` mirrors `src/`: a unit lives in a header there so Studio, the client, the
server and the tests can share it.

## Adding things

- **A source file** goes into the right `add_executable` in `LinearBuildLists.txt`. Sources
  are listed explicitly per target — there is no globbing, so a new file that is not
  listed simply does not compile into anything.
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
