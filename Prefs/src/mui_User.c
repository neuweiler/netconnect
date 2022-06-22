/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_User.h"
#include "mui_UserWindow.h"
#include "protos.h"

///
/// external variables
extern struct MUI_CustomClass  *CL_MainWindow, *CL_UserWindow;
extern struct Hook objstrhook, des_hook;
extern Object *win, *app;
extern BOOL changed_passwd;

///

/// User_SetStates
ULONG User_SetStates(struct IClass *cl, Object *obj, Msg msg)
{
   struct User_Data *data = INST_DATA(cl, obj);
   struct Prefs_User *p_user;

   changed_passwd = TRUE;

   DoMethod(data->LI_User   , MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &p_user);
   set(data->BT_Delete, MUIA_Disabled, !p_user);
   set(data->BT_Edit  , MUIA_Disabled, !p_user);

   return(NULL);
}

///
/// User_NewUser
ULONG User_NewUser(struct IClass *cl, Object *obj, Msg msg)
{
   struct User_Data *data = INST_DATA(cl, obj);
   struct Prefs_User *p_user, *tmp_user;
   ULONG pos;
   BOOL found;

   DoMethod(data->LI_User, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
   set(data->LI_User, MUIA_List_Active, MUIV_List_Active_Bottom);

   DoMethod(data->LI_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &p_user);
   if(p_user)
   {
      p_user->pu_uid = 100;
      p_user->pu_gid = 100;

      do    // find free user ID
      {
         found = FALSE;
         pos = 0;
         while(!found)
         {
            DoMethod(data->LI_User, MUIM_List_GetEntry, pos++, &tmp_user);
            if(!tmp_user)
               break;
            if((p_user->pu_uid == tmp_user->pu_uid) && (p_user != tmp_user))
               found = TRUE;
         }
         if(found)
            p_user->pu_uid++;
      }  while(found);

      strcpy(p_user->pu_shell, "noshell");
   }
   DoMethod(obj, MUIM_User_Edit);

   return(NULL);
}

///
/// User_Edit
ULONG User_Edit(struct IClass *cl, Object *obj, Msg msg)
{
   struct User_Data *data = INST_DATA(cl, obj);
   struct Prefs_User *p_user;

   DoMethod(data->LI_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &p_user);
   if(p_user)
   {
      Object *window;

      set(app, MUIA_Application_Sleep, TRUE);
      if(window = NewObject(CL_UserWindow->mcc_Class, NULL, MUIA_Genesis_Originator, obj, TAG_DONE))
      {
         DoMethod(app, OM_ADDMEMBER, window);
         DoMethod(window, MUIM_UserWindow_Init, p_user);
         set(window, MUIA_Window_Title, p_user->pu_login);
         set(window, MUIA_Window_Open, TRUE);
      }
   }
   else
      DisplayBeep(NULL);

   return(NULL);
}

///
/// User_EditFinish
ULONG User_EditFinish(struct IClass *cl, Object *obj, struct MUIP_User_EditFinish *msg)
{
   struct User_Data *data = INST_DATA(cl, obj);

   set(msg->win, MUIA_Window_Open, FALSE);
   if(msg->ok)
      DoMethod(msg->win, MUIM_UserWindow_CopyData);
   DoMethod(app, OM_REMMEMBER, msg->win);
   MUI_DisposeObject(msg->win);
   set(app, MUIA_Application_Sleep, FALSE);
   DoMethod(data->LI_User, MUIM_List_Redraw, MUIV_List_Redraw_Active);

   return(NULL);
}

///

/// user_consfunc
SAVEDS ASM APTR user_consfunc(REG(a2) APTR pool, REG(a1) struct User *src)
{
   struct Prefs_User *new;

   if(new = (struct Prefs_User *)AllocVec(sizeof(struct Prefs_User), MEMF_ANY | MEMF_CLEAR))
   {
      if(src && ((LONG)src != -1))
         memcpy(new, src, sizeof(struct Prefs_User));
   }

   return(new);
}
static struct Hook user_conshook = {{NULL, NULL}, (VOID *)user_consfunc, NULL, NULL};

///
/// user_dspfunc
SAVEDS ASM LONG user_dspfunc(REG(a2) char **array, REG(a1) struct Prefs_User *p_user)
{
   if(p_user)
   {
      static char buf1[16], buf2[16];

      sprintf(buf1, "%ld", p_user->pu_uid);
      sprintf(buf2, "%ld", p_user->pu_gid);

      *array++ = p_user->pu_login;
      *array++ = p_user->pu_realname;
      *array++ = buf1;
      *array++ = buf2;
      *array++ = p_user->pu_homedir;
      *array++ = p_user->pu_shell;
      *array   = (((p_user->pu_password[0] == '*') && (p_user->pu_password[1] == NULL)) ? GetStr(MSG_TX_Disabled) : (!*p_user->pu_password ? GetStr(MSG_TX_NoPasswd) : GetStr(MSG_TX_Normal)));
   }
   else
   {
      *array++ = GetStr(MSG_TX_User);
      *array++ = GetStr(MSG_TX_RealName);
      *array++ = GetStr(MSG_TX_UID);
      *array++ = GetStr(MSG_TX_GID);
      *array++ = GetStr(MSG_TX_HomeDir);
      *array++ = GetStr(MSG_TX_Shell);
      *array   = GetStr(MSG_TX_Status);
   }

   return(NULL);
}
static struct Hook user_dsphook = {{NULL, NULL}, (VOID *)user_dspfunc, NULL, NULL};

///

/// User_New
ULONG User_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct User_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      Child, VGroup,
         GroupSpacing(0),
         Child, tmp.LV_User = NListviewObject,
            MUIA_CycleChain         , 1,
            MUIA_NListview_NList      , tmp.LI_User = NListObject,
               InputListFrame,
               MUIA_NList_DragType     , MUIV_NList_DragType_Default,
               MUIA_NList_DragSortable , TRUE,
               MUIA_NList_ConstructHook, &user_conshook,
               MUIA_NList_DestructHook , &des_hook,
               MUIA_NList_DisplayHook  , &user_dsphook,
               MUIA_NList_Format       , "BAR,BAR,BAR,BAR,BAR,BAR,",
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
      struct User_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->BT_Delete  , MUIA_Disabled, TRUE);
      set(data->BT_Edit    , MUIA_Disabled, TRUE);

      set(data->LV_User    , MUIA_ShortHelp, GetStr(MSG_Help_UserList));
      set(data->BT_New     , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_Delete  , MUIA_ShortHelp, GetStr(MSG_Help_Delete));
      set(data->BT_Edit    , MUIA_ShortHelp, GetStr(MSG_Help_Edit));

      DoMethod(data->BT_New    , MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_User_NewUser);
      DoMethod(data->BT_Delete , MUIM_Notify, MUIA_Pressed, FALSE, data->LI_User, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->BT_Edit   , MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_User_Edit);
      DoMethod(data->LI_User   , MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_User_SetStates);
      DoMethod(data->LI_User   , MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 1, MUIM_User_Edit);
   }
   return((ULONG)obj);
}

///
/// User_Dispatcher
SAVEDS ASM ULONG User_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                : return(User_New                      (cl, obj, (APTR)msg));
      case MUIM_User_SetStates   : return(User_SetStates                (cl, obj, (APTR)msg));
      case MUIM_User_NewUser     : return(User_NewUser                  (cl, obj, (APTR)msg));
      case MUIM_User_Edit        : return(User_Edit                     (cl, obj, (APTR)msg));
      case MUIM_User_EditFinish  : return(User_EditFinish               (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

