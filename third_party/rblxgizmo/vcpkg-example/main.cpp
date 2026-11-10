#define RBLXUI_DEFINE_MATH_OPERATORS
#include "rblxui.h"
#include "rblxui_internal.h"
#define IMAPP_IMPL
#include "RblxApp.h"

#include "RblxGizmo.h"
#include "RblxSequencer.h"
#include "RblxZoomSlider.h"
#include "RblxCurveEdit.h"
#include "GraphEditor.h"

#include <cmath>
#include <vector>
#include <algorithm>

bool useWindow = true;
int gizmoCount = 1;
float camDistance = 8.f;
static RblxGizmo::OPERATION mCurrentGizmoOperation(RblxGizmo::TRANSLATE);

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

void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float* m16)
{
   float temp, temp2, temp3, temp4;
   temp = 2.0f * znear;
   temp2 = right - left;
   temp3 = top - bottom;
   temp4 = zfar - znear;
   m16[0] = temp / temp2;
   m16[1] = 0.0;
   m16[2] = 0.0;
   m16[3] = 0.0;
   m16[4] = 0.0;
   m16[5] = temp / temp3;
   m16[6] = 0.0;
   m16[7] = 0.0;
   m16[8] = (right + left) / temp2;
   m16[9] = (top + bottom) / temp3;
   m16[10] = (-zfar - znear) / temp4;
   m16[11] = -1.0f;
   m16[12] = 0.0;
   m16[13] = 0.0;
   m16[14] = (-temp * zfar) / temp4;
   m16[15] = 0.0;
}

void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float* m16)
{
   float ymax, xmax;
   ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
   xmax = ymax * aspectRatio;
   Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
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

void LookAt(const float* eye, const float* at, const float* up, float* m16)
{
   float X[3], Y[3], Z[3], tmp[3];

   tmp[0] = eye[0] - at[0];
   tmp[1] = eye[1] - at[1];
   tmp[2] = eye[2] - at[2];
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

void EditTransform(float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition)
{
   static RblxGizmo::MODE mCurrentGizmoMode(RblxGizmo::LOCAL);
   static bool useSnap = false;
   static float snap[3] = { 1.f, 1.f, 1.f };
   static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
   static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
   static bool boundSizing = false;
   static bool boundSizingSnap = false;

   if (editTransformDecomposition)
   {
#if RBLXUI_VERSION_NUM >= 18700
      if (RblxUI::IsKeyPressed(RblxUIKey_Z))
#else
      if (RblxUI::IsKeyPressed('Z'))
#endif
         mCurrentGizmoOperation = RblxGizmo::TRANSLATE;
#if RBLXUI_VERSION_NUM >= 18700
      if (RblxUI::IsKeyPressed(RblxUIKey_E))
#else
      if (RblxUI::IsKeyPressed('E'))
#endif
         mCurrentGizmoOperation = RblxGizmo::ROTATE;
#if RBLXUI_VERSION_NUM >= 18700
      if (RblxUI::IsKeyPressed(RblxUIKey_R))
#else
      if (RblxUI::IsKeyPressed('R'))
#endif
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
#if RBLXUI_VERSION_NUM >= 18700
      if (RblxUI::IsKeyPressed(RblxUIKey_S))
#else
      if (RblxUI::IsKeyPressed('S'))
#endif
         useSnap = !useSnap;
      RblxUI::Checkbox("##useSnap", &useSnap);
      RblxUI::SameLine();

      switch (mCurrentGizmoOperation)
      {
      case RblxGizmo::TRANSLATE:
         RblxUI::InputFloat3("Snap", &snap[0]);
         break;
      case RblxGizmo::ROTATE:
         RblxUI::InputFloat("Angle Snap", &snap[0]);
         break;
      case RblxGizmo::SCALE:
         RblxUI::InputFloat("Scale Snap", &snap[0]);
         break;
      }
      RblxUI::Checkbox("Bound Sizing", &boundSizing);
      if (boundSizing)
      {
         RblxUI::PushID(3);
         RblxUI::Checkbox("##boundSizingSnap", &boundSizingSnap);
         RblxUI::SameLine();
         RblxUI::InputFloat3("Snap", boundsSnap);
         RblxUI::PopID();
      }
   }

   RblxUIIO& io = RblxUI::GetIO();
   float viewManipulateRight = io.DisplaySize.x;
   float viewManipulateTop = 0;
   if (useWindow)
   {
      RblxUI::SetNextWindowSize(RblxVec2(800, 400), RblxUICond_FirstUseEver);
      RblxUI::SetNextWindowPos(RblxVec2(400,20), RblxUICond_FirstUseEver);
      RblxUI::PushStyleColor(RblxUICol_WindowBg, (RblxVec4)RblxColor(0.35f, 0.3f, 0.3f));
      RblxUI::Begin("Gizmo", 0);
      RblxGizmo::SetDrawlist();
      float windowWidth = (float)RblxUI::GetWindowWidth();
      float windowHeight = (float)RblxUI::GetWindowHeight();
      RblxGizmo::SetRect(RblxUI::GetWindowPos().x, RblxUI::GetWindowPos().y, windowWidth, windowHeight);
      viewManipulateRight = RblxUI::GetWindowPos().x + windowWidth;
      viewManipulateTop = RblxUI::GetWindowPos().y;
   }
   else
   {
      RblxGizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
   }

   RblxGizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
   RblxGizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0], gizmoCount);
   RblxGizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);

   RblxGizmo::ViewManipulate(cameraView, camDistance, RblxVec2(viewManipulateRight - 128, viewManipulateTop), RblxVec2(128, 128), 0x10101010);

   if (useWindow)
   {
      RblxUI::End();
      RblxUI::PopStyleColor(1);
   }
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
         for (int j = 0; j < rampEdit.mPointCount[i]; j++)
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

