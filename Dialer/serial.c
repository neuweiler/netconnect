#include "/AmiTCP.h"
#include "globals.c"

#define SERIAL_BUFSIZE 16384  /* default size */

/// serial_stopread
VOID serial_stopread(VOID)
{
   if(SerReq)
   {
      if(!(CheckIO(SerReq)))
         AbortIO(SerReq);

      WaitIO(SerReq);
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

      SendIO(SerReq);
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
      DoIO(SerReq);
   }
}

///
/// serial_waitfor
BOOL serial_waitfor(STRPTR string, int secs)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   ULONG signal;
   char ser_buf[5], buffer[1024];
   int buf_pos = 0;

   TimeReq->tr_node.io_Command   = TR_ADDREQUEST;
   TimeReq->tr_time.tv_secs      = secs;
   TimeReq->tr_time.tv_micro     = NULL;
   SendIO(TimeReq);

   serial_startread(ser_buf, 1);
   FOREVER
   {
      signal = Wait((1L << SerPort->mp_SigBit) | (1L<< TimePort->mp_SigBit) | SIGBREAKF_CTRL_C);

      if(signal & (1L << SerPort->mp_SigBit))
      {
         if(CheckIO(SerReq))
         {
            WaitIO(SerReq);

            buffer[buf_pos++] = ser_buf[0];
            buffer[buf_pos] = NULL;

            DoMainMethod(data->TR_Terminal, TCM_WRITE, ser_buf, (APTR)1, NULL);

            if(strstr(buffer, string))
            {
               /** find out connection speed and copy to config **/
               if(!strcmp(string, "CONNECT"))
               {
                  if(ser_buf[0] == '\r')
                  {
                     STRPTR ptr1, ptr2;

                     ptr1 = buffer;
                     while(!isdigit(*ptr1) && *ptr1)
                        ptr1++;

                     ptr2 = ptr1;
                     while(isdigit(*ptr2))
                        ptr2++;
                     if(ptr2 > ptr1)
                        *ptr2 = NULL;

                     Config.cnf_connectspeed = atol(ptr1);

                     break;
                  }
               }
               else
                  break;
            }

            if(ser_buf[0] == '\r' || ser_buf[0] == '\n' || ser_buf[0] == NULL || buf_pos > 1020)
            {
               if(strstr(buffer, "NO CARRIER") || strstr(buffer, "NO DIALTONE") || strstr(buffer, "NODIALTONE"))
                  break;
               buf_pos = 0;
            }
            serial_startread(ser_buf, 1);
         }
      }
      if(signal & (1L << TimePort->mp_SigBit))
      {
         if(CheckIO(TimeReq))
         {
            WaitIO(TimeReq);
            break;
         }
      }
      if(signal & SIGBREAKF_CTRL_C)
         break;
   }
   serial_stopread();

   if(!CheckIO(TimeReq))
   {
      AbortIO(TimeReq);
      WaitIO(TimeReq);
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
   DoIO(SerReq);
   return((BOOL)(CD & SerReq->io_Status ? FALSE : TRUE));
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

/// serial_create
BOOL serial_create(VOID)
{
   ULONG flags;

   flags = SERF_XDISABLED | SERF_RAD_BOOGIE | SERF_QUEUEDBRK | SERF_SHARED;
   if(Config.cnf_7wire)
      flags |= SERF_7WIRE;

   if(SerPort = CreateMsgPort())
   {
      if(SerReq = (struct IOExtSer *)CreateExtIO(SerPort, sizeof(struct IOExtSer)))
      {
         /* set flags before OpenDevice() */
         SerReq->io_SerFlags = flags;
   
         if(!OpenDevice(Config.cnf_serialdevice, Config.cnf_serialunit, (struct IORequest *)SerReq, 0))
         {
            /* Set up our serial parameters */
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
               DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), "Serial SETPARAMS failed.");
         }
      }
   }
   DoMethod(app, MUIM_Application_PushMethod, win, 5, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), "Could not open serial device (%ls, unit %ld).", Config.cnf_serialdevice, Config.cnf_serialunit);

   serial_delete();

   return(FALSE);
}

///
/// serial_delete
VOID serial_delete(VOID)
{
   if(SerReq)
   {
      if(SerReq->IOSer.io_Device)
         CloseDevice((struct IORequest *)SerReq);
      DeleteExtIO(SerReq);
      SerReq = NULL;
   }
   if(SerPort)
      DeleteMsgPort(SerPort);
   SerPort = NULL;
}

///

