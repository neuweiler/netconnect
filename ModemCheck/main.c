/// includes
#include "/includes.h"
#pragma header

#include "protos.h"

///
/// variables
extern struct ExecBase *SysBase;

IOExtSer      *SerReadReq    = NULL;   /* Serial IORequest */
MsgPort       *SerReadPort   = NULL;   /* Serial reply port */
IOExtSer      *SerWriteReq   = NULL;   /* Serial IORequest */
MsgPort       *SerWritePort  = NULL;   /* Serial reply port */

char serial_buffer[1024], device[81];
WORD unit;

struct timerequest *time_req; // have to open our own timer, global one is for main task. won't work if used in different task
struct MsgPort *time_port;

///

// user should be able to enter serial device and unit in command line

/// main
int main(int argc, char *argv[])
{
   if(argc != 1 && argc != 3)
   {
      Printf("Usage: modemcheck [<device> <unit>]\ne.g. \"modemdetect myser.device 0\"\n\n");
      exit(20);
   }
   strcpy(device, (argc == 3 ? argv[1] : "serial.device"));
   unit = (argc == 3 ? atol(argv[2]) : 0);

   if(time_port = CreateMsgPort())
   {
      if(time_req = (struct timerequest *)CreateIORequest(time_port, sizeof(struct timerequest)))
      {
         if(!(OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)time_req, 0)))
         {
            if(serial_create(device, unit))
            {
               BPTR fh;

               if(fh = Open("RAM:ModemInfo", MODE_NEWFILE))
               {
                  int i = 0;
                  char buf[31];

                  Printf("\nModemcheck will analyze your modem.\nPlease enter the exact name/type of your modem now:\n");
                  gets(serial_buffer);
                  FPrintf(fh, "Modem Type: %ls\n", serial_buffer);
                  FPrintf(fh, "Serial device: %ls %ld\n", device, unit);
                  FPrintf(fh, "DSR present: %ls\n\n", (serial_dsr() ? "yes" : "no"));

                  Printf("Thank you.. please stand by.\n");

                  serial_send("\r", -1);
                  serial_waitfor("OK\r\n", 1);
                  serial_send("AAT\r", -1);
                  serial_waitfor("OK\r\n", 1);

                  while(i < 10)
                  {
                     sprintf(buf, "ATI%ld\r\n", i);
                     serial_send(buf, -1);
                     serial_waitfor("OK\r\n", 1);
                     FPrintf(fh, serial_buffer);
                     i++;
                  }
                  strcpy(buf, "AT#MFR?\r\n");
                  serial_send(buf, -1);
                  serial_waitfor("OK\r\n", 1);
                  FPrintf(fh, serial_buffer);

                  strcpy(buf, "AT+FMI?\r\n");
                  serial_send(buf, -1);
                  serial_waitfor("OK\r\n", 1);
                  FPrintf(fh, serial_buffer);

                  Close(fh);

                  Printf("Finished.\n\nNow please send an email message to dolphin@zool.unizh.ch with the\nsubject 'Modem Check' and attach the just created file 'RAM:ModemInfo'\nto it. Thank you very much for your help !\n\n");
               }
               serial_delete();
            }
            else
               Printf("could not open '%ls', unit %ld\n", device, unit);

            CloseDevice((struct IORequest *)time_req);
         }
         DeleteIORequest((struct IORequest *)time_req);
      }
      DeleteMsgPort(time_port);
   }

}

///

