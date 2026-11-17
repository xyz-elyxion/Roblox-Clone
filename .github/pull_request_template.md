## What this changes

<!-- One change. If this is two, say so and split it. -->

## Why

<!-- The problem it fixes or the behaviour it adds, and why this shape rather than another.
Link the issue if there is one: Fixes #000 -->

## How it was checked

<!-- Paste the commands and their result. "Tested locally" is not a check. -->
- Built with: <!-- e.g. `linearbuild --build --preset windows-release`, MSVC 2022 -->
- Tests: <!-- e.g. `lineartest --preset windows-release`, and which suites cover this -->
- Manual check: <!-- what you ran and saw; say plainly what you did not try -->

## Rules

<!-- From AGENTS.md and CONTRIBUTING.md. Tick, or explain why a line does not apply. -->
- [ ] Windows only: no portability fork, sockets through `include/Network.h`
- [ ] No new dependency, submodule, package manager or generated blob
- [ ] No launcher script or wrapper
- [ ] Any new UI text or colour comes from `include/UiTheme.h` roles, in both palettes
- [ ] New source files are listed in `LinearBuildLists.txt`; new tests are registered inside
      `if(LINEAR_BUILD_TESTS)`
- [ ] A new executable is in `scripts/package-build.ps1`, the `README.md` folder listing and the
      generated `README.txt`
- [ ] Builds warning-clean under `LINEAR_ENABLE_WARNINGS`
- [ ] Vendored trees untouched, or the fix is a defect the compiler is right about and is added
      to the list in `AGENTS.md`

## Documentation

- [ ] `README.md` describes what changed in the relevant section
- [ ] `HOSTING.md`, `LINEARBUILD.md` or `SECURITY.md` updated if this touches them
- [ ] Nothing to update, and that is deliberate

## Screenshots

<!-- Only for something a person sees. Before and after, if the change is visual. -->
