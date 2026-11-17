// https://github.com/CedricGuillemet/LinearGizmo
// v1.92.5 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2016-2026 Cedric Guillemet and contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#define LINEARUI_DEFINE_MATH_OPERATORS
#include "linearui.h"
#include "linearui_internal.h"
#include <math.h>
#include <vector>
#include <float.h>
#include <array>
#include "GraphEditor.h"

namespace GraphEditor {

static inline float Distance(const LinearVec2& a, const LinearVec2& b)
{
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

static inline float sign(float v)
{
    return (v >= 0.f) ? 1.f : -1.f;
}

static LinearVec2 GetInputSlotPos(Delegate& delegate, const Node& node, SlotIndex slotIndex, float factor)
{
    LinearVec2 Size = node.mRect.GetSize() * factor;
    size_t InputsCount = delegate.GetTemplate(node.mTemplateIndex).mInputCount;
    return LinearVec2(node.mRect.Min.x * factor,
                  node.mRect.Min.y * factor + Size.y * ((float)slotIndex + 1) / ((float)InputsCount + 1) + 8.f);
}

static LinearVec2 GetOutputSlotPos(Delegate& delegate, const Node& node, SlotIndex slotIndex, float factor)
{
    LinearVec2 Size = node.mRect.GetSize() * factor;
    size_t OutputsCount = delegate.GetTemplate(node.mTemplateIndex).mOutputCount;
    return LinearVec2(node.mRect.Min.x * factor + Size.x,
                  node.mRect.Min.y * factor + Size.y * ((float)slotIndex + 1) / ((float)OutputsCount + 1) + 8.f);
}

static LinearRect GetNodeRect(const Node& node, float factor)
{
    LinearVec2 Size = node.mRect.GetSize() * factor;
    return LinearRect(node.mRect.Min * factor, node.mRect.Min * factor + Size);
}

static LinearVec2 editingNodeSource;
static bool editingInput = false;
static LinearVec2 captureOffset;

enum NodeOperation
{
    NO_None,
    NO_EditingLink,
    NO_QuadSelecting,
    NO_MovingNodes,
    NO_EditInput,
    NO_PanView,
};
static NodeOperation nodeOperation = NO_None;

static void HandleZoomScroll(LinearRect regionRect, ViewState& viewState, const Options& options)
{
    LinearUIIO& io = LinearUI::GetIO();

    if (regionRect.Contains(io.MousePos))
    {
        if (io.MouseWheel < -FLT_EPSILON)
        {
            viewState.mFactorTarget *= 1.f - options.mZoomRatio;
        }

        if (io.MouseWheel > FLT_EPSILON)
        {
            viewState.mFactorTarget *= 1.0f + options.mZoomRatio;
        }
    }

    LinearVec2 mouseWPosPre = (io.MousePos - LinearUI::GetCursorScreenPos()) / viewState.mFactor;
    viewState.mFactorTarget = LinearClamp(viewState.mFactorTarget, options.mMinZoom, options.mMaxZoom);
    viewState.mFactor = LinearLerp(viewState.mFactor, viewState.mFactorTarget, options.mZoomLerpFactor);
    LinearVec2 mouseWPosPost = (io.MousePos - LinearUI::GetCursorScreenPos()) / viewState.mFactor;
    if (LinearUI::IsMousePosValid())
    {
        viewState.mPosition += mouseWPosPost - mouseWPosPre;
    }
}

void GraphEditorClear()
{
    nodeOperation = NO_None;
}

static void FitNodes(Delegate& delegate, ViewState& viewState, const LinearVec2 viewSize, bool selectedNodesOnly)
{
    const size_t nodeCount = delegate.GetNodeCount();

    if (!nodeCount)
    {
        return;
    }

    bool validNode = false;
    LinearVec2 min(FLT_MAX, FLT_MAX);
    LinearVec2 max(-FLT_MAX, -FLT_MAX);
    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        const Node& node = delegate.GetNode(nodeIndex);
        
        if (selectedNodesOnly && !node.mSelected)
        {
            continue;
        }
        
        min = LinearMin(min, node.mRect.Min);
        min = LinearMin(min, node.mRect.Max);
        max = LinearMax(max, node.mRect.Min);
        max = LinearMax(max, node.mRect.Max);
        validNode = true;
    }
    
    if (!validNode)
    {
        return;
    }
    
    min -= viewSize * 0.05f;
    max += viewSize * 0.05f;
    LinearVec2 nodesSize = max - min;
    LinearVec2 nodeCenter = (max + min) * 0.5f;
    
    float ratioY = viewSize.y / nodesSize.y;
    float ratioX = viewSize.x / nodesSize.x;

    viewState.mFactor = viewState.mFactorTarget = LinearMin(LinearMin(ratioY, ratioX), 1.f);
    viewState.mPosition = LinearVec2(-nodeCenter.x, -nodeCenter.y) + (viewSize * 0.5f) / viewState.mFactorTarget;
}

static void DisplayLinks(Delegate& delegate,
                         LinearDrawList* drawList,
                         const LinearVec2 offset,
                         const float factor,
                         const LinearRect regionRect,
                         NodeIndex hoveredNode,
                         const Options& options)
{
    const size_t linkCount = delegate.GetLinkCount();
    for (LinkIndex linkIndex = 0; linkIndex < linkCount; linkIndex++)
    {
        const auto link = delegate.GetLink(linkIndex);
        const auto nodeInput = delegate.GetNode(link.mInputNodeIndex);
        const auto nodeOutput = delegate.GetNode(link.mOutputNodeIndex);
        LinearVec2 p1 = offset + GetOutputSlotPos(delegate, nodeInput, link.mInputSlotIndex, factor);
        LinearVec2 p2 = offset + GetInputSlotPos(delegate, nodeOutput, link.mOutputSlotIndex, factor);

        // con. view clipping
        if ((p1.y < 0.f && p2.y < 0.f) || (p1.y > regionRect.Max.y && p2.y > regionRect.Max.y) ||
            (p1.x < 0.f && p2.x < 0.f) || (p1.x > regionRect.Max.x && p2.x > regionRect.Max.x))
            continue;

        bool highlightCons = hoveredNode == link.mInputNodeIndex || hoveredNode == link.mOutputNodeIndex;
        uint32_t col = delegate.GetTemplate(nodeInput.mTemplateIndex).mHeaderColor | (highlightCons ? 0xF0F0F0 : 0);
        if (options.mDisplayLinksAsCurves)
        {
            // curves
             drawList->AddBezierCubic(p1, p1 + LinearVec2(50, 0) * factor, p2 + LinearVec2(-50, 0) * factor, p2, 0xFF000000, options.mLineThickness * 1.5f * factor);
             drawList->AddBezierCubic(p1, p1 + LinearVec2(50, 0) * factor, p2 + LinearVec2(-50, 0) * factor, p2, col, options.mLineThickness * 1.5f * factor);
             /*
            LinearVec2 p10 = p1 + LinearVec2(20.f * factor, 0.f);
            LinearVec2 p20 = p2 - LinearVec2(20.f * factor, 0.f);

            LinearVec2 dif = p20 - p10;
            LinearVec2 p1a, p1b;
            if (fabsf(dif.x) > fabsf(dif.y))
            {
                p1a = p10 + LinearVec2(fabsf(fabsf(dif.x) - fabsf(dif.y)) * 0.5 * sign(dif.x), 0.f);
                p1b = p1a + LinearVec2(fabsf(dif.y) * sign(dif.x) , dif.y);
            }
            else
            {
                p1a = p10 + LinearVec2(0.f, fabsf(fabsf(dif.y) - fabsf(dif.x)) * 0.5 * sign(dif.y));
                p1b = p1a + LinearVec2(dif.x, fabsf(dif.x) * sign(dif.y));
            }
            drawList->AddLine(p1,  p10, col, 3.f * factor);
            drawList->AddLine(p10, p1a, col, 3.f * factor);
            drawList->AddLine(p1a, p1b, col, 3.f * factor);
            drawList->AddLine(p1b, p20, col, 3.f * factor);
            drawList->AddLine(p20,  p2, col, 3.f * factor);
            */
        }
        else
        {
            // straight lines
            std::array<LinearVec2, 6> pts;
            int ptCount = 0;
            LinearVec2 dif = p2 - p1;

            LinearVec2 p1a, p1b;
            const float limitx = 12.f * factor;
            if (dif.x < limitx)
            {
                LinearVec2 p10 = p1 + LinearVec2(limitx, 0.f);
                LinearVec2 p20 = p2 - LinearVec2(limitx, 0.f);

                dif = p20 - p10;
                p1a = p10 + LinearVec2(0.f, dif.y * 0.5f);
                p1b = p1a + LinearVec2(dif.x, 0.f);

                pts = { p1, p10, p1a, p1b, p20, p2 };
                ptCount = 6;
            }
            else
            {
                if (fabsf(dif.y) < 1.f)
                {
                    pts = { p1, (p1 + p2) * 0.5f, p2 };
                    ptCount = 3;
                }
                else
                {
                    if (fabsf(dif.y) < 10.f)
                    {
                        if (fabsf(dif.x) > fabsf(dif.y))
                        {
                            p1a = p1 + LinearVec2(fabsf(fabsf(dif.x) - fabsf(dif.y)) * 0.5f * sign(dif.x), 0.f);
                            p1b = p1a + LinearVec2(fabsf(dif.y) * sign(dif.x), dif.y);
                        }
                        else
                        {
                            p1a = p1 + LinearVec2(0.f, fabsf(fabsf(dif.y) - fabsf(dif.x)) * 0.5f * sign(dif.y));
                            p1b = p1a + LinearVec2(dif.x, fabsf(dif.x) * sign(dif.y));
                        }
                    }
                    else
                    {
                        if (fabsf(dif.x) > fabsf(dif.y))
                        {
                            float d = fabsf(dif.y) * sign(dif.x) * 0.5f;
                            p1a = p1 + LinearVec2(d, dif.y * 0.5f);
                            p1b = p1a + LinearVec2(fabsf(fabsf(dif.x) - fabsf(d) * 2.f) * sign(dif.x), 0.f);
                        }
                        else
                        {
                            float d = fabsf(dif.x) * sign(dif.y) * 0.5f;
                            p1a = p1 + LinearVec2(dif.x * 0.5f, d);
                            p1b = p1a + LinearVec2(0.f, fabsf(fabsf(dif.y) - fabsf(d) * 2.f) * sign(dif.y));
                        }
                    }
                    pts = { p1, p1a, p1b, p2 };
                    ptCount = 4;
                }
            }
            float highLightFactor = factor * (highlightCons ? 2.0f : 1.f);
            for (int pass = 0; pass < 2; pass++)
            {
#if LINEARUI_VERSION_NUM < 19276
                drawList->AddPolyline(pts.data(), ptCount, pass ? col : 0xFF000000, false, (pass ? options.mLineThickness : (options.mLineThickness * 1.5f)) * highLightFactor);
#else
                drawList->AddPolyline(pts.data(), ptCount, pass ? col : 0xFF000000, (pass ? options.mLineThickness : (options.mLineThickness * 1.5f)) * highLightFactor);
#endif
            }
        }
    }
}

static void HandleQuadSelection(Delegate& delegate, LinearDrawList* drawList, const LinearVec2 offset, const float factor, LinearRect contentRect, const Options& options)
{
    if (!options.mAllowQuadSelection)
    {
        return;
    }
    LinearUIIO& io = LinearUI::GetIO();
    static LinearVec2 quadSelectPos;
    //auto& nodes = delegate->GetNodes();
    auto nodeCount = delegate.GetNodeCount();

    if (nodeOperation == NO_QuadSelecting && LinearUI::IsWindowFocused())
    {
        const LinearVec2 bmin = LinearMin(quadSelectPos, io.MousePos);
        const LinearVec2 bmax = LinearMax(quadSelectPos, io.MousePos);
        drawList->AddRectFilled(bmin, bmax, options.mQuadSelection, 1.f);
        drawList->AddRect(bmin, bmax, options.mQuadSelectionBorder, 1.f);
        if (!io.MouseDown[0])
        {
            if (!io.KeyCtrl && !io.KeyShift)
            {
                for (size_t nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
                {
                    delegate.SelectNode(nodeIndex, false);
                }
            }

            nodeOperation = NO_None;
            LinearRect selectionRect(bmin, bmax);
            for (unsigned int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
            {
                const auto node = delegate.GetNode(nodeIndex);
                LinearVec2 nodeRectangleMin = offset + node.mRect.Min * factor;
                LinearVec2 nodeRectangleMax = nodeRectangleMin + node.mRect.GetSize() * factor;
                if (selectionRect.Overlaps(LinearRect(nodeRectangleMin, nodeRectangleMax)))
                {
                    if (io.KeyCtrl)
                    {
                        delegate.SelectNode(nodeIndex, false);
                    }
                    else
                    {
                        delegate.SelectNode(nodeIndex, true);
                    }
                }
                else
                {
                    if (!io.KeyShift)
                    {
                        delegate.SelectNode(nodeIndex, false);
                    }
                }
            }
        }
    }
    else if (nodeOperation == NO_None && io.MouseDown[0] && LinearUI::IsWindowFocused() &&
             contentRect.Contains(io.MousePos))
    {
        nodeOperation = NO_QuadSelecting;
        quadSelectPos = io.MousePos;
    }
}

static bool HandleConnections(LinearDrawList* drawList,
                       NodeIndex nodeIndex,
                       const LinearVec2 offset,
                       const float factor,
                       Delegate& delegate,
                       const Options& options,
                       bool bDrawOnly,
                       SlotIndex& inputSlotOver,
                       SlotIndex& outputSlotOver,
                       const bool inMinimap)
{
    static NodeIndex editingNodeIndex;
    static SlotIndex editingSlotIndex;

    LinearUIIO& io = LinearUI::GetIO();
    const auto node = delegate.GetNode(nodeIndex);
    const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);
    const auto linkCount = delegate.GetLinkCount();

    size_t InputsCount = nodeTemplate.mInputCount;
    size_t OutputsCount = nodeTemplate.mOutputCount;
    inputSlotOver = -1;
    outputSlotOver = -1;

    // draw/use inputs/outputs
    bool hoverSlot = false;
    for (int i = 0; i < 2; i++)
    {
        float closestDistance = FLT_MAX;
        SlotIndex closestConn = -1;
        LinearVec2 closestTextPos;
        LinearVec2 closestPos;
        const size_t slotCount[2] = {InputsCount, OutputsCount};
        
        for (SlotIndex slotIndex = 0; slotIndex < slotCount[i]; slotIndex++)
        {
            const char** con = i ? nodeTemplate.mOutputNames : nodeTemplate.mInputNames;
            const char* conText = (con && con[slotIndex]) ? con[slotIndex] : "";

            LinearVec2 p =
                offset + (i ? GetOutputSlotPos(delegate, node, slotIndex, factor) : GetInputSlotPos(delegate, node, slotIndex, factor));
            float distance = Distance(p, io.MousePos);
            bool overCon = (nodeOperation == NO_None || nodeOperation == NO_EditingLink) &&
                           (distance < options.mNodeSlotRadius * 2.f) && (distance < closestDistance);

            
            LinearVec2 textSize;
            textSize = LinearUI::CalcTextSize(conText);
            LinearVec2 textPos =
                p + LinearVec2(-options.mNodeSlotRadius * (i ? -1.f : 1.f) * (overCon ? 3.f : 2.f) - (i ? 0 : textSize.x),
                           -textSize.y / 2);

            LinearRect nodeRect = GetNodeRect(node, factor);
            if (!inMinimap && (overCon || (nodeRect.Contains(io.MousePos - offset) && closestConn == -1 &&
                            (editingInput == (i != 0)) && nodeOperation == NO_EditingLink)))
            {
                closestDistance = distance;
                closestConn = slotIndex;
                closestTextPos = textPos;
                closestPos = p;
                
                if (i)
                {
                    outputSlotOver = slotIndex;
                }
                else
                {
                    inputSlotOver = slotIndex;
                }
            }
            else
            {
               const LinearU32* slotColorSource = i ? nodeTemplate.mOutputColors : nodeTemplate.mInputColors;
               const LinearU32 slotColor = slotColorSource ? slotColorSource[slotIndex] : options.mDefaultSlotColor;
               drawList->AddCircleFilled(p, options.mNodeSlotRadius, LINEARUI_COL32(0, 0, 0, 200));
               drawList->AddCircleFilled(p, options.mNodeSlotRadius * 0.75f, slotColor);
               if (!options.mDrawIONameOnHover)
               {
                    drawList->AddText(io.FontDefault, 14, textPos + LinearVec2(2, 2), LINEARUI_COL32(0, 0, 0, 255), conText);
                    drawList->AddText(io.FontDefault, 14, textPos, LINEARUI_COL32(150, 150, 150, 255), conText);
               }
            }
        }

        if (closestConn != -1)
        {
            const char** con = i ? nodeTemplate.mOutputNames : nodeTemplate.mInputNames;
            const char* conText = (con && con[closestConn]) ? con[closestConn] : "";
            const LinearU32* slotColorSource = i ? nodeTemplate.mOutputColors : nodeTemplate.mInputColors;
            const LinearU32 slotColor = slotColorSource ? slotColorSource[closestConn] : options.mDefaultSlotColor;
            hoverSlot = true;
            drawList->AddCircleFilled(closestPos, options.mNodeSlotRadius * options.mNodeSlotHoverFactor * 0.75f, LINEARUI_COL32(0, 0, 0, 200));
            drawList->AddCircleFilled(closestPos, options.mNodeSlotRadius * options.mNodeSlotHoverFactor, slotColor);
            drawList->AddText(io.FontDefault, 16, closestTextPos + LinearVec2(1, 1), LINEARUI_COL32(0, 0, 0, 255), conText);
            drawList->AddText(io.FontDefault, 16, closestTextPos, LINEARUI_COL32(250, 250, 250, 255), conText);
            bool inputToOutput = (!editingInput && !i) || (editingInput && i);
            if (nodeOperation == NO_EditingLink && !io.MouseDown[0] && !bDrawOnly)
            {
                if (inputToOutput)
                {
                    // check loopback
                    Link nl;
                    if (editingInput)
                        nl = Link{nodeIndex, closestConn, editingNodeIndex, editingSlotIndex};
                    else
                        nl = Link{editingNodeIndex, editingSlotIndex, nodeIndex, closestConn};

                    if (!delegate.AllowedLink(nl.mOutputNodeIndex, nl.mInputNodeIndex))
                    {
                        break;
                    }
                    bool alreadyExisting = false;
                    for (size_t linkIndex = 0; linkIndex < linkCount; linkIndex++)
                    {
                        const auto link = delegate.GetLink(linkIndex);
                        if (!memcmp(&link, &nl, sizeof(Link)))
                        {
                            alreadyExisting = true;
                            break;
                        }
                    }

                    if (!alreadyExisting)
                    {
                        for (unsigned int linkIndex = 0; linkIndex < linkCount; linkIndex++)
                        {
                            const auto link = delegate.GetLink(linkIndex);
                            if (link.mOutputNodeIndex == nl.mOutputNodeIndex && link.mOutputSlotIndex == nl.mOutputSlotIndex)
                            {
                                delegate.DelLink(linkIndex);
                                
                                break;
                            }
                        }

                        delegate.AddLink(nl.mInputNodeIndex, nl.mInputSlotIndex, nl.mOutputNodeIndex, nl.mOutputSlotIndex);
                    }
                }
            }
            // when LinearUI::IsWindowHovered() && !LinearUI::IsAnyItemActive() is uncommented, one can't click the node
            // input/output when mouse is over the node itself.
            if (nodeOperation == NO_None &&
                /*LinearUI::IsWindowHovered() && !LinearUI::IsAnyItemActive() &&*/ io.MouseClicked[0] && !bDrawOnly)
            {
                nodeOperation = NO_EditingLink;
                editingInput = i == 0;
                editingNodeSource = closestPos;
                editingNodeIndex = nodeIndex;
                editingSlotIndex = closestConn;
                if (editingInput)
                {
                    // remove existing link
                    for (unsigned int linkIndex = 0; linkIndex < linkCount; linkIndex++)
                    {
                        const auto link = delegate.GetLink(linkIndex);
                        if (link.mOutputNodeIndex == nodeIndex && link.mOutputSlotIndex == closestConn)
                        {
                            delegate.DelLink(linkIndex);
                            break;
                        }
                    }
                }
            }
        }
    }
    return hoverSlot;
}

static void DrawGrid(LinearDrawList* drawList, LinearVec2 windowPos, const ViewState& viewState, const LinearVec2 canvasSize, LinearU32 gridColor, LinearU32 gridColor2, float gridSize)
{
    float gridSpace = gridSize * viewState.mFactor;
    int divx = static_cast<int>(-viewState.mPosition.x / gridSize);
    int divy = static_cast<int>(-viewState.mPosition.y / gridSize);
    for (float x = fmodf(viewState.mPosition.x * viewState.mFactor, gridSpace); x < canvasSize.x; x += gridSpace, divx ++)
    {
        bool tenth = !(divx % 10);
        drawList->AddLine(LinearVec2(x, 0.0f) + windowPos, LinearVec2(x, canvasSize.y) + windowPos, tenth ? gridColor2 : gridColor);
    }
    for (float y = fmodf(viewState.mPosition.y * viewState.mFactor, gridSpace); y < canvasSize.y; y += gridSpace, divy ++)
    {
        bool tenth = !(divy % 10);
        drawList->AddLine(LinearVec2(0.0f, y) + windowPos, LinearVec2(canvasSize.x, y) + windowPos, tenth ? gridColor2 : gridColor);
    }
}

// return true if node is hovered
static bool DrawNode(LinearDrawList* drawList,
                     NodeIndex nodeIndex,
                     const LinearVec2 offset,
                     const float factor,
                     Delegate& delegate,
                     bool overInput,
                     const Options& options,
                     const bool inMinimap,
                     const LinearRect& viewPort)
{
    LinearUIIO& io = LinearUI::GetIO();
    const auto node = delegate.GetNode(nodeIndex);
    LINEARUI_ASSERT((node.mRect.GetWidth() != 0.f) && (node.mRect.GetHeight() != 0.f) && "Nodes must have a non-zero rect.");
    const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);
    const LinearVec2 nodeRectangleMin = offset + node.mRect.Min * factor;

    const bool old_any_active = LinearUI::IsAnyItemActive();
    LinearUI::SetCursorScreenPos(nodeRectangleMin);
    const LinearVec2 nodeSize = node.mRect.GetSize() * factor;

    // test nested IO
    drawList->ChannelsSetCurrent(1); // Background
    const size_t InputsCount = nodeTemplate.mInputCount;
    const size_t OutputsCount = nodeTemplate.mOutputCount;

    /*
    for (int i = 0; i < 2; i++)
    {
        const size_t slotCount[2] = {InputsCount, OutputsCount};
        
        for (size_t slotIndex = 0; slotIndex < slotCount[i]; slotIndex++)
        {
            const char* con = i ? nodeTemplate.mOutputNames[slotIndex] : nodeTemplate.mInputNames[slotIndex];//node.mOutputs[slot_idx] : node->mInputs[slot_idx];
            if (!delegate->IsIOPinned(nodeIndex, slot_idx, i == 1))
            {
               
            }
            continue;

            LinearVec2 p = offset + (i ? GetOutputSlotPos(delegate, node, slotIndex, factor) : GetInputSlotPos(delegate, node, slotIndex, factor));
            const float arc = 28.f * (float(i) * 0.3f + 1.0f) * (i ? 1.f : -1.f);
            const float ofs = 0.f;

            LinearVec2 pts[3] = {p + LinearVec2(arc + ofs, 0.f), p + LinearVec2(0.f + ofs, -arc), p + LinearVec2(0.f + ofs, arc)};
            drawList->AddTriangleFilled(pts[0], pts[1], pts[2], i ? 0xFFAA5030 : 0xFF30AA50);
            drawList->AddTriangle(pts[0], pts[1], pts[2], 0xFF000000, 2.f);
        }
    }
    */

    LinearUI::SetCursorScreenPos(nodeRectangleMin);
    float maxHeight = LinearMin(viewPort.Max.y, nodeRectangleMin.y + nodeSize.y) - nodeRectangleMin.y;
    float maxWidth = LinearMin(viewPort.Max.x, nodeRectangleMin.x + nodeSize.x) - nodeRectangleMin.x;
    LinearUI::InvisibleButton("node", LinearVec2(maxWidth, maxHeight));
    // must be called right after creating the control we want to be able to move
    bool nodeMovingActive = LinearUI::IsItemActive();

    // Save the size of what we have emitted and whether any of the widgets are being used
    bool nodeWidgetsActive = (!old_any_active && LinearUI::IsAnyItemActive());
    LinearVec2 nodeRectangleMax = nodeRectangleMin + nodeSize;

    bool nodeHovered = false;
    if (LinearUI::IsItemHovered() && nodeOperation == NO_None && !overInput)
    {
        nodeHovered = true;
    }

    if (LinearUI::IsWindowFocused())
    {
        if ((nodeWidgetsActive || nodeMovingActive) && !inMinimap)
        {
            if (!node.mSelected)
            {
                if (!io.KeyShift)
                {
                    const auto nodeCount = delegate.GetNodeCount();
                    for (size_t i = 0; i < nodeCount; i++)
                    {
                        delegate.SelectNode(i, false);
                    }
                }
                delegate.SelectNode(nodeIndex, true);
            }
        }
    }
    if (nodeMovingActive && io.MouseDown[0] && nodeHovered && !inMinimap)
    {
        if (nodeOperation != NO_MovingNodes)
        {
            nodeOperation = NO_MovingNodes;
        }
    }

    const bool currentSelectedNode = node.mSelected;
    const LinearU32 node_bg_color = nodeHovered ? nodeTemplate.mBackgroundColorOver : nodeTemplate.mBackgroundColor;

#if LINEARUI_VERSION_NUM < 19276
    drawList->AddRect(nodeRectangleMin,
                      nodeRectangleMax,
                      currentSelectedNode ? options.mSelectedNodeBorderColor : options.mNodeBorderColor,
                      options.mRounding,
                      LinearDrawFlags_RoundCornersAll,
                      currentSelectedNode ? options.mBorderSelectionThickness : options.mBorderThickness);
#else
    drawList->AddRect(nodeRectangleMin,
                      nodeRectangleMax,
                      currentSelectedNode ? options.mSelectedNodeBorderColor : options.mNodeBorderColor,
                      options.mRounding,
                      currentSelectedNode ? options.mBorderSelectionThickness : options.mBorderThickness,
                      LinearDrawFlags_RoundCornersAll);
#endif

    LinearVec2 imgPos = nodeRectangleMin + LinearVec2(14, 25);
    LinearVec2 imgSize = nodeRectangleMax + LinearVec2(-5, -5) - imgPos;
    float imgSizeComp = std::min(imgSize.x, imgSize.y);

    drawList->AddRectFilled(nodeRectangleMin, nodeRectangleMax, node_bg_color, options.mRounding);
    /*float progress = delegate->NodeProgress(nodeIndex);
    if (progress > FLT_EPSILON && progress < 1.f - FLT_EPSILON)
    {
        LinearVec2 progressLineA = nodeRectangleMax - LinearVec2(nodeSize.x - 2.f, 3.f);
        LinearVec2 progressLineB = progressLineA + LinearVec2(nodeSize.x * factor - 4.f, 0.f);
        drawList->AddLine(progressLineA, progressLineB, 0xFF400000, 3.f);
        drawList->AddLine(progressLineA, LinearLerp(progressLineA, progressLineB, progress), 0xFFFF0000, 3.f);
    }*/
    LinearVec2 imgPosMax = imgPos + LinearVec2(imgSizeComp, imgSizeComp);

    //LinearVec2 imageSize = delegate->GetEvaluationSize(nodeIndex);
    /*float imageRatio = 1.f;
    if (imageSize.x > 0.f && imageSize.y > 0.f)
    {
        imageRatio = imageSize.y / imageSize.x;
    }
    LinearVec2 quadSize = imgPosMax - imgPos;
    LinearVec2 marge(0.f, 0.f);
    if (imageRatio > 1.f)
    {
        marge.x = (quadSize.x - quadSize.y / imageRatio) * 0.5f;
    }
    else
    {
        marge.y = (quadSize.y - quadSize.y * imageRatio) * 0.5f;
    }*/

    //delegate->DrawNodeImage(drawList, LinearRect(imgPos, imgPosMax), marge, nodeIndex);

    drawList->AddRectFilled(nodeRectangleMin,
                            LinearVec2(nodeRectangleMax.x, nodeRectangleMin.y + 20),
                            nodeTemplate.mHeaderColor, options.mRounding);

    drawList->PushClipRect(nodeRectangleMin, LinearVec2(nodeRectangleMax.x, nodeRectangleMin.y + 20), true);
    drawList->AddText(nodeRectangleMin + LinearVec2(2, 2), LINEARUI_COL32(0, 0, 0, 255), node.mName);
    drawList->PopClipRect();

    LinearRect customDrawRect(nodeRectangleMin + LinearVec2(options.mRounding, 20 + options.mRounding), nodeRectangleMax - LinearVec2(options.mRounding, options.mRounding));
    if (customDrawRect.Max.y > customDrawRect.Min.y && customDrawRect.Max.x > customDrawRect.Min.x)
    {
        delegate.CustomDraw(drawList, customDrawRect, nodeIndex);
    }
/*
    const LinearTextureID bmpInfo = (LinearTextureID)(uint64_t)delegate->GetBitmapInfo(nodeIndex).idx;
    if (bmpInfo)
    {
        LinearVec2 bmpInfoPos(nodeRectangleMax - LinearVec2(26, 12));
        LinearVec2 bmpInfoSize(20, 20);
        if (delegate->NodeIsCompute(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos,
                                   bmpInfoPos + LinearVec2(bmpInfoSize.x, 0.f),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + LinearVec2(0., bmpInfoSize.y));
        }
        else if (delegate->NodeIs2D(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos,
                                   bmpInfoPos + LinearVec2(bmpInfoSize.x, 0.f),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + LinearVec2(0., bmpInfoSize.y));
        }
        else if (delegate->NodeIsCubemap(nodeIndex))
        {
            drawList->AddImageQuad(bmpInfo,
                                   bmpInfoPos + LinearVec2(0., bmpInfoSize.y),
                                   bmpInfoPos + bmpInfoSize,
                                   bmpInfoPos + LinearVec2(bmpInfoSize.x, 0.f),
                                   bmpInfoPos);
        }
    }*/
    return nodeHovered;
}

bool DrawMiniMap(LinearDrawList* drawList, Delegate& delegate, ViewState& viewState, const Options& options, const LinearVec2 windowPos, const LinearVec2 canvasSize)
{
    if (Distance(options.mMinimap.Min, options.mMinimap.Max) <= FLT_EPSILON)
    {
        return false;
    }

    const size_t nodeCount = delegate.GetNodeCount();

    if (!nodeCount)
    {
        return false;
    }

    LinearVec2 min(FLT_MAX, FLT_MAX);
    LinearVec2 max(-FLT_MAX, -FLT_MAX);
    const LinearVec2 margin(50, 50);
    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        const Node& node = delegate.GetNode(nodeIndex);
        min = LinearMin(min, node.mRect.Min - margin);
        min = LinearMin(min, node.mRect.Max + margin);
        max = LinearMax(max, node.mRect.Min - margin);
        max = LinearMax(max, node.mRect.Max + margin);
    }

