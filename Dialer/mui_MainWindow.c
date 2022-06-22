#include "globals.c"
#include "protos.h"
#include "sana.h"

/// inet_getaddr
int inet_getaddr(struct Library *SocketBase, const char *s, const char *name, u_long *addr)
{
   struct hostent *hp;
   struct netent *np;

   if(hp = gethostbyname(s))
      bcopy(hp->h_addr, (char *)addr, hp->h_length);
   else if(np = getnetbyname(s))
      *addr = Inet_MakeAddr(np->n_net, INADDR_ANY);
   else
   {
      Printf("Invalid value for argument \"%s\": %s.\n", name, s);
      return(-1);
   }
   return(0);
}

///
/// MainWindow_LoadConfig
ULONG MainWindow_LoadConfig(struct IClass *cl, Object *obj, struct MUIP_MainWindow_LoadConfig *msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct Library *SocketBase;
   struct pc_Data pc_data;
   STRPTR ptr;
   LONG value;
   BOOL success = FALSE;

   clear_config(&Config);

   if(SocketBase = OpenLibrary("bsdsocket.library", 0))
   {
      if(ParseConfig(msg->file, &pc_data))
      {
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.Argument, "LoginName"))
               strncpy(Config.cnf_loginname  , pc_data.Contents, sizeof(Config.cnf_loginname));
            else if(!stricmp(pc_data.Argument, "Password"))
               strncpy(Config.cnf_password   , pc_data.Contents, sizeof(Config.cnf_password));
            else if(!stricmp(pc_data.Argument, "RealName"))
               strncpy(Config.cnf_realname   , pc_data.Contents, sizeof(Config.cnf_realname));

            else if(!stricmp(pc_data.Argument, "Phone"))
               strncpy(Config.cnf_phonenumber, pc_data.Contents, sizeof(Config.cnf_phonenumber));

            else if(!stricmp(pc_data.Argument, "SerialDevice"))
               strncpy(Config.cnf_serialdevice, pc_data.Contents, sizeof(Config.cnf_serialdevice));
            else if(!stricmp(pc_data.Argument, "SerialUnit"))
               Config.cnf_serialunit = atol(pc_data.Contents);
            else if(!stricmp(pc_data.Argument, "BaudRate"))
               Config.cnf_baudrate = atol(pc_data.Contents);
            else if(!stricmp(pc_data.Argument, "CarrierDetect"))
               Config.cnf_carrierdetect = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "7Wire"))
               Config.cnf_7wire = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "SerBufLen"))
               Config.cnf_serbuflen = atol(pc_data.Contents);

            else if(!stricmp(pc_data.Argument, "Modem"))
               strncpy(Config.cnf_modemname  , pc_data.Contents, sizeof(Config.cnf_modemname));
            else if(!stricmp(pc_data.Argument, "InitString"))
               strncpy(Config.cnf_initstring , pc_data.Contents, sizeof(Config.cnf_initstring));
            else if(!stricmp(pc_data.Argument, "DialPrefix"))
               strncpy(Config.cnf_dialprefix , pc_data.Contents, sizeof(Config.cnf_dialprefix));
            else if(!stricmp(pc_data.Argument, "DialSuffix"))
               strncpy(Config.cnf_dialsuffix , pc_data.Contents, sizeof(Config.cnf_dialsuffix));
            else if(!stricmp(pc_data.Argument, "RedialAttempts"))
               Config.cnf_redialattempts = atol(pc_data.Contents);
            else if(!stricmp(pc_data.Argument, "RedialDelay"))
               Config.cnf_redialdelay = atol(pc_data.Contents);

            else if(!stricmp(pc_data.Argument, "Sana2Device"))
               strncpy(Config.cnf_sana2device, pc_data.Contents, sizeof(Config.cnf_sana2device));
            else if(!stricmp(pc_data.Argument, "Sana2Unit"))
               Config.cnf_sana2unit = atol(pc_data.Contents);
            else if(!stricmp(pc_data.Argument, "Sana2Config"))
               strncpy(Config.cnf_sana2config, pc_data.Contents, sizeof(Config.cnf_sana2config));
            else if(!stricmp(pc_data.Argument, "Sana2ConfigText") && !Config.cnf_sana2configtext)
            {
               int i;

               if(Config.cnf_sana2configtext = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY))
               {
                  i = 0;
                  ptr = pc_data.Contents;
                  while(*ptr)
                  {
                     if(ptr[0] == '\\' && ptr[1] == 'n')
                     {
                        Config.cnf_sana2configtext[i++] = '\n';
                        ptr++;
                     }
                     else
                        Config.cnf_sana2configtext[i++] = *ptr;
                     ptr++;
                  }
                  Config.cnf_sana2configtext[i] = NULL;
               }
            }
            else if(!stricmp(pc_data.Argument, "Interface"))
            {
               strncpy(Config.cnf_ifname, pc_data.Contents, sizeof(Config.cnf_ifname));
               strlwr(Config.cnf_ifname);
            }
            else if(!stricmp(pc_data.Argument, "IfConfigParams") && !Config.cnf_ifconfigparams)
            {
               if(Config.cnf_ifconfigparams = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY))
                  strcpy(Config.cnf_ifconfigparams, pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "MTU"))
            {
               if((value = atol(pc_data.Contents)) >= 2)
                  Config.cnf_MTU = value;
            }
            else if(!stricmp(pc_data.Argument, "KeepAlive"))
               Config.cnf_keepalive = atol(pc_data.Contents);
            else if(!stricmp(pc_data.Argument, "PingInterval"))
               Config.cnf_pinginterval = atol(pc_data.Contents);

            else if(!stricmp(pc_data.Argument, "IPAddr"))
            {
               if(inet_getaddr(SocketBase, pc_data.Contents, "Config file, IP", (ULONG *)&value) == 0)
                  Config.cnf_addr = value;
               else
                  Printf("Illegal IP address %ls.\n", pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "HostName"))
               strncpy(Config.cnf_hostname, pc_data.Contents, sizeof(Config.cnf_hostname));
            else if(!stricmp(pc_data.Argument, "DestIP"))
            {
               if(inet_getaddr(SocketBase, pc_data.Contents, "Config file, DestIP", (ULONG *)&value) == 0)
                  Config.cnf_dst = value;
               else
                  Printf("Illegal DestIP address %ls.\n", pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "Gateway"))
            {
               if(inet_getaddr(SocketBase, pc_data.Contents, "Config file, Gateway", (ULONG *)&value) == 0)
                  Config.cnf_gateway = value;
               else
                  Printf("Illegal Gateway address %ls.\n", pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "Netmask"))
            {
               if(inet_getaddr(SocketBase, pc_data.Contents, "Config file, Netmask", (ULONG *)&value) == 0)
                  Config.cnf_netmask = value;
               else
                  Printf("Illegal netmask %ls.\n", pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "DomainName"))
               strncpy(Config.cnf_domainname, pc_data.Contents, sizeof(Config.cnf_domainname));
            else if(!stricmp(pc_data.Argument, "NameServer"))
            {
               if(inet_getaddr(SocketBase, pc_data.Contents, "Config file, NameServer", (ULONG *)&value) == 0)
               {
                  if(Config.cnf_dns1 == INADDR_ANY)
                     Config.cnf_dns1 = value;
                  else if(Config.cnf_dns2 == INADDR_ANY)
                     Config.cnf_dns2 = value;
               }
               else
                  Printf("Illegal NameServer address %ls.\n", pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "BOOTPServer"))
            {
               if(inet_getaddr(SocketBase, pc_data.Contents, "Config file, BOOTPServer", (ULONG *)&value) == 0)
                  Config.cnf_bootpserver = value;
               else
                  Printf("Illegal BOOTPServer address %ls.\n", pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "UseBootP"))
               Config.cnf_use_bootp = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "TimeServer"))
               strncpy(Config.cnf_timename, pc_data.Contents, sizeof(Config.cnf_timename));

            else if(!stricmp(pc_data.Argument, "AutoLogin"))
               Config.cnf_autologin = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "OnlineOnStartup"))
               Config.cnf_onlineonstartup = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "QuickReconnect"))
               Config.cnf_quickreconnect = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "SynClock"))
               Config.cnf_synclock = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "ShowStatus"))
               Config.cnf_showstatus = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "ShowSpeed"))
               Config.cnf_showspeed = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "ShowOnline"))
               Config.cnf_showonlinetime = !stricmp(pc_data.Contents, "yes");
            else if(!stricmp(pc_data.Argument, "ShowButtons"))
               Config.cnf_showbuttons = !stricmp(pc_data.Contents, "yes");

            else if(!stricmp(pc_data.Argument, "Online") && !Config.cnf_online)
            {
               if(Config.cnf_online = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY))
                  strcpy(Config.cnf_online, pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "OnlineFail") && !Config.cnf_onlinefail)
            {
               if(Config.cnf_onlinefail = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY))
                  strcpy(Config.cnf_onlinefail, pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "OfflineActive") && !Config.cnf_offlineactive)
            {
               if(Config.cnf_offlineactive = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY))
                  strcpy(Config.cnf_offlineactive, pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "OfflinePassive") && !Config.cnf_offlinepassive)
            {
               if(Config.cnf_offlinepassive = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY))
                  strcpy(Config.cnf_offlinepassive, pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "Startup") && !Config.cnf_startup)
            {
               if(Config.cnf_startup = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY))
                  strcpy(Config.cnf_startup, pc_data.Contents);
            }
            else if(!stricmp(pc_data.Argument, "Shutdown") && !Config.cnf_shutdown)
            {
               if(Config.cnf_shutdown = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY))
                  strcpy(Config.cnf_shutdown, pc_data.Contents);
            }

            else if(!stricmp(pc_data.Argument, "WinOnline"))
               Config.cnf_winonline = (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0));
            else if(!stricmp(pc_data.Argument, "WinOnlineFail"))
               Config.cnf_winonlinefail = (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0));
            else if(!stricmp(pc_data.Argument, "WinOfflineActive"))
               Config.cnf_winofflineactive = (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0));
            else if(!stricmp(pc_data.Argument, "WinOfflinePassive"))
               Config.cnf_winofflinepassive = (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0));
            else if(!stricmp(pc_data.Argument, "WinStartup"))
               Config.cnf_winstartup = (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0));

            else if(!stricmp(pc_data.Argument, "LoginScript"))
            {
               struct ScriptLine *script_line;

               while(ParseNext(&pc_data))
               {
                  if(!stricmp(pc_data.Argument, "EOS"))
                     break;

                  if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY | MEMF_CLEAR))
                  {
                     if(!stricmp(pc_data.Argument, "Send"))
                        script_line->sl_command = SL_Send;
                     if(!stricmp(pc_data.Argument, "WaitFor"))
                        script_line->sl_command = SL_WaitFor;
                     if(!stricmp(pc_data.Argument, "Dial"))
                        script_line->sl_command = SL_Dial;
                     if(!stricmp(pc_data.Argument, "GoOnline"))
                        script_line->sl_command = SL_GoOnline;
                     if(!stricmp(pc_data.Argument, "SendLogin"))
                        script_line->sl_command = SL_SendLogin;
                     if(!stricmp(pc_data.Argument, "SendPassword"))
                        script_line->sl_command = SL_SendPassword;
                     if(!stricmp(pc_data.Argument, "SendBreak"))
                        script_line->sl_command = SL_SendBreak;
                     if(!stricmp(pc_data.Argument, "Pause"))
                        script_line->sl_command = SL_Pause;

                     if(script_line->sl_command)
                     {
                        strncpy(script_line->sl_contents, pc_data.Contents, sizeof(script_line->sl_contents));
                        AddTail((struct List *)&dialscript, (struct Node *)&script_line->sl_node);
                     }
                     else
                        FreeVec(script_line);
                  }
               }
            }
            else if(!stricmp(pc_data.Argument, "UserStartNet"))
            {
               struct ScriptLine *script_line;

               while(ParseNextLine(&pc_data))
               {
                  if(!stricmp(pc_data.Contents, "EOS"))
                     break;
                  if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY | MEMF_CLEAR))
                  {
                     script_line->sl_command = SL_Exec;
                     strncpy(script_line->sl_contents, pc_data.Contents, sizeof(script_line->sl_contents));
                     AddTail((struct List *)&user_startnet, (struct Node *)script_line);
                  }
               }
            }
            else if(!stricmp(pc_data.Argument, "UserStopNet"))
            {
               struct ScriptLine *script_line;

               while(ParseNextLine(&pc_data))
               {
                  if(!stricmp(pc_data.Contents, "EOS"))
                     break;

                  if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY | MEMF_CLEAR))
                  {
                     script_line->sl_command = SL_Exec;
                     strncpy(script_line->sl_contents, pc_data.Contents, sizeof(script_line->sl_contents));
                     AddTail((struct List *)&user_stopnet, (struct Node *)script_line);
                  }
               }
            }
         }
         ParseEnd(&pc_data);

         set(data->GR_Log        , MUIA_ShowMe, Config.cnf_showstatus);
         set(data->GR_Speed      , MUIA_ShowMe, Config.cnf_showspeed);
         set(data->GR_Online     , MUIA_ShowMe, Config.cnf_showonlinetime);
         set(data->GR_Buttons    , MUIA_ShowMe, Config.cnf_showbuttons);

         success = TRUE;
      }
      else
         MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_Okay), "Could not load config file '%ls'", msg->file);

      CloseLibrary(SocketBase);
   }
   else
      MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_Okay), "Could not open bsdsocket.libary.\nAmiTCP kernel is not running.");

   return((ULONG)success);
}
///

