/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Modems.h"
#include "mui_ModemWindow.h"
#include "protos.h"

///
/// external variables
extern struct Hook strobjhook, des_hook, sorthook, objstrhook;
extern Object *win;
extern struct Library *GenesisBase;

///

/// ModemWindow_Init
ULONG ModemWindow_Init(struct IClass *cl, Object *obj, struct MUIP_ModemWindow_Init *msg)
{
   struct ModemWindow_Data *data = INST_DATA(cl, obj);
   struct ParseConfig_Data pc_data;

   if(data->modem = msg->modem)
   {
      setstring(data->STR_Modem        , data->modem->mo_name);
      setstring(data->STR_Comment      , data->modem->mo_comment);
      setstring(data->STR_Init         , data->modem->mo_init);
      setstring(data->STR_DialPrefix   , data->modem->mo_dialprefix);
      setstring(data->STR_DialSuffix   , data->modem->mo_dialsuffix);
      setstring(data->STR_Answer       , data->modem->mo_answer);
      setstring(data->STR_Hangup       , data->modem->mo_hangup);

      setstring(data->STR_Device       , data->modem->mo_device);
      set(data->STR_Unit               , MUIA_String_Integer, data->modem->mo_unit);
      set(data->STR_BaudRate           , MUIA_String_Integer, data->modem->mo_baudrate);
      set(data->STR_SerBufLen          , MUIA_String_Integer, data->modem->mo_serbuflen);
      setslider(data->SL_RedialAttempts, data->modem->mo_redialattempts);
      setslider(data->SL_RedialDelay   , data->modem->mo_redialdelay);
      setslider(data->SL_CommandDelay  , data->modem->mo_commanddelay);

      setcheckmark(data->CH_IgnoreDSR  , data->modem->mo_flags & MFL_IgnoreDSR);
      setcheckmark(data->CH_RadBoogie  , data->modem->mo_flags & MFL_RadBoogie);
      setcheckmark(data->CH_OwnDevUnit , data->modem->mo_flags & MFL_OwnDevUnit);
      setcheckmark(data->CH_DropDTR    , data->modem->mo_flags & MFL_DropDTR);
      setcycle(data->CY_Handshake      , (data->modem->mo_flags & MFL_7Wire ? 0 : (data->modem->mo_flags & MFL_XonXoff ? 1 : 2)));

      setstring(data->STR_Ring      , data->modem->mo_ring);
      setstring(data->STR_Connect   , data->modem->mo_connect);
      setstring(data->STR_NoCarrier , data->modem->mo_nocarrier);
      setstring(data->STR_NoDialtone, data->modem->mo_nodialtone);
      setstring(data->STR_Busy      , data->modem->mo_busy);
      setstring(data->STR_Ok        , data->modem->mo_ok);
      setstring(data->STR_Error     , data->modem->mo_error);
   }

   // load the ModemDatabase
   if(ParseConfig("AmiTCP:db/modems", &pc_data))
   {
      struct ModemX *modem;
      int protocol_nr = 0;

      if(modem = (struct ModemX *)AllocVec(sizeof(struct ModemX), MEMF_ANY | MEMF_CLEAR))
      {
         DoMethod(data->LI_Modems, MUIM_List_Clear);
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.pc_argument, "Modem"))
            {
               if(*modem->Name)
               {
                  modem->ProtocolName[protocol_nr][0] = NULL;
                  DoMethod(data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
                  bzero(modem, sizeof(struct ModemX));
               }
               strncpy(modem->Name, pc_data.pc_contents, sizeof(modem->Name));
               protocol_nr = 0;
            }
            if(!stricmp(pc_data.pc_argument, "Protocol"))
               strncpy(modem->ProtocolName[protocol_nr], pc_data.pc_contents, 20);

            if(!stricmp(pc_data.pc_argument, "InitString"))
            {
               strncpy(modem->InitString[protocol_nr], pc_data.pc_contents, 40);
               if(!*modem->ProtocolName[protocol_nr])
                  strcpy(modem->ProtocolName[protocol_nr], "Default");
               if(protocol_nr < 9)
                  protocol_nr++;
            }
         }

         if(*modem->Name)
         {
            modem->ProtocolName[protocol_nr][0] = NULL;
            DoMethod(data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
         }
         strcpy(modem->Name, GetStr(MSG_TX_Generic));
         strcpy(modem->ProtocolName[0], GetStr(MSG_TX_Default));
         strcpy(modem->InitString[0], "AT&F&D2\\r");
         modem->ProtocolName[1][0] = NULL;
         DoMethod(data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Top);

         FreeVec(modem);
      }
      ParseEnd(&pc_data);
   }
   DoMethod(obj, MUIM_ModemWindow_UpdateProtocolList);

   // load the devices

   DoMethod(data->LI_Devices, MUIM_List_Clear);
   {
      BPTR lock;
      struct FileInfoBlock *fib;

      if(lock = Lock("DEVS:", ACCESS_READ))
      {
         if(fib = AllocDosObject(DOS_FIB, NULL))
         {
            if(Examine(lock, fib))
            {
               while(ExNext(lock, fib))
               {
                  if((fib->fib_DirEntryType < 0) && strstr(fib->fib_FileName, ".device"))
                  {
                     if(stricmp(fib->fib_FileName, "printer.device") &&
                        stricmp(fib->fib_FileName, "mfm.device") &&
                        stricmp(fib->fib_FileName, "clipboard.device") &&
                        stricmp(fib->fib_FileName, "ahi.device") &&
                        stricmp(fib->fib_FileName, "parallel.device"))
                     DoMethod(data->LI_Devices, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
                  }
               }
            }
            FreeDosObject(DOS_FIB, fib);
         }
         UnLock(lock);
      }
   }

   return(NULL);
}

