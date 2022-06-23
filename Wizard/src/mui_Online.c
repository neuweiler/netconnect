/*
   1) iface_init puts iface online
   2) sana_getaddresses gets ip addr into iface->if_addr (if ppp or configured in sana2config)
   3) iface_config sets ip addr from iface->if_addr
      iface_data->ifd_addr is only used to delete previous config and in bootp stuff
*/

/// includes
#include "/includes.h"

#include "Strings.h"
#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "mui.h"
#include "mui_Online.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "bootpc.h"
#include "sana.h"
#include "iface.h"

///
/// external variables
extern struct Library *MUIMasterBase, *SocketBase, *GenesisBase;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_Online;
extern struct Library *SocketBase;
extern struct Config Config;
extern int errno;
extern int addr_assign, dst_assign, dns_assign, domainname_assign, gw_assign;
extern struct MsgPort *MainPort;
extern Object *win, *status_win, *app;
extern char serial_in[], serial_buffer[];
extern WORD ser_buf_pos;
extern BOOL dialup;
extern const char AmiTCP_PortName[];
extern struct Interface Iface;
extern struct Library *NetConnectBase;

///

/// get_request
BOOL get_request(STRPTR text, STRPTR buffer, struct Process *proc)
{
   ULONG sigs;

   DoMainMethod(win, MUIM_MainWindow_Request, text, buffer, proc);
   sigs = Wait(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);

   if((sigs & SIGBREAKF_CTRL_D) && *buffer)
      return(TRUE);

   return(FALSE);
}

///
/// is_inaddr_any
BOOL is_inaddr_any(STRPTR addr)
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
BOOL config_complete(struct Interface_Data *iface_data, struct Interface *iface)
{
   // Special checks for point-to-point interfaces
   if(!dialup && iface_data->ifd_flags & IFF_POINTOPOINT)
   {
      // try to find destination IP for a p-to-p link if not given
      if(is_inaddr_any(iface->if_dst))
      {
         if(!is_inaddr_any(iface->if_gateway))
         {
            syslog_AmiTCP(LOG_DEBUG, "Using the default gateway address as destination address.");
            strncpy(iface->if_dst, iface->if_gateway, sizeof(iface->if_dst));
         }
         // Use a fake IP address for the destination IP address if nothing else is available.
         else
         {
            syslog_AmiTCP(LOG_DEBUG, "Using the local IP address %ls as a 'fake' destination IP address.", iface->if_addr);
            strncpy(iface->if_dst, iface->if_addr, sizeof(iface->if_dst));
            dst_assign = addr_assign;
         }
      }
      // use destination as default gateway if not given
      if(is_inaddr_any(iface->if_gateway))
      {
         if(!is_inaddr_any(iface->if_dst))
         {
            syslog_AmiTCP(LOG_DEBUG, "Using the destination address as default gateway address.");
            strncpy(iface->if_gateway, iface->if_dst, sizeof(iface->if_gateway));
            gw_assign = dst_assign;
         }
         // could try existing destination, existing gateway etc.
      }
   }

   // broadcast address will be set to default by AmiTCP

   // netmask will be set to default by AmiTCP if not present at all
   // warn if default netmask will be used on a broadcast interface
   if(is_inaddr_any(iface->if_netmask) && (iface_data->ifd_flags & IFF_BROADCAST))
      syslog_AmiTCP(LOG_DEBUG, "Note: Default netmask will be used for the interface %ls", iface->if_name);

   // One additional validity test for the netmask we might now have the IP
   if(!netmask_check(inet_addr(iface->if_netmask), inet_addr(iface->if_addr)))
   {
      syslog_AmiTCP(LOG_ERR, "Invalid netmask value (%ls). Reverting to default.", iface->if_netmask);
      *iface->if_netmask = NULL;
   }

   return(TRUE);
}

