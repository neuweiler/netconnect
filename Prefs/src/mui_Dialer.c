/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Dialer.h"
#include "protos.h"

///
/// external variables
extern struct Hook des_hook;
extern struct Hook strobjhook;
extern struct Hook objstrhook;
extern Object *win;
extern Object *app;

///

/// ScriptList_ConstructFunc
struct ScriptLine * SAVEDS ScriptList_ConstructFunc(register __a2 APTR pool, register __a1 struct ScriptLine *src)
{
   struct ScriptLine *new;

   if((new = (struct ScriptLine *)AllocVec(sizeof(struct ScriptLine), MEMF_ANY | MEMF_CLEAR)) && src && src != (APTR)-1)
      memcpy(new, src, sizeof(struct ScriptLine));
   else if(new)
      new->sl_command = SL_Send;

   return(new);
}

///
/// ScriptList_DisplayFunc
SAVEDS LONG ScriptList_DisplayFunc(register __a2 char **array, register __a1 struct ScriptLine *script_line)
{
   if(script_line)
   {
      *array++ = script_commands[script_line->sl_command];
      *array = script_line->sl_contents;
   }
   else
   {
      *array++ = GetStr("  \033bCommand");
      *array   = GetStr("  \033bString");
   }
   return(NULL);
}

///

/// Dialer_PopString_AddPhone
ULONG Dialer_PopString_AddPhone(struct IClass *cl, Object *obj, struct MUIP_Dialer_PopString_AddPhone *msg)
{
   struct Dialer_Data *data = INST_DATA(cl, obj);

   if(msg->doit)
   {
      STRPTR ptr;

      if(ptr = (STRPTR)xget(data->STR_AddPhone, MUIA_String_Contents))
      {
         if(*ptr)
         {
            if(strlen((STRPTR)xget(data->STR_Phone, MUIA_String_Contents)))
               DoMethod(data->STR_Phone, MUIM_Textinput_AppendText, " | ", 3);
            DoMethod(data->STR_Phone, MUIM_Textinput_AppendText, ptr, strlen(ptr));
         }
      }
   }
   DoMethod(data->PO_Phone, MUIM_Popstring_Close, TRUE);
   set(data->STR_AddPhone, MUIA_String_Contents, "");

   return(NULL);
}


///
/// Dialer_ScriptActive
ULONG Dialer_ScriptActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct Dialer_Data *data = INST_DATA(cl, obj);
   struct ScriptLine *script_line;

   DoMethod(data->LI_Script, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &script_line);
   if(script_line)
   {
      nnset(data->CY_Command, MUIA_Cycle_Active, script_line->sl_command);
      if(script_line->sl_command > SL_WaitFor && script_line->sl_command < SL_Exec)
      {
         nnset(data->STR_String, MUIA_String_Contents, "");
         set(data->STR_String, MUIA_Disabled, TRUE);
      }
      else
      {
         nnset(data->STR_String, MUIA_String_Contents, script_line->sl_contents);
         set(data->STR_String, MUIA_Disabled, FALSE);
         set(_win(obj), MUIA_Window_ActiveObject, data->STR_String);
      }
      set(data->CY_Command, MUIA_Disabled, FALSE);
      set(data->BT_Remove, MUIA_Disabled, FALSE);
   }
   else
   {
      nnset(data->STR_String, MUIA_String_Contents, "");
      set(data->STR_String, MUIA_Disabled, TRUE);
      set(data->CY_Command, MUIA_Disabled, TRUE);
      set(data->BT_Remove, MUIA_Disabled, TRUE);
   }
   return(NULL);
}

///
/// Dialer_LineModified
ULONG Dialer_LineModified(struct IClass *cl, Object *obj, Msg msg)
{
   struct Dialer_Data *data = INST_DATA(cl, obj);
   struct ScriptLine *script_line;

   DoMethod(data->LI_Script, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &script_line);
   if(script_line)
   {
      script_line->sl_command = xget(data->CY_Command, MUIA_Cycle_Active);
      strncpy(script_line->sl_contents, (STRPTR)xget(data->STR_String, MUIA_String_Contents), MAXPATHLEN);
      DoMethod(data->LI_Script, MUIM_List_Redraw, MUIV_List_Redraw_Active);
      if(script_line->sl_command > SL_WaitFor && script_line->sl_command < SL_Exec)
         set(data->STR_String, MUIA_Disabled, TRUE);
      else
         set(data->STR_String, MUIA_Disabled, FALSE);
   }

   return(NULL);
}

///

