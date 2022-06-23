/// includes & defines
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "protos.h"
#include "/genesis.lib/pragmas/genesislogger_lib.h"
#include "/genesis.lib/proto/genesislogger.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_Online.h"
#include "mui_Led.h"
#include "iface.h"
#include "sana.h"

#define ioctl IoctlSocket
#define IFR iface_data->ifd_ifr
#define IFRA iface_data->ifd_ifra
#define FD  iface_data->ifd_fd

#define LOOPBACK_NAME "lo0"
#define LOOPBACK_ADDR 0x7f000001
///
/// external variables
extern int errno;
extern Object *status_win;
extern struct Library *GenesisLoggerBase;
extern struct MUI_CustomClass *CL_Online;
extern struct MUI_CustomClass *CL_MainWindow;
extern struct Config Config;
extern Object *app;
extern Object *win;
extern BOOL dialup;

///

/// route_add
int route_add(struct Library *SocketBase, int fd, short flags, u_long from, u_long to, BOOL force)
{
   struct ortentry rte;

   bzero(&rte, sizeof(struct ortentry));
   rte.rt_flags = flags;

   rte.rt_dst.sa_family = AF_INET;
   rte.rt_dst.sa_len    = sizeof(rte.rt_dst);
   ((struct sockaddr_in *)&rte.rt_dst)->sin_addr.s_addr = from;

   rte.rt_gateway.sa_family = AF_INET;
   rte.rt_gateway.sa_len    = sizeof(rte.rt_gateway);
   ((struct sockaddr_in *)&rte.rt_gateway)->sin_addr.s_addr = to;

again:
   if(ioctl(fd, SIOCADDRT, (char *)&rte) < 0)
   {
      if(errno == EEXIST && force)
      {
         force = 0; /* only try once again */
         if(ioctl(fd, SIOCDELRT, (char *)&rte) < 0)
         {
            syslog_AmiTCP(SocketBase, LOG_ERR, "route_add: SIOCDELRT: %m");
            return(errno);
         }
         goto again;
      }
      else
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "route_add: SIOCADDRT: %m");
         return(errno);
      }
   }
   return(0);
}

///
/// route_delete
int route_delete(struct Library *SocketBase, int fd, u_long from)
{
   struct ortentry rte = { 0 };

   rte.rt_dst.sa_family = AF_INET;
   rte.rt_dst.sa_len    = sizeof(rte.rt_dst);
   ((struct sockaddr_in *)&rte.rt_dst)->sin_addr.s_addr = from;

   /* not sure if these are necessary */
   rte.rt_gateway.sa_family = AF_INET;
   rte.rt_gateway.sa_len = sizeof(rte.rt_gateway);

   if(ioctl(fd, SIOCDELRT, (char *)&rte) < 0)
   {
      if(errno != ESRCH) /* requested route not found */
         syslog_AmiTCP(SocketBase, LOG_ERR, "route_delete: SIOCDELRT: %m");

      return(errno);
   }
   return(0);
}

///
/// encrypt_ppp_secret
#define NUM_BITS 6
#define BITMASK 0x3f
#define cput(c)  (*dest++ = (c & BITMASK) + 0x30)
VOID encrypt_ppp_secret(UBYTE *dest, UBYTE *src)
{
   ULONG rnd, c, val, c_cnt;
   ULONG bits = 0, bval = 0;
   UBYTE *s;

   for(rnd = 0, s = src; *s; rnd += *s++)
      ;
   rnd = (rnd % 327) & 0xff;

   cput(rnd >> 4);
   cput(rnd & 0xf);

   c = 1;
   c_cnt = 0;

   do
   {
      if(c)
         c = *src++;
      if(c == 0)
         c_cnt++;
      val = c ^ rnd;

      bval = (bval << 8) | val;
      cput(bval >> (bits + 8 - NUM_BITS));
      bits += (8 - NUM_BITS);
      bval &= (0xff >> (8 - bits));
      if(bits >= NUM_BITS)
      {
         cput(bval >> (bits - NUM_BITS));
         bits -= NUM_BITS;
         bval &= (0xff >> (8 - bits));
      }

      rnd = (val + 17) & 0xff;
   } while(c_cnt != 2);
   *dest = 0;
}

///

/// iface_alloc
struct Interface_Data *iface_alloc(struct Library *SocketBase)
{
   struct Interface_Data *iface_data;
 