///
/// TCPHandler
SAVEDS ASM VOID TCPHandler(register __a0 STRPTR args, register __d0 LONG arg_len)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct Interface_Data *iface_data = NULL;
   struct bootpc *bpc;
   struct ServerEntry *server;
   BPTR fh;
   BOOL success = FALSE;
   char buffer[81];

   *Iface.if_addr    = NULL;
   *Iface.if_dst     = NULL;
   *Iface.if_gateway = NULL;
   strcpy(Iface.if_netmask, "255.255.255.255");
   Iface.if_MTU = 1500;
   *Iface.if_hostname = NULL;
   Iface.if_flags |= (IFL_UseHostName | IFL_UseDomainName);
   clear_list(&Iface.if_nameservers);
   clear_list(&Iface.if_domainnames);
   addr_assign = dst_assign = dns_assign = domainname_assign = gw_assign = NULL;

   if(fh = Open("AmiTCP:db/resolv.conf", MODE_NEWFILE))
   {
      FPrintf(fh, "; This file is built dynamically - do not edit\n");
      Close(fh);
   }

   // launch AmiTCP if it isn't already running
   if(!FindPort((STRPTR)AmiTCP_PortName))
   {
      if(!launch_amitcp())
      {
         DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorLaunchAmiTCP));
         goto abort;
      }
   }
   if(data->abort)   goto abort;

   // open bsdsocket.library and ifconfig.library
   SocketBase = (struct Library *)NCL_OpenSocket();;
   if(!SocketBase)
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorBsdsocketLib));
      goto abort;
   }

   if(SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno, SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno, TAG_END))
      Printf("WARINING: Could not set errno ptr.");
   if((*(BYTE *)((struct Library *)SocketBase + 1)) & 0x04)
      dialup = 1;

   if(data->abort)   goto abort;

   DeleteFile("ENV:APPPdns1");
   DeleteFile("ENV:APPPdns2");
   if(fh = CreateDir("ENV:NetConfig"))
      UnLock(fh);

   if(fh = Open("ENV:NetConfig/AutoInterfaces", MODE_NEWFILE))
   {
      FPrintf(fh, "%ls DEV=%ls UNIT=%ld", Iface.if_name, Iface.if_sana2device, Iface.if_sana2unit);
      if(Iface.if_configparams)
         FPrintf(fh, " %ls", Iface.if_configparams);

      Close(fh);
   }
   else
   {
      Printf("ERROR: Could not create ENV:NetConfig/Autointerfaces.\n");
      goto abort;
   }

   if(data->abort)   goto abort;

   if(!is_inaddr_any(Iface.if_addr) && !addr_assign)
      addr_assign  = ASSIGN_Static;

   if(!(iface_data = iface_alloc()))
   {
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorIfaceAlloc), NULL);
      goto abort;
   }

   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_InitIface), NULL);
   if(data->abort)   goto abort;
Printf("init iface\n");
   NCL_CallMeSometimes();
   if(!(iface_init(iface_data, &Iface)))
      goto abort;
Printf("iface initialized\n");

   // get appp.device's ms-dns addresses
   if(ReadFile("ENV:APPPdns1", buffer, 20) > 0)
   {
      add_server(&Iface.if_nameservers, buffer);
      Iface.if_flags |= IFL_UseNameServer;
   }
   if(ReadFile("ENV:APPPdns2", buffer, 20) > 0)
   {
      add_server(&Iface.if_nameservers, buffer);
      Iface.if_flags |= IFL_UseNameServer;
   }

   if(!is_inaddr_any(Iface.if_addr) && !addr_assign)
      addr_assign = ASSIGN_IFace;
   if(!is_inaddr_any(Iface.if_gateway) && !gw_assign)
      gw_assign = ASSIGN_IFace;
   if(!is_inaddr_any(Iface.if_dst) && !dst_assign)
      dst_assign = ASSIGN_IFace;
   if((Iface.if_nameservers.mlh_TailPred != (struct MinNode *)&Iface.if_nameservers) && !dns_assign)
      dns_assign = ASSIGN_IFace;

   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_SendingBOOTP), NULL);
   if(data->abort)   goto abort;
