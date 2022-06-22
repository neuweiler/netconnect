/// includes & defines
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "Strings.h"
#include "protos.h"
#include "mui.h"
#include "iface.h"
#include "sana.h"

#define ioctl IoctlSocket
#define IFR iface->iface_ifr
#define IFRA iface->iface_ifra
#define FD  iface->iface_fd

#define LOOPBACK_NAME "lo0"
#define LOOPBACK_ADDR 0x7f000001
///
/// external variables
extern int errno;
extern Object *status_win;
extern struct MUI_CustomClass *CL_Online;
extern struct MinList dialscript;
extern struct IOExtSer *SerReq;
extern Object *app;
extern Object *win;

///

/// iface_alloc
struct iface *iface_alloc(VOID)
{
   struct iface *iface;
 
   if(iface = AllocVec(sizeof(struct iface), MEMF_ANY | MEMF_CLEAR))
   {
      iface->iface_fd = socket(AF_INET, SOCK_DGRAM, 0);
      if(iface->iface_fd < 0)
      {
         syslog(LOG_ERR, "iface_alloc: socket(): %m");
         iface_free(iface);
         return(NULL);
      }
   }
   return(iface);
}

///
/// iface_free
VOID iface_free(struct iface *iface)
{
   if(iface->iface_fd >= 0)
      CloseSocket(iface->iface_fd);
   FreeVec(iface);
}

///
/// tagfilter
static struct TagItem tagfilter[] = {
  IF_AutoConfig, TRUE,
  TAG_DONE, 0
};

///
/// iface_runscript
BOOL iface_runscript(struct iface *iface, struct config * conf)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   struct ScriptLine *script_line;
   char buf[256];
   BOOL success = FALSE, ok;
   int dial_tries;

   if(dialscript.mlh_TailPred == (struct MinNode *)&dialscript)
      return(TRUE);

   script_line = (struct ScriptLine *)dialscript.mlh_Head;
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
            sprintf(buf, "Dialing '%ls'\n(attempt %ld/%ld)", conf->cnf_phonenumber, dial_tries, conf->cnf_redialattempts);
            DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, buf, NULL);

            serial_send("AAT\r", -1);
            serial_waitfor("OK", 1);
            if(data->abort)
               return(FALSE);

            EscapeString(buf, conf->cnf_initstring);
            strncat(buf, "\r", sizeof(buf) - strlen(buf));
            serial_send(buf, -1);
            serial_waitfor("OK", 2);
            if(data->abort)
               return(FALSE);

            sprintf(buf, "%ls%ls%ls\r", conf->cnf_dialprefix, conf->cnf_phonenumber, conf->cnf_dialsuffix);
            serial_send(buf, -1);
            ok = serial_waitfor("CONNECT", 90);
            if(data->abort)
               return(FALSE);

            if(!ok && dial_tries < conf->cnf_redialattempts)
            {
               int dly = conf->cnf_redialdelay;

               while(dly-- > 0)
               {
                  Delay(49);
                  if(data->abort)
                     return(FALSE);
               }
            }
         } while(!ok && (dial_tries++ < conf->cnf_redialattempts));
         if(!ok)
            return(FALSE);
         if(!data->abort)
            DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, "Connected to provider.\nExecuting login script.", NULL);
      }
      else if(script_line->sl_command == SL_GoOnline)
      {
         DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, "Establishing network connection", NULL);
         success = TRUE;
      }
      else if(script_line->sl_command == SL_SendLogin)
      {
         sprintf(buf, "%ls\r", conf->cnf_loginname);
         serial_send(buf, -1);
      }
      else if(script_line->sl_command == SL_SendPassword)
      {
         sprintf(buf, "%ls\r", conf->cnf_password);
         serial_send(buf, -1);
      }
      else if(script_line->sl_command == SL_SendBreak)
      {
         SerReq->IOSer.io_Command = SDCMD_BREAK;
         DoIO((struct IORequest *)SerReq);
      }
      else if(script_line->sl_command == SL_Pause)
         Delay(atol(script_line->sl_contents) * 50);

      script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
   }
   return(success);
}

