# LinearGizmo

[![Windows MSVC](https://img.shields.io/github/actions/workflow/status/CedricGuillemet/LinearGizmo/build.yml?branch=master&label=Windows%20MSVC)](https://github.com/CedricGuillemet/LinearGizmo/actions/workflows/build.yml)
[![Linux GCC](https://img.shields.io/github/actions/workflow/status/CedricGuillemet/LinearGizmo/build.yml?branch=master&label=Linux%20GCC)](https://github.com/CedricGuillemet/LinearGizmo/actions/workflows/build.yml)
[![Linux Clang](https://img.shields.io/github/actions/workflow/status/CedricGuillemet/LinearGizmo/build.yml?branch=master&label=Linux%20Clang)](https://github.com/CedricGuillemet/LinearGizmo/actions/workflows/build.yml)
[![macOS Apple Clang](https://img.shields.io/github/actions/workflow/status/CedricGuillemet/LinearGizmo/build.yml?branch=master&label=macOS%20Apple%20Clang)](https://github.com/CedricGuillemet/LinearGizmo/actions/workflows/build.yml)

A collection of [Dear LinearUI](https://github.com/ocornut/linearui) widgets for 3D manipulation and more.

For API reference and usage examples, see the [documentation](docs/documentation.md).

## Widgets

Each widget is a standalone `.h`/`.cpp` pair that can be used independently. A unified static library containing all components is also available via the provided CMake build.

### LinearViewGizmo

Manipulate view orientation with a single line of code.

![LinearViewGizmo](images/LinearViewGizmo.gif)

### LinearGizmo

A small library built on top of Dear LinearUI that allows you to manipulate 4x4 float matrices (rotation, translation, and scale). Designed with the Immediate Mode philosophy in mind and has no additional dependencies.

![Rotation](images/rotation.gif)
![Translation](images/translation.gif)
![Bounds](images/bounds.gif)

![Sample](images/sample.png)

### LinearSequencer

A timeline sequencer for editing frame start/end ranges across multiple events.

![LinearSequencer](images/sequencer.png)

### GraphEditor

A node graph editor with connections and a delegate system for custom rendering inside nodes.

![GraphEditor](images/nodeeditor.jpg)

### LinearVectorEditor

A path editor widget for 2D vector geometry. Supports a pen tool, anchor and handle editing, open and closed paths, and host-provided transforms that compose with LinearGizmo object manipulation.

![LinearVectorEditor](images/vectoreditor.png)

## Install

LinearGizmo can be installed via [vcpkg](https://github.com/microsoft/vcpkg):

```bash
vcpkg install lineargizmo
```

See the [vcpkg example](vcpkg-example/) for more details.

## License

LinearGizmo is licensed under the MIT License. See [LICENSE](LICENSE) for more information.
