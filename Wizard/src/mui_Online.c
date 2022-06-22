#include "globals.c"
#include "bootpc.h"
#include "iface.h"

/// amirexx_do_command
static const char AmiTCP_PortName[] = "AMITCP";

LONG amirexx_do_command(const char *fmt, ...)
{
   char buf[256];
   struct MsgPort *port; /* our reply port */
   struct MsgPort *AmiTCP_Port;
   struct RexxMsg *rmsg;
   LONG rc = 20;         /* fail */

   vsprintf(buf, fmt, (STRPTR)(&fmt + 1));

   if(port = CreateMsgPort())
   {
      port->mp_Node.ln_Name = "BOOTPCONFIG";
      if(rmsg = CreateRexxMsg(port, NULL, (STRPTR)AmiTCP_PortName))
      {
         rmsg->rm_Action = RXCOMM;
         rmsg->rm_Args[0] = (STRPTR)buf;
         if(FillRexxMsg(rmsg, 1, 0))
         {
            Forbid();
            if(AmiTCP_Port = FindPort((STRPTR)AmiTCP_PortName))
            {
               PutMsg(AmiTCP_Port, (struct Message *)rmsg);
               Permit();
               do
               {
                  WaitPort(port);
               } while(GetMsg(port) != rmsg);
               rc = rmsg->rm_Result1;
            }
            else
               Permit();

            ClearRexxMsg(rmsg, 1);
         }
         DeleteRexxMsg(rmsg);
      }
      DeleteMsgPort(port);
   }
   return(rc);
}

