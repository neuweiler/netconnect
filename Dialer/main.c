#include "globals.c"

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)              CloseCatalog(cat);

   if(RexxSysBase)      CloseLibrary((struct Library *)RexxSysBase);
   if(UtilityBase)      CloseLibrary(UtilityBase);
   if(MUIMasterBase)    CloseLibrary(MUIMasterBase);
   if(LocaleBase)       CloseLibrary(LocaleBase);
   if(IntuitionBase)    CloseLibrary(IntuitionBase);
   if(DOSBase)          CloseLibrary(DOSBase);

   cat            = NULL;
   IntuitionBase  = UtilityBase =
   MUIMasterBase  = LocaleBase  =
   DOSBase        = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
   DOSBase        = (struct DosLibrary *)OpenLibrary("dos.library", 0);
   IntuitionBase  = OpenLibrary("intuition.library"   , 0);

   if(LocaleBase  = OpenLibrary("locale.library", 38))
      cat = OpenCatalog(NULL, "SetupAmiTCP.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

   MUIMasterBase  = OpenLibrary("muimaster.library"   , 11);
   UtilityBase    = OpenLibrary("utility.library"     , 36);

   if(!(RexxSysBase = (struct RxsLib *)OpenLibrary(RXSNAME, 0)))
   {
      SystemTags("SYS:System/RexxMast",
         SYS_Asynch     , FALSE,
         TAG_DONE);

      RexxSysBase = (struct RxsLib *)OpenLibrary(RXSNAME, 0);
   }

   if(DOSBase && IntuitionBase && MUIMasterBase && UtilityBase && RexxSysBase)
      return(TRUE);

   if(!MUIMasterBase)
      Printf("Could not open muimaster.library\n");
   if(!UtilityBase)
      Printf("Could not open utility.library\n");
   if(!RexxSysBase)
      Printf("Could not open rexxsyslib.library\n");

   exit_libs();
   return(FALSE);
}

///
/// exit_classes
VOID exit_classes(VOID)
{
   if(CL_MainWindow)          MUI_DeleteCustomClass(CL_MainWindow);
   if(CL_Online)              MUI_DeleteCustomClass(CL_Online);

   CL_MainWindow = CL_Online = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct MainWindow_Data)    , MainWindow_Dispatcher);
   CL_Online         = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct Online_Data)        , Online_Dispatcher);

   if(CL_MainWindow && CL_Online)
      return(TRUE);

   exit_classes();
   return(FALSE);
}

///
/// exit_ports
VOID exit_ports(VOID)
{
   if(MainPort)
      DeleteMsgPort(MainPort);
   MainPort = NULL;

   if(TimeReq)
   {
// do not abortio / waitio timereq here because its possible a TR_ADDREQUEST was never sent => enforcer hit  (see serial.c: waitfor)
      CloseDevice(TimeReq);
      DeleteIORequest(TimeReq);
      TimerBase = NULL;
      TimeReq = NULL;
   }
   if(TimePort)
      DeleteMsgPort(TimePort);
   TimePort = NULL;
}

///
/// init_ports
BOOL init_ports(VOID)
{
   BOOL success = FALSE;

   if(MainPort = CreateMsgPort())
   {
      if(TimePort = (struct MsgPort *)CreateMsgPort())
      {
         if(TimeReq = (struct timerequest *)CreateIORequest(TimePort, sizeof(struct timerequest)))
         {
            if(!(OpenDevice("timer.device", UNIT_VBLANK, TimeReq, 0)))
            {
               TimerBase = &TimeReq->tr_node.io_Device->dd_Library;
               success = TRUE;
            }
         }
      }
   }
   if(success)
      return(TRUE);

   exit_ports();
   return(FALSE);
}

///

/// check_date
#ifdef DEMO
#include <resources/battclock.h>
#include <clib/battclock_protos.h>
BOOL check_date(VOID)
{
   if(BattClockBase = OpenResource("battclock.resource"))
   {
      if(ReadBattClock() < 623173854)
         return(TRUE);
   }
   return(FALSE);
}
#endif

///
/// LocalizeNewMenu
VOID LocalizeNewMenu(struct NewMenu *nm)
{
   for(; nm && nm->nm_Type!=NM_END; nm++)
   {
      if(nm->nm_Label != NM_BARLABEL)
         nm->nm_Label = GetStr(nm->nm_Label);
      if(nm->nm_CommKey)
         nm->nm_CommKey = GetStr(nm->nm_CommKey);
   }
}

///
/// HandleMainMethod
VOID HandleMainMethod(struct MsgPort *port)
{
   struct MainMessage *message;

   while(message = (struct MainMessage *)GetMsg(port))
   {
      switch(message->MethodID)
      {
         case TCM_INIT:
         case MUIM_MainWindow_WeAreOnline:
         case MUIM_List_Clear:
            message->result = DoMethod(message->obj, message->MethodID);
            break;
         case MUIM_List_Remove:
         case MUIM_List_Redraw:
            message->result = DoMethod(message->obj, message->MethodID, message->data1);
            break;
         case MUIM_Set:
         case TCM_WRITE:
         case MUIM_List_InsertSingle:
         case MUIM_List_GetEntry:
            message->result = DoMethod(message->obj, message->MethodID, message->data1, message->data2);
            break;
         case MUIM_List_Select:
            message->result = DoMethod(message->obj, message->MethodID, message->data1, message->data2, message->data3);
            break;

         case MUIM_AmiTCP_Get:
            message->result = xget(message->obj, (ULONG)message->data1);
            break;
      }
      message->MethodID = MUIM_AmiTCP_Handshake;
      ReplyMsg(message);
   }
}

