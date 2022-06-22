/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_UserWindow.h"
#include "mui_User.h"
#include "mui_PasswdReq.h"
#include "protos.h"

///
/// external variables
extern struct Library *GenesisBase;
extern struct Hook strobjhook, des_hook;
extern struct MUI_CustomClass *CL_PasswdReq;
extern Object *win, *app;

///

/// UserWindow_DisableActive
ULONG UserWindow_DisableActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);
   BOOL disabled;

   disabled = xget(data->CH_Disabled, MUIA_Selected);
   set(data->BT_ChangePassword, MUIA_Disabled, disabled);
   set(data->BT_RemovePassword, MUIA_Disabled, !*data->p_user->pu_password || disabled);

   return(NULL);
}

///
/// UserWindow_ChangePassword
ULONG UserWindow_ChangePassword(struct IClass *cl, Object *obj, Msg msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);
   Object *window;

   set(app, MUIA_Application_Sleep, TRUE);
   if(window = NewObject(CL_PasswdReq->mcc_Class, NULL,
      MUIA_Genesis_Originator, obj,
      MUIA_PasswdReq_OldPassword, data->password,
      TAG_DONE))
   {
      DoMethod(app, OM_ADDMEMBER, window);
      set(window, MUIA_Window_Open, TRUE);
   }

   return(NULL);
}

///
/// UserWindow_PasswdReqFinish
ULONG UserWindow_PasswdReqFinish(struct IClass *cl, Object *obj, struct MUIP_Genesis_Finish *msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);

   if(msg->status == 1)
   {
      struct PasswdReq_Data *pw_data = INST_DATA(CL_PasswdReq->mcc_Class, msg->obj);

      if(strlen((STRPTR)xget(pw_data->STR_pw1, MUIA_String_Contents)))
      {
         if(!UserGroupBase)
            UserGroupBase = OpenLibrary(USERGROUPNAME, 0);

         if(UserGroupBase)
         {
            strcpy(data->password, crypt((STRPTR)xget(pw_data->STR_pw1, MUIA_String_Contents), data->password));
            CloseLibrary(UserGroupBase);
            UserGroupBase = NULL;
         }
      }
      else
         MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CantChangePasswd));
   }
   set(msg->obj, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, msg->obj),
   MUI_DisposeObject(msg->obj);
   set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///
/// UserWindow_RemovePassword
ULONG UserWindow_RemovePassword(struct IClass *cl, Object *obj, Msg msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);

   if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_RemoveCancel), GetStr(MSG_TX_ReallyRemovePasswd)))
   {
      MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_PasswordWarning));
      *data->p_user->pu_password = NULL;
      set(data->BT_RemovePassword, MUIA_Disabled, TRUE);
   }
   return(NULL);
}

///
/// UserWindow_NameActive
ULONG UserWindow_NameActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);

   if(!strlen((STRPTR)xget(data->STR_HomeDir, MUIA_String_Contents)))
   {
      char buf[81];

      sprintf(buf, "AmiTCP:home/%ls", (STRPTR)xget(data->STR_Name, MUIA_String_Contents));
      setstring(data->STR_HomeDir, buf);
      DoMethod(obj, MUIM_UserWindow_HomeDirActive);
   }
   set(win, MUIA_Window_ActiveObject, data->STR_RealName);

   return(NULL);
}

///
/// UserWindow_HomeDirActive
ULONG UserWindow_HomeDirActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);
   STRPTR file;
   BPTR fh;

   file = (STRPTR)xget(data->STR_HomeDir, MUIA_String_Contents);
   if(fh = Lock(file, ACCESS_READ))
      UnLock(fh);
   else
   {
      if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_CreateCancel), GetStr(MSG_TX_CreateDir), file))
      {
         if(fh = CreateDir(file))
            UnLock(fh);
         else
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotCreateX), file);
      }
   }
   set(win, MUIA_Window_ActiveObject, data->STR_Shell);

   return(NULL);
}

///

