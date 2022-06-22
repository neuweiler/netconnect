/// includes & defines
#include "/includes.h"
#pragma header

#include <libraries/owndevunit.h>
#include <pragma/owndevunit_lib.h>
#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "protos.h"

#define SERIAL_BUFSIZE 16384  /* default size */
///
/// external variables
extern struct ExecBase *SysBase;
extern struct Library  *OwnDevUnitBase;
extern struct IOExtSer *SerReq;
extern struct MsgPort  *SerPort;
extern Object *app;
extern Object *win;
extern Object *status_win;
extern struct Config Config;
extern struct MUI_CustomClass  *CL_Online;
extern char connectspeed[];
extern BOOL SerialLocked;
///

/// serial_stopread
VOID serial_stopread(VOID)
{
   if(SerReq)
   {
      if(!(CheckIO((struct IORequest *)SerReq)))
         AbortIO((struct IORequest *)SerReq);

      WaitIO((struct IORequest *)SerReq);
   }
}

///
/// serial_startread
VOID serial_startread(STRPTR data, LONG len)
{
   if(SerReq)
   {
      SerReq->IOSer.io_Command  = CMD_READ;
      SerReq->IOSer.io_Length   = len;
      SerReq->IOSer.io_Data     = data;

      SetSignal(0, 1L << SerPort->mp_SigBit);

      SendIO((struct IORequest *)SerReq);
   }
}

///
/// serial_send
VOID serial_send(STRPTR cmd, LONG len)
{
   if(SerReq && *cmd)
   {
      if(len < 0)
         len = strlen(cmd);

      SerReq->IOSer.io_Length  = len;
      SerReq->IOSer.io_Command = CMD_WRITE;
      SerReq->IOSer.io_Data    = cmd;
      DoIO((struct IORequest *)SerReq);
   }
}