    // add view in world space
    const LinearVec2 worldSizeView = canvasSize / viewState.mFactor;
    const LinearVec2 viewMin(-viewState.mPosition.x, -viewState.mPosition.y);
    const LinearVec2 viewMax = viewMin + worldSizeView;
    min = LinearMin(min, viewMin);
    max = LinearMax(max, viewMax);
    const LinearVec2 nodesSize = max - min;
    const LinearVec2 middleWorld = (min + max) * 0.5f;
    const LinearVec2 minScreen = windowPos + options.mMinimap.Min * canvasSize;
    const LinearVec2 maxScreen = windowPos + options.mMinimap.Max * canvasSize;
    const LinearVec2 viewSize = maxScreen - minScreen;
    const LinearVec2 middleScreen = (minScreen + maxScreen) * 0.5f;
    const float ratioY = viewSize.y / nodesSize.y;
    const float ratioX = viewSize.x / nodesSize.x;
    const float factor = LinearMin(LinearMin(ratioY, ratioX), 1.f);

    drawList->AddRectFilled(minScreen, maxScreen, LINEARUI_COL32(30, 30, 30, 200), 3, LinearDrawFlags_RoundCornersAll);

    for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        const Node& node = delegate.GetNode(nodeIndex);
        const auto nodeTemplate = delegate.GetTemplate(node.mTemplateIndex);

