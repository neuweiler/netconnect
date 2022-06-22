/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_Advanced.h"
#include "protos.h"

///
/// external variables
extern BOOL easy_ppp;

///

/// Advanced_New
ULONG Advanced_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Advanced_Data tmp;
   static STRPTR ARR_RA_Advanced[3];

   ARR_RA_Advanced[0] = "Try to connect now";
   ARR_RA_Advanced[1] = "Enter more info";
   ARR_RA_Advanced[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, HVSpace,
      Child, HGroup,
         Child, HVSpace,
         Child, tmp.RA_Advanced = RadioObject,
            MUIA_CycleChain   , 1,
            MUIA_Radio_Entries, ARR_RA_Advanced,
         End,
         Child, HVSpace,
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Advanced_Data *data = INST_DATA(cl, obj);

      *data = tmp;
   }

   return((ULONG)obj);
}

///
/// Advanced_Dispatcher
SAVEDS ULONG Advanced_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                   : return(Advanced_New      (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

