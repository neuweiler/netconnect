/// includes & defines
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "/genesis.lib/pragmas/genesislogger_lib.h"
#include "/genesis.lib/proto/genesislogger.h"
#include "vupdate.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Online.h"
#include "mui_MainWindow.h"
#include "mui_Led.h"
#include "mui_NetInfo.h"
#include "protos.h"
#include "bootp/bootpc.h"
#include "rev.h"
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
extern BOOL dialup;
extern struct timerequest *TimeReq;
extern struct Library *VUPBase;
extern APTR vuphandle;

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
/// is_inaddr_any
BOOL is_inaddr_any(struct Library *SocketBase, STRPTR addr)
{
   if(!*addr || !strcmp(addr, "0.0.0.0") || (inet_addr(addr) == INADDR_NONE) || (inet_addr(addr) == INADDR_ANY))
   {
      *addr = NULL;
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
BOOL config_complete(struct Library *SocketBase, struct Interface_Data *iface_data, struct Interface *iface)
{
   // Special checks for point-to-point interfaces
   if(!dialup && (iface_data->ifd_flags & IFF_POINTOPOINT))
   {
      // try to find destination IP for a p-to-p link if not given
      if(is_inaddr_any(SocketBase, iface->if_dst))
      {
         if(!is_inaddr_any(SocketBase, iface->if_gateway))
         {
            syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_UsingDefaultGatewayAsDest));
            strncpy(iface->if_dst, iface->if_gateway, sizeof(iface->if_dst));
         }
         // Use a fake IP address for the destination IP address if nothing else is available.
         else
         {
            syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_UsingLocalIPAsFakeDest), iface->if_addr);
            strncpy(iface->if_dst, iface->if_addr, sizeof(iface->if_dst));
         }
      }
      // use destination as default gateway if not given
      if(is_inaddr_any(SocketBase, iface->if_gateway))
      {
         if(!is_inaddr_any(SocketBase, iface->if_dst))
         {
            syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_UsingDestIPAsDefaultGateway));
            strncpy(iface->if_gateway, iface->if_dst, sizeof(iface->if_gateway));
         }
         // could try existing destination, existing gateway etc.
      }
   }

   // broadcast address will be set to default by AmiTCP

   // netmask will be set to default by AmiTCP if not present at all
   // warn if default netmask will be used on a broadcast interface
   if(is_inaddr_any(SocketBase, iface->if_netmask) && (iface_data->ifd_flags & IFF_BROADCAST))
      syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_UsingDefaultNetmask), iface->if_name);

   // One additional validity test for the netmask we might now have the IP
   if(!netmask_check(inet_addr(iface->if_netmask), inet_addr(iface->if_addr)))
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_InvalidNetmask), iface->if_netmask);
      *iface->if_netmask = NULL;
   }

   return(TRUE);
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
         syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_CouldNotLaunchSubProc));

      DeletePort(port);
   }

   return(success);
}
*/
///
/// adjust_default_gateway
VOID adjust_default_gateway(struct Library *SocketBase, struct MinList *list)
{
   struct Interface *iface;
   BOOL gw_set = FALSE;

   if(list->mlh_TailPred != (struct MinNode *)list)
   {
      iface = (struct Interface *)list->mlh_Head;
      while(iface->if_node.mln_Succ)
      {
         if((iface->if_flags & IFL_IsOnline) && !gw_set)
         {
            if(iface->if_flags & IFL_IsDefaultGW)  // first online is already gw => no change
               break;

            if(!dialup)
            {
               if(is_inaddr_any(SocketBase, iface->if_gateway))
                  syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_NoDefaultGatewayInfo), iface->if_name);
               else
               {
                  int fd;

                  fd = socket(AF_INET, SOCK_DGRAM, 0);
                  if(fd >= 0)
                  {
                     route_add(SocketBase, fd, RTF_UP|RTF_GATEWAY, INADDR_ANY, inet_addr(iface->if_gateway), TRUE /* force */);
                     CloseSocket(fd);
                  }
                  iface->if_flags |= IFL_IsDefaultGW;
                  gw_set = TRUE;
               }
            }
         }
         else
            iface->if_flags &= ~IFL_IsDefaultGW;

         iface = (struct Interface *)iface->if_node.mln_Succ;
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
         syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_DayTimeCouldNotOpen), host);
   }

   return(success);
}