        LinearRect rect = node.mRect;
        rect.Min -= middleWorld;
        rect.Min *= factor;
        rect.Min += middleScreen;

        rect.Max -= middleWorld;
        rect.Max *= factor;
        rect.Max += middleScreen;

        drawList->AddRectFilled(rect.Min, rect.Max, nodeTemplate.mBackgroundColor, 1, LinearDrawFlags_RoundCornersAll);
        if (node.mSelected)
        {
            drawList->AddRect(rect.Min, rect.Max, options.mSelectedNodeBorderColor, 1, LinearDrawFlags_RoundCornersAll);
        }
    }

    // add view
    LinearVec2 viewMinScreen = (viewMin - middleWorld) * factor + middleScreen;
    LinearVec2 viewMaxScreen = (viewMax - middleWorld) * factor + middleScreen;
    drawList->AddRectFilled(viewMinScreen, viewMaxScreen, LINEARUI_COL32(255, 255, 255, 32), 1, LinearDrawFlags_RoundCornersAll);
    drawList->AddRect(viewMinScreen, viewMaxScreen, LINEARUI_COL32(255, 255, 255, 128), 1, LinearDrawFlags_RoundCornersAll);
    
    LinearUIIO& io = LinearUI::GetIO();
    const bool mouseInMinimap = LinearRect(minScreen, maxScreen).Contains(io.MousePos);
    if (mouseInMinimap && io.MouseClicked[0])
    {
        const LinearVec2 clickedRatio = (io.MousePos - minScreen) / viewSize;
        const LinearVec2 worldPosCenter = LinearVec2(LinearLerp(min.x, max.x, clickedRatio.x), LinearLerp(min.y, max.y, clickedRatio.y));
        
        LinearVec2 worldPosViewMin = worldPosCenter - worldSizeView * 0.5;
        LinearVec2 worldPosViewMax = worldPosCenter + worldSizeView * 0.5;
        if (worldPosViewMin.x < min.x)
        {
            worldPosViewMin.x = min.x;
            worldPosViewMax.x = worldPosViewMin.x + worldSizeView.x;
        }
        if (worldPosViewMin.y < min.y)
        {
            worldPosViewMin.y = min.y;
            worldPosViewMax.y = worldPosViewMin.y + worldSizeView.y;
        }
        if (worldPosViewMax.x > max.x)
        {
            worldPosViewMax.x = max.x;
            worldPosViewMin.x = worldPosViewMax.x - worldSizeView.x;
        }
        if (worldPosViewMax.y > max.y)
        {
            worldPosViewMax.y = max.y;
            worldPosViewMin.y = worldPosViewMax.y - worldSizeView.y;
        }
        viewState.mPosition = LinearVec2(-worldPosViewMin.x, -worldPosViewMin.y);
    }
    return mouseInMinimap;
}

