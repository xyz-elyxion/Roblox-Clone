#pragma once

#include "rblxui.h"

#include <string>
#include <vector>

namespace RblxVectorEditor {

enum class Tool {
    Select,
    Pen,
};

enum class HitPart {
    None,
    Anchor,
    HandleIn,
    HandleOut,
};

enum class HandleMode {
    Corner,
    Mirrored,
    Aligned,
    Free,
};

enum class ControlPointShape {
    Circle,
    Square,
    Diamond,
};

enum class EditKind {
    MoveAnchor,
    MoveHandle,
    AddAnchor,
    DeleteAnchor,
    AddHandle,
    DeleteHandle,
    ChangePointMode,
    PathOperation,
};

struct Anchor {
    RblxVec2 position = RblxVec2(0.0f, 0.0f);
    RblxVec2 handleIn = RblxVec2(0.0f, 0.0f);
    RblxVec2 handleOut = RblxVec2(0.0f, 0.0f);
    bool hasHandleIn = false;
    bool hasHandleOut = false;
    HandleMode handleMode = HandleMode::Corner;
};

struct Path {
    std::vector<Anchor> anchors;
    bool closed = false;

    bool empty() const { return anchors.empty(); }
    void clear() {
        anchors.clear();
        closed = false;
    }
};

bool MakeCorner(Anchor& anchor);
bool MakeAligned(Anchor& anchor);
bool MakeMirrored(Anchor& anchor);
bool AddHandles(Anchor& anchor, float length = 50.0f);
bool DeleteHandles(Anchor& anchor);
void ReversePath(Path& path);

struct Delegate {
    virtual void BeginEdit(EditKind /*kind*/, int /*anchorIndex*/) {}
    virtual void EndEdit() {}
    virtual ~Delegate() = default;
};

// Host-provided transform state. Points are stored in path-local coordinates.
// The editor composes object transform first, then canvas pan/zoom.
struct Transform {
    RblxVec2 pan = RblxVec2(0.0f, 0.0f);
    float zoom = 1.0f;
    RblxVec2 objectPivot = RblxVec2(0.0f, 0.0f);
    RblxVec2 objectTranslation = RblxVec2(0.0f, 0.0f);
    float objectRotationRadians = 0.0f;
    RblxVec2 objectScale = RblxVec2(1.0f, 1.0f);

    RblxVec2 LocalToCanvas(const RblxVec2& local) const;
    RblxVec2 CanvasToLocal(const RblxVec2& canvas) const;
};

struct Style {
    RblxU32 backgroundColor = RBLXUI_COL32(31, 34, 42, 255);
    RblxU32 gridColor = RBLXUI_COL32(255, 255, 255, 22);
    RblxU32 pathColor = RBLXUI_COL32(245, 245, 245, 255);
    RblxU32 previewColor = RBLXUI_COL32(245, 245, 245, 120);
    RblxU32 anchorColor = RBLXUI_COL32(255, 255, 255, 255);
    RblxU32 anchorSelectedColor = RBLXUI_COL32(255, 190, 84, 255);
    RblxU32 handleColor = RBLXUI_COL32(112, 174, 255, 255);
    RblxU32 handleLineColor = RBLXUI_COL32(112, 174, 255, 130);
    RblxU32 closeTargetColor = RBLXUI_COL32(120, 240, 160, 255);
    float pathThickness = 2.0f;
    float handleLineThickness = 1.0f;
    float anchorRadius = 5.0f;
    float handleRadius = 3.5f;
    ControlPointShape anchorShape = ControlPointShape::Square;
    ControlPointShape handleShape = ControlPointShape::Circle;
    float hitRadius = 9.0f;
    float gridStep = 32.0f;
};

struct Config {
    RblxVec2 canvasSize = RblxVec2(0.0f, 360.0f);
    Tool tool = Tool::Pen;
    Transform transform;
    Style style;
    Delegate* delegate = nullptr;
    bool showGrid = true;
    bool allowKeyboardShortcuts = true;
    bool readOnly = false;
};

struct Result {
    bool changed = false;
    bool committed = false;
    bool canceled = false;
    bool pathClosed = false;
    bool hovered = false;
    bool active = false;
    bool wantsMouseCapture = false;
    bool wantsKeyboardCapture = false;
    RblxVec2 viewPanDelta = RblxVec2(0.0f, 0.0f);
    RblxVec2 viewZoomCenterCanvas = RblxVec2(0.0f, 0.0f);
    float viewZoomFactor = 1.0f;
};

class Editor {
public:
    Result Draw(const char* id, Path& path, const Config& config = Config());

    bool IsHovered() const { return hovered_; }
    bool IsActive() const { return active_; }
    bool WantsMouseCapture() const { return wantsMouseCapture_; }
    bool WantsKeyboardCapture() const { return wantsKeyboardCapture_; }

    void ClearSelection();
    int GetSelectedAnchor() const { return selectedAnchor_; }
    int GetSelectedAnchorCount() const { return static_cast<int>(selectedAnchors_.size()); }
    HitPart GetSelectedPart() const { return selectedPart_; }

private:
    struct Hit {
        int anchor = -1;
        HitPart part = HitPart::None;
        float distance = 0.0f;
    };

    Hit HitTest(const Path& path, const Config& config, const RblxVec2& canvasOrigin,
                const RblxVec2& mouseScreen) const;
    RblxVec2 LocalToScreen(const Config& config, const RblxVec2& canvasOrigin,
                         const RblxVec2& local) const;
    RblxVec2 ScreenToLocal(const Config& config, const RblxVec2& canvasOrigin,
                         const RblxVec2& screen) const;

    void DrawGrid(RblxDrawList* drawList, const RblxVec2& origin, const RblxVec2& size,
                  const Config& config) const;
    void DrawPath(RblxDrawList* drawList, const Path& path, const Config& config,
                  const RblxVec2& canvasOrigin) const;
    void DrawControls(RblxDrawList* drawList, const Path& path, const Config& config,
                      const RblxVec2& canvasOrigin, const Hit& hoveredHit) const;

    bool HandleKeyboard(Path& path, const Config& config, Result& result);
    bool HandleMouse(Path& path, const Config& config, const RblxVec2& canvasOrigin,
                     const Hit& hoveredHit, Result& result);
    bool IsAnchorSelected(int anchor) const;
    void SelectAnchor(int anchor, HitPart part, bool addToSelection);
    void RemoveSelectedAnchors(Path& path);
    void BeginAnchorDrag(const Path& path, const RblxVec2& mouseLocal);
    void UpdateBoxSelection(Path& path, const Config& config, const RblxVec2& canvasOrigin,
                            bool addToSelection);
    void BeginEdit(const Config& config, EditKind kind, int anchorIndex);
    void EndEdit(const Config& config);

    int selectedAnchor_ = -1;
    HitPart selectedPart_ = HitPart::None;
    std::vector<int> selectedAnchors_;
    bool dragging_ = false;
    bool penActive_ = false;
    bool newAnchorDrag_ = false;
    bool boxSelecting_ = false;
    bool editActive_ = false;
    bool dragEditStarted_ = false;
    RblxVec2 dragStartLocal_ = RblxVec2(0.0f, 0.0f);
    RblxVec2 boxSelectStartScreen_ = RblxVec2(0.0f, 0.0f);
    RblxVec2 boxSelectCurrentScreen_ = RblxVec2(0.0f, 0.0f);
    std::vector<RblxVec2> dragStartAnchorPositions_;

    bool hovered_ = false;
    bool active_ = false;
    bool wantsMouseCapture_ = false;
    bool wantsKeyboardCapture_ = false;
};

} // namespace RblxVectorEditor
