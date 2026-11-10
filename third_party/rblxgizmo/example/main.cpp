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
#define RBLXUI_DEFINE_MATH_OPERATORS
#include "rblxui.h"
#include "rblxui_internal.h"
#define IMAPP_IMPL
#include "RblxApp.h"

#ifdef None
#undef None
#endif

#include "RblxGizmo.h"
#include "RblxSequencer.h"
#include "RblxZoomSlider.h"
#include "RblxCurveEdit.h"
#include "RblxVectorEditor.h"
#include "GraphEditor.h"
#include "RblxLightRig.h"
#include <cmath>
#include <vector>
#include <algorithm>

bool useWindow = true;
int gizmoCount = 1;
float camDistance = 8.f;
float camYAngle = 165.f / 180.f * 3.14159f;
float camXAngle = 32.f / 180.f * 3.14159f;
static RblxGizmo::OPERATION mCurrentGizmoOperation(RblxGizmo::TRANSLATE);
static RblxGizmo::MODE mCurrentGizmoMode(RblxGizmo::WORLD);
static bool useSnap(false);
static float snap[3] = { 1.f, 1.f, 1.f };
static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
static bool boundSizingSnap = false;

float objectMatrix[4][16] = {
  { 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  2.f, 0.f, 0.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  2.f, 0.f, 2.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 2.f, 1.f }
};

static const float identityMatrix[16] =
{ 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f };

void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float* m16, bool rightHanded)
{
   float temp = 2.0f * znear;
   float temp2 = right - left;
   float temp3 = top - bottom;
   float temp4 = zfar - znear;
   float sign = rightHanded ? -1.0f : 1.0f;
   m16[0] = temp / temp2;
   m16[1] = 0.0f;
   m16[2] = 0.0f;
   m16[3] = 0.0f;
   m16[4] = 0.0f;
   m16[5] = temp / temp3;
   m16[6] = 0.0f;
   m16[7] = 0.0f;
   m16[8] = (right + left) / temp2;
   m16[9] = (top + bottom) / temp3;
   m16[10] = sign * (zfar + znear) / temp4;
   m16[11] = sign;
   m16[12] = 0.0f;
   m16[13] = 0.0f;
   m16[14] = -(temp * zfar) / temp4;
   m16[15] = 0.0f;
}

void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float* m16, bool rightHanded = true, bool infiniteFarPlane = false)
{
   float ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
   float xmax = ymax * aspectRatio;
   if (infiniteFarPlane)
   {
      float sign = rightHanded ? -1.0f : 1.0f;
      float temp = 2.0f * znear;
      float temp2 = 2.0f * xmax;
      float temp3 = 2.0f * ymax;
      m16[0] = temp / temp2;
      m16[1] = 0.0f;
      m16[2] = 0.0f;
      m16[3] = 0.0f;
      m16[4] = 0.0f;
      m16[5] = temp / temp3;
      m16[6] = 0.0f;
      m16[7] = 0.0f;
      m16[8] = 0.0f;
      m16[9] = 0.0f;
      m16[10] = sign;
      m16[11] = sign;
      m16[12] = 0.0f;
      m16[13] = 0.0f;
      // Keep the Z translation term negative for both handedness modes.
      // Using +2n in LH flips clipping orientation and appears like inverted winding.
      m16[14] = -temp;
      m16[15] = 0.0f;
   }
   else
   {
      Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16, rightHanded);
   }
}

void Cross(const float* a, const float* b, float* r)
{
   r[0] = a[1] * b[2] - a[2] * b[1];
   r[1] = a[2] * b[0] - a[0] * b[2];
   r[2] = a[0] * b[1] - a[1] * b[0];
}