/// MainWindow_MUIRequest
ULONG MainWindow_MUIRequest(struct IClass *cl, Object *obj, struct MUIP_MainWindow_MUIRequest *msg)
{
   char buf[MAXPATHLEN + 20];

   vsprintf(buf, msg->message, (STRPTR)(&msg->message + 1));
   MUI_Request(app, win, NULL, NULL, msg->buttons, buf);
   return(NULL);
}

///
/// MainWindow_About
ULONG MainWindow_About(struct IClass *cl, Object *obj, Msg msg)
{
   MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), "\033b\033c" VERS "\033n\033c\n\n%ls\n\nAREXX port: '%ls'", GetStr(MSG_TX_About), xget(app, MUIA_Application_Base));
   return(NULL);
}

///
/// MainWindow_Abort
ULONG MainWindow_Abort(struct IClass *cl, Object *obj, Msg msg)
{
   if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_QuitCancel), GetStr(MSG_TX_ReallyQuit)))
      DoMethod(app, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

   return(NULL);
}

///
/// MainWindow_Online
ULONG MainWindow_Online(struct IClass *cl, Object *obj, Msg msg)
{
   set(app, MUIA_Application_Sleep, TRUE);
   if(status_win = NewObject(CL_Online->mcc_Class, NULL, TAG_DONE))
   {
      struct Online_Data *ol_data = INST_DATA(CL_Online->mcc_Class, status_win);

      DoMethod(app, OM_ADDMEMBER, status_win);
      set(status_win, MUIA_Window_Open, TRUE);

      if(dialscript.mlh_TailPred != (struct Node *)&dialscript)
         set(ol_data->GR_Terminal, MUIA_ShowMe, TRUE);   // this has to be set after the window is opened (otherwise we get a huge window)
      DoMethod(status_win, MUIM_Online_GoOnline);
   }

   return(NULL);
}

