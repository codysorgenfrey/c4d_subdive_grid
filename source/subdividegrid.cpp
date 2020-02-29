#include "subdividegrid.h"
#include "c4d_symbols.h"
#include "customgui_splinecontrol.h"

//
// MARK: Helper Functions
//

inline Float32 MapRange(Float32 value, Float32 min_input, Float32 max_input, Float32 min_output, Float32 max_output, SplineData *curve = NULL)
{
  Float32 inrange = max_input - min_input;
  if (CompareFloatTolerant(inrange, 0.0f))
    value = 0.0f; // Prevent DivByZero error
  else
    value = (value - min_input) / inrange; // Map input range to [0.0 ... 1.0]
  
  if (curve)
    value = curve->GetPoint(value).y; // Apply spline curve
  
  return  min_output + (max_output - min_output) * value; // Map to output range and return result
}

//
// MARK: Classes
//

Bool SubdivideGrid::GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags)
{
  // Before adding dynamic parameters, load the parameters from the description resource
  if (!description->LoadDescription(node->GetType()))
    return false;

  // Get description single ID
  const DescID *singleID = description->GetSingleDescID();

  // Add Complete control
  DescID completeID = DescLevel(ID_SG_COMPLETE, DTYPE_REAL, 0);
  if (!singleID || completeID.IsPartOf(*singleID, nullptr))
  {
    BaseContainer bc = GetCustomDataTypeDefault(DTYPE_REAL);
    bc.SetString(DESC_NAME, "Complete"_s);
    bc.SetString(DESC_SHORT_NAME, "Complete"_s);
    bc.SetInt32(DESC_UNIT, DESC_UNIT_PERCENT);
    bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_REALSLIDER);
    bc.SetFloat(DESC_DEFAULT, 1.0f);
    bc.SetFloat(DESC_MIN, 0.0f);
    bc.SetFloat(DESC_MAX, 1.0f);
    bc.SetFloat(DESC_MINSLIDER, 0.0f);
    bc.SetFloat(DESC_MAXSLIDER, 1.0f);
    bc.SetFloat(DESC_STEP, 0.01f);
    bc.SetBool(DESC_GUIOPEN, true);
    if (!description->SetParameter(completeID, bc, ID_TAGPROPERTIES))
      return false;
  }

  // Add spline group
  DescID splineGroupID = DescLevel(ID_SG_SPLINE_GROUP, DTYPE_GROUP, 0);
  if (!singleID || splineGroupID.IsPartOf(*singleID, nullptr))
  {
    BaseContainer bc = GetCustomDataTypeDefault(DTYPE_GROUP);
    bc.SetString(DESC_NAME, "Time Ramps"_s);
    bc.SetString(DESC_SHORT_NAME, "Time Ramps"_s);
    bc.SetBool(DESC_GUIOPEN, false);
    if (!description->SetParameter(splineGroupID, bc, DESCID_ROOT))
      return false;
  }
  
  // Add x ramp control
  DescID splineXID = DescLevel(ID_SG_SPLINE_X, CUSTOMDATATYPE_SPLINE, 0);
  if (!singleID || splineXID.IsPartOf(*singleID, nullptr))
  {
    BaseContainer bc = GetCustomDataTypeDefault(CUSTOMDATATYPE_SPLINE);
    bc.SetString(DESC_NAME, "X Ramp"_s);
    bc.SetString(DESC_SHORT_NAME, "X Ramp"_s);
    bc.SetBool(DESC_GUIOPEN, false);
    bc.SetFloat(SPLINECONTROL_X_MIN, 0.0f);
    bc.SetFloat(SPLINECONTROL_X_MAX, 1.0f);
    bc.SetFloat(SPLINECONTROL_Y_MIN, 0.0f);
    bc.SetFloat(SPLINECONTROL_Y_MAX, 1.0f);
    if (!description->SetParameter(splineXID, bc, splineGroupID))
      return false;
  }
  
  // Add y ramp control
  DescID splineYID = DescLevel(ID_SG_SPLINE_Y, CUSTOMDATATYPE_SPLINE, 0);
  if (!singleID || splineYID.IsPartOf(*singleID, nullptr))
  {
    BaseContainer bc = GetCustomDataTypeDefault(CUSTOMDATATYPE_SPLINE);
    bc.SetString(DESC_NAME, "Y Ramp"_s);
    bc.SetString(DESC_SHORT_NAME, "Y Ramp"_s);
    bc.SetBool(DESC_GUIOPEN, false);
    bc.SetFloat(SPLINECONTROL_X_MIN, 0.0f);
    bc.SetFloat(SPLINECONTROL_X_MAX, 1.0f);
    bc.SetFloat(SPLINECONTROL_Y_MIN, 0.0f);
    bc.SetFloat(SPLINECONTROL_Y_MAX, 1.0f);
    if (!description->SetParameter(splineYID, bc, splineGroupID))
      return false;
  }

  // Add x ramp control
  DescID splineZID = DescLevel(ID_SG_SPLINE_Z, CUSTOMDATATYPE_SPLINE, 0);
  if (!singleID || splineZID.IsPartOf(*singleID, nullptr))
  {
    BaseContainer bc = GetCustomDataTypeDefault(CUSTOMDATATYPE_SPLINE);
    bc.SetString(DESC_NAME, "Z Ramp"_s);
    bc.SetString(DESC_SHORT_NAME, "Z Ramp"_s);
    bc.SetBool(DESC_GUIOPEN, false);
    bc.SetFloat(SPLINECONTROL_X_MIN, 0.0f);
    bc.SetFloat(SPLINECONTROL_X_MAX, 1.0f);
    bc.SetFloat(SPLINECONTROL_Y_MIN, 0.0f);
    bc.SetFloat(SPLINECONTROL_Y_MAX, 1.0f);
    if (!description->SetParameter(splineXID, bc, splineGroupID))
      return false;
  }
  
  flags |= DESCFLAGS_DESC::LOADED;
  return true;
}

