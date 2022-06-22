/// includes
#include "/includes.h"

#include "/NetConnect.h"
#include "/locale/Strings.h"
#include "mui.h"
#include "mui_MenuPrefs.h"
#include "mui_MainWindow.h"
#include "mui_ProgramList.h"
#include "protos.h"

///
/// external variables
extern struct Hook ProgramList_ConstructHook, ProgramList_DestructHook;
extern Object *app, *win;
extern struct MUI_CustomClass *CL_MainWindow, *CL_MenuPrefs, *CL_ProgramList;

///

SAVEDS ASM struct MenuEntry *MenuList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct MenuEntry *src)
{
   struct MenuEntry *new;

   if(new = (struct MenuEntry *)AllocVec(sizeof(struct MenuEntry), MEMF_ANY | MEMF_CLEAR))
   {
      if(src)
         memcpy(new, src, sizeof(struct MenuEntry));

      if(!(new->LI_Programs = ListObject,
         MUIA_List_ConstructHook , &ProgramList_ConstructHook,
         MUIA_List_DestructHook  , &ProgramList_DestructHook,
         End))
      {
         FreeVec(new);
         new = NULL;
      }
   }
   return(new);
}
struct Hook MenuList_ConstructHook = { { 0,0 }, (VOID *)MenuList_ConstructFunc  , NULL, NULL };

SAVEDS ASM VOID MenuList_DestructFunc(REG(a2) APTR pool, REG(a1) struct MenuEntry *menu)
{
   if(menu)
   {
      if(menu->Name)
         FreeVec(menu->Name);
      if(menu->LI_Programs)
         MUI_DisposeObject(menu->LI_Programs);
      FreeVec(menu);
   }
}
struct Hook MenuList_DestructHook  = { { 0,0 }, (VOID *)MenuList_DestructFunc   , NULL, NULL };

SAVEDS ASM LONG MenuList_DisplayFunc(REG(a0) struct Hook *hook, REG(a2) char **array, REG(a1) struct MenuEntry *menu)
{
   if(menu)
   {
      *array   = (menu->Name ? menu->Name : (STRPTR)"");
   }
   return(NULL);
}
struct Hook MenuList_DisplayHook   = { { 0,0 }, (VOID *)MenuList_DisplayFunc    , NULL, NULL };

ULONG MenuPrefs_NewEntry(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);
   struct MenuEntry menu;

   bzero(&menu, sizeof(struct MenuEntry));
   switch(MUI_Request(app, win, 0, 0, GetStr(MSG_BT_Menu_BarLabel), GetStr(MSG_TX_CreateWhat)))
   {
      case 0:
         menu.Name = update_string(NULL, "~~~~~~~~~~");
         break;
      case 1:
         menu.Name = update_string(NULL, GetStr(MSG_TX_NewMenu));
         break;
   }
   DoMethod(data->LI_Menus, MUIM_List_InsertSingle, &menu, MUIV_List_Insert_Bottom);
   set(data->LI_Menus, MUIA_List_Active, MUIV_List_Active_Bottom);
   set(win, MUIA_Window_ActiveObject, data->STR_Name);

   return(NULL);
}

SAVEDS ASM LONG MenuList_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
   struct MainWindow_Data *main_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct MenuPrefs_Data *data = INST_DATA(CL_MenuPrefs->mcc_Class, main_data->GR_Menus);
   struct WBArg *ap;
   struct AppMessage *amsg = *x;
   struct MenuEntry menu;
   struct Program program;
   STRPTR buf;

   bzero(&menu, sizeof(struct MenuEntry));
   bzero(&program, sizeof(struct Program));
   ap = amsg->am_ArgList;
   if(amsg->am_NumArgs)
   {
      if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
      {
         NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
         AddPart(buf, ap->wa_Name, MAXPATHLEN);

         menu.Name = update_string(NULL, ap->wa_Name);
         DoMethod(obj, MUIM_List_InsertSingle, &menu, MUIV_List_Insert_Bottom);
         set(obj, MUIA_List_Active, MUIV_List_Active_Bottom);

         program.File = update_string(NULL, buf);
         program.Flags = PRG_WORKBENCH | PRG_Asynch;
         DoMethod(data->LI_Programs, MUIM_List_InsertSingle, &program, MUIV_List_Insert_Bottom);
         set(data->LI_Programs, MUIA_List_Active, MUIV_List_Active_Bottom);
         DoMethod(main_data->GR_Menus, MUIM_MenuPrefs_GetProgramList);

         FreeVec(buf);
      }
   }
   return(NULL);
}
struct Hook MenuList_AppMsgHook = { {NULL, NULL}, (VOID *)MenuList_AppMsgFunc, NULL, NULL };

