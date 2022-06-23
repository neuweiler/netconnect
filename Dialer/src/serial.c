/// includes & defines
#include "/includes.h"

#include <libraries/owndevunit.h>
#include <pragmas/owndevunit_pragmas.h>
#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Online.h"
#include "protos.h"

#define SERIAL_BUFSIZE 16384  /* default size */
///
/// external variables
extern struct ExecBase *SysBase;
extern struct Library  *OwnDevUnitBase;
extern Object *app;
extern Object *win;
extern Object *status_win;
extern struct Config Config;
extern struct MUI_CustomClass  *CL_Online;
extern char connectspeed[];
extern BOOL SerialLocked;
///

/// serial_stopread
VOID serial_stopread(struct Modem *modem)
{
   if(modem->mo_serreq)
   {
      if(!(CheckIO((struct IORequest *)modem->mo_serreq)))
         AbortIO((struct IORequest *)modem->mo_serreq);

      WaitIO((struct IORequest *)modem->mo_serreq);
   }
}

///
/// serial_startread
VOID serial_startread(struct Modem *modem, STRPTR data, LONG len)
{
   if(modem->mo_serreq)
   {
      modem->mo_serreq->IOSer.io_Command  = CMD_READ;
      modem->mo_serreq->IOSer.io_Length   = len;
      modem->mo_serreq->IOSer.io_Data     = data;

      SendIO((struct IORequest *)modem->mo_serreq);
   }
}

///
/// serial_send
VOID serial_send(struct Modem *modem, STRPTR cmd, LONG len)
{
   if(modem->mo_serreq && *cmd)
   {
      if(len < 0)
         len = strlen(cmd);

      modem->mo_serreq->IOSer.io_Length  = len;
      modem->mo_serreq->IOSer.io_Command = CMD_WRITE;
      modem->mo_serreq->IOSer.io_Data    = cmd;
      DoIO((struct IORequest *)modem->mo_serreq);
   }
}

///
/// serial_waitfor
BOOL serial_waitfor(struct Modem *modem, STRPTR string, int secs)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   ULONG sig;
   struct timerequest *time_req; // have to open our own timer, global one is for main task. won't work if used in different task
   struct MsgPort *time_port;
   char ser_buf[5], buffer[1024];
   int buf_pos = 0;
   BOOL timer_running = FALSE, terminal_visible;

   terminal_visible = ((DoMainMethod(status_win, MUIM_Genesis_Get, (APTR)MUIA_Window_Open, NULL, NULL) != NULL) && (Config.cnf_flags & CFL_ShowSerialInput));

   if(time_port = CreateMsgPort())
   {
      if(time_req = (struct timerequest *)CreateIORequest(time_port, sizeof(struct timerequest)))
      {
         if(!(OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)time_req, 0)))
         {
            time_req->tr_node.io_Command   = TR_ADDREQUEST;
            time_req->tr_time.tv_secs      = secs;
            time_req->tr_time.tv_micro     = NULL;
            SetSignal(0, 1L << time_port->mp_SigBit);
            SendIO((struct IORequest *)time_req);
            timer_running = TRUE;

            serial_startread(modem, ser_buf, 1);
            while(!data->abort)
            {
               sig = Wait((1L << modem->mo_serport->mp_SigBit) | (1L<< time_port->mp_SigBit) | SIGBREAKF_CTRL_C);
               if(sig & (1L << modem->mo_serport->mp_SigBit))
               {
                  if(CheckIO((struct IORequest *)modem->mo_serreq))
                  {
                     WaitIO((struct IORequest *)modem->mo_serreq);

                     buffer[buf_pos++] = ser_buf[0];
                     buffer[buf_pos] = NULL;

                     if(!data->abort && terminal_visible)
                        DoMainMethod(data->TR_Terminal, TCM_WRITE, ser_buf, (APTR)1, NULL);

                     if(strstr(buffer, string))
                     {
                        // find out connection speed and copy to config
                        if(!strcmp(string, modem->mo_connect))
                        {
                           if(ser_buf[0] == '\r')
                           {
                              strncpy(connectspeed, buffer, 40);
                              connectspeed[strlen(connectspeed) - 1] = NULL;
                              break;
                           }
                        }
                        else
                           break;
                     }

                     if(ser_buf[0] == '\r' || ser_buf[0] == '\n' || ser_buf[0] == NULL || buf_pos > 1020)
                     {
                        if(strstr(buffer, modem->mo_nocarrier) || strstr(buffer, modem->mo_nodialtone) || strstr(buffer, modem->mo_busy))
                           break;
                        buf_pos = 0;
                     }
                     serial_startread(modem, ser_buf, 1);
                  }
               }
               if(sig & (1L << time_port->mp_SigBit))
               {
                  if(CheckIO((struct IORequest *)time_req))
                  {
                     WaitIO((struct IORequest *)time_req);
                     timer_running = FALSE;
                     break;
                  }
               }
               if(sig & SIGBREAKF_CTRL_C)
                  break;
            }
            serial_stopread(modem);

            if(timer_running)
            {
               if(!CheckIO((struct IORequest *)time_req))
               {
                  AbortIO((struct IORequest *)time_req);
                  WaitIO((struct IORequest *)time_req);
               }
               timer_running = FALSE;
            }
            CloseDevice((struct IORequest *)time_req);
         }
         DeleteIORequest((struct IORequest *)time_req);
      }
      DeleteMsgPort(time_port);
   }
   return((BOOL)(strstr(buffer, string) ? TRUE : FALSE));
}

