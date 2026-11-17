---
name: Feature request
about: Something the project should do that it does not
title: ''
labels: enhancement
assignees: ''
---

**The problem**
<!-- What you were trying to do, and where the project got in the way. A concrete sequence
beats an abstract wish: "I want to place 200 parts without the toolbox closing" is actionable,
"I want a better editor" is not. -->

**What you would like to happen**

**Which part of the tree it touches**
- [ ] Client
- [ ] Server / gameplay
- [ ] Studio (worlds, scripting)
- [ ] LinearCode
- [ ] Website (`web/public`, `WebApp.exe`)
- [ ] Hosting (`LinearHosts.exe`, `LinearProxy.exe`, `server.conf`)
- [ ] Installer, updater, packaging
- [ ] The build tool or tests

**Does it fit the project's shape?**
<!-- Tick what applies; the first three are rules in AGENTS.md, not preferences. -->
- [ ] It works on Windows only, with no new dependency
- [ ] It needs no launcher script or wrapper to run
- [ ] Any UI it adds reads its colours from `include/UiTheme.h`, in both palettes
- [ ] It is a new executable, and I have read rule 5 in `AGENTS.md` about the packager
- [ ] It changes authentication, the data file or moderation, so `SECURITY.md` would need a
      paragraph

**Alternatives you considered**
<!-- Including "do nothing", if the current behaviour is defensible. -->

**How you would check it works**
<!-- The check you would want to see in the pull request: a test in tests/, a screenshot, a
command and its output. -->
