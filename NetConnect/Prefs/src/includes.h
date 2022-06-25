#ifndef INCLUDES_H_
#define INCLUDES_H_

#include <exec/memory.h>
#include <exec/types.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <proto/datatypes.h>
#include <proto/commodities.h>
#include <proto/wb.h>
#include <proto/icon.h>
#include <proto/diskfont.h>
#include <proto/graphics.h>

#include <libraries/iffparse.h>
#include <libraries/gadtools.h>
#include <libraries/mui.h>
#include <libraries/asl.h>
#include <libraries/diskfont.h>

#include <exec/memory.h>
#include <exec/execbase.h>

#include <dos/dostags.h>

#include <intuition/icclass.h>

#include <graphics/text.h>

#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>

#include <workbench/workbench.h>

#ifdef __SASC
#include <proto/muimaster.h>
#else
#include <clib/muimaster_protos.h>
#endif

#endif