/// UserWindow_Init
ULONG UserWindow_Init(struct IClass *cl, Object *obj, struct MUIP_UserWindow_Init *msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);

   data->p_user = msg->p_user;

   setstring(data->STR_Name         , data->p_user->pu_login);
   setstring(data->STR_RealName     , data->p_user->pu_realname);
   setcheckmark(data->CH_Disabled   , !strcmp(data->p_user->pu_password, "*"));
   setstring(data->STR_HomeDir      , data->p_user->pu_homedir);
   setstring(data->STR_Shell        , data->p_user->pu_shell);
   set(data->STR_UserID             , MUIA_String_Integer, data->p_user->pu_uid);
   set(data->STR_GroupID            , MUIA_String_Integer, data->p_user->pu_gid);
   if(!*data->p_user->pu_password)
      set(data->BT_RemovePassword, MUIA_Disabled, TRUE);

   strcpy(data->password, data->p_user->pu_password);

   return(NULL);
}
///
/// UserWindow_CopyData
ULONG UserWindow_CopyData(struct IClass *cl, Object *obj, Msg msg)
{
   struct UserWindow_Data *data = INST_DATA(cl, obj);

   if(strcmp(data->p_user->pu_login , (STRPTR)xget(data->STR_Name, MUIA_String_Contents)) ||
      strcmp(data->p_user->pu_homedir, (STRPTR)xget(data->STR_HomeDir, MUIA_String_Contents)) ||
      (xget(data->CH_Disabled, MUIA_Selected) && strcmp(data->p_user->pu_password, "*")) ||
      (!xget(data->CH_Disabled, MUIA_Selected) && !strcmp(data->p_user->pu_password, "*") && strcmp(data->p_user->pu_login, "root")) ||
      (data->p_user->pu_uid != xget(data->STR_UserID, MUIA_String_Integer)) ||
      (data->p_user->pu_gid != xget(data->STR_GroupID, MUIA_String_Integer)))
   {
      struct User *user;

      if(user = GetUser("root", GetStr(MSG_TX_ModifyUserRootAccess), GUF_TextObject))
      {
         FreeUser(user);
         if(strcmp(data->p_user->pu_login, "root"))
            strcpy(data->p_user->pu_login         , (STRPTR)xget(data->STR_Name         , MUIA_String_Contents));
         strcpy(data->p_user->pu_homedir       , (STRPTR)xget(data->STR_HomeDir      , MUIA_String_Contents));
         if(strcmp(data->p_user->pu_login, "root"))
            data->p_user->pu_uid = xget(data->STR_UserID  , MUIA_String_Integer);
         else
            data->p_user->pu_uid = 0;
         data->p_user->pu_gid = xget(data->STR_GroupID , MUIA_String_Integer);
         if(xget(data->CH_Disabled, MUIA_Selected) && strcmp(data->p_user->pu_login, "root"))
            strcpy(data->p_user->pu_password, "*");
         else
            strcpy(data->p_user->pu_password, data->password);
      }
      else
         MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_NotAllowedModifyUser));
   }
   else
      strcpy(data->p_user->pu_password, data->password);

   strcpy(data->p_user->pu_realname      , (STRPTR)xget(data->STR_RealName     , MUIA_String_Contents));
   strcpy(data->p_user->pu_shell         , (STRPTR)xget(data->STR_Shell        , MUIA_String_Contents));

   return(NULL);
}
///