///
/// iface_init
BOOL iface_init(struct iface *iface, struct config *conf)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   int len;
   const char *ifname;

   ifname = conf->cnf_ifname;

   if(ifname)
   {
      /* check the name & copy it */
      len = strlen(ifname);
      if(len >= IFNAMSIZ)
      {
         syslog(LOG_ERR, "iface_init: interface name too long, max is 15 characters");
         return(FALSE);
      }
   }
   else
   {
      syslog(LOG_ERR, "iface_init: no infterface name given");
      return(FALSE);
   }

   /* Copy the interface name */
   memcpy(iface->iface_name, ifname, len + 1);
   if(ifname != conf->cnf_ifname)   /* to conf too */
      memcpy(conf->cnf_ifname, ifname, len + 1);

   /* clear pointers to resources we possibly own */
   iface->iface_s2 = NULL;

   /* get the AmiTCP configuration entry for the interface */
   if(strcmp(ifname, LOOPBACK_NAME))   /* lo0 is not in the database */
   {
      /* Create the Sana2 configuration file, if present on the configuration */
      if(*conf->cnf_sana2config && conf->cnf_sana2configtext)
      {
         BPTR fh;

         /* Create ENV:Sana2 if necessary */
         if(fh = CreateDir("ENV:Sana2"))
            UnLock(fh);
         if(fh = Open(conf->cnf_sana2config, MODE_NEWFILE))
         {
            Write(fh, conf->cnf_sana2configtext, strlen(conf->cnf_sana2configtext));
            Close(fh);
         }
         else
            syslog(LOG_CRIT, "iface_init: could not open %ls for writing!", conf->cnf_sana2config);
      }

      if(data->abort)   goto fail;

      if(!(iface->iface_s2 = sana2_create(conf)))
      {
         syslog(LOG_CRIT, "iface_init: could not open %ls unit %ld.", conf->cnf_sana2device, conf->cnf_sana2unit);
         return(FALSE);
      }
      
      /* query device information */
      if(sana2_devicequery(iface->iface_s2) == FALSE)
      {
         syslog(LOG_ERR, "iface_init: could not get device information of %ls (unit %ld).", iface->iface_s2->s2_name, iface->iface_s2->s2_unit);
         goto fail;
      }

      if(data->abort)   goto fail;
      
      /* Get Sana-II hardware type */
      iface->iface_sanatype = iface->iface_s2->s2_hwtype;

      /* Only if should be put online AND if hw-type is PPP, SLIP or CSLIP */

      /* Determine whether the interface operates over the serial line */
      if(iface->iface_s2->s2_hwtype == S2WireType_PPP ||
         iface->iface_s2->s2_hwtype == S2WireType_SLIP ||
         iface->iface_s2->s2_hwtype == S2WireType_CSLIP)
      {
         if(dialscript.mlh_TailPred != (struct MinNode *)&dialscript)
         {
            /* dial with given dialscript */

            if(!iface_offline(iface))
               goto fail;

            if(data->abort)   goto fail;

            if(!serial_create())
               goto fail;

            if(data->abort)   goto fail;

            if(!iface_runscript(iface, conf))
               goto fail;
         }
         if(data->abort)   goto fail;

         if(!iface_online(iface))
            goto fail;

         if(data->abort)   goto fail;

         /* Query the hardware addresses of the Sana-II device */
         if(!sana2_getaddresses(iface->iface_s2, conf))
            syslog(LOG_WARNING, "iface_init: sana2_getaddresses() failed");
      }

      /*
       * The Sana2 device needs to be closed here (before AmiTCP gets to know
       * about the device) because of the bugs of the original CBM slip-driver
       * & derivatives.
       */

      iface_close_sana2(iface);
   }

   if(data->abort)   goto fail;

   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, "Configuring the interface", NULL);

   /* get the interface address */
   if(iface_getaddr(iface, &iface->iface_addr))
      goto fail;
   /* Fetch the interface flags */
   if(iface_getflags(iface, &iface->iface_flags))
      goto fail;
   /* get interface dst/broadcast address */
   if(iface_getdstaddr(iface, &iface->iface_dst))
      goto fail;
   /* get interface netmask */
   if(iface_getnetmask(iface, &iface->iface_netmask))
      goto fail;
   /* get interface mtu */
   if(iface_getmtu(iface, (int *)&iface->iface_MTU))
      goto fail;
   if(iface_getlinkinfo(iface))
      goto fail;

   return(TRUE);

