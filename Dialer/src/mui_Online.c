/// includes & defines
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "/genesis.lib/pragmas/genesislogger_lib.h"
#include "/genesis.lib/proto/genesislogger.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Online.h"
#include "mui_MainWindow.h"
#include "mui_Led.h"
#include "mui_NetInfo.h"
#include "protos.h"
#include "bootp/bootpc.h"
//#include "dhcp/dhcp.h"
#include "iface.h"

#define LOOPBACK_NAME "lo0"
#define LOOPBACK_ADDR 0x7f000001

///
/// external variables
extern struct Library *GenesisBase, *GenesisLoggerBase, *NetConnectBase;
extern struct Library *BattClockBase;
extern int errno;
extern struct Config Config;
extern Object *app, *win, *status_win, *netinfo_win;
extern struct MUI_CustomClass  *CL_MainWindow, *CL_Online;
extern struct MsgPort *MainPort;
extern const char AmiTCP_PortName[];
extern BOOL dialup, use_reconnect;
extern struct timerequest *TimeReq;

///

/// config_lo0
BOOL config_lo0(struct Library *SocketBase)
{
   struct Interface_Data *iface_data;

   if(iface_data = iface_alloc(SocketBase))
   {
      strcpy(iface_data->ifd_name, LOOPBACK_NAME);
      iface_setaddr(SocketBase, iface_data, LOOPBACK_ADDR);
      iface_free(SocketBase, iface_data);
      return(TRUE);
   }
   return(FALSE);
}

///
/// netmask_check
BOOL netmask_check(ULONG netmask, ULONG ipaddr)
{
   // Check that given netmask is contiguous and on a valid range
   if(netmask != INADDR_ANY)
   {
      ULONG mask     = 0xfffffffc;
      ULONG lastmask = IN_CLASSA_NET;

      // restrict the range if IP address is known
      if(ipaddr != INADDR_ANY)
      {
         if(IN_CLASSB(ipaddr))
            lastmask = IN_CLASSB_NET;
         else if(IN_CLASSC(ipaddr))
            lastmask = IN_CLASSC_NET;
      }
      for(; mask >= lastmask; mask <<= 1)
      {
         if(netmask == mask)
            return(TRUE);
      }
      return(FALSE); // mask was not on the valid range
   }
   return(TRUE);
}

///
/// config_complete
BOOL config_complete(struct Library *SocketBase, struct Interface_Data *iface_data, struct Interface *iface, struct ISP *isp)
{
   char *tmpstr;
   BOOL is_secondary = (iface != (struct Interface *)isp->isp_ifaces.mlh_Head ? TRUE : FALSE);
   // Special checks for point-to-point interfaces
   if(!dialup && iface_data->ifd_flags & IFF_POINTOPOINT)
   {
      // try to find destination IP for a p-to-p link if not given
      if(!*iface->if_dst)
      {
         if(*iface->if_gateway)
         {
            syslog_AmiTCP(SocketBase, LOG_DEBUG, "Using the default gateway address as destination address.");
            strcpy(iface->if_dst, iface->if_gateway);
         }
         // Use a fake IP address for the destination IP address if nothing else is available.
         else if(!is_secondary)
         {
            syslog_AmiTCP(SocketBase, LOG_DEBUG, "Using a 'fake' IP address 0.1.2.3 as the destination address.");
            strcpy(iface->if_dst, "0.1.2.3");
         }
         // NOTE: could try existing destination, existing gateway etc.
         else
         {
            syslog_AmiTCP(SocketBase, LOG_ERR, "Could not get destination IP address for a point-to-point link.");
            return(FALSE);
         }
      }
      // use destination as default gateway if not given
      if(!*iface->if_gateway)
      {
         if(*iface->if_dst)
         {
            syslog_AmiTCP(SocketBase, LOG_DEBUG, "Using the destination address as default gateway address.");
            strcpy(iface->if_gateway, iface->if_dst);
         }
         // could try existing destination, existing gateway etc.
      }
   }

   // broadcast address will be set to default by AmiTCP

   // netmask will be set to default by AmiTCP if not present at all
   // warn if default netmask will be used on a broadcast interface
   if(!*iface->if_netmask && iface_data->ifd_flags & IFF_BROADCAST)
      syslog_AmiTCP(SocketBase, LOG_DEBUG, "Note: Default netmask will be used for the interface %ls", iface->if_name);

   // One additional validity test for the netmask we might now have the IP
   if(!netmask_check(inet_addr(iface->if_netmask), inet_addr(iface->if_addr)))
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "Invalid netmask value (%ls). Reverting to default.", iface->if_netmask);
      *iface->if_netmask = NULL;
   }

   if(!is_secondary)
   {
      // Checks for host name and domain name:
      // 1. If hostname is not set, try to get it from an environment variable
      // 2. If host name contains the domain part, check if domain name is not set, and set it.
      // 3. If host name does not contain the domain part, check if we can complete the name from the domain name.
      if((tmpstr = strchr(isp->isp_hostname, '.')) != NULL)
      {
         // is no domainname set yet ?
         if(isp->isp_domainnames.mlh_TailPred == (struct MinNode *)&isp->isp_domainnames)
            add_server(&isp->isp_domainnames, tmpstr+1);
      }
      else // dot NOT present on the host name
      {
         if((isp->isp_domainnames.mlh_TailPred != (struct MinNode *)&isp->isp_domainnames) && *isp->isp_hostname)
         {
            struct ServerEntry *server;

            server = (struct ServerEntry *)isp->isp_domainnames.mlh_Head;
            if(server->se_node.mln_Succ)
            {
               size_t hostnamelen = strlen(isp->isp_hostname);

               // Concatenate "." and domain name to the host name
               if(sizeof(isp->isp_hostname) > strlen(server->se_name) + hostnamelen + 1)
                  isp->isp_hostname[hostnamelen++] = '.';
               strcpy(isp->isp_hostname + hostnamelen, server->se_name);
            }
         }
      }
   }
   return(TRUE);
}