/// UserWindow_New
ULONG UserWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct UserWindow_Data tmp;
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_TX_EditUser),
      MUIA_Window_ID       , MAKE_ID('U','S','R','E'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      WindowContents       , VGroup,
         Child, ColGroup(2),
            GroupFrame,
            Child, MakeKeyLabel2(MSG_LA_UserName, MSG_CC_UserName),
            Child, tmp.STR_Name = TextinputObject,
               StringFrame,
               MUIA_ControlChar     , *GetStr(MSG_CC_UserName),
               MUIA_CycleChain      , 1,
               MUIA_String_MaxLen   , 40,
               MUIA_String_Reject   , "|-.",
            End,
            Child, MakeKeyLabel2(MSG_LA_RealName, MSG_CC_RealName),
            Child, tmp.STR_RealName = MakeKeyString(NULL, 80, MSG_CC_RealName),
            Child, MakeKeyLabel2(MSG_LA_HomeDir, MSG_CC_HomeDir),
            Child, tmp.PA_HomeDir   = MakePopAsl(tmp.STR_HomeDir = MakeKeyString(NULL, MAXPATHLEN, MSG_CC_HomeDir), GetStr(MSG_TX_ChooseHomeDir), TRUE),
            Child, MakeKeyLabel2(MSG_LA_Shell, MSG_CC_Shell),
            Child, HGroup,
               Child, tmp.STR_Shell = MakeKeyString(NULL, 80, MSG_CC_Shell),
               Child, MakeKeyLabel2(MSG_LA_DisableUser, MSG_CC_DisableUser),
               Child, tmp.CH_Disabled  = MakeKeyCheckMark(FALSE, MSG_CC_DisableUser),
            End,
            Child, MakeKeyLabel2(MSG_LA_UserID, MSG_CC_UserID),
            Child, HGroup,
               Child, tmp.STR_UserID   = TextinputObject,
                  StringFrame,
                  MUIA_ControlChar     , *GetStr(MSG_CC_UserID),
                  MUIA_CycleChain      , 1,
                  MUIA_String_MaxLen   , 7,
                  MUIA_String_Accept   , "1234567890-",
               End,
               Child, tmp.BT_ChangePassword = MakeButton(MSG_BT_ChangePassword),
            End,
            Child, MakeKeyLabel2(MSG_LA_UserGID, MSG_CC_UserGID),
            Child, HGroup,
               Child, tmp.STR_GroupID        = TextinputObject,
                  StringFrame,
                  MUIA_ControlChar     , *GetStr(MSG_CC_UserGID),
                  MUIA_CycleChain      , 1,
                  MUIA_String_MaxLen   , 7,
                  MUIA_String_Accept   , "1234567890-",
               End,
               Child, tmp.BT_RemovePassword  = MakeButton(MSG_BT_RemovePassword),
            End,
         End,
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton(MSG_BT_Okay),
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct UserWindow_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(data->STR_Name         , MUIA_ShortHelp, GetStr(MSG_Help_UserName));
      set(data->STR_RealName     , MUIA_ShortHelp, GetStr(MSG_Help_RealName));
      set(data->STR_HomeDir      , MUIA_ShortHelp, GetStr(MSG_Help_HomeDir));
      set(data->STR_Shell        , MUIA_ShortHelp, GetStr(MSG_Help_Shell));
      set(data->CH_Disabled      , MUIA_ShortHelp, GetStr(MSG_Help_DisableUser));
      set(data->STR_UserID       , MUIA_ShortHelp, GetStr(MSG_Help_UserID));
      set(data->BT_ChangePassword, MUIA_ShortHelp, GetStr(MSG_Help_ChangePassword));
      set(data->BT_RemovePassword, MUIA_ShortHelp, GetStr(MSG_Help_RemovePassword));
      set(data->STR_GroupID      , MUIA_ShortHelp, GetStr(MSG_Help_GroupID));

      DoMethod(obj                 , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_User_EditFinish, obj, 0);
      DoMethod(data->BT_Cancel     , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_User_EditFinish, obj, 0);
      DoMethod(data->BT_Okay       , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_User_EditFinish, obj, 1);

      DoMethod(data->CH_Disabled       , MUIM_Notify, MUIA_Selected       , MUIV_EveryTime  , obj, 1, MUIM_UserWindow_DisableActive);
      DoMethod(data->BT_ChangePassword , MUIM_Notify, MUIA_Pressed        , FALSE           , obj, 1, MUIM_UserWindow_ChangePassword);
      DoMethod(data->BT_RemovePassword , MUIM_Notify, MUIA_Pressed        , FALSE           , obj, 1, MUIM_UserWindow_RemovePassword);

      DoMethod(data->STR_Name          , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_UserWindow_NameActive);
      DoMethod(data->STR_HomeDir       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_UserWindow_HomeDirActive);
      DoMethod(data->STR_RealName      , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_HomeDir);
      DoMethod(data->STR_Shell         , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_UserID);
      DoMethod(data->STR_UserID        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_GroupID);
      DoMethod(data->STR_GroupID       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->BT_Okay);
   }
   return((ULONG)obj);
}

///
/// UserWindow_Dispatcher
SAVEDS ASM ULONG UserWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                         : return(UserWindow_New               (cl, obj, (APTR)msg));
      case MUIM_UserWindow_Init           : return(UserWindow_Init              (cl, obj, (APTR)msg));
      case MUIM_UserWindow_CopyData       : return(UserWindow_CopyData          (cl, obj, (APTR)msg));
      case MUIM_Genesis_Finish            : return(UserWindow_PasswdReqFinish   (cl, obj, (APTR)msg));
      case MUIM_UserWindow_DisableActive  : return(UserWindow_DisableActive     (cl, obj, (APTR)msg));
      case MUIM_UserWindow_ChangePassword : return(UserWindow_ChangePassword    (cl, obj, (APTR)msg));
      case MUIM_UserWindow_RemovePassword : return(UserWindow_RemovePassword    (cl, obj, (APTR)msg));
      case MUIM_UserWindow_NameActive     : return(UserWindow_NameActive        (cl, obj, (APTR)msg));
      case MUIM_UserWindow_HomeDirActive  : return(UserWindow_HomeDirActive     (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

