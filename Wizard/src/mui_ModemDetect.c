/// includes
#include "/includes.h"
#pragma header

#include "rev.h"
#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_ModemDetect.h"
#include "protos.h"
///

#define SIG_SER   (1L << SerReadPort->mp_SigBit)

/// external variables
extern struct   IOExtSer       *SerReadReq;
extern struct   MsgPort        *SerReadPort;
extern char serial_in[], serial_buffer[];
extern WORD ser_buf_pos;
extern struct config Config;
extern Object *app;
extern Object *win, *status_win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_ModemDetect;
///

/// serial devices

   static STRPTR dev_list[] = {
      "serial.device", "artser.device", "bscisdn.device",
      "comports.device", "draser.device", "duart.device", "empser.device",
      "envoyserial.device", "fossil.device", "gvpser.device", "highspeed.device",
      "ibmser.device", "netser.device", "newser.device", "scisdn.device",
      "siosbx.device", "squirrelserial.device", "USRSerial.device",
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

/// waitfor
BOOL waitfor(struct ModemDetect_Data *data, STRPTR string, LONG secs, LONG fifths)
{
   BOOL timer_running;
   ULONG sig;

   data->time_req->tr_node.io_Command   = TR_ADDREQUEST;
   data->time_req->tr_time.tv_secs      = secs;
   data->time_req->tr_time.tv_micro     = fifths * 20000;
   SetSignal(0, 1L << data->time_port->mp_SigBit);
   SendIO((struct IORequest *)data->time_req);
   timer_running = TRUE;

   ser_buf_pos = 0;
   serial_buffer[0] = NULL;
   serial_startread(serial_in, 1);
   while(!data->abort)
   {
      sig = Wait((1L << SerReadPort->mp_SigBit) | (1L << data->time_port->mp_SigBit) | SIGBREAKF_CTRL_C);
      if(sig & (1L << SerReadPort->mp_SigBit))
      {
         if(CheckIO((struct IORequest *)SerReadReq))
         {
            WaitIO((struct IORequest *)SerReadReq);

            if(ser_buf_pos < 80)    // if string==NULL, we have to check (see below) !
               serial_buffer[ser_buf_pos++] = serial_in[0];
            serial_buffer[ser_buf_pos] = NULL;

            if(string)
            {
               if(strstr(serial_buffer, string))
                  break;

               if(serial_in[0] == '\r' || serial_in[0] == '\n' || serial_in[0] == NULL || ser_buf_pos > 79)
                  ser_buf_pos = 0;
            }
            serial_startread(serial_in, 1);
         }
      }
      if(sig & (1L << data->time_port->mp_SigBit))
      {
         if(CheckIO((struct IORequest *)data->time_req))
         {
            WaitIO((struct IORequest *)data->time_req);
            timer_running = FALSE;
            break;
         }
      }
      if(sig & SIGBREAKF_CTRL_C)
         break;
   }
   serial_stopread();

   if(timer_running)
   {
      if(!CheckIO((struct IORequest *)data->time_req))
         AbortIO((struct IORequest *)data->time_req);
      WaitIO((struct IORequest *)data->time_req);
      timer_running = FALSE;
   }

   if(string)
      return((strstr(serial_buffer, string) ? TRUE : FALSE));
   else
      return(TRUE);
}

///
/// ModemHandler
VOID SAVEDS ModemHandler(register __a0 STRPTR args, register __d0 LONG arg_len)
{
   struct ModemDetect_Data *data = INST_DATA(CL_ModemDetect->mcc_Class, status_win);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   BOOL found_modem, found_type;
   WORD device_nr, unit_nr, ati_nr;
   LONG pos;
   char buffer[81], ATI[10][81], MFR[81], FMI[81];

   if(!(data->time_port = CreateMsgPort()))
      goto abort;
   if(!(data->time_req = (struct timerequest *)CreateExtIO(data->time_port, sizeof(struct timerequest))))
      goto abort;
   if(OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)data->time_req,0))
      goto abort;

   // find device

   found_modem = FALSE;
   device_nr = 0;
   while(dev_list[device_nr] && !found_modem && !data->abort)
   {
      unit_nr = 0;
      while(unit_nr < 10 && !found_modem && !data->abort)
      {
         if(serial_create(dev_list[device_nr], unit_nr))
         {
            Delay(10);  // to allow the modem to wake up .. (don't remove this !!)
            serial_send("\r", -1);
            Delay(10);
            serial_clear();
            serial_send("AAT\r", -1);

            if(data->abort)   goto abort;

            if(!(waitfor(data, "OK", 0, 25)))
               serial_delete();
            else
               found_modem = TRUE;
         }
         if(!found_modem)
            unit_nr++;
      }
      if(!found_modem)
         device_nr++;
   }

   if(data->abort)   goto abort;

   if(!found_modem)
   {
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_ModemNotFound), NULL);
      goto abort;
   }

   sprintf(buffer, GetStr(MSG_TX_FoundModem), dev_list[device_nr], unit_nr);
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, buffer, NULL);
   strncpy(Config.cnf_serialdevice, dev_list[device_nr], sizeof(Config.cnf_serialdevice));
   Config.cnf_serialunit = unit_nr;

   if(data->abort)   goto abort;

   // check ati's

   found_type = FALSE;
   ati_nr = 0;
   while(ati_nr < 10 && !found_type && !data->abort)
   {
      serial_clear();
      sprintf(buffer, "ATI%ld\r", ati_nr);
      serial_send(buffer, -1);
      waitfor(data, NULL, 0, 25);

      strncpy(ATI[ati_nr], serial_buffer, 80);

      if(modem_detect[pos].ATI_nr1 < 10 && modem_detect[pos].ATI_nr2 < 10)
      {
         pos = 0;
         while(modem_detect[pos].ModemName)
         {
            if(strstr(ATI[modem_detect[pos].ATI_nr1], modem_detect[pos].ATI1))
            {
               if(modem_detect[pos].ATI_nr2 != -1)
               {
                  if(strstr(ATI[modem_detect[pos].ATI_nr2], modem_detect[pos].ATI2))
                     break;
               }
               else
                  break;
            }
            pos++;
         }
         if(modem_detect[pos].ModemName)
            found_type = TRUE;
      }
      if(!found_type)
         ati_nr++;
   }

   if(data->abort)   goto abort;

   // check class 2 commands

   if(!found_type)
   {
      serial_clear();
      serial_send("AT#MFR?\r", -1);
      waitfor(data, NULL, 0, 25);
      strncpy(MFR, serial_buffer, 80);

      if(data->abort)   goto abort;

      serial_clear();
      serial_send("AT+FMI?\r", -1);
      waitfor(data, NULL, 0, 25);
      strncpy(FMI, serial_buffer, 80);

      pos = 0;
      while(modem_detect[pos].ModemName && !found_type)
      {
         if(modem_detect[pos].ATI_nr1 < 10)
         {
            if(strstr(ATI[modem_detect[pos].ATI_nr1], modem_detect[pos].ATI1))
               found_type = TRUE;
         }
         else if(modem_detect[pos].ATI_nr1 == 10)
         {
            if(strstr(MFR, modem_detect[pos].ATI1))
               found_type = TRUE;
         }
         else
         {
            if(strstr(FMI, modem_detect[pos].ATI1))
               found_type = TRUE;
         }

         if(found_type && modem_detect[pos].ATI_nr2 != -1)
         {
            found_type = FALSE;

            if(modem_detect[pos].ATI_nr2 < 10)
            {
               if(strstr(ATI[modem_detect[pos].ATI_nr2], modem_detect[pos].ATI2))
                  found_type = TRUE;
            }
            else if(modem_detect[pos].ATI_nr2 == 10)
            {
               if(strstr(MFR, modem_detect[pos].ATI2))
                  found_type = TRUE;
            }
            else
            {
               if(strstr(FMI, modem_detect[pos].ATI2))
                  found_type = TRUE;
            }
         }

         if(!found_type)
            pos++;
      }
   }

   if(data->abort)   goto abort;

   if(found_type)
      strncpy(Config.cnf_modemname, modem_detect[pos].ModemName, sizeof(Config.cnf_modemname));
   else
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_UnknownModemType), NULL);

