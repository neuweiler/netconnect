/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Modem.h"
#include "protos.h"

///
/// external variables
extern struct Hook des_hook;
extern struct Hook strobjhook;
extern struct Hook objstrhook;
extern struct Hook sorthook;
extern Object *win;

///

/// ModemList_ConstructFunc
struct Modem * SAVEDS ModemList_ConstructFunc(register __a2 APTR pool, register __a1 struct Modem *src)
{
   struct Modem *new;

   if((new = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct Modem));
   return(new);
}

///
/// Modem_ProtocolList_ConstructFunc
struct ModemProtocol * SAVEDS Modem_ProtocolList_ConstructFunc(register __a2 APTR pool, register __a1 struct ModemProtocol *src)
{
   struct ModemProtocol *new;

   if((new = (struct ModemProtocol *)AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct ModemProtocol));
   return(new);
}

///
/// Modem_ProtocolList_DisplayFunc
SAVEDS LONG Modem_ProtocolList_DisplayFunc(register __a2 char **array, register __a1 struct ModemProtocol *modem_protocol)
{
   if(modem_protocol)
   {
      *array++ = modem_protocol->Name;
      *array   = modem_protocol->InitString;
   }
   else
   {
      *array++ = GetStr("  \033bProtocol");
      *array   = GetStr("  \033bInitString");
   }
   return(NULL);
}

///

/// Modem_UpdateProtocolList
ULONG Modem_UpdateProtocolList(struct IClass *cl, Object *obj, Msg msg)
{
   struct Modem_Data *data = INST_DATA(cl, obj);
   STRPTR ptr;
   struct Modem *modem;
   struct ModemProtocol *modem_protocol;
   LONG pos;

   get(data->STR_Modem, MUIA_String_Contents, &ptr);
   pos = 0;
   FOREVER
   {
      DoMethod(data->LV_Modems, MUIM_List_GetEntry, pos, &modem);
      if(!modem)
      {
         pos = 0;
         break;
      }
      else
      {
         if(!stricmp(modem->Name, ptr))
            break;
      }
      pos++;
   }

   DoMethod(data->LV_Protocols, MUIM_List_Clear);
   DoMethod(data->LV_Modems, MUIM_List_GetEntry, pos, &modem);
   if(modem)
   {
      if(modem_protocol = AllocVec(sizeof(struct ModemProtocol), MEMF_ANY))
      {
         pos = 0;
         while(*modem->ProtocolName[pos] && pos < 10)
         {
            strcpy(modem_protocol->Name, modem->ProtocolName[pos]);
            strcpy(modem_protocol->InitString, modem->InitString[pos]);
            DoMethod(data->LV_Protocols, MUIM_List_InsertSingle, modem_protocol, MUIV_List_Insert_Bottom);
            pos++;
         }
         FreeVec(modem_protocol);
      }
   }

   return(NULL);
}

///
/// Modem_PopString_Close
ULONG Modem_PopString_Close(struct IClass *cl, Object *obj, struct MUIP_Modem_PopString_Close *msg)
{
   struct Modem_Data *data = INST_DATA(cl, obj);

   if(msg->flags == MUIV_Modem_PopString_Modem)
   {
      struct Modem *modem;

      DoMethod(data->LV_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
      if(modem)
      {
         set(data->STR_Modem, MUIA_String_Contents, modem->Name);
         setstring(data->STR_InitString, modem->InitString[0]);
      }
      DoMethod(data->PO_Modem, MUIM_Popstring_Close, TRUE);
      DoMethod(obj, MUIM_Modem_UpdateProtocolList);
   }
   else if(msg->flags == MUIV_Modem_PopString_Protocol)
   {
      struct ModemProtocol *modem_protocol;

      DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem_protocol);
      if(modem_protocol)
         setstring(data->STR_InitString, modem_protocol->InitString);
      DoMethod(data->PO_InitString, MUIM_Popstring_Close, TRUE);
   }

   return(NULL);
}

///