///
/// TCPHandler
SAVEDS ASM VOID TCPHandler(REG(a0) STRPTR args, REG(d0) LONG arg_len)
{
   Object *obj;
   struct Online_Data *data;
   struct iface *iface;
   struct bootpc *bpc;
   BPTR fh;
   int i;

   if(!(obj = (Object *)atol(args)))
   {
      MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorTCPHandlerArgs));
      return;
   }
   data = INST_DATA(CL_Online->mcc_Class, obj);

   ObtainSemaphore(&data->HandlerSemaphore);

   DeleteFile("ENV:APPdns1");
   DeleteFile("ENV:APPdns2");

   /* create env:NetConfig if necessary */
   if((fh = CreateDir("ENV:NetConfig")) != NULL)
      UnLock(fh);

   /* create env:netconfig/autointerfaces */
   if(fh = Open("ENV:NetConfig/AutoInterfaces", MODE_NEWFILE))
   {
      if(!strcmp(Config.cnf_ifname, "ppp"))
      {
FPrintf(fh, "ppp DEV=DEVS:Networks/appp.device UNIT=0 DoOffline ConfigFileName=ENV:Sana2/appp0.config ConfigFileContents=\"%ls %ld 38400 Shared %ls CD 7Wire MTU=1500 ALLOWPAP=YES ALLOWCHAPMS=YES ALLOWCHAPMD5=YES USERID=%ls PASSWORD=%ls\"", Config.cnf_serialdevice, Config.cnf_serialunit, data->addr_ptr, data->login_ptr, data->passwd_ptr);
//         FPrintf(fh, "ppp DEV=DEVS:Networks/appp.device UNIT=0 DoOffline ConfigFileName=ENV:Sana2/appp0.config ConfigFileContents=\"");
//         FPrintf(fh, "sername %ls\\n", Config.cnf_serialdevice);
//         FPrintf(fh, "serunit %ld\\n", Config.cnf_serialunit);
//         FPrintf(fh, "serbaud 38400\\n");
//         FPrintf(fh, "cd yes\\n");
//         FPrintf(fh, "localipaddress %ls\\n", data->addr_ptr);
//         FPrintf(fh, "user %ls\\n", data->login_ptr);
//         FPrintf(fh, "secret %ls\\n\"", data->passwd_ptr);
      }
      else if(!strcmp(Config.cnf_ifname, "slip"))
         FPrintf(fh, "slip DEV=DEVS:Networks/aslip.device UNIT=0 DoOffline ConfigFileName=ENV:Sana2/aslip0.config ConfigFileContents=\"%ls %ld 38400 Shared %ls CD 7Wire MTU=1500\"", Config.cnf_serialdevice, Config.cnf_serialunit, data->addr_ptr);
      else
         FPrintf(fh, "%ls DEV=%ls UNIT=%ld P2P NOARP IPTYPE=2048 IPREQ=32 WRITEREQ=32", Config.cnf_ifname, Config.cnf_sana2device, Config.cnf_sana2unit);

      Close(fh);
   }
   else
   {
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorAutoInterfaces));
      goto abort;
   }

   if(data->abort)   goto abort;

   /* launch amitcp if it isn't already running */
   if(!FindPort("AMITCP"))
   {
      BPTR ofh, ifh;

      DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_LaunchingAmiTCP));

      if(ofh = Open("NIL:", MODE_NEWFILE))
      {
         if(ifh = Open("NIL:", MODE_OLDFILE))
         {
            if(SystemTags("AmiTCP:AmiTCP",
               SYS_Output     , ofh,
               SYS_Input      , ifh,
               SYS_Asynch     , TRUE,
               SYS_UserShell  , TRUE,
               NP_StackSize   , 8192,
               NP_Priority    , 0,
               TAG_DONE) == -1)
            {
               Close(ofh);
               Close(ifh);
               MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorAmiTCPLaunch));
               goto abort;
            }
         }
      }
   }

   /* wait until amitcp is running */
   i = 0;
   while(!FindPort("AMITCP") && i++ < 40 && !data->abort)
      Delay(25);

   if(data->abort)   goto abort;

   /* open bsdsocket.library and ifconfig.library */
   if(!(SocketBase = OpenLibrary("bsdsocket.library", 0)))
   {
      MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorBsdsocketLib));
      goto abort;
   }

   if(SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno, SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno, TAG_END))
      Printf("ERROR: Could not set errno ptr. Please report this to your support site.");

   if(!(IfConfigBase = OpenLibrary(IFCONFIGNAME, 0L)))
   {
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorIfConfigLib));
      goto abort;
   }

   Config.cnf_netmask   = inet_addr("255.255.255.0");
   Config.cnf_addr      = inet_addr(data->addr_ptr);
   Config.cnf_MTU       = 1500;

   if(Config.cnf_addr != INADDR_ANY && !addr_assign)
      addr_assign  = CNF_Assign_Static;

   if(!(iface = iface_alloc()))
   {
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorIfaceAlloc));
      goto abort;
   }

   if(data->abort)   goto abort;

   DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_InitDevice));

   if(!(iface_init(iface, &Config)))
   {
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorIfaceInit));
      goto abort;
   }

   /* get appp.device's ms-dns addresses */
   if(GetEnvDOS("APPPdns1", dns1, 20))
   {
      if(strcmp(dns1, "0.0.0.0"))
         Config.cnf_dns1 = inet_addr(dns1);
      Printf("MS-DNS1: %ls\n", dns1);
   }
   if(GetEnvDOS("APPPdns2", dns2, 20))
   {
      if(strcmp(dns2, "0.0.0.0"))
         Config.cnf_dns2 = inet_addr(dns2);
      Printf("MS-DNS2: %ls\n", dns2);
   }

   if(Config.cnf_addr != INADDR_ANY && !addr_assign)
      addr_assign = CNF_Assign_IFace;
   if(Config.cnf_dst != INADDR_ANY && !dst_assign)
      dst_assign = CNF_Assign_IFace;
   if((Config.cnf_dns1 != INADDR_ANY || Config.cnf_dns2 != INADDR_ANY) && !dns_assign)
      dns_assign = CNF_Assign_IFace;

   if(data->abort)   goto abort;

   DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_SendingBOOTP));

   iface_prepare_bootp(iface, &Config);
   if(bpc = bootpc_create())
   {
      if(bootpc_do(bpc, iface, &Config))
      {
         Printf("BOOTP failed to have an answer.\n");
         DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_NoBOOTP));
      }
      else
         DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_GotBOOTP));
      bootpc_delete(bpc);
      Delay(50);
   }
   else
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningBOOTPMemory));
   iface_cleanup_bootp(iface, &Config);

   if(Config.cnf_addr != INADDR_ANY && !addr_assign)
      addr_assign = CNF_Assign_BootP;
   if(Config.cnf_dst != INADDR_ANY && !dst_assign)
      dst_assign = CNF_Assign_BootP;
   if((Config.cnf_dns1 != INADDR_ANY || Config.cnf_dns2 != INADDR_ANY) && !dns_assign)
      dns_assign = CNF_Assign_BootP;
   if(*Config.cnf_domainname && !domainname_assign)
      domainname_assign = CNF_Assign_BootP;

   if(data->abort)   goto abort;

   if(Config.cnf_addr == INADDR_ANY)
   {
// popup reuester here
   }

   if(data->abort)   goto abort;

   if(Config.cnf_addr == INADDR_ANY)
   {
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorNoIP));
      goto abort;
   }

   DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_ConfiguringAmiTCP));

   if(!(iface_config(iface, &Config)))
   {
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorConfigIface), iface->iface_name);
      goto abort;
   }

   if(amirexx_do_command("RESET RESOLV") != RETURN_OK)
   {
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorResetResolv));
      goto abort;
   }

   if(Config.cnf_dns1 == INADDR_ANY && Config.cnf_dns2 == INADDR_ANY)
   {
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningNoDNS));
      Config.cnf_dns1 = inet_addr("198.41.0.4");
      Config.cnf_dns2 = inet_addr("128.9.0.107");
      dns_assign = CNF_Assign_Root;
   }

   /* add in reverse order since each entry is added to top of list */
   if(Config.cnf_dns2 != INADDR_ANY)
   {
      if(amirexx_do_command("ADD START NAMESERVER %ls", Inet_NtoA(Config.cnf_dns2)) != RETURN_OK)
         MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorSetDNS), Inet_NtoA(Config.cnf_dns2));
   }
   if(Config.cnf_dns1 != INADDR_ANY)
   {
      if(amirexx_do_command("ADD START NAMESERVER %ls", Inet_NtoA(Config.cnf_dns1)) != RETURN_OK)
      {
         MUI_Request(0,0,0,0, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorSetDNS), Inet_NtoA(Config.cnf_dns1));
         goto abort;
      }
   }

   /* is hostname or domainname missing but got rest ? */
   if((Config.cnf_addr != INADDR_ANY && (Config.cnf_dns1 != INADDR_ANY || Config.cnf_dns2 != INADDR_ANY)) &&
      (!*Config.cnf_hostname || !*Config.cnf_domainname))
   {
      DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_GetDomain));

      if(!*Config.cnf_hostname)
      {
         if(gethostname(Config.cnf_hostname, sizeof(Config.cnf_hostname)) || !*Config.cnf_hostname)
            MUI_Request(0,0,0,0, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningNoHostname));
      }
      if(!*Config.cnf_domainname && *Config.cnf_hostname)
      {
         STRPTR ptr;

         if(ptr = strchr(Config.cnf_hostname, '.'))
         {
            ptr++;
            strcpy(Config.cnf_domainname, ptr);
         }
      }
   }

   if(*Config.cnf_domainname && !domainname_assign)
      domainname_assign = CNF_Assign_DNSQuery;

   if(!*Config.cnf_domainname)
      MUI_Request(0,0,0,0, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningNoDomain));
   else
   {
      if(amirexx_do_command("ADD START DOMAIN %ls", Config.cnf_domainname) != RETURN_OK)
         MUI_Request(0,0,0,0, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningSetDomain), Config.cnf_domainname);
   }

   if(*Config.cnf_hostname)
   {
      if(amirexx_do_command("ADD START HOST %ls %ls", Inet_NtoA(Config.cnf_addr), Config.cnf_hostname) != RETURN_OK)
         MUI_Request(0,0,0,0, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningSetHostname), Config.cnf_hostname);
   }

   DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_Finished));