///
/// ModemWindow_CopyData
ULONG ModemWindow_CopyData(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemWindow_Data *data = INST_DATA(cl, obj);

   if(data->modem)
   {
      strncpy(data->modem->mo_name        , (STRPTR)xget(data->STR_Modem     , MUIA_String_Contents), sizeof(data->modem->mo_name));
      strncpy(data->modem->mo_comment     , (STRPTR)xget(data->STR_Comment   , MUIA_String_Contents), sizeof(data->modem->mo_comment));
      strncpy(data->modem->mo_init        , (STRPTR)xget(data->STR_Init      , MUIA_String_Contents), sizeof(data->modem->mo_init));
      strncpy(data->modem->mo_dialprefix  , (STRPTR)xget(data->STR_DialPrefix, MUIA_String_Contents), sizeof(data->modem->mo_dialprefix));
      strncpy(data->modem->mo_dialsuffix  , (STRPTR)xget(data->STR_DialSuffix, MUIA_String_Contents), sizeof(data->modem->mo_dialsuffix));
      strncpy(data->modem->mo_answer      , (STRPTR)xget(data->STR_Answer    , MUIA_String_Contents), sizeof(data->modem->mo_answer));
      strncpy(data->modem->mo_hangup      , (STRPTR)xget(data->STR_Hangup    , MUIA_String_Contents), sizeof(data->modem->mo_hangup));

      strncpy(data->modem->mo_device      , (STRPTR)xget(data->STR_Device    , MUIA_String_Contents), sizeof(data->modem->mo_device));
      data->modem->mo_unit                = xget(data->STR_Unit      , MUIA_String_Integer);
      data->modem->mo_baudrate            = xget(data->STR_BaudRate  , MUIA_String_Integer);
      data->modem->mo_serbuflen           = xget(data->STR_SerBufLen , MUIA_String_Integer);
      data->modem->mo_redialattempts      = xget(data->SL_RedialAttempts, MUIA_Numeric_Value);
      data->modem->mo_redialdelay         = xget(data->SL_RedialDelay   , MUIA_Numeric_Value);
      data->modem->mo_commanddelay        = xget(data->SL_CommandDelay  , MUIA_Numeric_Value);

      data->modem->mo_flags = NULL;
      if(xget(data->CH_IgnoreDSR, MUIA_Selected))
         data->modem->mo_flags |= MFL_IgnoreDSR;
      if(xget(data->CH_RadBoogie, MUIA_Selected))
         data->modem->mo_flags |= MFL_RadBoogie;
      if(xget(data->CH_OwnDevUnit, MUIA_Selected))
         data->modem->mo_flags |= MFL_OwnDevUnit;
      if(xget(data->CH_DropDTR, MUIA_Selected))
         data->modem->mo_flags |= MFL_DropDTR;
      switch(xget(data->CY_Handshake, MUIA_Cycle_Active))
      {
         case 0:
            data->modem->mo_flags |= MFL_7Wire;
            break;
         case 1:
            data->modem->mo_flags |= MFL_XonXoff;
            break;
      }

      strncpy(data->modem->mo_ring        , (STRPTR)xget(data->STR_Ring       , MUIA_String_Contents), sizeof(data->modem->mo_ring));
      strncpy(data->modem->mo_connect     , (STRPTR)xget(data->STR_Connect    , MUIA_String_Contents), sizeof(data->modem->mo_connect));
      strncpy(data->modem->mo_nocarrier   , (STRPTR)xget(data->STR_NoCarrier  , MUIA_String_Contents), sizeof(data->modem->mo_nocarrier));
      strncpy(data->modem->mo_nodialtone  , (STRPTR)xget(data->STR_NoDialtone , MUIA_String_Contents), sizeof(data->modem->mo_nodialtone));
      strncpy(data->modem->mo_busy        , (STRPTR)xget(data->STR_Busy       , MUIA_String_Contents), sizeof(data->modem->mo_busy));
      strncpy(data->modem->mo_ok          , (STRPTR)xget(data->STR_Ok         , MUIA_String_Contents), sizeof(data->modem->mo_ok));
      strncpy(data->modem->mo_error       , (STRPTR)xget(data->STR_Error      , MUIA_String_Contents), sizeof(data->modem->mo_error));
   }
   return(NULL);
}

