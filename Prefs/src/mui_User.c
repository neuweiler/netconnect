/// includes
#include "/includes.h"
#pragma header

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
   struct User *user;

   changed_passwd = TRUE;

   DoMethod(data->LI_User   , MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
   set(data->BT_Delete, MUIA_Disabled, !user);
   set(data->BT_Edit  , MUIA_Disabled, !user);

   return(NULL);
}

///
/// User_NewUser
ULONG User_NewUser(struct IClass *cl, Object *obj, Msg msg)
{
   struct User_Data *data = INST_DATA(cl, obj);
   struct User *user, *tmp_user;
   ULONG pos;
   BOOL found;

   DoMethod(data->LI_User, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
   set(data->LI_User, MUIA_List_Active, MUIV_List_Active_Bottom);

   DoMethod(data->LI_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
   if(user)
   {
      user->us_uid = 100;
      user->us_gid = 100;

      do    // find free user ID
      {
         found = FALSE;
         pos = 0;
         while(!found)
         {
            DoMethod(data->LI_User, MUIM_List_GetEntry, pos++, &tmp_user);
            if(!tmp_user)
               break;
            if((user->us_uid == tmp_user->us_uid) && (user != tmp_user))
               found = TRUE;
         }
         if(found)
            user->us_uid++;
      }  while(found);

      strcpy(user->us_shell, "noshell");
   }
   DoMethod(obj, MUIM_User_Edit);

   return(NULL);
}

///
/// User_Edit
ULONG User_Edit(struct IClass *cl, Object *obj, Msg msg)
{
   struct User_Data *data = INST_DATA(cl, obj);
   struct User *user;

   DoMethod(data->LI_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
   if(user)
   {
      Object *window;

      set(app, MUIA_Application_Sleep, TRUE);
      if(window = NewObject(CL_UserWindow->mcc_Class, NULL, MUIA_Genesis_Originator, obj, TAG_DONE))
      {
         DoMethod(app, OM_ADDMEMBER, window);
         DoMethod(window, MUIM_UserWindow_Init, user);
         set(window, MUIA_Window_Title, user->us_login);
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
   struct User *new;

   if(new = (struct User *)AllocVec(sizeof(struct User), MEMF_ANY | MEMF_CLEAR))
   {
      if(src && ((LONG)src != -1))
         memcpy(new, src, sizeof(struct User));
   }

   return(new);
}
static struct Hook user_conshook = {{NULL, NULL}, (VOID *)user_consfunc, NULL, NULL};

///
/// user_dspfunc
SAVEDS ASM LONG user_dspfunc(REG(a2) char **array, REG(a1) struct User *user)
{
   if(user)
   {
      static char buf1[16], buf2[16];

      sprintf(buf1, "%ld", user->us_uid);
      sprintf(buf2, "%ld", user->us_gid);

      *array++ = user->us_login;
      *array++ = user->us_realname;
      *array++ = buf1;
      *array++ = buf2;
      *array++ = user->us_homedir;
      *array++ = user->us_shell;
      *array   = (((user->us_password[0] == '*') && (user->us_password[1] == NULL)) ? "disabled" : (!*user->us_password ? "\033bno password\033n" : "normal"));
   }
   else
   {
      *array++ = "\033bUser";
      *array++ = "\033bReal Name";
      *array++ = "\033bUID";
      *array++ = "\033bGID";
      *array++ = "\033bHome Dir";
      *array++ = "\033bShell";
      *array   = "\033bStatus";
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
            Child, tmp.BT_New    = MakeButton("  Ne_w"),
            Child, tmp.BT_Delete = MakeButton("  _Delete"),
            Child, tmp.BT_Edit   = MakeButton("  _Edit"),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct User_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->BT_Delete        , MUIA_Disabled, TRUE);
      set(data->BT_Edit          , MUIA_Disabled, TRUE);

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
SAVEDS ULONG User_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
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

