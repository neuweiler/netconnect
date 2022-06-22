#include "globals.c"
#include "protos.h"

/// ModemWindow_Init
ULONG ModemWindow_Init(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemWindow_Data *data = INST_DATA(cl, obj);
   struct pc_Data pc_data;

   /** load devices into serial.dev list **/
   set(data->LV_SerialDevices, MUIA_List_Quiet, TRUE);
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
                     DoMethod(data->LV_SerialDevices, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
               }
            }
            FreeDosObject(DOS_FIB, fib);
         }
         UnLock(lock);
      }
   }
   set(data->LV_SerialDevices, MUIA_List_Quiet, FALSE);

   /**** load the ModemSettings into the List ****/
   if(ParseConfig("NetConnect:Data/Misc/ModemDatabase", &pc_data))
   {
      struct Modem *modem;
      int protocol_nr = 0;

      if(modem = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY))
      {
         set(data->LV_Modems, MUIA_List_Quiet, TRUE);
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.Argument, "Modem"))
            {
               if(*modem->Name)
               {
                  modem->ProtocolName[protocol_nr][0] = NULL;
                  DoMethod(data->LV_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
                  bzero(modem, sizeof(struct Modem));
               }
               strncpy(modem->Name, pc_data.Contents, 80);
               protocol_nr = 0;
            }
            if(!stricmp(pc_data.Argument, "Protocol"))
               strncpy(modem->ProtocolName[protocol_nr], pc_data.Contents, 20);

            if(!stricmp(pc_data.Argument, "InitString"))
            {
               strncpy(modem->InitString[protocol_nr], pc_data.Contents, 20);
               if(!*modem->ProtocolName[protocol_nr])
                  strcpy(modem->ProtocolName[protocol_nr], "Default");
               if(protocol_nr < 9)
                  protocol_nr++;
            }
         }

         if(*modem->Name)
         {
            modem->ProtocolName[protocol_nr][0] = NULL;
            DoMethod(data->LV_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
         }
         strcpy(modem->Name, "Generic");
         strcpy(modem->ProtocolName[0], "Default");
         strcpy(modem->InitString[0], "AT&F&D2");
         modem->ProtocolName[1][0] = NULL;
         DoMethod(data->LV_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Top);

         FreeVec(modem);
         set(data->LV_Modems, MUIA_List_Quiet, FALSE);
      }
      ParseEnd(&pc_data);
   }
   DoMethod(obj, MUIM_ModemWindow_UpdateProtocolList);

   return(NULL);
}

///
/// ModemWindow_Finish
ULONG ModemWindow_Finish(struct IClass *cl, Object *obj, struct MUIP_ModemWindow_Finish *msg)
{
   struct ModemWindow_Data *data = INST_DATA(cl, obj);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   char buffer[91];

   if(msg->okay)
   {
      strcpy(Config.cnf_serialdevice, xgetstr(data->STR_SerialDevice));
      Config.cnf_serialunit = xget(data->SL_SerialUnit, MUIA_Numeric_Value);
      strcpy(Config.cnf_modemname, (STRPTR)xget(data->TX_ModemName, MUIA_Text_Contents));
      strcpy(Config.cnf_initstring, xgetstr(data->STR_InitString));
      strcpy(Config.cnf_dialprefix, xgetstr(data->STR_DialPrefix));
      strcpy(Config.cnf_dialsuffix, xgetstr(data->STR_DialSuffix));

      sprintf(buffer, "%ls, unit %ld", Config.cnf_serialdevice, Config.cnf_serialunit);
      set(mw_data->TX_Device, MUIA_Text_Contents, buffer);
      set(mw_data->TX_Modem, MUIA_Text_Contents, Config.cnf_modemname);
      set(mw_data->TX_InitString, MUIA_Text_Contents, Config.cnf_initstring);
      set(mw_data->TX_DialPrefix, MUIA_Text_Contents, Config.cnf_dialprefix);
   }

   DoMethod(_app(obj), MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);

   return(NULL);
}

