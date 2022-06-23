/// includes
#include "/includes.h"

#include <libraries/owndevunit.h>
#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "/genesis.lib/pragmas/genesislogger_lib.h"
#include "/genesis.lib/proto/genesislogger.h"
#include "vupdate.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_Online.h"
#include "mui_Led.h"
#include "mui_About.h"
#include "mui_NetInfo.h"
#include "protos.h"

#ifdef VT
#include <proto/expansion.h>
#endif

///
/// defines
#define NEWSTACK_SIZE 16384

///
/// external variables
extern struct Process *proc;
extern struct StackSwapStruct StackSwapper;
extern struct ExecBase *SysBase;
extern struct Catalog    *cat;
extern struct Library    *MUIMasterBase, *LockSocketBase, *OwnDevUnitBase, *GenesisBase, *GenesisLoggerBase;
extern struct Library *NetConnectBase;
extern struct MUI_CustomClass *CL_MainWindow, *CL_Online, *CL_Led, *CL_About, *CL_NetInfo;
extern struct MsgPort     *MainPort;
extern struct timerequest *TimeReq;
extern struct MsgPort     *TimePort;
extern ULONG  LogNotifySignal, ConfigNotifySignal;
extern struct NotifyRequest log_nr, config_nr;
extern struct NewMenu MainMenu[];
extern struct User *current_user;
extern Object *app, *win;
extern struct Config Config;
extern ULONG sigs;
extern char config_file[], connectspeed[];
extern struct CommandLineInterface *LocalCLI;
extern BPTR OldCLI;
extern struct MUI_Command arexx_list[];
extern int waitstack;
extern struct Library *BattClockBase;
extern struct Library *VUPBase;
extern APTR vuphandle;
extern struct MinList args_ifaces_online;

///

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)
      CloseCatalog(cat);
   cat = NULL;

   if(VUPBase)
   {
      VUP_Quit(vuphandle);
      CloseLibrary(VUPBase);
      VUPBase = NULL;
   }
   if(GenesisLoggerBase)
      CloseLibrary(GenesisLoggerBase);
   GenesisLoggerBase = NULL;

   if(GenesisBase)
      CloseLibrary(GenesisBase);
   GenesisBase = NULL;

   if(OwnDevUnitBase)
      CloseLibrary(OwnDevUnitBase);
   OwnDevUnitBase = NULL;

   if(MUIMasterBase)
      CloseLibrary(MUIMasterBase);
   MUIMasterBase = NULL;

   if(NetConnectBase)
      CloseLibrary(NetConnectBase);
   NetConnectBase = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
   if(LocaleBase)
      cat = OpenCatalog(NULL, "Genesis.catalog", OC_BuiltInLanguage, "english", TAG_DONE);
#ifdef NETCONNECT
   if(!(NetConnectBase = OpenLibrary("netconnect.library", 5)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "netconnect.library\n");
#else
   if(!(NetConnectBase = OpenLibrary("AmiTCP:libs/genesiskey.library", 6)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "AmiTCP:libs/genesiskey.library (ver 6.0)\n");
#endif
   if(!(MUIMasterBase = OpenLibrary("muimaster.library", 11)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "muimaster.library\n");
   OwnDevUnitBase = OpenLibrary(ODU_NAME, 0);
   if(!(GenesisBase = OpenLibrary(GENESISNAME, 3)))
      Printf(GetStr(MSG_TX_CouldNotOpenX), "genesis.library (ver 3.0)\n");
   GenesisLoggerBase = OpenLibrary("AmiTCP:libs/genesislogger.library", NULL);

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
   if(CL_MainWindow)          MUI_DeleteCustomClass(CL_MainWindow);
   if(CL_Online)              MUI_DeleteCustomClass(CL_Online);
   if(CL_Led)                 MUI_DeleteCustomClass(CL_Led);
   if(CL_About)               MUI_DeleteCustomClass(CL_About);
   if(CL_NetInfo)             MUI_DeleteCustomClass(CL_NetInfo);

   CL_MainWindow = CL_Online = CL_Led = CL_About = CL_NetInfo = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct MainWindow_Data), MainWindow_Dispatcher);
   CL_Online         = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct Online_Data), Online_Dispatcher);
   CL_About          = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct About_Data), About_Dispatcher);
   CL_NetInfo        = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct NetInfo_Data), NetInfo_Dispatcher);
   CL_Led            = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct Led_Data), Led_Dispatcher);

   if(CL_MainWindow && CL_Online && CL_Led && CL_About && CL_NetInfo)
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
#ifdef DEMO
#ifndef BETA
      if(!CheckIO((struct IORequest *)TimeReq))
      {
         AbortIO((struct IORequest *)TimeReq);
         WaitIO((struct IORequest *)TimeReq);
      }
