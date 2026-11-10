# RblxGizmo API Documentation

## Table of Contents

- [Getting Started](#getting-started)
- [Core Functions](#core-functions)
- [Matrix Helpers](#matrix-helpers)
- [Debug](#debug)
- [Gizmo Manipulation](#gizmo-manipulation)
- [Usage Example](#usage-example)

---

## Getting Started

Call `BeginFrame` once per frame, right after `RblxUI_XXXX_NewFrame()`:

```cpp
RblxGizmo::BeginFrame();
```

---

## Core Functions

### IsOver

Returns `true` if the mouse cursor is over any gizmo control (axis, plane, or screen component).

```cpp
bool IsOver();
```

### IsUsing

Returns `true` if the mouse is over the gizmo or if the gizmo is currently being moved.

```cpp
bool IsUsing();
```

### Enable

Enables or disables the gizmo. The state persists until the next call to `Enable`. When disabled, the gizmo is rendered with a semi-transparent gray color.

```cpp
void Enable(bool enable);
```

---

## Matrix Helpers

Helper functions for manually editing translation, rotation, and scale using float inputs.
Each parameter (`translation`, `rotation`, `scale`) points to an array of 3 floats.
Angles are in degrees.

```cpp
void DecomposeMatrixToComponents(const float* matrix, float* translation, float* rotation, float* scale);
void RecomposeMatrixFromComponents(const float* translation, const float* rotation, const float* scale, float* matrix);
```

> **Note:** These functions have some numerical stability limitations. Use with caution.

**Example:**

```cpp
float matrixTranslation[3], matrixRotation[3], matrixScale[3];
RblxGizmo::DecomposeMatrixToComponents(gizmoMatrix.m16, matrixTranslation, matrixRotation, matrixScale);
RblxUI::InputFloat3("Tr", matrixTranslation, 3);
RblxUI::InputFloat3("Rt", matrixRotation, 3);
RblxUI::InputFloat3("Sc", matrixScale, 3);
RblxGizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, gizmoMatrix.m16);
```

---

## Debug

### DrawCube

Renders a cube with face colors corresponding to face normals. Useful for debugging and testing.

```cpp
void DrawCube(const float* view, const float* projection, float* matrix);
```

---

## Gizmo Manipulation

### Manipulate

Draws and handles a gizmo for the given matrix. Requires view and projection matrices.

The `matrix` parameter is both input and output: it defines where the gizmo is drawn and is updated when the user interacts with it. `deltaMatrix` is optional and receives the incremental transform. `snap` points to a `float[3]` for translation snapping, or a single `float` for rotation or scale snapping. Snap angles are in degrees.

> **Note:** Both finite and infinite far plane projection matrices are supported. Left-handed and right-handed coordinate systems are both supported — RblxGizmo derives its behavior from the matrices you provide.

```cpp
enum OPERATION
{
    TRANSLATE,
    ROTATE,
    SCALE
};

enum MODE
{
    LOCAL,
    WORLD
};

void Manipulate(const float* view, const float* projection, OPERATION operation, MODE mode,
                float* matrix, float* deltaMatrix = 0, float* snap = 0);
```

---

## Usage Example

![RblxGizmo dialog](../images/dialog.png)

```cpp
void EditTransform(float* cameraView, float* cameraProjection, float* matrix)
{
    static RblxGizmo::OPERATION mCurrentGizmoOperation(RblxGizmo::ROTATE);
    static RblxGizmo::MODE mCurrentGizmoMode(RblxGizmo::WORLD);
    if (RblxUI::IsKeyPressed(RblxUIKey_T))
        mCurrentGizmoOperation = RblxGizmo::TRANSLATE;
    if (RblxUI::IsKeyPressed(RblxUIKey_E))
        mCurrentGizmoOperation = RblxGizmo::ROTATE;
    if (RblxUI::IsKeyPressed(RblxUIKey_R))
        mCurrentGizmoOperation = RblxGizmo::SCALE;
    if (RblxUI::RadioButton("Translate", mCurrentGizmoOperation == RblxGizmo::TRANSLATE))
        mCurrentGizmoOperation = RblxGizmo::TRANSLATE;
    RblxUI::SameLine();
    if (RblxUI::RadioButton("Rotate", mCurrentGizmoOperation == RblxGizmo::ROTATE))
        mCurrentGizmoOperation = RblxGizmo::ROTATE;
    RblxUI::SameLine();
    if (RblxUI::RadioButton("Scale", mCurrentGizmoOperation == RblxGizmo::SCALE))
        mCurrentGizmoOperation = RblxGizmo::SCALE;
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    RblxGizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
    RblxUI::InputFloat3("Tr", matrixTranslation);
    RblxUI::InputFloat3("Rt", matrixRotation);
    RblxUI::InputFloat3("Sc", matrixScale);
    RblxGizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

    if (mCurrentGizmoOperation != RblxGizmo::SCALE)
    {
        if (RblxUI::RadioButton("Local", mCurrentGizmoMode == RblxGizmo::LOCAL))
            mCurrentGizmoMode = RblxGizmo::LOCAL;
        RblxUI::SameLine();
        if (RblxUI::RadioButton("World", mCurrentGizmoMode == RblxGizmo::WORLD))
            mCurrentGizmoMode = RblxGizmo::WORLD;
    }
    static bool useSnap(false);
    if (RblxUI::IsKeyPressed(RblxUIKey_S))
        useSnap = !useSnap;
    RblxUI::Checkbox("##useSnap", &useSnap);
    RblxUI::SameLine();
    vec_t snap;
    switch (mCurrentGizmoOperation)
    {
    case RblxGizmo::TRANSLATE:
        snap = config.mSnapTranslation;
        RblxUI::InputFloat3("Snap", &snap.x);
        break;
    case RblxGizmo::ROTATE:
        snap = config.mSnapRotation;
        RblxUI::InputFloat("Angle Snap", &snap.x);
        break;
    case RblxGizmo::SCALE:
        snap = config.mSnapScale;
        RblxUI::InputFloat("Scale Snap", &snap.x);
        break;
    default:
        break;
    }
    RblxUIIO& io = RblxUI::GetIO();
    RblxGizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    RblxGizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation,
                         mCurrentGizmoMode, matrix, NULL, useSnap ? &snap.x : NULL);
}
```