MUI_Request(NULL, NULL, 0,0,"ok","interface is configured. you should have full access to the net now.\n(this requester will disappear in the final version.)");

   if(data->abort)   goto abort;

   DoMethod(_app(obj), MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_ClosingConnection));
   Delay(10);

   if(data->abort)   goto abort;

   {
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

      mw_data->Page = 6;
   }

   /* set global variables for ShowConfig (cause amitcp won't run there anymore ! */

   *ip = *dest = *dns1 = *dns2 = *mask = NULL;
   strcpy(ip, Inet_NtoA(Config.cnf_addr));
   if(Config.cnf_dst != INADDR_ANY)
      strcpy(dest, Inet_NtoA(Config.cnf_dst));
   if(Config.cnf_dns1 != INADDR_ANY)
      strcpy(dns1, Inet_NtoA(Config.cnf_dns1));
   if(Config.cnf_dns2 != INADDR_ANY)
      strcpy(dns2, Inet_NtoA(Config.cnf_dns2));
   if(Config.cnf_netmask != INADDR_ANY)
      strcpy(mask, Inet_NtoA(Config.cnf_netmask));

abort:

   if(!data->abort)
      DoMethod(_app(obj), MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);

   if(IfConfigBase)
   {
      if(iface)
      {
         if(iface->iface_ifc)
            iface_deinit(iface);
         iface_free(iface);
         iface = NULL;
      }

      CloseLibrary(IfConfigBase);
   }
   IfConfigBase = NULL;

   if(SocketBase)
      CloseLibrary(SocketBase);
   SocketBase = NULL;

   amirexx_do_command("KILL");

   DoMethod(_app(obj), MUIM_Application_PushMethod, win, 1, MUIM_MainWindow_HangUp);
   Delay(100);

   /* start serial reading and add inputhandler (by misusing SetPage :) */
   StartSerialRead(serial_in, 1);
   DoMethod(_app(obj), MUIM_Application_PushMethod, win, 1, MUIM_MainWindow_SetPage);
   Delay(50);

   data->TCPHandler = NULL;
   ReleaseSemaphore(&data->HandlerSemaphore);
   return;
}

