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
/// NetInfo_Redraw
ULONG NetInfo_Redraw(struct IClass *cl, Object *obj, Msg msg)
{
   struct NetInfo_Data *data = INST_DATA(cl,obj);

   DoMethod(data->LI_Ifaces, MUIM_List_Redraw, MUIV_List_Redraw_All);
   return(NULL);
}

///

/// Ifaces_DisplayFunc
SAVEDS ASM LONG Iface_DisplayFunc(register __a2 char **array, register __a1 struct Interface *iface)
{
   if(iface)
   {
      *array++ = iface->if_name;
      *array++ = (iface->if_flags & IFL_IsOnline ? GetStr(MSG_TX_Online) : GetStr(MSG_TX_Offline));
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
      End))
   {
      struct NetInfo_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod,
         win, 3, MUIM_MainWindow_DisposeWindow, obj);
   }
   return((ULONG)obj);
}

///
/// NetInfo_Dispatcher
SAVEDS ASM ULONG NetInfo_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                : return(NetInfo_New    (cl,obj,(APTR)msg));
      case MUIM_NetInfo_Update   : return(NetInfo_Update (cl,obj,(APTR)msg));
      case MUIM_NetInfo_Redraw   : return(NetInfo_Redraw (cl,obj,(APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

