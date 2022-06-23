/// includes & defines
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Online.h"
#include "mui_MainWindow.h"
#include "mui_Led.h"
#include "mui_NetInfo.h"
#include "protos.h"
#include "bootpc.h"
#include "iface.h"

#define LOOPBACK_NAME "lo0"
#define LOOPBACK_ADDR 0x7f000001

///
/// external variables
extern struct Library *GenesisBase;
extern int errno;
extern struct Config Config;
extern Object *app, *win, *status_win, *netinfo_win;
extern struct MUI_CustomClass  *CL_MainWindow, *CL_Online;
extern struct MsgPort *MainPort;
extern const char AmiTCP_PortName[];
extern BOOL dialup;
#ifdef NETCONNECT
extern struct Library *NetConnectBase;
#endif

///

/// config_lo0
BOOL config_lo0(VOID)
{
   struct Interface_Data *iface_data;

   if(iface_data = iface_alloc())
   {
      strcpy(iface_data->ifd_name, LOOPBACK_NAME);
      iface_setaddr(iface_data, LOOPBACK_ADDR);
      iface_free(iface_data);
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
BOOL config_complete(struct Interface_Data *iface_data, struct Interface *iface, struct ISP *isp)
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
            syslog_AmiTCP(LOG_DEBUG, "Using the default gateway address as destination address.");
            strcpy(iface->if_dst, iface->if_gateway);
         }
         // Use a fake IP address for the destination IP address if nothing else is available.
         else if(!is_secondary)
         {
            syslog_AmiTCP(LOG_DEBUG, "Using a 'fake' IP address 0.1.2.3 as the destination address.");
            strcpy(iface->if_dst, "0.1.2.3");
         }
         // NOTE: could try existing destination, existing gateway etc.
         else
         {
            syslog_AmiTCP(LOG_ERR, "Could not get destination IP address for a point-to-point link.");
            return(FALSE);
         }
      }
      // use destination as default gateway if not given
      if(!is_secondary)
      {
         if(!*iface->if_gateway)
         {
            if(*iface->if_dst)
            {
               syslog_AmiTCP(LOG_DEBUG, "Using the destination address as default gateway address.");
               strcpy(iface->if_gateway, iface->if_dst);
            }
            // could try existing destination, existing gateway etc.
         }
      }
   }

   // broadcast address will be set to default by AmiTCP

   // netmask will be set to default by AmiTCP if not present at all
   // warn if default netmask will be used on a broadcast interface
   if(!*iface->if_netmask && iface_data->ifd_flags & IFF_BROADCAST)
      syslog_AmiTCP(LOG_DEBUG, "Note: Default netmask will be used for the interface %ls", iface->if_name);

   // One additional validity test for the netmask we might now have the IP
   if(!netmask_check(inet_addr(iface->if_netmask), inet_addr(iface->if_addr)))
   {
      syslog_AmiTCP(LOG_ERR, "Invalid netmask value (%ls). Reverting to default.", iface->if_netmask);
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
/// TCPHandler
SAVEDS ASM VOID TCPHandler(register __a0 STRPTR args, register __d0 LONG arg_len)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct Interface *iface = NULL;
   struct Interface_Data *iface_data = NULL;
   BPTR fh;
   BOOL success = FALSE, is_secondary;
   char buffer[MAXPATHLEN];

   // open bsdsocket.library
#ifdef NETCONNECT
   SocketBase = NCL_OpenSocket();;
#else
   SocketBase = OpenLibrary("bsdsocket.library", 0);
#endif
   if(!SocketBase)
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorBsdsocketLib));
      goto abort;
   }

   if(SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno, SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno, TAG_END))
      syslog_AmiTCP(LOG_CRIT, "netconfig: could not set 'errno' ptr.");
   if((*(BYTE *)((struct Library *)SocketBase + 1)) & 0x04)
      dialup = 1;

   DeleteFile("ENV:APPPdns1");
   DeleteFile("ENV:APPPdns2");

   if(data->abort)   goto abort;

   if(mw_data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_ifaces)
   {
      iface = (struct Interface *)mw_data->isp.isp_ifaces.mlh_Head;
      is_secondary = FALSE;
      while(iface->if_node.mln_Succ && !data->abort)
      {
         if(iface->if_flags & IFL_PutOnline)
         {
            iface->if_flags &= ~IFL_PutOnline;
            if(!(iface->if_flags & IFL_IsOnline))
            {
               DoMainMethod(mw_data->GR_Led[(int)iface->if_userdata], MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Yellow, NULL);
               if(!(iface_data = iface_alloc()))
               {
                  syslog_AmiTCP(LOG_CRIT, GetStr(MSG_TX_ErrorIfaceAlloc));
                  goto abort;
               }

               if(data->abort)   goto abort;
               DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_InitDevice), NULL);
               if(data->abort)   goto abort;
               if(DoMainMethod(status_win, MUIM_Genesis_Get, (APTR)MUIA_Window_Open, NULL, NULL) && (Config.cnf_flags & CFL_ShowSerialInput))
                  DoMainMethod(data->TR_Terminal, TCM_INIT, NULL, NULL, NULL);
               if(data->abort)   goto abort;
               if(!(iface_init(iface_data, iface, &mw_data->isp, &Config)))
                  goto abort;

               // get appp.device's ms-dns addresses
               ReadFile("ENV:APPPdns1", buffer, 20);
               if(!find_server_by_name(&mw_data->isp.isp_nameservers, buffer))
                  add_server(&mw_data->isp.isp_nameservers, buffer);
               ReadFile("ENV:APPPdns2", buffer, 20);
               if(!find_server_by_name(&mw_data->isp.isp_nameservers, buffer))
                  add_server(&mw_data->isp.isp_nameservers, buffer);
#ifdef NETCONNECT
               NCL_CallMeSometimes();
#endif
               if(data->abort)   goto abort;

               if((mw_data->isp.isp_flags & ISF_UseBootp) && !is_secondary)
               {
                  struct bootpc *bpc;

                  DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_SendingBOOTP), NULL);
                  if(data->abort)   goto abort;
                  iface_prepare_bootp(iface_data, iface, &Config);
                  if(bpc = bootpc_create())
                  {
                     if(bootpc_do(bpc, iface_data, iface, &mw_data->isp))
                        DoMethod(app, MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_NoBOOTP));
                     else
                        DoMethod(app, MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_GotBOOTP));
                     bootpc_delete(bpc);
                  }
                  else
                     syslog_AmiTCP(LOG_CRIT, GetStr(MSG_TX_WarningBOOTPMemory));
                  iface_cleanup_bootp(iface_data, &Config);
               }

               if(data->abort)   goto abort;

