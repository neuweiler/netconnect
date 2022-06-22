/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_PasswdReq.h"
#include "protos.h"

///
/// external variables
extern Object *win, *app;

///

/// PasswdReq_OldActive
ULONG PasswdReq_OldActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct PasswdReq_Data *data = INST_DATA(cl, obj);
   STRPTR salt;

   data->tries++;

   if(*data->old_password)
      salt = data->old_password;
   else
      salt = "xx";

   if(!UserGroupBase)
      UserGroupBase = OpenLibrary(USERGROUPNAME, 0);

   if(UserGroupBase)
   {
      if(!strcmp(crypt((STRPTR)xget(data->STR_old_pw, MUIA_String_Contents), salt), data->old_password))
      {
         set(data->STR_pw1, MUIA_Disabled, FALSE);
         set(data->STR_pw2, MUIA_Disabled, FALSE);
         set(obj, MUIA_Window_ActiveObject, data->STR_pw1);
      }
      else
      {
         DisplayBeep(NULL);
         nnset(data->STR_old_pw, MUIA_String_Contents, NULL);
         set(obj, MUIA_Window_ActiveObject, data->STR_old_pw);

         if(data->tries > 2)
            DoMethod(app, MUIM_Application_PushMethod, data->originator, 3, MUIM_Genesis_Finish, obj, 0);
      }
      CloseLibrary(UserGroupBase);
      UserGroupBase = NULL;
   }
   else
   {
      MUI_Request(app, obj, NULL, NULL, GetStr(MSG_BT_Okay), "Could not open usergroup.library.\nChanging password not possible at the moment.");
      DoMethod(app, MUIM_Application_PushMethod, data->originator, 3, MUIM_Genesis_Finish, obj, 0);
   }

   return(NULL);
}
///
/// PasswdReq_Pw2Active
ULONG PasswdReq_Pw2Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct PasswdReq_Data *data = INST_DATA(cl, obj);
   STRPTR pw1, pw2;

   pw1 = (STRPTR)xget(data->STR_pw1, MUIA_String_Contents);
   pw2 = (STRPTR)xget(data->STR_pw2, MUIA_String_Contents);

   if(strcmp(pw1, pw2))
   {
      MUI_Request(app, obj, NULL, NULL, "*_Okay", "The passwords you have\nentered are not equal.");
      setstring(data->STR_pw1, NULL);
      setstring(data->STR_pw2, NULL);
      set(data->BT_Okay, MUIA_Disabled, TRUE);
      set(obj, MUIA_Window_ActiveObject, data->STR_pw1);
   }
   else
   {
      if((strlen(pw1) < 8) ||
        (!strchr(pw1, '0') && !strchr(pw1, '1') && !strchr(pw1, '2') && !strchr(pw1, '3') &&
         !strchr(pw1, '4') && !strchr(pw1, '5') && !strchr(pw1, '6') && !strchr(pw1, '7') &&
         !strchr(pw1, '8') && !strchr(pw1, '9') && !strchr(pw1, '-') && !strchr(pw1, '/') &&
         !strchr(pw1, '%') && !strchr(pw1, '&') && !strchr(pw1, '+') && !strchr(pw1, ',') &&
         !strchr(pw1, '.') && !strchr(pw1, ':') && !strchr(pw1, ';') && !strchr(pw1, '$') &&
         !strchr(pw1, '!') && !strchr(pw1, '>') && !strchr(pw1, '<') && !strchr(pw1, '@') &&
         !strchr(pw1, '#') && !strchr(pw1, '*') && !strchr(pw1, '=') && !strchr(pw1, '(') &&
         !strchr(pw1, ')') && !strchr(pw1, '\"')))
      {
         MUI_Request(app, obj, NULL, NULL, "*_Okay", "The new password must be at least 8 characters long and\ncontain at least one digit or one of the following characters: \n\033c- / % & + , . ; : $ ! < > ( ) @ # * \" \033n");
         setstring(data->STR_pw1, NULL);
         setstring(data->STR_pw2, NULL);
         set(data->BT_Okay, MUIA_Disabled, TRUE);
         set(obj, MUIA_Window_ActiveObject, data->STR_pw1);
      }
      else
      {
         set(data->BT_Okay, MUIA_Disabled, FALSE);
         set(obj, MUIA_Window_ActiveObject, data->BT_Okay);
      }
   }
   return(NULL);
}
///

