/// includes
#include "/includes.h"
#pragma header

#include "rev.h"
#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_Online.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "bootpc.h"
#include "sana.h"
#include "iface.h"

///
/// external variables
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_Online;
extern struct Library *SocketBase;
extern struct config Config;
extern int errno;
extern int addr_assign, dst_assign, dns_assign, domainname_assign;
extern char ip[], dest[], dns1[], dns2[], mask[];
extern Object *win, *status_win;
extern Object *app;
extern char serial_in[], serial_buffer[];
extern WORD ser_buf_pos;
extern const char AmiTCP_PortName[];

///

/// amirexx_do_command
LONG amirexx_do_command(const char *fmt, ...)
{
   char buf[256];
   struct MsgPort *port; /* our reply port */
   struct MsgPort *AmiTCP_Port;
   struct RexxMsg *rmsg;
   LONG rc = 20;         /* fail */

   vsprintf(buf, fmt, (va_list)(&fmt + (va_list)1));
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
               } while(GetMsg(port) != (struct Message *)rmsg);
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
VOID SAVEDS TCPHandler(register __a0 STRPTR args, register __d0 LONG arg_len)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct iface *iface = NULL;
   struct bootpc *bpc;
   BPTR fh;

   if(fh = Open("AmiTCP:db/resolv.conf", MODE_NEWFILE))
   {
      FPrintf(fh, "; This file is built dynamically - do not edit\n");
      Close(fh);
   }

   /* launch AmiTCP if it isn't already running */
   if(!FindPort((STRPTR)AmiTCP_PortName))
   {
      if(!launch_amitcp())
      {
         DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorLaunchAmiTCP), NULL);
         goto abort;
      }
   }
   if(data->abort)   goto abort;

   /* open bsdsocket.library and ifconfig.library */
   if(!(SocketBase = OpenLibrary("bsdsocket.library", 0)))
   {
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorBsdsocketLib), NULL);
      goto abort;
   }

   if(SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno, SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno, TAG_END))
      Printf("WARINING: Could not set errno ptr.");

   if(data->abort)   goto abort;

   if(fh = CreateDir("ENV:NetConfig"))
      UnLock(fh);

   if(fh = Open("ENV:NetConfig/AutoInterfaces", MODE_NEWFILE))
   {
      FPrintf(fh, "%ls DEV=%ls UNIT=%ld", Config.cnf_ifname, Config.cnf_sana2device, Config.cnf_sana2unit);
      if(Config.cnf_ifconfigparams)
         FPrintf(fh, " %ls", Config.cnf_ifconfigparams);

      Close(fh);
   }
   else
   {
      Printf("ERROR: Could not create ENV:NetConfig/Autointerfaces.\n");
      goto abort;
   }

   DeleteFile("ENV:APPdns1");
   DeleteFile("ENV:APPdns2");

   if(data->abort)   goto abort;

   Config.cnf_netmask   = inet_addr("255.255.255.0");
   Config.cnf_addr      = inet_addr(ip);
   Config.cnf_MTU       = 1500;
   if(Config.cnf_addr != INADDR_ANY && !addr_assign)
      addr_assign  = CNF_Assign_Static;

   if(!(iface = iface_alloc()))
   {
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorIfaceAlloc), NULL);
      goto abort;
   }

   if(data->abort)   goto abort;
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_InitDevice), NULL);

   if(!(iface_init(iface, &Config)))
   {
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorIfaceInit), NULL);
      goto abort;
   }

   /* get appp.device's ms-dns addresses */
   if(GetEnvDOS("APPPdns1", dns1, 20))
   {
      if(strcmp(dns1, "0.0.0.0") && Config.cnf_dns1 == INADDR_ANY)
         Config.cnf_dns1 = inet_addr(dns1);
      Printf("MS-DNS1: %ls\n", dns1);
   }
   if(GetEnvDOS("APPPdns2", dns2, 20))
   {
      if(strcmp(dns2, "0.0.0.0") && Config.cnf_dns2 == INADDR_ANY)
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
   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_SendingBOOTP), NULL);

   iface_prepare_bootp(iface, &Config);
   if(bpc = bootpc_create())
   {
      if(bootpc_do(bpc, iface, &Config))
      {
         Printf("BOOTP failed to have an answer.\n");
         DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_NoBOOTP), NULL);
      }
      else
         DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_GotBOOTP), NULL);

      bootpc_delete(bpc);
   }
   else
      Printf("WARNING: No memory for bootp request !\n");
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
      if(!data->abort)
         DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorNoIP), NULL);
      goto abort;
   }

   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConfiguringAmiTCP), NULL);

   if(!(iface_config(iface, &Config)))
   {
      char buffer[81];

      sprintf(buffer, GetStr(MSG_TX_ErrorConfigIface), iface->iface_name);
      if(!data->abort)
         DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), buffer, NULL);
      goto abort;
   }

   if(data->abort)   goto abort;

   if(amirexx_do_command("RESET RESOLV") != RETURN_OK)
      Printf("ERROR: Could not reset resolv database.\n");

   if(data->abort)   goto abort;

   if(Config.cnf_dns1 == INADDR_ANY && Config.cnf_dns2 == INADDR_ANY)
   {
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), (APTR)GetStr(MSG_TX_WarningNoDNS), NULL);
      Config.cnf_dns1 = inet_addr("198.41.0.4");
      Config.cnf_dns2 = inet_addr("128.9.0.107");
      dns_assign = CNF_Assign_Root;
   }

   /* add in reverse order since each entry is added to top of list */
   if(Config.cnf_dns2 != INADDR_ANY)
   {
      if(amirexx_do_command("ADD START NAMESERVER %ls", Inet_NtoA(Config.cnf_dns2)) != RETURN_OK)
         Printf(GetStr(MSG_TX_ErrorSetDNS), Inet_NtoA(Config.cnf_dns2));
   }
   if(Config.cnf_dns1 != INADDR_ANY)
   {
      if(amirexx_do_command("ADD START NAMESERVER %ls", Inet_NtoA(Config.cnf_dns1)) != RETURN_OK)
      {
         char buffer[81];

         sprintf(buffer, GetStr(MSG_TX_ErrorSetDNS), Inet_NtoA(Config.cnf_dns1));
         DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), buffer, NULL);
         goto abort;
      }
   }

   if(data->abort)   goto abort;

   /* is hostname or domainname missing but got rest ? */
   if((Config.cnf_addr != INADDR_ANY && (Config.cnf_dns1 != INADDR_ANY || Config.cnf_dns2 != INADDR_ANY)) &&
      (!*Config.cnf_hostname || !*Config.cnf_domainname))
   {
      DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_GetDomain), NULL);

      if(!*Config.cnf_hostname)
      {
         if(gethostname(Config.cnf_hostname, sizeof(Config.cnf_hostname)) || !*Config.cnf_hostname)
         {
            if(!data->abort)
               DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningNoHostname), NULL);
         }
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

   if(data->abort)   goto abort;

   if(!*Config.cnf_domainname)
      DoMainMethod(win, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningNoDomain), NULL);
   else
   {
      if(amirexx_do_command("ADD START DOMAIN %ls", Config.cnf_domainname) != RETURN_OK)
         Printf(GetStr(MSG_TX_WarningSetDomain), Config.cnf_domainname);
   }

   if(data->abort)   goto abort;

   if(!*Config.cnf_hostname)
      Printf("WARNING: Got no hostname !\n");
   else
   {
      if(amirexx_do_command("ADD START HOST %ls %ls", Inet_NtoA(Config.cnf_addr), Config.cnf_hostname) != RETURN_OK)
         Printf(GetStr(MSG_TX_WarningSetHostname), Config.cnf_hostname);
   }

   if(fh = Open("AmiTCP:db/resolv.conf", MODE_NEWFILE))
   {
      FPrintf(fh, "; This file is built dynamically - do not edit\n");
      FPrintf(fh, "; Name Servers\n");
      if(Config.cnf_dns1 != INADDR_ANY)
         FPrintf(fh, "NAMESERVER %ls\n", Inet_NtoA(Config.cnf_dns1));
      if(Config.cnf_dns2 != INADDR_ANY)
         FPrintf(fh, "NAMESERVER %ls\n", Inet_NtoA(Config.cnf_dns2));
      FPrintf(fh, "; Search domain\n");
      FPrintf(fh, "DOMAIN %ls\n", (*Config.cnf_domainname ? Config.cnf_domainname : "unknown.domain"));
      Close(fh);
   }

   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_Finished), NULL);