//             if(!*iface->if_addr)
//             {
//                popup reuester here
//             }
//             if(data->abort)   goto abort;

               if(!*iface->if_addr)
               {
                  syslog_AmiTCP(LOG_CRIT, GetStr(MSG_TX_ErrorNoIP));
                  goto abort;
               }

               if(!config_complete(iface_data, iface, &mw_data->isp))
               {
                  syslog_AmiTCP(LOG_CRIT, "Not enough information to configure '%ls'.", iface->if_name);
                  goto abort;
               }

               DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConfiguringAmiTCP), NULL);
               if(data->abort)   goto abort;

#ifdef NETCONNECT
               NCL_CallMeSometimes();
#endif
               if(!(iface_config(iface_data, iface, &Config)))
               {
                  syslog_AmiTCP(LOG_CRIT, GetStr(MSG_TX_ErrorConfigIface), iface->if_name);
                  goto abort;
               }

               if(data->abort)   goto abort;

               if(!is_secondary)
               {
                  struct ServerEntry *server;

                  if(!dialup)
                  {
                     if(!*iface->if_gateway)
                        syslog_AmiTCP(LOG_DEBUG, "Default gateway information not found.");
                     else
                        route_add(iface_data->ifd_fd, RTF_UP|RTF_GATEWAY, INADDR_ANY, inet_addr(iface->if_gateway), TRUE /* force */);
                  }
                  if(mw_data->isp.isp_nameservers.mlh_TailPred == (struct MinNode *)&mw_data->isp.isp_nameservers)
                  {
                     syslog_AmiTCP(LOG_WARNING, GetStr(MSG_TX_WarningNoDNS));
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
                           syslog_AmiTCP(LOG_ERR, GetStr(MSG_TX_ErrorSetDNS), server->se_name);
                        server = (struct ServerEntry *)server->se_node.mln_Pred;
                        if(data->abort)   goto abort;
                     }
                  }

                  if(data->abort)   goto abort;

                  if(!(mw_data->isp.isp_flags & ISF_DontQueryHostname))
                  {
                     // is hostname or domainname missing but got rest ?
                     if((*iface->if_addr && (mw_data->isp.isp_nameservers.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_nameservers)) &&
                        (!*mw_data->isp.isp_hostname || mw_data->isp.isp_domainnames.mlh_TailPred == (struct MinNode *)&mw_data->isp.isp_domainnames))
                     {
                        DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_GetDomain), NULL);
                        if(data->abort)   goto abort;
                        if(!*mw_data->isp.isp_hostname)
                        {
                           if(gethostname(mw_data->isp.isp_hostname, sizeof(mw_data->isp.isp_hostname)) || !*mw_data->isp.isp_hostname)
                              syslog_AmiTCP(LOG_WARNING, GetStr(MSG_TX_WarningNoHostname));
                        }
                        if(mw_data->isp.isp_domainnames.mlh_TailPred == (struct MinNode *)&mw_data->isp.isp_domainnames && *mw_data->isp.isp_hostname)
                        {
                           STRPTR ptr;

                           if(ptr = strchr(mw_data->isp.isp_hostname, '.'))
                           {
                              ptr++;
                              add_server(&mw_data->isp.isp_domainnames, ptr);
                           }
                        }
                     }

                     if(data->abort)   goto abort;

                     if(mw_data->isp.isp_domainnames.mlh_TailPred == (struct MinNode *)&mw_data->isp.isp_domainnames)
                        syslog_AmiTCP(LOG_WARNING, GetStr(MSG_TX_WarningNoDomain));
                  }
                  if(data->abort)   goto abort;

                  if(mw_data->isp.isp_domainnames.mlh_TailPred != (struct MinNode *)&mw_data->isp.isp_domainnames)
                  {
                     server = (struct ServerEntry *)mw_data->isp.isp_domainnames.mlh_TailPred;
                     while(server->se_node.mln_Pred)
                     {
                        if(amirexx_do_command("ADD START DOMAIN %ls", server->se_name) != RETURN_OK)
                           syslog_AmiTCP(LOG_WARNING, GetStr(MSG_TX_WarningSetDomain), server->se_name);
                        server = (struct ServerEntry *)server->se_node.mln_Pred;
                        if(data->abort)   goto abort;
                     }
                  }
                  if(*mw_data->isp.isp_hostname)
                  {
                     if(amirexx_do_command("ADD START HOST %ls %ls", iface->if_addr, mw_data->isp.isp_hostname) != RETURN_OK)
                        syslog_AmiTCP(LOG_WARNING, GetStr(MSG_TX_WarningSetHostname), mw_data->isp.isp_hostname);
                     WriteFile("ENV:HOSTNAME", mw_data->isp.isp_hostname, -1);
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
               } // end !is_secondary

               iface_deinit(iface_data);
               iface_free(iface_data);
               iface_data = NULL;

               DoMainMethod(mw_data->GR_Led[(int)iface->if_userdata], MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Green, NULL);
               if(netinfo_win)
                  DoMethod(netinfo_win, MUIM_NetInfo_Redraw);
               syslog_AmiTCP(LOG_NOTICE, "%ls is now online", iface->if_name);
               iface->if_flags |= IFL_IsOnline;
               exec_event(&iface->if_events, IFE_Online);
            }
         }
         iface = (struct Interface *)iface->if_node.mln_Succ;
         is_secondary = TRUE;
      }
   }

   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_Finished), NULL);
   if(data->abort)   goto abort;
   DoMainMethod(win, MUIM_MainWindow_SetStates, NULL, NULL, NULL);
   success = TRUE;

   if((mw_data->isp.isp_flags & (ISF_GetTime | ISF_SaveTime)) && *mw_data->isp.isp_timename)
   {
      sprintf(buffer, "C:Execute AmiTCP:bin/SynClock %ls%ls", mw_data->isp.isp_timename, (mw_data->isp.isp_flags & ISF_SaveTime ? " SAVE" : ""));
      run_async(buffer);
   }

abort:

// **********************************************************************
// NO DoMainMethod() beyond this point !! otherwise abort will block  !!!
// **********************************************************************

    if(iface_data)
    {
       iface_deinit(iface_data);
       iface_free(iface_data);
       iface_data = NULL;
       if(!data->abort)
       {
          exec_event(&iface->if_events, IFE_OnlineFail);
          DoMethod(app, MUIM_Application_PushMethod, mw_data->GR_Led[(int)iface->if_userdata], 3, MUIM_Set, (APTR)MUIA_Group_ActivePage, (APTR)MUIV_Led_Red);
          if(netinfo_win)
             DoMethod(netinfo_win, MUIM_NetInfo_Redraw);
       }
    }

   if(SocketBase)
      CloseLibrary(SocketBase);
   SocketBase = NULL;

   IsOnline((is_one_online(&mw_data->isp.isp_ifaces) ? 22 : -22));

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
      NP_Output, Open("AmiTCP:log/GENESiS.log", MODE_NEWFILE),
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

