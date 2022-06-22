#include "globals.c"
#include "protos.h"

/// serial devices

   static STRPTR dev_list[] = {
      "serial.device", "artser.device", "bscisdn.device",
      "comports.device", "draser.device", "duart.device", "empser.device",
      "envoyserial.device", "fossil.device", "gvpser.device", "highspeed.device",
      "ibmser.device", "netser.device", "newser.device", "scisdn.device",
      "siosbx.device", "squirrelserial.device", "telser.device", "USRSerial.device",
      "uw.device", "v34serial.device", "someserial.device", NULL };

///
/// Modem detect strings

struct ModemDetect
{
   STRPTR ModemName;
   int ATI_nr1;
   STRPTR ATI1;
   int ATI_nr2;
   STRPTR ATI2;
};

static struct ModemDetect modem_detect[] =
{
   { "Arion 1414"          , 3, "WS-1414EV7G"      , -1, NULL },
   { "Arion 2813"          , 3, "WS-2814EM4"       , -1, NULL },

   { "Creatix LC 144VF"    , 3, "V1.600-DS29F"     , -1, NULL },
   { "Creatix SG 2834"     , 3, "V1.000-DS39 VFC"  , -1, NULL },
   { "Creatix SG 2834"     , 3, "V1.430-V34_DS"    , -1, NULL },

   { "Dynalink 1433VQE"    , 3, "CRS-01 EGL"       , -1, NULL },

   { "Mwave 28800"         , 3, "IBM Mwave MODEM"  , -1, NULL },
   { "Smart ST2814"        , 3, "V1.300EV-V34_LP"  , -1, NULL },
   { "Smart ST2814"        , 3, "V1.510J-V34-LP"   , -1, NULL },

   { "Zeus 1414"           , 4, "F-1114"           , -1, NULL },
   { "Zeus 2814"           , 4, "F-1128"           , -1, NULL },

   { "ZyXEL Elite 2864I"   , 0, "28642"            , -1, NULL },
   { "ZyXEL Elite 2864"    , 0, "2864"             , -1, NULL },
   { "ZyXEL Omni.Net"      , 0, "1292"             , -1, NULL },
   { "ZyXEL Omni TA128"    , 0, "1282"             , -1, NULL },
   { "ZyXEL Omni 288s"     , 1, "288"              , -1, NULL },
   { "ZyXEL U-1496 series" , 0, "1496"             , -1, NULL },

   { NULL                  , NULL, NULL            , NULL, NULL }
};

///

#define Action_Abort             255
#define Action_LookingForModem   1
#define Action_CheckingATIs      2

/// ModemDetect_Trigger
ULONG ModemDetect_Trigger(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemDetect_Data *data = INST_DATA(cl, obj);
   BOOL used = FALSE;

   if(ReadSER)
   {
      if(CheckIO(ReadSER))
      {
         WaitIO(ReadSER);

         if(*serial_in && data->buf_pos < 80)
         {
            if(*serial_in == '\r' || *serial_in == '\n')
               data->buffer[data->buf_pos++] = ' ';
            else
               data->buffer[data->buf_pos++] = *serial_in;
            data->buffer[data->buf_pos]   = NULL;
         }
         StartSerialRead(serial_in, 1);

         used = TRUE;
      }
   }

   if(CheckIO(data->time_req))
   {
      WaitIO(data->time_req);

Printf("serial: '%ls'\n", data->buffer);
      if(data->action == Action_LookingForModem)
      {
         if(strstr(data->buffer, "OK"))
         {
            char buf[81];

            strncpy(Config.cnf_serialdevice, data->devices[data->device_nr], 80);
            Config.cnf_serialunit = data->unit;

Printf("found modem on %ls, unit %ld\n", data->devices[data->device_nr], data->unit);
            data->action = Action_CheckingATIs;
            data->buffer[0] = NULL;
            data->buf_pos = 0;

            sprintf(buf, GetStr(MSG_TX_FoundModem), data->devices[data->device_nr], data->unit);
            set(data->TX_Info, MUIA_Text_Contents, buf);
            DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 1, MUIM_ModemDetect_CheckATI);
         }
         else
         {
            data->unit++;
            data->buffer[0] = NULL;
            data->buf_pos = 0;
            DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 1, MUIM_ModemDetect_FindModem);
         }
      }
      else if(data->action == Action_CheckingATIs)
      {
         strcpy(data->ATI[data->ATI_nr], data->buffer);
         data->buffer[0] = NULL;
         data->buf_pos = 0;
         data->ATI_nr++;
         DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 1, MUIM_ModemDetect_CheckATI);
      }

      used = TRUE;
   }

   return(used);
}

