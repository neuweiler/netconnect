/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Online.h"
#include "protos.h"

///
/// external variables
extern struct Config Config;
extern struct MUI_CustomClass  *CL_Online;
extern struct Library *NetConnectBase, *GenesisBase;

///

/// DialinHandler
SAVEDS ASM VOID DialinHandler(register __a0 STRPTR args, register __d0 LONG arg_len)
{
   struct Library *SocketBase;
   struct Interface *iface;
   struct Modem *modem;
   ULONG sigs = NULL;
   int buf_pos;
   char serial_in[3], serial_buffer[MAXPATHLEN], user[45], password[45];

   if(!(iface = (struct Interface *)atol(args)))
      return;

   if(SocketBase = NCL_OpenSocket())
   {
      if(modem = find_modem_by_id(&Config.cnf_modems, iface->if_modemid))
      {
         if(serial_create(modem, SocketBase))
         {
            buf_pos = 0;
            Delay(modem->mo_commanddelay);
            serial_send(modem, "AT\r", -1);
            Delay(modem->mo_commanddelay);
            EscapeString(serial_buffer, modem->mo_init);
            serial_send(modem, serial_buffer, -1);
            serial_startread(modem, serial_in, 1);
            FOREVER
            {
               sigs = Wait(SIGBREAKF_CTRL_C | (1L << modem->mo_serport->mp_SigBit));

               if(sigs & SIGBREAKF_CTRL_C)
                  break;
               if(sigs & (1L << modem->mo_serport->mp_SigBit))
               {
                  if(CheckIO((struct IORequest *)modem->mo_serreq))
                  {
                     WaitIO((struct IORequest *)modem->mo_serreq);

                     serial_buffer[buf_pos++] = serial_in[0];
                     serial_buffer[buf_pos] = NULL;
Printf("serial: %lc\n", serial_in[0]);
                     if(serial_in[0] == '\r' || serial_in[0] == '\n' || serial_in[0] == NULL || buf_pos > (sizeof(serial_buffer) - 2))
                     {
                        if(strstr(serial_buffer, modem->mo_connect))
                        {
                           if(GetFileSize("AmiTCP:db/motd.dialin") > 0)
                           {
                              BPTR fh;

                              if(fh = Open("AmiTCP:db/motd.dialin", MODE_OLDFILE))
                              {
                                 while(FGets(fh, serial_buffer, sizeof(serial_buffer)))
                                 {
                                    strncat(serial_buffer, "\r", sizeof(serial_buffer));
                                    serial_send(modem, serial_buffer, -1);
                                 }
                                 Close(fh);
                              }
                           }
                           Delay(modem->mo_commanddelay);
                           serial_clear(modem);
                           serial_send(modem, "\r\nUsername: ", -1);
                           if(serial_readln(modem, user, sizeof(user), 30, TRUE))
                           {
Printf("username: %ls\n", user);
                              serial_send(modem, "\r\nPassword: ", -1);
                              if(serial_readln(modem, password, sizeof(password), 30, FALSE))
                              {
Printf("password: %ls\n", password);
//                                 if(password match user)
                                 {
                                    syslog_AmiTCP(SocketBase, LOG_NOTICE, "user %ls has logged in on %ls", user, iface->if_name);
                                    sprintf(serial_buffer, "\r\n\r\nYour IP address is %ls\r\nDestination IP is %ls , MTU is %ld\r\n", iface->if_dst, iface->if_addr, iface->if_MTU);
                                    serial_send(modem, serial_buffer, -1);

//                                    iface->if_flags |= IFL_PutOnline;
//                                    DoMainMethod(win, MUIM_MainWindow_PutOnline, NULL, NULL, NULL);


                                    while(serial_carrier(modem))
                                       Delay(250);

                                    Delay(50);
                                    EscapeString(serial_buffer, modem->mo_init);
                                    serial_send(modem, serial_buffer, -1);
                                 }
                              }
                           }

                           // look at serial_hangup() first before changing
                           serial_delete(modem);
                           if(modem->mo_flags & MFL_DropDTR)
                              Delay(50);
                           else
                              serial_hangup(modem, SocketBase);
                           if(!(serial_create(modem, SocketBase)))
                              break;
                        }
                        if(strstr(serial_buffer, modem->mo_nocarrier))
                        {
                           EscapeString(serial_buffer, modem->mo_init);
                           serial_send(modem, serial_buffer, -1);
                        }
                        if(strstr(serial_buffer, modem->mo_ring))
                        {
                           EscapeString(serial_buffer, modem->mo_answer);
                           serial_send(modem, serial_buffer, -1);
                        }

                        buf_pos = 0;
                     }
                     serial_startread(modem, serial_in, 1);
                  }
               }
            }
            serial_delete(modem);
         }
      }
      CloseLibrary(SocketBase);
   }

   iface->if_dialin = NULL;
}

///