/// Dialer_New
ULONG Dialer_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Dialer_Data tmp;
   static const struct Hook ScriptList_ConstructHook= { { 0,0 }, (VOID *)ScriptList_ConstructFunc , NULL, NULL };
   static const struct Hook ScriptList_DisplayHook= { { 0,0 }, (VOID *)ScriptList_DisplayFunc , NULL, NULL };
   static STRPTR ARR_CY_Window[] = { "-", "open", "close", NULL };
   static STRPTR ARR_CY_Login[] = { "Login automatically", "Login manually", NULL };

   if(obj = tmp.GR_Script = (Object *)DoSuperNew(cl, obj,
      MUIA_InnerLeft, 0,
      MUIA_InnerRight, 0,
      MUIA_InnerBottom, 0,
      MUIA_InnerTop, 0,
      Child, VGroup,
         MUIA_Group_Spacing, 0,
         Child, tmp.LV_Script = NListviewObject,
            MUIA_CycleChain            , 1,
            MUIA_NListview_NList       , tmp.LI_Script = NListObject,
               MUIA_Frame              , MUIV_Frame_InputList,
               MUIA_NList_DragType     , MUIV_NList_DragType_Default,
               MUIA_NList_ConstructHook, &ScriptList_ConstructHook,
               MUIA_NList_DisplayHook  , &ScriptList_DisplayHook,
               MUIA_NList_DestructHook , &des_hook,
               MUIA_NList_DragSortable , TRUE,
               MUIA_NList_Format       , "BAR,",
               MUIA_NList_Title        , TRUE,
            End,
         End,
         Child, HGroup,
            MUIA_Group_Spacing, 0,
            Child, tmp.BT_Add = MakeButton("  _Add"),
            Child, tmp.BT_Remove = MakeButton("  _Remove"),
         End,
      End,
      Child, HGroup,
         Child, tmp.CY_Command = Cycle(script_commands),
         Child, tmp.STR_String = MakeKeyString(NULL, MAXPATHLEN, "  l"),
      End,
      Child, HGroup,
         Child, MakeKeyLabel2("  Phone numbers:", "  p"),
         Child, tmp.PO_Phone = PopobjectObject,
            MUIA_Popstring_String      , tmp.STR_Phone = MakeKeyString(NULL, MAXPATHLEN, "  p"),
            MUIA_Popstring_Button      , PopButton(MUII_PopUp),
            MUIA_Popobject_Object      , VGroup,
               MUIA_Frame, MUIV_Frame_Group,
               Child, KeyLLabel("Add number...", 'n'),
               Child, tmp.STR_AddPhone = MakeKeyString(NULL, 30, "  n"),
               Child, HGroup,
                  Child, tmp.BT_AddPhone = MakeButton("  _Add"),
                  Child, tmp.BT_CancelPhone = MakeButton("  _Cancel"),
               End,
            End,
         End,
      End,
   TAG_MORE, msg->ops_AttrList))
   {
      struct Dialer_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      data->GR_Events = VGroup,
         MUIA_InnerLeft, 0,
         MUIA_InnerRight, 0,
         MUIA_InnerBottom, 0,
         MUIA_InnerTop, 0,
         Child, HVSpace,
         Child, ColGroup(3),
            Child, MUI_MakeObject(MUIO_BarTitle, "Event"),
            Child, MUI_MakeObject(MUIO_BarTitle, "Execute"),
            Child, MUI_MakeObject(MUIO_BarTitle, "Window"),
            Child, MakeKeyLabel2("  Online:", "  o"),
            Child, data->PA_Online = MakePopAsl(data->STR_Online = MakeKeyString("", MAXPATHLEN, "  o"), "  Choose file", FALSE),
            Child, data->CY_WinOnline = Cycle(ARR_CY_Window),
            Child, MakeKeyLabel2("  Online failed:", "  f"),
            Child, data->PA_OnlineFailed = MakePopAsl(data->STR_OnlineFailed = MakeKeyString("", MAXPATHLEN, "  f"), "  Choose file", FALSE),
            Child, data->CY_WinOnlineFailed = Cycle(ARR_CY_Window),
            Child, VVSpace,
            Child, HVSpace,
            Child, VVSpace,
            Child, MakeKeyLabel2("  Offline active:", "  a"),
            Child, data->PA_OfflineActive = MakePopAsl(data->STR_OfflineActive = MakeKeyString("", MAXPATHLEN, "  a"), "  Choose file", FALSE),
            Child, data->CY_WinOfflineActive = Cycle(ARR_CY_Window),
            Child, MakeKeyLabel2("  Offline passive:", "  p"),
            Child, data->PA_OfflinePassive = MakePopAsl(data->STR_OfflinePassive = MakeKeyString("", MAXPATHLEN, "  p"), "  Choose file", FALSE),
            Child, data->CY_WinOfflinePassive = Cycle(ARR_CY_Window),
            Child, VVSpace,
            Child, HVSpace,
            Child, VVSpace,
            Child, MakeKeyLabel2("  Startup:", "  t"),
            Child, data->PA_Startup = MakePopAsl(data->STR_Startup = MakeKeyString("", MAXPATHLEN, "  t"), "  Choose file", FALSE),
            Child, data->CY_WinStartup = Cycle(ARR_CY_Window),
            Child, MakeKeyLabel2("  Shutdown:", "  h"),
            Child, data->PA_Shutdown = MakePopAsl(data->STR_Shutdown = MakeKeyString("", MAXPATHLEN, "  t"), "  Choose file", FALSE),
            Child, VVSpace,
         End,
         Child, HVSpace,
      End;

      data->GR_Misc = ColGroup(3),
         MUIA_InnerLeft, 0,
         MUIA_InnerRight, 0,
         MUIA_InnerBottom, 0,
         MUIA_InnerTop, 0,
         Child, HVSpace,
         Child, VGroup,
            Child, HVSpace,
            Child, MUI_MakeObject(MUIO_BarTitle, "Connection"),
            Child, ColGroup(3),
               Child, VGroup,
                  Child, data->CY_AutoLogin = Cycle(ARR_CY_Login),
                  Child, ColGroup(2),
                     Child, data->CH_GoOnline = MakeKeyCheckMark(FALSE, "  i"),
                     Child, KeyLLabel1(GetStr("  Go online on startup"), *GetStr("  i")),
                  End,
               End,
               Child, HVSpace,
               Child, ColGroup(2),
                  Child, data->CH_QuickReconnect = MakeKeyCheckMark(FALSE, "  q"),
                  Child, KeyLLabel1(GetStr("  Quick reconnect"), *GetStr("  q")),
                  Child, data->CH_SynClock = MakeKeyCheckMark(FALSE, "  y"),
                  Child, KeyLLabel1(GetStr("  Sync system clock"), *GetStr("  y")),
               End,
            End,
            Child, HVSpace,
            Child, MUI_MakeObject(MUIO_BarTitle, "Display options"),
            Child, ColGroup(5),
               Child, data->CH_ShowStatus = MakeKeyCheckMark(TRUE, "  t"),
               Child, KeyLLabel1(GetStr("  Show status"), *GetStr("  t")),
               Child, HVSpace,
               Child, data->CH_ShowSpeed = MakeKeyCheckMark(TRUE, "  p"),
               Child, KeyLLabel1(GetStr("  Show speed"), *GetStr("  p")),
               Child, data->CH_ShowOnline = MakeKeyCheckMark(TRUE, "  o"),
               Child, KeyLLabel1(GetStr("  Show online time"), *GetStr("  o")),
               Child, HVSpace,
               Child, data->CH_ShowButtons = MakeKeyCheckMark(TRUE, "  b"),
               Child, KeyLLabel1(GetStr("  Show buttons"), *GetStr("  b")),
            End,
            Child, HVSpace,
         End,
         Child, HVSpace,
      End;

      if(data->GR_Script)
      {
         set(data->CY_Command, MUIA_Disabled, TRUE);
         set(data->STR_String, MUIA_Disabled, TRUE);
         set(data->BT_Remove, MUIA_Disabled, TRUE);
         set(data->STR_String, MUIA_String_AttachedList, data->LV_Script);
         set(data->CY_WinStartup, MUIA_Cycle_Active, 1);
         set(data->CY_Command, MUIA_Weight, 0);

         DoMethod(data->CY_Command, MUIM_Notify, MUIA_Cycle_Active         , MUIV_EveryTime , obj, 1, MUIM_Dialer_LineModified);
         DoMethod(data->STR_String , MUIM_Notify, MUIA_String_Contents     , MUIV_EveryTime , obj, 1, MUIM_Dialer_LineModified);
         DoMethod(data->LV_Script, MUIM_Notify, MUIA_List_Active           , MUIV_EveryTime , obj, 1, MUIM_Dialer_ScriptActive);
         DoMethod(data->BT_Add   , MUIM_Notify, MUIA_Pressed               , FALSE          , data->LI_Script, 3, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
         DoMethod(data->BT_Add   , MUIM_Notify, MUIA_Pressed               , FALSE          , data->LI_Script, 3, MUIM_Set, MUIA_List_Active, MUIV_List_Active_Bottom);
         DoMethod(data->BT_Remove, MUIM_Notify, MUIA_Pressed               , FALSE          , data->LI_Script, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
         DoMethod(data->BT_AddPhone, MUIM_Notify, MUIA_Pressed             , FALSE          , obj, 2, MUIM_Dialer_PopString_AddPhone, TRUE);
         DoMethod(data->BT_CancelPhone, MUIM_Notify, MUIA_Pressed          , FALSE          , obj, 2, MUIM_Dialer_PopString_AddPhone, FALSE);
         DoMethod(data->CY_AutoLogin, MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime , obj, 6, MUIM_MultiSet, MUIA_Disabled, MUIV_TriggerValue, data->LV_Script, data->BT_Add, NULL);

         DoMethod(data->STR_Online        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_OnlineFailed);
         DoMethod(data->STR_OnlineFailed  , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_OfflineActive);
         DoMethod(data->STR_OfflineActive , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_OfflinePassive);
         DoMethod(data->STR_OfflinePassive, MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Startup);
         DoMethod(data->STR_Startup       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Shutdown);
      }
      else
         obj = NULL;
   }
   return((ULONG)obj);
}

///
/// Dialer_Dispatcher
SAVEDS ULONG Dialer_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(Dialer_New               (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_Dialer_PopString_AddPhone)
      return(Dialer_PopString_AddPhone(cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_Dialer_LineModified)
      return(Dialer_LineModified      (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_Dialer_ScriptActive)
      return(Dialer_ScriptActive      (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