///
/// MainWindow_Offline
ULONG MainWindow_Offline(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct sana2 *s2;

   set(app, MUIA_Application_Sleep, TRUE);

   exec_script(&user_stopnet);

   if(data->online)
   {
      DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->online_ihn);
      data->online = 0;
   }
   set(data->BT_Online  , MUIA_Disabled, FALSE);
   set(data->BT_Offline , MUIA_Disabled, TRUE);
   set(data->TX_Status  , MUIA_Text_Contents, "offline");
   set(data->TX_Speed   , MUIA_Text_Contents, "-");

   if(s2 = sana2_create(&Config))
   {
      sana2_offline(s2);
      sana2_delete(s2);
   }
   else
      MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Abort), "Could not open %ls, unit %ld", Config.cnf_sana2device, Config.cnf_sana2unit);

   set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///
/// MainWindow_TimeTrigger
ULONG MainWindow_TimeTrigger(struct IClass *cl, Object *obj)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   ULONG h, m, s;

   GetSysTime(&data->time);
   s = data->time.tv_secs - data->online;
   m = s / 60;
   h = m / 60;
   s = s % 60;
   m = m % 60;
   sprintf(data->time_str, "%02ld:%02ld:%02ld", h, m, s);
   set(data->TX_Online, MUIA_Text_Contents, data->time_str);

   return(TRUE);
}
///
/// MainWindow_WeAreOnline
ULONG MainWindow_WeAreOnline(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   char buf[21];

   set(data->BT_Online, MUIA_Disabled, TRUE);
   set(data->BT_Offline, MUIA_Disabled, FALSE);
   set(data->TX_Status, MUIA_Text_Contents, "online");

   sprintf(buf, "%ld baud", Config.cnf_connectspeed);
   set(data->TX_Speed, MUIA_Text_Contents, (Config.cnf_connectspeed ? (STRPTR)buf : (STRPTR)"unknown"));

   if(!data->online)
   {
      GetSysTime(&data->time);
      data->online = data->time.tv_secs;

      data->online_ihn.ihn_Object = obj;
      data->online_ihn.ihn_Method = MUIM_MainWindow_TimeTrigger;
      data->online_ihn.ihn_Flags  = MUIIHNF_TIMER;
      data->online_ihn.ihn_Millis = 1000;
      DoMethod(_app(obj), MUIM_Application_AddInputHandler, &data->online_ihn);
   }

   return(NULL);
}

