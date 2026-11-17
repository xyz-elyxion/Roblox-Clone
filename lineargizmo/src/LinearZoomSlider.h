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
#pragma once

namespace LinearZoomSlider
{
   typedef int LinearUIZoomSliderFlags;
   enum LinearUIPopupFlags_
   {
      LinearUIZoomSliderFlags_None = 0,
      LinearUIZoomSliderFlags_Vertical = 1,
      LinearUIZoomSliderFlags_NoAnchors = 2,
      LinearUIZoomSliderFlags_NoMiddleCarets = 4,
      LinearUIZoomSliderFlags_NoWheel = 8,
   };

   template<typename T> bool LinearZoomSlider(const T lower, const T higher, T& viewLower, T& viewHigher, float wheelRatio = 0.01f, LinearUIZoomSliderFlags flags = LinearUIZoomSliderFlags_None)
   {
      bool interacted = false;
      LinearUIIO& io = LinearUI::GetIO();
      LinearDrawList* draw_list = LinearUI::GetWindowDrawList();

      static const float handleSize = 12;
      static const float roundRadius = 3.f;
      static const char* controlName = "LinearZoomSlider";

      static bool movingScrollBarSvg = false;
      static bool sizingRBarSvg = false;
      static bool sizingLBarSvg = false;
      static LinearUIID editingId = (LinearUIID)-1;
      static float scrollingSource = 0.f;
      static float saveViewLower;
      static float saveViewHigher;

      const bool isVertical = flags & LinearUIZoomSliderFlags_Vertical;
      const LinearVec2 canvasPos = LinearUI::GetCursorScreenPos();
      const LinearVec2 canvasSize = LinearUI::GetContentRegionAvail();
      const float canvasSizeLength = isVertical ? LinearUI::GetItemRectSize().y : canvasSize.x;
      const LinearVec2 scrollBarSize = isVertical ? LinearVec2(14.f, canvasSizeLength) : LinearVec2(canvasSizeLength, 14.f);

      LinearUI::InvisibleButton(controlName, scrollBarSize);
      const LinearUIID currentId = LinearUI::GetID(controlName);

      const bool usingEditingId = currentId == editingId;
      const bool canUseControl = usingEditingId || editingId == -1;
      const bool movingScrollBar = usingEditingId ? movingScrollBarSvg : false;
      const bool sizingRBar = usingEditingId ? sizingRBarSvg : false;
      const bool sizingLBar = usingEditingId ? sizingLBarSvg : false;
      const int componentIndex = isVertical ? 1 : 0;
      const LinearVec2 scrollBarMin = LinearUI::GetItemRectMin();
      const LinearVec2 scrollBarMax = LinearUI::GetItemRectMax();
      const LinearVec2 scrollBarA = LinearVec2(scrollBarMin.x, scrollBarMin.y) - (isVertical ? LinearVec2(2,0) : LinearVec2(0,2));
      const LinearVec2 scrollBarB = isVertical ? LinearVec2(scrollBarMax.x - 1.f, scrollBarMin.y + canvasSizeLength) : LinearVec2(scrollBarMin.x + canvasSizeLength, scrollBarMax.y - 1.f);
      const float scrollStart = ((viewLower - lower) / (higher - lower)) * canvasSizeLength + scrollBarMin[componentIndex];
      const float scrollEnd = ((viewHigher - lower) / (higher - lower)) * canvasSizeLength + scrollBarMin[componentIndex];
      const float screenSize = scrollEnd - scrollStart;
      const LinearVec2 scrollTopLeft = isVertical ? LinearVec2(scrollBarMin.x, scrollStart) : LinearVec2(scrollStart, scrollBarMin.y);
      const LinearVec2 scrollBottomRight = isVertical ? LinearVec2(scrollBarMax.x - 2.f, scrollEnd) : LinearVec2(scrollEnd, scrollBarMax.y - 2.f);
      const bool inScrollBar = canUseControl && LinearRect(scrollTopLeft, scrollBottomRight).Contains(io.MousePos);
      const LinearRect scrollBarRect(scrollBarA, scrollBarB);
      const float deltaScreen = io.MousePos[componentIndex] - scrollingSource;
      const float deltaView = ((higher - lower) / canvasSizeLength) * deltaScreen;
      const LinearU32 barColor = LinearUI::GetColorU32((inScrollBar || movingScrollBar) ? LinearUICol_FrameBgHovered : LinearUICol_FrameBg);
      const float middleCoord = (scrollStart + scrollEnd) * 0.5f;
      const bool insideControl = canUseControl && LinearRect(scrollBarMin, scrollBarMax).Contains(io.MousePos);
      const bool hasAnchors = !(flags & LinearUIZoomSliderFlags_NoAnchors);
      const float viewMinSize = ((3.f * handleSize) / canvasSizeLength) * (higher - lower);
      const auto ClipView = [lower, higher, &viewLower, &viewHigher]() {
         if (viewLower < lower)
         {
            const float deltaClip = lower - viewLower;
            viewLower += deltaClip;
            viewHigher += deltaClip;
         }
         if (viewHigher > higher)
         {
            const float deltaClip = viewHigher - higher;
            viewLower -= deltaClip;
            viewHigher -= deltaClip;
         }
      };

      bool onLeft = false;
      bool onRight = false;

      draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, roundRadius);
      draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);
      draw_list->AddRectFilled(scrollTopLeft, scrollBottomRight, barColor, roundRadius);

      if (!(flags & LinearUIZoomSliderFlags_NoMiddleCarets))
      {
         for (float i = 0.5f; i < 3.f; i += 1.f)
         {
            const float coordA = middleCoord - handleSize * 0.5f;
            const float coordB = middleCoord + handleSize * 0.5f;
            LinearVec2 base = scrollBarMin;
            base.x += scrollBarSize.x * 0.25f * i;
            base.y += scrollBarSize.y * 0.25f * i;

            if (isVertical)
            {
               draw_list->AddLine(LinearVec2(base.x, coordA), LinearVec2(base.x, coordB), LinearUI::GetColorU32(LinearUICol_SliderGrab));
            }
            else
            {
               draw_list->AddLine(LinearVec2(coordA, base.y), LinearVec2(coordB, base.y), LinearUI::GetColorU32(LinearUICol_SliderGrab));
            }
         }
      }

      // Mouse wheel
      if (io.MouseClicked[0] && insideControl && !inScrollBar)
      {
         const float ratio = (io.MousePos[componentIndex] - scrollBarMin[componentIndex]) / (scrollBarMax[componentIndex] - scrollBarMin[componentIndex]);
         const float size = (higher - lower);
         const float halfViewSize = (viewHigher - viewLower) * 0.5f;
         const float middle = ratio * size + lower;
         viewLower = middle - halfViewSize;
         viewHigher = middle + halfViewSize;
         ClipView();
         interacted = true;
      }

      if (!(flags & LinearUIZoomSliderFlags_NoWheel) && inScrollBar && fabsf(io.MouseWheel) > 0.f)
      {
         const float ratio = (io.MousePos[componentIndex] - scrollStart) / (scrollEnd - scrollStart);
         const float amount = io.MouseWheel * wheelRatio * (viewHigher - viewLower);
         
         viewLower -= ratio * amount;
         viewHigher += (1.f - ratio) * amount;
         ClipView();
         interacted = true;
      }

      if (screenSize > handleSize * 2.f && hasAnchors)
      {
         const LinearRect barHandleLeft(scrollTopLeft, isVertical ? LinearVec2(scrollBottomRight.x, scrollTopLeft.y + handleSize) : LinearVec2(scrollTopLeft.x + handleSize, scrollBottomRight.y));
         const LinearRect barHandleRight(isVertical ? LinearVec2(scrollTopLeft.x, scrollBottomRight.y - handleSize) : LinearVec2(scrollBottomRight.x - handleSize, scrollTopLeft.y), scrollBottomRight);

         onLeft = barHandleLeft.Contains(io.MousePos);
         onRight = barHandleRight.Contains(io.MousePos);

         draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max, LinearUI::GetColorU32((onLeft || sizingLBar) ? LinearUICol_SliderGrabActive : LinearUICol_SliderGrab), roundRadius);
         draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max, LinearUI::GetColorU32((onRight || sizingRBar) ? LinearUICol_SliderGrabActive : LinearUICol_SliderGrab), roundRadius);
      }

      if (sizingRBar)
      {
         if (!io.MouseDown[0])
         {
            sizingRBarSvg = false;
            editingId = (LinearUIID)-1;
         }
         else
         {
            viewHigher = LinearMin(saveViewHigher + deltaView, higher);
         }
      }
      else if (sizingLBar)
      {
         if (!io.MouseDown[0])
         {
            sizingLBarSvg = false;
            editingId = (LinearUIID)-1;
         }
         else
         {
            viewLower = LinearMax(saveViewLower + deltaView, lower);
         }
      }
      else
      {
         if (movingScrollBar)
         {
            if (!io.MouseDown[0])
            {
               movingScrollBarSvg = false;
               editingId = (LinearUIID)-1;
            }
            else
            {
               viewLower = saveViewLower + deltaView;
               viewHigher = saveViewHigher + deltaView;
               ClipView();
            }
         }
         else
         {
            if (inScrollBar && LinearUI::IsMouseClicked(0))
            {
               movingScrollBarSvg = true;
               scrollingSource = io.MousePos[componentIndex];
               saveViewLower = viewLower;
               saveViewHigher = viewHigher;
               editingId = currentId;
            }
            if (!sizingRBar && onRight && LinearUI::IsMouseClicked(0) && hasAnchors)
            {
               sizingRBarSvg = true;
               editingId = currentId;
            }
            if (!sizingLBar && onLeft && LinearUI::IsMouseClicked(0) && hasAnchors)
            {
               sizingLBarSvg = true;
               editingId = currentId;
            }
         }
      }

      // minimal size check
      if ((viewHigher - viewLower) < viewMinSize)
      {
         const float middle = (viewLower + viewHigher) * 0.5f;
         viewLower = middle - viewMinSize * 0.5f;
         viewHigher = middle + viewMinSize * 0.5f;
         ClipView();
      }

      return movingScrollBar || sizingRBar || sizingLBar || interacted;
   }

} // namespace
