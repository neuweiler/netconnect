/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui_SerialSana.h"
#include "protos.h"

///
/// external variables

///

/// SerialSana_New
ULONG SerialSana_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct SerialSana_Data tmp;
   static STRPTR ARR_RA_Interface[3];

   ARR_RA_Interface[0] = GetStr("  \nUse a modem (analog/ISDN) to dialup\na provider.\n");
   ARR_RA_Interface[1] = GetStr("  \nSpecify a Sana-II driver for your\nnetwork card or ISDN board.");
   ARR_RA_Interface[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, HVSpace,
      Child, MakeText(GetStr("  Analog modem: choose the first option\nISDN board: first one => bscisdn.device,\n    fossil.device or simmilar will be used.\n    second one => use sana2 driver like iwan.device\ndirect connections with network card\n(ariadne, etc.): choose 2nd option")),
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
   }

   return((ULONG)obj);
}

///
/// SerialSana_Dispatcher
SAVEDS ULONG SerialSana_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(SerialSana_New         (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

