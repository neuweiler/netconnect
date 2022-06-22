/// includes
#include "/includes.h"

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_Sana2.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables
extern struct Library *MUIMasterBase;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct Config Config;

///

/// Sana2_New
ULONG Sana2_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Sana2_Data tmp;
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoSana2Device)),
      Child, HGroup,
         Child, MakePopAsl(tmp.STR_SanaDevice = MakeString("DEVS:Networks/", MAXPATHLEN), GetStr(MSG_ASL_ChooseSana2Device), FALSE),
         Child, Label2(GetStr(MSG_LA_Unit)),
         Child, tmp.SL_SanaUnit = NumericbuttonObject,
            MUIA_CycleChain      , 1,
            MUIA_Numeric_Min     , 0,
            MUIA_Numeric_Max     , 20,
            MUIA_Numeric_Value   , 0,
         End,
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Sana2_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, originator);

      *data = tmp;

      set(data->STR_SanaDevice, MUIA_ShortHelp, GetStr(MSG_HELP_SanaDevice));
      set(data->SL_SanaUnit   , MUIA_ShortHelp, GetStr(MSG_HELP_SanaUnit));

      DoMethod(data->STR_SanaDevice      , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, originator, 3, MUIM_Set, MUIA_Window_ActiveObject, mw_data->BT_Next);
   }

   return((ULONG)obj);
}

///
/// Sana2_Dispatcher
SAVEDS ASM ULONG Sana2_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW    : return(Sana2_New         (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///