///
/// MainWindow_DisposeWindow
ULONG MainWindow_DisposeWindow(struct IClass *cl, Object *obj, struct MUIP_MainWindow_DisposeWindow *msg)
{
   if(msg->window)
   {
      set(msg->window, MUIA_Window_Open, FALSE);
      Delay(10);
      DoMethod(_app(msg->window), OM_REMMEMBER, msg->window);
      MUI_DisposeObject(msg->window);
   }

   set(app, MUIA_Application_Sleep, FALSE);

   if(status_win == msg->window)
      status_win = NULL;

   return(NULL);
}

///
/// MainWindow_UpdateLog
ULONG MainWindow_UpdateLog(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct LogEntry *log_entry;
   char buf[MAXPATHLEN];
   STRPTR ptr1, ptr2, ptr3;
   BPTR fh;

   if(log_entry = AllocVec(sizeof(struct LogEntry), MEMF_ANY))
   {
      if(fh = Open("T:AmiTCP.log", MODE_OLDFILE))
      {
         Seek(fh, data->log_pos, OFFSET_BEGINNING);
         DoMethod(data->LI_Log, MUIA_NList_Quiet, TRUE);

         while(FGets(fh, buf, MAXPATHLEN))
         {
            bzero(log_entry, sizeof(struct LogEntry));

            if(ptr1 = strchr(buf, '['))
            {
               ptr1--;
               *ptr1 = NULL;

               strncpy(log_entry->time, buf, 30);
               ptr1 += 2;

               if(ptr3 = ptr2 = strchr(ptr1, ']'))
               {
                  ptr2--;
                  while(*ptr2 == ' ')
                     ptr2--;
                  ptr2++;
                  *ptr2 = NULL;

                  strncpy(log_entry->type, ptr1, 20);

                  ptr3 += 2;
                  strncpy(log_entry->info, ptr3, 80);

                  DoMethod(data->LI_Log, MUIM_NList_InsertSingle, log_entry, MUIV_NList_Insert_Bottom);
               }
            }
         }

         data->log_pos = Seek(fh, 0, OFFSET_CURRENT);
         DoMethod(data->LI_Log, MUIA_NList_Quiet, FALSE);

         Close(fh);
      }
      FreeVec(log_entry);
   }

   DoMethod(data->LI_Log, MUIM_NList_Jump, xget(data->LI_Log, MUIA_NList_Entries));

   return(NULL);
}
///

