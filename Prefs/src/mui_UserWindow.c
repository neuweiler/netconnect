/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_UserWindow.h"
#include "mui_User.h"
#include "mui_PasswdReq.h"
#include "protos.h"

///
/// external variables
extern struct Hook strobjhook, des_hook;
extern struct MUI_CustomClass *CL_PasswdReq;
extern Object *win, *app;

///
/*
      case MUIV_Databases_Modification_Disable:
         DoMethod(data->LV_Users, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user);
         if(user)
         {
            user->Disabled = xget(data->CH_Disabled, MUIA_Selected);
            set(data->BT_ChangePassword, MUIA_Disabled, user->Disabled);
            set(data->BT_RemovePassword, MUIA_Disabled, !*user->Password || user->Disabled);
            DoMethod(data->LV_Users, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
         }
         break;
      case MUIV_Databases_Modification_ChangePassword:
      {
         Object *window;

         set(app, MUIA_Application_Sleep, TRUE);
         if(window = NewObject(CL_PasswdReq->mcc_Class, NULL, MUIA_Genesis_Originator, obj, TAG_DONE))
         {
            DoMethod(app, OM_ADDMEMBER, window);
            set(window, MUIA_Window_Open, TRUE);
         }
      }
      break;
      case MUIV_Databases_Modification_RemovePassword:
         DoMethod(data->LV_Users, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user);
         if(user)
         {
            if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_ReallyRemovePasswd), GetStr(MSG_TX_ReallyRemovePasswd)))
            {
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_Okay), GetStr(MSG_TX_PasswordWarning));
               *user->Password = NULL;
               set(data->BT_RemovePassword, MUIA_Disabled, TRUE);
            }
            DoMethod(data->LV_Users, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
         }
         break;
*/
/// UserWindow_PasswdReqFinish
ULONG UserWindow_PasswdReqFinish(struct IClass *cl, Object *obj, struct MUIP_Genesis_Finish *msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);

   if(msg->status == 1)
   {
      struct PasswdReq_Data *pw_data = INST_DATA(CL_PasswdReq->mcc_Class, msg->obj);

      if(strcmp((STRPTR)xget(pw_data->STR_pw1, MUIA_String_Contents), (STRPTR)xget(pw_data->STR_pw2, MUIA_String_Contents)))
      {
         MUI_Request(app, win, NULL, NULL, "*_Okay", "The passwords you have\nentered are not equal.");
         setstring(pw_data->STR_pw1, NULL);
         setstring(pw_data->STR_pw2, NULL);
         set(msg->obj, MUIA_Window_ActiveObject, pw_data->STR_pw1);
         return(NULL);
      }
      else
      {
         if(strlen((STRPTR)xget(pw_data->STR_pw1, MUIA_String_Contents)))
         {
            if(!UserGroupBase)
               UserGroupBase = OpenLibrary(USERGROUPNAME, 0);

            if(UserGroupBase)
            {
               strcpy(data->user->us_password, crypt((STRPTR)xget(pw_data->STR_pw1, MUIA_String_Contents), data->user->us_password));
               CloseLibrary(UserGroupBase);
               UserGroupBase = NULL;
            }
         }
         else
            MUI_Request(app, win, NULL, NULL, "*_Abort", "Unable to change password.");
      }
   }
   set(msg->obj, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, msg->obj),
   MUI_DisposeObject(msg->obj);
   set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///


/// UserWindow_Init
ULONG UserWindow_Init(struct IClass *cl, Object *obj, struct MUIP_UserWindow_Init *msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);

   data->user = msg->user;

   setstring(data->STR_Name         , data->user->us_login);
   setstring(data->STR_RealName     , data->user->us_realname);
   setstring(data->STR_MailLogin    , data->user->us_maillogin);
   setstring(data->STR_MailPassword , data->user->us_mailpassword);
   setstring(data->STR_EMail        , data->user->us_email);
   setstring(data->STR_MailServer   , data->user->us_mailserver);
   setcheckmark(data->CH_Disabled   , !strcmp(data->user->us_password, "*"));
   setstring(data->STR_HomeDir      , data->user->us_homedir);
   setstring(data->STR_Shell        , data->user->us_shell);
   set(data->STR_UserID             , MUIA_String_Integer, data->user->us_uid);
   set(data->STR_GroupID            , MUIA_String_Integer, data->user->us_gid);

   return(NULL);
}
///
/// UserWindow_CopyData
ULONG UserWindow_CopyData(struct IClass *cl, Object *obj, Msg msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);

   strcpy(data->user->us_login         , (STRPTR)xget(data->STR_Name         , MUIA_String_Contents));
   strcpy(data->user->us_realname      , (STRPTR)xget(data->STR_RealName     , MUIA_String_Contents));
   strcpy(data->user->us_maillogin     , (STRPTR)xget(data->STR_MailLogin    , MUIA_String_Contents));
   strcpy(data->user->us_mailpassword  , (STRPTR)xget(data->STR_MailPassword , MUIA_String_Contents));
   strcpy(data->user->us_email         , (STRPTR)xget(data->STR_EMail        , MUIA_String_Contents));
   strcpy(data->user->us_mailserver    , (STRPTR)xget(data->STR_MailServer   , MUIA_String_Contents));

   if(xget(data->CH_Disabled, MUIA_Selected))
      strcpy(data->user->us_password, "*");
   strcpy(data->user->us_homedir       , (STRPTR)xget(data->STR_HomeDir      , MUIA_String_Contents));
   strcpy(data->user->us_shell         , (STRPTR)xget(data->STR_Shell        , MUIA_String_Contents));
   data->user->us_uid = xget(data->STR_UserID  , MUIA_String_Integer);
   data->user->us_gid = xget(data->STR_GroupID , MUIA_String_Integer);

   return(NULL);
}
///