///
/// sync_clock
BOOL sync_clock(struct Library *SocketBase, STRPTR host, BOOL save)
{
   STRPTR months = "JanFebMarAprMayJunJulAugSepOctNovDec";
   char buf[MAXPATHLEN];
   BOOL success = FALSE;
   STRPTR ptr;
   char year[10], month[10], day[10], time[20];
   struct DateTime dat_time;
   ULONG sys_secs, month_number;

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

                  month_number = 13;   // getsystime will fail when month_number can't be found
                  if(ptr = strstr(months, month))
                     month_number = (ptr - months) / 3 + 1;

                  sprintf(buf, "%02.2ld-%02.2ls-%02.2ls", month_number, day, year);
                  dat_time.dat_Format  = FORMAT_USA;
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
                           syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_NoBattClock));
                     }
                  }
                  else
                     syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_InvalidDatetimeString));
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
   struct ServerEntry *server;
   BOOL success = FALSE;
   STRPTR ptr;

   // open bsdsocket.library
   if(!(SocketBase = NCL_OpenSocket()))
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorBsdsocketLib));
      goto abort;
   }
   if(SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno, SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno, TAG_END))
      syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_SYSLOG_CouldNotSetErrno));
   if((*(BYTE *)((struct Library *)SocketBase + 1)) & 0x04)
      dialup = 1;

   if(data->abort)   goto abort;

   if(netinfo_win)   // to display the "connecting" message
      DoMainMethod(netinfo_win, MUIM_NetInfo_SetStates, NULL, NULL, NULL);

   if(Config.cnf_ifaces.mlh_TailPred != (struct MinNode *)&Config.cnf_ifaces)
   {
      struct bootpc *bpc;
      char buffer[MAXPATHLEN];

      DeleteFile("ENV:APPPdns1");
      DeleteFile("ENV:APPPdns2");

      iface = (struct Interface *)Config.cnf_ifaces.mlh_Head;
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
               if(!(iface_init(SocketBase, iface_data, iface, &Config)))
                  goto abort;

               // get appp.device's ms-dns addresses
               if(iface->if_flags & IFL_UseNameServer)
               {
                  if(ReadFile("ENV:APPPdns2", buffer, 20) > 0)
                     add_server(&iface->if_nameservers, buffer, TRUE); // add to top
                  if(ReadFile("ENV:APPPdns1", buffer, 20) > 0)
                     add_server(&iface->if_nameservers, buffer, TRUE); // add to top
               }

               NCL_CallMeSometimes();

               if(data->abort)   goto abort;

               if(iface->if_flags & IFL_BOOTP)
               {
                  DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_SendingBOOTP), NULL);

//                  do_dhcp(SocketBase, data, iface, iface_data);

                  iface_prepare_bootp(SocketBase, iface_data, iface, &Config);
                  if(bpc = bootpc_create(SocketBase))
                  {
                     if(bootpc_do(SocketBase, bpc, iface_data, iface, &Config))
                     {
                        syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_NoBootpAnswer));
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