float Dot(const float* a, const float* b)
{
   return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void Normalize(const float* a, float* r)
{
   float il = 1.f / (sqrtf(Dot(a, a)) + FLT_EPSILON);
   r[0] = a[0] * il;
   r[1] = a[1] * il;
   r[2] = a[2] * il;
}

void LookAt(const float* eye, const float* at, const float* up, float* m16, bool rightHanded = true)
{
   float X[3], Y[3], Z[3], tmp[3];

   if (rightHanded)
   {
      tmp[0] = eye[0] - at[0];
      tmp[1] = eye[1] - at[1];
      tmp[2] = eye[2] - at[2];
   }
   else
   {
      tmp[0] = at[0] - eye[0];
      tmp[1] = at[1] - eye[1];
      tmp[2] = at[2] - eye[2];
   }
   Normalize(tmp, Z);
   Normalize(up, Y);

   Cross(Y, Z, tmp);
   Normalize(tmp, X);

   Cross(Z, X, tmp);
   Normalize(tmp, Y);

   m16[0] = X[0];
   m16[1] = Y[0];
   m16[2] = Z[0];
   m16[3] = 0.0f;
   m16[4] = X[1];
   m16[5] = Y[1];
   m16[6] = Z[1];
   m16[7] = 0.0f;
   m16[8] = X[2];
   m16[9] = Y[2];
   m16[10] = Z[2];
   m16[11] = 0.0f;
   m16[12] = -Dot(X, eye);
   m16[13] = -Dot(Y, eye);
   m16[14] = -Dot(Z, eye);
   m16[15] = 1.0f;
}

void OrthoGraphic(const float l, float r, float b, const float t, float zn, const float zf, float* m16)
{
   m16[0] = 2 / (r - l);
   m16[1] = 0.0f;
   m16[2] = 0.0f;
   m16[3] = 0.0f;
   m16[4] = 0.0f;
   m16[5] = 2 / (t - b);
   m16[6] = 0.0f;
   m16[7] = 0.0f;
   m16[8] = 0.0f;
   m16[9] = 0.0f;
   m16[10] = 1.0f / (zf - zn);
   m16[11] = 0.0f;
   m16[12] = (l + r) / (l - r);
   m16[13] = (t + b) / (b - t);
   m16[14] = zn / (zn - zf);
   m16[15] = 1.0f;
}

inline void rotationY(const float angle, float* m16)
{
   float c = cosf(angle);
   float s = sinf(angle);

   m16[0] = c;
   m16[1] = 0.0f;
   m16[2] = -s;
   m16[3] = 0.0f;
   m16[4] = 0.0f;
   m16[5] = 1.f;
   m16[6] = 0.0f;
   m16[7] = 0.0f;
   m16[8] = s;
   m16[9] = 0.0f;
   m16[10] = c;
   m16[11] = 0.0f;
   m16[12] = 0.f;
   m16[13] = 0.f;
   m16[14] = 0.f;
   m16[15] = 1.0f;
}

void TransformStart(float* cameraView, float* cameraProjection, float* matrix, bool rightHanded)
{
    if (RblxUI::IsKeyPressed(RblxUIKey_T))
        mCurrentGizmoOperation = RblxGizmo::TRANSLATE;
    if (RblxUI::IsKeyPressed(RblxUIKey_E))
        mCurrentGizmoOperation = RblxGizmo::ROTATE;
    if (RblxUI::IsKeyPressed(RblxUIKey_R)) // r Key
        mCurrentGizmoOperation = RblxGizmo::SCALE;
    bool translateActive = (mCurrentGizmoOperation & RblxGizmo::TRANSLATE) != 0;
    bool rotateActive   = (mCurrentGizmoOperation & RblxGizmo::ROTATE) != 0;
    bool scaleActive    = (mCurrentGizmoOperation & RblxGizmo::SCALE) != 0;
    bool boundsActive   = (mCurrentGizmoOperation & RblxGizmo::BOUNDS) != 0;
    if (RblxUI::Checkbox("Translate", &translateActive))
        mCurrentGizmoOperation = (RblxGizmo::OPERATION)((int)mCurrentGizmoOperation ^ (int)RblxGizmo::TRANSLATE);
    RblxUI::SameLine();
    if (RblxUI::Checkbox("Rotate", &rotateActive))
        mCurrentGizmoOperation = (RblxGizmo::OPERATION)((int)mCurrentGizmoOperation ^ (int)RblxGizmo::ROTATE);
    RblxUI::SameLine();
    if (RblxUI::Checkbox("Scale", &scaleActive))
        mCurrentGizmoOperation = (RblxGizmo::OPERATION)((int)mCurrentGizmoOperation ^ (int)RblxGizmo::SCALE);
    RblxUI::SameLine();
    if (RblxUI::Checkbox("Bounds", &boundsActive))
        mCurrentGizmoOperation = (RblxGizmo::OPERATION)((int)mCurrentGizmoOperation ^ (int)RblxGizmo::BOUNDS);
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    RblxGizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
    RblxUI::InputFloat3("Tr", matrixTranslation);
    RblxUI::InputFloat3("Rt", matrixRotation);
    RblxUI::InputFloat3("Sc", matrixScale);
    RblxGizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

    if (mCurrentGizmoOperation & (RblxGizmo::TRANSLATE | RblxGizmo::ROTATE | RblxGizmo::SCALE))
    {
        if (RblxUI::RadioButton("Local", mCurrentGizmoMode == RblxGizmo::LOCAL))
            mCurrentGizmoMode = RblxGizmo::LOCAL;
        RblxUI::SameLine();
        if (RblxUI::RadioButton("World", mCurrentGizmoMode == RblxGizmo::WORLD))
            mCurrentGizmoMode = RblxGizmo::WORLD;
    }

    if (RblxUI::IsKeyPressed(RblxUIKey_S))
        useSnap = !useSnap;
    RblxUI::Checkbox("Use Snap", &useSnap);
    RblxUI::SameLine();
    if (mCurrentGizmoOperation & RblxGizmo::TRANSLATE)
        RblxUI::InputFloat3("Snap", &snap[0]);
    if (mCurrentGizmoOperation & RblxGizmo::ROTATE)
        RblxUI::InputFloat("Angle Snap", &snap[0]);
    if (mCurrentGizmoOperation & RblxGizmo::SCALE)
        RblxUI::InputFloat("Scale Snap", &snap[0]);
    if (mCurrentGizmoOperation & RblxGizmo::BOUNDS)
    {
        RblxUI::InputFloat3("Bounds Min", &bounds[0]);
        RblxUI::InputFloat3("Bounds Max", &bounds[3]);
        RblxUI::Checkbox("Snap Bounds", &boundSizingSnap);
        if (boundSizingSnap)
            RblxUI::InputFloat3("Bounds Snap", &boundsSnap[0]);
    }

    RblxUIIO& io = RblxUI::GetIO();
    float viewManipulateRight = io.DisplaySize.x;
    float viewManipulateTop = 0;
    static RblxUIWindowFlags gizmoWindowFlags = 0;
    RblxUI::SetNextWindowSize(RblxVec2(800, 400), RblxUICond_Appearing);
    RblxUI::SetNextWindowPos(RblxVec2(400, 20), RblxUICond_Appearing);
    RblxUI::PushStyleColor(RblxUICol_WindowBg, (RblxVec4)RblxColor(0.35f, 0.3f, 0.3f));
    if (useWindow)
    {
       RblxUI::Begin("Gizmo", 0, gizmoWindowFlags);
       RblxGizmo::SetDrawlist();
    }
    float windowWidth = (float)RblxUI::GetWindowWidth();
    float windowHeight = (float)RblxUI::GetWindowHeight();

    if (!useWindow)
    {
       RblxGizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    }
    else
    {
       RblxGizmo::SetRect(RblxUI::GetWindowPos().x, RblxUI::GetWindowPos().y, windowWidth, windowHeight);
    }
    viewManipulateRight = RblxUI::GetWindowPos().x + windowWidth;
    viewManipulateTop = RblxUI::GetWindowPos().y;
    RblxUIWindow* window = RblxUI::GetCurrentWindow();
    gizmoWindowFlags = RblxUI::IsWindowHovered() && RblxUI::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? RblxUIWindowFlags_NoMove : 0;

    // Drag in empty viewport area to orbit the camera
    RblxUIIO& ioVP = RblxUI::GetIO();
    if (RblxUI::IsWindowHovered() && !RblxGizmo::IsOver() && !RblxGizmo::IsUsingViewManipulate() && ioVP.MouseDown[0])
    {
       const float handednessSign = rightHanded ? 1.f : -1.f;
       camYAngle += ioVP.MouseDelta.x * 0.01f * handednessSign;
       camXAngle += ioVP.MouseDelta.y * 0.01f;
       camXAngle = RblxClamp(camXAngle, -3.14159f * 0.49f, 3.14159f * 0.49f);
       float eye[] = { cosf(camYAngle) * cosf(camXAngle) * camDistance, sinf(camXAngle) * camDistance, sinf(camYAngle) * cosf(camXAngle) * camDistance };
       float at[] = { 0.f, 0.f, 0.f };
       float up[] = { 0.f, 1.f, 0.f };
       LookAt(eye, at, up, cameraView, rightHanded);
    }

    RblxGizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
    RblxGizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0], gizmoCount);

    RblxGizmo::ViewManipulate(cameraView, camDistance, RblxVec2(viewManipulateRight - 128, viewManipulateTop), RblxVec2(128, 128), 0x10101010);
}