///
/// save_reconnectinfo
BOOL save_reconnectinfo(struct ISP *isp)
{
   BPTR fh;
   BOOL success = FALSE;
   struct ServerEntry *server;
   struct Interface *iface;

   if(fh = Open("AmiTCP:db/reconnect.conf", MODE_NEWFILE))
   {
      FPrintf(fh, "ISP           %ls\n", isp->isp_name);
      if(isp->isp_nameservers.mlh_TailPred != (struct MinNode *)&isp->isp_nameservers)
      {
         server = (struct ServerEntry *)isp->isp_nameservers.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            FPrintf(fh, "NameServer    %ls\n", server->se_name);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
      if(isp->isp_domainnames.mlh_TailPred != (struct MinNode *)&isp->isp_domainnames)
      {
         server = (struct ServerEntry *)isp->isp_domainnames.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            FPrintf(fh, "DomainName    %ls\n", server->se_name);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
      if(isp->isp_ifaces.mlh_TailPred != (struct MinNode *)&isp->isp_ifaces)
      {
         iface = (struct Interface *)isp->isp_ifaces.mlh_Head;
         while(iface->if_node.mln_Succ)
         {
            if(iface->if_flags & IFL_IsOnline)
            {
               FPrintf(fh, "Interface     %ls\n", iface->if_name);
               if(*iface->if_addr);
                  FPrintf(fh, "IPAddr        %ls\n", iface->if_addr);
               if(*iface->if_dst)
                  FPrintf(fh, "DestIP        %ls\n", iface->if_dst);
               if(*iface->if_gateway)
                  FPrintf(fh, "Gateway       %ls\n", iface->if_gateway);
            }
            iface = (struct Interface *)iface->if_node.mln_Succ;
         }
      }
      Close(fh);
   }

   return(success);
}

///
/// do_dhcp
/*
BOOL do_dhcp(struct Library *SocketBase, struct Online_Data *data, struct Interface *iface, struct Interface_Data *iface_data)
{
   BOOL success = FALSE;
   struct MsgPort *port;
   struct DHCP_Msg *msg;

   if(port = CreatePort(DHCP_PORTNAME, 0))
   {
      if(iface->if_dhcp_proc = CreateNewProcTags(
         NP_Entry      , DHCPHandler,
         NP_Name       , "GENESiS DHCP",
         NP_StackSize  , 16384,
         NP_WindowPtr  , -1,
         NP_CloseOutput, TRUE,
//         NP_Output, Open("AmiTCP:log/dhcp.log", MODE_NEWFILE),
NP_Output, Open("CON:/300/640/200/GENESiS DHCP/AUTO/WAIT/CLOSE", MODE_NEWFILE),
         TAG_END))
      {
         ULONG sig = NULL;
         BOOL do_loop = TRUE;

         while(do_loop && iface->if_dhcp_proc && !data->abort)
         {
            sig = Wait((1 << port->mp_SigBit) | SIGBREAKF_CTRL_C);

            if(sig & (1 << port->mp_SigBit))
            {
               while(msg = (struct DHCP_Msg *)GetMsg(port))
               {
                  if(msg->dhcp_abort)
                     do_loop = FALSE;
                  if(msg->dhcp_success)
                  {
                     success = TRUE;
                     do_loop = FALSE;
                  }
                  msg->dhcp_iface = iface;
                  msg->dhcp_iface_data = iface_data;

                  ReplyMsg((struct Message *)msg);
               }
            }
            if(sig & SIGBREAKF_CTRL_C)
               do_loop = FALSE;
         }
      }
      else
         syslog_AmiTCP(SocketBase, LOG_ERR, "could not launch dhcp subprocess");

      DeletePort(port);
   }

   return(success);
}
*/
///
/// adjust_default_gateway
VOID adjust_default_gateway(struct Library *SocketBase, struct MinList *list, struct Interface *iface)
{
   struct Interface *iface_ptr;
   BOOL gw_set = FALSE;

   if(list->mlh_TailPred != (struct MinNode *)list)
   {
      iface_ptr = (struct Interface *)list->mlh_Head;
      while(iface_ptr->if_node.mln_Succ)
      {
         if((iface == iface_ptr) && !gw_set && (iface->if_flags & IFL_IsOnline))
         {
            if(!dialup)
            {
               if(!*iface->if_gateway)
                  syslog_AmiTCP(SocketBase, LOG_DEBUG, "%ls: default gateway information not found.", iface->if_name);
               else
               {
                  int fd;

                  fd = socket(AF_INET, SOCK_DGRAM, 0);
                  if(fd >= 0)
                  {
                     route_add(SocketBase, fd, RTF_UP|RTF_GATEWAY, INADDR_ANY, inet_addr(iface->if_gateway), TRUE /* force */);
                     CloseSocket(fd);
                  }
               }
            }
            iface->if_flags |= IFL_IsDefaultGW;
            gw_set = TRUE;
         }
         else
         {
            if(!gw_set && (iface_ptr->if_flags & IFL_IsDefaultGW) && (iface_ptr->if_flags & IFL_IsOnline))
               gw_set = TRUE;
            else
               iface_ptr->if_flags &= ~IFL_IsDefaultGW;
         }
         iface_ptr = (struct Interface *)iface_ptr->if_node.mln_Succ;
      }
   }
}

///
/// get_daytime
BOOL get_daytime(struct Library *SocketBase, STRPTR host, STRPTR buf)
{
   BOOL success = FALSE;
   BPTR fh;


   if(host)
   {
      sprintf(buf, "TCP:%ls/daytime", host);
      if(fh = Open(buf, MODE_OLDFILE))
      {
         FGets(fh, buf, 255);
         success = TRUE;
         Close(fh);
      }
      else
         syslog_AmiTCP(SocketBase, LOG_ERR, "get_daytime: could not open %ls/daytime", host);
   }

   return(success);
}

///
/// sync_clock
BOOL sync_clock(struct Library *SocketBase, STRPTR host, BOOL save)
{
   char buf[MAXPATHLEN];
   BOOL success = FALSE;
   STRPTR ptr;
   char year[10], month[10], day[10], time[20];
   struct DateTime dat_time;
   ULONG sys_secs;

// format: 'Sat Sep 26 10:56:55 1998\r\n'

   if(get_daytime(SocketBase, host, buf))
   {
      ptr = buf;
      if(ptr = extract_arg(ptr, month, sizeof(month), NULL))   // skip dayname
      {
         if(ptr = extract_arg(ptr, month, sizeof(month), NULL))
         {
            if(ptr = extract_arg(ptr, day, sizeof(day), NULL))
            {
               if(ptr = extract_arg(ptr, time, sizeof(time), NULL))
               {
                  ptr += 2;
                  ptr[2] = NULL;
                  strncpy(year, ptr, sizeof(year));

                  sprintf(buf, "%ls-%ls-%ls", day, month, year);
                  dat_time.dat_Format  = FORMAT_DOS;
                  dat_time.dat_StrDate = buf;
                  dat_time.dat_StrTime = time;
                  if(StrToDate(&dat_time))
                  {
                     sys_secs = dat_time.dat_Stamp.ds_Days*86400 + dat_time.dat_Stamp.ds_Minute*60 + atol(&time[6]);

                     TimeReq->tr_node.io_Command   = TR_SETSYSTIME;
                     TimeReq->tr_time.tv_secs      = sys_secs;
                     TimeReq->tr_time.tv_micro     = NULL;
                     SendIO((struct IORequest *)TimeReq);

                     if(save)
                     {
                        if(BattClockBase = OpenResource("battclock.resource"))
                           WriteBattClock(sys_secs);
                        else
                           syslog_AmiTCP(SocketBase, LOG_ERR, "sync_clock: could not open battclock.resource");
                     }
                  }
                  else
                     syslog_AmiTCP(SocketBase, LOG_ERR, "sync_clock: got invalid datetime sting");
               }
            }
         }
      }
   }

   return(success);
}

///
/// TCPHandler
SAVEDS ASM VOID TCPHandler(register __a0 STRPTR args, register __d0 LONG arg_len)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct Interface *iface = NULL;
   struct Interface_Data *iface_data = NULL;
   struct Library *SocketBase;
   BOOL success = FALSE;

   // open bsdsocket.library
   if(!(SocketBase = NCL_OpenSocket()))
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorBsdsocketLib));
      goto abort;
   }
   if(SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno, SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno, TAG_END))
      syslog_AmiTCP(SocketBase, LOG_CRIT, "netconfig: could not set 'errno' ptr.");
   if((*(BYTE *)((struct Library *)SocketBase + 1)) & 0x04)
      dialup = 1;

   if(data->abort)   goto abort;

   if(mw_data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_ifaces)
   {
      BPTR fh;
      struct bootpc *bpc;
      char buffer[MAXPATHLEN];
      struct ServerEntry *server;

      DeleteFile("ENV:APPPdns1");
      DeleteFile("ENV:APPPdns2");

      iface = (struct Interface *)mw_data->isp.isp_ifaces.mlh_Head;
      while(iface->if_node.mln_Succ && !data->abort)
      {
         if(iface->if_flags & IFL_PutOnline)
         {
            if(!(iface->if_flags & IFL_IsOnline))
            {
               DoMainMethod(mw_data->GR_Led[(int)iface->if_userdata], MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Yellow, NULL);
               if(!(iface_data = iface_alloc(SocketBase)))
               {
                  syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_TX_ErrorIfaceAlloc));
                  goto abort;
               }

               if(data->abort)   goto abort;
               DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_InitDevice), NULL);
               if(data->abort)   goto abort;
               if(DoMainMethod(status_win, MUIM_Genesis_Get, (APTR)MUIA_Window_Open, NULL, NULL) && (Config.cnf_flags & CFL_ShowSerialInput))
                  DoMainMethod(data->TR_Terminal, TCM_INIT, NULL, NULL, NULL);
               if(data->abort)   goto abort;

               /*
                * The iface_init() does Sana-II processing for the interface
                * + dialing if configured.
                */
               if(!(iface_init(SocketBase, iface_data, iface, &mw_data->isp, &Config)))
                  goto abort;

               // get appp.device's ms-dns addresses
               ReadFile("ENV:APPPdns1", buffer, 20);
               if(!find_server_by_name(&mw_data->isp.isp_nameservers, buffer))
                  add_server(&mw_data->isp.isp_nameservers, buffer);
               ReadFile("ENV:APPPdns2", buffer, 20);
               if(!find_server_by_name(&mw_data->isp.isp_nameservers, buffer))
                  add_server(&mw_data->isp.isp_nameservers, buffer);

               NCL_CallMeSometimes();

               if(data->abort)   goto abort;

               if(iface->if_flags & IFL_BOOTP)
               {
//                  do_dhcp(SocketBase, data, iface, iface_data);
                  iface_prepare_bootp(SocketBase, iface_data, iface, &Config);
                  if(bpc = bootpc_create(SocketBase))
                  {
                     if(bootpc_do(SocketBase, bpc, iface_data, iface, &mw_data->isp))
                     {
                        syslog_AmiTCP(SocketBase, LOG_ERR, "BOOTP failed to have an answer.");
                        if(data->abort)   goto abort;
                        DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_NoBOOTP), NULL);
                     }
                     else
                     {
                        if(data->abort)   goto abort;
                        DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_GotBOOTP), NULL);
                     }
                     bootpc_delete(bpc);
                  }
                  else
                  iface_cleanup_bootp(SocketBase, iface_data, &Config);
               }
               if(data->abort)   goto abort;