Bool SubdivideGrid::Init(GeListNode* node)
{
  return true;
}

EXECUTIONRESULT SubdivideGrid::Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags)
{
  iferr_scope_handler
  {
    // print error to console
    err.DiagOutput();
    // debug stop
    err.DbgStop();
    return EXECUTIONRESULT::OUTOFMEMORY;
  };
  
  // gather inputs
  BaseObject *parent = tag->GetObject();
  GeData data;
  tag->GetParameter(DescID(ID_SG_COMPLETE), data, DESCFLAGS_GET::NONE);
  Float complete = data.GetFloat();
  tag->GetParameter(DescID(ID_SG_SPLINE_X), data, DESCFLAGS_GET::NONE);
  SplineData *splineX = static_cast<SplineData *>(data.GetCustomDataType(CUSTOMDATATYPE_SPLINE));
  tag->GetParameter(DescID(ID_SG_SPLINE_Y), data, DESCFLAGS_GET::NONE);
  SplineData *splineY = static_cast<SplineData *>(data.GetCustomDataType(CUSTOMDATATYPE_SPLINE));
  tag->GetParameter(DescID(ID_SG_SPLINE_Z), data, DESCFLAGS_GET::NONE);
  SplineData *splineZ = static_cast<SplineData *>(data.GetCustomDataType(CUSTOMDATATYPE_SPLINE));
  
  // collect splines
  // if nothing in list, get children
  maxon::BaseArray<BaseObject *> splines;
  BaseObject *child = parent->GetDown();
  while (child)
  {
    splines.Append(child) iferr_return;
    child = child->GetNext();
  }
  
  if (splines.GetCount() == 0)
    return EXECUTIONRESULT::OK;
  
  //  gather parent info so not to recalculate later
  Vector parentRad = parent->GetRad();
  Matrix parentMg = parent->GetMg();
  Vector parentAnchor = parentMg.off;
  Vector parentObjSpaceAnchor = (-parent->GetMp()) + parentRad;

  //  parentCorners = self.GetCornersFromBBox(self.GetCollectiveBBox(splines))
  //  def DistFromAnchor(obj, anchor=parentAnchor):
  //      return (anchor - obj).GetLength()
  //  parentCorners.sort(key=DistFromAnchor)
  //  parentFarCorner = parentCorners[7]

  // calculate movements
  for (Int32 x = 0; x < splines.GetCount(); x++)
  {
    BaseObject *spline = splines[x];
    
    if (!spline->GetDeformMode()) // spline is disabled
      continue;

    Vector splineRad = spline->GetRad();
//      makesFarSides = self.MakesFarSides(spline, parentFarCorner)

      // scale
    Vector maxScaleOff(0.0001f); // cannot be 0 or else connect object freaks out
//      if makesFarSides['x']:
//          if splineRad.x != 0:
//              maxScaleOff.x = parentRad.x / splineRad.x
//      if makesFarSides['y']:
//          if splineRad.y != 0:
//              maxScaleOff.y = parentRad.y / splineRad.y
//      if makesFarSides['z']:
//          if splineRad.z != 0:
//              maxScaleOff.z = parentRad.z / splineRad.z
//
    Vector scaleOff(1.0f);
    scaleOff.x = MapRange(complete, 1.0f, 0.0f, 1.0f, maxScaleOff.x, splineX);
    scaleOff.y = MapRange(complete, 1.0f, 0.0f, 1.0f, maxScaleOff.y, splineY);
    scaleOff.z = MapRange(complete, 1.0f, 0.0f, 1.0f, maxScaleOff.z, splineZ);

      // position
    Vector splineRelPos = spline->GetRelPos();
    Vector maxPosOff = -splineRelPos;
    Vector splineObjSpaceAnchor = (-spline->GetMp()) + splineRad;
//      if makesFarSides['x']:
//          if splineRad.x != 0:
//              origPosX = splineRelPos.x
//              newSplineObjSpaceAnchorX = (splineObjSpaceAnchor.x / splineRad.x) * parentRad.x
//              newPosX = newSplineObjSpaceAnchorX - parentObjSpaceAnchor.x
//              maxPosOff.x = newPosX - origPosX
//      if makesFarSides['y']:
//          if splineRad.y != 0:
//              origPosY = splineRelPos.y
//              newSplineObjSpaceAnchorY = (splineObjSpaceAnchor.y / splineRad.y) * parentRad.y
//              newPosY = newSplineObjSpaceAnchorY - parentObjSpaceAnchor.y
//              maxPosOff.y = newPosY - origPosY
//      if makesFarSides['z']:
//          if splineRad.z != 0:
//              origPosZ = splineRelPos.z
//              newSplineObjSpaceAnchorZ = (splineObjSpaceAnchor.z / splineRad.z) * parentRad.z
//              newPosZ = newSplineObjSpaceAnchorZ - parentObjSpaceAnchor.z
//              maxPosOff.z = newPosZ - origPosZ
//
    Vector posOff(0);
    posOff.x = MapRange(complete, 1.0f, 0.0f, 0.0f, maxPosOff.x, splineX);
    posOff.y = MapRange(complete, 1.0f, 0.0f, 0.0f, maxPosOff.y, splineY);
    posOff.z = MapRange(complete, 1.0f, 0.0f, 0.0f, maxPosOff.z, splineZ);

    // apply
    spline->SetFrozenPos(posOff);
    spline->SetFrozenScale(scaleOff);
    spline->Message(MSG_UPDATE);
  }
  
  return EXECUTIONRESULT::OK;
}

//
// MARK: Register
//

Bool RegisterSubdivideGrid()
{
  return RegisterTagPlugin(
                           ID_SUBDIVIDEGRID,
                           "Subdivide Grid"_s,
                           TAG_EXPRESSION | TAG_VISIBLE,
                           SubdivideGrid::Alloc,
                           "Tsubdividegrid"_s,
                           AutoBitmap("subdivide_grid.tiff"_s),
                           0
                           );
}
