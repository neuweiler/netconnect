#include "globals.c"
#include "bootpc.h"
#include "iface.h"

/// login
VOID login(struct config *conf)
{
   struct Library *UserGroupBase;
   struct passwd *pwd;
   BPTR homedir;

   if(UserGroupBase = OpenLibrary(USERGROUPNAME, 0))
   {
      if(pwd = getpwnam(conf->cnf_loginname))
      {
         if(homedir = Lock(pwd->pw_dir, SHARED_LOCK))
         {
            if(!AssignLock("HOME", homedir))
               UnLock(homedir);
         }
         setgid(pwd->pw_gid);
         initgroups(conf->cnf_loginname, pwd->pw_gid);

         SetEnvDOS("HOME", pwd->pw_dir, -1, FALSE);
         SetEnvDOS("LOGNAME", pwd->pw_name, -1, FALSE);
         SetEnvDOS("USER", pwd->pw_name, -1, FALSE);
         setlogin(pwd->pw_name);
         setlastlog(pwd->pw_uid, conf->cnf_loginname, "Console");
      }
      else
      {
         SetEnvDOS("LOGNAME", conf->cnf_loginname, -1, FALSE);
         SetEnvDOS("USER", conf->cnf_loginname, -1, FALSE);
         setlogin(conf->cnf_loginname);
      }

      CloseLibrary(UserGroupBase);
   }     
}

///
/// TCPHandler
SAVEDS ASM VOID TCPHandler(REG(a0) STRPTR args, REG(d0) LONG arg_len)
{
   struct Online_Data *data = INST_DATA(CL_Online->mcc_Class, status_win);
   struct iface *iface = NULL;
   struct bootpc *bpc;
   BPTR fh;
char dns1[21], dns2[21];

   ObtainSemaphore(&data->HandlerSemaphore);

   /* launch amitcp if it isn't already running */
   if(!FindPort((STRPTR)AmiTCP_PortName))
   {
      if(!launch_amitcp())
      {
         DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), "Could not launch AmiTCP kernel\n");
         goto abort;
      }
   }
   if(data->abort)   goto abort;

   /* open bsdsocket.library */
   if(!(SocketBase = OpenLibrary("bsdsocket.library", 0)))
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorBsdsocketLib));
      goto abort;
   }

   if(SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), &errno, SBTM_SETVAL(SBTC_HERRNOLONGPTR), &h_errno, TAG_END))
      Printf("ERROR: Could not set errno ptr. Please report this to your support site.");

   if(data->abort)   goto abort;

   login(&Config);

   if(fh = CreateDir("ENV:NetConfig"))
      UnLock(fh);

   if(fh = Open("ENV:NetConfig/AutoInterfaces", MODE_NEWFILE))
   {
      FPrintf(fh, "%ls DEV=%ls UNIT=%ld", Config.cnf_ifname, Config.cnf_sana2device, Config.cnf_sana2unit);
      if(Config.cnf_ifconfigparams)
         FPrintf(fh, " %ls", Config.cnf_ifconfigparams);

      Close(fh);
   }

   DeleteFile("ENV:APPdns1");
   DeleteFile("ENV:APPdns2");

   if(data->abort)   goto abort;

   if(!(iface = iface_alloc()))
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorIfaceAlloc));
      goto abort;
   }

   if(data->abort)   goto abort;

   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_InitDevice), NULL);
   DoMainMethod(data->TR_Terminal, TCM_INIT, NULL, NULL, NULL);

   if(data->abort)   goto abort;

   if(!(iface_init(iface, &Config)))
      goto abort;

   /* get appp.device's ms-dns addresses */
   if(GetEnvDOS("APPPdns1", dns1, 20))
   {
      if(strcmp(dns1, "0.0.0.0") && Config.cnf_dns1 != INADDR_ANY)
         Config.cnf_dns1 = inet_addr(dns1);
   }
   if(GetEnvDOS("APPPdns2", dns2, 20))
   {
      if(strcmp(dns2, "0.0.0.0") && Config.cnf_dns2 != INADDR_ANY)
         Config.cnf_dns2 = inet_addr(dns2);
   }

   if(data->abort)   goto abort;

   if(Config.cnf_use_bootp)
   {
      DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_SendingBOOTP), NULL);
      iface_prepare_bootp(iface, &Config);
      if(bpc = bootpc_create())
      {
         if(bootpc_do(bpc, iface, &Config))
            DoMethod(app, MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_NoBOOTP));
         else
            DoMethod(app, MUIM_Application_PushMethod, data->TX_Info, 3, MUIM_Set, MUIA_Text_Contents, GetStr(MSG_TX_GotBOOTP));
         bootpc_delete(bpc);
         Delay(50);
      }
      else
         DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningBOOTPMemory));
      iface_cleanup_bootp(iface, &Config);
   }

   if(data->abort)   goto abort;

   if(Config.cnf_addr == INADDR_ANY)
   {
// popup reuester here
   }

   if(data->abort)   goto abort;

   if(Config.cnf_addr == INADDR_ANY)
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorNoIP));
      goto abort;
   }

   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_ConfiguringAmiTCP), NULL);

   if(data->abort)   goto abort;

   if(!(iface_config(iface, &Config)))
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 4, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorConfigIface), iface->iface_name);
      goto abort;
   }

   if(amirexx_do_command("RESET RESOLV") != RETURN_OK)
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorResetResolv));
      goto abort;
   }

   if(data->abort)   goto abort;

   if(Config.cnf_dns1 == INADDR_ANY && Config.cnf_dns2 == INADDR_ANY)
   {
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningNoDNS));
      Config.cnf_dns1 = inet_addr("198.41.0.4");
      Config.cnf_dns2 = inet_addr("128.9.0.107");
   }

   /* add in reverse order since each entry is added to top of list */
   if(Config.cnf_dns2 != INADDR_ANY)
   {
      if(amirexx_do_command("ADD START NAMESERVER %ls", Inet_NtoA(Config.cnf_dns2)) != RETURN_OK)
         DoMethod(app, MUIM_Application_PushMethod, win, 4, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorSetDNS), Inet_NtoA(Config.cnf_dns2));
   }
   if(Config.cnf_dns1 != INADDR_ANY)
   {
      if(amirexx_do_command("ADD START NAMESERVER %ls", Inet_NtoA(Config.cnf_dns1)) != RETURN_OK)
      {
         DoMethod(app, MUIM_Application_PushMethod, win, 4, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorSetDNS), Inet_NtoA(Config.cnf_dns1));
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
            DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningNoHostname));
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

   if(!*Config.cnf_domainname)
      DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningNoDomain));
   else
   {
      if(amirexx_do_command("ADD START DOMAIN %ls", Config.cnf_domainname) != RETURN_OK)
         DoMethod(app, MUIM_Application_PushMethod, win, 4, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningSetDomain), Config.cnf_domainname);
   }

   if(data->abort)   goto abort;

   if(*Config.cnf_hostname)
   {
      if(amirexx_do_command("ADD START HOST %ls %ls", Inet_NtoA(Config.cnf_addr), Config.cnf_hostname) != RETURN_OK)
         DoMethod(app, MUIM_Application_PushMethod, win, 4, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Okay), GetStr(MSG_TX_WarningSetHostname), Config.cnf_hostname);
      SetEnvDOS("HOSTNAME", Config.cnf_hostname, -1, FALSE);
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
      FPrintf(fh, "DOMAIN %ls\n", Config.cnf_domainname);
      Close(fh);
   }

   if(data->abort)   goto abort;

   if(fh = Lock("TCP:", ACCESS_READ))
      UnLock(fh);
   else
   {
      DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, "Mounting TCP:", NULL);
      launch_async("C:Mount TCP: from AmiTCP:Devs/Inet-mountlist");
      Delay(20);
   }

   if(data->abort)   goto abort;

   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, "Launching inetd", NULL);
   launch_async("AmiTCP:bin/inetd");
   Delay(20);

   if(data->abort)   goto abort;

   if(user_startnet.mlh_TailPred != (struct Node *)&user_startnet)
   {
      DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, "Executing user-startnet", NULL);
      exec_script(&user_startnet);
   }

   DoMainMethod(data->TX_Info, MUIM_Set, (APTR)MUIA_Text_Contents, GetStr(MSG_TX_Finished), NULL);
   DoMainMethod(win, MUIM_MainWindow_WeAreOnline, NULL, NULL, NULL);

