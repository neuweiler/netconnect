/// includes & defines
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Online.h"
#include "protos.h"

#define SERIAL_BUFSIZE 16384  /* default size */

   /* Definitions to access the line signal setting functions supported by
    * some IO serial boards (namely the ASDG board).
    */

#define SIOCMD_SETCTRLLINES   (CMD_NONSTD + 7)
#define SIOB_RTSB    0
#define SIOB_DTRB    1
#define SIOB_RTSF    (1L << SIOB_RTSB)
#define SIOB_DTRF    (1L << SIOB_DTRB)

///
/// external variables
extern struct IOExtSer      *SerReadReq, *SerWriteReq;
extern struct MsgPort       *SerReadPort, *SerWritePort;
extern Object *app, *win, *status_win;
extern struct Config Config;
extern struct MUI_CustomClass  *CL_Online;
extern struct ExecBase *SysBase;
extern char serial_in[], serial_buffer[];
extern struct Modem Modem;

///

/// serial_stopread
VOID serial_stopread(VOID)
{
   if(SerReadReq)
   {
      if(!(CheckIO((struct IORequest *)SerReadReq)))
         AbortIO((struct IORequest *)SerReadReq);

      WaitIO((struct IORequest *)SerReadReq);
   }
}

///
/// serial_startread
VOID serial_startread(STRPTR data, LONG len)
{
   if(SerReadReq)
   {
      SerReadReq->IOSer.io_Command  = CMD_READ;
      SerReadReq->IOSer.io_Length   = len;
      SerReadReq->IOSer.io_Data     = data;

      SetSignal(0, 1L << SerReadPort->mp_SigBit);

      SendIO((struct IORequest *)SerReadReq);
   }
}

///
/// serial_send
VOID serial_send(STRPTR cmd, LONG len)
{
   if(SerWriteReq && *cmd)
   {
      if(len < 0)
         len = strlen(cmd);

      SerWriteReq->IOSer.io_Length  = len;
      SerWriteReq->IOSer.io_Command = CMD_WRITE;
      SerWriteReq->IOSer.io_Data    = cmd;

      DoIO((struct IORequest *)SerWriteReq);
   }
}

