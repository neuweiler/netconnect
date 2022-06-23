/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Provider.h"
#include "mui_ProviderWindow.h"
#include "protos.h"

///
/// external variables
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_Databases;
extern struct MUI_CustomClass  *CL_ProviderWindow;
extern struct Hook objstrhook, des_hook;
extern Object *win, *app;

///

/// Provider_NewISP
ULONG Provider_NewISP(struct IClass *cl, Object *obj, Msg msg)
{
   struct Provider_Data *data = INST_DATA(cl, obj);

   DoMethod(data->LI_ISP, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
   set(data->LI_ISP, MUIA_List_Active, MUIV_List_Active_Bottom);
   DoMethod(obj, MUIM_Provider_EditISP);

   return(NULL);
}

///
/// Provider_SetStates
ULONG Provider_SetStates(struct IClass *cl, Object *obj, Msg msg)
{
   struct Provider_Data *data = INST_DATA(cl, obj);
   struct ISP *isp;

   DoMethod(data->LI_ISP , MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &isp);
   DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, !isp, data->BT_Edit, data->BT_Delete, NULL);

   return(NULL);
}

///
/// Provider_EditISP
ULONG Provider_EditISP(struct IClass *cl, Object *obj, Msg msg)
{
   struct Provider_Data *data = INST_DATA(cl, obj);
   struct ISP *isp;

   DoMethod(data->LI_ISP, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &isp);
   if(isp)
   {
      Object *window;

      set(app, MUIA_Application_Sleep, TRUE);
      if(window = NewObject(CL_ProviderWindow->mcc_Class, NULL, MUIA_Genesis_Originator, obj, TAG_DONE))
      {
         DoMethod(app, OM_ADDMEMBER, window);
         DoMethod(window, MUIM_ProviderWindow_Init, isp);
         set(window, MUIA_Window_Title, isp->isp_name);
         set(window, MUIA_Window_Open, TRUE);
      }
   }
   else
      DisplayBeep(NULL);

   return(NULL);
}

