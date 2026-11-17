---
name: Bug report
about: Something built, started or behaved wrongly
title: ''
labels: bug
assignees: ''
---

**What happened**
<!-- The error text if there is one. A blank console and no error is still a symptom, say so. -->

**What you expected instead**

**Steps to reproduce**
1.
2.
3.

**Which part**
<!-- Tick everything that misbehaved. -->
- [ ] `Client.exe` (the game)
- [ ] `Server.exe` (the game server)
- [ ] `Studio.exe` (world editor)
- [ ] `LinearCode.exe` (source editor)
- [ ] `WebApp.exe` (the website)
- [ ] `LinearHosts.exe` / `LinearProxy.exe` (hosting)
- [ ] `LinearSetup.exe` / `LinearUpdater.exe`
- [ ] The build itself (`linearbuild`, `lineartest`, `package-build.ps1`)

**Build and machine**
- Commit or release you ran:
- Compiler and preset (for example MSVC 2022, `windows-release`):
- How you built it (the exact commands):
- Windows version:
- Anti-cheat built in? (`-DLINEAR_BUILD_ANTICHEAT`, default on):

**Evidence**
<!-- Paste it rather than describe it. Redact tokens, passwords and other people's data. -->
- Console output:
- `startup_trace.txt` from the folder you ran it in, if the client showed nothing:
- `web/game-server-logs/game-server-<port>.log` for a server that did not start:
- CI job link, for a build failure:

**Anything else**
<!-- What you already tried, and what it changed. -->
