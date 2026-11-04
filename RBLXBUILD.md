# RBLXBuild

RBLXBuild is the build-tool source carried in `rblxbuild/`, and the thing that
occupies the slot the project's own `RBLXCC` compiler used to.

**It is not how this repository builds.** The targets in the root `CMakeLists.txt`,
the presets and all three CI jobs are still driven by the CMake installed on the
machine. Nothing in the root build references `rblxbuild/`.

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
- **It has never been compiled.** No configure, no build, no test: the tree had
  already lost `Tests/` and `Help/`, which its own top-level build requires. Treat
  buildability as unproven and probably broken.
- **It is reversible.** `git revert` of the rename commit restores upstream
  spelling; `git checkout 62b91b06 -- compiler` restores the original unpruned
  tree under its old directory name.

## Attribution

`rblxbuild/Licenses/` is untouched - the third-party licence texts and copyright
notices are byte-identical to what shipped, because the BSD 3-clause terms require
the copyright notice and disclaimer to travel with the source. That directory is
the only place the upstream project's name still appears, and it stays for that
legal reason, not as branding.
