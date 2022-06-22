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

   exit_libs();
   return(FALSE);
}

///
/// exit_classes
VOID exit_classes(VOID)
{
   if(CL_MainWindow)          MUI_DeleteCustomClass(CL_MainWindow);
   if(CL_ModemDetect)         MUI_DeleteCustomClass(CL_ModemDetect);
   if(CL_ModemProtocol)       MUI_DeleteCustomClass(CL_ModemProtocol);
   if(CL_ModemWindow)         MUI_DeleteCustomClass(CL_ModemWindow);
   if(CL_Online)              MUI_DeleteCustomClass(CL_Online);

   CL_MainWindow = CL_ModemDetect = CL_ModemProtocol = CL_ModemWindow = CL_Online = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct MainWindow_Data)    , MainWindow_Dispatcher);
   CL_ModemProtocol  = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct ModemProtocol_Data) , ModemProtocol_Dispatcher);
   CL_ModemDetect    = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct ModemDetect_Data)   , ModemDetect_Dispatcher);
   CL_ModemWindow    = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct ModemDetect_Data)   , ModemWindow_Dispatcher);
   CL_Online         = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct Online_Data)        , Online_Dispatcher);

   if(CL_MainWindow && CL_ModemDetect && CL_ModemProtocol && CL_ModemWindow && CL_Online)
      return(TRUE);

   exit_classes();
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
      if(ReadBattClock() < 623581854)
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
/// Handler
VOID __saveds Handler(VOID)
{
   ULONG id;

   if(init_classes())
   {
      LocalizeNewMenu(MainMenu);
      if(app = ApplicationObject,
         MUIA_Application_Author       , "Michael Neuweiler",
         MUIA_Application_Base         , "SetupAmiTCP",
         MUIA_Application_Title        , "Setup AmiTCP",
         MUIA_Application_Version      , VERSTAG,
         MUIA_Application_Copyright    , "Michael Neuweiler 1997",
         MUIA_Application_Description  , GetStr(MSG_AppDescription),
         MUIA_Application_HelpFile     , "HELP:SetupAmiTCP.guide",
         MUIA_Application_Window       , WindowObject,
            WindowContents, group = VGroup,
               Child, li_script = ListObject,
                  MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                  MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
               End,
            End,
         End,
      End)
      {
         if(win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE))
         {
            struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

            DoMethod(app, OM_ADDMEMBER, win);
            set(win, MUIA_Window_Open, TRUE);
            if(!xget(win, MUIA_Window_Open))    // check if there was enogh space to open window
            {
               set(data->GR_Picture, MUIA_ShowMe, FALSE);
               set(win, MUIA_Window_Open, TRUE);
            }

            DoMethod(win, MUIM_MainWindow_About);

            ser_buf_pos = 0;
            serial_buffer_old1[0] = NULL;
            serial_buffer_old2[0] = NULL;
            bzero(&Config, sizeof(struct config));
#ifdef DEMO
            if(!check_date())
               MUI_Request(app, 0, 0, 0, "*_Sigh..", "Sorry, program has become invalid !");
            else
#endif
            while((id = DoMethod(app, MUIM_Application_NewInput, &sigs)) != MUIV_Application_ReturnID_Quit)
            {
               if(id == ID_NODIALTONE_REQ)
                  MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_NoDialtone));
               if(sigs)
               {
                  sigs = Wait(sigs | SIGBREAKF_CTRL_C);

                  if(sigs & SIGBREAKF_CTRL_C)
                     break;
               }
            }
            if(data->ihnode_added)
            {
               DoMethod(app, MUIM_Application_RemInputHandler, &data->ihnode);
               data->ihnode_added = FALSE;
            }
            set(win, MUIA_Window_Open, FALSE);
         }
         MUI_DisposeObject(app);
         app = NULL;

         close_serial();
      }
      exit_classes();
   }
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