void TransformEnd()
{
   if (useWindow)
   {
      RblxUI::End();
   }
   RblxUI::PopStyleColor(1);
}

void EditTransform(float* cameraView, float* cameraProjection, float* matrix)
{
    RblxUIIO& io = RblxUI::GetIO();
    float windowWidth = (float)RblxUI::GetWindowWidth();
    float windowHeight = (float)RblxUI::GetWindowHeight();
    if (!useWindow)
    {
       RblxGizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    }
    else
    {
       RblxGizmo::SetRect(RblxUI::GetWindowPos().x, RblxUI::GetWindowPos().y, windowWidth, windowHeight);
    }
    const bool hasBounds = (mCurrentGizmoOperation & RblxGizmo::BOUNDS) != 0;
    RblxGizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, hasBounds ? bounds : NULL, hasBounds && boundSizingSnap ? boundsSnap : NULL);
}

//
//
// RblxSequencer interface
//
//
static const char* SequencerItemTypeNames[] = { "Camera","Music", "ScreenEffect", "FadeIn", "Animation" };

struct RampEdit : public RblxCurveEdit::Delegate
{
   RampEdit()
   {
      mPts[0][0] = RblxVec2(-10.f, 0);
      mPts[0][1] = RblxVec2(20.f, 0.6f);
      mPts[0][2] = RblxVec2(25.f, 0.2f);
      mPts[0][3] = RblxVec2(70.f, 0.4f);
      mPts[0][4] = RblxVec2(120.f, 1.f);
      mPointCount[0] = 5;

      mPts[1][0] = RblxVec2(-50.f, 0.2f);
      mPts[1][1] = RblxVec2(33.f, 0.7f);
      mPts[1][2] = RblxVec2(80.f, 0.2f);
      mPts[1][3] = RblxVec2(82.f, 0.8f);
      mPointCount[1] = 4;


      mPts[2][0] = RblxVec2(40.f, 0);
      mPts[2][1] = RblxVec2(60.f, 0.1f);
      mPts[2][2] = RblxVec2(90.f, 0.82f);
      mPts[2][3] = RblxVec2(150.f, 0.24f);
      mPts[2][4] = RblxVec2(200.f, 0.34f);
      mPts[2][5] = RblxVec2(250.f, 0.12f);
      mPointCount[2] = 6;
      mbVisible[0] = mbVisible[1] = mbVisible[2] = true;
      mMax = RblxVec2(1.f, 1.f);
      mMin = RblxVec2(0.f, 0.f);
   }
   size_t GetCurveCount()
   {
      return 3;
   }

   bool IsVisible(size_t curveIndex)
   {
      return mbVisible[curveIndex];
   }
   size_t GetPointCount(size_t curveIndex)
   {
      return mPointCount[curveIndex];
   }

   uint32_t GetCurveColor(size_t curveIndex)
   {
      uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
      return cols[curveIndex];
   }
   RblxVec2* GetPoints(size_t curveIndex)
   {
      return mPts[curveIndex];
   }
   virtual RblxCurveEdit::CurveType GetCurveType(size_t curveIndex) const { return RblxCurveEdit::CurveSmooth; }
   virtual int EditPoint(size_t curveIndex, int pointIndex, RblxVec2 value)
   {
      mPts[curveIndex][pointIndex] = RblxVec2(value.x, value.y);
      SortValues(curveIndex);
      for (size_t i = 0; i < GetPointCount(curveIndex); i++)
      {
         if (mPts[curveIndex][i].x == value.x)
            return (int)i;
      }
      return pointIndex;
   }
   virtual void AddPoint(size_t curveIndex, RblxVec2 value)
   {
      if (mPointCount[curveIndex] >= 8)
         return;
      mPts[curveIndex][mPointCount[curveIndex]++] = value;
      SortValues(curveIndex);
   }
   virtual RblxVec2& GetMax() { return mMax; }
   virtual RblxVec2& GetMin() { return mMin; }
   virtual unsigned int GetBackgroundColor() { return 0; }
   RblxVec2 mPts[3][8];
   size_t mPointCount[3];
   bool mbVisible[3];
   RblxVec2 mMin;
   RblxVec2 mMax;
private:
   void SortValues(size_t curveIndex)
   {
      auto b = std::begin(mPts[curveIndex]);
      auto e = std::begin(mPts[curveIndex]) + GetPointCount(curveIndex);
      std::sort(b, e, [](RblxVec2 a, RblxVec2 b) { return a.x < b.x; });

   }
};

struct MySequence : public RblxSequencer::SequenceInterface
{
   // interface with sequencer

   virtual int GetFrameMin() const {
      return mFrameMin;
   }
   virtual int GetFrameMax() const {
      return mFrameMax;
   }
   virtual int GetItemCount() const { return (int)myItems.size(); }

   virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char*); }
   virtual const char* GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }
   virtual const char* GetItemLabel(int index) const
   {
      static char tmps[512];
      snprintf(tmps, 512, "[%02d] %s", index, SequencerItemTypeNames[myItems[index].mType]);
      return tmps;
   }

   virtual void Get(int index, int** start, int** end, int* type, unsigned int* color)
   {
      MySequenceItem& item = myItems[index];
      if (color)
         *color = 0xFFAA8080; // same color for everyone, return color based on type
      if (start)
         *start = &item.mFrameStart;
      if (end)
         *end = &item.mFrameEnd;
      if (type)
         *type = item.mType;
   }
   virtual void Add(int type) { myItems.push_back(MySequenceItem{ type, 0, 10, false }); };
   virtual void Del(int index) { myItems.erase(myItems.begin() + index); }
   virtual void Duplicate(int index) { myItems.push_back(myItems[index]); }

   virtual size_t GetCustomHeight(int index) { return myItems[index].mExpanded ? 300 : 0; }

   // my datas
   MySequence() : mFrameMin(0), mFrameMax(0) {}
   int mFrameMin, mFrameMax;
   struct MySequenceItem
   {
      int mType;
      int mFrameStart, mFrameEnd;
      bool mExpanded;
   };
   std::vector<MySequenceItem> myItems;
   RampEdit rampEdit;

   virtual void DoubleClick(int index) {
      if (myItems[index].mExpanded)
      {
         myItems[index].mExpanded = false;
         return;
      }
      for (auto& item : myItems)
         item.mExpanded = false;
      myItems[index].mExpanded = !myItems[index].mExpanded;
   }

   virtual void CustomDraw(int index, RblxDrawList* draw_list, const RblxRect& rc, const RblxRect& legendRect, const RblxRect& clippingRect, const RblxRect& legendClippingRect)
   {
      static const char* labels[] = { "Translation", "Rotation" , "Scale" };

      rampEdit.mMax = RblxVec2(float(mFrameMax), 1.f);
      rampEdit.mMin = RblxVec2(float(mFrameMin), 0.f);
      draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
      for (int i = 0; i < 3; i++)
      {
         RblxVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
         RblxVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
         draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
         if (RblxRect(pta, ptb).Contains(RblxUI::GetMousePos()) && RblxUI::IsMouseClicked(0))
            rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
      }
      draw_list->PopClipRect();

      RblxUI::SetCursorScreenPos(rc.Min);
      RblxCurveEdit::Edit(rampEdit, rc.Max - rc.Min, 137 + index, &clippingRect);
   }

   virtual void CustomDrawCompact(int index, RblxDrawList* draw_list, const RblxRect& rc, const RblxRect& clippingRect)
   {
      rampEdit.mMax = RblxVec2(float(mFrameMax), 1.f);
      rampEdit.mMin = RblxVec2(float(mFrameMin), 0.f);
      draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
      for (int i = 0; i < 3; i++)
      {
         for (unsigned int j = 0; j < rampEdit.mPointCount[i]; j++)
         {
            float p = rampEdit.mPts[i][j].x;
            if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd)
               continue;
            float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
            float x = RblxLerp(rc.Min.x, rc.Max.x, r);
            draw_list->AddLine(RblxVec2(x, rc.Min.y + 6), RblxVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
         }
      }
      draw_list->PopClipRect();
   }
};

//
//
// GraphEditor interface
//
//


template <typename T, std::size_t N>
struct Array
{
   T data[N];
   const size_t size() const { return N; }

   const T operator [] (size_t index) const { return data[index]; }
   operator T* () {
      T* p = new T[N];
      memcpy(p, data, sizeof(data));
      return p;
   }
};

template <typename T, typename ... U> Array(T, U...)->Array<T, 1 + sizeof...(U)>;

struct GraphEditorDelegate : public GraphEditor::Delegate
{
   bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override
   {
      return true;
   }

   void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override
   {
      mNodes[nodeIndex].mSelected = selected;
   }

   void MoveSelectedNodes(const RblxVec2 delta) override
   {
      for (auto& node : mNodes)
      {
         if (!node.mSelected)
         {
            continue;
         }
         node.x += delta.x;
         node.y += delta.y;
      }
   }

   virtual void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override
   {
   }

   void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override
   {
      mLinks.push_back({ inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex });
   }

   void DelLink(GraphEditor::LinkIndex linkIndex) override
   {
      mLinks.erase(mLinks.begin() + linkIndex);
   }

   void CustomDraw(RblxDrawList* drawList, RblxRect rectangle, GraphEditor::NodeIndex nodeIndex) override
   {
      drawList->AddLine(rectangle.Min, rectangle.Max, RBLXUI_COL32(0, 0, 0, 255));
      drawList->AddText(rectangle.Min, RBLXUI_COL32(255, 128, 64, 255), "Draw");
   }

   const size_t GetTemplateCount() override
   {
      return sizeof(mTemplates) / sizeof(GraphEditor::Template);
   }

   const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override
   {
      return mTemplates[index];
   }

   const size_t GetNodeCount() override
   {
      return mNodes.size();
   }

   const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override
   {
      const auto& myNode = mNodes[index];
      return GraphEditor::Node
      {
          myNode.name,
          myNode.templateIndex,
          RblxRect(RblxVec2(myNode.x, myNode.y), RblxVec2(myNode.x + 200, myNode.y + 200)),
          myNode.mSelected
      };
   }

   const size_t GetLinkCount() override
   {
      return mLinks.size();
   }

   const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override
   {
      return mLinks[index];
   }

   // Graph datas
   static const inline GraphEditor::Template mTemplates[] = {
       {
           RBLXUI_COL32(160, 160, 180, 255),
           RBLXUI_COL32(100, 100, 140, 255),
           RBLXUI_COL32(110, 110, 150, 255),
           1,
           Array{"MyInput"},
           nullptr,
           2,
           Array{"MyOutput0", "MyOuput1"},
           nullptr
       },

       {
           RBLXUI_COL32(180, 160, 160, 255),
           RBLXUI_COL32(140, 100, 100, 255),
           RBLXUI_COL32(150, 110, 110, 255),
           3,
           nullptr,
           Array{ RBLXUI_COL32(200,100,100,255), RBLXUI_COL32(100,200,100,255), RBLXUI_COL32(100,100,200,255) },
           1,
           Array{"MyOutput0"},
           Array{ RBLXUI_COL32(200,200,200,255)}
       }
   };

   struct Node
   {
      const char* name;
      GraphEditor::TemplateIndex templateIndex;
      float x, y;
      bool mSelected;
   };

   std::vector<Node> mNodes = {
       {
           "My Node 0",
           0,
           0, 0,
           false
       },

       {
           "My Node 1",
           0,
           400, 0,
           false
       },

       {
           "My Node 2",
           1,
           400, 400,
           false
       }
   };

   std::vector<GraphEditor::Link> mLinks = { {0, 0, 1, 0} };
};

//
//
// RblxVectorEditor demo
//
//