/// PasswdReq_New
ULONG PasswdReq_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct PasswdReq_Data tmp;

   tmp.originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , "Change password",
      MUIA_Window_ID       , MAKE_ID('P','A','S','S'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      MUIA_Window_Width    , MUIV_Window_Width_MinMax(10),
      WindowContents       , VGroup,
         Child, VGroup,
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, tmp.GR_old_pw = VGroup,
               Child, ColGroup(2),
                  Child, MakeKeyLabel2("  Old password:", "  o"),
                  Child, tmp.STR_old_pw = TextinputObject,
                     MUIA_ControlChar        , *GetStr("  o"),
                     MUIA_CycleChain         , 1,
                     MUIA_Frame              , MUIV_Frame_String,
                     MUIA_Textinput_Secret   , TRUE,
                     MUIA_Textinput_Multiline, FALSE,
                     MUIA_Textinput_MaxLen   , 80,
                  End,
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_HBar, 2),
               Child, HVSpace,
            End,
            Child, ColGroup(2),
               Child, MakeKeyLabel2("  New password:", "  p"),
               Child, tmp.STR_pw1 = TextinputObject,
                  MUIA_ControlChar        , *GetStr("  p"),
                  MUIA_CycleChain         , 1,
                  MUIA_Frame              , MUIV_Frame_String,
                  MUIA_Textinput_Secret   , TRUE,
                  MUIA_Textinput_Multiline, FALSE,
                  MUIA_Textinput_MaxLen   , 40,
               End,
               Child, MakeKeyLabel2("  Verify password:", "  v"),
               Child, tmp.STR_pw2 = TextinputObject,
                  MUIA_ControlChar        , *GetStr("  v"),
                  MUIA_CycleChain         , 1,
                  MUIA_Frame              , MUIV_Frame_String,
                  MUIA_Textinput_Secret   , TRUE,
                  MUIA_Textinput_Multiline, FALSE,
                  MUIA_Textinput_MaxLen   , 40,
               End,
            End,
         End,
         Child, MUI_MakeObject(MUIO_HBar, 2),
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton("  _Okay"),
            Child, tmp.BT_Cancel = MakeButton("  _Cancel"),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct PasswdReq_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->old_password = (STRPTR)GetTagData(MUIA_PasswdReq_OldPassword, 0, msg->ops_AttrList);
      if(!data->old_password || !*data->old_password)
      {
         set(data->GR_old_pw, MUIA_ShowMe, FALSE);
         set(obj, MUIA_Window_ActiveObject, data->STR_pw1);
      }
      else
      {
         set(obj, MUIA_Window_ActiveObject, data->STR_old_pw);
         set(data->STR_pw1, MUIA_Disabled, TRUE);
         set(data->STR_pw2, MUIA_Disabled, TRUE);
      }
      set(data->BT_Okay, MUIA_Disabled, TRUE);

      data->tries = 0;

      DoMethod(data->STR_old_pw , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj, 1, MUIM_PasswdReq_OldActive);
      DoMethod(obj              , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod, data->originator   , 3, MUIM_Genesis_Finish, obj, 0);
      DoMethod(data->BT_Cancel  , MUIM_Notify, MUIA_Pressed            , FALSE           ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod, data->originator   , 3, MUIM_Genesis_Finish, obj, 0);
      DoMethod(data->BT_Okay    , MUIM_Notify, MUIA_Pressed            , FALSE           ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod, data->originator   , 3, MUIM_Genesis_Finish, obj, 1);
      DoMethod(data->STR_pw1    , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_pw2);
      DoMethod(data->STR_pw2    , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj, 1, MUIM_PasswdReq_Pw2Active);
   }
   return((ULONG)obj);
}

///
/// PasswdReq_Dispatcher
SAVEDS ULONG PasswdReq_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                   : return(PasswdReq_New           (cl, obj, (APTR)msg));
      case MUIM_PasswdReq_OldActive : return(PasswdReq_OldActive     (cl, obj, (APTR)msg));
      case MUIM_PasswdReq_Pw2Active : return(PasswdReq_Pw2Active     (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

