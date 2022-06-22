/// includes & defines
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "protos.h"

#define SERIAL_BUFSIZE 16384  /* default size */
///
/// external variables
extern struct IOExtSer      *SerReadReq, *SerWriteReq;
extern struct MsgPort       *SerReadPort, *SerWritePort;
extern Object *app;
extern Object *win;
extern Object *status_win;
extern struct config Config;
extern struct ExecBase *SysBase;
extern struct MUI_CustomClass  *CL_Online;
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
   if(dev = (struct Device *)FindName(&SysBase->DeviceList, Config.cnf_serialdevice))
      RemDevice(dev);
   Permit();
}

///
/// serial_create
BOOL serial_create(STRPTR device, ULONG unit)
{
   ULONG flags = SERF_XDISABLED | SERF_RAD_BOOGIE | SERF_QUEUEDBRK | SERF_SHARED | SERF_7WIRE;

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
            SerWriteReq->io_Baud      = (Config.cnf_baudrate < 300 ? 38400 : Config.cnf_baudrate);
            SerWriteReq->io_RBufLen   = (Config.cnf_serbuflen < 512 ? 512 : Config.cnf_serbuflen);
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
   serial_delete();
   Delay(70);
   serial_create(Config.cnf_serialdevice, Config.cnf_serialunit);

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