DoMainMethod(win, MUIM_MainWindow_MUIRequest, "ok", "interface is configured. you should have access to the net now.\n(this requester will disappear in the final version.)", NULL);

   if(data->abort)   goto abort;

   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ClosingConnection), NULL);

   mw_data->Page = 7;

   /* set global variables for ShowConfig (cause AmiTCP won't run there anymore ! */

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

   if(iface)
   {
      if(!iface->iface_s2)    // put sana2 offline
         iface->iface_s2 = sana2_create(&Config);
      if(iface->iface_s2)
         iface_offline(iface);

      iface_deinit(iface);
      iface_free(iface);
      iface = NULL;
   }

   if(SocketBase)
      CloseLibrary(SocketBase);
   SocketBase = NULL;

   amirexx_do_command("KILL");

   if(serial_carrier())
      serial_hangup();

   // go to next page or start serial reading and add inputhandler by misusing setpage
   DoMainMethod(win, MUIM_MainWindow_SetPage, NULL, NULL, NULL);

   Forbid();
   data->TCPHandler = NULL;
   DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, status_win);
   status_win = NULL;
}

///
/// Online_GoOnline
ULONG Online_GoOnline(struct IClass *cl, Object *obj, Msg msg)
{
   struct Online_Data *data = INST_DATA(cl, obj);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

   serial_stopread();         // otherwise GoOnline would be called indefinitely
   serial_buffer[0] = NULL;   // when a ppp_frame arrived (see MainWindow_Trigger)
   ser_buf_pos = 0;

   if(data->TCPHandler = CreateNewProcTags(
      NP_Entry       , TCPHandler,
      NP_Name        , "GenesisWizard netconfig",
      NP_StackSize   , 16384,
      NP_WindowPtr   , -1,
      NP_CloseOutput , TRUE,
      NP_Output      , Open("RAM:GenesisWizard.log", MODE_NEWFILE),
//NP_Output, Open("CON:/300/640/200/GenesisWizard netconfig/AUTO/WAIT/CLOSE", MODE_NEWFILE),
      TAG_END))
   {
      return(TRUE);
   }
   else
   {
      MUI_Request(_app(obj), obj, NULL, NULL, GetStr(MSG_BT__Abort), GetStr(MSG_TX_ErrorLaunchSubtask));
      DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);
   }
   return(FALSE);
}

///
/// Online_Abort
ULONG Online_Abort(struct IClass *cl, Object *obj, Msg msg)
{
   struct Online_Data *data = INST_DATA(cl, obj);

   data->abort = TRUE;
   Forbid();
   if(data->TCPHandler)
      Signal((struct Task *)data->TCPHandler, SIGBREAKF_CTRL_C);
   Permit();

   return(NULL);
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
         Child, tmp.BT_Abort = MakeButton(MSG_BT__Abort),
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Online_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->TCPHandler  = NULL;
      data->abort       = FALSE;

      DoMethod(data->BT_Abort, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_Online_Abort);
   }

   return((ULONG)obj);
}

///
/// Online_Dispatcher
SAVEDS ULONG Online_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW               : return(Online_New         (cl, obj, (APTR)msg));
      case MUIM_Online_GoOnline : return(Online_GoOnline    (cl, obj, (APTR)msg));
      case MUIM_Online_Abort    : return(Online_Abort       (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

