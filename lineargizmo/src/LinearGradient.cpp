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
#include "LinearGradient.h"
#include "linearui.h"
#include "linearui_internal.h"

namespace LinearGradient
{
#ifndef LINEARUI_DEFINE_MATH_OPERATORS
   static inline LinearVec2 operator*(const LinearVec2& lhs, const float rhs) { return LinearVec2(lhs.x * rhs, lhs.y * rhs); }
   static inline LinearVec2 operator/(const LinearVec2& lhs, const float rhs) { return LinearVec2(lhs.x / rhs, lhs.y / rhs); }
   static inline LinearVec2 operator+(const LinearVec2& lhs, const LinearVec2& rhs) { return LinearVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
   static inline LinearVec2 operator-(const LinearVec2& lhs, const LinearVec2& rhs) { return LinearVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
   static inline LinearVec2 operator*(const LinearVec2& lhs, const LinearVec2& rhs) { return LinearVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
   static inline LinearVec2 operator/(const LinearVec2& lhs, const LinearVec2& rhs) { return LinearVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
#endif

   static int DrawPoint(LinearDrawList* draw_list, LinearVec4 color, const LinearVec2 size, bool editing, LinearVec2 pos)
   {
      LinearUIIO& io = LinearUI::GetIO();

      LinearVec2 p1 = LinearLerp(pos, LinearVec2(pos + LinearVec2(size.x - size.y, 0.f)), color.w) + LinearVec2(3, 3);
      LinearVec2 p2 = LinearLerp(pos + LinearVec2(size.y, size.y), LinearVec2(pos + size), color.w) - LinearVec2(3, 3);
      LinearRect rc(p1, p2);

      color.w = 1.f;
      draw_list->AddRectFilled(p1, p2, LinearColor(color));
#if LINEARUI_VERSION_NUM < 19276
      if (editing)
         draw_list->AddRect(p1, p2, 0xFFFFFFFF, 2.f, 15, 2.5f);
      else
         draw_list->AddRect(p1, p2, 0x80FFFFFF, 2.f, 15, 1.25f);
#else
      if (editing)
         draw_list->AddRect(p1, p2, 0xFFFFFFFF, 2.f, 2.5f);
      else
         draw_list->AddRect(p1, p2, 0x80FFFFFF, 2.f, 1.25f);
#endif

      if (rc.Contains(io.MousePos))
      {
         if (io.MouseClicked[0])
            return 2;
         return 1;
      }
      return 0;
   }

   bool Edit(Delegate& delegate, const LinearVec2& size, int& selection)
   {
      bool ret = false;
      LinearUIIO& io = LinearUI::GetIO();
      LinearUI::PushStyleVar(LinearUIStyleVar_FramePadding, LinearVec2(0, 0));
      LinearUI::BeginChild(137, size, LinearUIChildFlags_FrameStyle);

      LinearDrawList* draw_list = LinearUI::GetWindowDrawList();
      const LinearVec2 offset = LinearUI::GetCursorScreenPos();

      const LinearVec4* pts = delegate.GetPoints();
      static int currentSelection = -1;
      static int movingPt = -1;
      if (currentSelection >= int(delegate.GetPointCount()))
         currentSelection = -1;
      if (movingPt != -1)
      {
         LinearVec4 current = pts[movingPt];
         current.w += io.MouseDelta.x / size.x;
         current.w = LinearClamp(current.w, 0.f, 1.f);
         delegate.EditPoint(movingPt, current);
         ret = true;
         if (!io.MouseDown[0])
            movingPt = -1;
      }
      for (size_t i = 0; i < delegate.GetPointCount(); i++)
      {
         int ptSel = DrawPoint(draw_list, pts[i], size, i == currentSelection, offset);
         if (ptSel == 2)
         {
            currentSelection = int(i);
            ret = true;
         }
         if (ptSel == 1 && io.MouseDown[0] && movingPt == -1)
         {
            movingPt = int(i);
         }
      }
      LinearRect rc(offset, offset + size);
      if (rc.Contains(io.MousePos) && io.MouseDoubleClicked[0])
      {
         float t = (io.MousePos.x - offset.x) / size.x;
         delegate.AddPoint(delegate.GetPoint(t));
         ret = true;
      }
      LinearUI::EndChild();
      LinearUI::PopStyleVar();

      selection = currentSelection;
      return ret;
   }
}
