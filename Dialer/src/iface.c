/// includes & defines
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "protos.h"
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
extern struct MUI_CustomClass *CL_Online;
extern struct MUI_CustomClass *CL_MainWindow;
extern struct IOExtSer *SerWriteReq;
extern Object *app;
extern Object *win;
extern BOOL dialup, use_reconnect;

///

/// route_add
int route_add(int fd, short flags, u_long from, u_long to, BOOL force)
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
         force = 0; /* try only once again */
         if(ioctl(fd, SIOCDELRT, (char *)&rte) < 0)
         {
            syslog_AmiTCP(LOG_ERR, "route_add: SIOCDELRT: %m");
            return(errno);
         }
         goto again;
      }
      else
      {
         syslog_AmiTCP(LOG_ERR, "route_add: SIOCADDRT: %m");
         return(errno);
      }
   }
   return(0);
}

///
/// route_delete
int route_delete(int fd, u_long from)
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
      {
         syslog_AmiTCP(LOG_ERR, "route_delete: SIOCDELRT: %m");
      }
      return(errno);
   }
   return(0);
}

///

/// iface_alloc
struct Interface_Data *iface_alloc(VOID)
{
   struct Interface_Data *iface_data;
 
   if(iface_data = AllocVec(sizeof(struct Interface_Data), MEMF_ANY | MEMF_CLEAR))
   {
      iface_data->ifd_fd = socket(AF_INET, SOCK_DGRAM, 0);
      if(iface_data->ifd_fd < 0)
      {
         syslog_AmiTCP(LOG_ERR, "iface_alloc: socket(): %m");
         iface_free(iface_data);
         return(NULL);
      }
   }
   return(iface_data);
}

///
/// iface_free
VOID iface_free(struct Interface_Data *iface_data)
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
BOOL iface_runscript(struct Interface_Data *iface_data, struct Interface *iface, struct ISP *isp, struct Config *conf)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   struct ScriptLine *script_line;
   char buf[256], number_buf[101];
   STRPTR number_ptr, ptr;
   BOOL success = FALSE, ok;
   int dial_tries;

   if(isp->isp_loginscript.mlh_TailPred == (struct MinNode *)&isp->isp_loginscript)
      return(TRUE);

   script_line = (struct ScriptLine *)isp->isp_loginscript.mlh_Head;
   while(script_line->sl_node.mln_Succ && !success)
   {
      if(data->abort)
         return(FALSE);

      if(script_line->sl_command == SL_Send)
      {
         EscapeString(buf, script_line->sl_contents);
         strcat(buf, "\r");
         serial_send(buf, -1);
      }
      else if(script_line->sl_command == SL_WaitFor)
      {
         serial_waitfor(script_line->sl_contents, 20);
      }
      else if(script_line->sl_command == SL_Dial)
      {
         dial_tries = 1;
         do
         {
            strncpy(number_buf, isp->isp_phonenumber, 100);
            number_ptr = number_buf;
            FOREVER
            {
               if(ptr = strchr(number_ptr, '|'))
                  *ptr = NULL;

               sprintf(buf, GetStr(MSG_TX_Dialing), number_ptr, dial_tries, conf->cnf_redialattempts);
               if(data->abort)   return(FALSE);
               DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, buf, NULL);
               if(data->abort)   return(FALSE);

               serial_send("\r", -1);
               Delay(10);
               serial_send("AAT\r", -1);
               serial_waitfor("OK", 1);
               Delay(10);
               if(data->abort)
                  return(FALSE);

               if(*conf->cnf_initstring)
               {
                  EscapeString(buf, conf->cnf_initstring);
                  strncat(buf, "\r", sizeof(buf) - strlen(buf));
                  serial_send(buf, -1);
                  serial_waitfor("OK", 3);
                  Delay(10);
               }
               if(data->abort)
                  return(FALSE);

               sprintf(buf, "%ls%ls%ls\r", conf->cnf_dialprefix, number_ptr, conf->cnf_dialsuffix);
               serial_send(buf, -1);
               ok = serial_waitfor("CONNECT", 90);
               Delay(10);
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

            if(!ok && dial_tries < conf->cnf_redialattempts)
            {
               int dly = conf->cnf_redialdelay;

               while(dly-- > 0)
               {
                  sprintf(buf, GetStr(MSG_TX_Waiting), dly + 1);
                  if(data->abort)   return(FALSE);
                  DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, buf, NULL);
                  if(data->abort)   return(FALSE);
                  Delay(48);
                  if(data->abort)
                     return(FALSE);
               }
            }
         } while(!ok && (dial_tries++ < conf->cnf_redialattempts));
         if(!ok)
            return(FALSE);
         if(!data->abort)
            DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConnectedToProvider), NULL);
      }
      else if(script_line->sl_command == SL_GoOnline)
      {
         struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

         if(data->abort)   return(FALSE);
         DoMainMethod(mw_data->GR_Led[(int)iface->if_userdata], MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Orange, NULL);
         if(data->abort)   return(FALSE);
         DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_EstablishingNetworkConnection), NULL);
         success = TRUE;
      }
      else if(script_line->sl_command == SL_SendLogin)
      {
         sprintf(buf, "%ls\r", isp->isp_login);
         serial_send(buf, -1);
      }
      else if(script_line->sl_command == SL_SendPassword)
      {
         sprintf(buf, "%ls\r", isp->isp_password);
         serial_send(buf, -1);
      }
      else if(script_line->sl_command == SL_SendBreak)
      {
         SerWriteReq->IOSer.io_Command = SDCMD_BREAK;
         DoIO((struct IORequest *)SerWriteReq);
      }
      else if(script_line->sl_command == SL_Pause)
         Delay(atol(script_line->sl_contents) * 50);
      script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
   }
   return(success);
}

