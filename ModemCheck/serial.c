/// includes & defines
#include "/includes.h"
#pragma header

#include "protos.h"

#define SERIAL_BUFSIZE 16384  /* default size */
///
/// external variables
extern struct IOExtSer      *SerReadReq, *SerWriteReq;
extern struct MsgPort       *SerReadPort, *SerWritePort;
extern struct ExecBase *SysBase;

extern char serial_buffer[1024], device[81];

extern struct timerequest *time_req;
extern struct MsgPort *time_port;

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
BOOL serial_waitfor(STRPTR string, int secs)
{
   ULONG sig;
   char ser_buf[5];
   BOOL timer_running = FALSE;
   int ser_buf_pos = 0;

   time_req->tr_node.io_Command   = TR_ADDREQUEST;
   time_req->tr_time.tv_secs      = secs;
   time_req->tr_time.tv_micro     = NULL;
   SetSignal(0, 1L << time_port->mp_SigBit);
   SendIO((struct IORequest *)time_req);
   timer_running = TRUE;

   serial_startread(ser_buf, 1);
   FOREVER
   {
      sig = Wait((1L << SerReadPort->mp_SigBit) | (1L<< time_port->mp_SigBit) | SIGBREAKF_CTRL_C);
      if(sig & (1L << SerReadPort->mp_SigBit))
      {
         if(CheckIO((struct IORequest *)SerReadReq))
         {
            WaitIO((struct IORequest *)SerReadReq);

            serial_buffer[ser_buf_pos++] = ser_buf[0];
            serial_buffer[ser_buf_pos] = NULL;

            if(strstr(serial_buffer, string))
               break;

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

   return((BOOL)strstr(serial_buffer, string));
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
/// serial_clear
VOID serial_clear(VOID)
{
   if(SerReadReq)
   {
      serial_stopread();

      SerReadReq->IOSer.io_Command = CMD_CLEAR;
      DoIO((struct IORequest *)SerReadReq);
   }
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
   if(dev = (struct Device *)FindName(&SysBase->DeviceList, device))
      RemDevice(dev);
   Permit();
}

///
/// serial_create
BOOL serial_create(STRPTR device, LONG unit)
{
   ULONG flags;

   flags = SERF_XDISABLED | SERF_RAD_BOOGIE | SERF_QUEUEDBRK | SERF_SHARED | SERF_7WIRE;

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
            SerWriteReq->io_Baud      = 19200;
            SerWriteReq->io_RBufLen   = SERIAL_BUFSIZE;
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