template <typename T, typename ... U>
Array(T, U...) -> Array<T, 1 + sizeof...(U)>;

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
   float camYAngle = 165.f / 180.f * 3.14159f;
   float camXAngle = 32.f / 180.f * 3.14159f;

   bool firstFrame = true;

   // Main loop
   while (!imApp.Done())
   {
      imApp.NewFrame();

      RblxUIIO& io = RblxUI::GetIO();
      if (isPerspective)
      {
         Perspective(fov, io.DisplaySize.x / io.DisplaySize.y, 0.1f, 100.f, cameraProjection);
      }
      else
      {
         float viewHeight = viewWidth * io.DisplaySize.y / io.DisplaySize.x;
         OrthoGraphic(-viewWidth, viewWidth, -viewHeight, viewHeight, 1000.f, -1000.f, cameraProjection);
      }
      RblxGizmo::SetOrthographic(!isPerspective);
      RblxGizmo::BeginFrame();

      RblxUI::SetNextWindowPos(RblxVec2(1024, 100), RblxUICond_FirstUseEver);
      RblxUI::SetNextWindowSize(RblxVec2(256, 256), RblxUICond_FirstUseEver);

      // create a window and insert the inspector
      RblxUI::SetNextWindowPos(RblxVec2(10, 10), RblxUICond_FirstUseEver);
      RblxUI::SetNextWindowSize(RblxVec2(320, 340), RblxUICond_FirstUseEver);
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
      RblxUI::SliderInt("Gizmo count", &gizmoCount, 1, 4);

      if (viewDirty || firstFrame)
      {
         float eye[] = { cosf(camYAngle) * cosf(camXAngle) * camDistance, sinf(camXAngle) * camDistance, sinf(camYAngle) * cosf(camXAngle) * camDistance };
         float at[] = { 0.f, 0.f, 0.f };
         float up[] = { 0.f, 1.f, 0.f };
         LookAt(eye, at, up, cameraView);
         firstFrame = false;
      }

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
      for (int matId = 0; matId < gizmoCount; matId++)
      {
         RblxGizmo::PushID(matId);

         EditTransform(cameraView, cameraProjection, objectMatrix[matId], lastUsing == matId);
         if (RblxGizmo::IsUsing())
         {
            lastUsing = matId;
         }
         RblxGizmo::PopID();
      }

      RblxUI::End();

      RblxUI::SetNextWindowPos(RblxVec2(10, 350), RblxUICond_FirstUseEver);

      RblxUI::SetNextWindowSize(RblxVec2(940, 480), RblxUICond_FirstUseEver);
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