void Show(Delegate& delegate, const Options& options, ViewState& viewState, bool enabled, FitOnScreen* fit)
{
    LinearUI::PushStyleVar(LinearUIStyleVar_ChildBorderSize, 0.f);
    LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(0.f, 0.f));
    LinearUI::PushStyleVar(LinearUIStyleVar_FrameBorderSize, 0.f);

    const LinearVec2 windowPos = LinearUI::GetCursorScreenPos();
    const LinearVec2 canvasSize = LinearUI::GetContentRegionAvail();
    const LinearVec2 scrollRegionLocalPos(0, 0);

    LinearRect regionRect(windowPos, windowPos + canvasSize);

    HandleZoomScroll(regionRect, viewState, options);
    LinearVec2 offset = LinearUI::GetCursorScreenPos() + viewState.mPosition * viewState.mFactor;
    captureOffset = viewState.mPosition * viewState.mFactor;

    //LinearUI::InvisibleButton("GraphEditorButton", canvasSize);
    LinearUI::BeginChild(71711, canvasSize, LinearUIChildFlags_FrameStyle);

    LinearUI::SetCursorPos(windowPos);
    LinearUI::BeginGroup();

    LinearUIIO& io = LinearUI::GetIO();

    // Create our child canvas
    LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(1, 1));
    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0, 0));
    LinearUI::PushStyleColor(LinearUICol_ChildBg, LINEARUI_COL32(30, 30, 30, 200));

    LinearDrawList* drawList = LinearUI::GetWindowDrawList();
    LinearUI::PushClipRect(regionRect.Min, regionRect.Max, true);
    drawList->AddRectFilled(windowPos, windowPos + canvasSize, options.mBackgroundColor);

    // Background or Display grid
    if (options.mRenderGrid)
    {
        DrawGrid(drawList, windowPos, viewState, canvasSize, options.mGridColor, options.mGridColor2, options.mGridSize);
    }
    
    // Fit view
    if (fit && ((*fit == Fit_AllNodes) || (*fit == Fit_SelectedNodes)))
    {
        FitNodes(delegate, viewState, canvasSize, (*fit == Fit_SelectedNodes));
    }

    if (enabled)
    {
        static NodeIndex hoveredNode = -1;
        // Display links
        drawList->ChannelsSplit(3);

        // minimap
        drawList->ChannelsSetCurrent(2); // minimap
        const bool inMinimap = DrawMiniMap(drawList, delegate, viewState, options, windowPos, canvasSize);

        // Focus rectangle
        if (LinearUI::IsWindowFocused())
        {
#if LINEARUI_VERSION_NUM < 19276
           drawList->AddRect(regionRect.Min, regionRect.Max, options.mFrameFocus, 1.f, 0, 2.f);
#else
           drawList->AddRect(regionRect.Min, regionRect.Max, options.mFrameFocus, 1.f, 2.f);
#endif
        }

        drawList->ChannelsSetCurrent(1); // Background

        // Links
        DisplayLinks(delegate, drawList, offset, viewState.mFactor, regionRect, hoveredNode, options);

        // edit node link
        if (nodeOperation == NO_EditingLink)
        {
            LinearVec2 p1 = editingNodeSource;
            LinearVec2 p2 = io.MousePos;
            drawList->AddLine(p1, p2, LINEARUI_COL32(200, 200, 200, 255), 3.0f);
        }

        // Display nodes
        drawList->PushClipRect(regionRect.Min, regionRect.Max, true);
        hoveredNode = -1;
        
        SlotIndex inputSlotOver = -1;
        SlotIndex outputSlotOver = -1;
        NodeIndex nodeOver = -1;

        const auto nodeCount = delegate.GetNodeCount();
        for (int i = 0; i < 2; i++)
        {
            for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
            {
                //const auto* node = &nodes[nodeIndex];
                const auto node = delegate.GetNode(nodeIndex);
                if (node.mSelected != (i != 0))
                {
                    continue;
                }

                // node view clipping
                LinearRect nodeRect = GetNodeRect(node, viewState.mFactor);
                nodeRect.Min += offset;
                nodeRect.Max += offset;
                if (!regionRect.Overlaps(nodeRect))
                {
                    continue;
                }

                LinearUI::PushID((int)nodeIndex);
                SlotIndex inputSlot = -1;
                SlotIndex outputSlot = -1;

                bool overInput = (!inMinimap) && HandleConnections(drawList, nodeIndex, offset, viewState.mFactor, delegate, options, false, inputSlot, outputSlot, inMinimap);

                // shadow
                /*
                LinearVec2 shadowOffset = LinearVec2(30, 30);
                LinearVec2 shadowPivot = (nodeRect.Min + nodeRect.Max) /2.f;
                LinearVec2 shadowPointMiddle = shadowPivot + shadowOffset;
                LinearVec2 shadowPointTop = LinearVec2(shadowPivot.x, nodeRect.Min.y) + shadowOffset;
                LinearVec2 shadowPointBottom = LinearVec2(shadowPivot.x, nodeRect.Max.y) + shadowOffset;
                LinearVec2 shadowPointLeft = LinearVec2(nodeRect.Min.x, shadowPivot.y) + shadowOffset;
                LinearVec2 shadowPointRight = LinearVec2(nodeRect.Max.x, shadowPivot.y) + shadowOffset;

                // top left
                drawList->AddRectFilledMultiColor(nodeRect.Min + shadowOffset, shadowPointMiddle, LINEARUI_COL32(0 ,0, 0, 0), LINEARUI_COL32(0,0,0,0), LINEARUI_COL32(0, 0, 0, 255), LINEARUI_COL32(0, 0, 0, 0));

                // top right
                drawList->AddRectFilledMultiColor(shadowPointTop, shadowPointRight, LINEARUI_COL32(0 ,0, 0, 0), LINEARUI_COL32(0,0,0,0), LINEARUI_COL32(0, 0, 0, 0), LINEARUI_COL32(0, 0, 0, 255));

                // bottom left
                drawList->AddRectFilledMultiColor(shadowPointLeft, shadowPointBottom, LINEARUI_COL32(0 ,0, 0, 0), LINEARUI_COL32(0, 0, 0, 255), LINEARUI_COL32(0, 0, 0, 0), LINEARUI_COL32(0,0,0,0));

                // bottom right
                drawList->AddRectFilledMultiColor(shadowPointMiddle, nodeRect.Max + shadowOffset, LINEARUI_COL32(0, 0, 0, 255), LINEARUI_COL32(0 ,0, 0, 0), LINEARUI_COL32(0,0,0,0), LINEARUI_COL32(0, 0, 0, 0));
                */
                if (DrawNode(drawList, nodeIndex, offset, viewState.mFactor, delegate, overInput, options, inMinimap, regionRect))
                {
                    hoveredNode = nodeIndex;
                }

                HandleConnections(drawList, nodeIndex, offset, viewState.mFactor, delegate, options, true, inputSlot, outputSlot, inMinimap);
                if (inputSlot != -1 || outputSlot != -1)
                {
                    inputSlotOver = inputSlot;
                    outputSlotOver = outputSlot;
                    nodeOver = nodeIndex;
                }

                LinearUI::PopID();
            }
        }
        

        
        drawList->PopClipRect();

        if (nodeOperation == NO_MovingNodes)
        {
            if (LinearUI::IsMouseDragging(0, 1))
            {
                LinearVec2 delta = io.MouseDelta / viewState.mFactor;
                if (fabsf(delta.x) >= 1.f || fabsf(delta.y) >= 1.f)
                {
                    delegate.MoveSelectedNodes(delta);
                }
            }
        }

        drawList->ChannelsSetCurrent(0);

        // quad selection
        if (!inMinimap)
        {
            HandleQuadSelection(delegate, drawList, offset, viewState.mFactor, regionRect, options);
        }

        drawList->ChannelsMerge();

        // releasing mouse button means it's done in any operation
        if (nodeOperation == NO_PanView)
        {
            if (!io.MouseDown[2])
            {
                nodeOperation = NO_None;
            }
        }
        else if (nodeOperation != NO_None && !io.MouseDown[0])
        {
            nodeOperation = NO_None;
        }

        // right click
        if (!inMinimap && nodeOperation == NO_None && regionRect.Contains(io.MousePos) &&
                (LinearUI::IsMouseClicked(1) /*|| (LinearUI::IsWindowFocused() && LinearUI::IsKeyPressedMap(LinearUIKey_Tab))*/))
        {
            delegate.RightClick(nodeOver, inputSlotOver, outputSlotOver);
        }

        // Scrolling
        if (LinearUI::IsWindowHovered() && !LinearUI::IsAnyItemActive() && io.MouseClicked[2] && nodeOperation == NO_None)
        {
            nodeOperation = NO_PanView;
        }
        if (nodeOperation == NO_PanView)
        {
            viewState.mPosition += io.MouseDelta / viewState.mFactor;
        }
    }

    LinearUI::PopClipRect();

    LinearUI::PopStyleColor(1);
    LinearUI::PopStyleVar(2);
    LinearUI::EndGroup();
    LinearUI::EndChild();

    LinearUI::PopStyleVar(3);
    
    // change fit to none
    if (fit)
    {
        *fit = Fit_None;
    }
}

