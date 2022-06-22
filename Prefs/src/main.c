#include "globals.c"
#include "protos.h"

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)              CloseCatalog(cat);

   if(DataTypesBase)    CloseLibrary(DataTypesBase);
   if(IFFParseBase)     CloseLibrary(IFFParseBase);
   if(IntuitionBase)    CloseLibrary(IntuitionBase);
   if(UtilityBase)      CloseLibrary(UtilityBase);
   if(MUIMasterBase)    CloseLibrary(MUIMasterBase);
   if(LocaleBase)       CloseLibrary(LocaleBase);
   if(DOSBase)          CloseLibrary(DOSBase);

   cat         = NULL;
   IFFParseBase   = IntuitionBase   = UtilityBase     =
   MUIMasterBase  = LocaleBase      = DataTypesBase   =
   DOSBase        = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
   DOSBase        = (struct DosLibrary *)OpenLibrary("dos.library", 0);
   IntuitionBase  = OpenLibrary("intuition.library"   , 0);

   if(LocaleBase  = OpenLibrary("locale.library", 38))
      cat = OpenCatalog(NULL, "AmiTCPPrefs.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

   MUIMasterBase  = OpenLibrary("muimaster.library"   , 11);
   UtilityBase    = OpenLibrary("utility.library"     , 0);
   IFFParseBase   = OpenLibrary("iffparse.library"    , 0);
   DataTypesBase  = OpenLibrary("datatypes.library"   , 0);

   if(DOSBase && MUIMasterBase && UtilityBase && IntuitionBase && IFFParseBase && DataTypesBase)
      return(TRUE);

   exit_libs();
   return(FALSE);
}

///
/// exit_classes
VOID exit_classes(VOID)
{
   if(CL_Modem)         MUI_DeleteCustomClass(CL_Modem);
   if(CL_Databases)     MUI_DeleteCustomClass(CL_Databases);
   if(CL_Dialer)        MUI_DeleteCustomClass(CL_Dialer);
   if(CL_User)          MUI_DeleteCustomClass(CL_User);
   if(CL_Provider)      MUI_DeleteCustomClass(CL_Provider);
   if(CL_MainWindow)    MUI_DeleteCustomClass(CL_MainWindow);
   if(CL_About)         MUI_DeleteCustomClass(CL_About);
   if(CL_PasswdReq)     MUI_DeleteCustomClass(CL_PasswdReq);

   CL_MainWindow = CL_User          = CL_Provider  =
   CL_Dialer     = CL_About         =
   CL_PasswdReq  = CL_Databases     = CL_Modem     =
   NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct MainWindow_Data) , MainWindow_Dispatcher);
   CL_User           = MUI_CreateCustomClass(NULL, MUIC_Group  , NULL, sizeof(struct User_Data)       , User_Dispatcher);
   CL_Provider       = MUI_CreateCustomClass(NULL, MUIC_Group  , NULL, sizeof(struct Provider_Data)   , Provider_Dispatcher);
   CL_Dialer         = MUI_CreateCustomClass(NULL, MUIC_Group  , NULL, sizeof(struct Dialer_Data)     , Dialer_Dispatcher);
   CL_Databases      = MUI_CreateCustomClass(NULL, MUIC_Group  , NULL, sizeof(struct Databases_Data)  , Databases_Dispatcher);
   CL_Modem          = MUI_CreateCustomClass(NULL, MUIC_Group  , NULL, sizeof(struct Modem_Data)      , Modem_Dispatcher);
   CL_About          = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct About_Data)      , About_Dispatcher);
   CL_PasswdReq      = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct PasswdReq_Data)  , PasswdReq_Dispatcher);

   if(CL_MainWindow     && CL_User          && CL_Provider    &&
      CL_Dialer         && CL_About         && CL_PasswdReq   &&
      CL_Databases      && CL_Modem)
      return(TRUE);

   exit_classes();
   return(FALSE);
}

///
/// GetStr
STRPTR GetStr(STRPTR idstr)
{
   STRPTR local;

   local = idstr + 2;

   if(LocaleBase)
      return((STRPTR)GetCatalogStr(cat, *(UWORD *)idstr, local));

   return(local);
}

///
/// LocalizeNewMenu
VOID LocalizeNewMenu(struct NewMenu *nm)
{
   for (;nm->nm_Type!=NM_END;nm++)
      if (nm->nm_Label != NM_BARLABEL)
         nm->nm_Label = GetStr(nm->nm_Label);
}