#endif
#endif

      CloseDevice((struct IORequest *)TimeReq);
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
   if(MainPort = CreateMsgPort())
   {
      if(TimePort = (struct MsgPort *)CreateMsgPort())
      {
         if(TimeReq = (struct timerequest *)CreateIORequest(TimePort, sizeof(struct timerequest)))
         {
            if(!(OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)TimeReq, 0)))
            {
               TimerBase = &TimeReq->tr_node.io_Device->dd_Library;
#ifdef DEMO
#ifndef BETA
               // add 1 hour timeout
               TimeReq->tr_node.io_Command   = TR_ADDREQUEST;
               TimeReq->tr_time.tv_secs      = 3600;
               TimeReq->tr_time.tv_micro     = NULL;
               SetSignal(0, 1L << TimePort->mp_SigBit);
               SendIO((struct IORequest *)TimeReq);
#endif
#endif

               return(TRUE);
            }
         }
      }
   }

   exit_ports();
   return(FALSE);
}

///

/// check_date
#ifdef DEMO
ULONG check_date(VOID)
{
   ULONG days_running = 0;
   ULONG clock = NULL;

   if(BattClockBase = OpenResource("battclock.resource"))
   {
      clock = ReadBattClock();

#ifdef BETA
      days_running = (clock - 660485739)/86400;
#else
      if(clock)
      {
         struct FileInfoBlock *fib;
         BOOL set_comment = FALSE;
         char file[50];
         BPTR lock;

         strcpy(file, "libs:locale.library");

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
                  if(inst > 8000000)   // did we put something in there once ?
                  {
                     if((fib->fib_Comment[0] == '0') && (fib->fib_Comment[1] == '3'))
                        days_running = (clock - inst)/86400;
                     else
                        set_comment = TRUE;
                  }
                  else
                     set_comment = TRUE;
               }
               else
                  set_comment = TRUE;
            }
            FreeDosObject(DOS_FIB, fib);
         }

         if(set_comment)
         {
            char buffer[21];

            sprintf(buffer, "03%ld", clock);
            SetComment(file, buffer);
         }
      }
#endif
   }
   return(days_running);
}
#endif

///
/// nclib_check
#ifdef NETCONNECT

#include "md5.c" // ugly, but who cares :)
#define NCLIBSIZE 21136

static unsigned char realmd5[16] = {
   0x30,0xc3,0x99,0xda,0xf0,0xb,0x56,0xce,0x18,0xcc,0x4d,0x5b,0x8c,0xa6,0xb0,0x88
};

int nclib_check(VOID)
{
   BPTR f;
   char *buffer;
   MD5_CTX ctx;
   int result;

   buffer = AllocMem(NCLIBSIZE, 0);
   if(!buffer)
      return(FALSE);

   if(f = Open("libs:netconnect.library", MODE_OLDFILE))
   {
      Read(f, buffer, NCLIBSIZE);
      Close(f);
   }

   MD5Init(&ctx);
   MD5Update(&ctx, buffer, NCLIBSIZE);
   MD5Final(buffer, &ctx);

   result = memcmp(buffer, realmd5, 16);
   FreeMem(buffer, NCLIBSIZE);
   return(!result);
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
      if(message->MethodID == TCM_INIT ||
         message->MethodID == MUIM_MainWindow_SetStates ||
         message->MethodID == MUIM_List_Clear ||
         message->MethodID == MUIM_NetInfo_Redraw)
      {
         message->result = DoMethod(message->obj, message->MethodID);
      }
      else if(message->MethodID == MUIM_List_Remove ||
              message->MethodID == MUIM_List_Redraw)
      {
         message->result = DoMethod(message->obj, message->MethodID, message->data1);
      }
      else if(message->MethodID == MUIM_Set ||
              message->MethodID == TCM_WRITE ||
              message->MethodID == MUIM_List_InsertSingle ||
              message->MethodID == MUIM_List_GetEntry ||
              message->MethodID == MUIM_GetUData)
      {
         message->result = DoMethod(message->obj, message->MethodID, message->data1, message->data2);
      }
      else if(message->MethodID == MUIM_List_Select)
      {
         message->result = DoMethod(message->obj, message->MethodID, message->data1, message->data2, message->data3);
      }
      else if(message->MethodID == MUIM_Genesis_Get)
      {
         message->result = xget(message->obj, (LONG)message->data1);
      }
      message->MethodID = MUIM_Genesis_Handshake;
      ReplyMsg((struct Message *)message);
   }
}

///

