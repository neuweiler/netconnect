/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_About.h"
#include "mui_DataBase.h"
#include "mui_Dialer.h"
#include "mui_MainWindow.h"
#include "mui_Modem.h"
#include "mui_PasswdReq.h"
#include "mui_User.h"
#include "mui_Provider.h"
#include "mui_ProviderWindow.h"
#include "mui_IfaceWindow.h"
#include "mui_UserWindow.h"
#include "protos.h"
#include "mui/grouppager_mcc.h"

///
/// external variables
extern struct Catalog *cat;
extern struct Process *proc;
extern struct StackSwapStruct StackSwapper;
extern struct ExecBase *SysBase;
extern struct Library *MUIMasterBase, *GenesisBase;
#ifdef DEMO
extern struct Library *BattClockBase;
#endif
extern struct MUI_CustomClass  *CL_MainWindow, *CL_User, *CL_ProviderWindow, *CL_Provider,
                               *CL_Dialer, *CL_Users, *CL_Databases, *CL_Modem, *CL_About,
                               *CL_PasswdReq, *CL_IfaceWindow, *CL_UserWindow;
extern struct NewMenu MainWindowMenu[];
extern Object *app;
extern Object *win;
extern char config_file[MAXPATHLEN];
extern BOOL changed_passwd, changed_group, changed_hosts, changed_protocols,
            changed_services, changed_inetd, changed_networks, changed_rpc, changed_inetaccess;
extern BOOL root_authenticated;

///

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)              CloseCatalog(cat);
   if(GenesisBase)      CloseLibrary(GenesisBase);
   if(MUIMasterBase)    CloseLibrary(MUIMasterBase);

   cat            = NULL;
   GenesisBase    = NULL;
   MUIMasterBase  = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
   if(LocaleBase)
      cat = OpenCatalog(NULL, "GenesisPrefs.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

   if(!(MUIMasterBase = OpenLibrary("muimaster.library", 11)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "muimaster.library\n");
   if(!(GenesisBase = OpenLibrary(GENESISNAME, 0)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), GENESISNAME ".\n");

   if(MUIMasterBase && GenesisBase)
      return(TRUE);

   exit_libs();
   return(FALSE);
}

///
/// exit_classes
VOID exit_classes(VOID)
{
   if(CL_Modem)            MUI_DeleteCustomClass(CL_Modem);
   if(CL_Databases)        MUI_DeleteCustomClass(CL_Databases);
   if(CL_Dialer)           MUI_DeleteCustomClass(CL_Dialer);
   if(CL_ProviderWindow)   MUI_DeleteCustomClass(CL_ProviderWindow);
   if(CL_Provider)         MUI_DeleteCustomClass(CL_Provider);
   if(CL_User)             MUI_DeleteCustomClass(CL_User);
   if(CL_MainWindow)       MUI_DeleteCustomClass(CL_MainWindow);
   if(CL_About)            MUI_DeleteCustomClass(CL_About);
   if(CL_PasswdReq)        MUI_DeleteCustomClass(CL_PasswdReq);
   if(CL_IfaceWindow)      MUI_DeleteCustomClass(CL_IfaceWindow);
   if(CL_UserWindow)       MUI_DeleteCustomClass(CL_UserWindow);

   CL_MainWindow  = CL_ProviderWindow= CL_Provider  =
   CL_Dialer      = CL_About         = CL_User      =
   CL_PasswdReq   = CL_Databases     = CL_Modem     =
   CL_IfaceWindow = CL_UserWindow    = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct MainWindow_Data)    , MainWindow_Dispatcher);
   CL_ProviderWindow = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct ProviderWindow_Data), ProviderWindow_Dispatcher);
   CL_UserWindow     = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct UserWindow_Data)    , UserWindow_Dispatcher);
   CL_Provider       = MUI_CreateCustomClass(NULL, MUIC_Group     , NULL, sizeof(struct Provider_Data)      , Provider_Dispatcher);
   CL_User           = MUI_CreateCustomClass(NULL, MUIC_Group     , NULL, sizeof(struct User_Data)          , User_Dispatcher);
   CL_Dialer         = MUI_CreateCustomClass(NULL, MUIC_Register  , NULL, sizeof(struct Dialer_Data)        , Dialer_Dispatcher);
   CL_Databases      = MUI_CreateCustomClass(NULL, MUIC_Group     , NULL, sizeof(struct Databases_Data)     , Databases_Dispatcher);
   CL_Modem          = MUI_CreateCustomClass(NULL, MUIC_Register  , NULL, sizeof(struct Modem_Data)         , Modem_Dispatcher);
   CL_About          = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct About_Data)         , About_Dispatcher);
   CL_PasswdReq      = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct PasswdReq_Data)     , PasswdReq_Dispatcher);
   CL_IfaceWindow    = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct IfaceWindow_Data)   , IfaceWindow_Dispatcher);

   if(CL_MainWindow     && CL_ProviderWindow && CL_Provider    &&
      CL_Dialer         && CL_About          && CL_PasswdReq   &&
      CL_Databases      && CL_Modem          && CL_User        &&
      CL_IfaceWindow    && CL_UserWindow)
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

   if(cat)
      return((STRPTR)GetCatalogStr(cat, *(UWORD *)idstr, local));

   return(local);
}

