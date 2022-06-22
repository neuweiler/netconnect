/// includes
#include "/includes.h"

#include "/NetConnect.h"
#include "/locale/Strings.h"
#include "mui.h"
#include "mui_EditIcon.h"
#include "mui_DockPrefs.h"
#include "protos.h"

///
/// external variables
extern struct MUI_CustomClass *CL_Editor;
extern struct Hook AppMsgHook, Editor_AppMsgHook;

///

ULONG EditIcon_Editor_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct EditIcon_Data *data = INST_DATA(cl, obj);
   STRPTR ptr;

   DoMethod(data->LI_Editor, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
   if(ptr)
   {
      set(data->STR_Line   , MUIA_Disabled, FALSE);
      set(data->BT_Delete  , MUIA_Disabled, FALSE);
      setstring(data->STR_Line, ptr);
   }
   else
   {
      set(data->STR_Line   , MUIA_Disabled, TRUE);
      set(data->BT_Delete  , MUIA_Disabled, TRUE);
      setstring(data->STR_Line, "");
   }

   return(NULL);
}

ULONG EditIcon_ChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
   struct EditIcon_Data *data = INST_DATA(cl, obj);
   LONG pos;

   pos = xget(data->LI_Editor, MUIA_List_Active);
   if(pos != MUIV_List_Active_Off)
   {
      set(data->LI_Editor, MUIA_List_Quiet, TRUE);
      DoMethod(data->LI_Editor, MUIM_List_InsertSingle, xget(data->STR_Line, MUIA_String_Contents), pos + 1);
      DoMethod(data->LI_Editor, MUIM_List_Remove, pos);
      set(data->LI_Editor, MUIA_List_Quiet, FALSE);

      set(obj, MUIA_Window_ActiveObject, data->STR_Line);
      set(data->LI_Editor, MUIA_UserData, 1);
   }

   return(NULL);
}


ULONG EditIcon_Type_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct EditIcon_Data *data = INST_DATA(cl, obj);
   LONG type;

   type = xget(data->CY_Type, MUIA_Cycle_Active);
   if(type == 2 || type == 3)
   {
      set(data->GR_Script, MUIA_Disabled, FALSE);
      editor_load((STRPTR)xget(data->PA_Program, MUIA_String_Contents), data->LI_Editor);
   }
   else
   {
      DoMethod(data->LI_Editor, MUIM_List_Clear);
      setstring(data->STR_Line, "");
      set(data->LI_Editor, MUIA_UserData, NULL);
      set(data->GR_Script, MUIA_Disabled, TRUE);
   }
   return(NULL);
}

ULONG EditIcon_Sound_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct EditIcon_Data *data = INST_DATA(cl, obj);
   BPTR lock;
   STRPTR ptr;

   lock = Lock((STRPTR)xget(data->PA_Sound, MUIA_String_Contents), ACCESS_READ);
   ptr = (STRPTR)xget(data->PA_Sound, MUIA_String_Contents);
   if(lock)
   {
      UnLock(lock);
      if(ptr && *ptr)
      {
         set(data->BT_PlaySound  , MUIA_Disabled, FALSE);
         set(data->SL_Volume     , MUIA_Disabled, FALSE);
      }
   }
   else
   {
      set(data->BT_PlaySound  , MUIA_Disabled, TRUE);
      set(data->SL_Volume     , MUIA_Disabled, TRUE);
   }
   return(NULL);
}

ULONG EditIcon_PlaySound(struct IClass *cl, Object *obj, Msg msg)
{
   struct EditIcon_Data *data = INST_DATA(cl, obj);

   play_sound((STRPTR)xget(data->PA_Sound, MUIA_String_Contents), xget(data->SL_Volume, MUIA_Numeric_Value));
   return(NULL);
}