///
/// serial_waitfor
BOOL serial_waitfor(STRPTR string, int secs)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   ULONG sig;
   struct timerequest *time_req; // have to open our own timer, global one is for main task. won't work if used in different task
   struct MsgPort *time_port;
   char ser_buf[5], buffer[1024];
   int buf_pos = 0;
   BOOL timer_running = FALSE;

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

            serial_startread(ser_buf, 1);
            while(!data->abort)
            {
               sig = Wait((1L << SerPort->mp_SigBit) | (1L<< time_port->mp_SigBit) | SIGBREAKF_CTRL_C);
               if(sig & (1L << SerPort->mp_SigBit))
               {
                  if(CheckIO((struct IORequest *)SerReq))
                  {
                     WaitIO((struct IORequest *)SerReq);

                     buffer[buf_pos++] = ser_buf[0];
                     buffer[buf_pos] = NULL;

                     if(!data->abort)
                        DoMainMethod(data->TR_Terminal, TCM_WRITE, ser_buf, (APTR)1, NULL);

                     if(strstr(buffer, string))
                     {
                        // find out connection speed and copy to config
                        if(!strcmp(string, "CONNECT"))
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
                        if(strstr(buffer, "NO CARRIER") || strstr(buffer, "NO DIALTONE") || strstr(buffer, "NODIALTONE") || strstr(buffer, "BUSY"))
                           break;
                        buf_pos = 0;
                     }
                     serial_startread(ser_buf, 1);
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
            serial_stopread();

            if(timer_running)
            {
               if(!CheckIO((struct IORequest *)time_req))
                  AbortIO((struct IORequest *)time_req);
               WaitIO((struct IORequest *)time_req);
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
/// serial_carrier
BOOL serial_carrier(VOID)
{
   ULONG CD = 1<<5;

   if(!SerReq)
      return(FALSE);
   SerReq->IOSer.io_Command = SDCMD_QUERY;
   DoIO((struct IORequest *)SerReq);
   return((BOOL)(CD & SerReq->io_Status ? FALSE : TRUE));
}

///
/// serial_dsr
BOOL serial_dsr(VOID)
{
   ULONG DSR = 1<<3;

   if(!SerReq)
      return(FALSE);
   SerReq->IOSer.io_Command = SDCMD_QUERY;
   DoIO((struct IORequest *)SerReq);
   return((BOOL)(DSR & SerReq->io_Status ? FALSE : TRUE));
}

///
/// serial_hangup
VOID serial_hangup(VOID)
{
   if(serial_carrier())
   {
      serial_send("+", 1);
      Delay(20);
      serial_send("+", 1);
      Delay(20);
      serial_send("+", 1);
      Delay(20);
      serial_send("ATH0\r", -1);
   }
   else
      serial_send("\r", 1);
}

///

/// serial_delete
VOID serial_delete(VOID)
{
   struct Device *dev;

   if(SerReq && SerReq->IOSer.io_Device)
   {
      if(!CheckIO((struct IORequest *)SerReq))
      {
         AbortIO((struct IORequest *)SerReq);
         WaitIO((struct IORequest *)SerReq);
      }
      CloseDevice((struct IORequest *)SerReq);
   }
   if(SerReq)
      DeleteExtIO((struct IORequest *)SerReq);
   SerReq = NULL;
   if(SerPort)
      DeleteMsgPort(SerPort);
   SerPort = NULL;

   Forbid();
   if(dev = (struct Device *)FindName(&SysBase->DeviceList, Config.cnf_serialdevice))
      RemDevice(dev);
   Permit();

   if(SerialLocked && OwnDevUnitBase && *Config.cnf_serialdevice)
      FreeDevUnit(Config.cnf_serialdevice, Config.cnf_serialunit);
   SerialLocked = FALSE;
}

///
/// serial_create
BOOL serial_create(VOID)
{
   ULONG flags;

   flags = SERF_SHARED;
   if(Config.cnf_flags & CFL_7Wire)
      flags |= SERF_7WIRE;
   if(!(Config.cnf_flags & CFL_XonXoff))
      flags |= SERF_XDISABLED;
   if(Config.cnf_flags & CFL_RadBoogie)
      flags |= SERF_RAD_BOOGIE;

   if((Config.cnf_flags & CFL_OwnDevUnit) && OwnDevUnitBase)
   {
      if(!AttemptDevUnit(Config.cnf_serialdevice, Config.cnf_serialunit, "Genesis", NULL))
         SerialLocked = TRUE;
      else
         return(FALSE);
   }

   if(SerPort = CreateMsgPort())
   {
      if(SerReq = (struct IOExtSer *)CreateExtIO(SerPort, sizeof(struct IOExtSer)))
      {
         // set flags before OpenDevice()
         SerReq->io_SerFlags = flags;
   
         if(!OpenDevice(Config.cnf_serialdevice, Config.cnf_serialunit, (struct IORequest *)SerReq, 0))
         {
            // Set up our serial parameters
            SerReq->IOSer.io_Command   = SDCMD_SETPARAMS;
            SerReq->io_Baud      = Config.cnf_baudrate;
            SerReq->io_RBufLen   = (Config.cnf_serbuflen < 512 ? 512 : Config.cnf_serbuflen);
            SerReq->io_ReadLen   = 8;
            SerReq->io_WriteLen  = 8;
            SerReq->io_StopBits  = 1;
            SerReq->io_SerFlags  = flags;
            SerReq->io_TermArray.TermArray0 =
            SerReq->io_TermArray.TermArray1 = 0;

            if(!DoIO((struct IORequest *)SerReq))
               return(TRUE);
            else
               syslog_AmiTCP(LOG_ERR, "serial_create: SETPARAMS failed.");
         }
      }
   }
   syslog_AmiTCP(LOG_CRIT, "serial_create: could not open serial device (%ls, unit %ld).", Config.cnf_serialdevice, Config.cnf_serialunit);

   serial_delete();

   return(FALSE);
}

///