///
/// LocalizeNewMenu
VOID LocalizeNewMenu(struct NewMenu *nm)
{
   for (;nm->nm_Type!=NM_END;nm++)
   {
      if (nm->nm_Label != NM_BARLABEL)
         nm->nm_Label = GetStr(nm->nm_Label);
      if(nm->nm_CommKey)
         nm->nm_CommKey = GetStr(nm->nm_CommKey);
   }
}

///
/// check_date
#ifdef DEMO
#include <resources/battclock.h>
#include <clib/battclock_protos.h>
BOOL check_date(VOID)
{
   // one month : 2592000

   if(BattClockBase = OpenResource("battclock.resource"))
   {
      if(ReadBattClock() < 647390515)
         return(TRUE);
   }
   return(FALSE);
}
#endif

///

/// Handler
VOID Handler(VOID)
{
   ULONG sigs = NULL;
   BPTR fh;

   if(init_libs())
   {
      if(init_classes())
      {
         LocalizeNewMenu(MainWindowMenu);

         if(app = ApplicationObject,
            MUIA_Application_Author       , "Michael Neuweiler",
            MUIA_Application_Base         , "GenesisPrefs",
            MUIA_Application_Title        , "Genesis Preferences",
            MUIA_Application_Version      , "$VER:GenesisPrefs "VERTAG,
            MUIA_Application_Copyright    , GetStr(MSG_AppCopyright),
            MUIA_Application_Description  , GetStr(MSG_AppDescription),
            MUIA_Application_Window       , win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE),
         End)
         {
            if(DoMethod(win, MUIM_MainWindow_InitGroups, NULL))
            {
               struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

               set(data->GR_Pager, MUIA_Grouppager_Active, 0);
               set(win, MUIA_Window_Open, TRUE);

               if(fh = CreateDir("AmiTCP:home"))
                  UnLock(fh);
               if(fh = CreateDir("AmiTCP:db"))
                  UnLock(fh);

               strcpy(config_file, DEFAULT_CONFIGFILE);
               DoMethod(win, MUIM_MainWindow_LoadConfig, config_file);
               DoMethod(win, MUIM_MainWindow_LoadDatabases);

               changed_passwd   = changed_group = changed_hosts    = changed_protocols =
               changed_services = changed_inetd = changed_networks = changed_rpc       =
               changed_inetaccess = FALSE;
               root_authenticated = FALSE;

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
         exit_classes();
      }
      exit_libs();
   }
}

///

#define NEWSTACK_SIZE 16384
/// main
int main(int argc, char *argv[])
{
   if(SysBase->LibNode.lib_Version < 37)
   {
      static UBYTE AlertData[] = "\0\214\020GenesisPrefs requires kickstart v37+ !!!\0\0";

      DisplayAlert(RECOVERY_ALERT, AlertData, 30);
      exit(30);
   }
   proc = (struct Process *)FindTask(NULL);
   if(((ULONG)proc->pr_Task.tc_SPUpper - (ULONG)proc->pr_Task.tc_SPLower) < NEWSTACK_SIZE)
   {
      if(!(StackSwapper.stk_Lower = AllocVec(NEWSTACK_SIZE, MEMF_ANY)))
         exit(20);
      StackSwapper.stk_Upper   = (ULONG)StackSwapper.stk_Lower + NEWSTACK_SIZE;
      StackSwapper.stk_Pointer = (APTR)StackSwapper.stk_Upper;
      StackSwap(&StackSwapper);

      Handler();

      StackSwap(&StackSwapper);
      FreeVec(StackSwapper.stk_Lower);
   }
   else
      Handler();
}

///

