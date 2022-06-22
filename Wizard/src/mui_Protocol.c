/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_Protocol.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables
extern struct MUI_CustomClass  *CL_MainWindow;

///

/// Protocol_New
ULONG Protocol_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Protocol_Data tmp;
   Object *originator;
   static STRPTR ARR_CY_Protocol[3], ARR_CY_Script[3];

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   ARR_CY_Protocol[0] = "ppp";
   ARR_CY_Protocol[1] = "slip";
   ARR_CY_Protocol[2] = NULL;

   ARR_CY_Script[0] = GetStr(MSG_CY_Script_DontRecord);
   ARR_CY_Script[1] = GetStr(MSG_CY_Script_Record);
   ARR_CY_Script[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoProtocol)),
      Child, tmp.CY_Protocol = Cycle(ARR_CY_Protocol),
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoUseLoginScript)),
      Child, tmp.CY_Script = Cycle(ARR_CY_Script),
      Child, HVSpace,
   TAG_MORE, msg->ops_AttrList))
   {
      struct Protocol_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, originator);

      *data = tmp;

      set(data->CY_Protocol , MUIA_CycleChain, 1);
      set(data->CY_Script   , MUIA_CycleChain, 1);
      set(data->CY_Protocol      , MUIA_ShortHelp, GetStr(MSG_HELP_Protocol));
      set(data->CY_Script        , MUIA_ShortHelp, GetStr(MSG_HELP_RecordScript));
   }

   return((ULONG)obj);
}

///
/// Protocol_Dispatcher
SAVEDS ULONG Protocol_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW    : return(Protocol_New         (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