bool EditOptions(Options& options)
{
    bool updated = false;
    if (LinearUI::CollapsingHeader("Colors", nullptr))
    {
        LinearColor backgroundColor(options.mBackgroundColor);
        LinearColor gridColor(options.mGridColor);
        LinearColor selectedNodeBorderColor(options.mSelectedNodeBorderColor);
        LinearColor nodeBorderColor(options.mNodeBorderColor);
        LinearColor quadSelection(options.mQuadSelection);
        LinearColor quadSelectionBorder(options.mQuadSelectionBorder);
        LinearColor defaultSlotColor(options.mDefaultSlotColor);
        LinearColor frameFocus(options.mFrameFocus);

        updated |= LinearUI::ColorEdit4("Background", (float*)&backgroundColor);
        updated |= LinearUI::ColorEdit4("Grid", (float*)&gridColor);
        updated |= LinearUI::ColorEdit4("Selected Node Border", (float*)&selectedNodeBorderColor);
        updated |= LinearUI::ColorEdit4("Node Border", (float*)&nodeBorderColor);
        updated |= LinearUI::ColorEdit4("Quad Selection", (float*)&quadSelection);
        updated |= LinearUI::ColorEdit4("Quad Selection Border", (float*)&quadSelectionBorder);
        updated |= LinearUI::ColorEdit4("Default Slot", (float*)&defaultSlotColor);
        updated |= LinearUI::ColorEdit4("Frame when has focus", (float*)&frameFocus);

        options.mBackgroundColor = backgroundColor;
        options.mGridColor = gridColor;
        options.mSelectedNodeBorderColor = selectedNodeBorderColor;
        options.mNodeBorderColor = nodeBorderColor;
        options.mQuadSelection = quadSelection;
        options.mQuadSelectionBorder = quadSelectionBorder;
        options.mDefaultSlotColor = defaultSlotColor;
        options.mFrameFocus = frameFocus;
    }

    if (LinearUI::CollapsingHeader("Options", nullptr))
    {
        updated |= LinearUI::InputFloat4("Minimap", &options.mMinimap.Min.x);
        updated |= LinearUI::InputFloat("Line Thickness", &options.mLineThickness);
        updated |= LinearUI::InputFloat("Grid Size", &options.mGridSize);
        updated |= LinearUI::InputFloat("Rounding", &options.mRounding);
        updated |= LinearUI::InputFloat("Zoom Ratio", &options.mZoomRatio);
        updated |= LinearUI::InputFloat("Zoom Lerp Factor", &options.mZoomLerpFactor);
        updated |= LinearUI::InputFloat("Border Selection Thickness", &options.mBorderSelectionThickness);
        updated |= LinearUI::InputFloat("Border Thickness", &options.mBorderThickness);
        updated |= LinearUI::InputFloat("Slot Radius", &options.mNodeSlotRadius);
        updated |= LinearUI::InputFloat("Slot Hover Factor", &options.mNodeSlotHoverFactor);
        updated |= LinearUI::InputFloat2("Zoom min/max", &options.mMinZoom);
        updated |= LinearUI::InputFloat("Slot Hover Factor", &options.mSnap);
        
        if (LinearUI::RadioButton("Curved Links", options.mDisplayLinksAsCurves))
        {
            options.mDisplayLinksAsCurves = !options.mDisplayLinksAsCurves;
            updated = true;
        }
        if (LinearUI::RadioButton("Straight Links", !options.mDisplayLinksAsCurves))
        {
            options.mDisplayLinksAsCurves = !options.mDisplayLinksAsCurves;
            updated = true;
        }

        updated |= LinearUI::Checkbox("Allow Quad Selection", &options.mAllowQuadSelection);
        updated |= LinearUI::Checkbox("Render Grid", &options.mRenderGrid);
        updated |= LinearUI::Checkbox("Draw IO names on hover", &options.mDrawIONameOnHover);
    }

    return updated;
}

} // namespace
