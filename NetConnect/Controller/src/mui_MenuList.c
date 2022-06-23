#include "mui_MenuList.h"

///
/// external variables
extern struct MsgPort *appmenu_port;

///

/// ProgramList_ConstructFunc
struct Program *ProgramList_ConstructFunc()
{
   register APTR pool __asm("a2");
   register struct Program *src __asm("a1");

   struct Program *new;

   if((new = (struct Program *)AllocVec(sizeof(struct Program), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct Program));
   return(new);
}

///
/// ProgramList_DestructFunc
VOID ProgramList_DestructFunc()
{
   register APTR pool __asm("a2");
   register struct Program *program __asm("a1");

   if(program)
   {
      if(program->File)
         FreeVec(program->File);
      if(program->CurrentDir)
         FreeVec(program->CurrentDir);
      if(program->OutputFile)
         FreeVec(program->OutputFile);
      if(program->PublicScreen)
         FreeVec(program->PublicScreen);

      FreeVec(program);
   }
}

///
/// MenuList_ConstructFunc
struct MenuEntry *MenuList_ConstructFunc()
{
   register APTR pool __asm("a2");
   register struct MenuEntry *src __asm("a1");

   static const struct Hook ProgramList_ConstructHook = { { 0,0 }, (VOID *)ProgramList_ConstructFunc  , NULL, NULL };
   static const struct Hook ProgramList_DestructHook  = { { 0,0 }, (VOID *)ProgramList_DestructFunc   , NULL, NULL };
   struct MenuEntry *new;

   if(new = (struct MenuEntry *)AllocVec(sizeof(struct MenuEntry), MEMF_ANY | MEMF_CLEAR))
   {
      if(src)
         memcpy(new, src, sizeof(struct MenuEntry));

      if(new->LI_Programs = ListObject,
         MUIA_List_ConstructHook , &ProgramList_ConstructHook,
         MUIA_List_DestructHook  , &ProgramList_DestructHook,
         End)
      {
         new->AppMenuItem = AddAppMenuItem(new->Id, NULL, new->Name, appmenu_port, TAG_END);
      }
      else
      {
         FreeVec(new);
         new = NULL;
      }
   }
   return(new);
}

///
/// MenuList_DestructFunc
VOID MenuList_DestructFunc()
{
   register APTR pool __asm("a2");
   register struct MenuEntry *menu __asm("a1");

   if(menu)
   {
      if(menu->AppMenuItem)
         RemoveAppMenuItem(menu->AppMenuItem);
      if(menu->LI_Programs)
         MUI_DisposeObject(menu->LI_Programs);
      if(menu->Name)
         FreeVec(menu->Name);

      FreeVec(menu);
   }
}

///
/// MenuList_TriggerMenu
ULONG MenuList_TriggerMenu(struct IClass *cl, Object *obj, Msg msg)
{
   struct AppMessage *app_msg;
   struct MenuEntry *menu;

   while(app_msg = (struct AppMessage *)GetMsg(appmenu_port))
   {
      DoMethod(obj, MUIM_List_GetEntry, app_msg->am_ID, &menu);
      if(menu && strcmp((menu->Name ? menu->Name : (STRPTR)""), "~~~~~~~~~~"))
      {
         int i = 0;
         struct Program *program;

         FOREVER
         {
            DoMethod(menu->LI_Programs, MUIM_List_GetEntry, i++, &program);
            if(!program)
               break;
            program->Flags |= PRG_Arguments;
            StartProgram(program, app_msg);
         }
      }
      ReplyMsg(app_msg);
   }
   return(NULL);
}

///
/// MenuList_New
ULONG MenuList_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static const struct Hook MenuList_ConstructHook = { { 0,0 }, (VOID *)MenuList_ConstructFunc  , NULL, NULL };
   static const struct Hook MenuList_DestructHook  = { { 0,0 }, (VOID *)MenuList_DestructFunc   , NULL, NULL };

   obj = (Object *)DoSuperNew(cl, obj,
      MUIA_List_ConstructHook , &MenuList_ConstructHook,
      MUIA_List_DestructHook  , &MenuList_DestructHook,
      TAG_MORE, msg->ops_AttrList);

   return((ULONG)obj);
}

///
/// MenuList_Dispatcher
ULONG MenuList_Dispatcher()
{
   register struct IClass *cl __asm("a0");
   register Object *obj __asm("a2");
   register Msg msg __asm("a1");

   switch (msg->MethodID)
   {
      case OM_NEW                   : return(MenuList_New         (cl, obj, (APTR)msg));
      case MUIM_MenuList_TriggerMenu: return(MenuList_TriggerMenu (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}
///