//             if(!*iface->if_addr)
//             {
//                popup reuester here
//             }
//             if(data->abort)   goto abort;

               if(!*iface->if_addr)
               {
                  syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_TX_ErrorNoIP));
                  goto abort;
               }

               if(!config_complete(SocketBase, iface_data, iface, &mw_data->isp))
               {
                  syslog_AmiTCP(SocketBase, LOG_CRIT, "Not enough information to configure '%ls'.", iface->if_name);
                  goto abort;
               }

               DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConfiguringAmiTCP), NULL);
               if(data->abort)   goto abort;

               NCL_CallMeSometimes();

               if(!(iface_config(SocketBase, iface_data, iface, &Config)))
               {
                  syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_TX_ErrorConfigIface), iface->if_name);
                  goto abort;
               }

               if(data->abort)   goto abort;

               // iface is online now..
               iface->if_flags |= IFL_IsOnline;    // required for adjust_default_gateway
               iface->if_flags &= ~IFL_PutOnline;
               adjust_default_gateway(SocketBase, &mw_data->isp.isp_ifaces, iface);

               DoMainMethod(mw_data->GR_Led[(int)iface->if_userdata], MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Green, NULL);
               if(netinfo_win)
                  DoMainMethod(netinfo_win, MUIM_NetInfo_Redraw, NULL, NULL, NULL);
               syslog_AmiTCP(SocketBase, LOG_NOTICE, "%ls is now online", iface->if_name);
               exec_event(&iface->if_events, IFE_Online);

               iface_deinit(iface_data);
               iface_free(SocketBase, iface_data);
               iface_data = NULL;
            }
         }
         iface = (struct Interface *)iface->if_node.mln_Succ;
      }

      // now that the ifaces are online, take care of resolv

      amirexx_do_command("RESET RESOLV");

      if(mw_data->isp.isp_nameservers.mlh_TailPred == (struct MinNode *)&mw_data->isp.isp_nameservers)
      {
         syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_TX_WarningNoDNS));
         add_server(&mw_data->isp.isp_nameservers, "198.41.0.4");
         add_server(&mw_data->isp.isp_nameservers, "128.9.0.107");
         mw_data->isp.isp_flags |= ISF_DontQueryHostname;
      }

      // add in reverse order since each entry is added to top of list
      if(mw_data->isp.isp_nameservers.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_nameservers)
      {
         server = (struct ServerEntry *)mw_data->isp.isp_nameservers.mlh_TailPred;
         while(server->se_node.mln_Pred)
         {
            if(amirexx_do_command("ADD START NAMESERVER %ls", server->se_name) != RETURN_OK)
               syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_TX_ErrorSetDNS), server->se_name);
            server = (struct ServerEntry *)server->se_node.mln_Pred;
            if(data->abort)   goto abort;
         }
      }

      if(data->abort)   goto abort;

      if(!(mw_data->isp.isp_flags & ISF_DontQueryHostname))
      {
         // is hostname or domainname missing but got rest ?
         if((mw_data->isp.isp_nameservers.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_nameservers) &&
            (!*mw_data->isp.isp_hostname || mw_data->isp.isp_domainnames.mlh_TailPred == (struct MinNode *)&mw_data->isp.isp_domainnames))
         {
            DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_GetDomain), NULL);
            if(data->abort)   goto abort;
            if(!*mw_data->isp.isp_hostname)
            {
               if(gethostname(mw_data->isp.isp_hostname, sizeof(mw_data->isp.isp_hostname)) || !*mw_data->isp.isp_hostname)
                  syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_TX_WarningNoHostname));
            }
            if((mw_data->isp.isp_domainnames.mlh_TailPred == (struct MinNode *)&mw_data->isp.isp_domainnames) && *mw_data->isp.isp_hostname)
            {
               STRPTR ptr;

               if(ptr = strchr(mw_data->isp.isp_hostname, '.'))
               {
                  ptr++;
                  add_server(&mw_data->isp.isp_domainnames, ptr);
               }
            }
         }
      }
      if(data->abort)   goto abort;

      if(mw_data->isp.isp_domainnames.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_domainnames)
      {
         server = (struct ServerEntry *)mw_data->isp.isp_domainnames.mlh_TailPred;
         while(server->se_node.mln_Pred)
         {
            if(amirexx_do_command("ADD START DOMAIN %ls", server->se_name) != RETURN_OK)
               syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_TX_WarningSetDomain), server->se_name);
            server = (struct ServerEntry *)server->se_node.mln_Pred;
            if(data->abort)   goto abort;
         }
      }
      else
         syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_TX_WarningNoDomain));

      if(*mw_data->isp.isp_hostname)
      {
         struct hostent *host;
         ULONG ip;

         if(host = gethostbyname(mw_data->isp.isp_hostname))
         {
            memcpy(&ip, *host->h_addr_list, 4);
            if(amirexx_do_command("ADD START HOST %ls %ls", Inet_NtoA(ip), mw_data->isp.isp_hostname) != RETURN_OK)
               syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_TX_WarningSetHostname), mw_data->isp.isp_hostname);
            WriteFile("ENV:HOSTNAME", mw_data->isp.isp_hostname, -1);
         }
      }

      if(data->abort)   goto abort;

      if(fh = Open("AmiTCP:db/resolv.conf", MODE_NEWFILE))
      {
         FPrintf(fh, "; This file is built dynamically - do not edit\n");
         FPrintf(fh, "; Name Servers\n");
         if(mw_data->isp.isp_nameservers.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_nameservers)
         {
            server = (struct ServerEntry *)mw_data->isp.isp_nameservers.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               FPrintf(fh, "NAMESERVER %ls\n", server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }
         FPrintf(fh, "; Search domains\n");
         if(mw_data->isp.isp_domainnames.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_domainnames)
         {
            server = (struct ServerEntry *)mw_data->isp.isp_domainnames.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               FPrintf(fh, "DOMAIN %ls\n", server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }
         Close(fh);
      }
   }

   if((mw_data->isp.isp_flags & (ISF_GetTime | ISF_SaveTime)) && *mw_data->isp.isp_timename)
   {
      DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_QueryTimeserver), NULL);
      sync_clock(SocketBase, mw_data->isp.isp_timename, (mw_data->isp.isp_flags & ISF_SaveTime));
   }

   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_Finished), NULL);
   if(data->abort)   goto abort;
   DoMainMethod(win, MUIM_MainWindow_SetStates, NULL, NULL, NULL);
   success = TRUE;