   if(iface_data = AllocVec(sizeof(struct Interface_Data), MEMF_ANY | MEMF_CLEAR))
   {
      iface_data->ifd_fd = socket(AF_INET, SOCK_DGRAM, 0);
      if(iface_data->ifd_fd < 0)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "iface_alloc: socket(): %m");
         iface_free(SocketBase, iface_data);
         return(NULL);
      }
   }
   return(iface_data);
}

///
/// iface_free
VOID iface_free(struct Library *SocketBase, struct Interface_Data *iface_data)
{
   if(iface_data->ifd_fd >= 0)
      CloseSocket(iface_data->ifd_fd);
   FreeVec(iface_data);
}

///
/// tagfilter
static struct TagItem tagfilter[] = {
  IF_AutoConfig, TRUE,
  TAG_DONE, 0
};

///
/// iface_runscript
BOOL iface_runscript(struct Interface *iface, struct Modem *modem)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   struct ScriptLine *script_line;
   char buf[256], number_buf[101];
   STRPTR number_ptr, ptr;
   BOOL success = FALSE, ok;
   int dial_tries;

   if(iface->if_loginscript.mlh_TailPred == (struct MinNode *)&iface->if_loginscript)
      return(TRUE);

   script_line = (struct ScriptLine *)iface->if_loginscript.mlh_Head;
   while(script_line->sl_node.mln_Succ && !success)
   {
      if(data->abort)
         return(FALSE);

      switch(script_line->sl_command)
      {
         case SL_Send:
            EscapeString(buf, script_line->sl_contents);
            strcat(buf, "\r");
            serial_send(modem, buf, -1);
            break;

         case SL_WaitFor:
            serial_waitfor(modem, script_line->sl_contents, 20);
            break;

         case SL_Dial:
            dial_tries = 1;
            do
            {
               strncpy(number_buf, iface->if_phonenumber, 100);
               number_ptr = number_buf;
               FOREVER
               {
                  if(ptr = strchr(number_ptr, '|'))
                     *ptr = NULL;

                  sprintf(buf, GetStr(MSG_TX_Dialing), number_ptr, dial_tries, modem->mo_redialattempts);
                  if(data->abort)   return(FALSE);
                  DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, buf, NULL);
                  if(data->abort)   return(FALSE);

                  serial_send(modem, "\r", -1);
                  Delay(modem->mo_commanddelay);
                  serial_send(modem, "AT\r", -1);
                  if(!serial_waitfor(modem, modem->mo_ok, 1))
                  {
                     if(data->abort)
                        return(FALSE);
                     Delay(modem->mo_commanddelay);
                     serial_send(modem, "AT\r", -1);
                     serial_waitfor(modem, modem->mo_ok, 1);
                  }
                  Delay(modem->mo_commanddelay);
                  if(data->abort)
                     return(FALSE);

                  if(*modem->mo_init)
                  {
                     EscapeString(buf, modem->mo_init);
                     serial_send(modem, buf, -1);
                     serial_waitfor(modem, modem->mo_ok, 3);
                     Delay(modem->mo_commanddelay);
                  }
                  if(data->abort)
                     return(FALSE);

                  EscapeString(buf, modem->mo_dialprefix);
                  strcat(buf, number_ptr);
                  EscapeString(&buf[strlen(buf)], modem->mo_dialsuffix);
                  serial_send(modem, buf, -1);
                  if(ok = serial_waitfor(modem, modem->mo_connect, 90))
                  {
                     if(GenesisLoggerBase)
                        iface->if_loggerhandle = GL_StartLogger(number_ptr);
                     break;
                  }
                  Delay(modem->mo_commanddelay);
                  if(data->abort)
                     return(FALSE);

                  if(ptr)
                  {
                     number_ptr = ptr + 1;
                     Delay(100);
                     if(data->abort)
                        return(FALSE);
                  }
                  else
                     break;
               }

               if(!ok && dial_tries < modem->mo_redialattempts)
               {
                  int dly = modem->mo_redialdelay;

                  while(dly-- > 0)
                  {
                     sprintf(buf, GetStr(MSG_TX_Waiting), dly + 1);
                     if(data->abort)   return(FALSE);
                     DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, buf, NULL);
                     if(data->abort)   return(FALSE);
                     Delay(49);
                     if(data->abort)
                        return(FALSE);
                  }
               }
            } while(!ok && (dial_tries++ < modem->mo_redialattempts));
            if(!ok)
               return(FALSE);
            if(!data->abort)
               DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConnectedToProvider), NULL);
            break;

         case SL_GoOnline:
         {
            struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

            if(data->abort)   return(FALSE);
            DoMainMethod(mw_data->GR_Led[(int)iface->if_userdata], MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Orange, NULL);
            if(data->abort)   return(FALSE);
            DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_EstablishingNetworkConnection), NULL);
            success = TRUE;
            break;
         }

         case SL_SendLogin:
            sprintf(buf, "%ls\r", iface->if_login);
            serial_send(modem, buf, -1);
            break;

         case SL_SendPassword:
            sprintf(buf, "%ls\r", iface->if_password);
            serial_send(modem, buf, -1);
            break;

         case SL_SendBreak:
            modem->mo_serreq->IOSer.io_Command = SDCMD_BREAK;
            DoIO((struct IORequest *)modem->mo_serreq);
            break;

         case SL_Pause:
            Delay(atol(script_line->sl_contents) * 50);
            break;
      }
      script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
   }
   return(success);
}