///
/// Online_GoOnline
ULONG Online_GoOnline(struct IClass *cl, Object *obj, Msg msg)
{
   struct Online_Data *data = INST_DATA(cl, obj);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   static char args[21];

   StopSerialRead();          // otherwise GoOnline would be called indefinitely
   serial_buffer[0] = NULL;   // when a ppp_frame arrived (see MainWindow_Trigger)
   ser_buf_pos = 0;

   data->addr_ptr       = (xget(mw_data->CY_IPAddress, MUIA_Cycle_Active) ? xgetstr(mw_data->STR_IPAddress) : "0.0.0.0");
   data->login_ptr      = xgetstr(mw_data->STR_UserName);
   data->passwd_ptr     = xgetstr(mw_data->STR_Password);

   sprintf(args, "%ld", obj);
   if(data->TCPHandler = CreateNewProcTags(
      NP_Entry,       TCPHandler,
      NP_Name,        "SetupAmiTCP netconfig",
      NP_StackSize,   16384,
      NP_WindowPtr,   -1,
      NP_Arguments,   args,
      NP_Output, Open("RAM:SetupAmiTCP.log", MODE_NEWFILE),
//NP_Output, Open("CON:/300/640/200/SetupAmiTCP/AUTO/WAIT/CLOSE", MODE_NEWFILE),
      TAG_END))
   {
      return(TRUE);
   }
   else
   {
      MUI_Request(_app(obj), obj, NULL, NULL, GetStr(MSG_BT__Abort), GetStr(MSG_TX_ErrorLaunchSubtask));
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
   if(data->TCPHandler)
      Signal(data->TCPHandler, SIGBREAKF_CTRL_C);
   ObtainSemaphore(&data->HandlerSemaphore);
   ReleaseSemaphore(&data->HandlerSemaphore);

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
         End,
         Child, tmp.BT_Abort = MakeButton(MSG_BT__Abort),
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Online_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->TCPHandler  = NULL;
      data->abort       = FALSE;
      InitSemaphore(&data->HandlerSemaphore);

      DoMethod(data->BT_Abort, MUIM_Notify, MUIA_Pressed, FALSE,
         MUIV_Notify_Application, 5, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);
   }

   return((ULONG)obj);
}

///
/// Online_Dispatcher
SAVEDS ASM ULONG Online_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                  : return(Online_New         (cl, obj, (APTR)msg));
      case OM_DISPOSE              : return(Online_Dispose       (cl, obj, (APTR)msg));
      case MUIM_Online_GoOnline    : return(Online_GoOnline    (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

