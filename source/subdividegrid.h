#ifndef SUBDIVIDEGRID_H__
#define SUBDIVIDEGRID_H__

#include "main.h"

#define ID_SUBDIVIDEGRID 1054125

enum {
  ID_SG_COMPLETE = 1000,
  ID_SG_SPLINE_GROUP = 1001,
  ID_SG_SPLINE_X = 1002,
  ID_SG_SPLINE_Y = 1003,
  ID_SG_SPLINE_Z = 1004,
};

class SubdivideGrid : public TagData {
public:
  virtual Bool Init(GeListNode* node);
  virtual EXECUTIONRESULT Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags);
  virtual Bool GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags);
  static NodeData* Alloc() { return NewObjClear(SubdivideGrid); }
};

#endif
