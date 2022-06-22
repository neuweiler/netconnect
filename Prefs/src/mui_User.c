#include "globals.c"
#include "protos.h"

/// User_InsertFile
ULONG User_InsertFile(struct IClass *cl, Object *obj, struct MUIP_User_InsertFile *msg)
{
   struct User_Data *data = INST_DATA(cl, obj);
   STRPTR file, buf;
   BPTR fh;
   LONG size;

   if(file = getfilename(_win(obj), "Choose textfile to insert", NULL, FALSE))
   {
      if((size = get_file_size(file)) > 0)
      {
         if(buf = AllocVec(size + 8, MEMF_ANY))
         {
            if(fh = Open(file, MODE_OLDFILE))
            {
               Read(fh, buf, size);
               Close(fh);
               DoMethod((msg->stopnet ? data->TI_UserStopnet : data->TI_UserStartnet), MUIM_Textinput_InsertText, buf, size);
            }
            FreeVec(buf);
         }
      }
   }

   return(NULL);
}

///
/// User_Popstring_UserName
ULONG User_Popstring_UserName(struct IClass *cl, Object *obj, struct MUIP_User_PopString_UserName *msg)
{
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct Databases_Data *db_data = INST_DATA(CL_Databases->mcc_Class, mw_data->GR_Databases);
   struct User_Data *data = INST_DATA(cl, obj);
   struct User *user;
   STRPTR ptr;
   LONG pos;

   DoMethod(data->LV_UserName, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
   if(ptr)
   {
      set(data->STR_UserName, MUIA_String_Contents, ptr);

      pos = 0;
      FOREVER
      {
         DoMethod(db_data->LV_Users, MUIM_NList_GetEntry, pos++, &user);
         if(!user)
            break;
         if(!strcmp(user->Login, ptr))
            break;
      }
      if(user)
         setstring(data->STR_RealName, user->Name);
   }
   DoMethod(data->PO_UserName, MUIM_Popstring_Close, TRUE);

   return(NULL);
}

///


/// Textinput_AppMsgFunc
SAVEDS ASM LONG Textinput_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
   struct WBArg *ap;
   struct AppMessage *amsg = *x;
   STRPTR buf;

   ap = amsg->am_ArgList;
   if(amsg->am_NumArgs)
   {
      if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
      {
         NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
         AddPart(buf, ap->wa_Name, MAXPATHLEN);
         strncat(buf, "\n", MAXPATHLEN);
         DoMethod(obj, MUIM_Textinput_InsertText, buf, strlen(buf));
         FreeVec(buf);
      }
   }
   return(NULL);
}

///
/// UserName_strobjfunc
SAVEDS ASM LONG UserName_strobjfunc(REG(a2) Object *list, REG(a1) Object *str)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct Databases_Data *db_data = INST_DATA(CL_Databases->mcc_Class, data->GR_Databases);
   struct User *user;
   char *x, *s;
   int i;

   get(str, MUIA_String_Contents, &s);

   DoMethod(list, MUIM_List_Clear);
   i = 0;
   FOREVER     // copy usernames from passwd
   {
      DoMethod(db_data->LV_Users, MUIM_List_GetEntry, i++, &user);
      if(!user)
         break;
      DoMethod(list, MUIM_List_InsertSingle, user->Login, MUIV_List_Insert_Bottom);
   }

   i = 0;
   FOREVER
   {
      DoMethod(list, MUIM_List_GetEntry, i, &x);
      if(!x)
      {
         set(list, MUIA_List_Active, MUIV_List_Active_Off);
         break;
      }
      else if(!stricmp(x, s))
      {
            set(list, MUIA_List_Active, i);
            break;
      }
      i++;
   }
   return(TRUE);
}

///

