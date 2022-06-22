/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_IfaceReq.h"
#include "protos.h"

///
/// external variables
extern Object *app, *win, *status_win;

///

/// IfaceReq_BuildList
ULONG IfaceReq_BuildList(struct IClass *cl, Object *obj, struct MUIP_IfaceReq_BuildList *msg)
{
   struct IfaceReq_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;

   data->list = msg->list;
   data->online = msg->online;

   set(app, MUIA_Application_Sleep, TRUE);

   if(msg->list->mlh_TailPred != (struct MinNode *)msg->list)
   {
      iface = (struct Interface *)msg->list->mlh_Head;
      while(iface->if_node.mln_Succ)
      {
         if((data->online && !(iface->if_flags & IFL_IsOnline)) || (!data->online && (iface->if_flags & IFL_IsOnline) && !(iface->if_flags & IFL_AlwaysOnline)))
            DoMethod(data->LI_Interfaces, MUIM_List_InsertSingle, iface, MUIV_List_Insert_Bottom);
         iface = (struct Interface *)iface->if_node.mln_Succ;
      }
   }

   return((ULONG)xget(data->LI_Interfaces, MUIA_NList_Entries));
}

///
/// IfaceReq_Finished
ULONG IfaceReq_Finished(struct IClass *cl, Object *obj, struct MUIP_IfaceReq_Finished *msg)
{
   struct IfaceReq_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;
   LONG pos, selected = NULL;

   if(msg->okay)
   {
      pos = 0;
      FOREVER
      {
         DoMethod(data->LI_Interfaces, MUIM_NList_GetEntry, pos, &iface);
         if(!iface)
            break;

         DoMethod(data->LI_Interfaces, MUIM_NList_Select, pos, MUIV_NList_Select_Ask, &selected);
         if(selected)
            iface->if_flags |= (data->online ? IFL_PutOnline : IFL_PutOffline);
         else
         {
            iface->if_flags &= ~IFL_PutOnline;
            iface->if_flags &= ~IFL_PutOffline;
         }
         pos++;
      }
      set(obj, MUIA_Window_Open, FALSE);
      DoMethod(win, (data->online ? MUIM_MainWindow_PutOnline : MUIM_MainWindow_PutOffline));
   }

   set(app, MUIA_Application_Sleep, FALSE);
   DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);

   return(NULL);
}

///

/// IfaceReq_DisplayFunc
SAVEDS ASM LONG IfaceReq_DisplayFunc(register __a2 char **array, register __a1 struct Interface *iface)
{
   if(iface)
      *array   = iface->if_name;

   return(NULL);
}

const struct Hook IfaceReq_DisplayHook   = { { 0,0 }, (VOID *)IfaceReq_DisplayFunc , NULL, NULL };

///
/// IfaceReq_New
ULONG IfaceReq_New(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceReq_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title       , GetStr(MSG_TX_InterfaceRequester),
      MUIA_Window_ID          , MAKE_ID('I','F','A','C'),
      MUIA_Window_RefWindow   , win,
      MUIA_Window_LeftEdge    , MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge     , MUIV_Window_TopEdge_Centered,
      MUIA_Window_Height      , MUIV_Window_Height_MinMax(0),
      MUIA_Window_Width       , MUIV_Window_Width_MinMax(0),
      WindowContents, VGroup,
         MUIA_Background, MUII_GroupBack,
         MUIA_FrameTitle, GetStr(MSG_TX_ChooseInterface),
         Child, tmp.LV_Interfaces = NListviewObject,
            MUIA_CycleChain            , 1,
            MUIA_NListview_NList       , tmp.LI_Interfaces = NListObject,
               MUIA_Frame              , MUIV_Frame_InputList,
               MUIA_NList_DisplayHook  , &IfaceReq_DisplayHook,
               MUIA_NList_MultiSelect  , MUIV_NList_MultiSelect_Always,
            End,
         End,
         Child, HGroup,
            Child, tmp.BT_Okay = MakeButton(MSG_BT_Okay),
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End))
   {
      struct IfaceReq_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      DoMethod(data->BT_Okay  , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_IfaceReq_Finished, TRUE);
      DoMethod(data->BT_Cancel, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_IfaceReq_Finished, FALSE);
      DoMethod(obj            , MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2, MUIM_IfaceReq_Finished, FALSE);
   }
   return((ULONG)obj);
}

///
/// IfaceReq_Dispatcher
SAVEDS ASM ULONG IfaceReq_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch(msg->MethodID)
   {
      case OM_NEW                  :  return(IfaceReq_New      (cl, obj, (APTR)msg));
      case MUIM_IfaceReq_BuildList :  return(IfaceReq_BuildList(cl, obj, (APTR)msg));
      case MUIM_IfaceReq_Finished  :  return(IfaceReq_Finished (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