///
/// iface_init
BOOL iface_init(struct Library *SocketBase, struct Interface_Data *iface_data, struct Interface *iface, struct Config *conf)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);

   if(*iface->if_name)
   {
      // check the name & copy it
      if(strlen(iface->if_name) >= IFNAMSIZ)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_IfaceNameTooLong));
         return(FALSE);
      }
   }
   else
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_NoIfaceName));
      return(FALSE);
   }

   if(iface->if_flags & IFL_IPDynamic)
      *iface->if_addr = NULL;
   if(iface->if_flags & IFL_DSTDynamic)
      *iface->if_dst = NULL;
   if(iface->if_flags & IFL_GWDynamic)
      *iface->if_gateway = NULL;
   if(iface->if_flags & IFL_NMDynamic)
      *iface->if_netmask = NULL;

   if(is_inaddr_any(SocketBase, iface->if_addr))
      iface->if_flags |= IFL_IPDynamic;
   if(is_inaddr_any(SocketBase, iface->if_dst))
      iface->if_flags |= IFL_DSTDynamic;
   if(is_inaddr_any(SocketBase, iface->if_gateway))
      iface->if_flags |= IFL_GWDynamic;
   if(is_inaddr_any(SocketBase, iface->if_netmask))
      iface->if_flags |= IFL_NMDynamic;

   strcpy(iface_data->ifd_name, iface->if_name);
   // clear pointers to resources we possibly own
   iface_data->ifd_s2 = NULL;

   if(strcmp(iface->if_name, LOOPBACK_NAME))   // lo0 is not in the database
   {
      // Create the Sana2 configuration file, if present on the configuration
      if(*iface->if_sana2config && iface->if_sana2configtext)
      {
         BPTR fh;
         STRPTR ptr;

         // Create ENV:Sana2 if necessary
         if(fh = CreateDir("ENV:Sana2"))
            UnLock(fh);
         if(fh = Open(iface->if_sana2config, MODE_NEWFILE))
         {
            if(ptr = iface->if_sana2configtext)
            {
               while(*ptr)
               {
                  switch(*ptr)
                  {
                     case '%':
                        ptr++;
                        switch(*ptr)
                        {
                           case 'a':
                              FPrintf(fh, "%ls", (is_inaddr_any(SocketBase, iface->if_addr) ? "0.0.0.0" : iface->if_addr));
                              break;
                           case 'u':
                              FPrintf(fh, "%ls", iface->if_login);
                              break;
                           case 'p':
                              if((iface->if_flags & IFL_PPP) && strstr(iface->if_sana2configtext, "secret_secret"))
                              {
                                 char buf[81];

                                 encrypt_ppp_secret(buf, iface->if_password);
                                 FPrintf(fh, "%ls", buf);
                              }
                              else
                                 FPrintf(fh, "%ls", iface->if_password);
                              break;
                           default:
                              FPrintf(fh, "%%%lc", *ptr);
                              break;
                        }
                        break;

                     default:
                        FPrintf(fh, "%lc", *ptr);
                        break;
                  }
                  ptr++;
               }
            }
            Close(fh);
         }
         else
            syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_SYSLOG_CouldNotCreateSana2Config), iface->if_sana2config);
      }

      if(data->abort)   goto fail;

      if(!(iface_data->ifd_s2 = sana2_create(iface->if_sana2device, iface->if_sana2unit)))
      {
         syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_SYSLOG_CouldNotOpenSana2Device), "iface_init", iface->if_sana2device, iface->if_sana2unit);
         return(FALSE);
      }
      
      // query device information
      if(sana2_devicequery(iface_data->ifd_s2) == FALSE)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_CouldNotGetDeviceInfo), iface_data->ifd_s2->s2_name, iface_data->ifd_s2->s2_unit);
         goto fail;
      }
      // Get Sana-II hardware type
      iface_data->ifd_sanatype = iface_data->ifd_s2->s2_hwtype;

      if(data->abort)   goto fail;

      // Determine whether the interface operates over the serial line
      if(iface->if_modemid > 0)
      {
         struct Modem *modem;

         if(!iface_offline(SocketBase, iface_data))
            goto fail;
         if(data->abort)   goto fail;

         if(modem = find_modem_by_id(&Config.cnf_modems, iface->if_modemid))
         {
            if(modem->mo_serreq)
            {
               syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_ModemAlreadyInUse), iface->if_name);
               goto fail;
            }
            if(serial_create(modem, SocketBase))
            {
               Delay(modem->mo_commanddelay);

               if(!(modem->mo_flags & MFL_IgnoreDSR) && !serial_dsr(modem))
                  syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_NoDSRSignal), modem->mo_name);
               else
               {
                  if(iface_runscript(iface, modem))
                  {
                     if(!data->abort)
                     {
                        if(!iface_online(SocketBase, iface_data))
                        {
                           serial_delete(modem);
                           goto fail;
                        }
                     }
                  }
               }
               serial_delete(modem);
            }

            if(data->abort)   goto fail;

            // Query the hardware addresses of the Sana-II device
            if(!sana2_getaddresses(SocketBase, iface_data->ifd_s2, iface))
               syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_SYSLOG_GetAddressesFailed), iface->if_name);
         }
         else
            syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_ModemIDNotFound), iface->if_name, iface->if_modemid);
      }
      else
         iface_online(SocketBase, iface_data);

      // we have no local or destination address => do icmp info req before closing sana2
      if((is_inaddr_any(SocketBase, iface->if_dst) && is_inaddr_any(SocketBase, iface->if_gateway)) || is_inaddr_any(SocketBase, iface->if_addr))
          sana2_do_icmp(SocketBase, iface_data->ifd_s2, iface);


       // The Sana2 device needs to be closed here (before AmiTCP gets to know
       // about the device) because of the bugs of the original CBM slip-driver
       // & derivatives.

      iface_close_sana2(iface_data);
   }

   if(data->abort)   goto fail;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConfiguringInterface), NULL);
   if(data->abort)   goto fail;

   // get the interface address
   if(iface_getaddr(SocketBase, iface_data, &iface_data->ifd_addr))
      goto fail;
   // Fetch the interface flags
   if(iface_getflags(SocketBase, iface_data, &iface_data->ifd_flags))
      goto fail;
   if(iface_getlinkinfo(SocketBase, iface_data))
      goto fail;

   return(TRUE);

