# RBLXBuild

RBLXBuild is the build-tool source carried in `rblxbuild/`, and the thing that
occupies the slot the project's own `RBLXCC` compiler used to.

**It is how this repository builds.** The root `RBLXBuildLists.txt`, its
`RBLXBuildPresets.json` and the CI job are driven by it: CI bootstraps
`rblxbuild/`, then configures, builds, tests and packages the game with the binary
that comes out. No CMake install is involved.

## What it is

A renamed fork of the CMake source tree: every product identifier, file name,
module extension, documentation string, variable prefix and directory name was
renamed to the RBLXBuild spelling, so a case-insensitive search for the upstream
name finds nothing outside `Licenses/`.

| upstream | here |
| --- | --- |
| `cmake` command | `rblxbuild` |
| `ctest` / `cpack` | `rblxtest` / `rblxpack` |
| `CMakeLists.txt` | `RBLXBuildLists.txt` |
| `.cmake` modules (1,700+) | `.rblxbuild` |
| `CMAKE_*` variables | `RBLXBUILD_*` |
| `cmake_minimum_required` | `rblxbuild_minimum_required` |
| `CMake_VERSION` | `RBLXBuild_VERSION` |
| `Modules/CMakeAddFortranSubdirectory` | `Modules/RBLXBuildAddFortranSubdirectory` |

## What that costs

The rename is textual and complete, not a port:

- **Nothing upstream interoperates with it.** No real CMake will configure a
  directory whose build file is `RBLXBuildLists.txt` or whose modules end in
  `.rblxbuild`, and no upstream module will load into `rblxbuild`. Adapting real
  code to it is a port, not a flag.
- **`rblxbuild` builds.** The prune removed `Tests/`, `Help/`, `Auxiliary/` and
  `Packaging/`, but the root build file still added `Tests` and `Auxiliary`
  unconditionally and configured `Tests/.NoDartCoverage`; those three references are
  now `EXISTS`-guarded. Bootstrap, configure and `make rblxbuild` succeed, and the
  resulting binary answers `-E echo` and `--version`. No tests exist, because
  `Tests/` was pruned. Installation is still broken: `install(DIRECTORY Help ...)`
  names a directory this tree does not carry.
- **Everything this repository builds is in the dialect, including what it
  vendors.** The root file, the anti-cheat library and the library trees beside them
  (rblxwin, rblxmath, rblxui, rblxgizmo, rblxtextedit,
  rblxphysics, rblxstb, rblxgl) all use `RBLXBuildLists.txt` and `RBLXBUILD_*`
  variables, so configuring downloads nothing and reads no upstream file.
  `.github/workflows/build.yml` builds the tool, then uses that tool to configure,
  build, test and package the project, so the game is built by RBLXBuild rather
  than by an installed CMake.
- **It still reads the files it was forked from, as a fallback.** The tool accepts
  `CMakeLists.txt` alongside `RBLXBuildLists.txt`, registers the `cmake_*` command
  names alongside the `rblxbuild_*` ones, resolves `CMAKE_*` variable reads
  through their `RBLXBUILD_*` counterparts, forwards the upstream module names
  from `Modules/CMake*.rblxbuild`, and lets `add_subdirectory` and FetchContent
  take a dependency that carries either file. Nothing tracked in this repository
  relies on those paths any more; they exist for code brought in later. They are
  single-sided: variables written under the upstream spelling are not mirrored
  back, so `-D` options the project passes to the tool use the `RBLXBUILD_`
  spelling.
- **It is reversible.** `git revert` of the rename commit restores upstream
  spelling; `git checkout 62b91b06 -- compiler` restores the original unpruned
  tree under its old directory name.

## Attribution

`rblxbuild/Licenses/` is untouched - the third-party licence texts and copyright
notices are byte-identical to what shipped, because the BSD 3-clause terms require
the copyright notice and disclaimer to travel with the source. That directory is
the only place the upstream project's name still appears, and it stays for that
legal reason, not as branding.