abort:

// **********************************************************************
// NO DoMainMethod() beyond this point !! otherwise abort will block  !!!
// **********************************************************************

    if(iface_data)
    {
       iface_deinit(iface_data);
       iface_free(SocketBase, iface_data);
       iface_data = NULL;

       if(iface->if_flags & IFL_IsSerial)
         serial_hangup(SocketBase);

       if(!data->abort)
       {
          exec_event(&iface->if_events, IFE_OnlineFail);
          DoMethod(app, MUIM_Application_PushMethod, mw_data->GR_Led[(int)iface->if_userdata], 3, MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Red);
          if(netinfo_win)
             DoMethod(app, MUIM_Application_PushMethod, netinfo_win, 1, MUIM_NetInfo_Redraw);
       }
    }

   if(!success && iface)
   {
      if(iface->if_loggerhandle && GenesisLoggerBase)
         GL_StopLogger(iface->if_loggerhandle);
      iface->if_loggerhandle = NULL;
      iface->if_flags &= ~IFL_PutOnline;
   }

   if(Config.cnf_flags & CFL_QuickReconnect)
      save_reconnectinfo(&mw_data->isp);
   use_reconnect = FALSE;

   if(SocketBase)
      CloseLibrary(SocketBase);
   SocketBase = NULL;

   Forbid();
   data->TCPHandler = NULL;
   if(!data->abort)
      DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, status_win);
}