SAVEDS ASM LONG ProgramList_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
   struct MainWindow_Data *main_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct WBArg *ap;
   struct AppMessage *amsg = *x;
   struct Program program;
   STRPTR buf;

   bzero(&program, sizeof(struct Program));
   ap = amsg->am_ArgList;
   if(amsg->am_NumArgs)
   {
      if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
      {
         NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
         AddPart(buf, ap->wa_Name, MAXPATHLEN);

         program.File = update_string(NULL, buf);
         program.Flags = PRG_WORKBENCH | PRG_Asynch;
         DoMethod(obj, MUIM_List_InsertSingle, &program, MUIV_List_Insert_Bottom);
         set(obj, MUIA_List_Active, MUIV_List_Active_Bottom);
         DoMethod(main_data->GR_Menus, MUIM_MenuPrefs_GetProgramList);

         FreeVec(buf);
      }
   }
   return(NULL);
}
struct Hook ProgramList_AppMsgHook = { {NULL, NULL}, (VOID *)ProgramList_AppMsgFunc, NULL, NULL };

ULONG MenuPrefs_MenuList_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);
   struct MenuEntry *menu;
   struct Program *program, *program2;
   int i;

   DoMethod(data->LI_Programs, MUIM_List_Clear);
   DoMethod(data->LI_Menus, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &menu);
   if(menu && strcmp((menu->Name ? menu->Name : (STRPTR)""), "~~~~~~~~~~"))
   {
      set(data->STR_Name      , MUIA_Disabled, FALSE);
      set(data->BT_Delete     , MUIA_Disabled, FALSE);
      set(data->LV_Programs   , MUIA_Disabled, FALSE);
      set(data->BT_NewProgram , MUIA_Disabled, FALSE);
      setstring(data->STR_Name, (menu->Name ? menu->Name : (STRPTR)""));

      i = 0;
      FOREVER
      {
         DoMethod(menu->LI_Programs, MUIM_List_GetEntry, i++, &program);
         if(!program)
            break;
         DoMethod(data->LI_Programs, MUIM_List_InsertSingle, program, MUIV_List_Insert_Bottom);
         DoMethod(data->LI_Programs, MUIM_List_GetEntry, xget(data->LI_Programs, MUIA_List_Entries) - 1, &program2);
         if(program2)
         {
            program2->File          = update_string(NULL, program->File);
            program2->CurrentDir    = update_string(NULL, program->CurrentDir);
            program2->OutputFile    = update_string(NULL, program->OutputFile);
            program2->PublicScreen  = update_string(NULL, program->PublicScreen);
         }
      }
   }
   else
   {
      set(data->STR_Name      , MUIA_Disabled, TRUE);
      set(data->BT_Delete     , MUIA_Disabled, !(menu));
      set(data->LV_Programs   , MUIA_Disabled, TRUE);
      set(data->BT_NewProgram , MUIA_Disabled, TRUE);
      setstring(data->STR_Name, (menu ? menu->Name : (STRPTR)""));
   }

   return(NULL);
}

ULONG MenuPrefs_GetProgramList(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);
   struct MenuEntry *menu;
   struct Program *program, *program2;
   int i;

   DoMethod(data->LI_Menus, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &menu);
   if(menu)
   {
      DoMethod(menu->LI_Programs, MUIM_List_Clear);
      i = 0;
      FOREVER
      {
         DoMethod(data->LI_Programs, MUIM_List_GetEntry, i++, &program);
         if(!program)
            break;
         DoMethod(menu->LI_Programs, MUIM_List_InsertSingle, program, MUIV_List_Insert_Bottom);
         DoMethod(menu->LI_Programs, MUIM_List_GetEntry, xget(menu->LI_Programs, MUIA_List_Entries) - 1, &program2);
         if(program2)
         {
            program2->File          = update_string(NULL, program->File);
            program2->CurrentDir    = update_string(NULL, program->CurrentDir);
            program2->OutputFile    = update_string(NULL, program->OutputFile);
            program2->PublicScreen  = update_string(NULL, program->PublicScreen);
         }
      }
   }
   return(NULL);
}