///
/// Provider_EditISPFinish
ULONG Provider_EditISPFinish(struct IClass *cl, Object *obj, struct MUIP_Provider_EditFinish *msg)
{
//   struct Provider_Data *data = INST_DATA(cl, obj);

   set(msg->win, MUIA_Window_Open, FALSE);
   if(msg->ok)
      DoMethod(msg->win, MUIM_ProviderWindow_CopyData);
   DoMethod(app, OM_REMMEMBER, msg->win);
   MUI_DisposeObject(msg->win);
   set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///

/// isp_consfunc
SAVEDS ASM APTR isp_consfunc(REG(a2) APTR pool, REG(a1) struct ISP *src)
{
   struct ISP *new;

   if(new = (struct ISP *)AllocVec(sizeof(struct ISP), MEMF_ANY | MEMF_CLEAR))
   {
      if(src && ((LONG)src != -1))
         memcpy(new, src, sizeof(struct ISP));
      if((LONG)src == -1)
      {
         strcpy(new->isp_name, GetStr(MSG_TX_NewISP));
      }
      NewList((struct List *)&new->isp_nameservers);
      NewList((struct List *)&new->isp_domainnames);
      NewList((struct List *)&new->isp_ifaces);
      NewList((struct List *)&new->isp_loginscript);
   }

   return(new);
}
static struct Hook isp_conshook = {{NULL, NULL}, (VOID *)isp_consfunc, NULL, NULL};

///
/// isp_desfunc
SAVEDS ASM VOID isp_desfunc(register __a2 APTR pool, register __a1 struct ISP *isp)
{
   if(isp)
   {
      clear_list(&isp->isp_nameservers);
      clear_list(&isp->isp_domainnames);
      clear_list(&isp->isp_loginscript);

      if(isp->isp_ifaces.mlh_TailPred != (struct MinNode *)&isp->isp_ifaces)
      {
         struct Interface *iface1, *iface2;

         iface1 = (struct Interface *)isp->isp_ifaces.mlh_Head;
         while(iface2 = (struct Interface *)iface1->if_node.mln_Succ)
         {
            if(iface1->if_sana2configtext)
               FreeVec(iface1->if_sana2configtext);
            if(iface1->if_configparams)
               FreeVec(iface1->if_configparams);
            if(iface1->if_userdata)
               FreeVec(iface1->if_userdata);

            clear_list(&iface1->if_events);

            Remove((struct Node *)iface1);
            FreeVec(iface1);
            iface1 = iface2;
         }
      }

      FreeVec(isp);
   }
}
static struct Hook isp_deshook = {{NULL, NULL}, (VOID *)isp_desfunc, NULL, NULL};

///
/// isp_dspfunc
SAVEDS ASM LONG isp_dspfunc(REG(a2) char **array, REG(a1) struct ISP *isp)
{
   if(isp)
   {
      *array++ = isp->isp_name;
      *array   = isp->isp_comment;
   }
   else
   {
      *array++ = GetStr(MSG_TX_ISP);
      *array   = GetStr(MSG_TX_Comment);
   }

   return(NULL);
}
static struct Hook isp_dsphook = {{NULL, NULL}, (VOID *)isp_dspfunc, NULL, NULL};

///

/// Provider_New
ULONG Provider_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Provider_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      Child, VGroup,
         GroupSpacing(0),
         Child, tmp.LV_ISP = NListviewObject,
            MUIA_CycleChain      , 1,
            MUIA_NListview_NList , tmp.LI_ISP = NListObject,
               InputListFrame,
               MUIA_NList_DragType     , MUIV_NList_DragType_Default,
               MUIA_NList_DragSortable , TRUE,
               MUIA_NList_DoubleClick  , TRUE,
               MUIA_NList_ConstructHook, &isp_conshook,
               MUIA_NList_DestructHook , &isp_deshook,
               MUIA_NList_DisplayHook  , &isp_dsphook,
               MUIA_NList_Format       , "BAR,",
               MUIA_NList_Title        , TRUE,
            End,
         End,
         Child, HGroup,
            GroupSpacing(0),
            Child, tmp.BT_New    = MakeButton(MSG_BT_New),
            Child, tmp.BT_Delete = MakeButton(MSG_BT_Delete),
            Child, tmp.BT_Edit   = MakeButton(MSG_BT_Edit),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Provider_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->BT_Edit       , MUIA_Disabled, TRUE);
      set(data->BT_Delete     , MUIA_Disabled, TRUE);

      set(data->LV_ISP     , MUIA_ShortHelp, GetStr(MSG_Help_ISPList));
      set(data->BT_New     , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_Delete  , MUIA_ShortHelp, GetStr(MSG_Help_Remove));
      set(data->BT_Edit    , MUIA_ShortHelp, GetStr(MSG_Help_Edit));

      DoMethod(data->BT_New    , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_Provider_NewISP);
      DoMethod(data->BT_Edit   , MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_Provider_EditISP);
      DoMethod(data->BT_Delete , MUIM_Notify, MUIA_Pressed, FALSE, data->LI_ISP, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->LI_ISP    , MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_Provider_SetStates);
      DoMethod(data->LI_ISP    , MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 1, MUIM_Provider_EditISP);
   }
   return((ULONG)obj);
}

///
/// Provider_Dispatcher
SAVEDS ASM ULONG Provider_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                      : return(Provider_New          (cl, obj, (APTR)msg));
      case MUIM_Provider_EditISP       : return(Provider_EditISP      (cl, obj, (APTR)msg));
      case MUIM_Provider_EditISPFinish : return(Provider_EditISPFinish(cl, obj, (APTR)msg));
      case MUIM_Provider_NewISP        : return(Provider_NewISP       (cl, obj, (APTR)msg));
      case MUIM_Provider_SetStates     : return(Provider_SetStates    (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