abort:

   if(data->time_req)
   {
      CloseDevice((struct IORequest *)data->time_req);
      DeleteExtIO((struct IORequest *)data->time_req);
   }
   if(data->time_port)
      DeleteMsgPort(data->time_port);

   serial_delete();

mw_data->Page = 2;
//   mw_data->Page++;
   DoMainMethod(win, MUIM_MainWindow_SetPage, NULL, NULL, NULL);

   Forbid();
   data->Handler = NULL;
   DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, status_win);
   status_win = NULL;
}

///
/// ModemDetect_FindModem
ULONG ModemDetect_FindModem(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemDetect_Data *data = INST_DATA(cl, obj);

   if(data->Handler = CreateNewProcTags(
      NP_Entry       , ModemHandler,
      NP_Name        , "GenesisWizard modem detect",
      NP_StackSize   , 16384,
      NP_WindowPtr   , -1,
NP_Output, Open("CON:0/0/200/100/GenesisWizard modemdetect/AUTO/WAIT/CLOSE", MODE_NEWFILE),
      TAG_END))
   {
      return(TRUE);
   }
   else
   {
      MUI_Request(_app(obj), obj, NULL, NULL, GetStr(MSG_BT__Abort), GetStr(MSG_TX_ErrorLaunchSubtask));
      DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);
   }
   return(FALSE);
}
///
/// ModemDetect_Abort
ULONG ModemDetect_Abort(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemDetect_Data *data = INST_DATA(cl, obj);

   data->abort = TRUE;
   Forbid();
   if(data->Handler)
      Signal((struct Task *)data->Handler, SIGBREAKF_CTRL_C);
   Permit();

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

      data->Handler     = NULL;
      data->abort       = FALSE;
      ser_buf_pos       = 0;

      DoMethod(data->BT_Abort, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_ModemDetect_Abort);
   }
   return((ULONG)obj);
}

///
/// ModemDetect_Dispatcher
SAVEDS ULONG ModemDetect_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                      : return(ModemDetect_New         (cl, obj, (APTR)msg));
      case MUIM_ModemDetect_FindModem  : return(ModemDetect_FindModem   (cl, obj, (APTR)msg));
      case MUIM_ModemDetect_Abort      : return(ModemDetect_Abort       (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