///
/// ModemDetect_FindModem
ULONG ModemDetect_FindModem(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemDetect_Data *data = INST_DATA(cl, obj);

   data->action = Action_LookingForModem;

   if(data->ihnode_added)
   {
      DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->ihnode);
      data->ihnode_added = FALSE;
   }
   close_serial();

   while(data->action != Action_Abort)
   {
      if(data->devices[data->device_nr])
      {
         if(data->unit < 10)
         {
Printf("trying %ls unit %ld\n", data->devices[data->device_nr], data->unit);
            if(open_serial(data->devices[data->device_nr], data->unit))
            {
               data->ihnode.ihn_Object  = obj;
               data->ihnode.ihn_Signals = SIG_SER | IO_SIGMASK(data->time_req);
               data->ihnode.ihn_Flags   = 0;
               data->ihnode.ihn_Method  = MUIM_ModemDetect_Trigger;
               DoMethod(_app(obj), MUIM_Application_AddInputHandler, &data->ihnode);
               data->ihnode_added = TRUE;

               send_serial("AAT\r", -1);   // do it twice since the first one might get swallowed
               Delay(10);
               send_serial("AT\r", -1);

               data->time_req->tr_node.io_Command = TR_ADDREQUEST;
               data->time_req->tr_time.tv_secs    = 1;
               data->time_req->tr_time.tv_micro   = 0;
               SendIO((struct IORequest *)data->time_req);

               return(NULL);
            }
            else
            {
               if(data->action != Action_Abort)
                  DoMethod(app, MUIM_Application_InputBuffered);
               data->unit++;
            }
         }
         else
         {
            data->unit = 0;
            data->device_nr++;
         }
      }
      else
      {
         switch(MUI_Request(_app(obj), obj, NULL, NULL, GetStr(MSG_ReqBT_RetryIgnoreCancel), GetStr(MSG_TX_ModemNotFound)))
         {
            case 0:  // cancel
               DoMethod(obj, MUIM_ModemDetect_Abort);
               return(NULL);
            break;
            case 1:  // retry
               data->device_nr = 0;
               data->unit = 0;
               continue;
            default: // ignore
            {
               struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

               mw_data->Page++;
               DoMethod(win, MUIM_MainWindow_SetPage);

               DoMethod(obj, MUIM_ModemDetect_Abort);
               return(NULL);
            }
         }
      }
   }

   return(NULL);
}

///
/// ModemDetect_CheckATI
ULONG ModemDetect_CheckATI(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemDetect_Data *data = INST_DATA(cl, obj);
   char buffer[10];
   int i = 0;

   while(modem_detect[i].ModemName)
   {
      if(strstr(data->ATI[modem_detect[i].ATI_nr1], modem_detect[i].ATI1))
      {
         if(modem_detect[i].ATI_nr2 != -1)
         {
            if(strstr(data->ATI[modem_detect[i].ATI_nr2], modem_detect[i].ATI2))
               break;
         }
         else
            break;
      }
      i++;
   }

   if(modem_detect[i].ModemName || data->ATI_nr > 9)
   {
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

      if(data->ihnode_added)     // got to remove it - otherwise GetModem will be calles again and again
         DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->ihnode);
      data->ihnode_added = FALSE;

      if(modem_detect[i].ModemName)
      {
         strcpy(Config.cnf_modemname, modem_detect[i].ModemName);
         set(obj, MUIA_Window_Open, FALSE);
         DoMethod(obj, MUIM_ModemDetect_GetModem, modem_detect[i].ModemName);
      }
      else
      {
Printf("\n\nCould not find out which modem you are using !\nPlease send the above output to dolphin@zool.unizh.ch\ntogether with the EXACT modem type/name\n(i.g. not only \"Zeus\" but \"Zeus 2814\")\n\n");
      }

mw_data->Page = 2;
//      mw_data->Page++;
      DoMethod(win, MUIM_MainWindow_SetPage);

      DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 1, MUIM_ModemDetect_Abort);
      return(NULL);
   }

   if(data->action == Action_Abort)
      return(NULL);

   data->action = Action_CheckingATIs;

   sprintf(buffer, "ATI%ld\r", data->ATI_nr);
   send_serial(buffer, -1);

   data->time_req->tr_node.io_Command = TR_ADDREQUEST;
   data->time_req->tr_time.tv_secs    = 0;
   data->time_req->tr_time.tv_micro   = 500000;
   SendIO((struct IORequest *)data->time_req);

   return(NULL);
}