///

/// Handler
VOID __saveds Handler(VOID)
{
   ULONG id;

   if(init_classes())
   {
      if(init_ports())
      {
         LocalizeNewMenu(MainMenu);
         if(app = ApplicationObject,
            MUIA_Application_Author       , "Michael Neuweiler",
            MUIA_Application_Base         , "AmiTCPControl",
            MUIA_Application_Title        , "AmiTCP Controller",
            MUIA_Application_Version      , VERSTAG,
            MUIA_Application_Copyright    , "Michael Neuweiler 1997",
            MUIA_Application_Description  , GetStr(MSG_AppDescription),
            MUIA_Application_HelpFile     , "HELP:AmiTCP.guide",
            MUIA_Application_Window       , win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE),
         End)
         {
            struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

            if(launch_amitcp())
            {
               bzero(&Config, sizeof(struct config));
               NewList((struct List *)&dialscript);
               NewList((struct List *)&user_startnet);
               NewList((struct List *)&user_stopnet);

               nr.nr_Name     = "T:AmiTCP.log";
               nr.nr_FullName = NULL;
               nr.nr_UserData = NULL;
               nr.nr_Flags    = NRF_SEND_SIGNAL;

               if((NotifySignal = AllocSignal(-1)) != -1)
               {
                  nr.nr_stuff.nr_Signal.nr_Task = (struct Task *)FindTask(NULL);
                  nr.nr_stuff.nr_Signal.nr_SignalNum = NotifySignal;
                  StartNotify(&nr);
               }

               DoMethod(win, MUIM_MainWindow_LoadConfig, DEFAULT_CONFIGFILE);
               if(Config.cnf_winstartup == 1)
                  set(win, MUIA_Window_Open, TRUE);
               if(Config.cnf_startup)
                  SystemTags(Config.cnf_startup,
                     SYS_Asynch     , FALSE,
                     TAG_DONE);
#ifdef DEMO
               if(check_date())
#endif
               if(Config.cnf_onlineonstartup)
                  DoMethod(win, MUIM_MainWindow_Online);

#ifdef DEMO
               if(!check_date())
                  MUI_Request(app, 0, 0, 0, "*_Sigh..", "Sorry, program has become invalid !");
               else
#endif
               while((id = DoMethod(app, MUIM_Application_NewInput, &sigs)) != MUIV_Application_ReturnID_Quit)
               {
                  if(sigs)
                  {
                     sigs = Wait(sigs | SIGBREAKF_CTRL_C | 1L << MainPort->mp_SigBit | 1L << NotifySignal);

                     if(sigs & SIGBREAKF_CTRL_C)
                        break;
                     if(sigs & (1L << MainPort->mp_SigBit))
                        HandleMainMethod(MainPort);
                     if(sigs & (1L << NotifySignal))
                        DoMethod(win, MUIM_MainWindow_UpdateLog);
                  }
               }
               set(win, MUIA_Window_Open, FALSE);

               if(data->online)
                  DoMethod(win, MUIM_MainWindow_Offline);
               amirexx_do_command("KILL");
               clear_config(&Config);

               if(NotifySignal != -1)
               {
                  EndNotify(&nr);
                  FreeSignal(NotifySignal);
               }
            }
            else
               MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorAmiTCPLaunch));

            MUI_DisposeObject(app);
            app = NULL;
         }
         else
            MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), "Could not create MUI application.");

         exit_ports();
      }

      exit_classes();
   }
   else
      MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), "Could not create MUI classes.");
}

///
/// main
LONG main(VOID)
{
   process = (struct Process *)FindTask(NULL);

   if(!process->pr_CLI)
   {
      WaitPort(&process->pr_MsgPort);

      WBenchMsg = (struct WBStartup *)GetMsg(&process->pr_MsgPort);
   }
   else
      WBenchMsg = NULL;

   if(init_libs())
   {
      if(!process->pr_CLI)
      {
         if(LocalCLI = CloneCLI(&WBenchMsg->sm_Message))
         {
            OldCLI = process->pr_CLI;
            process->pr_CLI = MKBADDR(LocalCLI);
         }
         WBenchLock = CurrentDir(WBenchMsg->sm_ArgList->wa_Lock);
      }

      if(StackSize(NULL) < 16384)
      {
         LONG success;
         StackCall(&success, 16384, 0, (LONG (* __stdargs)())Handler);
      }
      else
         Handler();

      if(WBenchMsg)
         CurrentDir(WBenchLock);
      if(LocalCLI)
      {
         process->pr_CLI = OldCLI;
         DeleteCLI(LocalCLI);
      }
      exit_libs();
   }

   if(WBenchMsg)
   {
      Forbid();
      ReplyMsg((struct Message *)WBenchMsg);
   }

   return(RETURN_OK);
}

///

