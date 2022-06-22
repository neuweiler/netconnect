/// includes
#include "/includes.h"

#include "/NetConnect.h"
#include "/locale/Strings.h"
#include "mui.h"
#include "mui_Editor.h"
#include "protos.h"

///

ULONG Editor_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
   return((msg->obj == obj) ? DoSuperMethodA(cl, obj, msg) : MUIV_DragQuery_Refuse);
}

ULONG Editor_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
   if(msg->obj == obj)
   {
      set(obj, MUIA_UserData, 1);
      return(DoSuperMethodA(cl, obj, msg));
   }
   return(NULL);
}


ULONG Editor_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
   obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Frame              , MUIV_Frame_InputList,
      MUIA_List_AutoVisible   , TRUE,
      MUIA_List_DragSortable  , TRUE,
      MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
      MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
      TAG_MORE, msg->ops_AttrList);

   return((ULONG)obj);
}

SAVEDS ASM ULONG Editor_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
   switch(msg->MethodID)
   {
      case OM_NEW          : return(Editor_New        (cl, obj, (APTR)msg));
      case MUIM_DragQuery  : return(Editor_DragQuery  (cl, obj, (APTR)msg));
      case MUIM_DragDrop   : return(Editor_DragDrop   (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}


