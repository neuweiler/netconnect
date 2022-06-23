/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_About.h"
#include "mui_DataBase.h"
#include "mui_IfaceWindow.h"
#include "mui_Interfaces.h"
#include "mui_MainWindow.h"
#include "mui_Modems.h"
#include "mui_ModemWindow.h"
#include "mui_Options.h"
#include "mui_PasswdReq.h"
#include "mui_User.h"
#include "mui_UserWindow.h"
#include "protos.h"
#include "grouppager_mcc.h"
#include "images/logo.h"

///
/// external variables
extern struct Catalog *cat;
extern struct Process *proc;
extern struct StackSwapStruct StackSwapper;
extern struct ExecBase *SysBase;
extern struct Library *MUIMasterBase, *GenesisBase;
extern struct Library *NetConnectBase;
extern struct MUI_CustomClass *CL_About, *CL_Database, *CL_IfaceWindow, *CL_Interfaces,
                              *CL_MainWindow, *CL_Modems, *CL_ModemWindow, *CL_Options,
                              *CL_PasswdReq, *CL_User, *CL_UserWindow, *CL_LogLevel;
extern struct NewMenu MainWindowMenu[];
extern struct MinList McpList;
extern Object *app;
extern Object *win;
extern char config_file[MAXPATHLEN];
extern BOOL changed_passwd, changed_group, changed_hosts, changed_protocols,
            changed_services, changed_inetd, changed_networks, changed_rpc, changed_inetaccess;
extern struct User *current_user;
extern struct BitMapHeader logo_header;
extern ULONG logo_colors[];
extern UBYTE logo_body[];

///

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)              CloseCatalog(cat);
   if(GenesisBase)      CloseLibrary(GenesisBase);
   if(MUIMasterBase)    CloseLibrary(MUIMasterBase);
   if(NetConnectBase)   CloseLibrary(NetConnectBase);

   NetConnectBase = NULL;
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
#ifdef NETCONNECT
   if(!(NetConnectBase = OpenLibrary("netconnect.library", 6)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "netconnect.library (ver 6)\n");
#else
#ifdef VT
   if(!(NetConnectBase = OpenLibrary("genesiskey.library", 6)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "genesiskey.library (ver 6)\n");
#else
   if(!(NetConnectBase = OpenLibrary("genesiskey.library", 7)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "genesiskey.library (ver 7)\n");
#endif
#endif

   if(!(MUIMasterBase = OpenLibrary("muimaster.library", 11)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "muimaster.library (ver 11)\n");
   if(!(GenesisBase = OpenLibrary(GENESISNAME, 3)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), GENESISNAME " (ver 3).\n");

   if(MUIMasterBase && GenesisBase && NetConnectBase)
   {
      if(NCL_GetOwner())
         return(TRUE);
      Printf("registration failed.\n");
   }

   exit_libs();
   return(FALSE);
}

///
/// exit_classes
VOID exit_classes(VOID)
{
   if(CL_MainWindow)    MUI_DeleteCustomClass(CL_MainWindow);
   if(CL_About)         MUI_DeleteCustomClass(CL_About);
   if(CL_Interfaces)    MUI_DeleteCustomClass(CL_Interfaces);
   if(CL_IfaceWindow)   MUI_DeleteCustomClass(CL_IfaceWindow);
   if(CL_Options)       MUI_DeleteCustomClass(CL_Options);
   if(CL_Modems)        MUI_DeleteCustomClass(CL_Modems);
   if(CL_ModemWindow)   MUI_DeleteCustomClass(CL_ModemWindow);
   if(CL_Database)      MUI_DeleteCustomClass(CL_Database);
   if(CL_LogLevel)      MUI_DeleteCustomClass(CL_LogLevel);

#ifdef INTERNAL_USER
   if(CL_PasswdReq)        MUI_DeleteCustomClass(CL_PasswdReq);
   if(CL_User)             MUI_DeleteCustomClass(CL_User);
   if(CL_UserWindow)       MUI_DeleteCustomClass(CL_UserWindow);
   CL_PasswdReq = CL_User = CL_UserWindow = NULL;
#endif

   CL_MainWindow  = CL_About     = CL_Interfaces  =
   CL_IfaceWindow = CL_Options   = CL_Modems      =
   CL_ModemWindow = CL_Database  = CL_LogLevel    = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct MainWindow_Data) , MainWindow_Dispatcher);
   CL_About          = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct About_Data)      , About_Dispatcher);
   CL_Interfaces     = MUI_CreateCustomClass(NULL, MUIC_Group     , NULL, sizeof(struct Interfaces_Data) , Interfaces_Dispatcher);
   CL_IfaceWindow    = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct IfaceWindow_Data), IfaceWindow_Dispatcher);
   CL_Options        = MUI_CreateCustomClass(NULL, MUIC_Register  , NULL, sizeof(struct Options_Data)    , Options_Dispatcher);
   CL_Modems         = MUI_CreateCustomClass(NULL, MUIC_Group     , NULL, sizeof(struct Modems_Data)     , Modems_Dispatcher);
   CL_ModemWindow    = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct ModemWindow_Data), ModemWindow_Dispatcher);
   CL_Database       = MUI_CreateCustomClass(NULL, MUIC_Group     , NULL, sizeof(struct Database_Data)   , Database_Dispatcher);
   CL_LogLevel       = MUI_CreateCustomClass(NULL, MUIC_Numericbutton, NULL, sizeof(struct LogLevel_Data)   , LogLevel_Dispatcher);