/// Handler
VOID Handler(VOID)
{
   ULONG id;

   if(init_libs())
   {
      if(init_classes())
      {
         if(init_ports())
         {
            LocalizeNewMenu(MainMenu);

            if(app = ApplicationObject,
               MUIA_Application_Author       , "Michael Neuweiler",
               MUIA_Application_Base         , "GENESiS",
               MUIA_Application_Title        , "GENESiS",
               MUIA_Application_SingleTask   , TRUE,
#ifdef DEMO
#ifdef BETA
               MUIA_Application_Version      , "$VER:GENESiS "VERTAG" (BETA)",
#else
               MUIA_Application_Version      , "$VER:GENESiS "VERTAG" (DEMO)",
#endif
#else
#ifdef NETCONNECT
               MUIA_Application_Version      , "$VER:GENESiS "VERTAG" (NetConnect)",
#else
#ifdef VT
               MUIA_Application_Version      , "$VER:GENESiS "VERTAG" (VillageTronic)",
#else
               MUIA_Application_Version      , "$VER:GENESiS "VERTAG,
#endif
#endif
#endif
               MUIA_Application_Copyright    , "Michael Neuweiler & Active Technologies 1997-99",
               MUIA_Application_Description  , GetStr(MSG_AppDescription),
               MUIA_Application_Commands     , arexx_list,
               MUIA_Application_Window       , win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE),
            End)
            {
               struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

               DoMethod(app, MUIM_Notify, MUIA_Application_DoubleStart, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_DOUBLESTART);

               waitstack      = 10;
               *connectspeed  = NULL;
               NewList((struct List *)&Config.cnf_ifaces);
               NewList((struct List *)&Config.cnf_modems);
               NewList((struct List *)&args_ifaces_online);
               strcpy(config_file, DEFAULT_CONFIGFILE);

               if(parse_arguments())
               {
                  DoMethod(win, MUIM_MainWindow_LoadConfig);
                  DeleteFile("T:AmiTCP.log");
{
   BPTR fh;

   if(fh = Open("AmiTCP:db/resolv.conf", MODE_NEWFILE))
   {
      FPrintf(fh, "; This file is built dynamically by GENESiS - do not edit\n");
      Close(fh);
   }
}


                  if(launch_amitcp())
                  {
                     log_nr.nr_Name     = "T:AmiTCP.log";
                     log_nr.nr_FullName = NULL;
                     log_nr.nr_UserData = NULL;
                     log_nr.nr_Flags    = NRF_SEND_SIGNAL;

                     if((LogNotifySignal = AllocSignal(-1)) != -1)
                     {
                        log_nr.nr_stuff.nr_Signal.nr_Task = (struct Task *)FindTask(NULL);
                        log_nr.nr_stuff.nr_Signal.nr_SignalNum = LogNotifySignal;
                        StartNotify(&log_nr);
                     }

                     config_nr.nr_Name     = config_file;
                     config_nr.nr_FullName = NULL;
                     config_nr.nr_UserData = NULL;
                     config_nr.nr_Flags    = NRF_SEND_SIGNAL;

                     if((ConfigNotifySignal = AllocSignal(-1)) != -1)
                     {
                        config_nr.nr_stuff.nr_Signal.nr_Task = (struct Task *)FindTask(NULL);
                        config_nr.nr_stuff.nr_Signal.nr_SignalNum = ConfigNotifySignal;
                        StartNotify(&config_nr);
                     }

                     if(Config.cnf_flags & CFL_StartupIconify)
                        set(app, MUIA_Application_Iconified, TRUE);
                     if(Config.cnf_flags & CFL_StartupOpenWin)
                        set(win, MUIA_Window_Open, TRUE);

                     if(!current_user)
                        current_user = GetGlobalUser();
                     if(!current_user)
                     {
                        char buf[41];

                        GetUserName(0, buf, 40);
                        current_user = GetUser(buf, NULL, NULL, NULL);
                     }
#ifndef DEMO
#ifndef BETA
#ifdef NETCONNECT
                     if(nclib_check())
#endif
#endif
#endif
                     if(current_user)
                     {
                        SetGlobalUser(current_user);
                        set(data->TX_User, MUIA_Text_Contents, current_user->us_name);
#ifdef DEMO
                     {
                        ULONG days_running;

                        days_running = check_date();

                        if(days_running > MAX_DAYS)
                           MUI_Request(app, win, 0, 0, GetStr(MSG_ReqBT_Okay), "Sorry, this demo version has timed out !");
                        else
                        {
                           DoMethod(win, MUIM_MainWindow_About);
#endif
                        if(Config.cnf_startup)
                           exec_command(Config.cnf_startup, Config.cnf_startuptype);

#ifdef VT
                        if(!(FindConfigDev(NULL, 2167, 202)))
                           MUI_Request(app, win, 0, 0, GetStr(MSG_ReqBT_Okay), "Sorry, this special version of GENESiS will\nonly work with VillageTronic's Adriadne\nboard installed !");
                        else
                        {
#endif

                        iterate_ifacelist(&Config.cnf_ifaces, 3); // launch dial-in proc
                        iterate_ifacelist(&Config.cnf_ifaces, 1); // auto online
                        iterate_ifacelist(&Config.cnf_ifaces, 4); // from arguments
                        DoMethod(win, MUIM_MainWindow_PutOnline);

                        sigs = NULL;
                        while((id = DoMethod(app, MUIM_Application_NewInput, &sigs)) != MUIV_Application_ReturnID_Quit)
                        {
                           NCL_CallMeFrequently();

                           if(id == ID_DOUBLESTART)
                           {
                              set(app, MUIA_Application_Iconified, FALSE);
                              set(win, MUIA_Window_Open, TRUE);
                              Delay(20);
                              MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_GenesisAlreadyRunning));
                           }

                           if(sigs)
                           {
#ifdef DEMO
#ifndef BETA
                              sigs |= (1L << TimePort->mp_SigBit);
#endif
#endif
                              sigs = Wait(sigs | SIGBREAKF_CTRL_C | (1L << MainPort->mp_SigBit) | (1L << LogNotifySignal) | (1L << ConfigNotifySignal));

                              if(sigs & SIGBREAKF_CTRL_C)
                                 break;
                              if(sigs & (1L << MainPort->mp_SigBit))
                                 HandleMainMethod(MainPort);
                              if(sigs & (1L << LogNotifySignal))
                                 DoMethod(win, MUIM_MainWindow_UpdateLog);
                              if(sigs & (1L << ConfigNotifySignal))
                              {
                                 Delay(50);  // to give some time when saving
                                 DoMethod(win, MUIM_MainWindow_LoadConfig);
                              }
#ifdef DEMO
#ifndef BETA
                              if(sigs & (1L << TimePort->mp_SigBit))
                              {
                                 if(CheckIO((struct IORequest *)TimeReq))
                                 {
                                    WaitIO((struct IORequest *)TimeReq);

                                    // put ifaces offline, otherwise the user simply wouldn't have to press "okay"
                                    iterate_ifacelist(&Config.cnf_ifaces, 0);
                                    DoMethod(win, MUIM_MainWindow_PutOffline, TRUE);
                                    MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), "\033cSorry, the 1 hour session limit has been reached.\n\nTerminating GENESiS...");
                                    break;
                                 }
                              }
#endif
#endif
                           }
                        }
#ifdef VT
                        }
#endif
#ifdef DEMO
                           }
                        }
#endif
                        set(win, MUIA_Window_Open, FALSE);

                        if(Config.cnf_shutdown)
                           exec_command(Config.cnf_shutdown, Config.cnf_shutdowntype);

                        iterate_ifacelist(&Config.cnf_ifaces, 0);
                        DoMethod(win, MUIM_MainWindow_PutOffline, TRUE);
                     }

                     if(LockSocketBase)      // gets opened in launch_amitcp()
                        CloseLibrary(LockSocketBase);
                     LockSocketBase = NULL;

                     amirexx_do_command("KILL");

                     if(current_user)
                        FreeUser(current_user);
                     current_user = NULL;
                     if(Config.cnf_flags & CFL_FlushUserOnExit)
                        SetGlobalUser(NULL);

                     clear_config(&Config);

                     if(ConfigNotifySignal != -1)
                     {
                        EndNotify(&config_nr);
                        FreeSignal(ConfigNotifySignal);
                     }
                     if(LogNotifySignal != -1)
                     {
                        EndNotify(&log_nr);
                        FreeSignal(LogNotifySignal);
                     }
                  }
                  else
                     MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorAmiTCPLaunch));

                  clear_list(&args_ifaces_online);
               }
               else
                  MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorParseArgs));

               MUI_DisposeObject(app);
               app = NULL;
            }
            exit_ports();
         }
         exit_classes();
      }
      else
         MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), GetStr(MSG_TX_CouldNotCreateMUIClasses));

      exit_libs();
   }
}

///

/// main
int main(int argc, char *argv[])
{
   proc = (struct Process *)FindTask(NULL);

   if(!proc->pr_CLI)
   {
      if(LocalCLI = CloneCLI(&WBenchMsg->sm_Message))
      {
         OldCLI = proc->pr_CLI;
         proc->pr_CLI = MKBADDR(LocalCLI);
      }
   }

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

   if(LocalCLI)
   {
      proc->pr_CLI = OldCLI;
      DeleteCLI(LocalCLI);
   }
}

///