fail:
   iface_deinit(iface);
   return(FALSE);
}

///
/// iface_deinit
VOID iface_deinit(struct iface *iface)
{
   serial_delete();
   iface_close_sana2(iface);
}

///
/// iface_close_sana2
VOID iface_close_sana2(struct iface *iface)
{
   if(iface->iface_s2 != NULL)
   {
      sana2_delete(iface->iface_s2);
      iface->iface_s2 = NULL;
   }
}

///
/// iface_prepare_bootp
BOOL iface_prepare_bootp(struct iface *iface, struct config *conf)
{
   LONG on = 1;

  /*
   * Make sure that interface has addresses defined.  Don't use the current
   * address of the interface, since it might be obsolete. If no address is
   * given via arguments or Sana-II configuration, then use INADDR_ANY
   * (0.0.0.0). It is ok for our own IP to be INADDR_ANY at this stage, if no
   * other value is known.
   * 
   * Additionally, on BROADCAST interfaces the netmask must be set to 0.0.0.0,
   * and the broadcast address to 255.255.255.255, if our IP address is
   * currently INADDR_ANY.  The socket used for BOOTP must also be marked to
   * allow broadcasting and to bypass routing.
   *
   * Additionally, on p-to-p interfaces the IP and destination IP addresses
   * need to be different.  We must also add a temporary route for the
   * INADDR_BROADCAST address, since broadcasting is not really supported on
   * p-to-p interfaces.
   */

   if(iface_setaddr(iface, conf->cnf_addr))
      return(FALSE);
   iface->iface_addr = conf->cnf_addr;
   iface_getdstaddr(iface, &iface->iface_dst);      /* These may also have changed, */
   iface_getnetmask(iface, &iface->iface_netmask);  /*   when the address is changed */

   /* Mark the socket to allow broadcasting. Note that this is now done for point-to-point interfaces as well. */
   if(setsockopt(iface->iface_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0)
      syslog(LOG_ERR, "iface_prepare_bootp: setsockopt(SO_BROADCAST): %m");
   else
      syslog(LOG_DEBUG, "iface_prepare_bootp: set socket to allow broadcasts.");

   if(iface->iface_flags & IFF_BROADCAST)
   {
      if(conf->cnf_addr == INADDR_ANY || conf->cnf_netmask)
      {
         /* Set up the netmask if IP is 0.0.0.0 or mask is explicitly given. */
         if(iface_setnetmask(iface, conf->cnf_netmask))
         return(FALSE);
      }

      if(conf->cnf_addr == INADDR_ANY || conf->cnf_dst)
      {
         ULONG addr;

         /* Set up the broadcast address if IP is 0.0.0.0 or broadcast address is explicitly given. */
         if(conf->cnf_dst == INADDR_ANY)
            addr = INADDR_BROADCAST;
         else
            addr = conf->cnf_dst;
      
         if(iface_setdstaddr(iface, addr))
            return FALSE;
      }
   }

   if(iface->iface_flags & IFF_POINTOPOINT)
   {
      ULONG addr;

      if(conf->cnf_dst == INADDR_ANY)
      {
         if(iface->iface_dst == INADDR_ANY)
            addr = 0x00010203;      /* fake address */
         else
            addr = iface->iface_dst;

         if(iface->iface_addr == addr) /* must be different */
            addr++;
      }
      else
         addr = conf->cnf_dst;   /* is different due to argument constraints */

      /* set up the destination address */
      if(iface_setdstaddr(iface, addr))
         return(FALSE);
      iface->iface_dst = addr;

      /* DialUp version does not need this nor can ad routes */
//      if(!dialup)
//      {
//         /* Route "broadcasts" to the point-to-point destination */
//         syslog(LOG_DEBUG, "iface_prepare_bootp: adding route for broadcast address on p-t-p link.");
//         if(error = route_add(FD, RTF_UP | RTF_GATEWAY|RTF_HOST, INADDR_BROADCAST, iface->iface_dst, TRUE /* force */))
//            return(FALSE);
//      }
   }

  /*
   * Since the Sana2 hw-type is wider (4 bytes) than used in BOOTP (1 byte),
   * the hardware types above 255 must be ignored. This is done by forcing
   * down the cnf_use_hwtype field from the config info.
   * Also, if we know that the hw-address is not usable for BOOTP, we'll
   * force not using it. This is the case for PPP, SLIP and CSLIP and other
   * point-to-point interfaces.
   */
   if(iface->iface_flags & IFF_POINTOPOINT)
      conf->cnf_use_hwtype = FALSE;
   else if(iface->iface_htype >= 256)
      conf->cnf_use_hwtype = FALSE;

   return(TRUE);
}

///
/// iface_cleanup_bootp
VOID iface_cleanup_bootp(struct iface *iface, struct config *conf)
{
   if(iface->iface_flags & IFF_POINTOPOINT)
   {
      /* DialUp version does not need this nor can ad routes */
//      if(!dialup)
//      {
//         /* delete the added "broadcasts" route */
//         syslog(LOG_DEBUG, "iface_cleanup_bootp: deleting route for broadcast address on p-t-p link");
//         route_delete(iface->iface_fd, INADDR_BROADCAST);
//      }
   }
}

///
/// iface_online
BOOL iface_online(struct iface *iface)
{
   if(sana2_online(iface->iface_s2) == FALSE)
      return(FALSE);

   return(TRUE);
}

///
/// iface_offline
BOOL iface_offline(struct iface *iface)
{
   if(sana2_offline(iface->iface_s2) == FALSE)
      return(FALSE);

   return(TRUE);
}

///
/// iface_config
BOOL iface_config(struct iface *iface, struct config *conf)
{
   /* Delete the old configuration */
   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family = AF_INET;
   IFR.ifr_addr.sa_len = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = iface->iface_addr;

   if(ioctl(FD, SIOCDIFADDR, (char *)&IFR) < 0)
   {
      if(errno != EADDRNOTAVAIL)
      {
         /* no previous address ? */
         syslog(LOG_ERR, "iface_config: SIOCDIFADDR: %m");
         return(FALSE);
      }
   }

   /* Build request to add new config */
   /* address */
   memset(&IFRA.ifra_addr, 0, sizeof(IFRA.ifra_addr));
   IFRA.ifra_addr.sa_family   = AF_INET;
   IFRA.ifra_addr.sa_len      = sizeof(IFRA.ifra_addr);
   ((struct sockaddr_in *)&IFRA.ifra_addr)->sin_addr.s_addr = conf->cnf_addr;

   /* destination/broadcast address */
   memset(&IFRA.ifra_broadaddr, 0, sizeof(IFRA.ifra_broadaddr));
   if(conf->cnf_dst != INADDR_ANY)
   {
      IFRA.ifra_broadaddr.sa_family = AF_INET;
      IFRA.ifra_broadaddr.sa_len    = sizeof(IFRA.ifra_broadaddr);
      ((struct sockaddr_in *)&IFRA.ifra_broadaddr)->sin_addr.s_addr = conf->cnf_dst;
   }

   /* netmask */
   memset(&IFRA.ifra_mask, 0, sizeof(IFRA.ifra_mask));
   if(conf->cnf_netmask != INADDR_ANY)
   {
      IFRA.ifra_mask.sa_family   = 0; /* no family for masks */
      IFRA.ifra_mask.sa_len      = sizeof(IFRA.ifra_mask);
      ((struct sockaddr_in *)&IFRA.ifra_mask)->sin_addr.s_addr = conf->cnf_netmask;
   }

   if(ioctl(FD, SIOCAIFADDR, (char *)&IFRA) < 0)
   {
      syslog(LOG_ERR, "iface_config: SIOCAIFADDR: %m");
      return(FALSE);
   }
  
   /* Set the interface MTU if explicitly set */
   if(conf->cnf_MTU > 0 && conf->cnf_MTU != iface->iface_MTU)
      iface_setmtu(iface, conf->cnf_MTU);

   return(TRUE);
}

///
/// my_link_ntoa
static char hexlist[] = "0123456789abcdef";

char *my_link_ntoa(u_char len, u_char *addr)
{
   static char obuf[64];
   register char *out = obuf;
   register int i;
   register u_char *in = addr;
   u_char *inlim = addr + len;
   int firsttime = 1;

   while(in < inlim)
   {
      if(firsttime) firsttime = 0; else *out++ = ':';
      i = *in++;
      if(i > 0xf)
      {
         out[1] = hexlist[i & 0xf];
         i >>= 4;
         out[0] = hexlist[i];
         out += 2;
      }
      else
         *out++ = hexlist[i];
   }
   *out = 0;
   return(obuf);
}

///

/// iface_getaddr
int iface_getaddr(struct iface *iface, u_long *addr)
{
   if(ioctl(FD, SIOCGIFADDR, (char *)&IFR) < 0)
   {
      if (errno != EADDRNOTAVAIL)
      {
         syslog(LOG_ERR, "iface_getaddr: SIOCGIFADDR: %m");
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
int iface_setaddr(struct iface *iface, u_long addr)
{
   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family  = AF_INET;
   IFR.ifr_addr.sa_len     = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = addr;

   if(ioctl(FD, SIOCSIFADDR, (char *)&IFR) < 0)
   {
      syslog(LOG_ERR, "iface_setaddr: SIOCSIFADDR: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getdstaddr
int iface_getdstaddr(struct iface *iface, u_long *addr)
{
   /* get interface dst/broadcast address */
   u_long request;

   if(iface->iface_flags & IFF_POINTOPOINT)
      request = SIOCGIFDSTADDR;
   else if(iface->iface_flags & IFF_BROADCAST)
      request = SIOCGIFBRDADDR;
   else
   {
      /* no destination address of any kind available */
      *addr = INADDR_ANY;
      return(0);
   }

   if(ioctl(FD, request, (char *)&IFR) < 0)
   {
      if(errno != EADDRNOTAVAIL)
      {
         syslog(LOG_ERR, "iface_getdstaddr: SIOCGIF(DST/BRD)ADDR: %m");
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
int iface_setdstaddr(struct iface *iface, u_long addr)
{
   u_long request;

   if(iface->iface_flags & IFF_POINTOPOINT)
      request = SIOCSIFDSTADDR;
   else if(iface->iface_flags & IFF_BROADCAST)
      request = SIOCSIFBRDADDR;
   else
      return(0);

   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family  = AF_INET;
   IFR.ifr_addr.sa_len     = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = addr;

   if(ioctl(FD, request, (char *)&IFR) < 0)
   {
      syslog(LOG_ERR, "iface_setdstaddr: SIOCGIF(DST/BRD)ADDR: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getnetmask
int iface_getnetmask(struct iface *iface, u_long *addr)
{
   if(!((*(BYTE *)((struct Library *)SocketBase + 1)) & 0x02))
      return(187);

   if(ioctl(FD, SIOCGIFNETMASK, (char *)&IFR) < 0)
   {
      if(errno != EADDRNOTAVAIL)
      {
         syslog(LOG_ERR, "iface_getnetmask: SIOCGIFNETMASK: %m");
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
int iface_setnetmask(struct iface *iface, u_long addr)
{
   memset(&IFR.ifr_addr, 0, sizeof(IFR.ifr_addr));
   IFR.ifr_addr.sa_family  = AF_INET;
   IFR.ifr_addr.sa_len     = sizeof(IFR.ifr_addr);
   ((struct sockaddr_in *)&IFR.ifr_addr)->sin_addr.s_addr = addr;

   if(ioctl(FD, SIOCSIFNETMASK, (char *)&IFR) < 0)
   {
      syslog(LOG_ERR, "iface_setnetmask: SIOCSIFNETMASK: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getflags
int iface_getflags(struct iface *iface, short *flags)
{
   if(ioctl(FD, SIOCGIFFLAGS, (char *)&IFR) < 0)
   {
      syslog(LOG_ERR, "iface_getflags: SIOCGIFFLAGS: %m");
      return(errno);
   }
   *flags = IFR.ifr_flags;
   return(0);
}

///
/// iface_setflags
int iface_setflags(struct iface *iface, short flags)
{
   IFR.ifr_flags = flags;

   if(ioctl(FD, SIOCSIFFLAGS, (char *)&IFR) < 0)
   {
      syslog(LOG_ERR, "iface_setflags: SIOCSIFFLAGS: %m");
      return errno;
   }
   return(0);
}

///
/// iface_getmtu
int iface_getmtu(struct iface *iface, int *mtu)
{
   if(ioctl(FD, SIOCGIFMTU, (char *)&IFR) < 0)
   {
      syslog(LOG_ERR, "iface_getmtu: SIOCGIFMTU: %m");
      return(errno);
   }

   *mtu = IFR.ifr_mtu;
   return(0);
}

///
/// iface_setmtu
int iface_setmtu(struct iface *iface, int mtu)
{
   IFR.ifr_mtu = mtu;

   if(ioctl(FD, SIOCSIFMTU, (char *)&IFR) < 0)
   {
      syslog(LOG_ERR, "iface_setmtu: SIOCSIFMTU: %m");
      return(errno);
   }
   return(0);
}

///
/// iface_getlinkinfo
int iface_getlinkinfo(struct iface *iface)
{
   register int n;
   struct ifreq ibuf[16];
   struct ifconf ifc;
   register struct ifreq *ifrp, *ifend;

   /* Fetch the interface configuration to find the link level address */
   ifc.ifc_len = sizeof(ibuf);
   ifc.ifc_buf = (caddr_t) ibuf;
   if(ioctl(FD, SIOCGIFCONF, (char *) &ifc) < 0)
   {
      syslog(LOG_ERR, "iface_getlinkinfo: SIOCGIFCONF: %m");
      return(errno);
   }

   if(ifc.ifc_len < sizeof(struct ifreq))
   {
      syslog(LOG_WARNING, "iface_getlinkinfo: link level information not available!");
      return(ENOENT);
   }

   /* Search interface configuration list for link layer address. */
   ifrp = ibuf;
   ifend = (struct ifreq *) ((char *) ibuf + ifc.ifc_len);
   while(ifrp < ifend)
   {
      /* Look for interface */
      if(strcmp(iface->iface_name, ifrp->ifr_name) == 0 && ifrp->ifr_addr.sa_family == AF_LINK)
      {
         struct sockaddr_dl * dl_addr = (struct sockaddr_dl *)&ifrp->ifr_addr;

         iface->iface_htype   = dl_addr->sdl_type;
         iface->iface_hlen    = dl_addr->sdl_alen; /* hwaddr len */
         memcpy(iface->iface_haddr, LLADDR(dl_addr), iface->iface_hlen);
      
         return(0);
      }
      /* Bump interface config pointer */
      n = ifrp->ifr_addr.sa_len + sizeof(ifrp->ifr_name);
      if(n < sizeof(*ifrp))
         n = sizeof(*ifrp);
      ifrp = (struct ifreq *) ((char *) ifrp + n);
   }
   return(ENOENT); /* link level address not found */
}

///