///
/// ModemDetect_GetModem
ULONG ModemDetect_GetModem(struct IClass *cl, Object *obj, struct MUIP_ModemDetect_GetModem *msg)
{
   if(msg->modemname && *msg->modemname)
   {
      struct pc_Data pc_data;

      if(ParseConfig("NetConnect:Data/Misc/ModemDatabase", &pc_data))
      {
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.Argument, "Modem"))
            {
               if(!stricmp(pc_data.Contents, msg->modemname))
               {
                  while(ParseNext(&pc_data))
                  {
                     if(!stricmp(pc_data.Argument, "InitString"))
                     {
                        strncpy(Config.cnf_initstring, pc_data.Contents, 80);
                        break;
                     }
                     if(!stricmp(pc_data.Argument, "Protocol"))
                     {
                        Object *window;

                        set(app, MUIA_Application_Sleep, TRUE);
                        if(window = NewObject(CL_ModemProtocol->mcc_Class, NULL, TAG_DONE))
                        {
                           DoMethod(app, OM_ADDMEMBER, window);
                           DoMethod(window, MUIM_ModemProtocol_InitFromPC, &pc_data);
                           set(window, MUIA_Window_Open, TRUE);
                        }
                        break;
                     }
                  }
               }
            }
         }
         ParseEnd(&pc_data);
      }
   }

   return(NULL);
}

///
/// ModemDetect_Abort
ULONG ModemDetect_Abort(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemDetect_Data *data = INST_DATA(cl, obj);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   char buffer[91];

   if(data->ihnode_added)
      DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->ihnode);
   data->ihnode_added = FALSE;

   data->action = Action_Abort;

   Delay(10);

   sprintf(buffer, "%ls, unit %ld", Config.cnf_serialdevice, Config.cnf_serialunit);
   set(mw_data->TX_Device, MUIA_Text_Contents, buffer);
   set(mw_data->TX_Modem, MUIA_Text_Contents, Config.cnf_modemname);
   set(mw_data->TX_InitString, MUIA_Text_Contents, Config.cnf_initstring);
   set(mw_data->TX_DialPrefix, MUIA_Text_Contents, Config.cnf_dialprefix);

   close_serial();

   DoMethod(_app(obj), MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);

   return(NULL);
}
///
/// ModemDetect_New
ULONG ModemDetect_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct ModemDetect_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title        , GetStr(MSG_LA_PleaseWait),
      MUIA_Window_CloseGadget  , FALSE,
      MUIA_Window_RefWindow    , win,
      MUIA_Window_LeftEdge     , MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge      , MUIV_Window_TopEdge_Centered,
      WindowContents          , VGroup,
         Child, tmp.TX_Info = TextObject,
            MUIA_Text_Contents, GetStr(MSG_TX_LookingForModem),
            MUIA_Text_PreParse, "\033c",
         End,
         Child, tmp.BU_Busy = BusyObject,
         End,
         Child, tmp.BT_Abort = MakeButton(MSG_BT__Abort),
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct ModemDetect_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      if(data->time_port = CreateMsgPort())
      {
         if(data->time_req = (struct timerequest *)CreateExtIO(data->time_port, sizeof(struct timerequest)))
         {
            OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)data->time_req,0);

            data->devices        = dev_list;
            data->device_nr      = 0;
            data->unit           = 0;
            data->ihnode_added   = FALSE;
            data->buffer[0]      = NULL;
            data->buf_pos        = 0;
            data->action         = 0;
            data->ATI_nr         = 0;

            DoMethod(data->BT_Abort, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_ModemDetect_Abort);

            return((ULONG)obj);
         }
      }

   }
   if(obj)
      MUI_DisposeObject(obj);
   return(NULL);
}

///
/// ModemDetect_Dispose
ULONG ModemDetect_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemDetect_Data *data = INST_DATA(cl, obj);

   if(data->ihnode_added)
      DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->ihnode);

   if(data->time_req)
   {
      if(!CheckIO(data->time_req))
         AbortIO(data->time_req);
      WaitIO(data->time_req);
      CloseDevice((struct IORequest *)data->time_req);
      DeleteIORequest(data->time_req);
   }
   if(data->time_port)
         DeleteMsgPort(data->time_port);

   return(DoSuperMethodA(cl, obj, msg));
}

///
/// ModemDetect_Dispatcher
SAVEDS ASM ULONG ModemDetect_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                         : return(ModemDetect_New         (cl, obj, (APTR)msg));
      case OM_DISPOSE                     : return(ModemDetect_Dispose     (cl, obj, (APTR)msg));
      case MUIM_ModemDetect_Trigger       : return(ModemDetect_Trigger     (cl, obj, (APTR)msg));
      case MUIM_ModemDetect_FindModem     : return(ModemDetect_FindModem   (cl, obj, (APTR)msg));
      case MUIM_ModemDetect_CheckATI      : return(ModemDetect_CheckATI    (cl, obj, (APTR)msg));
      case MUIM_ModemDetect_Abort         : return(ModemDetect_Abort       (cl, obj, (APTR)msg));
      case MUIM_ModemDetect_GetModem      : return(ModemDetect_GetModem    (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