///
/// serial_readln
BOOL serial_readln(struct Modem *modem, STRPTR buffer, ULONG buf_len, int secs, BOOL echo)
{
   ULONG sig;
   struct timerequest *time_req; // have to open our own timer, global one is for main task. won't work if used in different task
   struct MsgPort *time_port;
   char ser_buf[3];
   int buf_pos = 0;
   BOOL timer_running = FALSE, success = FALSE;

   if(time_port = CreateMsgPort())
   {
      if(time_req = (struct timerequest *)CreateIORequest(time_port, sizeof(struct timerequest)))
      {
         if(!(OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)time_req, 0)))
         {
            time_req->tr_node.io_Command   = TR_ADDREQUEST;
            time_req->tr_time.tv_secs      = secs;
            time_req->tr_time.tv_micro     = NULL;
            SetSignal(0, 1L << time_port->mp_SigBit);
            SendIO((struct IORequest *)time_req);
            timer_running = TRUE;

            serial_startread(modem, ser_buf, 1);
            FOREVER
            {
               sig = Wait((1L << modem->mo_serport->mp_SigBit) | (1L<< time_port->mp_SigBit) | SIGBREAKF_CTRL_C);
               if(sig & (1L << modem->mo_serport->mp_SigBit))
               {
                  if(CheckIO((struct IORequest *)modem->mo_serreq))
                  {
                     WaitIO((struct IORequest *)modem->mo_serreq);

                     buffer[buf_pos++] = ser_buf[0];
                     buffer[buf_pos] = NULL;

                     if(echo)
                        serial_send(modem, ser_buf, 1);

                     if(ser_buf[0] == '\r' || ser_buf[0] == '\n' || ser_buf[0] == NULL || buf_pos > buf_len - 2)
                     {
                        success = TRUE;
                        break;
                     }
                     serial_startread(modem, ser_buf, 1);
                  }
               }
               if(sig & (1L << time_port->mp_SigBit))
               {
                  if(CheckIO((struct IORequest *)time_req))
                  {
                     WaitIO((struct IORequest *)time_req);
                     timer_running = FALSE;
                     break;
                  }
               }
               if(sig & SIGBREAKF_CTRL_C)
                  break;
            }
            serial_stopread(modem);

            if(timer_running)
            {
               if(!CheckIO((struct IORequest *)time_req))
               {
                  AbortIO((struct IORequest *)time_req);
                  WaitIO((struct IORequest *)time_req);
               }
               timer_running = FALSE;
            }
            CloseDevice((struct IORequest *)time_req);
         }
         DeleteIORequest((struct IORequest *)time_req);
      }
      DeleteMsgPort(time_port);
   }
   return(success);
}

///
/// serial_carrier
BOOL serial_carrier(struct Modem *modem)
{
   ULONG CD = 1<<5;

   if(!modem->mo_serreq)
      return(FALSE);
   modem->mo_serreq->IOSer.io_Command = SDCMD_QUERY;
   DoIO((struct IORequest *)modem->mo_serreq);
   return((BOOL)(CD & modem->mo_serreq->io_Status ? FALSE : TRUE));
}

///
/// serial_dsr
BOOL serial_dsr(struct Modem *modem)
{
   ULONG DSR = 1<<3;

   if(!modem->mo_serreq)
      return(FALSE);
   modem->mo_serreq->IOSer.io_Command = SDCMD_QUERY;
   DoIO((struct IORequest *)modem->mo_serreq);
   return((BOOL)(DSR & modem->mo_serreq->io_Status ? FALSE : TRUE));
}