static void SeedVectorEditorPath(RblxVectorEditor::Path& path, RblxVectorEditor::Editor& editor)
{
   path.clear();

   RblxVectorEditor::Anchor a;
   a.position = RblxVec2(0.0f, 0.0f);
   a.handleOut = RblxVec2(70.0f, -90.0f);
   a.hasHandleOut = true;

   RblxVectorEditor::Anchor b;
   b.position = RblxVec2(180.0f, 0.0f);
   b.handleIn = RblxVec2(-70.0f, -90.0f);
   b.handleOut = RblxVec2(60.0f, 90.0f);
   b.hasHandleIn = true;
   b.hasHandleOut = true;
   b.handleMode = RblxVectorEditor::HandleMode::Free;

   RblxVectorEditor::Anchor c;
   c.position = RblxVec2(320.0f, 120.0f);
   c.handleIn = RblxVec2(-60.0f, 90.0f);
   c.hasHandleIn = true;

   path.anchors = { a, b, c };
   path.closed = false;
   editor.ClearSelection();
}

static void ControlPointShapeCombo(const char* label, RblxVectorEditor::ControlPointShape& shape)
{
   const char* shapes[] = { "Circle", "Square", "Diamond" };
   int shapeIndex = static_cast<int>(shape);
   if (RblxUI::Combo(label, &shapeIndex, shapes, RBLXUI_ARRAYSIZE(shapes)))
      shape = static_cast<RblxVectorEditor::ControlPointShape>(shapeIndex);
}

static const char* VectorEditorEditKindName(RblxVectorEditor::EditKind kind)
{
   switch (kind)
   {
   case RblxVectorEditor::EditKind::MoveAnchor: return "Move Anchor";
   case RblxVectorEditor::EditKind::MoveHandle: return "Move Handle";
   case RblxVectorEditor::EditKind::AddAnchor: return "Add Anchor";
   case RblxVectorEditor::EditKind::DeleteAnchor: return "Delete Anchor";
   case RblxVectorEditor::EditKind::AddHandle: return "Add Handles";
   case RblxVectorEditor::EditKind::DeleteHandle: return "Delete Handles";
   case RblxVectorEditor::EditKind::ChangePointMode: return "Change Point Mode";
   case RblxVectorEditor::EditKind::PathOperation: return "Path Operation";
   }
   return "Edit";
}

struct VectorEditorUndoDebug : RblxVectorEditor::Delegate
{
   int editCount = 0;
   int activeAnchor = -1;
   const char* activeEdit = "None";
   const char* lastEdit = "None";

   void BeginEdit(RblxVectorEditor::EditKind kind, int anchorIndex) override
   {
      activeAnchor = anchorIndex;
      activeEdit = VectorEditorEditKindName(kind);
   }

   void EndEdit() override
   {
      ++editCount;
      lastEdit = activeEdit;
      activeEdit = "None";
      activeAnchor = -1;
   }
};

static RblxVec2 VectorEditorPathCenter(const RblxVectorEditor::Path& path)
{
   if (path.anchors.empty())
      return RblxVec2(0.0f, 0.0f);

   RblxVec2 minPos = path.anchors.front().position;
   RblxVec2 maxPos = path.anchors.front().position;
   for (const RblxVectorEditor::Anchor& anchor : path.anchors)
   {
      minPos.x = std::min(minPos.x, anchor.position.x);
      minPos.y = std::min(minPos.y, anchor.position.y);
      maxPos.x = std::max(maxPos.x, anchor.position.x);
      maxPos.y = std::max(maxPos.y, anchor.position.y);
   }
   return RblxVec2((minPos.x + maxPos.x) * 0.5f, (minPos.y + maxPos.y) * 0.5f);
}

static void ApplyVectorEditorViewResult(const RblxVectorEditor::Result& result,
   RblxVectorEditor::Config& config)
{
   config.transform.pan.x += result.viewPanDelta.x;
   config.transform.pan.y += result.viewPanDelta.y;

   if (result.viewZoomFactor != 1.0f)
   {
      const float oldZoom = config.transform.zoom;
      const float newZoom = std::max(0.25f, std::min(oldZoom * result.viewZoomFactor, 4.0f));
      const float appliedFactor = newZoom / oldZoom;
      const RblxVec2 center = result.viewZoomCenterCanvas;
      config.transform.pan = RblxVec2(
         center.x - (center.x - config.transform.pan.x) * appliedFactor,
         center.y - (center.y - config.transform.pan.y) * appliedFactor);
      config.transform.zoom = newZoom;
   }
}