///

/// Online_GoOnline
ULONG Online_GoOnline(struct IClass *cl, Object *obj, struct MUIP_Online_GoOnline *msg)
{
   struct Online_Data *data = INST_DATA(cl, obj);

   if(data->TCPHandler = CreateNewProcTags(
      NP_Entry      , TCPHandler,
      NP_Name       , "GENESiS Netconfig",
      NP_StackSize  , 16384,
      NP_WindowPtr  , -1,
      NP_CloseOutput, TRUE,
//      NP_Output, Open("AmiTCP:log/GENESiS.log", MODE_NEWFILE),
NP_Output, Open("CON:/300/640/200/GENESiS Netconfig/AUTO/WAIT/CLOSE", MODE_NEWFILE),
      TAG_END))
   {
      return(TRUE);
   }
   else
   {
      MUI_Request(_app(obj), obj, NULL, NULL, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorLaunchSubtask));
      DoMethod(_app(obj), MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);
   }
   return(FALSE);
}

///
/// Online_Dispose
ULONG Online_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
   struct Online_Data *data = INST_DATA(cl, obj);

   data->abort = TRUE;
   while(data->TCPHandler)
   {
      Forbid();
      if(data->TCPHandler)
         Signal((struct Task *)data->TCPHandler, SIGBREAKF_CTRL_C);
      Permit();
      Delay(20);
      HandleMainMethod(MainPort);
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///
/// Online_New
ULONG Online_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Online_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title       , GetStr(MSG_LA_PleaseWait),
      MUIA_Window_CloseGadget , FALSE,
      MUIA_Window_ID          , MAKE_ID('O','N','L','I'),
      MUIA_Window_RefWindow   , win,
      MUIA_Window_LeftEdge    , MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge     , MUIV_Window_TopEdge_Centered,
      MUIA_Window_Height      , MUIV_Window_Height_MinMax(0),
      MUIA_Window_Width       , MUIV_Window_Width_MinMax(0),
      WindowContents          , VGroup,
         Child, VGroup,
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            MUIA_Weight, 1,
            Child, HVSpace,
            Child, tmp.TX_Info = TextObject,
               MUIA_Text_Contents, "                                                                           \n",
               MUIA_Text_PreParse, "\033c",
            End,
            Child, HVSpace,
         End,
         Child, tmp.BU_Busy = BusyObject,
            MUIA_FixHeight, 6,
         End,
         Child, tmp.GR_Terminal = HGroup,
            GroupSpacing(0),
            InnerSpacing(0, 0),
            MUIA_ShowMe, Config.cnf_flags & CFL_ShowSerialInput,
            Child, tmp.TR_Terminal = TermObject,
               MUIA_CycleChain, 1,
               InputListFrame,
               TCA_EMULATION  , TCV_EMULATION_VT100,
               TCA_LFASCRLF   , TRUE,
               TCA_DESTRBS    , TRUE,
               TCA_ECHO       , FALSE,
               TCA_DELASBS    , TRUE,
               TCA_CURSORSTYLE, TCV_CURSORSTYLE_UNDERLINED,
               TCA_SELECT     , TRUE,
               TCA_8BIT       , TRUE,
               TCA_WRAP       , FALSE,
            End,
            Child, tmp.SB_Terminal = ScrollbarObject,
               MUIA_Weight, 0,
            End,
         End,
         Child, tmp.BT_Abort = MakeButton(MSG_BT_Abort),
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Online_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->TCPHandler  = NULL;
      data->abort       = FALSE;

      set(data->TR_Terminal, TCA_SCROLLER, data->SB_Terminal);

      DoMethod(data->SB_Terminal , MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime, data->TR_Terminal, 1, TCM_SCROLLER);
      DoMethod(data->BT_Abort    , MUIM_Notify, MUIA_Pressed   , FALSE,          MUIV_Notify_Application, 5, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);
   }

   return((ULONG)obj);
}

///
/// Online_Dispatcher
SAVEDS ASM ULONG Online_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                   : return(Online_New         (cl, obj, (APTR)msg));
      case OM_DISPOSE               : return(Online_Dispose     (cl, obj, (APTR)msg));
      case MUIM_Online_GoOnline     : return(Online_GoOnline    (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

