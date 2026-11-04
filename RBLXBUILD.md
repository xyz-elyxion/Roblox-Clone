# RBLXBuild

RBLXBuild is the build-tool source carried in `rblxbuild/`: a pruned, rebranded
snapshot of CMake 4.5 (Kitware, BSD 3-clause), and the thing that now occupies the
slot the project's own `RBLXCC` compiler used to. This note covers what it is for,
what was removed, what was renamed, and what it is not.

## What it is not

**It is not how this repository builds.** The targets in `CMakeLists.txt` at the
root, the presets in `CMakePresets.json` and all three CI jobs are still driven by
the CMake installed on the machine, exactly as before. Nothing in the root build
references `rblxbuild/`: no `add_subdirectory`, no custom command, no install rule.
Vendoring the source did not swap the tool.

## What it is

- **A source snapshot.** `Source/`, `Modules/`, `Utilities/`, `Templates/`,
  `bootstrap`, `configure`, the top-level CMake scripts and `Licenses/`: 4,906
  files, 54 MB. It is meant to be read, not run.
- **Pruned.** CMake's own test suite (113 MB, 25,002 files), the manual sources,
  the autocompletion and packaging trees, and the upstream CI/editor metadata are
  gone. That is why the tree no longer configures as upstream CMake:
  `Source/../CMakeLists.txt` calls `add_subdirectory(Tests)` unconditionally and
  the manual is generated from `Help/`. Commit `62b91b06` still holds the complete
  tree if a piece has to come back -
  `git checkout 62b91b06 -- compiler/Tests` under the directory's old name.
- **Rebranded where it is identity.** The directory, the vendored README, the
  README section and the three command targets carry our name:

  | upstream | here |
  | --- | --- |
  | `cmake` | `rblxbuild` |
  | `ctest` | `rblxtest` |
  | `cpack` | `rblxpack` |

  The rename covers `add_executable`, the `_tools` list that drives installation,
  and the MSVC delay-load loop. `ccmake` and `cmake-gui` keep their names because
  they are optional front ends this project never builds.
- **Upstream underneath.** `CMAKE_*` variables, `CMakeLists.txt` as a file name,
  the `.cmake` module extension, the `cm*` C++ symbols and `${CMAKE_COMMAND}` are
  untouched. That is deliberate: they are load-bearing, and the modules in
  `Modules/` are what this project's own `CMakeLists.txt` is written against.

## Status, honestly

- **Not built here, at all.** The rename above is a declaration-level change
  checked by reading the file; no configure has run against this tree, and no CI
  job builds it. Treat the source as reference material whose buildability is
  unproven.
- **To make it buildable** you would restore the pruned directories, run
  `./bootstrap`, and then fix whatever the rename broke - the safest version of
  which is to revert `16b101b3`'s target rename rather than chase it.
- **Licence.** CMake's copyright notices, `Licenses/` and the BSD 3-clause text
  stay in place. Renaming a product is not relicensing it, and the attribution may
  not be stripped to make the tree look wholly ours.
