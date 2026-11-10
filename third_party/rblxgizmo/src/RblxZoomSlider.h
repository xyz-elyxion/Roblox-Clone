// https://github.com/CedricGuillemet/RblxGizmo
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

namespace RblxZoomSlider
{
   typedef int RblxUIZoomSliderFlags;
   enum RblxUIPopupFlags_
   {
      RblxUIZoomSliderFlags_None = 0,
      RblxUIZoomSliderFlags_Vertical = 1,
      RblxUIZoomSliderFlags_NoAnchors = 2,
      RblxUIZoomSliderFlags_NoMiddleCarets = 4,
      RblxUIZoomSliderFlags_NoWheel = 8,
   };

   template<typename T> bool RblxZoomSlider(const T lower, const T higher, T& viewLower, T& viewHigher, float wheelRatio = 0.01f, RblxUIZoomSliderFlags flags = RblxUIZoomSliderFlags_None)
   {
      bool interacted = false;
      RblxUIIO& io = RblxUI::GetIO();
      RblxDrawList* draw_list = RblxUI::GetWindowDrawList();

      static const float handleSize = 12;
      static const float roundRadius = 3.f;
      static const char* controlName = "RblxZoomSlider";

      static bool movingScrollBarSvg = false;
      static bool sizingRBarSvg = false;
      static bool sizingLBarSvg = false;
      static RblxUIID editingId = (RblxUIID)-1;
      static float scrollingSource = 0.f;
      static float saveViewLower;
      static float saveViewHigher;

      const bool isVertical = flags & RblxUIZoomSliderFlags_Vertical;
      const RblxVec2 canvasPos = RblxUI::GetCursorScreenPos();
      const RblxVec2 canvasSize = RblxUI::GetContentRegionAvail();
      const float canvasSizeLength = isVertical ? RblxUI::GetItemRectSize().y : canvasSize.x;
      const RblxVec2 scrollBarSize = isVertical ? RblxVec2(14.f, canvasSizeLength) : RblxVec2(canvasSizeLength, 14.f);

      RblxUI::InvisibleButton(controlName, scrollBarSize);
      const RblxUIID currentId = RblxUI::GetID(controlName);

      const bool usingEditingId = currentId == editingId;
      const bool canUseControl = usingEditingId || editingId == -1;
      const bool movingScrollBar = usingEditingId ? movingScrollBarSvg : false;
      const bool sizingRBar = usingEditingId ? sizingRBarSvg : false;
      const bool sizingLBar = usingEditingId ? sizingLBarSvg : false;
      const int componentIndex = isVertical ? 1 : 0;
      const RblxVec2 scrollBarMin = RblxUI::GetItemRectMin();
      const RblxVec2 scrollBarMax = RblxUI::GetItemRectMax();
      const RblxVec2 scrollBarA = RblxVec2(scrollBarMin.x, scrollBarMin.y) - (isVertical ? RblxVec2(2,0) : RblxVec2(0,2));
      const RblxVec2 scrollBarB = isVertical ? RblxVec2(scrollBarMax.x - 1.f, scrollBarMin.y + canvasSizeLength) : RblxVec2(scrollBarMin.x + canvasSizeLength, scrollBarMax.y - 1.f);
      const float scrollStart = ((viewLower - lower) / (higher - lower)) * canvasSizeLength + scrollBarMin[componentIndex];
      const float scrollEnd = ((viewHigher - lower) / (higher - lower)) * canvasSizeLength + scrollBarMin[componentIndex];
      const float screenSize = scrollEnd - scrollStart;
      const RblxVec2 scrollTopLeft = isVertical ? RblxVec2(scrollBarMin.x, scrollStart) : RblxVec2(scrollStart, scrollBarMin.y);
      const RblxVec2 scrollBottomRight = isVertical ? RblxVec2(scrollBarMax.x - 2.f, scrollEnd) : RblxVec2(scrollEnd, scrollBarMax.y - 2.f);
      const bool inScrollBar = canUseControl && RblxRect(scrollTopLeft, scrollBottomRight).Contains(io.MousePos);
      const RblxRect scrollBarRect(scrollBarA, scrollBarB);
      const float deltaScreen = io.MousePos[componentIndex] - scrollingSource;
      const float deltaView = ((higher - lower) / canvasSizeLength) * deltaScreen;
      const RblxU32 barColor = RblxUI::GetColorU32((inScrollBar || movingScrollBar) ? RblxUICol_FrameBgHovered : RblxUICol_FrameBg);
      const float middleCoord = (scrollStart + scrollEnd) * 0.5f;
      const bool insideControl = canUseControl && RblxRect(scrollBarMin, scrollBarMax).Contains(io.MousePos);
      const bool hasAnchors = !(flags & RblxUIZoomSliderFlags_NoAnchors);
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

      if (!(flags & RblxUIZoomSliderFlags_NoMiddleCarets))
      {
         for (float i = 0.5f; i < 3.f; i += 1.f)
         {
            const float coordA = middleCoord - handleSize * 0.5f;
            const float coordB = middleCoord + handleSize * 0.5f;
            RblxVec2 base = scrollBarMin;
            base.x += scrollBarSize.x * 0.25f * i;
            base.y += scrollBarSize.y * 0.25f * i;

            if (isVertical)
            {
               draw_list->AddLine(RblxVec2(base.x, coordA), RblxVec2(base.x, coordB), RblxUI::GetColorU32(RblxUICol_SliderGrab));
            }
            else
            {
               draw_list->AddLine(RblxVec2(coordA, base.y), RblxVec2(coordB, base.y), RblxUI::GetColorU32(RblxUICol_SliderGrab));
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

      if (!(flags & RblxUIZoomSliderFlags_NoWheel) && inScrollBar && fabsf(io.MouseWheel) > 0.f)
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
         const RblxRect barHandleLeft(scrollTopLeft, isVertical ? RblxVec2(scrollBottomRight.x, scrollTopLeft.y + handleSize) : RblxVec2(scrollTopLeft.x + handleSize, scrollBottomRight.y));
         const RblxRect barHandleRight(isVertical ? RblxVec2(scrollTopLeft.x, scrollBottomRight.y - handleSize) : RblxVec2(scrollBottomRight.x - handleSize, scrollTopLeft.y), scrollBottomRight);

         onLeft = barHandleLeft.Contains(io.MousePos);
         onRight = barHandleRight.Contains(io.MousePos);

         draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max, RblxUI::GetColorU32((onLeft || sizingLBar) ? RblxUICol_SliderGrabActive : RblxUICol_SliderGrab), roundRadius);
         draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max, RblxUI::GetColorU32((onRight || sizingRBar) ? RblxUICol_SliderGrabActive : RblxUICol_SliderGrab), roundRadius);
      }

      if (sizingRBar)
      {
         if (!io.MouseDown[0])
         {
            sizingRBarSvg = false;
            editingId = (RblxUIID)-1;
         }
         else
         {
            viewHigher = RblxMin(saveViewHigher + deltaView, higher);
         }
      }
      else if (sizingLBar)
      {
         if (!io.MouseDown[0])
         {
            sizingLBarSvg = false;
            editingId = (RblxUIID)-1;
         }
         else
         {
            viewLower = RblxMax(saveViewLower + deltaView, lower);
         }
      }
      else
      {
         if (movingScrollBar)
         {
            if (!io.MouseDown[0])
            {
               movingScrollBarSvg = false;
               editingId = (RblxUIID)-1;
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
            if (inScrollBar && RblxUI::IsMouseClicked(0))
            {
               movingScrollBarSvg = true;
               scrollingSource = io.MousePos[componentIndex];
               saveViewLower = viewLower;
               saveViewHigher = viewHigher;
               editingId = currentId;
            }
            if (!sizingRBar && onRight && RblxUI::IsMouseClicked(0) && hasAnchors)
            {
               sizingRBarSvg = true;
               editingId = currentId;
            }
            if (!sizingLBar && onLeft && RblxUI::IsMouseClicked(0) && hasAnchors)
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