///
/// serial_waitfor
int serial_waitfor(STRPTR string1, STRPTR string2, STRPTR string3, int secs)
{
   ULONG sig;
   struct timerequest *time_req; // have to open our own timer, global one is for main task. won't work if used in different task
   struct MsgPort *time_port;
   int buf_pos = 0, found = 0;
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

            serial_startread(serial_in, 1);
            FOREVER
            {
               sig = Wait((1L << SerReadPort->mp_SigBit) | (1L<< time_port->mp_SigBit) | SIGBREAKF_CTRL_C);
               if(sig & (1L << SerReadPort->mp_SigBit))
               {
                  if(CheckIO((struct IORequest *)SerReadReq))
                  {
                     WaitIO((struct IORequest *)SerReadReq);

                     serial_buffer[buf_pos++] = serial_in[0];
                     serial_buffer[buf_pos] = NULL;
                     if(buf_pos > 1020)
                        buf_pos = 0;

                     if(string1)
                        if(strstr(serial_buffer, string1))
                           found = 1;
                     if(string2)
                        if(strstr(serial_buffer, string2))
                           found = 2;
                     if(string3)
                        if(strstr(serial_buffer, string3))
                           found = 3;
                     if(found)
                        break;
                     serial_startread(serial_in, 1);
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
   return(found);
}

///
/// serial_carrier
BOOL serial_carrier(VOID)
{
   ULONG CD = 1<<5;

   if(!SerWriteReq)
      return(FALSE);
   SerWriteReq->IOSer.io_Command = SDCMD_QUERY;
   DoIO((struct IORequest *)SerWriteReq);
   return((BOOL)(CD & SerWriteReq->io_Status ? FALSE : TRUE));
}

///
/// serial_dsr
BOOL serial_dsr(VOID)
{
   ULONG DSR = 1<<3;

   if(!SerWriteReq)
      return(FALSE);
   SerWriteReq->IOSer.io_Command = SDCMD_QUERY;
   DoIO((struct IORequest *)SerWriteReq);
   return((BOOL)(DSR & SerWriteReq->io_Status ? FALSE : TRUE));
}

///

/// serial_delete
VOID serial_delete(VOID)
{
   struct Device *dev;

   if(SerReadReq && SerReadReq->IOSer.io_Device)
   {
      if(!(CheckIO((struct IORequest *)SerReadReq)))
      {
         AbortIO((struct IORequest *)SerReadReq);
         WaitIO((struct IORequest *)SerReadReq);
      }
      CloseDevice((struct IORequest *)(struct IORequest *)SerReadReq);
   }

   if(SerReadReq)    DeleteExtIO((struct IORequest *)SerReadReq);
   if(SerWriteReq)   DeleteExtIO((struct IORequest *)SerWriteReq);
   if(SerReadPort)   DeleteMsgPort(SerReadPort);
   if(SerWritePort)  DeleteMsgPort(SerWritePort);

   SerReadReq = SerWriteReq = NULL;
   SerReadPort = SerWritePort = NULL;

   Forbid();
   if(dev = (struct Device *)FindName(&SysBase->DeviceList, Modem.mo_device))
      RemDevice(dev);
   Permit();
}

///
/// serial_create
BOOL serial_create(STRPTR device, LONG unit)
{
   ULONG flags;

   flags = SERF_SHARED;
   if(Modem.mo_flags & MFL_7Wire)
      flags |= SERF_7WIRE;
   if(!(Modem.mo_flags & MFL_XonXoff))
      flags |= SERF_XDISABLED;
   if(Modem.mo_flags & MFL_RadBoogie)
      flags |= SERF_RAD_BOOGIE;

   SerReadPort = CreateMsgPort();
   SerWritePort = CreateMsgPort();

   if(SerReadPort && SerWritePort)
   {
      SerReadReq = (struct IOExtSer *)CreateExtIO(SerReadPort, sizeof(struct IOExtSer));
      SerWriteReq = (struct IOExtSer *)CreateExtIO(SerWritePort, sizeof(struct IOExtSer));

      if(SerReadReq && SerWriteReq)
      {
         /* set flags before OpenDevice() */
         SerWriteReq->io_SerFlags = flags;

         if(!OpenDevice(device, unit, (struct IORequest *)SerWriteReq, 0))
         {
            /* Set up our serial parameters */
            SerWriteReq->IOSer.io_Command   = SDCMD_SETPARAMS;
            SerWriteReq->io_Baud      = (Modem.mo_baudrate < 300 ? 38400 : Modem.mo_baudrate);
            SerWriteReq->io_RBufLen   = (Modem.mo_serbuflen < 512 ? 512 : Modem.mo_serbuflen);
            SerWriteReq->io_ReadLen   = 8;
            SerWriteReq->io_WriteLen  = 8;
            SerWriteReq->io_StopBits  = 1;
            SerWriteReq->io_SerFlags  = flags;
            SerWriteReq->io_TermArray.TermArray0 =
            SerWriteReq->io_TermArray.TermArray1 = 0;

            DoIO((struct IORequest *)SerWriteReq);

            memcpy(SerReadReq, SerWriteReq, sizeof(struct IOExtSer));
            SerReadReq->IOSer.io_Message.mn_ReplyPort = SerReadPort;
            return(TRUE);
         }
      }
   }
   serial_delete();
   return(FALSE);
}

///
/// serial_hangup
VOID serial_hangup(VOID)
{
   if(SerWriteReq)
   {
      SerWriteReq->IOSer.io_Command  = SIOCMD_SETCTRLLINES;
      SerWriteReq->IOSer.io_Offset   = SIOB_DTRF;
      SerWriteReq->IOSer.io_Length   = 0;

      if(!DoIO((struct IORequest *)SerWriteReq))
      {
         Delay(50);

         SerWriteReq->IOSer.io_Command  = SIOCMD_SETCTRLLINES;
         SerWriteReq->IOSer.io_Offset   = SIOB_DTRF;
         SerWriteReq->IOSer.io_Length   = SIOB_DTRF;

         DoIO((struct IORequest *)SerWriteReq);
      }
      else
      {
         serial_delete();
         Delay(50);
         serial_create(Modem.mo_device, Modem.mo_unit);
      }
   }

   if(serial_carrier())
   {
      serial_send("+", 1);
      Delay(Modem.mo_commanddelay);
      serial_send("+", 1);
      Delay(Modem.mo_commanddelay);
      serial_send("+", 1);
      Delay(50);
      serial_send("ATH0\r", -1);
   }
   else
      serial_send("\r", 1);
}

///