//             if(is_inaddr_any(SocketBase, iface->if_addr))
//             {
//                popup reuester here
//             }
//             if(data->abort)   goto abort;

               if(is_inaddr_any(SocketBase, iface->if_addr))
               {
                  syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_TX_ErrorNoIP));
                  goto abort;
               }

               if(!config_complete(SocketBase, iface_data, iface))
               {
                  syslog_AmiTCP(SocketBase, LOG_CRIT, GetStr(MSG_SYSLOG_NotEnoughInfoToConfigIface), iface->if_name);
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
               adjust_default_gateway(SocketBase, &Config.cnf_ifaces);
               if(data->abort)   goto abort;

               DoMainMethod(mw_data->GR_Led[(int)iface->if_userdata], MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Green, NULL);
               if(netinfo_win)
                  DoMainMethod(netinfo_win, MUIM_NetInfo_SetStates, NULL, NULL, NULL);
               syslog_AmiTCP(SocketBase, LOG_NOTICE, GetStr(MSG_SYSLOG_IfaceOnline), iface->if_name);
               exec_event(&iface->if_events, IFE_Online);
               if(data->abort)   goto abort;

               iface_deinit(iface_data);
               iface_free(SocketBase, iface_data);
               iface_data = NULL;
            }
         }
         iface = (struct Interface *)iface->if_node.mln_Succ;
      }

      if(data->abort)   goto abort;

      // now take care of resolv
      amirexx_do_command("RESET RESOLV");

      // scan list backwards
      if(Config.cnf_ifaces.mlh_TailPred != (struct MinNode *)&Config.cnf_ifaces)
      {
         iface = (struct Interface *)Config.cnf_ifaces.mlh_TailPred;
         while(iface->if_node.mln_Pred)
         {
            // first add the nameservers so we can query them, only use those of ifaces currently online
            if((iface->if_flags & IFL_IsOnline) && (iface->if_nameservers.mlh_TailPred != (struct MinNode *)&iface->if_nameservers))
            {
               server = (struct ServerEntry *)iface->if_nameservers.mlh_TailPred;
               while(server->se_node.mln_Pred)
               {
                  syslog_AmiTCP(SocketBase, LOG_INFO, GetStr(MSG_SYSLOG_AddingNameServer), server->se_name);
                  if(amirexx_do_command("ADD START NAMESERVER %ls", server->se_name) != RETURN_OK)
                     syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_TX_ErrorSetDNS), server->se_name);
                  server = (struct ServerEntry *)server->se_node.mln_Pred;
               }
            }

            if(data->abort)   goto abort;

            // have to query hostname or domainname ?
            if((iface->if_flags & IFL_PutOnline) && (iface->if_flags & (IFL_UseHostName | IFL_UseDomainName)))
            {
               struct hostent *host;
               ULONG addr;

               DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_GetDomain), NULL);
               if(data->abort)   goto abort;

               addr = inet_addr(iface->if_addr);
               if(host = gethostbyaddr((char *)&addr, 4, AF_INET))
               {
                  if((iface->if_flags & IFL_UseDomainName) && *host->h_name)
                     if(ptr = strchr(host->h_name, '.'))
                        add_server(&iface->if_domainnames, ++ptr, TRUE);
                  if((iface->if_flags & IFL_UseHostName) && *host->h_name)
                     strncpy(iface->if_hostname, host->h_name, sizeof(iface->if_hostname));
               }
               else
                  syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_TX_WarningCouldNotObtainHostname));
            }

            if(data->abort)   goto abort;

            if(iface->if_flags & IFL_IsOnline)
            {
               // add domainnames of all ifaces currently online
               if(iface->if_domainnames.mlh_TailPred != (struct MinNode *)&iface->if_domainnames)
               {
                  server = (struct ServerEntry *)iface->if_domainnames.mlh_TailPred;
                  while(server->se_node.mln_Pred)
                  {
                     syslog_AmiTCP(SocketBase, LOG_INFO, GetStr(MSG_SYSLOG_AddingDomainName), server->se_name);
                     if(amirexx_do_command("ADD START DOMAIN %ls", server->se_name) != RETURN_OK)
                        syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_TX_WarningSetDomain), server->se_name);
                     server = (struct ServerEntry *)server->se_node.mln_Pred;
                     if(data->abort)   goto abort;
                  }
               }
               // also add hostname of all ifaces currently online
               if(*iface->if_hostname)
               {
                  syslog_AmiTCP(SocketBase, LOG_INFO, GetStr(MSG_SYSLOG_AddingHostName), iface->if_hostname, iface->if_addr);
                  if(amirexx_do_command("ADD START HOST %ls %ls", iface->if_addr, iface->if_hostname) != RETURN_OK)
                     syslog_AmiTCP(SocketBase, LOG_WARNING, GetStr(MSG_TX_WarningSetHostname), iface->if_hostname, iface->if_addr);
                  WriteFile("ENV:HOSTNAME", iface->if_hostname, -1);
               }
            }

            if(data->abort)   goto abort;

            // now that dns, domainname and hostnames are set, query timeserver
            if(iface->if_flags & IFL_PutOnline)
            {
               if((iface->if_flags & (IFL_GetTime | IFL_SaveTime)) && *iface->if_timename)
               {
                  DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_QueryTimeserver), NULL);
                  sync_clock(SocketBase, iface->if_timename, (iface->if_flags & IFL_SaveTime));
               }
            }

            iface->if_flags &= ~IFL_PutOnline;
            if(data->abort)   goto abort;

            iface = (struct Interface *)iface->if_node.mln_Pred;
         }
      }
   }

   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_Finished), NULL);
   if(data->abort)   goto abort;
   DoMainMethod(win, MUIM_MainWindow_SetStates, NULL, NULL, NULL);
   success = TRUE;

   if(VUPBase) // abort previous transfer
   {
      VUP_Quit(vuphandle);
      CloseLibrary(VUPBase);
      VUPBase = NULL;
   }
   if(!(Config.cnf_flags & CFL_NoAutoTraffic))
   {
      if(VUPBase = OpenLibrary("vapor_update.library", 1))
         vuphandle = VUP_BeginCheckUpdate(10, VERHEXID, "GENESiS" VERTAG);
   }

abort:

// **********************************************************************
// NO DoMainMethod() beyond this point !! otherwise abort will block  !!!
// **********************************************************************

   if(iface_data) // there was an error
   {
      iface_deinit(iface_data);
      iface_free(SocketBase, iface_data);
      iface_data = NULL;

      if(iface)
      {
         if(iface->if_modemid > 0)
         {
            struct Modem *modem;

            if(modem = find_modem_by_id(&Config.cnf_modems, iface->if_modemid))
            {
               if(!(modem->mo_flags & MFL_DropDTR))
                  serial_hangup(modem, SocketBase);
            }
         }
      }

      if(!data->abort)
      {
         exec_event(&iface->if_events, IFE_OnlineFail);
         DoMethod(app, MUIM_Application_PushMethod, mw_data->GR_Led[(int)iface->if_userdata], 3, MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Red);
      }
   }

   if(!success && iface)
   {
      if(iface->if_loggerhandle && GenesisLoggerBase)
         GL_StopLogger(iface->if_loggerhandle);
      iface->if_loggerhandle = NULL;
      iface->if_flags &= ~IFL_PutOnline;
   }

   if(netinfo_win)
      DoMethod(app, MUIM_Application_PushMethod, netinfo_win, 1, MUIM_NetInfo_SetStates);

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
      NP_CloseInput , NULL,
      NP_CloseOutput, TRUE,
      NP_Input      , NULL,
	  NP_Output		, NULL,
//		NP_Output, Open("AmiTCP:log/GENESiS.log", MODE_NEWFILE),
//NP_Output, Open("CON:/300/640/200/GENESiS Netconfig/AUTO/WAIT/CLOSE", MODE_NEWFILE),
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
               MUIA_Text_Contents, "                                                         \n",
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