ULONG MenuPrefs_MenuList_ChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);
   struct MenuEntry *menu;

   DoMethod(data->LI_Menus, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &menu);
   if(menu)
   {
      menu->Name = update_string(menu->Name, (STRPTR)xget(data->STR_Name, MUIA_String_Contents));
      DoMethod(data->LI_Menus, MUIM_List_Redraw, MUIV_List_Redraw_Active);
      set((Object *)xget(data->STR_Name, MUIA_WindowObject), MUIA_Window_ActiveObject, data->STR_Name);
   }

   return(NULL);
}

ULONG MenuPrefs_NewProgram(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);

   DoMethod(data->LI_Programs, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
   set(data->LI_Programs, MUIA_List_Active, MUIV_List_Active_Bottom);
   set(win, MUIA_Window_ActiveObject, data->PA_Program);

   DoMethod(obj, MUIM_MenuPrefs_GetProgramList);

   return(NULL);
}

ULONG MenuPrefs_ProgramList_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);
   struct Program *program;

   DoMethod(data->LI_Programs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &program);
   if(program)
   {
      set(data->PA_Program       , MUIA_Disabled, FALSE);
      set(data->BT_DeleteProgram , MUIA_Disabled, FALSE);
      set(data->CY_Asynch        , MUIA_Disabled, (program->Flags & PRG_WORKBENCH));
      set(data->CY_Type          , MUIA_Disabled, FALSE);

      setstring(data->PA_Program , program->File);
      setcycle(data->CY_Asynch   , program->Flags & PRG_Asynch);
      setcycle(data->CY_Type     , (program->Flags & PRG_AREXX ? 3 : (program->Flags & PRG_SCRIPT ? 2 : (program->Flags & PRG_WORKBENCH ? 1 : 0))));
   }
   else
   {
      set(data->PA_Program       , MUIA_Disabled, TRUE);
      set(data->BT_DeleteProgram , MUIA_Disabled, TRUE);
      set(data->CY_Asynch        , MUIA_Disabled, TRUE);
      set(data->CY_Type          , MUIA_Disabled, TRUE);

      setstring(data->PA_Program , "");
   }
   return(NULL);
}

ULONG MenuPrefs_ProgramList_ChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);
   struct Program *program;

   DoMethod(data->LI_Programs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &program);
   if(program)
   {
      program->File = update_string(program->File, (STRPTR)xget(data->PA_Program, MUIA_String_Contents));
      DoMethod(data->LI_Programs, MUIM_List_Redraw, MUIV_List_Redraw_Active);
      set((Object *)xget(data->PA_Program, MUIA_WindowObject), MUIA_Window_ActiveObject, data->PA_Program);

      DoMethod(obj, MUIM_MenuPrefs_GetProgramList);
   }
   return(NULL);
}

ULONG MenuPrefs_Asynch_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);
   struct Program *program;

   DoMethod(data->LI_Programs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &program);
   if(program)
   {
      if(xget(data->CY_Asynch, MUIA_Cycle_Active))
         program->Flags |= PRG_Asynch;
      else
         program->Flags &= ~PRG_Asynch;
      DoMethod(obj, MUIM_MenuPrefs_GetProgramList);
   }
   return(NULL);
}

ULONG MenuPrefs_Type_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct MenuPrefs_Data *data = INST_DATA(cl, obj);
   struct Program *program;

   DoMethod(data->LI_Programs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &program);
   if(program)
   {
      program->Flags &= ~(PRG_CLI | PRG_WORKBENCH | PRG_SCRIPT | PRG_AREXX);
      switch(xget(data->CY_Type, MUIA_Cycle_Active))
      {
         case 1:
            program->Flags |= PRG_WORKBENCH;
            setcycle(data->CY_Asynch, 1);
            break;
         case 2:
            program->Flags |= PRG_SCRIPT;
            break;
         case 3:
            program->Flags |= PRG_AREXX;
            break;
         default:
            program->Flags |= PRG_CLI;
            break;
      }
      set(data->CY_Asynch, MUIA_Disabled, (program->Flags & PRG_WORKBENCH));
      DoMethod(obj, MUIM_MenuPrefs_GetProgramList);
   }
   return(NULL);
}