/// Modem_New
ULONG Modem_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static const struct Hook ModemList_ConstructHook= { { 0,0 }, (VOID *)ModemList_ConstructFunc , NULL, NULL };
   static const struct Hook Modem_ProtocolList_ConstructHook= { { 0,0 }, (VOID *)Modem_ProtocolList_ConstructFunc , NULL, NULL };
   static const struct Hook Modem_ProtocolList_DisplayHook= { { 0,0 }, (VOID *)Modem_ProtocolList_DisplayFunc , NULL, NULL };
   static STRPTR ARR_BaudRates[] = { "9600", "14400", "19200", "38400", "57600", "76800", "115200", "230400", "345600", "460800" , NULL };
   static STRPTR ARR_DialPrefix[] = { "ATDT", "ATDP", "ATD0w", "ATD0,", NULL };
   static STRPTR ARR_Modem_Register[] = { "Modem / TA", "Device", NULL };
   static STRPTR ARR_Handshake[] = { "RTS/CTS", "Xon/Xoff", "none", NULL };
   struct Modem_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Register_Titles, ARR_Modem_Register,
      Child, VGroup,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Modem / TA settings"),
         Child, ColGroup(2),
            Child, MakeKeyLabel2(MSG_LA_ModemType, "  m"),
            Child, tmp.PO_Modem = PopobjectObject,
               MUIA_Popstring_String      , tmp.STR_Modem = MakeKeyString("Generic", 80, "  m"),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_Object      , tmp.LV_Modems = ListviewObject,
                  MUIA_CycleChain            , 1,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , tmp.LI_Modems = ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook , &ModemList_ConstructHook,
                     MUIA_List_DestructHook  , &des_hook,
                     MUIA_List_CompareHook   , &sorthook,
                     MUIA_List_AutoVisible   , TRUE,
                  End,
               End,
            End,
            Child, MakeKeyLabel2(MSG_LA_InitString, MSG_CC_InitString),
            Child, tmp.PO_InitString = PopobjectObject,
               MUIA_Popstring_String      , tmp.STR_InitString = MakeKeyString("AT&F&D2", 80, MSG_CC_InitString),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_Object      , tmp.LV_Protocols = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , tmp.LI_Protocols = ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook , &Modem_ProtocolList_ConstructHook,
                     MUIA_List_DestructHook  , &des_hook,
                     MUIA_List_DisplayHook   , &Modem_ProtocolList_DisplayHook,
                     MUIA_List_CompareHook   , &sorthook,
                     MUIA_List_Format        , "BAR,",
                     MUIA_List_AutoVisible   , TRUE,
                     MUIA_List_Title         , TRUE,
                  End,
               End,
            End,
            Child, MakeKeyLabel2(MSG_LA_DialPrefix, MSG_CC_DialPrefix),
            Child, tmp.PO_DialPrefix = PopobjectObject,
               MUIA_Popstring_String      , tmp.STR_DialPrefix = MakeKeyString("ATDT", 40, MSG_CC_DialPrefix),
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
            Child, MakeKeyLabel2("  Dial suffix:", "  u"),
            Child, tmp.STR_DialSuffix = MakeKeyString(NULL, 40, "  u"),
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Redial settings"),
         Child, ColGroup(2),
            Child, MakeKeyLabel2(MSG_LA_RedialAttempts, MSG_CC_RedialAttempts),
            Child, tmp.SL_RedialAttempts  = MakeKeySlider(0, 99, 15, MSG_CC_RedialAttempts),
            Child, MakeKeyLabel2(MSG_LA_RedialDelay, MSG_CC_RedialDelay),
            Child, tmp.SL_RedialDelay     = MakeKeySlider(0, 120, 5, MSG_CC_RedialDelay),
         End,
         Child, HVSpace,
      End,

      Child, VGroup,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Device settings"),
         Child, ColGroup(2),
            Child, MakeKeyLabel2(MSG_LA_Device, MSG_CC_Device),
            Child, tmp.PO_SerialDevice = PopobjectObject,
               MUIA_Popstring_String      , tmp.STR_SerialDevice = MakeKeyString("serial.device", 80, MSG_CC_Device),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_ObjStrHook  , &objstrhook,
               MUIA_Popobject_Object      , tmp.LV_Devices = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , tmp.LI_Devices = ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                     MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
                     MUIA_List_CompareHook   , &sorthook,
                     MUIA_List_AutoVisible   , TRUE,
                  End,
               End,
            End,
            Child, MakeKeyLabel2(MSG_LA_Unit, MSG_CC_Unit),
            Child, tmp.STR_SerialUnit = TextinputObject,
               StringFrame,
               MUIA_ControlChar     , *GetStr(MSG_CC_Unit),
               MUIA_CycleChain      , 1,
               MUIA_String_MaxLen   , 5,
               MUIA_String_Integer  , 0,
               MUIA_String_Accept   , "1234567890",
            End,
            Child, MakeKeyLabel2(MSG_LA_BaudRate, MSG_CC_BaudRate),
            Child, tmp.PO_BaudRate = PopobjectObject,
               MUIA_Popstring_String, tmp.STR_BaudRate = TextinputObject,
                  StringFrame,
                  MUIA_ControlChar     , *GetStr(MSG_CC_BaudRate),
                  MUIA_CycleChain      , 1,
                  MUIA_String_MaxLen   , 8,
                  MUIA_String_Integer  , 38400,
                  MUIA_String_Accept   , "1234567890",
               End,
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_ObjStrHook  , &objstrhook,
               MUIA_Popobject_Object      , tmp.LV_BaudRate = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_SourceArray   , ARR_BaudRates,
                     MUIA_List_AutoVisible   , TRUE,
                  End,
               End,
            End,
            Child, MakeKeyLabel2("  Buffer size:", "  b"),
            Child, tmp.STR_SerBufLen = TextinputObject,
               StringFrame,
               MUIA_ControlChar     , *GetStr("  b"),
               MUIA_CycleChain      , 1,
               MUIA_String_MaxLen   , 7,
               MUIA_String_Integer  , 16384,
               MUIA_String_Accept   , "1234567890",
            End,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Options"),
         Child, HGroup,
            Child, HVSpace,
            Child, ColGroup(2),
               Child, tmp.CH_RadBoogie    = MakeKeyCheckMark(TRUE, "  h"),
               Child, KeyLLabel1(GetStr("  Highspeed mode"), *GetStr("  h")),
               Child, tmp.CH_IgnoreDSR  = MakeKeyCheckMark(FALSE, "  i"),
               Child, KeyLLabel1(GetStr("  Ignore DSR"), *GetStr("  i")),
               Child, tmp.CH_OwnDevUnit  = MakeKeyCheckMark(FALSE, "  o"),
               Child, KeyLLabel1(GetStr("  Use OwnDevUnit"), *GetStr("  o")),
            End,
            Child, HVSpace,
            Child, VGroup,
               Child, HGroup,
                  Child, MakeKeyLabel2("  Flow control:", "  f"),
                  Child, tmp.CY_Handshake = MakeKeyCycle(ARR_Handshake, "  f"),
               End,
               Child,HVSpace,
            End,
            Child,HVSpace,
         End,
         Child, HVSpace,
      End,

   TAG_MORE, msg->ops_AttrList))
   {
      struct Modem_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->CH_IgnoreDSR  , MUIA_CycleChain, 1);
      set(data->CY_Handshake  , MUIA_CycleChain, 1);

      set(data->STR_Modem        , MUIA_ShortHelp, GetStr(MSG_Help_Modem));
      set(data->STR_DialPrefix   , MUIA_ShortHelp, GetStr(MSG_Help_DialPrefix));
      set(data->STR_InitString   , MUIA_ShortHelp, GetStr(MSG_Help_InitString));
      set(data->SL_RedialAttempts, MUIA_ShortHelp, GetStr(MSG_Help_RedialAttempts));
      set(data->SL_RedialDelay   , MUIA_ShortHelp, GetStr(MSG_Help_RedialDelay));

      set(data->STR_SerialDevice, MUIA_ShortHelp, GetStr(MSG_Help_SerialDevice));
      set(data->STR_SerialUnit  , MUIA_ShortHelp, GetStr(MSG_Help_SerialUnit));
      set(data->STR_BaudRate    , MUIA_ShortHelp, GetStr(MSG_Help_BaudRate));