Printf("preparing BOOTP\n");

   iface_prepare_bootp(iface_data, &Iface);
   if(bpc = bootpc_create())
   {
Printf("do BOOTP\n");
      if(bootpc_do(bpc, iface_data, &Iface))
      {
         Printf("BOOTP failed to have an answer.\n");
         if(data->abort)   goto abort;
         DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_NoBOOTP), NULL);
         Iface.if_flags &= ~IFL_BOOTP;
      }
      else
      {
         if(data->abort)   goto abort;
         DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_GotBOOTP), NULL);
         Iface.if_flags |= IFL_BOOTP;
      }
Printf("delete BOOTP\n");

      bootpc_delete(bpc);
   }
   else
      Printf("WARNING: No memory for BOOTP request !\n");
Printf("Cleanup BOOTP\n");
   iface_cleanup_bootp(iface_data);

   if(data->abort)   goto abort;

   if(!is_inaddr_any(Iface.if_addr) && !addr_assign)
      addr_assign = ASSIGN_BOOTP;
   if(!is_inaddr_any(Iface.if_dst) && !dst_assign)
      dst_assign = ASSIGN_BOOTP;
   if(!is_inaddr_any(Iface.if_gateway) && !gw_assign)
      gw_assign = ASSIGN_BOOTP;
   if((Iface.if_nameservers.mlh_TailPred != (struct MinNode *)&Iface.if_nameservers) && !dns_assign)
      dns_assign = ASSIGN_BOOTP;
   if((Iface.if_domainnames.mlh_TailPred != (struct MinNode *)&Iface.if_domainnames) && !domainname_assign)
      domainname_assign = ASSIGN_BOOTP;

   if(data->abort)   goto abort;

   if(is_inaddr_any(Iface.if_addr))
   {
Printf("IP is missing. Use requester\n");
      get_request(GetStr(MSG_TX_NoIPReq), Iface.if_addr, data->TCPHandler);
      addr_assign = ASSIGN_Static;
   }
Printf("IP : %ls\n", Iface.if_addr);

   if(data->abort)   goto abort;

   if(is_inaddr_any(Iface.if_addr) && !data->abort)
   {
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorNoIP), NULL);
      goto abort;
   }

Printf("check if config is complete\n");
   if(!config_complete(iface_data, &Iface))
   {
      syslog_AmiTCP(LOG_CRIT, "Not enough information to configure '%ls'.", Iface.if_name);
      goto abort;
   }
   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConfiguringAmiTCP), NULL);
   if(data->abort)   goto abort;
   NCL_CallMeSometimes();
Printf("configure interface\n");

   if(!(iface_config(iface_data, &Iface)))
   {
      sprintf(buffer, GetStr(MSG_TX_ErrorConfigIface), Iface.if_name);
      if(!data->abort)
         DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Abort), buffer, NULL);
      goto abort;
   }

   if(data->abort)   goto abort;

   if(!dialup)
   {
Printf("add route to def gateway: %ls\n", Iface.if_gateway);
      if(is_inaddr_any(Iface.if_gateway))
         syslog_AmiTCP(LOG_WARNING, "Default gateway information not found.");
      else
         route_add(iface_data->ifd_fd, RTF_UP|RTF_GATEWAY, INADDR_ANY, inet_addr(Iface.if_gateway), TRUE /* force */);
   }

   if(data->abort)   goto abort;

Printf("de-initializing iface\n");
   iface_deinit(iface_data);
Printf("free iface\n");
   iface_free(iface_data);
   iface_data = NULL;