/// User_New
ULONG User_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static const struct Hook Textinput_AppMsgHook = { {NULL, NULL}, (VOID *)Textinput_AppMsgFunc, NULL, NULL };
   static const struct Hook UserName_strobjhook = { {NULL, NULL}, (VOID *)UserName_strobjfunc, NULL, NULL };
   static STRPTR ARR_UserName[] = { "same as login", "specify", NULL };
   struct User_Data tmp;

   if(obj = tmp.GR_User = (Object *)DoSuperNew(cl, obj,
MUIA_InnerLeft, 0,
MUIA_InnerRight, 0,
MUIA_InnerBottom, 0,
MUIA_InnerTop, 0,
      Child, HVSpace,
      Child, MUI_MakeObject(MUIO_BarTitle, "User identification"),
      Child, ColGroup(2),
         Child, MakeKeyLabel2("  Login:", MSG_CC_LoginName),
         Child, tmp.STR_LoginName = MakeKeyString("", 80, MSG_CC_LoginName),
         Child, MakeKeyLabel2(MSG_LA_Password, MSG_CC_Password),
         Child, tmp.STR_Password = TextinputObject,
            MUIA_ControlChar     , *GetStr(MSG_CC_Password),
            MUIA_CycleChain      , 1,
            MUIA_Frame           , MUIV_Frame_String,
            MUIA_String_Secret   , TRUE,
            MUIA_String_MaxLen   , 80,
         End,
      End,
      Child, HVSpace,
      Child, MUI_MakeObject(MUIO_BarTitle, "Additional information"),
      Child, ColGroup(2),
         Child, MakeKeyLabel2("  User Name:", "  u"),
         Child, HGroup,
            Child, tmp.PO_UserName = PopobjectObject,
               MUIA_Popstring_String      , tmp.STR_UserName = MakeKeyString(NULL, 80, "  u"),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &UserName_strobjhook,
               MUIA_Popobject_ObjStrHook  , &objstrhook,
               MUIA_Popobject_Object      , tmp.LV_UserName = ListviewObject,
                  MUIA_CycleChain      , 1,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                     MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
                     MUIA_List_AutoVisible   , TRUE,
                  End,
               End,
            End,
            Child, tmp.CY_UserName = Cycle(ARR_UserName),
         End,
         Child, MakeKeyLabel2(MSG_LA_RealName, MSG_CC_RealName),
         Child, tmp.STR_RealName = MakeKeyString("", 80, MSG_CC_RealName),
         Child, MakeKeyLabel2(MSG_LA_EMail, MSG_CC_EMail),
         Child, tmp.STR_EMail = MakeKeyString("", 80, MSG_CC_EMail),
         Child, MakeKeyLabel2(MSG_LA_Organisation, MSG_CC_Organisation),
         Child, tmp.STR_Organisation = MakeKeyString("Private User", 80, MSG_CC_Organisation),
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct User_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->GR_UserStartnet = VGroup,
MUIA_InnerLeft, 0,
MUIA_InnerRight, 0,
MUIA_InnerBottom, 0,
MUIA_InnerTop, 0,
         MUIA_Group_Spacing, 0,
         Child, data->TI_UserStartnet = TextinputscrollObject,
            StringFrame,
            MUIA_CycleChain      , 1,
            MUIA_Textinput_Multiline, TRUE,
            MUIA_Textinput_WordWrap, 257,
            MUIA_Textinput_AutoExpand, TRUE,
         End,
         Child, HGroup,
            MUIA_Group_Spacing, 0,
            Child, data->BT_StartInsert = MakeButton("  _Insert file"),
            Child, data->BT_StartClear = MakeButton("  _Clear"),
         End,
      End;

      data->GR_UserStopnet = VGroup,
MUIA_InnerLeft, 0,
MUIA_InnerRight, 0,
MUIA_InnerBottom, 0,
MUIA_InnerTop, 0,
         MUIA_Group_Spacing, 0,
         Child, data->TI_UserStopnet = TextinputscrollObject,
            StringFrame,
            MUIA_CycleChain      , 1,
            MUIA_Textinput_Multiline, TRUE,
            MUIA_Textinput_WordWrap, 257,
            MUIA_Textinput_AutoExpand, TRUE,
         End,
         Child, HGroup,
            MUIA_Group_Spacing, 0,
            Child, data->BT_StopInsert = MakeButton("  _Insert file"),
            Child, data->BT_StopClear = MakeButton("  _Clear"),
         End,
      End;

      if(data->GR_UserStartnet && data->GR_UserStopnet)
      {
         set(data->STR_LoginName   , MUIA_ShortHelp, GetStr(MSG_Help_LoginName));
         set(data->STR_Password    , MUIA_ShortHelp, GetStr(MSG_Help_Password));
         set(data->STR_EMail       , MUIA_ShortHelp, GetStr(MSG_Help_EMail));
         set(data->STR_RealName    , MUIA_ShortHelp, GetStr(MSG_Help_RealName));
         set(data->STR_Organisation, MUIA_ShortHelp, GetStr(MSG_Help_Organisation));

         set(data->TI_UserStartnet , MUIA_ShortHelp, GetStr(MSG_Help_UserStartnet));
         set(data->TI_UserStopnet  , MUIA_ShortHelp, GetStr(MSG_Help_UserStopnet));

         set(data->CY_UserName, MUIA_Weight, 0);
         set(data->PO_UserName, MUIA_Disabled, TRUE);

         DoMethod(data->LV_UserName    , MUIM_Notify, MUIA_Listview_DoubleClick, TRUE          , obj, 1, MUIM_User_Popstring_UserName);
         DoMethod(data->CY_UserName    , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime, data->PO_UserName, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

         DoMethod(data->TI_UserStartnet, MUIM_Notify, MUIA_AppMessage, MUIV_EveryTime, data->TI_UserStartnet , 3, MUIM_CallHook, &Textinput_AppMsgHook, MUIV_TriggerValue);
         DoMethod(data->TI_UserStopnet , MUIM_Notify, MUIA_AppMessage, MUIV_EveryTime, data->TI_UserStopnet  , 3, MUIM_CallHook, &Textinput_AppMsgHook, MUIV_TriggerValue);

         DoMethod(data->BT_StartInsert , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_User_InsertFile, 0);
         DoMethod(data->BT_StopInsert  , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_User_InsertFile, 1);
         DoMethod(data->BT_StartClear  , MUIM_Notify, MUIA_Pressed, FALSE, data->TI_UserStartnet, 3, MUIM_Set, MUIA_Textinput_Contents, "");
         DoMethod(data->BT_StopClear   , MUIM_Notify, MUIA_Pressed, FALSE, data->TI_UserStopnet, 3, MUIM_Set, MUIA_Textinput_Contents, "");

         DoMethod(data->STR_LoginName, MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Password);
         DoMethod(data->STR_Password , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_EMail);
         DoMethod(data->STR_EMail    , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_RealName);
         DoMethod(data->STR_RealName , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Organisation);
      }
      else
         obj = NULL;
   }
   return((ULONG)obj);
}

///
/// User_Dispatcher
SAVEDS ASM ULONG User_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                            : return(User_New                      (cl, obj, (APTR)msg));
      case MUIM_User_InsertFile              : return(User_InsertFile               (cl, obj, (APTR)msg));
      case MUIM_User_Popstring_UserName      : return(User_Popstring_UserName       (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

