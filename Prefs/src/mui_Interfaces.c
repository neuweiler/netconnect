/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Interfaces.h"
#include "mui_IfaceWindow.h"
#include "protos.h"

///
/// external variables
extern struct MUI_CustomClass  *CL_IfaceWindow;
extern Object *app;
extern struct Library *GenesisBase;

///

/// Interfaces_NewIface
ULONG Interfaces_NewIface(struct IClass *cl, Object *obj, struct MUIP_Interfaces_NewIface *msg)
{
   struct Interfaces_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;

   if(msg->copy)
      DoMethod(data->LI_Interfaces, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &iface);
   else
      iface = (APTR)-1;

   DoMethod(data->LI_Interfaces, MUIM_List_InsertSingle, iface, MUIV_List_Insert_Bottom);
   set(data->LI_Interfaces, MUIA_List_Active, MUIV_List_Active_Bottom);

   DoMethod(data->LI_Interfaces , MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &iface);
   if(iface)
      uniquify_iface_name(data->LI_Interfaces, iface);
   DoMethod(data->LI_Interfaces, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);

   DoMethod(obj, MUIM_Interfaces_Edit);

   return(NULL);
}

///
/// Interfaces_SetStates
ULONG Interfaces_SetStates(struct IClass *cl, Object *obj, Msg msg)
{
   struct Interfaces_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;

   DoMethod(data->LI_Interfaces , MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &iface);
   DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, !iface, data->BT_Copy,
      data->BT_Edit, data->BT_Delete, NULL);

   return(NULL);
}

///
/// Interfaces_Edit
ULONG Interfaces_Edit(struct IClass *cl, Object *obj, Msg msg)
{
   struct Interfaces_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;

   DoMethod(data->LI_Interfaces, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &iface);
   if(iface)
   {
      Object *window;

      set(app, MUIA_Application_Sleep, TRUE);
      if(window = NewObject(CL_IfaceWindow->mcc_Class, NULL, MUIA_Genesis_Originator, obj, TAG_DONE))
      {
         DoMethod(app, OM_ADDMEMBER, window);
         DoMethod(window, MUIM_IfaceWindow_Init, iface);
         set(window, MUIA_Window_Title, iface->if_name);
         set(window, MUIA_Window_Open, TRUE);
      }
   }
   else
      DisplayBeep(NULL);

   return(NULL);
}

