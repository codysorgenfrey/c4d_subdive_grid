#include "main.h"

Bool PluginStart()
{
  if (!RegisterSubdivideGrid())
    return false;
  
  return true;
}

void PluginEnd()
{
}

Bool PluginMessage(Int32 id, void* data)
{
  switch (id)
  {
    case C4DPL_INIT_SYS:
      // don't start plugin without resource
      if (!g_resource.Init())
        return false;
      
      return true;
      
    case C4DMSG_PRIORITY:
      break;
      
    case C4DPL_BUILDMENU:
      break;
      
    case C4DPL_COMMANDLINEARGS:
      break;
      
    case C4DPL_EDITIMAGE:
      break;
  }
  
  return false;
}

