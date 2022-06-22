#include "globals.c"
#include "protos.h"

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
   STRPTR ptr;

   DoMethod(data->LI_Script, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
   setstring(data->STR_Line, (ptr ? ptr : (STRPTR)""));
   set(data->PO_Line, MUIA_Disabled, !ptr);
   set(data->BT_Remove, MUIA_Disabled, !ptr);
   return(NULL);
}

///
/// Dialer_LineModified
ULONG Dialer_LineModified(struct IClass *cl, Object *obj, Msg msg)
{
   struct Dialer_Data *data = INST_DATA(cl, obj);
   STRPTR ptr;

   DoMethod(data->LI_Script, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
   if(ptr)
   {
      strcpy(ptr, (STRPTR)xget(data->STR_Line, MUIA_String_Contents));
      DoMethod(data->LI_Script, MUIM_List_Redraw, MUIV_List_Redraw_Active);
   }

   return(NULL);
}

///

/// Script_ConstructFunc
SAVEDS ASM STRPTR Script_ConstructFunc(REG(a2) APTR pool, REG(a1) STRPTR src)
{
   STRPTR new;

   if((new = (STRPTR)AllocVec(MAXPATHLEN + 1, MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, MAXPATHLEN + 1);
   return(new);
}

///
/// Dialer_New
ULONG Dialer_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Dialer_Data tmp;
   static const struct Hook Script_ConstructHook= { { 0,0 }, (VOID *)Script_ConstructFunc , NULL, NULL };
   static STRPTR ARR_CY_Window[] = { "-", "open", "close", NULL };
   static STRPTR ARR_CY_Login[] = { "Login automatically", "Login manually", NULL };
   static STRPTR ARR_PO_Lines[] = { "Dial", "SendLogin", "SendPassword", "Send", "WaitFor", "GoOnline", "SendBreak", "Pause", NULL };

   if(obj = tmp.GR_Script = (Object *)DoSuperNew(cl, obj,
      MUIA_InnerLeft, 0,
      MUIA_InnerRight, 0,
      MUIA_InnerBottom, 0,
      MUIA_InnerTop, 0,
      Child, VGroup,
         MUIA_Group_Spacing, 0,
         Child, tmp.LV_Script = ListviewObject,
            MUIA_CycleChain            , 1,
            MUIA_Listview_DragType     , 1,
            MUIA_Listview_DoubleClick  , TRUE,
            MUIA_Listview_List         , tmp.LI_Script = ListObject,
               MUIA_Frame              , MUIV_Frame_InputList,
               MUIA_List_ConstructHook , &Script_ConstructHook,
               MUIA_List_DestructHook  , &des_hook,
               MUIA_List_DragSortable  , TRUE,
            End,
         End,
         Child, HGroup,
            MUIA_Group_Spacing, 0,
            Child, tmp.BT_Add = MakeButton("  _Add"),
            Child, tmp.BT_Remove = MakeButton("  _Remove"),
         End,
         Child, tmp.PO_Line = PopobjectObject,
            MUIA_Popstring_String      , tmp.STR_Line = MakeKeyString(NULL, MAXPATHLEN, "  l"),
            MUIA_Popstring_Button      , PopButton(MUII_PopUp),
            MUIA_Popobject_StrObjHook  , &strobjhook,
            MUIA_Popobject_ObjStrHook  , &objstrhook,
            MUIA_Popobject_Object      , tmp.LV_Lines = ListviewObject,
               MUIA_Listview_DoubleClick  , TRUE,
               MUIA_Listview_List         , ListObject,
                  MUIA_Frame              , MUIV_Frame_InputList,
                  MUIA_List_AutoVisible   , TRUE,
                  MUIA_List_SourceArray   , ARR_PO_Lines,
               End,
            End,
         End,
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
         set(data->PO_Line, MUIA_Disabled, TRUE);
         set(data->BT_Remove, MUIA_Disabled, TRUE);
         set(data->STR_Line, MUIA_String_AttachedList, data->LV_Script);
         set(data->CY_WinStartup, MUIA_Cycle_Active, 1);

         DoMethod(data->LV_Lines , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_Line, 2, MUIM_Popstring_Close, TRUE);
         DoMethod(data->STR_Line , MUIM_Notify, MUIA_String_Contents       , MUIV_EveryTime , obj, 1, MUIM_Dialer_LineModified);
         DoMethod(data->LV_Script, MUIM_Notify, MUIA_List_Active           , MUIV_EveryTime , obj, 1, MUIM_Dialer_ScriptActive);
         DoMethod(data->LV_Script, MUIM_Notify, MUIA_List_Active           , MUIV_EveryTime , win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Line);
         DoMethod(data->BT_Add   , MUIM_Notify, MUIA_Pressed               , FALSE          , data->LI_Script, 3, MUIM_List_InsertSingle, "", MUIV_List_Insert_Bottom);
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
SAVEDS ASM ULONG Dialer_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                            : return(Dialer_New               (cl, obj, (APTR)msg));
      case MUIM_Dialer_PopString_AddPhone    : return(Dialer_PopString_AddPhone(cl, obj, (APTR)msg));
      case MUIM_Dialer_LineModified          : return(Dialer_LineModified      (cl, obj, (APTR)msg));
      case MUIM_Dialer_ScriptActive          : return(Dialer_ScriptActive      (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