fail:
   iface_deinit(iface_data);
   return(FALSE);
}

///
/// iface_deinit
VOID iface_deinit(struct Interface_Data *iface_data)
{
   iface_close_sana2(iface_data);
}

///
/// iface_close_sana2
VOID iface_close_sana2(struct Interface_Data *iface_data)
{
   if(iface_data->ifd_s2 != NULL)
   {
      sana2_delete(iface_data->ifd_s2);
      iface_data->ifd_s2 = NULL;
   }
}

///
/// iface_prepare_bootp
BOOL iface_prepare_bootp(struct Library *SocketBase, struct Interface_Data *iface_data, struct Interface *iface, struct Config *conf)
{
   LONG on = 1;


   // Make sure that interface has addresses defined.  Don't use the current
   // address of the interface, since it might be obsolete. If no address is
   // given via arguments or Sana-II configuration, then use INADDR_ANY
   // (0.0.0.0). It is ok for our own IP to be INADDR_ANY at this stage, if no
   // other value is known.
   //
   // Additionally, on BROADCAST interfaces the netmask must be set to 0.0.0.0,
   // and the broadcast address to 255.255.255.255, if our IP address is
   // currently INADDR_ANY.  The socket used for BOOTP must also be marked to
   // allow broadcasting and to bypass routing.
   //
   // Additionally, on p-to-p interfaces the IP and destination IP addresses
   // need to be different.  We must also add a temporary route for the
   // INADDR_BROADCAST address, since broadcasting is not really supported on
   // p-to-p interfaces.
   //

   if(iface_setaddr(SocketBase, iface_data, inet_addr(iface->if_addr)))
      return(FALSE);

   // Mark the socket to allow broadcasting. Note that this is now done for point-to-point interfaces as well.
   if(setsockopt(iface_data->ifd_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0)
      syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_BootpErrorSetSockOpt));
   else
      syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_BootpAllowBroadcasts));

   if(iface_data->ifd_flags & IFF_BROADCAST)
   {
      if(is_inaddr_any(SocketBase, iface->if_addr) || !is_inaddr_any(SocketBase, iface->if_netmask))
      {
         // Set up the netmask if IP is 0.0.0.0 or mask is explicitly given.
         if(iface_setnetmask(SocketBase, iface_data, inet_addr(iface->if_netmask)))
            return(FALSE);
      }

      if(is_inaddr_any(SocketBase, iface->if_addr) || !is_inaddr_any(SocketBase, iface->if_dst))
      {
         ULONG addr;

         // Set up the broadcast address if IP is 0.0.0.0 or broadcast address is explicitly given.
         if(inet_addr(iface->if_dst) == INADDR_ANY)
            addr = INADDR_BROADCAST;
         else
            addr = inet_addr(iface->if_dst);
      
         if(iface_setdstaddr(SocketBase, iface_data, addr))
            return(FALSE);
      }
   }

   if(iface_data->ifd_flags & IFF_POINTOPOINT)
   {
      ULONG addr;

      if(is_inaddr_any(SocketBase, iface->if_dst))
         addr = 0x00010203;      // fake address
      else
         addr = inet_addr(iface->if_dst);
      if(iface_data->ifd_addr == addr) // must be different
         addr++;

      // set up the destination address
      if(iface_setdstaddr(SocketBase, iface_data, addr))
         return(FALSE);

      // DialUp version does not need this nor can ad routes
      if(!dialup)
      {
         // Route "broadcasts" to the point-to-point destination
         syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_AddBroadcastRoute));
         if(route_add(SocketBase, FD, RTF_UP | RTF_GATEWAY|RTF_HOST, INADDR_BROADCAST, addr, TRUE))
            return(FALSE);
      }
   }


   // Since the Sana2 hw-type is wider (4 bytes) than used in BOOTP (1 byte),
   // the hardware types above 255 must be ignored. This is done by forcing
   // down the cnf_use_hwtype field from the config info.
   // Also, if we know that the hw-address is not usable for BOOTP, we'll
   // force not using it. This is the case for PPP, SLIP and CSLIP and other
   // point-to-point interfaces.

   if(iface_data->ifd_flags & IFF_POINTOPOINT)
      iface_data->ifd_use_hwtype = FALSE;
   else if(iface_data->ifd_htype >= 256)
      iface_data->ifd_use_hwtype = FALSE;

   return(TRUE);
}

