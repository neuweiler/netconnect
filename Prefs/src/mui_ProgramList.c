#include "mui_ProgramList.h"

#ifdef __SASC
SAVEDS ASM struct Program *ProgramList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Program *src) {
#else /* gcc */
struct Program *ProgramList_ConstructFunc()
{
   register APTR pool __asm("a2");
   register struct Program *src __asm("a1");
#endif

   struct Program *new;

   if(new = (struct Program *)AllocVec(sizeof(struct Program), MEMF_ANY | MEMF_CLEAR))
   {
      new->Flags = PRG_Asynch;
      new->Stack = 8192;
      if(src)
         memcpy(new, src, sizeof(struct Program));
   }
   return(new);
}
struct Hook ProgramList_ConstructHook = { { 0,0 }, (VOID *)ProgramList_ConstructFunc  , NULL, NULL };

#ifdef __SASC
SAVEDS ASM VOID ProgramList_DestructFunc(REG(a2) APTR pool, REG(a1) struct Program *program) {
#else /* gcc */
VOID ProgramList_DestructFunc()
{
   register APTR pool __asm("a2");
   register struct Program *program __asm("a1");
#endif

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
struct Hook ProgramList_DestructHook  = { { 0,0 }, (VOID *)ProgramList_DestructFunc   , NULL, NULL };

#ifdef __SASC
SAVEDS ASM LONG ProgramList_DisplayFunc(REG(a0) struct Hook *hook, REG(a2) char **array, REG(a1) struct Program *program) {
#else /* gcc */
LONG ProgramList_DisplayFunc()
{
   register struct Hook *hook __asm("a0");
   register char **array __asm("a2");
   register struct Program *program __asm("a1");
#endif

   if(program)
   {
      *array   = (program->File ? program->File : (STRPTR)"");
   }
   return(NULL);
}
struct Hook ProgramList_DisplayHook   = { { 0,0 }, (VOID *)ProgramList_DisplayFunc , NULL, NULL };


ULONG ProgramList_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
   if(msg->obj == obj)
      return(DoSuperMethodA(cl, obj, msg));
   else
      return(MUIV_DragQuery_Refuse);
}

ULONG ProgramList_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
   struct ProgramList_Data *data = INST_DATA(cl, obj);

   if(msg->obj == obj)
   {
      ULONG ret;

      ret = DoSuperMethodA(cl, obj, msg);    // move the entry first
      if(data->Originator)
         DoMethod(data->Originator, MUIM_MenuPrefs_GetProgramList);
      return(ret);
   }

   return(NULL);
}

ULONG ProgramList_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
   obj = (Object *)DoSuperNew(cl, obj,
      InputListFrame,
      MUIA_List_AutoVisible   , TRUE,
      MUIA_List_DragSortable  , TRUE,
      MUIA_List_ConstructHook , &ProgramList_ConstructHook,
      MUIA_List_DestructHook  , &ProgramList_DestructHook,
      MUIA_List_DisplayHook   , &ProgramList_DisplayHook,
      TAG_MORE, msg->ops_AttrList);

   return((ULONG)obj);
}

#ifdef __SASC
SAVEDS ASM ULONG ProgramList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg) {
#else /* gcc */
ULONG ProgramList_Dispatcher()
{
   register struct IClass *cl __asm("a0");
   register Object *obj __asm("a2");
   register Msg msg __asm("a1");
#endif

   switch(msg->MethodID)
   {
      case OM_NEW          : return(ProgramList_New         (cl, obj, (APTR)msg));
      case MUIM_DragQuery  : return(ProgramList_DragQuery   (cl, obj, (APTR)msg));
      case MUIM_DragDrop   : return(ProgramList_DragDrop    (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}


