#ifndef ICONLIST_H_
#define ICONLIST_H_

#include "includes.h"

#include "NetConnect.h"
#include "locale/NetConnect.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_DockPrefs.h"
#include "protos.h"

#define MUIM_IconList_DeleteAllImages  (TAGBASE_NETCONNECTPREFS | 0x1030)

struct IconList_Data
{
   LONG dummy;
};

#endif