/// Log_ConstructFunc
SAVEDS ASM struct LogEntry *Log_ConstructFunc(REG(a2) APTR pool, REG(a1) struct LogEntry *src)
{
   struct LogEntry *new;

   if((new = (struct LogEntry *)AllocVec(sizeof(struct LogEntry), MEMF_ANY | MEMF_CLEAR)) && src && src != (APTR)-1)
      memcpy(new, src, sizeof(struct LogEntry));
   return(new);
}

///
/// Log_DisplayFunc
SAVEDS ASM LONG Log_DisplayFunc(REG(a2) char **array, REG(a1) struct LogEntry *log_entry)
{
   if(log_entry)
   {
      *array++ = log_entry->time;
      *array++ = log_entry->type;
      *array = log_entry->info;
   }
   else
   {
      *array++ = GetStr("  \033bTime");
      *array++ = GetStr("  \033bType");
      *array++ = GetStr("  \033bInformation");
   }
   return(NULL);
}

///

/// MainWindow_New
ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static const struct Hook Log_ConstructHook= { { 0,0 }, (VOID *)Log_ConstructFunc , NULL, NULL };
   static const struct Hook Log_DisplayHook  = { { 0,0 }, (VOID *)Log_DisplayFunc   , NULL, NULL };
   struct MainWindow_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_TX_MainWindowTitle),
      MUIA_Window_ID       , MAKE_ID('M','A','I','N'),
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainMenu, NULL),
      WindowContents       , VGroup,
         Child, HGroup,
            Child, HGroup,
               GroupFrame,
               MUIA_Weight, 1,
               MUIA_Background, MUII_GroupBack,
               Child, tmp.TX_Status = MakeText("offline"),
            End,
            Child, tmp.GR_Online = HGroup,
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, LLabel("Time Online:"),
               Child, tmp.TX_Online = MakeText("--:--:--"),
            End,
         End,
         Child, tmp.GR_Speed = HGroup,
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, LLabel("Speed:"),
            Child, tmp.TX_Speed = MakeText("-          "),
         End,
         Child, tmp.GR_Log = VGroup,
            Child, tmp.LV_Log = NListviewObject,
               MUIA_CycleChain            , 1,
               MUIA_NListview_NList        , tmp.LI_Log = NListObject,
                  MUIA_Font                , MUIV_Font_Tiny,
                  MUIA_Frame               , MUIV_Frame_InputList,
                  MUIA_NList_DisplayHook   , &Log_DisplayHook,
                  MUIA_NList_ConstructHook , &Log_ConstructHook,
                  MUIA_NList_DestructHook  , &deshook,
                  MUIA_NList_Format        , "BAR,BAR,",
                  MUIA_NList_Title         , TRUE,
               End,
            End,
         End,
         Child, tmp.GR_Buttons = VGroup,
            Child, MUI_MakeObject(MUIO_HBar, 2),
            Child, HGroup,
               Child, tmp.BT_Online = MakeButton("  Go _online"),
               Child, tmp.BT_Offline = MakeButton("  Go o_ffline"),
            End,
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct MainWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->online = 0;
      set(data->BT_Offline, MUIA_Disabled, TRUE);

      DoMethod(obj            , MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 1, MUIM_MainWindow_Abort);
      DoMethod(data->BT_Online, MUIM_Notify, MUIA_Pressed            , FALSE, obj, 1, MUIM_MainWindow_Online);
      DoMethod(data->BT_Offline, MUIM_Notify, MUIA_Pressed            , FALSE, obj, 1, MUIM_MainWindow_Offline);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_About);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT_MUI), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_AboutMUI, win);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_Abort);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
   }
   return((ULONG)obj);
}

///
/// MainWindow_Dispatcher
SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                            : return(MainWindow_New                (cl, obj, (APTR)msg));
      case MUIM_MainWindow_LoadConfig        : return(MainWindow_LoadConfig         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MUIRequest        : return(MainWindow_MUIRequest         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About             : return(MainWindow_About              (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Abort             : return(MainWindow_Abort              (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Online            : return(MainWindow_Online             (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Offline           : return(MainWindow_Offline            (cl, obj, (APTR)msg));
      case MUIM_MainWindow_WeAreOnline       : return(MainWindow_WeAreOnline        (cl, obj, (APTR)msg));
      case MUIM_MainWindow_DisposeWindow     : return(MainWindow_DisposeWindow      (cl, obj, (APTR)msg));
      case MUIM_MainWindow_TimeTrigger       : return(MainWindow_TimeTrigger        (cl, obj));
      case MUIM_MainWindow_UpdateLog         : return(MainWindow_UpdateLog          (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