static void ShowVectorEditorDemo()
{
   static RblxVectorEditor::Editor editor;
   static RblxVectorEditor::Path path;
   static RblxVectorEditor::Config config;
   static RblxVectorEditor::Tool tool = RblxVectorEditor::Tool::Select;
   static VectorEditorUndoDebug undoDebug;
   static bool initialized = false;

   config.delegate = &undoDebug;

   if (!initialized)
   {
      initialized = true;
      config.transform.pan = RblxVec2(80.0f, 90.0f);
      config.canvasSize = RblxVec2(0.0f, 320.0f);
      SeedVectorEditorPath(path, editor);
   }

   if (RblxUI::RadioButton("Select##VectorEditor", tool == RblxVectorEditor::Tool::Select))
      tool = RblxVectorEditor::Tool::Select;
   RblxUI::SameLine();
   if (RblxUI::RadioButton("Pen##VectorEditor", tool == RblxVectorEditor::Tool::Pen))
      tool = RblxVectorEditor::Tool::Pen;
   RblxUI::SameLine();
   if (RblxUI::Button("Clear##VectorEditor"))
   {
      undoDebug.BeginEdit(RblxVectorEditor::EditKind::PathOperation, -1);
      path.clear();
      editor.ClearSelection();
      tool = RblxVectorEditor::Tool::Select;
      undoDebug.EndEdit();
   }
   RblxUI::SameLine();
   if (RblxUI::Button("Seed##VectorEditor"))
   {
      SeedVectorEditorPath(path, editor);
      tool = RblxVectorEditor::Tool::Select;
   }

   const int selectedAnchor = editor.GetSelectedAnchor();
   const bool hasSelection = selectedAnchor >= 0 && selectedAnchor < static_cast<int>(path.anchors.size());
   const RblxVectorEditor::HandleMode selectedMode = hasSelection
      ? path.anchors[selectedAnchor].handleMode
      : RblxVectorEditor::HandleMode::Corner;
   if (RblxUI::RadioButton("Corner##VectorEditor", selectedMode == RblxVectorEditor::HandleMode::Corner) && hasSelection)
   {
      undoDebug.BeginEdit(path.anchors[selectedAnchor].hasHandleIn || path.anchors[selectedAnchor].hasHandleOut
         ? RblxVectorEditor::EditKind::DeleteHandle
         : RblxVectorEditor::EditKind::ChangePointMode, selectedAnchor);
      RblxVectorEditor::MakeCorner(path.anchors[selectedAnchor]);
      undoDebug.EndEdit();
   }
   RblxUI::SameLine();
   if (RblxUI::RadioButton("Aligned##VectorEditor", selectedMode == RblxVectorEditor::HandleMode::Aligned) && hasSelection)
   {
      undoDebug.BeginEdit(!path.anchors[selectedAnchor].hasHandleIn && !path.anchors[selectedAnchor].hasHandleOut
         ? RblxVectorEditor::EditKind::AddHandle
         : RblxVectorEditor::EditKind::ChangePointMode, selectedAnchor);
      RblxVectorEditor::MakeAligned(path.anchors[selectedAnchor]);
      undoDebug.EndEdit();
   }
   RblxUI::SameLine();
   if (RblxUI::RadioButton("Mirrored##VectorEditor", selectedMode == RblxVectorEditor::HandleMode::Mirrored) && hasSelection)
   {
      undoDebug.BeginEdit(!path.anchors[selectedAnchor].hasHandleIn && !path.anchors[selectedAnchor].hasHandleOut
         ? RblxVectorEditor::EditKind::AddHandle
         : RblxVectorEditor::EditKind::ChangePointMode, selectedAnchor);
      RblxVectorEditor::MakeMirrored(path.anchors[selectedAnchor]);
      undoDebug.EndEdit();
   }
   RblxUI::SameLine();
   if (RblxUI::Button("Add Handles##VectorEditor") && hasSelection)
   {
      undoDebug.BeginEdit(RblxVectorEditor::EditKind::AddHandle, selectedAnchor);
      RblxVectorEditor::AddHandles(path.anchors[selectedAnchor]);
      undoDebug.EndEdit();
   }
   RblxUI::SameLine();
   if (RblxUI::Button("Delete Handles##VectorEditor") && hasSelection)
   {
      undoDebug.BeginEdit(RblxVectorEditor::EditKind::DeleteHandle, selectedAnchor);
      RblxVectorEditor::DeleteHandles(path.anchors[selectedAnchor]);
      undoDebug.EndEdit();
   }

   if (RblxUI::TreeNodeEx("Path##VectorEditor"))
   {
      if (RblxUI::Button(path.closed ? "Open Path##VectorEditor" : "Close Path##VectorEditor"))
      {
         undoDebug.BeginEdit(RblxVectorEditor::EditKind::PathOperation, -1);
         path.closed = !path.closed;
         undoDebug.EndEdit();
      }
      RblxUI::SameLine();
      if (RblxUI::Button("Reverse Path##VectorEditor"))
      {
         undoDebug.BeginEdit(RblxVectorEditor::EditKind::PathOperation, -1);
         RblxVectorEditor::ReversePath(path);
         undoDebug.EndEdit();
      }
      RblxUI::Text("Selected anchor: %d (%d selected)",
         selectedAnchor, editor.GetSelectedAnchorCount());
      RblxUI::TreePop();
   }

   if (editor.GetSelectedAnchorCount() == static_cast<int>(path.anchors.size()) && !path.anchors.empty())
      config.transform.objectPivot = VectorEditorPathCenter(path);
   else if (hasSelection)
      config.transform.objectPivot = path.anchors[selectedAnchor].position;
   else
      config.transform.objectPivot = VectorEditorPathCenter(path);

   if (RblxUI::TreeNodeEx("View##VectorEditor"))
   {
      RblxUI::DragFloat2("Pan##VectorEditor", &config.transform.pan.x, 1.0f);
      RblxUI::SliderFloat("Zoom##VectorEditor", &config.transform.zoom, 0.25f, 4.0f, "%.2f");
      RblxUI::SliderAngle("Rotation##VectorEditor", &config.transform.objectRotationRadians, -180.0f, 180.0f);
      RblxUI::Text("Rotation pivot: %.1f, %.1f",
         config.transform.objectPivot.x, config.transform.objectPivot.y);
      RblxUI::TreePop();
   }

   if (RblxUI::TreeNodeEx("Style##VectorEditor"))
   {
      RblxUI::Checkbox("Show Grid##VectorEditor", &config.showGrid);
      RblxUI::SliderFloat("Grid Step##VectorEditor", &config.style.gridStep, 8.0f, 96.0f, "%.1f");
      RblxUI::SliderFloat("Path Thickness##VectorEditor", &config.style.pathThickness, 1.0f, 8.0f, "%.1f");
      RblxUI::SliderFloat("Handle Line Thickness##VectorEditor", &config.style.handleLineThickness, 0.5f, 4.0f, "%.1f");
      RblxUI::SliderFloat("Anchor Size##VectorEditor", &config.style.anchorRadius, 2.0f, 12.0f, "%.1f");
      RblxUI::SliderFloat("Handle Size##VectorEditor", &config.style.handleRadius, 2.0f, 10.0f, "%.1f");
      ControlPointShapeCombo("Anchor Shape##VectorEditor", config.style.anchorShape);
      ControlPointShapeCombo("Handle Shape##VectorEditor", config.style.handleShape);
      RblxUI::SliderFloat("Hit Radius##VectorEditor", &config.style.hitRadius, 4.0f, 20.0f, "%.1f");
      RblxUI::TreePop();
   }

   config.tool = tool;
   config.canvasSize.x = RblxUI::GetContentRegionAvail().x;
   const RblxVectorEditor::Result result = editor.Draw("##RblxVectorEditor", path, config);
   ApplyVectorEditorViewResult(result, config);

   RblxUI::Text("changed=%s committed=%s capture mouse=%s keyboard=%s",
      result.changed ? "true" : "false",
      result.committed ? "true" : "false",
      editor.WantsMouseCapture() ? "true" : "false",
      editor.WantsKeyboardCapture() ? "true" : "false");
   RblxUI::Text("undo steps=%d last=%s active=%s anchor=%d",
      undoDebug.editCount, undoDebug.lastEdit, undoDebug.activeEdit, undoDebug.activeAnchor);
   RblxUI::TextWrapped("Select: drag anchors/handles, Shift-click or box-select anchors, Delete removes selection. Mouse wheel zooms, middle mouse pans. Pen: click anchors, click-drag handles, hold Shift while dragging to snap handles to 45 degrees, click first anchor to close.");
}