Printf("checking nameservers\n");
   if(Iface.if_nameservers.mlh_TailPred == (struct MinNode *)&Iface.if_nameservers)
   {
      while(get_request(GetStr(MSG_TX_NoDNSReq), buffer, data->TCPHandler))
         add_server(&Iface.if_nameservers, buffer);
      dns_assign = ASSIGN_Static;
   }

   if(Iface.if_nameservers.mlh_TailPred == (struct MinNode *)&Iface.if_nameservers)
   {
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Okay), (APTR)GetStr(MSG_TX_WarningNoDNS), NULL);
      add_server(&Iface.if_nameservers, "198.41.0.4");
      add_server(&Iface.if_nameservers, "128.9.0.107");
      Iface.if_flags &= ~IFL_UseHostName;
      Iface.if_flags &= ~IFL_UseDomainName;
      dns_assign = ASSIGN_Root;
   }

   // now take care of resolv
   amirexx_do_command("RESET RESOLV");

Printf("tell amitcp about nameservers\n");
   // add in reverse order since each entry is added to top of list
   if(Iface.if_nameservers.mlh_TailPred != (struct MinNode *)&Iface.if_nameservers)
   {
      server = (struct ServerEntry *)Iface.if_nameservers.mlh_TailPred;
      while(server->se_node.mln_Pred)
      {
         if(amirexx_do_command("ADD START NAMESERVER %ls", server->se_name) != RETURN_OK)
            syslog_AmiTCP(LOG_ERR, GetStr(MSG_TX_ErrorSetDNS), server->se_name);
         server = (struct ServerEntry *)server->se_node.mln_Pred;
         if(data->abort)   goto abort;
      }
   }

   if(data->abort)   goto abort;
Printf("check hostname / domainname\n");

   // is hostname or domainname missing but got rest ?
   if(Iface.if_flags & (IFL_UseHostName | IFL_UseDomainName))
   {
      struct hostent *host;
      STRPTR ptr;
      ULONG addr;

      DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_GetDomain), NULL);
      if(data->abort)   goto abort;

      addr = inet_addr(Iface.if_addr);
      if(host = gethostbyaddr((char *)&addr, 4, AF_INET))
      {
         if((Iface.if_flags & IFL_UseDomainName) && *host->h_name)
            if(ptr = strchr(host->h_name, '.'))
               add_server(&Iface.if_domainnames, ++ptr);
         if((Iface.if_flags & IFL_UseHostName) && *host->h_name)
            strncpy(Iface.if_hostname, host->h_name, sizeof(Iface.if_hostname));
      }
   }
   if(data->abort)   goto abort;

   if(Iface.if_domainnames.mlh_TailPred == (struct MinNode *)&Iface.if_domainnames)
   {
Printf("No domainname. Use requester.\n");
      if(get_request(GetStr(MSG_TX_NoDomainnameReq), buffer, data->TCPHandler))
      {
         add_server(&Iface.if_domainnames, buffer);
         domainname_assign = ASSIGN_Static;
      }
   }
Printf("tell amitcp about domainnames\n");
   if(Iface.if_domainnames.mlh_TailPred != (struct MinNode *)&Iface.if_domainnames)
   {
      server = (struct ServerEntry *)Iface.if_domainnames.mlh_TailPred;
      while(server->se_node.mln_Pred)
      {
         if(amirexx_do_command("ADD START DOMAIN %ls", server->se_name) != RETURN_OK)
Printf("could not tell AmiTCP about domainname '%ls'.\n", server->se_name);
         server = (struct ServerEntry *)server->se_node.mln_Pred;
         if(data->abort)   goto abort;
      }
   }
   else
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_WarningNoDomain), NULL);

   if(data->abort)   goto abort;
Printf("tell amitcp about hostname: %ls\n", Iface.if_hostname);

   if(*Iface.if_hostname)
   {
      if(amirexx_do_command("ADD START HOST %ls %ls", Iface.if_addr, Iface.if_hostname) != RETURN_OK)
Printf("could not tell AmiTCP about hostname '%ls'.\n", Iface.if_hostname);
   }
   else
      Printf("WARNING: Got no hostname !\n");

   if(data->abort)   goto abort;
Printf("save new resolv.conf\n");

   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_Finished), NULL);
   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ClosingConnection), NULL);
   success = TRUE;
Printf("everything went well.\n");

