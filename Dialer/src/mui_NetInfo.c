/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_NetInfo.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables
extern Object *app, *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct Config Config;
extern struct NewMenu NetInfoMenu[];

///

/// NetInfo_Update
ULONG NetInfo_Update(struct IClass *cl, Object *obj, Msg msg)
{
   struct NetInfo_Data *data = INST_DATA(cl,obj);
   struct Interface *iface;

   set(data->LI_Ifaces, MUIA_List_Quiet, TRUE);
   DoMethod(data->LI_Ifaces, MUIM_List_Clear);
   if(Config.cnf_ifaces.mlh_TailPred != (struct MinNode *)&Config.cnf_ifaces)
   {
      iface = (struct Interface *)Config.cnf_ifaces.mlh_Head;
      while(iface->if_node.mln_Succ)
      {
         DoMethod(data->LI_Ifaces, MUIM_List_InsertSingle, iface, MUIV_List_Insert_Bottom);
         iface = (struct Interface *)iface->if_node.mln_Succ;
      }
   }
   set(data->LI_Ifaces, MUIA_List_Quiet, FALSE);

   return(NULL);
}

///
/// NetInfo_SetStates
ULONG NetInfo_SetStates(struct IClass *cl, Object *obj, Msg msg)
{
   struct NetInfo_Data *data = INST_DATA(cl,obj);
   struct Interface *iface;

   DoMethod(data->LI_Ifaces, MUIM_List_Redraw, MUIV_List_Redraw_All);

   DoMethod(data->LI_Ifaces, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &iface);
   if(iface)
   {
      set(data->BT_Online, MUIA_Disabled, (iface->if_flags & IFL_IsOnline));
      set(data->BT_Offline, MUIA_Disabled, !(iface->if_flags & IFL_IsOnline));
   }
   else
   {
      set(data->BT_Online, MUIA_Disabled, TRUE);
      set(data->BT_Offline, MUIA_Disabled, TRUE);
   }

   return(NULL);
}

///
/// NetInfo_Close
ULONG NetInfo_Close(struct IClass *cl, Object *obj, Msg msg)
{
   if(xget(win, MUIA_Window_Open))
      DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);
   else
      DoMethod(app, MUIM_Application_PushMethod, win, 1, MUIM_MainWindow_Quit);

   return(NULL);
}

///
/// NetInfo_OnOffline
ULONG NetInfo_OnOffline(struct IClass *cl, Object *obj, struct MUIP_MainWindow_OnOffline *msg)
{
   struct NetInfo_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;

   DoMethod(data->LI_Ifaces, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &iface);
   if(iface)
   {
      switch(msg->online)
      {
         case 0:
            iface->if_flags |= IFL_PutOffline;
            break;
         case 1:
            iface->if_flags |= IFL_PutOnline;
            break;
         case 2:
            iface->if_flags |= ((iface->if_flags & IFL_IsOnline) ? IFL_PutOffline : IFL_PutOnline);
            break;
      }
      DoMethod(win, ((iface->if_flags & IFL_PutOffline) ? MUIM_MainWindow_PutOffline : MUIM_MainWindow_PutOnline), FALSE);
   }

   return(NULL);
}

///

/// Ifaces_DisplayFunc
SAVEDS ASM LONG Iface_DisplayFunc(register __a2 char **array, register __a1 struct Interface *iface)
{
   if(iface)
   {
      *array++ = iface->if_name;
      *array++ = (iface->if_flags & IFL_IsOnline ? GetStr(MSG_TX_Online) : (iface->if_flags & IFL_PutOnline ? GetStr(MSG_TX_Connecting) : GetStr(MSG_TX_Offline)));
      *array++ = iface->if_addr;
      *array++ = iface->if_dst;
      *array   = iface->if_gateway;
   }
   else
   {
      *array++ = GetStr(MSG_TX_Iface);
      *array++ = GetStr(MSG_TX_Status);
      *array++ = GetStr(MSG_TX_IPAddr);
      *array++ = GetStr(MSG_TX_Destination);
      *array   = GetStr(MSG_TX_Gateway);
   }
   return(NULL);
}
const struct Hook Iface_DisplayHook  = { { 0,0 }, (VOID *)Iface_DisplayFunc   , NULL, NULL };

///

/// NetInfo_New
ULONG NetInfo_New(struct IClass *cl, Object *obj, Msg msg)
{
   struct NetInfo_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title       , GetStr(MSG_TX_NetinfoWindowTitle),
      MUIA_Window_ID          , MAKE_ID('N','I','N','F'),
      MUIA_Window_Menustrip   , tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, NetInfoMenu, NULL),
      MUIA_Window_Height      , MUIV_Window_Height_MinMax(5),
      MUIA_Window_Width       , MUIV_Window_Width_MinMax(10),
      WindowContents, VGroup,
         Child, tmp.LV_Ifaces = NListviewObject,
            MUIA_CycleChain            , 1,
            MUIA_NListview_NList        , tmp.LI_Ifaces = NListObject,
               MUIA_Font                , MUIV_Font_Tiny,
               MUIA_Frame               , MUIV_Frame_InputList,
               MUIA_NList_DisplayHook   , &Iface_DisplayHook,
               MUIA_NList_Format        , "BAR,BAR,BAR,BAR,",
               MUIA_NList_Title         , TRUE,
               MUIA_NList_Input         , TRUE,
            End,
         End,
         Child, HGroup,
            Child, tmp.BT_Online = MakeButton(MSG_BT_Connect),
            Child, tmp.BT_Offline = MakeButton(MSG_BT_Disconnect),
         End,
      End))
   {
      struct NetInfo_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(data->BT_Online  , MUIA_Disabled, TRUE);
      set(data->BT_Offline , MUIA_Disabled, TRUE);

      DoMethod(obj, MUIM_Notify  , MUIA_Window_CloseRequest       , TRUE            , obj, 1, MUIM_NetInfo_Close);
      DoMethod(data->BT_Offline  , MUIM_Notify, MUIA_Pressed      , FALSE           , obj, 2, MUIM_NetInfo_OnOffline, 0);
      DoMethod(data->BT_Online   , MUIM_Notify, MUIA_Pressed      , FALSE           , obj, 2, MUIM_NetInfo_OnOffline, 1);
      DoMethod(data->LV_Ifaces   , MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 2, MUIM_NetInfo_OnOffline, 2);
      DoMethod(data->LI_Ifaces   , MUIM_Notify, MUIA_NList_Active , MUIV_EveryTime  , obj, 1, MUIM_NetInfo_SetStates);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, win, 1, MUIM_MainWindow_About);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_REPORT)   , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, win, 1, MUIM_MainWindow_GenesisReport);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MAINWINDOW), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_Open, TRUE);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT_MUI), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_AboutMUI, win);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, win, 1, MUIM_MainWindow_Quit);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_GENESIS)  , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, win, 1, MUIM_MainWindow_GenesisPrefs);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
   }
   return((ULONG)obj);
}

///
/// NetInfo_Dispatcher
SAVEDS ASM ULONG NetInfo_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                : return(NetInfo_New       (cl,obj,(APTR)msg));
      case MUIM_NetInfo_Update   : return(NetInfo_Update    (cl,obj,(APTR)msg));
      case MUIM_NetInfo_Close    : return(NetInfo_Close     (cl,obj,(APTR)msg));
      case MUIM_NetInfo_SetStates: return(NetInfo_SetStates (cl,obj,(APTR)msg));
      case MUIM_NetInfo_OnOffline: return(NetInfo_OnOffline (cl,obj,(APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