///
/// iface_cleanup_bootp
VOID iface_cleanup_bootp(struct Library *SocketBase, struct Interface_Data *iface_data, struct Config *conf)
{
   if(iface_data->ifd_flags & IFF_POINTOPOINT)
   {
      // DialUp version does not need this nor can ad routes
      if(!dialup)
      {
         // delete the added "broadcasts" route
         syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_DeleteBroadcastRoute));
         route_delete(SocketBase, iface_data->ifd_fd, INADDR_BROADCAST);
      }
   }
}

///
/// iface_online
BOOL iface_online(struct Library *SocketBase, struct Interface_Data *iface_data)
{
   if(sana2_online(SocketBase, iface_data->ifd_s2) == FALSE)
      return(FALSE);

   return(TRUE);
}

///
/// iface_offline
BOOL iface_offline(struct Library *SocketBase, struct Interface_Data *iface_data)
{
   if(sana2_offline(SocketBase, iface_data->ifd_s2) == FALSE)
      return(FALSE);

   return(TRUE);
}

///
/// iface_config
BOOL iface_config(struct Library *SocketBase, struct Interface_Data *iface_data, struct Interface *iface, struct Config *conf)
{
   // Delete the old configuration
   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family = AF_INET;
   IFR.ifr_addr.sa_len = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = iface_data->ifd_addr;

   // delete IF addr
   if(ioctl(FD, SIOCDIFADDR, (char *)&IFR) < 0)
   {
      if(errno != EADDRNOTAVAIL)
      {
         // no previous address ?
         syslog_AmiTCP(SocketBase, LOG_ERR, "iface_config: SIOCDIFADDR: %m");
         return(FALSE);
      }
   }

   // Build request to add new config
   // address
   memset(&IFRA.ifra_addr, 0, sizeof(IFRA.ifra_addr));
   IFRA.ifra_addr.sa_family   = AF_INET;
   IFRA.ifra_addr.sa_len      = sizeof(IFRA.ifra_addr);
   ((struct sockaddr_in *)&IFRA.ifra_addr)->sin_addr.s_addr = inet_addr(iface->if_addr);

   // destination/broadcast address
   memset(&IFRA.ifra_broadaddr, 0, sizeof(IFRA.ifra_broadaddr));
   if(!is_inaddr_any(SocketBase, iface->if_dst))
   {
      IFRA.ifra_broadaddr.sa_family = AF_INET;
      IFRA.ifra_broadaddr.sa_len    = sizeof(IFRA.ifra_broadaddr);
      ((struct sockaddr_in *)&IFRA.ifra_broadaddr)->sin_addr.s_addr = inet_addr(iface->if_dst);
   }

   // netmask
   memset(&IFRA.ifra_mask, 0, sizeof(IFRA.ifra_mask));
   if(!is_inaddr_any(SocketBase, iface->if_netmask))
   {
      IFRA.ifra_mask.sa_family   = 0; // no family for masks
      IFRA.ifra_mask.sa_len      = sizeof(IFRA.ifra_mask);
      ((struct sockaddr_in *)&IFRA.ifra_mask)->sin_addr.s_addr = inet_addr(iface->if_netmask);
   }

   // add IF alias
   if(ioctl(FD, SIOCAIFADDR, (char *)&IFRA) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_config: SIOCAIFADDR: %m");
      return(FALSE);
   }
  
   // Set the interface MTU
   if(iface->if_MTU > 0)
      iface_setmtu(SocketBase, iface_data, iface->if_MTU);

   return(TRUE);
}