/// UserWindow_New
ULONG UserWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct UserWindow_Data tmp;
   static STRPTR ARR_Pages[] = { "User settings", "Advanced", NULL };
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , "Edit User",
      MUIA_Window_ID       , MAKE_ID('U','S','R','E'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      WindowContents       , VGroup,
         Child, RegisterGroup(ARR_Pages),
            Child, VGroup,
               Child, ColGroup(2),
                  Child, MakeKeyLabel2("  User Name:", "  n"),
                  Child, tmp.STR_Name = MakeKeyString(NULL, 80, "  n"),
                  Child, MakeKeyLabel2("  Real Name:", "  o"),
                  Child, tmp.STR_RealName = MakeKeyString(NULL, 80, "  o"),
                  Child, MakeKeyLabel2("  EMail:", "  o"),
                  Child, tmp.STR_EMail = MakeKeyString(NULL, 80, "  e"),
                  Child, MakeKeyLabel2("  Mail Login:", "  l"),
                  Child, HGroup,
                     Child, tmp.STR_MailLogin = MakeKeyString(NULL, 80, "  l"),
                     Child, MakeKeyLabel2("  Mail Password:", "  p"),
                     Child, tmp.STR_MailPassword = TextinputObject,
                        MUIA_ControlChar     , *GetStr("  p"),
                        MUIA_CycleChain      , 1,
                        MUIA_Frame           , MUIV_Frame_String,
                        MUIA_String_Secret   , TRUE,
                        MUIA_String_MaxLen   , 80,
                     End,
                  End,
                  Child, MakeKeyLabel2("  Mail Server:", "  v"),
                  Child, tmp.STR_MailServer = MakeKeyString(NULL, 80, "  v"),
               End,
            End,
            Child, ColGroup(2),
               Child, MakeKeyLabel2(MSG_LA_DisableUser, MSG_CC_DisableUser),
               Child, HGroup,
                  Child, tmp.CH_Disabled  = MakeKeyCheckMark(FALSE, MSG_CC_DisableUser),
                  Child, HVSpace,
               End,
               Child, MakeKeyLabel2(MSG_LA_HomeDir, MSG_CC_HomeDir),
               Child, HGroup,
                  Child, tmp.PA_HomeDir   = MakePopAsl(tmp.STR_HomeDir = MakeKeyString(NULL, MAXPATHLEN, MSG_CC_FullName), GetStr(MSG_TX_ChooseHomeDir), TRUE),
                  Child, HGroup,
                     Child, MakeKeyLabel2(MSG_LA_Shell, MSG_CC_Shell),
                     Child, tmp.STR_Shell = MakeKeyString(NULL, 80, MSG_CC_Shell),
                  End,
               End,
               Child, MakeKeyLabel2(MSG_LA_UserID, MSG_CC_UserID),
               Child, HGroup,
                  Child, tmp.STR_UserID   = TextinputObject,
                     StringFrame,
                     MUIA_ControlChar     , *GetStr(MSG_CC_UserID),
                     MUIA_CycleChain      , 1,
                     MUIA_String_MaxLen   , 7,
                     MUIA_String_Accept   , "1234567890",
                  End,
                  Child, tmp.BT_ChangePassword = MakeButton(MSG_BT_ChangePassword),
               End,
               Child, MakeKeyLabel2(MSG_LA_UserGID, MSG_CC_UserGID),
               Child, HGroup,
                  Child, tmp.STR_GroupID        = MakeKeyString(NULL, 7, MSG_CC_UserGID),
                  Child, tmp.BT_RemovePassword  = MakeButton(MSG_BT_RemovePassword),
               End,
            End,
         End,
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton("  _Okay"),
            Child, tmp.BT_Cancel = MakeButton("  _Cancel"),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct UserWindow_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(data->STR_MailLogin       , MUIA_ShortHelp, GetStr(MSG_Help_LoginName));
      set(data->STR_MailPassword    , MUIA_ShortHelp, GetStr(MSG_Help_Password));

      DoMethod(obj                 , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_User_EditFinish, obj, 0);
      DoMethod(data->BT_Cancel     , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_User_EditFinish, obj, 0);
      DoMethod(data->BT_Okay       , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_User_EditFinish, obj, 1);

      DoMethod(data->STR_Name          , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_RealName);
      DoMethod(data->STR_RealName      , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_EMail);
      DoMethod(data->STR_EMail         , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_MailLogin);
      DoMethod(data->STR_MailLogin     , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_MailPassword);
      DoMethod(data->STR_MailPassword  , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_MailServer);
      DoMethod(data->STR_MailServer     , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Name);
   }
   return((ULONG)obj);
}

///
/// UserWindow_Dispatcher
SAVEDS ULONG UserWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                      : return(UserWindow_New               (cl, obj, (APTR)msg));
      case MUIM_UserWindow_Init        : return(UserWindow_Init              (cl, obj, (APTR)msg));
      case MUIM_UserWindow_CopyData    : return(UserWindow_CopyData          (cl, obj, (APTR)msg));
      case MUIM_Genesis_Finish         : return(UserWindow_PasswdReqFinish  (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