//      set(data->CH_7Wire        , MUIA_ShortHelp, GetStr(MSG_Help_7Wire));

      DoMethod(data->LV_Modems     , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_Modem);
      DoMethod(data->LV_Protocols  , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_Protocol);
      DoMethod(data->LV_BaudRate   , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_BaudRate, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->LV_DialPrefix , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_DialPrefix, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->LV_Devices    , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_SerialDevice, 2, MUIM_Popstring_Close, TRUE);

      DoMethod(data->STR_Modem        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_InitString);
      DoMethod(data->STR_InitString   , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_DialPrefix);
      DoMethod(data->STR_DialPrefix   , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_DialSuffix);
      DoMethod(data->STR_SerialDevice , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_SerialUnit);
      DoMethod(data->STR_SerialUnit   , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_BaudRate);
      DoMethod(data->STR_BaudRate     , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_SerBufLen);
   }
   return((ULONG)obj);
}

///
/// Modem_Dispatcher
SAVEDS ULONG Modem_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                         : return(Modem_New                (cl, obj, (APTR)msg));
      case MUIM_Modem_PopString_Close     : return(Modem_PopString_Close    (cl, obj, (APTR)msg));
      case MUIM_Modem_UpdateProtocolList  : return(Modem_UpdateProtocolList (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