///

/// iface_getaddr
int iface_getaddr(struct Library *SocketBase, struct Interface_Data *iface_data, u_long *addr)
{
   if(ioctl(FD, SIOCGIFADDR, (char *)&IFR) < 0)
   {
      if (errno != EADDRNOTAVAIL)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "iface_getaddr: SIOCGIFADDR: %m");
         return(errno);
      }
      *addr = INADDR_ANY;
      return(0);
   }
   *addr = ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr;
   return(0);
}

///
/// iface_setaddr
int iface_setaddr(struct Library *SocketBase, struct Interface_Data *iface_data, u_long addr)
{
   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family  = AF_INET;
   IFR.ifr_addr.sa_len     = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = addr;

   if(ioctl(FD, SIOCSIFADDR, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_setaddr: SIOCSIFADDR: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getdstaddr
int iface_getdstaddr(struct Library *SocketBase, struct Interface_Data *iface_data, u_long *addr)
{
   // get interface dst/broadcast address
   u_long request;

   if(iface_data->ifd_flags & IFF_POINTOPOINT)
      request = SIOCGIFDSTADDR;
   else if(iface_data->ifd_flags & IFF_BROADCAST)
      request = SIOCGIFBRDADDR;
   else
   {
      // no destination address of any kind available
      *addr = INADDR_ANY;
      return(0);
   }

   if(ioctl(FD, request, (char *)&IFR) < 0)
   {
      if(errno != EADDRNOTAVAIL)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "iface_getdstaddr: SIOCGIF(DST/BRD)ADDR: %m");
         return(errno);
      }
      *addr = INADDR_ANY;
      return(0);
   }
   *addr = ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr;
   return(0);
}

///
/// iface_setdstaddr
int iface_setdstaddr(struct Library *SocketBase, struct Interface_Data *iface_data, u_long addr)
{
   u_long request;

   if(iface_data->ifd_flags & IFF_POINTOPOINT)
      request = SIOCSIFDSTADDR;
   else if(iface_data->ifd_flags & IFF_BROADCAST)
      request = SIOCSIFBRDADDR;
   else
      return(0);

   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family  = AF_INET;
   IFR.ifr_addr.sa_len     = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = addr;

   if(ioctl(FD, request, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_setdstaddr: SIOCGIF(DST/BRD)ADDR: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getnetmask
int iface_getnetmask(struct Library *SocketBase, struct Interface_Data *iface_data, u_long *addr)
{
   if(!((*(BYTE *)((struct Library *)SocketBase + 1)) & 0x02))
      return(187);

   if(ioctl(FD, SIOCGIFNETMASK, (char *)&IFR) < 0)
   {
      if(errno != EADDRNOTAVAIL)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "iface_getnetmask: SIOCGIFNETMASK: %m");
         return(errno);
      }
      *addr = INADDR_ANY;
      return(0);
   }
   *addr = ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr;
   return(0);
}

///
/// iface_setnetmask
int iface_setnetmask(struct Library *SocketBase, struct Interface_Data *iface_data, u_long addr)
{
   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family  = AF_INET;
   IFR.ifr_addr.sa_len     = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = addr;

   if(ioctl(FD, SIOCSIFNETMASK, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_setnetmask: SIOCSIFNETMASK: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getflags
int iface_getflags(struct Library *SocketBase, struct Interface_Data *iface_data, short *flags)
{
   if(ioctl(FD, SIOCGIFFLAGS, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_getflags: SIOCGIFFLAGS: %m");
      return(errno);
   }
   *flags = IFR.ifr_flags;
   return(0);
}

///
/// iface_setflags
int iface_setflags(struct Library *SocketBase, struct Interface_Data *iface_data, short flags)
{
   IFR.ifr_flags = flags;

   if(ioctl(FD, SIOCSIFFLAGS, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_setflags: SIOCSIFFLAGS: %m");
      return errno;
   }
   return(0);
}

///
/// iface_getmtu
int iface_getmtu(struct Library *SocketBase, struct Interface_Data *iface_data, int *mtu)
{
   if(ioctl(FD, SIOCGIFMTU, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_getmtu: SIOCGIFMTU: %m");
      return(errno);
   }

   *mtu = IFR.ifr_mtu;
   return(0);
}

///
/// iface_setmtu
int iface_setmtu(struct Library *SocketBase, struct Interface_Data *iface_data, int mtu)
{
   IFR.ifr_mtu = mtu;

   if(ioctl(FD, SIOCSIFMTU, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_setmtu: SIOCSIFMTU: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getlinkinfo
int iface_getlinkinfo(struct Library *SocketBase, struct Interface_Data *iface_data)
{
   register int n;
   struct ifreq ibuf[16];
   struct ifconf ifc;
   register struct ifreq *ifrp, *ifend;

   // Fetch the interface configuration to find the link level address
   ifc.ifc_len = sizeof(ibuf);
   ifc.ifc_buf = (caddr_t) ibuf;
   if(ioctl(FD, SIOCGIFCONF, (char *) &ifc) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "iface_getlinkinfo: SIOCGIFCONF: %m");
      return(errno);
   }

   if(ifc.ifc_len < sizeof(struct ifreq))
   {
      syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_SYSLOG_NoLinkLevelInfo));
      return(ENOENT);
   }

   // Search interface configuration list for link layer address.
   ifrp = ibuf;
   ifend = (struct ifreq *) ((char *) ibuf + ifc.ifc_len);
   while(ifrp < ifend)
   {
      // Look for interface
      if(strcmp(iface_data->ifd_name, ifrp->ifr_name) == 0 && ifrp->ifr_addr.sa_family == AF_LINK)
      {
         struct sockaddr_dl * dl_addr = (struct sockaddr_dl *)&ifrp->ifr_addr;

         iface_data->ifd_htype   = dl_addr->sdl_type;
         iface_data->ifd_hlen    = dl_addr->sdl_alen; // hwaddr len
         memcpy(iface_data->ifd_haddr, LLADDR(dl_addr), iface_data->ifd_hlen);
      
         return(0);
      }
      // Bump interface config pointer
      n = ifrp->ifr_addr.sa_len + sizeof(ifrp->ifr_name);
      if(n < sizeof(*ifrp))
         n = sizeof(*ifrp);
      ifrp = (struct ifreq *) ((char *) ifrp + n);
   }
   return(ENOENT); // link level address not found
}

///