int main(int, char**)
{
   RblxApp::RblxApp imApp;

   RblxApp::Config config;
   config.mWidth = 1280;
   config.mHeight = 720;
   //config.mFullscreen = true;
   imApp.Init(config);

   int lastUsing = 0;

   float cameraView[16] =
   { 1.f, 0.f, 0.f, 0.f,
     0.f, 1.f, 0.f, 0.f,
     0.f, 0.f, 1.f, 0.f,
     0.f, 0.f, 0.f, 1.f };

   float cameraProjection[16];

   // build a procedural texture. Copy/pasted and adapted from https://rosettacode.org/wiki/Plasma_effect#Graphics_version
   unsigned int procTexture;
   glGenTextures(1, &procTexture);
   glBindTexture(GL_TEXTURE_2D, procTexture);
   uint32_t* tempBitmap = new uint32_t[256 * 256];
   int index = 0;
   for (int y = 0; y < 256; y++)
   {
      for (int x = 0; x < 256; x++)
      {
         float dx = x + .5f;
         float dy = y + .5f;
         float dv = sinf(x * 0.02f) + sinf(0.03f * (x + y)) + sinf(sqrtf(0.4f * (dx * dx + dy * dy) + 1.f));

         tempBitmap[index] = 0xFF000000 +
         (int(255 * fabsf(sinf(dv * 3.141592f))) << 16) +
         (int(255 * fabsf(sinf(dv * 3.141592f + 2 * 3.141592f / 3))) << 8) + 
         (int(255 * fabs(sin(dv * 3.141592f + 4.f * 3.141592f / 3.f))));

         index++;
      }
   }
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempBitmap);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   delete [] tempBitmap;

   // sequence with default values
   MySequence mySequence;
   mySequence.mFrameMin = -100;
   mySequence.mFrameMax = 1000;
   mySequence.myItems.push_back(MySequence::MySequenceItem{ 0, 10, 30, false });
   mySequence.myItems.push_back(MySequence::MySequenceItem{ 1, 20, 30, true });
   mySequence.myItems.push_back(MySequence::MySequenceItem{ 3, 12, 60, false });
   mySequence.myItems.push_back(MySequence::MySequenceItem{ 2, 61, 90, false });
   mySequence.myItems.push_back(MySequence::MySequenceItem{ 4, 90, 99, false });

   // Camera projection
   bool isPerspective = true;
   float fov = 27.f;
   float viewWidth = 10.f; // for orthographic
   bool infiniteFarPlane = false;
   int handedness = 0; // 0 = right-handed, 1 = left-handed

   bool firstFrame = true;

   // Main loop
   while (!imApp.Done())
   {
      imApp.NewFrame();

      RblxUIIO& io = RblxUI::GetIO();
      bool rightHanded = (handedness == 0);
      if (isPerspective)
      {
         Perspective(fov, io.DisplaySize.x / io.DisplaySize.y, 0.1f, 100.f, cameraProjection, rightHanded, infiniteFarPlane);
      }
      else
      {
         float viewHeight = viewWidth * io.DisplaySize.y / io.DisplaySize.x;
         float zn = rightHanded ? 1000.f : -1000.f;
         float zf = rightHanded ? -1000.f : 1000.f;
         OrthoGraphic(-viewWidth, viewWidth, -viewHeight, viewHeight, zn, zf, cameraProjection);
      }
      RblxGizmo::SetOrthographic(!isPerspective);
      RblxGizmo::BeginFrame();

      RblxUI::SetNextWindowPos(RblxVec2(1024, 100), RblxUICond_Appearing);
      RblxUI::SetNextWindowSize(RblxVec2(256, 256), RblxUICond_Appearing);

      // create a window and insert the inspector
      RblxUI::SetNextWindowPos(RblxVec2(10, 10), RblxUICond_Appearing);
      RblxUI::SetNextWindowSize(RblxVec2(320, 500), RblxUICond_Appearing);
      RblxUI::Begin("Editor");
      if (RblxUI::RadioButton("Full view", !useWindow)) useWindow = false;
      RblxUI::SameLine();
      if (RblxUI::RadioButton("Window", useWindow)) useWindow = true;

      RblxUI::Text("Camera");
      bool viewDirty = false;
      if (RblxUI::RadioButton("Perspective", isPerspective)) isPerspective = true;
      RblxUI::SameLine();
      if (RblxUI::RadioButton("Orthographic", !isPerspective)) isPerspective = false;
      if (isPerspective)
      {
         RblxUI::SliderFloat("Fov", &fov, 20.f, 110.f);
      }
      else
      {
         RblxUI::SliderFloat("Ortho width", &viewWidth, 1, 20);
      }
      viewDirty |= RblxUI::SliderFloat("Distance", &camDistance, 1.f, 10.f);
      if (RblxUI::IsItemHovered() && io.MouseWheel != 0.f)
      {
         camDistance = RblxClamp(camDistance - io.MouseWheel * 0.5f, 1.f, 10.f);
         viewDirty = true;
      }
      viewDirty |= RblxUI::Combo("Handedness", &handedness, "Right-handed\0Left-handed\0");
      // Recompute rightHanded immediately after combo so the LookAt below uses the new value
      rightHanded = (handedness == 0);
      if (isPerspective)
      {
         viewDirty |= RblxUI::Checkbox("Infinite far plane", &infiniteFarPlane);
      }
      RblxUI::SliderInt("Gizmo count", &gizmoCount, 1, 4);

      if (viewDirty || firstFrame)
      {
         float eye[] = { cosf(camYAngle) * cosf(camXAngle) * camDistance, sinf(camXAngle) * camDistance, sinf(camYAngle) * cosf(camXAngle) * camDistance };
         float at[] = { 0.f, 0.f, 0.f };
         float up[] = { 0.f, 1.f, 0.f };
         LookAt(eye, at, up, cameraView, rightHanded);
         firstFrame = false;
      }
      // Also refresh next frame so projection catches up when handedness changed
      static int prevHandedness = handedness;
      if (prevHandedness != handedness) { firstFrame = true; }
      prevHandedness = handedness;

      RblxUI::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
      if (RblxGizmo::IsUsing())
      {
         RblxUI::Text("Using gizmo");
      }
      else
      {
         RblxUI::Text(RblxGizmo::IsOver()?"Over gizmo":"");
         RblxUI::SameLine();
         RblxUI::Text(RblxGizmo::IsOver(RblxGizmo::TRANSLATE) ? "Over translate gizmo" : "");
         RblxUI::SameLine();
         RblxUI::Text(RblxGizmo::IsOver(RblxGizmo::ROTATE) ? "Over rotate gizmo" : "");
         RblxUI::SameLine();
         RblxUI::Text(RblxGizmo::IsOver(RblxGizmo::SCALE) ? "Over scale gizmo" : "");
      }
      RblxUI::Separator();
      
      TransformStart(cameraView, cameraProjection, objectMatrix[lastUsing], rightHanded);
      for (int matId = 0; matId < gizmoCount; matId++)
      {
          RblxGizmo::PushID(matId);
      
          EditTransform(cameraView, cameraProjection, objectMatrix[matId]);
          if (RblxGizmo::IsUsing())
          {
              lastUsing = matId;
          }
          RblxGizmo::PopID();
      }
      TransformEnd();

      RblxUI::End();

      RblxUI::SetNextWindowPos(RblxVec2(10, 500), RblxUICond_Appearing);

      RblxUI::SetNextWindowSize(RblxVec2(940, 480), RblxUICond_Appearing);
      RblxUI::Begin("Other controls");
      if (RblxUI::CollapsingHeader("Zoom Slider"))
      {
         static float uMin = 0.4f, uMax = 0.6f;
         static float vMin = 0.4f, vMax = 0.6f;
         RblxUI::Image((RblxTextureID)(uint64_t)procTexture, RblxVec2(900,300), RblxVec2(uMin, vMin), RblxVec2(uMax, vMax));
         {
            RblxUI::SameLine();
            RblxUI::PushID(18);
            RblxZoomSlider::RblxZoomSlider(0.f, 1.f, vMin, vMax, 0.01f, RblxZoomSlider::RblxUIZoomSliderFlags_Vertical);
            RblxUI::PopID();
         }
      
         {
            RblxUI::PushID(19);
            RblxZoomSlider::RblxZoomSlider(0.f, 1.f, uMin, uMax);
            RblxUI::PopID();
         }
      }
      if (RblxUI::CollapsingHeader("Sequencer"))
      {
         // let's create the sequencer
         static int selectedEntry = -1;
         static int firstFrame = 0;
         static bool expanded = true;
         static int currentFrame = 100;

         RblxUI::PushItemWidth(130);
         RblxUI::InputInt("Frame Min", &mySequence.mFrameMin);
         RblxUI::SameLine();
         RblxUI::InputInt("Frame ", &currentFrame);
         RblxUI::SameLine();
         RblxUI::InputInt("Frame Max", &mySequence.mFrameMax);
         RblxUI::PopItemWidth();
         Sequencer(&mySequence, &currentFrame, &expanded, &selectedEntry, &firstFrame, RblxSequencer::SEQUENCER_EDIT_STARTEND | RblxSequencer::SEQUENCER_ADD | RblxSequencer::SEQUENCER_DEL | RblxSequencer::SEQUENCER_COPYPASTE | RblxSequencer::SEQUENCER_CHANGE_FRAME);
         // add a UI to edit that particular item
         if (selectedEntry != -1)
         {
           const MySequence::MySequenceItem &item = mySequence.myItems[selectedEntry];
           RblxUI::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
           // switch (type) ....
         }
      }
      if (RblxUI::CollapsingHeader("Vector Editor"))
      {
         ShowVectorEditorDemo();
      }

      // Graph Editor
      static GraphEditor::Options options;
      static GraphEditorDelegate delegate;
      static GraphEditor::ViewState viewState;
      static GraphEditor::FitOnScreen fit = GraphEditor::Fit_None;
      static bool showGraphEditor = true;

      if (RblxUI::CollapsingHeader("Graph Editor"))
      {
         RblxUI::Checkbox("Show GraphEditor", &showGraphEditor);
         GraphEditor::EditOptions(options);
      }

      // Light Rig
      if (RblxUI::CollapsingHeader("Light Rig"))
      {
         static RblxLightRig::Light lights[128] = {
            { 1.f, 0.3f, 0.1f,  1.f,  0.5f, 0.5f},
            { 0.3f, 1.f, 0.1f,  1.f,  -0.5f, 0.5f},
            { 0.1f, 0.3f, 1.f,  1.f,  -0.5f, -0.5f},
         };
         static int lightCount = 3;
         static int selectedLight = -1;
         selectedLight = RblxLightRig::Edit(lights, lightCount, selectedLight, RblxVec2(200,200));
         if (selectedLight >= 0 && selectedLight < lightCount)
         {
            auto& light = lights[selectedLight];
            RblxUI::ColorEdit3("RGB", &light.r);
            RblxUI::SliderFloat("Intensity", &light.intensity, 0.f, 10.f);
         }
      }

      RblxUI::End();

      if (showGraphEditor)
      {
         RblxUI::Begin("Graph Editor", NULL, 0);
         if (RblxUI::Button("Fit all nodes"))
         {
            fit = GraphEditor::Fit_AllNodes;
         }
         RblxUI::SameLine();
         if (RblxUI::Button("Fit selected nodes"))
         {
            fit = GraphEditor::Fit_SelectedNodes;
         }
         GraphEditor::Show(delegate, options, viewState, true, &fit);

         RblxUI::End();
      }

      // render everything
      glClearColor(0.45f, 0.4f, 0.4f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT);
      imApp.EndFrame();
   }

   imApp.Finish();

   return 0;
}