///
/// check_date
#ifdef DEMO
#include <resources/battclock.h>
#include <clib/battclock_protos.h>
BOOL check_date(VOID)
{
   struct FileInfoBlock *fib;
   BOOL success = TRUE, set_comment = FALSE;
   char file[50];
   BPTR lock;

   strcpy(file, "libs:locale.library");

   if(BattClockBase = OpenResource("battclock.resource"))
   {
      if(fib = AllocDosObject(DOS_FIB, NULL))
      {
         if(lock = Lock(file, ACCESS_READ))
         {
            Examine(lock, fib);
            UnLock(lock);

            if(strlen(fib->fib_Comment) > 4)
            {
               ULONG inst;

               inst = atol((STRPTR)(fib->fib_Comment + 2));
// 8640000 = 100 days
// 2592000 = 30 days
               if(inst > 8640000)
               {
                  if(inst + 8640000 < ReadBattClock())
                  {
                     if((fib->fib_Comment[0] == '0') && (fib->fib_Comment[1] == '1'))
                        success = FALSE;
                     else
                        set_comment = TRUE;
                  }
               }
               else
                  set_comment = TRUE;
            }
            else
               set_comment = TRUE;
         }
         FreeDosObject(DOS_FIB, fib);
      }
   }

   if(set_comment)
   {
      char buffer[15];

      sprintf(buffer, "01%ld", ReadBattClock());
      SetComment(file, buffer);
   }

   return(success);
}
#endif

///
/// Handler
VOID __saveds Handler(VOID)
{
   ULONG sigs = NULL;

   if(app = ApplicationObject,
      MUIA_Application_Author       , "Michael Neuweiler",
      MUIA_Application_Base         , "AmiTCPPrefs",
      MUIA_Application_Title        , "AmiTCP Prefs",
      MUIA_Application_Version      , VERSTAG,
      MUIA_Application_Copyright    , GetStr(MSG_AppCopyright),
      MUIA_Application_Description  , GetStr(MSG_AppDescription),
      MUIA_Application_HelpFile     , "NetConnect:Docs/AmiTCP.guide",
      MUIA_Application_Window       , win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE),
   End)
   {
      if(DoMethod(win, MUIM_MainWindow_InitGroups, NULL))
      {
         struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

         set(win, MUIA_Window_Open, TRUE);
         set(data->GR_Pager, MUIA_Grouppager_Active, 0);

         strcpy(config_file, DEFAULT_CONFIGFILE);
         DoMethod(win, MUIM_MainWindow_LoadConfig, config_file);

         changed_passwd   = changed_group = changed_hosts    = changed_protocols =
         changed_services = changed_inetd = changed_networks = changed_rpc       = FALSE;

#ifdef DEMO
         DoMethod(win, MUIM_MainWindow_About);
         if(!check_date())
            MUI_Request(app, 0, 0, 0, "*_Sigh..", "Sorry, program has become invalid !");
         else
#endif
         while(DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
         {
            if(sigs)
            {
               sigs = Wait(sigs | SIGBREAKF_CTRL_C);
               if(sigs & SIGBREAKF_CTRL_C)
                  break;
            }
         }
         set(win, MUIA_Window_Open, FALSE);
      }
      MUI_DisposeObject(app);
      app = NULL;
   }
}

///
/// main
LONG main(VOID)
{
   ThisProcess = (struct Process *)FindTask(NULL);

   if(!ThisProcess->pr_CLI)
   {
      WaitPort(&ThisProcess->pr_MsgPort);

      WBenchMsg = (struct WBStartup *)GetMsg(&ThisProcess->pr_MsgPort);
   }
   else
      WBenchMsg = NULL;

   if(init_libs())
   {
      if(!ThisProcess -> pr_CLI)
         WBenchLock = CurrentDir(WBenchMsg->sm_ArgList->wa_Lock);

      if(init_classes())
      {
         LocalizeNewMenu(MainWindowMenu);

         if(StackSize(NULL) < 16384)
         {
            LONG success;
            StackCall(&success,16384,0,(LONG (* __stdargs)())Handler);
         }
         else
            Handler();

         exit_classes();
      }
      if(WBenchMsg)
         CurrentDir(WBenchLock);
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

