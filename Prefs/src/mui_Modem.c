#include "globals.c"
#include "protos.h"

/// ModemList_ConstructFunc
SAVEDS ASM struct Modem *ModemList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Modem *src)
{
   struct Modem *new;

   if((new = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct Modem));
   return(new);
}

///
/// Modem_ProtocolList_ConstructFunc
SAVEDS ASM struct ModemProtocol *Modem_ProtocolList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct ModemProtocol *src)
{
   struct ModemProtocol *new;

   if((new = (struct ModemProtocol *)AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct ModemProtocol));
   return(new);
}

///
/// Modem_ProtocolList_DisplayFunc
SAVEDS ASM LONG Modem_ProtocolList_DisplayFunc(REG(a2) char **array, REG(a1) struct ModemProtocol *modem_protocol)
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
         setstring(data->STR_ModemInit, modem->InitString[0]);
      }
      DoMethod(data->PO_Modem, MUIM_Popstring_Close, TRUE);
      DoMethod(obj, MUIM_Modem_UpdateProtocolList);
   }
   else if(msg->flags == MUIV_Modem_PopString_Protocol)
   {
      struct ModemProtocol *modem_protocol;

      DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem_protocol);
      if(modem_protocol)
         setstring(data->STR_ModemInit, modem_protocol->InitString);
      DoMethod(data->PO_ModemInit, MUIM_Popstring_Close, TRUE);
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
   struct Modem_Data tmp;

   if(obj = tmp.GR_Modem = (Object *)DoSuperNew(cl, obj,
MUIA_InnerLeft, 0,
MUIA_InnerRight, 0,
MUIA_InnerBottom, 0,
MUIA_InnerTop, 0,
      Child, HVSpace,
      Child, MUI_MakeObject(MUIO_BarTitle, "Modem settings"),
      Child, ColGroup(2),
         Child, MakeKeyLabel2(MSG_LA_ModemType, "  m"),
         Child, tmp.PO_Modem = PopobjectObject,
            MUIA_Popstring_String      , tmp.STR_Modem = MakeKeyString("Generic", 80, "  m"),
            MUIA_Popstring_Button      , PopButton(MUII_PopUp),
            MUIA_Popobject_StrObjHook  , &strobjhook,
            MUIA_Popobject_Object      , tmp.LV_Modems = ListviewObject,
               MUIA_CycleChain      , 1,
               MUIA_Listview_DoubleClick  , TRUE,
               MUIA_Listview_List         , ListObject,
                  MUIA_Frame              , MUIV_Frame_InputList,
                  MUIA_List_ConstructHook , &ModemList_ConstructHook,
                  MUIA_List_DestructHook  , &des_hook,
                  MUIA_List_CompareHook   , &sorthook,
                  MUIA_List_AutoVisible   , TRUE,
               End,
            End,
         End,
         Child, MakeKeyLabel2(MSG_LA_ModemInit, MSG_CC_ModemInit),
         Child, tmp.PO_ModemInit = PopobjectObject,
            MUIA_Popstring_String      , tmp.STR_ModemInit = MakeKeyString("AT&F&D2", 80, MSG_CC_ModemInit),
            MUIA_Popstring_Button      , PopButton(MUII_PopUp),
            MUIA_Popobject_StrObjHook  , &strobjhook,
            MUIA_Popobject_Object      , tmp.LV_Protocols = ListviewObject,
               MUIA_Listview_DoubleClick  , TRUE,
               MUIA_Listview_List         , ListObject,
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
            MUIA_Popstring_String      , tmp.STR_DialPrefix = MakeKeyString("ATDT", 80, MSG_CC_DialPrefix),
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
         Child, MakeKeyLabel2("  Dial suffix", "  u"),
         Child, tmp.STR_DialSuffix = MakeKeyString(NULL, 80, "  u"),
      End,
      Child, HVSpace,
      Child, MUI_MakeObject(MUIO_BarTitle, "Redial settings"),
      Child, ColGroup(2),
         Child, MakeKeyLabel2(MSG_LA_RedialAttempts, MSG_CC_RedialAttempts),
         Child, tmp.SL_RedialAttempts = MakeKeySlider(0, 99, 15, MSG_CC_RedialAttempts),
         Child, MakeKeyLabel2(MSG_LA_RedialDelay, MSG_CC_RedialDelay),
         Child, tmp.SL_RedialDelay = MakeKeySlider(0, 120, 5, MSG_CC_RedialDelay),
      End,
      Child, HVSpace,
   TAG_MORE, msg->ops_AttrList))
   {
      struct Modem_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->GR_Serial = VGroup,
         MUIA_InnerLeft, 0,
         MUIA_InnerRight, 0,
         MUIA_InnerBottom, 0,
         MUIA_InnerTop, 0,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Device settings"),
         Child, ColGroup(2),
            Child, MakeKeyLabel2(MSG_LA_Device, MSG_CC_Device),
            Child, data->PO_SerialDriver = PopobjectObject,
               MUIA_Popstring_String, data->STR_SerialDriver = MakeKeyString("serial.device", MAXPATHLEN, MSG_CC_Device),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_ObjStrHook  , &objstrhook,
               MUIA_Popobject_Object      , data->LV_Devices = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                     MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
                     MUIA_List_CompareHook   , &sorthook,
                     MUIA_List_AutoVisible   , TRUE,
                  End,
               End,
            End,
            Child, MakeKeyLabel2(MSG_LA_Unit, MSG_CC_Unit),
            Child, data->STR_SerialUnit = TextinputObject,
               MUIA_ControlChar     , *GetStr(MSG_CC_Unit),
               MUIA_CycleChain      , 1,
               StringFrame,
               MUIA_String_MaxLen   , 5,
               MUIA_String_Integer  , 0,
               MUIA_String_Accept   , "1234567890",
            End,
            Child, MakeKeyLabel2(MSG_LA_BaudRate, MSG_CC_BaudRate),
            Child, data->PO_BaudRate = PopobjectObject,
               MUIA_Popstring_String, data->STR_BaudRate = TextinputObject,
                  MUIA_ControlChar     , *GetStr(MSG_CC_BaudRate),
                  MUIA_CycleChain      , 1,
                  StringFrame,
                  MUIA_String_MaxLen   , 8,
                  MUIA_String_Integer  , 57600,
                  MUIA_String_Accept   , "1234567890",
               End,
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_ObjStrHook  , &objstrhook,
               MUIA_Popobject_Object      , data->LV_BaudRate = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , ListObject,
                     MUIA_Frame           , MUIV_Frame_InputList,
                     MUIA_List_SourceArray, ARR_BaudRates,
                     MUIA_List_AutoVisible   , TRUE,
                  End,
               End,
            End,
            Child, MakeKeyLabel2("  Buffer size:", "  b"),
            Child, data->STR_SerBufLen = TextinputObject,
               MUIA_ControlChar     , *GetStr("  b"),
               MUIA_CycleChain      , 1,
               StringFrame,
               MUIA_String_MaxLen   , 7,
               MUIA_String_Integer  , 16384,
               MUIA_String_Accept   , "1234567890",
            End,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Options"),
         Child, ColGroup(4),
            Child, HVSpace,
            Child, data->CH_Carrier = MakeKeyCheckMark(TRUE, MSG_CC_CarrierDetect),
            Child, KeyLLabel1(GetStr(MSG_LA_CarrierDetect), *GetStr(MSG_CC_CarrierDetect)),
            Child, HVSpace,
            Child, HVSpace,
            Child, data->CH_7Wire = MakeKeyCheckMark(TRUE, MSG_CC_HardwareHandshake),
            Child, KeyLLabel1(GetStr(MSG_LA_HardwareHandshake), *GetStr(MSG_CC_HardwareHandshake)),
            Child, HVSpace,
         End,
         Child, HVSpace,
      End;

      if(data->GR_Serial)
      {
         set(data->CH_Carrier, MUIA_CycleChain, 1);
         set(data->CH_7Wire, MUIA_CycleChain, 1);

         set(data->STR_Modem       , MUIA_ShortHelp, GetStr(MSG_Help_Modem));
         set(data->STR_DialPrefix  , MUIA_ShortHelp, GetStr(MSG_Help_DialPrefix));
         set(data->STR_ModemInit   , MUIA_ShortHelp, GetStr(MSG_Help_ModemInit));
         set(data->SL_RedialAttempts, MUIA_ShortHelp, GetStr(MSG_Help_RedialAttempts));
         set(data->SL_RedialDelay  , MUIA_ShortHelp, GetStr(MSG_Help_RedialDelay));

         set(data->STR_SerialDriver, MUIA_ShortHelp, GetStr(MSG_Help_SerialDriver));
         set(data->STR_SerialUnit  , MUIA_ShortHelp, GetStr(MSG_Help_SerialUnit));
         set(data->STR_BaudRate    , MUIA_ShortHelp, GetStr(MSG_Help_BaudRate));
         set(data->CH_Carrier      , MUIA_ShortHelp, GetStr(MSG_Help_Carrier));
         set(data->CH_7Wire        , MUIA_ShortHelp, GetStr(MSG_Help_7Wire));

         DoMethod(data->LV_Modems     , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_Modem);
         DoMethod(data->LV_Protocols  , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_Protocol);
         DoMethod(data->LV_BaudRate   , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_BaudRate, 2, MUIM_Popstring_Close, TRUE);
         DoMethod(data->LV_DialPrefix , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_DialPrefix, 2, MUIM_Popstring_Close, TRUE);
         DoMethod(data->LV_Devices    , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_SerialDriver, 2, MUIM_Popstring_Close, TRUE);

         DoMethod(data->STR_Modem        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_ModemInit);
         DoMethod(data->STR_ModemInit    , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_DialPrefix);
         DoMethod(data->STR_DialPrefix   , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_DialSuffix);
         DoMethod(data->STR_SerialDriver , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_SerialUnit);
         DoMethod(data->STR_SerialUnit   , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_BaudRate);
         DoMethod(data->STR_BaudRate     , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_SerBufLen);
      }
      else
         obj = NULL;
   }
   return((ULONG)obj);
}

///
/// Modem_Dispatcher
SAVEDS ASM ULONG Modem_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                        : return(Modem_New                (cl, obj, (APTR)msg));
      case MUIM_Modem_PopString_Close    : return(Modem_PopString_Close    (cl, obj, (APTR)msg));
      case MUIM_Modem_UpdateProtocolList : return(Modem_UpdateProtocolList (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

