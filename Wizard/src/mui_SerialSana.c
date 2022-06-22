/// includes
#include "/includes.h"

#include "Strings.h"
#include "/Genesis.h"
#include "mui_SerialSana.h"
#include "protos.h"

///
/// external variables
extern struct Library *MUIMasterBase;

///

/// SerialSana_New
ULONG SerialSana_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct SerialSana_Data tmp;
   static STRPTR ARR_RA_Interface[3];

   ARR_RA_Interface[0] = GetStr(MSG_TX_InterfaceType1);
   ARR_RA_Interface[1] = GetStr(MSG_TX_InterfaceType2);
   ARR_RA_Interface[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoSerialSana)),
      Child, HGroup,
         Child, HVSpace,
         Child, tmp.RA_Interface = RadioObject,
            MUIA_CycleChain   , 1,
            MUIA_Radio_Entries, ARR_RA_Interface,
         End,
         Child, HVSpace,
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct SerialSana_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->RA_Interface, MUIA_ShortHelp, GetStr(MSG_HELP_SerialSana));
   }

   return((ULONG)obj);
}

///
/// SerialSana_Dispatcher
SAVEDS ASM ULONG SerialSana_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW       : return(SerialSana_New         (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