abort:

   if(!data->abort)
      DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, status_win);

    if(iface)
    {
       iface_deinit(iface);
       iface_free(iface);
       iface = NULL;
    }

   if(SocketBase)
      CloseLibrary(SocketBase);
   SocketBase = NULL;

   data->TCPHandler = NULL;
   ReleaseSemaphore(&data->HandlerSemaphore);
   return;
}

///

/// Online_GoOnline
ULONG Online_GoOnline(struct IClass *cl, Object *obj, Msg msg)
{
   struct Online_Data *data = INST_DATA(cl, obj);
   static char args[21];

   sprintf(args, "%ld", obj);
   if(data->TCPHandler = CreateNewProcTags(
      NP_Entry,       TCPHandler,
      NP_Name,        "AmiTCP netconfig",
      NP_StackSize,   16384,
      NP_WindowPtr,   -1,
      NP_Arguments,   args,
      NP_Output, Open("RAM:AmiTCP.log", MODE_NEWFILE),
//NP_Output, Open("CON:/300/640/200/AmiTCP/AUTO/WAIT/CLOSE", MODE_NEWFILE),
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
      MUIA_Window_ID       , MAKE_ID('O','N','L','I'),
      MUIA_Window_RefWindow    , win,
      MUIA_Window_LeftEdge     , MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge      , MUIV_Window_TopEdge_Centered,
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
         Child, VGroup,
            GroupSpacing(0),
            Child, tmp.GR_Terminal = HGroup,
               GroupSpacing(0),
               InnerSpacing(0, 0),
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
            Child, tmp.GR_Buttons = ColGroup(3),
               GroupSpacing(0),
               Child, tmp.BT_Dial         = MakeButton(MSG_BT_Dial),
               Child, tmp.BT_GoOnline     = MakeButton(MSG_BT_GoOnline),
               Child, tmp.BT_HangUp       = MakeButton(MSG_BT_HangUp),
               Child, tmp.BT_SendLogin    = MakeButton(MSG_BT_SendLogin),
               Child, tmp.BT_SendPassword = MakeButton(MSG_BT_SendPassword),
               Child, tmp.BT_SendBreak    = MakeButton(MSG_BT_SendBreak),
            End,
         End,
         Child, tmp.BT_Abort = MakeButton(MSG_BT_Abort),
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Online_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->GR_Terminal, MUIA_ShowMe, FALSE);
      set(data->GR_Buttons , MUIA_ShowMe, !Config.cnf_autologin);

      data->TCPHandler  = NULL;
      data->abort       = FALSE;
      InitSemaphore(&data->HandlerSemaphore);

      set(data->TR_Terminal, TCA_SCROLLER, data->SB_Terminal);

      DoMethod(data->SB_Terminal , MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime, data->TR_Terminal, 1, TCM_SCROLLER);
      DoMethod(data->BT_Abort    , MUIM_Notify, MUIA_Pressed   , FALSE,          MUIV_Notify_Application, 5, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);
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

