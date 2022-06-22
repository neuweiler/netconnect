/// includes
#include "/includes.h"

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "mui_SanaConfig.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables
extern struct Library *MUIMasterBase;
extern struct GenesisBase *GenesisBase;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct Config Config;

///

/// SanaConfig_LoadConfig
ULONG SanaConfig_LoadConfig(struct IClass *cl, Object *obj, Msg msg)
{
   struct SanaConfig_Data *data = INST_DATA(cl, obj);
   STRPTR file;

   file = (STRPTR)xget(data->STR_Sana2ConfigFile, MUIA_String_Contents);
   if(*file)
   {
      if(GetFileSize(file) > 0)
         DoMethod(data->TI_Sana2ConfigText, MUIM_Textinput_LoadFromFile, file);
   }

   return(NULL);
}

///

/// SanaConfig_New
ULONG SanaConfig_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct SanaConfig_Data tmp;
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, MakeText(GetStr(MSG_TX_InfoSana2ConfigFile)),
      Child, tmp.PA_Sana2ConfigFile = MakePopAsl(tmp.STR_Sana2ConfigFile = MakeString(NULL, MAXPATHLEN), GetStr(MSG_ASL_ChooseSana2ConfigFile), FALSE),
      Child, tmp.TI_Sana2ConfigText = TextinputscrollObject,
         StringFrame,
         MUIA_Weight    , 300,
         MUIA_CycleChain, 1,
         MUIA_Textinput_Multiline, TRUE,
         MUIA_Textinput_WordWrap, 512,
         MUIA_Textinput_AutoExpand, TRUE,
      End,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoSana2Params)),
      Child, tmp.STR_ConfigParams = MakeString(NULL, 1024),
      TAG_MORE, msg->ops_AttrList))
   {
      struct SanaConfig_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, originator);

      *data = tmp;

      set(data->STR_Sana2ConfigFile  , MUIA_ShortHelp, GetStr(MSG_HELP_SanaConfigFile));
      set(data->TI_Sana2ConfigText   , MUIA_ShortHelp, GetStr(MSG_HELP_SanaConfig));
      set(data->STR_ConfigParams     , MUIA_ShortHelp, GetStr(MSG_HELP_ConfigParams));

      DoMethod(data->STR_Sana2ConfigFile , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 1, MUIM_SanaConfig_LoadConfig);
      DoMethod(data->STR_ConfigParams    , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, originator, 3, MUIM_Set, MUIA_Window_ActiveObject, mw_data->BT_Next);
   }

   return((ULONG)obj);
}

///
/// SanaConfig_Dispatcher
SAVEDS ASM ULONG SanaConfig_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                      : return(SanaConfig_New         (cl, obj, (APTR)msg));
      case MUIM_SanaConfig_LoadConfig  : return(SanaConfig_LoadConfig  (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

