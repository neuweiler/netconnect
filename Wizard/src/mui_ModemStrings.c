/// includes
#include "/includes.h"

#include "Strings.h"
#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "mui.h"
#include "mui_ModemStrings.h"
#include "mui_MainWindow.h"
#include "mui_SerialModem.h"
#include "protos.h"

#include "images/setup_page2.h"

///
/// external variables
extern struct Library *MUIMasterBase;
extern Object *win;
extern struct GenesisBase *GenesisBase;
extern struct MUI_CustomClass  *CL_MainWindow, *CL_SerialModem;
extern struct Config Config;

extern struct Hook strobjhook, sorthook, txtobjhook, deshook, objstrhook;

///

/// ProtocolList_ConstructFunc
SAVEDS ASM struct ModemProtocol *ProtocolList_ConstructFunc(register __a2 APTR pool, register __a1 struct ModemProtocol *src)
{
   struct ModemProtocol *new;

   if((new = (struct ModemProtocol *)AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct ModemProtocol));
   return(new);
}

///
/// ProtocolList_DisplayFunc
SAVEDS ASM LONG ProtocolList_DisplayFunc(register __a2 char **array, register __a1 struct ModemProtocol *modem_protocol)
{
   if(modem_protocol)
   {
      *array++ = modem_protocol->Name;
      *array   = modem_protocol->InitString;
   }
   else
   {
      *array++ = GetStr(MSG_BLA_Protocol);
      *array   = GetStr(MSG_BLA_InitString2);
   }
   return(NULL);
}

///
/// initstring_objstrfunc
SAVEDS ASM VOID initstring_objstrfunc(register __a2 Object *list,register __a1 Object *str)
{
   struct ModemProtocol *modem_protocol;

   DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem_protocol);
   if(modem_protocol)
      set(str, MUIA_String_Contents, modem_protocol->InitString);
}

///

/// ModemStrings_LoadData
ULONG ModemStrings_LoadData(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemStrings_Data *data = INST_DATA(cl, obj);
   struct MainWindow_Data   *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct SerialModem_Data *sm_data = INST_DATA(CL_SerialModem->mcc_Class, mw_data->GR_SerialModem);
   struct ParseConfig_Data pc_data;
   struct ModemProtocol *modem_protocol;
   int counter = 0;
   STRPTR modemname;

   // load the Modem InitStrings into the List
   if(ParseConfig("AmiTCP:db/modems", &pc_data))
   {
      set(data->LV_Protocols, MUIA_List_Quiet, TRUE);
      DoMethod(data->LV_Protocols, MUIM_List_Clear);

      if(modem_protocol = AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR))
      {
         modemname = (STRPTR)xget(sm_data->TX_ModemName, MUIA_Text_Contents);
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.pc_argument, "Modem"))
            {
               if(!strcmp(pc_data.pc_contents, modemname))
                  break;
            }
         }
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.pc_argument, "Modem"))
               break;

            if(!stricmp(pc_data.pc_argument, "Protocol"))
               strncpy(modem_protocol->Name, pc_data.pc_contents, 40);

            if(!stricmp(pc_data.pc_argument, "InitString"))
            {
               if(!counter++)
                  set(data->STR_InitString, MUIA_String_Contents, pc_data.pc_contents);

               strncpy(modem_protocol->InitString, pc_data.pc_contents, 40);
               if(!*modem_protocol->Name)
                  strcpy(modem_protocol->Name, GetStr(MSG_TX_DefaultProtocolName));
               DoMethod(data->LV_Protocols, MUIM_List_InsertSingle, modem_protocol, MUIV_List_Insert_Bottom);
               modem_protocol->Name[0] = NULL;
            }
         }
         FreeVec(modem_protocol);
      }
      set(data->LV_Protocols, MUIA_List_Quiet, FALSE);
      ParseEnd(&pc_data);
   }

   return(NULL);
}

///

/// ModemStrings_New
ULONG ModemStrings_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct ModemStrings_Data tmp;
   static const struct Hook ProtocolList_ConstructHook= { { 0,0 }, (VOID *)ProtocolList_ConstructFunc , NULL, NULL };
   static const struct Hook ProtocolList_DisplayHook= { { 0,0 }, (VOID *)ProtocolList_DisplayFunc , NULL, NULL };
   static const struct Hook initstring_objstrhook= { { 0,0 }, (VOID *)initstring_objstrfunc , NULL, NULL };
   static STRPTR ARR_DialPrefix[] = { "AT", "ATDT", "ATDP", "ATD0w", "ATD0,", NULL };
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoInitString)),
      Child, tmp.PO_InitString = PopobjectObject,
         MUIA_Popstring_String      , tmp.STR_InitString = MakeString("AT&F&D2", 80),
         MUIA_Popstring_Button      , PopButton(MUII_PopUp),
         MUIA_Popobject_StrObjHook  , &strobjhook,
         MUIA_Popobject_ObjStrHook  , &initstring_objstrhook,
         MUIA_Popobject_Object      , tmp.LV_Protocols = ListviewObject,
            MUIA_Listview_DoubleClick  , TRUE,
            MUIA_Listview_List         , ListObject,
               MUIA_Frame              , MUIV_Frame_InputList,
               MUIA_List_ConstructHook , &ProtocolList_ConstructHook,
               MUIA_List_DestructHook  , &deshook,
               MUIA_List_DisplayHook   , &ProtocolList_DisplayHook,
               MUIA_List_CompareHook   , &sorthook,
               MUIA_List_Format        , "BAR,",
               MUIA_List_AutoVisible   , TRUE,
               MUIA_List_Title         , TRUE,
            End,
         End,
      End,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoDialPrefix)),
      Child, tmp.PO_DialPrefix = PopobjectObject,
         MUIA_Popstring_String      , tmp.STR_DialPrefix = MakeString("ATDT", 80),
         MUIA_Popstring_Button      , PopButton(MUII_PopUp),
         MUIA_Popobject_StrObjHook  , &strobjhook,
         MUIA_Popobject_ObjStrHook  , &objstrhook,
         MUIA_Popobject_Object      , tmp.LV_DialPrefix = ListviewObject,
            MUIA_Listview_DoubleClick  , TRUE,
            MUIA_Listview_List         , ListObject,
               MUIA_Frame              , MUIV_Frame_InputList,
               MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
               MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
               MUIA_List_SourceArray   , ARR_DialPrefix,
            End,
         End,
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct ModemStrings_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, originator);

      *data = tmp;

      set(data->STR_InitString   , MUIA_ShortHelp, GetStr(MSG_HELP_InitString));
      set(data->STR_DialPrefix   , MUIA_ShortHelp, GetStr(MSG_HELP_DialPrefix));

      DoMethod(data->LV_Protocols        , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime, data->PO_InitString, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->LV_DialPrefix       , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime, data->PO_DialPrefix, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->STR_InitString      , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, originator, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_DialPrefix);
      DoMethod(data->STR_DialPrefix      , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, originator, 3, MUIM_Set, MUIA_Window_ActiveObject, mw_data->BT_Next);
   }

   return((ULONG)obj);
}

///
/// ModemString_Dispatcher
SAVEDS ASM ULONG ModemStrings_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                      : return(ModemStrings_New         (cl, obj, (APTR)msg));
      case MUIM_ModemStrings_LoadData  : return(ModemStrings_LoadData    (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

