#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "includes.h"

#include "NetConnect.h"
#include "locale/NetConnect.h"
#include "mui.h"
#include "mui_DockPrefs.h"
#include "mui_MenuPrefs.h"
#include "mui_IconList.h"
#include "protos.h"
#include "rev.h"
#include "images/logo.h"

#define MUIM_MainWindow_LoadPrefs      (TAGBASE_NETCONNECTPREFS | 0x1010)
#define MUIM_MainWindow_Finish         (TAGBASE_NETCONNECTPREFS | 0x1011)
#define MUIM_MainWindow_Test           (TAGBASE_NETCONNECTPREFS | 0x1012)
#define MUIM_MainWindow_NewDock        (TAGBASE_NETCONNECTPREFS | 0x1013)
#define MUIM_MainWindow_RemoveDock     (TAGBASE_NETCONNECTPREFS | 0x1014)
#define MUIM_MainWindow_About          (TAGBASE_NETCONNECTPREFS | 0x1015)
#define MUIM_MainWindow_About_Finish   (TAGBASE_NETCONNECTPREFS | 0x1016)
#define MUIM_MainWindow_DoubleStart    (TAGBASE_NETCONNECTPREFS | 0x1017)
#define MUIM_MainWindow_SetPage        (TAGBASE_NETCONNECTPREFS | 0x1018)
#define MUIM_MainWindow_InitGroups     (TAGBASE_NETCONNECTPREFS | 0x1019)
#define MUIM_MainWindow_Load           (TAGBASE_NETCONNECTPREFS | 0x101a)
#define MUIM_MainWindow_SaveAs         (TAGBASE_NETCONNECTPREFS | 0x101b)
#define MUIM_MainWindow_Reset          (TAGBASE_NETCONNECTPREFS | 0x101c)
#define MUIM_MainWindow_Help           (TAGBASE_NETCONNECTPREFS | 0x101d)

struct MUIP_MainWindow_Finish                { ULONG MethodID; LONG level; };
struct MUIP_MainWindow_About_Finish          { ULONG MethodID; Object *window; ULONG level; };
struct MUIP_MainWindow_LoadPrefs             { ULONG MethodID; STRPTR file; };

struct MainWindow_Data
{
   Object *MN_Strip;

   Object *GR_Pager;
   Object *LV_Pager;
   Object *LI_Pager;
   Object *BT_New;
   Object *BT_Remove;
   Object *GR_Active;

   Object *BT_Save;
   Object *BT_Use;
   Object *BT_Test;
   Object *BT_Cancel;

   Object *GR_Info;
   Object *GR_Menus;
   Object *GR_Dock;
};

#endif