///
/// serial_hangup
VOID serial_hangup(struct Modem *modem, struct Library *SocketBase)
{
   if(modem)
   {
      if(serial_create(modem, SocketBase))
      {
         if(serial_carrier(modem))
         {
            serial_send(modem, "+", 1);
            Delay(modem->mo_commanddelay);
            serial_send(modem, "+", 1);
            Delay(modem->mo_commanddelay);
            serial_send(modem, "+", 1);
            Delay(100);
            serial_send(modem, "ATH0\r", -1);
            Delay(modem->mo_commanddelay);
         }
         else
         {
            serial_send(modem, "\r", 1);
            Delay(modem->mo_commanddelay);
         }

         serial_delete(modem);
      }
   }
}

///
/// serial_clear
VOID serial_clear(struct Modem *modem)
{
   if(modem->mo_serreq)
   {
      serial_stopread(modem);

      modem->mo_serreq->IOSer.io_Command = CMD_CLEAR;
      DoIO((struct IORequest *)modem->mo_serreq);
   }
}

///

/// serial_delete
VOID serial_delete(struct Modem *modem)
{
   struct Device *dev;

   if(modem->mo_serreq && modem->mo_serreq->IOSer.io_Device)
   {
      if(!CheckIO((struct IORequest *)modem->mo_serreq))
      {
         AbortIO((struct IORequest *)modem->mo_serreq);
         WaitIO((struct IORequest *)modem->mo_serreq);
      }
      CloseDevice((struct IORequest *)modem->mo_serreq);
   }
   if(modem->mo_serreq)    DeleteExtIO((struct IORequest *)modem->mo_serreq);
   if(modem->mo_serport)   DeleteMsgPort(modem->mo_serport);

   modem->mo_serreq  = NULL;
   modem->mo_serport = NULL;

   if((modem->mo_flags & MFL_SerialLocked) && OwnDevUnitBase && *modem->mo_device)
   {
      FreeDevUnit(modem->mo_device, modem->mo_unit);
      modem->mo_flags &= ~MFL_SerialLocked;
   }

   Forbid();
   if(dev = (struct Device *)FindName(&SysBase->DeviceList, modem->mo_device))
      RemDevice(dev);
   Permit();
}

///
/// serial_create
BOOL serial_create(struct Modem *modem, struct Library *SocketBase)
{
   ULONG flags;

   if(!modem || modem->mo_serreq)
      return(FALSE);

   flags = SERF_SHARED;
   if(modem->mo_flags & MFL_7Wire)
      flags |= SERF_7WIRE;
   if(!(modem->mo_flags & MFL_XonXoff))
      flags |= SERF_XDISABLED;
   if(modem->mo_flags & MFL_RadBoogie)
      flags |= SERF_RAD_BOOGIE;

   if((modem->mo_flags & MFL_OwnDevUnit) && OwnDevUnitBase)
   {
      if(!AttemptDevUnit(modem->mo_device, modem->mo_unit, "GENESiS", NULL))
         modem->mo_flags |= MFL_SerialLocked;
      else
         return(FALSE);
   }

   if(modem->mo_serport = CreateMsgPort())
   {
      if(modem->mo_serreq = (struct IOExtSer *)CreateExtIO(modem->mo_serport, sizeof(struct IOExtSer)))
      {
         // set flags before OpenDevice()
         modem->mo_serreq->io_SerFlags = flags;
   
         if(!OpenDevice(modem->mo_device, modem->mo_unit, (struct IORequest *)modem->mo_serreq, 0))
         {
            // Set up our serial parameters
            modem->mo_serreq->IOSer.io_Command   = SDCMD_SETPARAMS;
            modem->mo_serreq->io_Baud      = modem->mo_baudrate;
            modem->mo_serreq->io_RBufLen   = (modem->mo_serbuflen < 512 ? 512 : modem->mo_serbuflen);
            modem->mo_serreq->io_ReadLen   = 8;
            modem->mo_serreq->io_WriteLen  = 8;
            modem->mo_serreq->io_StopBits  = 1;
            modem->mo_serreq->io_SerFlags  = flags;
            modem->mo_serreq->io_TermArray.TermArray0 =
            modem->mo_serreq->io_TermArray.TermArray1 = 0;

            if(!DoIO((struct IORequest *)modem->mo_serreq))
               return(TRUE);
            else
               syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_SerialSetparamsFailed));
         }
      }
   }
   syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_SYSLOG_CouldNotOpenSerialDevice), modem->mo_device, modem->mo_unit);

   serial_delete(modem);

   return(FALSE);
}

///

