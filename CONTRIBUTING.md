# Contributing

Linear is a Windows-only C++17 project with no package manager, no Node.js and no external
build tool to install: everything including the build tool itself is in this tree. Read this
file, then [AGENTS.md](AGENTS.md) for the layout, and [README.md](README.md) for what the
project actually is.

## Set up once

Visual Studio 2022 with the C++ workload, or MinGW with Ninja. Then bootstrap the vendored
build tool:

```powershell
mkdir -p build/linearbuild-tool
cd build/linearbuild-tool
../../linearbuild/bootstrap --parallel 4
cd ../..
```

## Build and test

```powershell
build\linearbuild-tool\bin\linearbuild --preset windows-release
build\linearbuild-tool\bin\linearbuild --build --preset windows-release
build\linearbuild-tool\bin\lineartest --preset windows-release
```

`LinearBuildPresets.json` has one preset. Binaries land in `build\windows-release\Release\`.
To assemble the folder that ships, including the `README.txt`, `manifest.json` and
`update.json` a release needs:

```powershell
./scripts/package-build.ps1 -Configuration Release
```

## What a pull request should look like

- **One change.** A fix and a rename in the same branch is two pull requests, because the
  second one hides the first in review.
- **Built and tested by you.** Say which configuration you built and paste the commands and
  their result, not the words "tested locally".
- **Warnings clean.** The project builds without warnings under `LINEAR_ENABLE_WARNINGS`; a
  new warning is a failure, not noise. Do not silence one the compiler is right about - fix
  the code, or say why the warning is wrong.
- **Docs updated in the same pull request.** The answer to "how does this work" is `README.md`,
  not the code. A visible feature gets a paragraph in the relevant section; a longer topic gets
  its own file linked from there.
- **No build output, no `dist/`, no `artifacts/`.** Those are ignored for a reason.

## Rules that get a pull request closed

These come from [AGENTS.md](AGENTS.md) and are not negotiable. They are listed here because
they are the ones most often missed.

- **Windows only.** No `#ifdef _WIN32` portability fork, no POSIX path, no "works on Linux"
  cleanup. Sockets go through `include/Network.h`, HTTP through `include/HttpClient.h`.
- **No new dependencies.** No package manager, no submodule, no generated blob, no "just one
  small library". The vendored trees are pinned snapshots.
- **No launcher scripts.** No `.bat`, `.cmd` or wrapper "to help" - every program finds its
  data in the folder it runs from.
- **One palette.** Colours come from `include/UiTheme.h`, as a named role in both palettes.
  A hard-coded colour in a window is a defect.
- **Nothing ships unless the packager knows about it.** A new executable needs a line in
  `scripts/package-build.ps1`, the folder listing in `README.md`, and the generated `README.txt`
  in that script.
- **A new source file must be listed.** `LinearBuildLists.txt` names its sources explicitly;
  there is no globbing, so an unlisted file silently compiles into nothing.
- **Vendored trees are renamed or left alone.** `linearwin`, `linearmath`, `linearui`,
  `lineargizmo`, `lineartextedit`, `linearphysics`, `linearstb`, `lineargl`, `linearbuild` and
  `LinearAntiCheat` are upstream code with this project's names. Do not reformat them, do not
  rename identifiers piecemeal, and do not "clean them up". The one exception is a defect the
  compiler is right about: fix it in place and add it to the list in `AGENTS.md`.

## Tests

There is no test framework and none should be added. A test is `tests/<area>_tests.cpp`: a
plain `main()` that checks the unit, prints what failed and returns nonzero. Register it with
`add_executable` and `add_test` inside the `if(LINEAR_BUILD_TESTS)` block of
`LinearBuildLists.txt`, and link the source it covers directly rather than the whole game.

A test has to be worth running: exercise the boundary that actually breaks (an empty file, a
window edge, a hostile packet), not the getter you just wrote.

## Style

C++17, four spaces, roughly 100 columns, `#pragma once` in headers, namespaces under
`linear::`. Failures come back as `bool` plus `std::string* error` rather than exceptions;
console programs print the error and return nonzero. Comments say *why* a thing is the way it
is - especially where a rule looks odd - and not what the line below already says. No formatter
or linter is configured; match the file you are in.

## Security

Never report a vulnerability in a public issue. [SECURITY.md](SECURITY.md) has the private
advisory route and describes what the project does and does not defend against. Anything
touching accounts, sessions, tokens, the data file, rate limits or moderation updates that file
in the same pull request.

## Licensing of contributions

This project is licensed under the GNU Affero General Public License v3 - see
[LICENSE](LICENSE) for why that licence and not a permissive one. By opening a pull request you
agree that your contribution is offered under the same terms. There is no contributor licence
agreement and no copyright assignment.

Only send code you have the right to send. Copying a snippet from a project under an
incompatible licence, or from generated text you cannot license, will be rejected.

## Questions

Open an issue using the templates in `.github/ISSUE_TEMPLATE`. A question with a command and
its output is a useful issue; "how do I do X" with no context usually gets a link to the
README instead of an answer.