abort:
Printf("reached 'abort'\n");

// **********************************************************************
// NO DoMainMethod() beyond this point !! other wise abort will block !!!
// **********************************************************************
   if(iface_data)
   {
      if(iface_data->ifd_s2)
      {
Printf("putting sana2 dev offline\n");
         sana2_offline(iface_data->ifd_s2);
      }
Printf("deinit iface\n");
      iface_deinit(iface_data);
Printf("free iface\n");
      iface_free(iface_data);
      iface_data = NULL;
   }
   else
   {
      struct sana2 *s2;

Printf("re-open sana2 device (%ls / %ld)\n", Iface.if_sana2device, Iface.if_sana2unit);
      if(s2 = sana2_create(Iface.if_sana2device, Iface.if_sana2unit))
      {
Printf("put it offline\n");
         sana2_offline(s2);
Printf("close sana2 dev\n");
         sana2_delete(s2);
      }
   }
Printf("close socketbase\n");

   if(SocketBase)
      CloseLibrary(SocketBase);
   SocketBase = NULL;

Printf("kill amitcp\n");
   amirexx_do_command("KILL");

Printf("hangup modem\n");
   DoMainMethod(app, MUIM_Serial_HangUp, NULL, NULL, NULL);

Printf("done\n");
   // go to next page or start serial reading and add inputhandler by misusing setpage
   if(success)
      mw_data->Page = PAGE_Finished;
   DoMethod(app, MUIM_Application_PushMethod, win, 1, MUIM_MainWindow_SetPage);
Printf("remove child process\n");

   Forbid();
   data->TCPHandler = NULL;
   if(!data->abort)
      DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, status_win);
}

///

/// Online_GoOnline
ULONG Online_GoOnline(struct IClass *cl, Object *obj, Msg msg)
{
   struct Online_Data *data = INST_DATA(cl, obj);
//   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

   serial_stopread();         // otherwise GoOnline would be called indefinitely
   serial_buffer[0] = NULL;   // when a ppp_frame arrived (see MainWindow_Trigger)
   ser_buf_pos = 0;

   if(data->TCPHandler = CreateNewProcTags(
      NP_Entry       , TCPHandler,
      NP_Name        , "GENESiSWizard Netconfig",
      NP_StackSize   , 16384,
      NP_WindowPtr   , -1,
      NP_CloseOutput , TRUE,
      NP_Output      , Open("AmiTCP:log/GENESiSWizard.log", MODE_NEWFILE),
//NP_Output, Open("CON:/100/640/200/GENESiSWizard Netconfig/AUTO/WAIT/CLOSE", MODE_NEWFILE),
      TAG_END))
   {
      return(TRUE);
   }
   else
   {
      MUI_Request(_app(obj), obj, NULL, NULL, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorLaunchSubtask));
      DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);
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
      HandleMainMethod(MainPort);
      Delay(20);
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///
/// Online_New
ULONG Online_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Online_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title        , GetStr(MSG_LA_PleaseWait),
      MUIA_Window_CloseGadget  , FALSE,
      MUIA_Window_RefWindow    , win,
      MUIA_Window_LeftEdge     , MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge      , MUIV_Window_TopEdge_Centered,
      WindowContents          , VGroup,
         Child, tmp.TX_Info = TextObject,
            MUIA_Text_Contents, GetStr(MSG_TX_GatherNetInfo),
            MUIA_Text_PreParse, "\033c",
         End,
         Child, tmp.BU_Busy = BusyObject,
            MUIA_FixHeight, 6,
         End,
         Child, tmp.BT_Abort = MakeButton(MSG_BT_Abort),
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Online_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->TCPHandler  = NULL;
      data->abort       = FALSE;

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
      case OM_NEW               : return(Online_New         (cl, obj, (APTR)msg));
      case OM_DISPOSE           : return(Online_Dispose     (cl, obj, (APTR)msg));
      case MUIM_Online_GoOnline : return(Online_GoOnline    (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