#ifdef INTERNAL_USER
   CL_UserWindow     = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct UserWindow_Data) , UserWindow_Dispatcher);
   CL_User           = MUI_CreateCustomClass(NULL, MUIC_Group     , NULL, sizeof(struct User_Data)       , User_Dispatcher);
   CL_PasswdReq      = MUI_CreateCustomClass(NULL, MUIC_Window    , NULL, sizeof(struct PasswdReq_Data)  , PasswdReq_Dispatcher);

   if(CL_MainWindow  && CL_About    && CL_Interfaces  &&
      CL_IfaceWindow && CL_Options  && CL_Modems      &&
      CL_ModemWindow && CL_Database && CL_LogLevel    &&
      CL_PasswdReq   && CL_User     && CL_UserWindow)
      return(TRUE);
#else
   if(CL_MainWindow  && CL_About    && CL_Interfaces  &&
      CL_IfaceWindow && CL_Options  && CL_Modems      &&
      CL_ModemWindow && CL_Database && CL_LogLevel)
      return(TRUE);
#endif

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

/// Handler
VOID Handler(VOID)
{
   ULONG sigs = NULL;
   BPTR fh;
   Object *tmp_win = NULL;

   if(init_libs())
   {
      if(init_classes())
      {
         LocalizeNewMenu(MainWindowMenu);

         if(app = ApplicationObject,
            MUIA_Application_Author       , "Michael Neuweiler",
            MUIA_Application_Base         , "GENESiSPrefs",
            MUIA_Application_Title        , "GENESiS Preferences",
            MUIA_Application_SingleTask   , TRUE,
#ifdef DEMO
#ifdef BETA
            MUIA_Application_Version      , "$VER:GENESiSPrefs "VERTAG" (BETA)",
#else
            MUIA_Application_Version      , "$VER:GENESiSPrefs "VERTAG" (DEMO)",
#endif
#else
#ifdef NETCONNECT
               MUIA_Application_Version      , "$VER:GENESiSPrefs "VERTAG" (NetConnect)",
#else
#ifdef VT
               MUIA_Application_Version      , "$VER:GENESiSPrefs "VERTAG" (VillageTronic)",
#else
               MUIA_Application_Version      , "$VER:GENESiSPrefs "VERTAG,
#endif
#endif
#endif
            MUIA_Application_Copyright    , GetStr(MSG_AppCopyright),
            MUIA_Application_Description  , GetStr(MSG_AppDescription),
            MUIA_Application_Window       , win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE),
         End)
         {
            // user must be logged in before groups are initialized
            if(current_user = GetGlobalUser())
            {
               if(current_user->us_uid != 0 || current_user->us_gid != 0)
               {
                  FreeUser(current_user);
                  current_user = NULL;
               }
            }
            if(!current_user)
               current_user = GetUser("root", NULL, NULL, NULL);

            if(tmp_win = WindowObject,
               MUIA_Window_Title       , NULL,
               MUIA_Window_CloseGadget , FALSE,
               MUIA_Window_LeftEdge    , MUIV_Window_LeftEdge_Centered,
               MUIA_Window_TopEdge     , MUIV_Window_TopEdge_Centered,
               MUIA_Window_DepthGadget , FALSE,
               MUIA_Window_SizeGadget  , FALSE,
               MUIA_Window_DragBar     , FALSE,
               WindowContents, VGroup,
                  MUIA_Background, "2:9c9c9c9c,9c9c9c9c,9c9c9c9c",
                  Child, HVSpace,
                  Child, HGroup,
                     Child, HVSpace,
                     Child, BodychunkObject,
                        MUIA_FixWidth             , LOGO_WIDTH ,
                        MUIA_FixHeight            , LOGO_HEIGHT,
                        MUIA_Bitmap_Width         , LOGO_WIDTH ,
                        MUIA_Bitmap_Height        , LOGO_HEIGHT,
                        MUIA_Bodychunk_Depth      , LOGO_DEPTH ,
                        MUIA_Bodychunk_Body       , (UBYTE *)logo_body,
                        MUIA_Bodychunk_Compression, LOGO_COMPRESSION,
                        MUIA_Bodychunk_Masking    , LOGO_MASKING,
                        MUIA_Bitmap_SourceColors  , (ULONG *)logo_colors,
                     End,
                     Child, HVSpace,
                  End,
                  Child, CLabel(GetStr(MSG_TX_LoadingPrefs)),
                  Child, HVSpace,
               End,
            End)
            {
               DoMethod(app, OM_ADDMEMBER, tmp_win);
               set(tmp_win, MUIA_Window_Open, TRUE);
            }
            set(app, MUIA_Application_Sleep, TRUE);

            NewList((struct List *)&McpList);
            if(DoMethod(win, MUIM_MainWindow_InitGroups, NULL))
            {
               struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

               set(data->GR_Pager, MUIA_Grouppager_Active, 0);
               set(win, MUIA_Window_Open, TRUE);
               if(tmp_win)
               {
                  set(tmp_win, MUIA_Window_Open, FALSE);
                  DoMethod(app, OM_REMMEMBER, tmp_win);
                  tmp_win = NULL;
               }
               set(app, MUIA_Application_Sleep, FALSE);

               if(fh = CreateDir("AmiTCP:home"))
                  UnLock(fh);
               if(fh = CreateDir("AmiTCP:db"))
                  UnLock(fh);

               strcpy(config_file, DEFAULT_CONFIGFILE);
               DoMethod(win, MUIM_MainWindow_ClearConfig);
               DoMethod(win, MUIM_MainWindow_LoadConfig, config_file);
               DoMethod(win, MUIM_MainWindow_LoadDatabase);

#ifdef INTERNAL_USER
               changed_passwd = FALSE;
#endif
               changed_group = changed_hosts = changed_protocols = changed_services =
               changed_inetd = changed_networks = changed_rpc = changed_inetaccess = FALSE;
#ifdef DEMO
               DoMethod(win, MUIM_MainWindow_About);
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

            if(current_user)
               FreeUser(current_user);
            current_user = NULL;
            clear_list(&McpList);

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
      static UBYTE AlertData[] = "\0\214\020GENESiSPrefs requires kickstart v37+ !!!\0\0";

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