ULONG MenuPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static STRPTR CY_Asynch[3];
   static STRPTR CY_ProgramTypes[5];
   struct MenuPrefs_Data tmp;

   CY_Asynch[0] = GetStr(MSG_CY_Synchron);
   CY_Asynch[1] = GetStr(MSG_CY_Asynchron);
   CY_Asynch[2] = NULL;

   CY_ProgramTypes[0] = GetStr(MSG_CY_CLI);
   CY_ProgramTypes[1] = GetStr(MSG_CY_Workbench);
   CY_ProgramTypes[2] = GetStr(MSG_CY_Script);
   CY_ProgramTypes[3] = GetStr(MSG_CY_ARexxScript);
   CY_ProgramTypes[4] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      Child, HGroup,
         Child, VGroup,
            MUIA_Group_Spacing, 0,
            Child, tmp.LV_Menus = ListviewObject,
               MUIA_FrameTitle, GetStr(MSG_LA_MenuEntries),
               MUIA_CycleChain         , 1,
               MUIA_Listview_Input     , TRUE,
               MUIA_Listview_DragType  , 1,
               MUIA_Listview_List      , tmp.LI_Menus = ListObject,
                  InputListFrame,
                  MUIA_List_ConstructHook , &MenuList_ConstructHook,
                  MUIA_List_DestructHook  , &MenuList_DestructHook,
                  MUIA_List_DisplayHook   , &MenuList_DisplayHook,
                  MUIA_List_DragSortable  , TRUE,
               End,
            End,
            Child, HGroup,
               MUIA_Group_Spacing, 0,
               Child, tmp.BT_New = MakeButton(MSG_BT_New2),
               Child, tmp.BT_Delete = MakeButton(MSG_BT_Delete1),
            End,
            Child, tmp.STR_Name = String("", 81),
         End,
         Child, VGroup,
            MUIA_Group_Spacing, 0,
            Child, tmp.LV_Programs = ListviewObject,
               MUIA_CycleChain,  1,
               MUIA_FrameTitle, GetStr(MSG_LA_Programs),
               MUIA_Listview_DragType     , 1,
               MUIA_Listview_List         , tmp.LI_Programs = NewObject(CL_ProgramList->mcc_Class, NULL, TAG_DONE),
            End,
            Child, HGroup,
               MUIA_Group_Spacing, 0,
               Child, tmp.BT_NewProgram = MakeButton(MSG_BT_New3),
               Child, tmp.BT_DeleteProgram = MakeButton(MSG_BT_Delete2),
               Child, tmp.CY_Asynch = Cycle(CY_Asynch),
            End,
            Child, tmp.CY_Type = Cycle(CY_ProgramTypes),
            Child, tmp.PA_Program = MakePopAsl(tmp.STR_Program = String("", MAXPATHLEN), MSG_LA_Program, FALSE),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct MenuPrefs_Data *data = INST_DATA(cl, obj);
      struct ProgramList_Data *cl_data = INST_DATA(CL_ProgramList->mcc_Class, tmp.LI_Programs);

      *data = tmp;

      cl_data->Originator = obj;

      set(tmp.BT_Delete       , MUIA_Disabled, TRUE);
      set(tmp.STR_Name        , MUIA_Disabled, TRUE);
      set(tmp.LV_Programs     , MUIA_Disabled, TRUE);
      set(tmp.BT_NewProgram   , MUIA_Disabled, TRUE);
      set(tmp.BT_DeleteProgram, MUIA_Disabled, TRUE);
      set(tmp.CY_Asynch       , MUIA_Disabled, TRUE);
      set(tmp.CY_Type         , MUIA_Disabled, TRUE);
      set(tmp.PA_Program      , MUIA_Disabled, TRUE);
      set(tmp.STR_Name        , MUIA_String_AttachedList, tmp.LV_Menus);
      set(tmp.PA_Program      , MUIA_String_AttachedList, tmp.LV_Programs);

      set(tmp.LV_Menus        , MUIA_ShortHelp, GetStr(MSG_HELP_MenuEntries));
      set(tmp.BT_New          , MUIA_ShortHelp, GetStr(MSG_HELP_NewMenu));
      set(tmp.BT_Delete       , MUIA_ShortHelp, GetStr(MSG_HELP_DeleteMenu));
      set(tmp.STR_Name        , MUIA_ShortHelp, GetStr(MSG_HELP_MenuName));
      set(tmp.LV_Programs     , MUIA_ShortHelp, GetStr(MSG_HELP_Programs));
      set(tmp.BT_NewProgram   , MUIA_ShortHelp, GetStr(MSG_HELP_NewProgram));
      set(tmp.BT_DeleteProgram, MUIA_ShortHelp, GetStr(MSG_HELP_DeleteProgram));
      set(tmp.CY_Asynch       , MUIA_ShortHelp, GetStr(MSG_HELP_Asynch));
      set(tmp.CY_Type         , MUIA_ShortHelp, GetStr(MSG_HELP_Type));
      set(tmp.PA_Program      , MUIA_ShortHelp, GetStr(MSG_HELP_Program));

      DoMethod(tmp.LI_Menus         , MUIM_Notify, MUIA_List_Active        , MUIV_EveryTime  , obj             , 1, MUIM_MenuPrefs_MenuList_Active);
      DoMethod(tmp.BT_New           , MUIM_Notify, MUIA_Pressed            , FALSE           , obj             , 1, MUIM_MenuPrefs_NewEntry);
      DoMethod(tmp.BT_Delete        , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Menus    , 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(tmp.STR_Name         , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj             , 1, MUIM_MenuPrefs_MenuList_ChangeLine);

      DoMethod(tmp.LI_Programs      , MUIM_Notify, MUIA_List_Active        , MUIV_EveryTime  , obj             , 1, MUIM_MenuPrefs_ProgramList_Active);
      DoMethod(tmp.BT_NewProgram    , MUIM_Notify, MUIA_Pressed            , FALSE           , obj             , 1, MUIM_MenuPrefs_NewProgram);
      DoMethod(tmp.BT_DeleteProgram , MUIM_Notify, MUIA_Pressed            , FALSE           , tmp.LI_Programs , 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(tmp.BT_DeleteProgram , MUIM_Notify, MUIA_Pressed            , FALSE           , obj             , 1, MUIM_MenuPrefs_GetProgramList);
      DoMethod(tmp.PA_Program       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj             , 1, MUIM_MenuPrefs_ProgramList_ChangeLine);
      DoMethod(tmp.CY_Asynch        , MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime  , obj             , 1, MUIM_MenuPrefs_Asynch_Active);
      DoMethod(tmp.CY_Type          , MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime  , obj             , 1, MUIM_MenuPrefs_Type_Active);

      DoMethod(tmp.LI_Menus         , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.LI_Menus    , 3, MUIM_CallHook, &MenuList_AppMsgHook, MUIV_TriggerValue);
      DoMethod(tmp.LI_Programs      , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.LI_Programs , 3, MUIM_CallHook, &ProgramList_AppMsgHook, MUIV_TriggerValue);
   }

   return((ULONG)obj);
}

SAVEDS ASM ULONG MenuPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                                  : return(MenuPrefs_New                    (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_NewEntry                 : return(MenuPrefs_NewEntry               (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_MenuList_Active          : return(MenuPrefs_MenuList_Active        (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_MenuList_ChangeLine      : return(MenuPrefs_MenuList_ChangeLine    (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_NewProgram               : return(MenuPrefs_NewProgram             (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_ProgramList_Active       : return(MenuPrefs_ProgramList_Active     (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_ProgramList_ChangeLine   : return(MenuPrefs_ProgramList_ChangeLine (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_Asynch_Active            : return(MenuPrefs_Asynch_Active          (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_Type_Active              : return(MenuPrefs_Type_Active            (cl, obj, (APTR)msg));
      case MUIM_MenuPrefs_GetProgramList           : return(MenuPrefs_GetProgramList         (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}


