/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui_ISPInfo2.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables

///

/// ISPInfo2_New
ULONG ISPInfo2_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct ISPInfo2_Data tmp;
   static STRPTR ARR_CY_Protocol[3], ARR_CY_IPAddress[3], ARR_CY_Script[3];

   ARR_CY_Protocol[0] = "ppp";
   ARR_CY_Protocol[1] = "slip";
   ARR_CY_Protocol[2] = NULL;

   ARR_CY_IPAddress[0] = GetStr(MSG_TX_Dynamic);
   ARR_CY_IPAddress[1] = GetStr(MSG_TX_Static);
   ARR_CY_IPAddress[2] = NULL;

   ARR_CY_Script[0] = GetStr(MSG_CY_Script_Record);
   ARR_CY_Script[1] = GetStr(MSG_CY_Script_DontRecord);
   ARR_CY_Script[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoIPAddress)),
      Child, HGroup,
         Child, tmp.CY_IPAddress = Cycle(ARR_CY_IPAddress),
         Child, tmp.STR_IPAddress = StringObject,
            MUIA_CycleChain      , 1,
            StringFrame,
            MUIA_String_MaxLen   , 20,
            MUIA_String_Accept, "1234567890.",
         End,
      End,
      Child, HVSpace,
      Child, MakeText(GetStr("  DNS1 Address")),
      Child, tmp.STR_DNS1 = StringObject,
         MUIA_CycleChain      , 1,
         StringFrame,
         MUIA_String_MaxLen   , 20,
         MUIA_String_Accept, "1234567890.",
      End,
      Child, tmp.STR_DNS2 = StringObject,
         MUIA_CycleChain      , 1,
         StringFrame,
         MUIA_String_MaxLen   , 20,
         MUIA_String_Accept, "1234567890.",
      End,
      Child, MakeText(GetStr("  Gateway/Destination Address")),
      Child, HGroup,
         Child, tmp.STR_Gateway = StringObject,
            MUIA_CycleChain      , 1,
            StringFrame,
            MUIA_String_MaxLen   , 20,
            MUIA_String_Accept, "1234567890.",
         End,
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct ISPInfo2_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->CY_IPAddress, MUIA_Weight, 10);
      set(data->CY_IPAddress, MUIA_CycleChain, 1);
      set(data->STR_IPAddress, MUIA_Disabled, TRUE);

      set(data->STR_IPAddress    , MUIA_ShortHelp, GetStr(MSG_HELP_IPAddress));
      set(data->CY_IPAddress     , MUIA_ShortHelp, GetStr(MSG_HELP_DynamicStatic));

      DoMethod(data->CY_IPAddress        , MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, data->STR_IPAddress, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
   }

   return((ULONG)obj);
}

///
/// ISPInfo2_Dispatcher
SAVEDS ULONG ISPInfo2_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW    : return(ISPInfo2_New         (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