///
/// iface_init
BOOL iface_init(struct Interface_Data *iface_data, struct Interface *iface, struct ISP *isp, struct Config *conf)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);

   if(*iface->if_name)
   {
      // check the name & copy it
      if(strlen(iface->if_name) >= IFNAMSIZ)
      {
         syslog_AmiTCP(LOG_ERR, "iface_init: interface name too long, max is 15 characters");
         return(FALSE);
      }
   }
   else
   {
      syslog_AmiTCP(LOG_ERR, "iface_init: no infterface name given");
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

   if(!*iface->if_addr)
      iface->if_flags |= IFL_IPDynamic;
   if(!*iface->if_dst)
      iface->if_flags |= IFL_DSTDynamic;
   if(!*iface->if_gateway)
      iface->if_flags |= IFL_GWDynamic;
   if(!*iface->if_netmask)
      iface->if_flags |= IFL_NMDynamic;

   strcpy(iface_data->ifd_name, iface->if_name);
   // clear pointers to resources we possibly own
   iface_data->ifd_s2 = NULL;
   iface_data->ifd_gateway = inet_addr(iface->if_gateway);

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
            ptr = iface->if_sana2configtext;
            while(*ptr)
            {
               switch(*ptr)
               {
                  case '%':
                     ptr++;
                     switch(*ptr)
                     {
                        case 'a':
                           FPrintf(fh, "%ls", (*iface->if_addr ? iface->if_addr : "0.0.0.0"));
                           break;
                        case 'u':
                           FPrintf(fh, "%ls", isp->isp_login);
                           break;
                        case 'p':
                           FPrintf(fh, "%ls", isp->isp_password);
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
            Close(fh);
         }
         else
            syslog_AmiTCP(LOG_CRIT, "iface_init: could not open %ls for writing!", iface->if_sana2config);
      }

      if(data->abort)   goto fail;

      if(!(iface_data->ifd_s2 = sana2_create(iface->if_sana2device, iface->if_sana2unit)))
      {
         syslog_AmiTCP(LOG_CRIT, "iface_init: could not open %ls unit %ld.", iface->if_sana2device, iface->if_sana2unit);
         return(FALSE);
      }
      
      // query device information
      if(sana2_devicequery(iface_data->ifd_s2) == FALSE)
      {
         syslog_AmiTCP(LOG_ERR, "iface_init: could not get device information of %ls (unit %ld).", iface_data->ifd_s2->s2_name, iface_data->ifd_s2->s2_unit);
         goto fail;
      }
      // Get Sana-II hardware type
      iface_data->ifd_sanatype = iface_data->ifd_s2->s2_hwtype;

      if(data->abort)   goto fail;

      // Determine whether the interface operates over the serial line
      if(iface_data->ifd_s2->s2_hwtype == S2WireType_PPP ||
         iface_data->ifd_s2->s2_hwtype == S2WireType_SLIP ||
         iface_data->ifd_s2->s2_hwtype == S2WireType_CSLIP)
      {
         if(!use_reconnect && (isp->isp_loginscript.mlh_TailPred != (struct MinNode *)&isp->isp_loginscript))
         {
            // dial with given dialscript

            if(!iface_offline(iface_data))
               goto fail;

            if(data->abort)   goto fail;

            if(!serial_create())
               goto fail;

            if(!conf->cnf_flags & CFL_IgnoreDSR)
            {
               if(!serial_dsr())
               {
                  syslog_AmiTCP(LOG_ERR, "iface_init: no DSR signal present.");
                  goto fail;
               }
            }

            if(data->abort)   goto fail;

            if(!iface_runscript(iface_data, iface, isp, conf))
               goto fail;
         }
         if(data->abort)   goto fail;

         if(!iface_online(iface_data))
            goto fail;

         if(data->abort)   goto fail;

         // Query the hardware addresses of the Sana-II device
         if(!sana2_getaddresses(iface_data->ifd_s2, iface))
            syslog_AmiTCP(LOG_WARNING, "iface_init: sana2_getaddresses() failed");
      }
      else
         iface_online(iface_data);

       // The Sana2 device needs to be closed here (before AmiTCP gets to know
       // about the device) because of the bugs of the original CBM slip-driver
       // & derivatives.

      iface_close_sana2(iface_data);
   }

   if(data->abort)   goto fail;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConfiguringInterface), NULL);
   if(data->abort)   goto fail;

   // get the interface address
   if(iface_getaddr(iface_data, &iface_data->ifd_addr))
      goto fail;
   // Fetch the interface flags
   if(iface_getflags(iface_data, &iface_data->ifd_flags))
      goto fail;
   // get interface dst/broadcast address
   if(iface_getdstaddr(iface_data, &iface_data->ifd_dst))
      goto fail;
   // get interface netmask
   if(iface_getnetmask(iface_data, &iface_data->ifd_netmask))
      goto fail;
   // get interface mtu
   if(iface_getmtu(iface_data, (int *)&iface_data->ifd_MTU))
      goto fail;
   if(iface_getlinkinfo(iface_data))
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
   serial_delete();
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
BOOL iface_prepare_bootp(struct Interface_Data *iface_data, struct Interface *iface, struct Config *conf)
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

   if(iface_setaddr(iface_data, inet_addr(iface->if_addr)))
      return(FALSE);
   iface_data->ifd_addr = inet_addr(iface->if_addr);
   iface_getdstaddr(iface_data, &iface_data->ifd_dst);      // These may also have changed,
   iface_getnetmask(iface_data, &iface_data->ifd_netmask);  //   when the address is changed

   // Mark the socket to allow broadcasting. Note that this is now done for point-to-point interfaces as well.
   if(setsockopt(iface_data->ifd_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0)
      syslog_AmiTCP(LOG_ERR, "iface_prepare_bootp: setsockopt(SO_BROADCAST): %m");
   else
      syslog_AmiTCP(LOG_DEBUG, "iface_prepare_bootp: set socket to allow broadcasts.");

   if(iface_data->ifd_flags & IFF_BROADCAST)
   {
      if(!*iface->if_addr || *iface->if_netmask)
      {
         // Set up the netmask if IP is 0.0.0.0 or mask is explicitly given.
         if(iface_setnetmask(iface_data, inet_addr(iface->if_netmask)))
         return(FALSE);
      }

      if(!*iface->if_addr || *iface->if_dst)
      {
         ULONG addr;

         // Set up the broadcast address if IP is 0.0.0.0 or broadcast address is explicitly given.
         if(inet_addr(iface->if_dst) == INADDR_ANY)
            addr = INADDR_BROADCAST;
         else
            addr = inet_addr(iface->if_dst);
      
         if(iface_setdstaddr(iface_data, addr))
            return FALSE;
      }
   }

   if(iface_data->ifd_flags & IFF_POINTOPOINT)
   {
      ULONG addr;

      if(!*iface->if_dst)
      {
         if(iface_data->ifd_dst == INADDR_ANY)
            addr = 0x00010203;      // fake address
         else
            addr = iface_data->ifd_dst;

         if(iface_data->ifd_addr == addr) // must be different
            addr++;
      }
      else
         addr = inet_addr(iface->if_dst);   // is different due to argument constraints

      // set up the destination address
      if(iface_setdstaddr(iface_data, addr))
         return(FALSE);
      iface_data->ifd_dst = addr;

      // DialUp version does not need this nor can ad routes
      if(!dialup)
      {
         // Route "broadcasts" to the point-to-point destination
         syslog_AmiTCP(LOG_DEBUG, "iface_prepare_bootp: adding route for broadcast address on p-t-p link.");
         if(route_add(FD, RTF_UP | RTF_GATEWAY|RTF_HOST, INADDR_BROADCAST, iface_data->ifd_dst, TRUE))
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
VOID iface_cleanup_bootp(struct Interface_Data *iface_data, struct Config *conf)
{
   if(iface_data->ifd_flags & IFF_POINTOPOINT)
   {
      // DialUp version does not need this nor can ad routes
      if(!dialup)
      {
         // delete the added "broadcasts" route
         syslog_AmiTCP(LOG_DEBUG, "iface_cleanup_bootp: deleting route for broadcast address on p-t-p link");
         route_delete(iface_data->ifd_fd, INADDR_BROADCAST);
      }
   }
}

///
/// iface_online
BOOL iface_online(struct Interface_Data *iface_data)
{
   if(sana2_online(iface_data->ifd_s2) == FALSE)
      return(FALSE);

   return(TRUE);
}

///
/// iface_offline
BOOL iface_offline(struct Interface_Data *iface_data)
{
   if(sana2_offline(iface_data->ifd_s2) == FALSE)
      return(FALSE);

   return(TRUE);
}

///
/// iface_config
BOOL iface_config(struct Interface_Data *iface_data, struct Interface *iface, struct Config *conf)
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
         syslog_AmiTCP(LOG_ERR, "iface_config: SIOCDIFADDR: %m");
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
   if(inet_addr(iface->if_dst) != INADDR_ANY)
   {
      IFRA.ifra_broadaddr.sa_family = AF_INET;
      IFRA.ifra_broadaddr.sa_len    = sizeof(IFRA.ifra_broadaddr);
      ((struct sockaddr_in *)&IFRA.ifra_broadaddr)->sin_addr.s_addr = inet_addr(iface->if_dst);
   }

   // netmask
   memset(&IFRA.ifra_mask, 0, sizeof(IFRA.ifra_mask));
   if(inet_addr(iface->if_netmask) != INADDR_ANY)
   {
      IFRA.ifra_mask.sa_family   = 0; // no family for masks
      IFRA.ifra_mask.sa_len      = sizeof(IFRA.ifra_mask);
      ((struct sockaddr_in *)&IFRA.ifra_mask)->sin_addr.s_addr = inet_addr(iface->if_netmask);
   }

   // add IF alias
   if(ioctl(FD, SIOCAIFADDR, (char *)&IFRA) < 0)
   {
      syslog_AmiTCP(LOG_ERR, "iface_config: SIOCAIFADDR: %m");
      return(FALSE);
   }
  
   // Set the interface MTU if explicitly set
   if(iface->if_MTU > 0 && iface->if_MTU != iface_data->ifd_MTU)
      iface_setmtu(iface_data, iface->if_MTU);

   return(TRUE);
}

///

/// iface_getaddr
int iface_getaddr(struct Interface_Data *iface_data, u_long *addr)
{
   if(ioctl(FD, SIOCGIFADDR, (char *)&IFR) < 0)
   {
      if (errno != EADDRNOTAVAIL)
      {
         syslog_AmiTCP(LOG_ERR, "iface_getaddr: SIOCGIFADDR: %m");
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
int iface_setaddr(struct Interface_Data *iface_data, u_long addr)
{
   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family  = AF_INET;
   IFR.ifr_addr.sa_len     = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = addr;

   if(ioctl(FD, SIOCSIFADDR, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(LOG_ERR, "iface_setaddr: SIOCSIFADDR: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getdstaddr
int iface_getdstaddr(struct Interface_Data *iface_data, u_long *addr)
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
         syslog_AmiTCP(LOG_ERR, "iface_getdstaddr: SIOCGIF(DST/BRD)ADDR: %m");
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
int iface_setdstaddr(struct Interface_Data *iface_data, u_long addr)
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
      syslog_AmiTCP(LOG_ERR, "iface_setdstaddr: SIOCGIF(DST/BRD)ADDR: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getnetmask
int iface_getnetmask(struct Interface_Data *iface_data, u_long *addr)
{
   if(!((*(BYTE *)((struct Library *)SocketBase + 1)) & 0x02))
      return(187);

   if(ioctl(FD, SIOCGIFNETMASK, (char *)&IFR) < 0)
   {
      if(errno != EADDRNOTAVAIL)
      {
         syslog_AmiTCP(LOG_ERR, "iface_getnetmask: SIOCGIFNETMASK: %m");
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
int iface_setnetmask(struct Interface_Data *iface_data, u_long addr)
{
   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family  = AF_INET;
   IFR.ifr_addr.sa_len     = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = addr;

   if(ioctl(FD, SIOCSIFNETMASK, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(LOG_ERR, "iface_setnetmask: SIOCSIFNETMASK: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getflags
int iface_getflags(struct Interface_Data *iface_data, short *flags)
{
   if(ioctl(FD, SIOCGIFFLAGS, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(LOG_ERR, "iface_getflags: SIOCGIFFLAGS: %m");
      return(errno);
   }
   *flags = IFR.ifr_flags;
   return(0);
}

///
/// iface_setflags
int iface_setflags(struct Interface_Data *iface_data, short flags)
{
   IFR.ifr_flags = flags;

   if(ioctl(FD, SIOCSIFFLAGS, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(LOG_ERR, "iface_setflags: SIOCSIFFLAGS: %m");
      return errno;
   }
   return(0);
}

///
/// iface_getmtu
int iface_getmtu(struct Interface_Data *iface_data, int *mtu)
{
   if(ioctl(FD, SIOCGIFMTU, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(LOG_ERR, "iface_getmtu: SIOCGIFMTU: %m");
      return(errno);
   }

   *mtu = IFR.ifr_mtu;
   return(0);
}

///
/// iface_setmtu
int iface_setmtu(struct Interface_Data *iface_data, int mtu)
{
   IFR.ifr_mtu = mtu;

   if(ioctl(FD, SIOCSIFMTU, (char *)&IFR) < 0)
   {
      syslog_AmiTCP(LOG_ERR, "iface_setmtu: SIOCSIFMTU: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getlinkinfo
int iface_getlinkinfo(struct Interface_Data *iface_data)
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
      syslog_AmiTCP(LOG_ERR, "iface_getlinkinfo: SIOCGIFCONF: %m");
      return(errno);
   }

   if(ifc.ifc_len < sizeof(struct ifreq))
   {
      syslog_AmiTCP(LOG_WARNING, "iface_getlinkinfo: link level information not available!");
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