///
/// ModemWindow_UpdateProtocolList
ULONG ModemWindow_UpdateProtocolList(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemWindow_Data *data = INST_DATA(cl, obj);
   STRPTR ptr;
   struct Modem *modem;
   struct ModemProtocol *modem_protocol;
   LONG pos;

   get(data->TX_ModemName, MUIA_Text_Contents, &ptr);
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
/// ModemWindow_PopString
ULONG ModemWindow_PopString(struct IClass *cl, Object *obj, struct MUIP_ModemWindow_PopString *msg)
{
   struct ModemWindow_Data *data = INST_DATA(cl, obj);

   switch(msg->flags)
   {
      case MUIV_ModemWindow_PopString_SerialDevice:
      {
         STRPTR ptr;

         DoMethod(data->LV_SerialDevices, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
         if(ptr)
            setstring(data->STR_SerialDevice, ptr);
         DoMethod(data->PO_SerialDevice, MUIM_Popstring_Close, TRUE);
      }
      break;

      case MUIV_ModemWindow_PopString_Modem:
      {
         struct Modem *modem;

         DoMethod(data->LV_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
         if(modem)
         {
            set(data->TX_ModemName, MUIA_Text_Contents, modem->Name);
            setstring(data->STR_InitString, modem->InitString[0]);
         }
         DoMethod(data->PO_ModemName, MUIM_Popstring_Close, TRUE);
         DoMethod(obj, MUIM_ModemWindow_UpdateProtocolList);
      }
      break;

      case MUIV_ModemWindow_PopString_Protocol:
      {
         struct ModemProtocol *modem_protocol;

         DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem_protocol);
         if(modem_protocol)
            setstring(data->STR_InitString, modem_protocol->InitString);
         DoMethod(data->PO_InitString, MUIM_Popstring_Close, TRUE);
      }
      break;
   }

   return(NULL);
}

///
/// ModemWindow_ModemList_ConstructFunc
SAVEDS ASM struct Modem *ModemWindow_ModemList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Modem *src)
{
   struct Modem *new;

   if((new = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct Modem));
   return(new);
}

///
/// ModemWindow_ProtocolList_ConstructFunc
SAVEDS ASM struct ModemProtocol *ModemWindow_ProtocolList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct ModemProtocol *src)
{
   struct ModemProtocol *new;

   if((new = (struct ModemProtocol *)AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct ModemProtocol));
   return(new);
}