///
/// ModemWindow_UpdateProtocolList
ULONG ModemWindow_UpdateProtocolList(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemWindow_Data *data = INST_DATA(cl, obj);
   STRPTR ptr;
   struct ModemX *modem;
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
/// ModemWindow_PopString_Close
ULONG ModemWindow_PopString_Close(struct IClass *cl, Object *obj, struct MUIP_ModemWindow_PopString_Close *msg)
{
   struct ModemWindow_Data *data = INST_DATA(cl, obj);

   if(msg->flags == MUIV_ModemWindow_PopString_Modem)
   {
      struct ModemX *modem;

      DoMethod(data->LV_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
      if(modem)
      {
         set(data->STR_Modem, MUIA_String_Contents, modem->Name);
         setstring(data->STR_Init, modem->InitString[0]);
      }
      DoMethod(data->PO_Modem, MUIM_Popstring_Close, TRUE);
      DoMethod(obj, MUIM_ModemWindow_UpdateProtocolList);
   }
   else if(msg->flags == MUIV_ModemWindow_PopString_Protocol)
   {
      struct ModemProtocol *modem_protocol;

      DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem_protocol);
      if(modem_protocol)
         setstring(data->STR_Init, modem_protocol->InitString);
      DoMethod(data->PO_Init, MUIM_Popstring_Close, TRUE);
   }

   return(NULL);
}

///

/// ModemTypesList_ConstructFunc
SAVEDS ASM struct ModemX *ModemTypesList_ConstructFunc(register __a2 APTR pool, register __a1 struct ModemX *src)
{
   struct ModemX *new;

   if((new = (struct ModemX *)AllocVec(sizeof(struct ModemX), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct ModemX));
   return(new);
}
struct Hook ModemTypesList_ConstructHook= { { 0,0 }, (VOID *)ModemTypesList_ConstructFunc , NULL, NULL };

///
/// ModemProtocolList_ConstructFunc
SAVEDS ASM struct ModemProtocol *ModemProtocolList_ConstructFunc(register __a2 APTR pool, register __a1 struct ModemProtocol *src)
{
   struct ModemProtocol *new;

   if((new = (struct ModemProtocol *)AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct ModemProtocol));
   return(new);
}
struct Hook ModemProtocolList_ConstructHook= { { 0,0 }, (VOID *)ModemProtocolList_ConstructFunc , NULL, NULL };

///
/// ModemProtocolList_DisplayFunc
SAVEDS ASM LONG ModemProtocolList_DisplayFunc(register __a2 char **array, register __a1 struct ModemProtocol *modem_protocol)
{
   if(modem_protocol)
   {
      *array++ = modem_protocol->Name;
      *array   = modem_protocol->InitString;
   }
   else
   {
      *array++ = GetStr(MSG_TX_Protocol);
      *array   = GetStr(MSG_TX_InitString);
   }
   return(NULL);
}
struct Hook ModemProtocolList_DisplayHook= { { 0,0 }, (VOID *)ModemProtocolList_DisplayFunc , NULL, NULL };

///

/// ModemWindow_New
ULONG ModemWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct ModemWindow_Data tmp;
   static STRPTR ARR_BaudRates[] = { "9600", "14400", "19200", "38400", "57600", "76800", "115200", "230400", "345600", "460800" , NULL };
   static STRPTR ARR_DialPrefix[] = { "ATDT", "ATDP", "ATD0w", "ATD0,", NULL };
   static STRPTR ARR_Modem_Register[5];
   static STRPTR ARR_Handshake[] = { "RTS/CTS", "Xon/Xoff", "none", NULL };
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   ARR_Modem_Register[0] = GetStr(MSG_TX_ModemRegister1);
   ARR_Modem_Register[1] = GetStr(MSG_TX_ModemRegister2);
   ARR_Modem_Register[2] = GetStr(MSG_TX_ModemRegister3);
   ARR_Modem_Register[3] = GetStr(MSG_TX_ModemRegister4);
   ARR_Modem_Register[4] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_ID       , MAKE_ID('M','D','M','E'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      WindowContents       , VGroup,
         Child, RegisterGroup(ARR_Modem_Register),
            MUIA_CycleChain, 1,

            Child, VGroup, // Modem / TA
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ModemTATitle)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_ModemType, MSG_CC_ModemType),
                  Child, tmp.PO_Modem = PopobjectObject,
                     MUIA_Popstring_String      , tmp.STR_Modem = MakeKeyString(40, MSG_CC_ModemType),
                     MUIA_Popstring_Button      , PopButton(MUII_PopUp),
                     MUIA_Popobject_StrObjHook  , &strobjhook,
                     MUIA_Popobject_Object      , tmp.LV_Modems = ListviewObject,
                        MUIA_CycleChain            , 1,
                        MUIA_Listview_DoubleClick  , TRUE,
                        MUIA_Listview_List         , tmp.LI_Modems = ListObject,
                           MUIA_Frame              , MUIV_Frame_InputList,
                           MUIA_List_ConstructHook , &ModemTypesList_ConstructHook,
                           MUIA_List_DestructHook  , &des_hook,
                           MUIA_List_CompareHook   , &sorthook,
                           MUIA_List_AutoVisible   , TRUE,
                        End,
                     End,
                  End,
                  Child, MakeKeyLabel2(MSG_LA_Comment, MSG_CC_Comment),
                  Child, tmp.STR_Comment = MakeKeyString(40, MSG_CC_Comment),
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_RedialSettingsTitle)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_RedialAttempts, MSG_CC_RedialAttempts),
                  Child, tmp.SL_RedialAttempts  = MakeKeySlider(0, 99, 15, MSG_CC_RedialAttempts),
                  Child, MakeKeyLabel2(MSG_LA_RedialDelay, MSG_CC_RedialDelay),
                  Child, tmp.SL_RedialDelay     = MakeKeySlider(0, 120, 5, MSG_CC_RedialDelay),
               End,
               Child, HVSpace,
            End,

            Child, VGroup, // Device settings
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_DeviceSettingsTitle)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_Device, MSG_CC_Device),
                  Child, tmp.PO_Device = PopobjectObject,
                     MUIA_Popstring_String      , tmp.STR_Device = MakeKeyString(MAXPATHLEN, MSG_CC_Device),
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
                  Child, tmp.STR_Unit = MakeKeyInteger(5, MSG_CC_Unit),
                  Child, MakeKeyLabel2(MSG_LA_BaudRate, MSG_CC_BaudRate),
                  Child, tmp.PO_BaudRate = PopobjectObject,
                     MUIA_Popstring_String, tmp.STR_BaudRate = MakeKeyInteger(8, MSG_CC_BaudRate),
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
                  Child, MakeKeyLabel2(MSG_LA_BufferSize, MSG_CC_BufferSize),
                  Child, tmp.STR_SerBufLen = MakeKeyInteger(7, MSG_CC_BufferSize),
                  Child, MakeKeyLabel2(MSG_LA_FlowControl, MSG_CC_FlowControl),
                  Child, tmp.CY_Handshake = MakeKeyCycle(ARR_Handshake, MSG_CC_FlowControl),
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_SerialOptionsTitle)),
               Child, ColGroup(5),
                  Child, tmp.CH_RadBoogie    = MakeKeyCheckMark(TRUE, MSG_CC_HighspeedMode),
                  Child, KeyLLabel1(GetStr(MSG_LA_HighspeedMode), *GetStr(MSG_CC_HighspeedMode)),
                  Child, HVSpace,
                  Child, tmp.CH_IgnoreDSR  = MakeKeyCheckMark(FALSE, MSG_CC_IgnoreDSR),
                  Child, KeyLLabel1(GetStr(MSG_LA_IgnoreDSR), *GetStr(MSG_CC_IgnoreDSR)),
                  Child, tmp.CH_OwnDevUnit  = MakeKeyCheckMark(FALSE, MSG_CC_OwnDevUnit),
                  Child, KeyLLabel1(GetStr(MSG_LA_OwnDevUnit), *GetStr(MSG_CC_OwnDevUnit)),
                  Child, HVSpace,
                  Child, tmp.CH_DropDTR     = MakeKeyCheckMark(FALSE, MSG_CC_DropDTR),
                  Child, KeyLLabel1(GetStr(MSG_LA_DropDTR), *GetStr(MSG_CC_DropDTR)),
               End,
               Child, HVSpace,
            End,

            Child, VGroup, // commands
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ModemCommandStringsTitle)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_InitString, MSG_CC_InitString),
                  Child, tmp.PO_Init = PopobjectObject,
                     MUIA_Popstring_String      , tmp.STR_Init = MakeKeyString(80, MSG_CC_InitString),
                     MUIA_Popstring_Button      , PopButton(MUII_PopUp),
                     MUIA_Popobject_StrObjHook  , &strobjhook,
                     MUIA_Popobject_Object      , tmp.LV_Protocols = ListviewObject,
                        MUIA_Listview_DoubleClick  , TRUE,
                        MUIA_Listview_List         , tmp.LI_Protocols = ListObject,
                           MUIA_Frame              , MUIV_Frame_InputList,
                           MUIA_List_ConstructHook , &ModemProtocolList_ConstructHook,
                           MUIA_List_DestructHook  , &des_hook,
                           MUIA_List_DisplayHook   , &ModemProtocolList_DisplayHook,
                           MUIA_List_CompareHook   , &sorthook,
                           MUIA_List_Format        , "BAR,",
                           MUIA_List_AutoVisible   , TRUE,
                           MUIA_List_Title         , TRUE,
                        End,
                     End,
                  End,
                  Child, MakeKeyLabel2(MSG_LA_DialPrefix, MSG_CC_DialPrefix),
                  Child, tmp.PO_DialPrefix = PopobjectObject,
                     MUIA_Popstring_String      , tmp.STR_DialPrefix = MakeKeyString(40, MSG_CC_DialPrefix),
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
                  Child, MakeKeyLabel2(MSG_LA_DialSuffix, MSG_CC_DialSuffix),
                  Child, tmp.STR_DialSuffix = MakeKeyString(40, MSG_CC_DialSuffix),
                  Child, MakeKeyLabel2(MSG_LA_Answer, MSG_CC_Answer),
                  Child, tmp.STR_Answer = MakeKeyString(40, MSG_CC_Answer),
                  Child, MakeKeyLabel2(MSG_LA_Hangup, MSG_CC_Hangup),
                  Child, tmp.STR_Hangup = MakeKeyString(40, MSG_CC_Hangup),
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_SettingsTitle)),
               Child, HGroup,
                  Child, MakeKeyLabel2(MSG_LA_CommandDelay, MSG_CC_CommandDelay),
                  Child, tmp.SL_CommandDelay    = MakeKeySlider(0, 100, 10, MSG_CC_CommandDelay),
               End,
               Child, HVSpace,
            End,

            Child, VGroup, // results
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ModemResultCodesTitle)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2("  Ring:", "  r"),
                  Child, tmp.STR_Ring        = MakeKeyString(20, "  r"),
                  Child, MakeKeyLabel2("  Connect:", "  o"),
                  Child, tmp.STR_Connect     = MakeKeyString(20, "  o"),
                  Child, MakeKeyLabel2("  No Carrier:", "  a"),
                  Child, tmp.STR_NoCarrier   = MakeKeyString(20, "  a"),
                  Child, MakeKeyLabel2("  No Dialtone:", "  d"),
                  Child, tmp.STR_NoDialtone  = MakeKeyString(20, "  d"),
                  Child, MakeKeyLabel2("  Busy:", "  u"),
                  Child, tmp.STR_Busy        = MakeKeyString(20, "  u"),
                  Child, MakeKeyLabel2("  OK:", "  k"),
                  Child, tmp.STR_Ok          = MakeKeyString(20, "  k"),
                  Child, MakeKeyLabel2("  Error:", "  e"),
                  Child, tmp.STR_Error       = MakeKeyString(20, "  e"),
               End,
            End,

         End,
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton(MSG_BT_Okay),
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End,
   TAG_MORE, msg->ops_AttrList))
   {
      struct ModemWindow_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(data->CH_IgnoreDSR  , MUIA_CycleChain, 1);
      set(data->CY_Handshake  , MUIA_CycleChain, 1);

      set(data->PO_Modem         , MUIA_ShortHelp, GetStr(MSG_Help_Modem));
      set(data->STR_Comment      , MUIA_ShortHelp, GetStr(MSG_Help_Comment));
      set(data->PO_Init          , MUIA_ShortHelp, GetStr(MSG_Help_InitString));
      set(data->PO_DialPrefix    , MUIA_ShortHelp, GetStr(MSG_Help_DialPrefix));
      set(data->STR_DialSuffix   , MUIA_ShortHelp, GetStr(MSG_Help_DialSuffix));
      set(data->STR_Answer       , MUIA_ShortHelp, GetStr(MSG_Help_Answer));
      set(data->STR_Hangup       , MUIA_ShortHelp, GetStr(MSG_Help_Hangup));
      set(data->SL_RedialAttempts, MUIA_ShortHelp, GetStr(MSG_Help_RedialAttempts));
      set(data->SL_RedialDelay   , MUIA_ShortHelp, GetStr(MSG_Help_RedialDelay));
      set(data->SL_CommandDelay  , MUIA_ShortHelp, GetStr(MSG_Help_CommandDelay));

      set(data->STR_Device       , MUIA_ShortHelp, GetStr(MSG_Help_SerialDevice));
      set(data->STR_Unit         , MUIA_ShortHelp, GetStr(MSG_Help_SerialUnit));
      set(data->STR_BaudRate     , MUIA_ShortHelp, GetStr(MSG_Help_BaudRate));
      set(data->STR_SerBufLen    , MUIA_ShortHelp, GetStr(MSG_Help_BufferSize));
      set(data->CH_RadBoogie     , MUIA_ShortHelp, GetStr(MSG_Help_HighspeedMode));
      set(data->CH_IgnoreDSR     , MUIA_ShortHelp, GetStr(MSG_Help_IgnoreDSR));
      set(data->CH_OwnDevUnit    , MUIA_ShortHelp, GetStr(MSG_Help_UseOwnDevUnit));
      set(data->CY_Handshake     , MUIA_ShortHelp, GetStr(MSG_Help_FlowControl));
      set(data->CH_DropDTR       , MUIA_ShortHelp, GetStr(MSG_Help_DropDTR));

      set(data->STR_Ring         , MUIA_ShortHelp, GetStr(MSG_Help_ModemResultRing));
      set(data->STR_Connect      , MUIA_ShortHelp, GetStr(MSG_Help_ModemResultConnect));
      set(data->STR_NoCarrier    , MUIA_ShortHelp, GetStr(MSG_Help_ModemResultNoCarrier));
      set(data->STR_NoDialtone   , MUIA_ShortHelp, GetStr(MSG_Help_ModemResultNoDialtone));
      set(data->STR_Busy         , MUIA_ShortHelp, GetStr(MSG_Help_ModemResultBusy));
      set(data->STR_Ok           , MUIA_ShortHelp, GetStr(MSG_Help_ModemResultOk));
      set(data->STR_Error        , MUIA_ShortHelp, GetStr(MSG_Help_ModemResultError));

      set(data->BT_Okay          , MUIA_ShortHelp, GetStr(MSG_Help_Okay));
      set(data->BT_Cancel        , MUIA_ShortHelp, GetStr(MSG_Help_Cancel));

      DoMethod(data->LV_Modems     , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , obj, 2, MUIM_ModemWindow_PopString_Close, MUIV_ModemWindow_PopString_Modem);
      DoMethod(data->LV_Protocols  , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , obj, 2, MUIM_ModemWindow_PopString_Close, MUIV_ModemWindow_PopString_Protocol);
      DoMethod(data->LV_BaudRate   , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_BaudRate, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->LV_DialPrefix , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_DialPrefix, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->LV_Devices    , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime , data->PO_Device, 2, MUIM_Popstring_Close, TRUE);

      DoMethod(data->STR_Modem        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Init);
      DoMethod(data->STR_Init         , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_DialPrefix);
      DoMethod(data->STR_DialPrefix   , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_DialSuffix);
      DoMethod(data->STR_Device       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Unit);
      DoMethod(data->STR_Unit         , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_BaudRate);
      DoMethod(data->STR_BaudRate     , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_SerBufLen);

      DoMethod(obj                  , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Modems_EditFinish, obj, 0);
      DoMethod(data->BT_Cancel      , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Modems_EditFinish, obj, 0);
      DoMethod(data->BT_Okay        , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Modems_EditFinish, obj, 1);
   }
   return((ULONG)obj);
}

///
/// ModemWindow_Dispatcher
SAVEDS ASM ULONG ModemWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                               : return(ModemWindow_New               (cl, obj, (APTR)msg));
      case MUIM_ModemWindow_PopString_Close     : return(ModemWindow_PopString_Close   (cl, obj, (APTR)msg));
      case MUIM_ModemWindow_UpdateProtocolList  : return(ModemWindow_UpdateProtocolList(cl, obj, (APTR)msg));
      case MUIM_ModemWindow_Init                : return(ModemWindow_Init              (cl, obj, (APTR)msg));
      case MUIM_ModemWindow_CopyData            : return(ModemWindow_CopyData          (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