ULONG EditIcon_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct EditIcon_Data tmp;
   Object *list, *originator;
   static STRPTR ARR_Register[4];
   static STRPTR CY_ProgramTypes[5];
   struct Icon *icon;

   ARR_Register[0] = GetStr(MSG_LA_MainSettings);
   ARR_Register[1] = GetStr(MSG_LA_AdvancedSettings);
   ARR_Register[2] = GetStr(MSG_LA_ScriptEditor);
   ARR_Register[3] = NULL;

   CY_ProgramTypes[0] = GetStr(MSG_CY_CLI);
   CY_ProgramTypes[1] = GetStr(MSG_CY_Workbench);
   CY_ProgramTypes[2] = GetStr(MSG_CY_Script);
   CY_ProgramTypes[3] = GetStr(MSG_CY_ARexxScript);
   CY_ProgramTypes[4] = NULL;

   icon        = (struct Icon *) GetTagData(MUIA_NetConnect_Icon        , (ULONG)"", msg->ops_AttrList);
   list        = (Object *)      GetTagData(MUIA_NetConnect_List        , (ULONG)"", msg->ops_AttrList);
   originator  = (Object *)      GetTagData(MUIA_NetConnect_Originator  , (ULONG)"", msg->ops_AttrList);

   if(!icon || !list || !originator)
      return(NULL);

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_WI_EditIcon),
      MUIA_Window_ID       , MAKE_ID('I','E','D','I'),
      MUIA_Window_AppWindow, TRUE,
      WindowContents    , VGroup,
         Child, tmp.GR_Register = RegisterObject,
            MUIA_Background, MUII_RegisterBack,
            MUIA_Register_Titles, ARR_Register,
            MUIA_CycleChain, 1,
            Child, tmp.GR_Button = ColGroup(2),
               Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
               Child, tmp.STR_Name     = MakeKeyString((icon->Name ? icon->Name : (STRPTR)""), 81, MSG_CC_Name),
               Child, MakeKeyLabel2(MSG_LA_Hotkey, MSG_CC_Hotkey),
               Child, tmp.STR_Hotkey   = MakeKeyString((icon->Hotkey ? icon->Hotkey : (STRPTR)""), 41, MSG_CC_Hotkey),
               Child, MakeKeyLabel2(MSG_LA_ProgramType, MSG_CC_ProgramType),
               Child, tmp.CY_Type      = MakeKeyCycle(CY_ProgramTypes, MSG_CC_ProgramType),
               Child, MakeKeyLabel2(MSG_LA_Program, MSG_CC_Program),
               Child, tmp.PA_Program   = MakePopAsl(MakeKeyString((icon->Program.File ? icon->Program.File : (STRPTR)""), MAXPATHLEN, MSG_CC_Program), MSG_LA_Program, FALSE),
               Child, MakeKeyLabel2(MSG_LA_Image, MSG_CC_Image),
               Child, tmp.PA_Image     = MakePopAsl(MakeKeyString((icon->ImageFile ? icon->ImageFile : (STRPTR)""), MAXPATHLEN, MSG_CC_Image), MSG_LA_Image, FALSE),
               Child, MakeKeyLabel2(MSG_LA_Sound, MSG_CC_Sound),
               Child, HGroup,
                  MUIA_Group_Spacing, 0,
                  Child, tmp.PA_Sound  = MakePopAsl(MakeKeyString((icon->Sound ? icon->Sound : (STRPTR)""), MAXPATHLEN, MSG_CC_Sound), MSG_LA_Sound, FALSE),
                  Child, tmp.BT_PlaySound = ImageObject,
                     ImageButtonFrame,
                     MUIA_CycleChain      , 1,
                     MUIA_InputMode       , MUIV_InputMode_RelVerify,
                     MUIA_Image_Spec      , MUII_TapePlay,
                     MUIA_Image_FreeVert  , TRUE,
                     MUIA_Background      , MUII_ButtonBack,
                  End,
               End,
               Child, MakeKeyLabel2(MSG_LA_Volume, MSG_CC_Volume),
               Child, tmp.SL_Volume    = MakeKeySlider(0, 64, icon->Volume, MSG_CC_Volume),
            End,
            Child, tmp.GR_Advanced = VGroup,
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_Stack, MSG_CC_Stack),
                  Child, tmp.STR_Stack = StringObject,
                     MUIA_ControlChar     , *GetStr(MSG_CC_Stack),
                     MUIA_CycleChain      , 1,
                     StringFrame,
                     MUIA_String_MaxLen   , 8,
                     MUIA_String_Integer  , (icon->Program.Stack < 1024 ? 8192 : icon->Program.Stack),
                     MUIA_String_Accept   , "1234567890",
                  End,
                  Child, MakeKeyLabel2(MSG_LA_Priority, MSG_CC_Priority),
                  Child, tmp.SL_Priority        = MakeKeySlider(-128, 127, icon->Program.Priority, MSG_CC_Priority),
                  Child, MakeKeyLabel2(MSG_LA_CurrentDir, MSG_CC_CurrentDir),
                  Child, tmp.PA_CurrentDir      = MakePopAsl(MakeKeyString((icon->Program.CurrentDir ? icon->Program.CurrentDir : (STRPTR)""), MAXPATHLEN, MSG_CC_CurrentDir), MSG_LA_CurrentDir, TRUE),
                  Child, MakeKeyLabel2(MSG_LA_OutputFile, MSG_CC_OutputFile),
                  Child, tmp.PA_OutputFile      = MakePopAsl(MakeKeyString((icon->Program.OutputFile ? icon->Program.OutputFile : (STRPTR)""), MAXPATHLEN, MSG_CC_OutputFile), MSG_LA_OutputFile, FALSE),
                  Child, MakeKeyLabel2(MSG_LA_PublicScreen, MSG_CC_PublicScreen),
                  Child, tmp.STR_PublicScreen   = MakeKeyString((icon->Program.PublicScreen ? icon->Program.PublicScreen : (STRPTR)""), 81, MSG_CC_PublicScreen),
               End,
               Child, ColGroup(7),
                  Child, HVSpace,
                  Child, tmp.CH_WBArgs    = MakeKeyCheckMark(icon->Program.Flags & PRG_Arguments, MSG_CC_WBArguments),
                  Child, KeyLLabel2(GetStr(MSG_LA_WBArguments), *GetStr(MSG_CC_WBArguments)),
                  Child, HVSpace,
                  Child, tmp.CH_DrawFrame = MakeKeyCheckMark(icon->Flags & IFL_DrawFrame, MSG_CC_DrawFrame),
                  Child, KeyLLabel2(GetStr(MSG_LA_DrawFrame), *GetStr(MSG_CC_DrawFrame)),
                  Child, HVSpace,
                  Child, HVSpace,
                  Child, tmp.CH_ToFront   = MakeKeyCheckMark(icon->Program.Flags & PRG_ToFront, MSG_CC_PubScreen2Front),
                  Child, KeyLLabel2(GetStr(MSG_LA_PubScreen2Front), *GetStr(MSG_CC_PubScreen2Front)),
                  Child, HVSpace,
                  Child, HVSpace,
                  Child, HVSpace,
                  Child, HVSpace,
               End,
            End,
            Child, tmp.GR_Script = VGroup,
               MUIA_Disabled     , TRUE,
               MUIA_Group_Spacing, 0,
               Child, tmp.LV_Editor = ListviewObject,
                  MUIA_CycleChain, 1,
                  MUIA_Listview_DragType     , 1,
                  MUIA_Listview_List         , tmp.LI_Editor = NewObject(CL_Editor->mcc_Class, NULL, TAG_DONE),
               End,
               Child, HGroup,
                  MUIA_Group_Spacing, 0,
                  Child, tmp.BT_New    = MakeButton(MSG_BT_New1),
                  Child, tmp.BT_Delete = MakeButton(MSG_BT_Delete1),
                  Child, tmp.BT_Clear  = MakeButton(MSG_BT_Clear),
               End,
               Child, tmp.STR_Line = MakeKeyString("", MAXPATHLEN, "   "),
            End,
         End,
         Child, HGroup,
            MUIA_Group_SameSize  , TRUE,
            Child, tmp.BT_Okay   = MakeButton(MSG_BT_Okay),
            Child, HSpace(0),
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct EditIcon_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(tmp.STR_Name        , MUIA_ShortHelp, GetStr(MSG_HELP_IconName));
      set(tmp.CY_Type         , MUIA_ShortHelp, GetStr(MSG_HELP_Type));
      set(tmp.PA_Program      , MUIA_ShortHelp, GetStr(MSG_HELP_Program));
      set(tmp.STR_Hotkey      , MUIA_ShortHelp, GetStr(MSG_HELP_IconHotkey));
      set(tmp.PA_Image        , MUIA_ShortHelp, GetStr(MSG_HELP_Image));
      set(tmp.PA_Sound        , MUIA_ShortHelp, GetStr(MSG_HELP_Sound));
      set(tmp.BT_PlaySound    , MUIA_ShortHelp, GetStr(MSG_HELP_PlaySound));
      set(tmp.SL_Volume       , MUIA_ShortHelp, GetStr(MSG_HELP_Volume));
      set(tmp.STR_Stack       , MUIA_ShortHelp, GetStr(MSG_HELP_Stack));
      set(tmp.SL_Priority     , MUIA_ShortHelp, GetStr(MSG_HELP_Priority));
      set(tmp.PA_CurrentDir   , MUIA_ShortHelp, GetStr(MSG_HELP_CurrentDir));
      set(tmp.PA_OutputFile   , MUIA_ShortHelp, GetStr(MSG_HELP_OutputFile));
      set(tmp.STR_PublicScreen, MUIA_ShortHelp, GetStr(MSG_HELP_PublicScreen));
      set(tmp.CH_WBArgs       , MUIA_ShortHelp, GetStr(MSG_HELP_WBArgs));
      set(tmp.CH_DrawFrame    , MUIA_ShortHelp, GetStr(MSG_HELP_DrawFrame));
      set(tmp.CH_ToFront      , MUIA_ShortHelp, GetStr(MSG_HELP_ToFront));
      set(tmp.LV_Editor       , MUIA_ShortHelp, GetStr(MSG_HELP_Editor));
      set(tmp.BT_New          , MUIA_ShortHelp, GetStr(MSG_HELP_NewLine));
      set(tmp.BT_Delete       , MUIA_ShortHelp, GetStr(MSG_HELP_DeleteLine));
      set(tmp.BT_Clear        , MUIA_ShortHelp, GetStr(MSG_HELP_Clear));
      set(tmp.BT_Okay         , MUIA_ShortHelp, GetStr(MSG_HELP_Okay));
      set(tmp.BT_Cancel       , MUIA_ShortHelp, GetStr(MSG_HELP_Cancel));

      set(tmp.STR_Line  , MUIA_String_AttachedList, tmp.LV_Editor);
      set(tmp.STR_Line  , MUIA_CycleChain, 1);
      set(tmp.STR_Line  , MUIA_Disabled, TRUE);
      set(tmp.BT_Delete , MUIA_Disabled, TRUE);
      set(tmp.SL_Volume , MUIA_Disabled, TRUE);

      DoMethod(tmp.LI_Editor     , MUIM_Notify, MUIA_List_Active        , MUIV_EveryTime  , obj          , 1, MUIM_EditIcon_Editor_Active);
      DoMethod(tmp.BT_New        , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Editor, 3, MUIM_List_InsertSingle, "", MUIV_List_Insert_Bottom);
      DoMethod(tmp.BT_New        , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Editor, 3, MUIM_Set, MUIA_List_Active, MUIV_List_Active_Bottom);
      DoMethod(tmp.BT_New        , MUIM_Notify, MUIA_Pressed            , FALSE           , obj          , 3, MUIM_Set, MUIA_Window_ActiveObject, tmp.STR_Line);
      DoMethod(tmp.BT_New        , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Editor, 3, MUIM_Set, MUIA_UserData, 1);
      DoMethod(tmp.BT_Delete     , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Editor, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(tmp.BT_Delete     , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Editor, 3, MUIM_Set, MUIA_UserData, 1);
      DoMethod(tmp.BT_Clear      , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Editor, 1, MUIM_List_Clear);
      DoMethod(tmp.BT_Clear      , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Editor, 3, MUIM_Set, MUIA_UserData, 1);
      DoMethod(tmp.STR_Line      , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj          , 1, MUIM_EditIcon_ChangeLine);

      DoMethod(tmp.STR_Name      , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.STR_Name , 3, MUIM_CallHook   , &AppMsgHook        , MUIV_TriggerValue);
      DoMethod(tmp.PA_Program    , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.PA_Program, 3, MUIM_CallHook  , &AppMsgHook        , MUIV_TriggerValue);
      DoMethod(tmp.PA_Image      , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.PA_Image , 3, MUIM_CallHook   , &AppMsgHook        , MUIV_TriggerValue);
      DoMethod(tmp.PA_Sound      , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.PA_Sound , 3, MUIM_CallHook   , &AppMsgHook        , MUIV_TriggerValue);
      DoMethod(tmp.PA_CurrentDir , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.PA_CurrentDir, 3, MUIM_CallHook  , &AppMsgHook        , MUIV_TriggerValue);
      DoMethod(tmp.PA_OutputFile , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.PA_OutputFile, 3, MUIM_CallHook  , &AppMsgHook        , MUIV_TriggerValue);
      DoMethod(tmp.LV_Editor     , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.LI_Editor, 3, MUIM_CallHook   , &Editor_AppMsgHook , MUIV_TriggerValue);

      DoMethod(tmp.CY_Type       , MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime  , obj          , 1, MUIM_EditIcon_Type_Active);
      DoMethod(tmp.PA_Program    , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj          , 1, MUIM_EditIcon_Type_Active);
      DoMethod(tmp.PA_Sound      , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj          , 1, MUIM_EditIcon_Sound_Active);
      DoMethod(tmp.BT_PlaySound  , MUIM_Notify, MUIA_Pressed            , FALSE           , obj          , 1, MUIM_EditIcon_PlaySound);

      DoMethod(obj               , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , MUIV_Notify_Application, 8, MUIM_Application_PushMethod, originator, 5, MUIM_DockPrefs_EditIcon_Finish, icon, list, 0);
      DoMethod(tmp.BT_Cancel     , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 8, MUIM_Application_PushMethod, originator, 5, MUIM_DockPrefs_EditIcon_Finish, icon, list, 0);
      DoMethod(tmp.BT_Okay       , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 8, MUIM_Application_PushMethod, originator, 5, MUIM_DockPrefs_EditIcon_Finish, icon, list, 1);
   }

   return((ULONG)obj);
}

SAVEDS ASM ULONG EditIcon_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                      : return(EditIcon_New            (cl, obj, (APTR)msg));
      case MUIM_EditIcon_Editor_Active : return(EditIcon_Editor_Active  (cl, obj, (APTR)msg));
      case MUIM_EditIcon_ChangeLine    : return(EditIcon_ChangeLine     (cl, obj, (APTR)msg));
      case MUIM_EditIcon_Type_Active   : return(EditIcon_Type_Active    (cl, obj, (APTR)msg));
      case MUIM_EditIcon_Sound_Active  : return(EditIcon_Sound_Active   (cl, obj, (APTR)msg));
      case MUIM_EditIcon_PlaySound     : return(EditIcon_PlaySound      (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}