///
/// ModemWindow_ProtocolList_DisplayFunc
SAVEDS ASM LONG ModemWindow_ProtocolList_DisplayFunc(REG(a2) char **array, REG(a1) struct ModemProtocol *modem_protocol)
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
/// ModemWindow_New
ULONG ModemWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct ModemWindow_Data tmp;
   static const struct Hook ModemWindow_ModemList_ConstructHook= { { 0,0 }, (VOID *)ModemWindow_ModemList_ConstructFunc , NULL, NULL };
   static const struct Hook ModemWindow_ProtocolList_ConstructHook= { { 0,0 }, (VOID *)ModemWindow_ProtocolList_ConstructFunc , NULL, NULL };
   static const struct Hook ModemWindow_ProtocolList_DisplayHook= { { 0,0 }, (VOID *)ModemWindow_ProtocolList_DisplayFunc , NULL, NULL };

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_LA_ModemWindow_Title),
      MUIA_Window_ID       , MAKE_ID('M','O','D','M'),
      WindowContents       , VGroup,
         Child, ColGroup(2),
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, MakeKeyLabel(MSG_LA_Device, MSG_CC_Device),
            Child, tmp.PO_SerialDevice = PopobjectObject,
               MUIA_Popstring_String      , tmp.STR_SerialDevice = MakeKeyString(Config.cnf_serialdevice, MAXPATHLEN, MSG_CC_Device),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_Object      , tmp.LV_SerialDevices = ListviewObject,
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
            Child, MakeKeyLabel(MSG_LA_Unit, MSG_CC_Unit),
            Child, HGroup,
               Child, tmp.SL_SerialUnit = NumericbuttonObject,
                  MUIA_CycleChain      , 1,
                  MUIA_ControlChar     , *GetStr(MSG_CC_Unit),
                  MUIA_Numeric_Min     , 0,
                  MUIA_Numeric_Max     , 20,
                  MUIA_Numeric_Value   , Config.cnf_serialunit,
               End,
               Child, HVSpace,
            End,
         End,
         Child, ColGroup(2),
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, Label(GetStr(MSG_LA_Modem)),
            Child, tmp.PO_ModemName = PopobjectObject,
               MUIA_Popstring_String      , tmp.TX_ModemName = TextObject,
                  TextFrame,
                  MUIA_Background, MUII_TextBack,
                  MUIA_Text_Contents, Config.cnf_modemname,
               End,
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &txtobjhook,
               MUIA_Popobject_Object      , tmp.LV_Modems = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , ListObject,
                     InputListFrame,
                     MUIA_List_ConstructHook , &ModemWindow_ModemList_ConstructHook,
                     MUIA_List_DestructHook  , &deshook,
                     MUIA_List_CompareHook   , &sorthook,
                     MUIA_List_AutoVisible   , TRUE,
                  End,
               End,
            End,
            Child, MakeKeyLabel(MSG_LA_InitString, MSG_CC_InitString),
            Child, tmp.PO_InitString = PopobjectObject,
               MUIA_Popstring_String      , tmp.STR_InitString = MakeKeyString(Config.cnf_initstring, 80, MSG_CC_InitString),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_Object      , tmp.LV_Protocols = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook , &ModemWindow_ProtocolList_ConstructHook,
                     MUIA_List_DestructHook  , &deshook,
                     MUIA_List_DisplayHook   , &ModemWindow_ProtocolList_DisplayHook,
                     MUIA_List_CompareHook   , &sorthook,
                     MUIA_List_Format        , "BAR,",
                     MUIA_List_AutoVisible   , TRUE,
                     MUIA_List_Title         , TRUE,
                  End,
               End,
            End,
            Child, MakeKeyLabel(MSG_LA_DialPrefix, MSG_CC_DialPrefix),
            Child, tmp.STR_DialPrefix = MakeKeyString(Config.cnf_dialprefix, 80, MSG_CC_DialPrefix),
            Child, MakeKeyLabel(MSG_LA_DialSuffix, MSG_CC_DialSuffix),
            Child, tmp.STR_DialSuffix = MakeKeyString(Config.cnf_dialsuffix, 10, MSG_CC_DialSuffix),
         End,
         Child, MUI_MakeObject(MUIO_HBar, 2),
         Child, HGroup,
            Child, tmp.BT_Okay = MakeButton(MSG_BT_Okay),
            Child, HVSpace,
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End))
   {
      struct ModemWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->STR_SerialDevice , MUIA_ShortHelp, GetStr(MSG_HELP_SerialDevice));
      set(data->SL_SerialUnit    , MUIA_ShortHelp, GetStr(MSG_HELP_SerialUnit));
      set(data->TX_ModemName     , MUIA_ShortHelp, GetStr(MSG_HELP_Modem));
      set(data->STR_InitString   , MUIA_ShortHelp, GetStr(MSG_HELP_InitString));
      set(data->STR_DialPrefix   , MUIA_ShortHelp, GetStr(MSG_HELP_DialPrefix));
      set(data->STR_DialSuffix   , MUIA_ShortHelp, GetStr(MSG_HELP_DialSuffix));
      set(data->BT_Okay          , MUIA_ShortHelp, GetStr(MSG_HELP_Okay));
      set(data->BT_Cancel        , MUIA_ShortHelp, GetStr(MSG_HELP_Cancel));

      set(obj, MUIA_Window_ActiveObject, data->STR_SerialDevice);

      DoMethod(data->LV_SerialDevices , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime  , obj, 2, MUIM_ModemWindow_PopString, MUIV_ModemWindow_PopString_SerialDevice);
      DoMethod(data->LV_Modems        , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime  , obj, 2, MUIM_ModemWindow_PopString, MUIV_ModemWindow_PopString_Modem);
      DoMethod(data->LV_Protocols     , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime  , obj, 2, MUIM_ModemWindow_PopString, MUIV_ModemWindow_PopString_Protocol);

      DoMethod(data->BT_Okay, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_ModemWindow_Finish, 1);
      DoMethod(data->BT_Cancel, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_ModemWindow_Finish, 0);
      DoMethod(obj            , MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2, MUIM_ModemWindow_Finish, 0);
   }
   return((ULONG)obj);
}

///

/// ModemWindow_Dispatcher
SAVEDS ASM ULONG ModemWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                         : return(ModemWindow_New         (cl, obj, (APTR)msg));
      case MUIM_ModemWindow_Finish        : return(ModemWindow_Finish      (cl, obj, (APTR)msg));
      case MUIM_ModemWindow_Init          : return(ModemWindow_Init        (cl, obj, (APTR)msg));
      case MUIM_ModemWindow_PopString     : return(ModemWindow_PopString   (cl, obj, (APTR)msg));
      case MUIM_ModemWindow_UpdateProtocolList  : return(ModemWindow_UpdateProtocolList (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