///
/// Interfaces_EditFinish
ULONG Interfaces_EditFinish(struct IClass *cl, Object *obj, struct MUIP_Interfaces_EditFinish *msg)
{
   struct Interfaces_Data *data = INST_DATA(cl, obj);

   set(msg->win, MUIA_Window_Open, FALSE);
   if(msg->ok)
      DoMethod(msg->win, MUIM_IfaceWindow_CopyData);
   DoMethod(app, OM_REMMEMBER, msg->win);
   MUI_DisposeObject(msg->win);
   DoMethod(data->LI_Interfaces, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
   set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///

/// IfaceList_ConstructFunc
SAVEDS ASM struct Interface *IfaceList_ConstructFunc(register __a2 APTR pool, register __a1 struct Interface *src)
{
   struct Interface *new;
   struct ScriptLine *script_line;
   struct ServerEntry *server;

   if(new = (struct Interface *)AllocVec(sizeof(struct Interface), MEMF_ANY | MEMF_CLEAR))
   {
      if(src && (src != (APTR)-1))
      {
         memcpy(new, src, sizeof(struct Interface));
         if(src->if_sana2configtext)
         {
            new->if_sana2configtext = NULL;
            ReallocCopy((STRPTR *)&new->if_sana2configtext, src->if_sana2configtext);
         }
         if(src->if_configparams)
         {
            new->if_configparams = NULL;
            ReallocCopy((STRPTR *)&new->if_configparams, src->if_configparams);
         }
         NewList((struct List *)&new->if_events);
         if(src->if_events.mlh_TailPred != (struct MinNode *)&src->if_events)
         {
            script_line = (struct ScriptLine *)src->if_events.mlh_Head;
            while(script_line->sl_node.mln_Succ)
            {
               add_script_line(&new->if_events, script_line->sl_command, script_line->sl_contents, script_line->sl_userdata);
               script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
            }
         }
         NewList((struct List *)&new->if_loginscript);
         if(src->if_loginscript.mlh_TailPred != (struct MinNode *)&src->if_loginscript)
         {
            script_line = (struct ScriptLine *)src->if_loginscript.mlh_Head;
            while(script_line->sl_node.mln_Succ)
            {
               add_script_line(&new->if_loginscript, script_line->sl_command, script_line->sl_contents, script_line->sl_userdata);
               script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
            }
         }

         NewList((struct List *)&new->if_nameservers);
         if(src->if_nameservers.mlh_TailPred != (struct MinNode *)&src->if_nameservers)
         {
            server = (struct ServerEntry *)src->if_nameservers.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               add_server(&new->if_nameservers, server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }
         NewList((struct List *)&new->if_domainnames);
         if(src->if_domainnames.mlh_TailPred != (struct MinNode *)&src->if_domainnames)
         {
            server = (struct ServerEntry *)src->if_domainnames.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               add_server(&new->if_domainnames, server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }
      }
      else
      {
         strcpy(new->if_name, "ppp");
         strcpy(new->if_sana2device, "DEVS:Networks/");
         strcpy(new->if_sana2config, "ENV:Sana2/");
         new->if_MTU = 1500;
         new->if_flags = IFL_PPP | IFL_UseNameServer;
         NewList((struct List *)&new->if_events);
         NewList((struct List *)&new->if_loginscript);
         NewList((struct List *)&new->if_nameservers);
         NewList((struct List *)&new->if_domainnames);
      }

      if(new->if_userdata = AllocVec(sizeof(struct PrefsPPPIface), MEMF_ANY | MEMF_CLEAR))
      {
         struct PrefsPPPIface *ppp_if = (struct PrefsPPPIface *)new->if_userdata;

         if(src && (src != (APTR)-1) && src->if_userdata)
            memcpy(new->if_userdata, src->if_userdata, sizeof(struct PrefsPPPIface));
         else
         {
            ppp_if->ppp_carrierdetect = TRUE;
         }
      }
   }
   return(new);
}
struct Hook IfaceList_ConstructHook= { { 0,0 }, (VOID *)IfaceList_ConstructFunc , NULL, NULL };

///
/// IfaceList_DestructFunc
SAVEDS ASM VOID IfaceList_DestructFunc(register __a2 APTR pool, register __a1 struct Interface *iface)
{
   if(iface)
   {
      if(iface->if_sana2configtext)
         FreeVec(iface->if_sana2configtext);
      if(iface->if_configparams)
         FreeVec(iface->if_configparams);
      if(iface->if_userdata)
         FreeVec(iface->if_userdata);

      clear_list(&iface->if_events);
      clear_list(&iface->if_loginscript);
      clear_list(&iface->if_domainnames);
      clear_list(&iface->if_nameservers);

      FreeVec(iface);
   }
}
struct Hook IfaceList_DestructHook= { { 0,0 }, (VOID *)IfaceList_DestructFunc , NULL, NULL };

///
/// IfaceList_DisplayFunc
SAVEDS ASM LONG IfaceList_DisplayFunc(register __a2 char **array, register __a1 struct Interface *iface)
{
   if(iface)
   {
      static char buf[5];

      sprintf(buf, "%ld", iface->if_sana2unit);
      *array++ = iface->if_name;
      *array++ = iface->if_comment;
      *array++ = FilePart(iface->if_sana2device);
      *array++ = buf;
      *array   = (*iface->if_addr ? (STRPTR)iface->if_addr : GetStr(MSG_TX_Dynamic));
   }
   else
   {
      *array++ = GetStr(MSG_TX_Name);
      *array++ = GetStr(MSG_TX_Comment);
      *array++ = GetStr(MSG_TX_Device);
      *array++ = GetStr(MSG_TX_Unit);
      *array   = GetStr(MSG_TX_Address);
   }
   return(NULL);
}
struct Hook IfaceList_DisplayHook= { { 0,0 }, (VOID *)IfaceList_DisplayFunc , NULL, NULL };

///

/// Interfaces_New
ULONG Interfaces_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Interfaces_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      Child, VGroup,
         GroupSpacing(0),
         Child, tmp.LV_Interfaces = NListviewObject,
            MUIA_CycleChain      , 1,
            MUIA_NListview_NList , tmp.LI_Interfaces = NListObject,
               InputListFrame,
               MUIA_NList_DragType     , MUIV_NList_DragType_Default,
               MUIA_NList_DragSortable , TRUE,
               MUIA_NList_DoubleClick  , TRUE,
               MUIA_NList_ConstructHook, &IfaceList_ConstructHook,
               MUIA_NList_DisplayHook  , &IfaceList_DisplayHook,
               MUIA_NList_DestructHook , &IfaceList_DestructHook,
               MUIA_NList_Format       , "BAR,BAR,BAR,BAR,",
               MUIA_NList_Title        , TRUE,
            End,
         End,
         Child, HGroup,
            GroupSpacing(0),
            Child, tmp.BT_New    = MakeButton(MSG_BT_New),
            Child, tmp.BT_Copy   = MakeButton(MSG_BT_Copy),
            Child, tmp.BT_Delete = MakeButton(MSG_BT_Delete),
            Child, tmp.BT_Edit   = MakeButton(MSG_BT_Edit),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Interfaces_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->BT_Edit       , MUIA_Disabled, TRUE);
      set(data->BT_Delete     , MUIA_Disabled, TRUE);
      set(data->BT_Copy       , MUIA_Disabled, TRUE);

      set(data->LV_Interfaces , MUIA_ShortHelp, GetStr(MSG_Help_IfaceList));
      set(data->BT_New        , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_Copy       , MUIA_ShortHelp, GetStr(MSG_Help_Copy));
      set(data->BT_Delete     , MUIA_ShortHelp, GetStr(MSG_Help_Delete));
      set(data->BT_Edit       , MUIA_ShortHelp, GetStr(MSG_Help_Edit));

      DoMethod(data->BT_New         , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_Interfaces_NewIface, FALSE);
      DoMethod(data->BT_Copy        , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_Interfaces_NewIface, TRUE);
      DoMethod(data->BT_Edit        , MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_Interfaces_Edit);
      DoMethod(data->BT_Delete      , MUIM_Notify, MUIA_Pressed, FALSE, data->LI_Interfaces, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->LI_Interfaces  , MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_Interfaces_SetStates);
      DoMethod(data->LI_Interfaces  , MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 1, MUIM_Interfaces_Edit);
   }
   return((ULONG)obj);
}

///
/// Interfaces_Dispatcher
SAVEDS ASM ULONG Interfaces_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                      : return(Interfaces_New          (cl, obj, (APTR)msg));
      case MUIM_Interfaces_Edit        : return(Interfaces_Edit         (cl, obj, (APTR)msg));
      case MUIM_Interfaces_EditFinish  : return(Interfaces_EditFinish   (cl, obj, (APTR)msg));
      case MUIM_Interfaces_NewIface    : return(Interfaces_NewIface     (cl, obj, (APTR)msg));
      case MUIM_Interfaces_SetStates   : return(Interfaces_SetStates    (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

