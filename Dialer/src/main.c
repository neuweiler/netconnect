// WARNING: catalog doesn't get opened, GetStr() crashes on a500

/// includes
#include "/includes.h"
#pragma header

#include <libraries/owndevunit.h>
#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/genesis_lib.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_IfaceReq.h"
#include "mui_Led.h"
#include "protos.h"

///
/// external variables
extern struct Process *proc;
extern struct StackSwapStruct StackSwapper;
extern struct ExecBase *SysBase;
extern struct Catalog    *cat;
extern struct Library    *MUIMasterBase, *LockSocketBase, *OwnDevUnitBase;
extern struct MUI_CustomClass *CL_MainWindow, *CL_Online, *CL_IfaceReq, *CL_Led;
extern struct MsgPort     *MainPort;
extern struct timerequest *TimeReq;
extern struct MsgPort     *TimePort;
extern ULONG  LogNotifySignal, ConfigNotifySignal;
extern struct NotifyRequest log_nr, config_nr;
extern struct NewMenu MainMenu[];
extern Object *app, *win;
extern struct Config Config;
extern ULONG sigs;
extern char config_file[], connectspeed[];
#ifdef DEMO
extern struct Library *BattClockBase;
#endif

///

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)
      CloseCatalog(cat);
   cat = NULL;

   if(OwnDevUnitBase)
      CloseLibrary(OwnDevUnitBase);
   OwnDevUnitBase = NULL;

   if(MUIMasterBase)
      CloseLibrary(MUIMasterBase);
   MUIMasterBase = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
//   if(LocaleBase)
//      cat = OpenCatalog(NULL, "Genesis.catalog", OC_BuiltInLanguage, "english", TAG_DONE);
   if(!(MUIMasterBase = OpenLibrary("muimaster.library", 11)))
      Printf("Could not open muimaster.library\n");
   OwnDevUnitBase = OpenLibrary(ODU_NAME, 0);

   if(MUIMasterBase)
      return(TRUE);

   exit_libs();
   return(FALSE);
}

///

/// exit_classes
VOID exit_classes(VOID)
{
   if(CL_MainWindow)          MUI_DeleteCustomClass(CL_MainWindow);
   if(CL_Online)              MUI_DeleteCustomClass(CL_Online);
   if(CL_IfaceReq)            MUI_DeleteCustomClass(CL_IfaceReq);
   if(CL_Led)                 MUI_DeleteCustomClass(CL_Led);

   CL_MainWindow = CL_Online = CL_IfaceReq = CL_Led = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct MainWindow_Data), MainWindow_Dispatcher);
   CL_Online         = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct Online_Data), Online_Dispatcher);
   CL_IfaceReq       = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct IfaceReq_Data), IfaceReq_Dispatcher);
   CL_Led            = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct Led_Data), Led_Dispatcher);

   if(CL_MainWindow && CL_Online && CL_IfaceReq && CL_Led)
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
#include <resources/battclock.h>
#include <clib/battclock_protos.h>
BOOL check_date(VOID)
{
   // one month : 2592000

   if(BattClockBase = OpenResource("battclock.resource"))
   {
      if(ReadBattClock() < 644407784)
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
      if(message->MethodID == TCM_INIT ||
         message->MethodID == MUIM_MainWindow_SetStates ||
         message->MethodID == MUIM_List_Clear)
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
              message->MethodID == MUIM_List_GetEntry)
      {
         message->result = DoMethod(message->obj, message->MethodID, message->data1, message->data2);
      }
      else if(message->MethodID == MUIM_List_Select)
      {
         message->result = DoMethod(message->obj, message->MethodID, message->data1, message->data2, message->data3);
      }
      else if(message->MethodID == MUIM_Genesis_Get)
      {
         message->result = xget(message->obj, (ULONG)message->data1);
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
               MUIA_Application_Base         , "Genesis",
               MUIA_Application_Title        , "Genesis",
               MUIA_Application_SingleTask   , TRUE,
               MUIA_Application_Version      , "$VER:Genesis "VERTAG,
               MUIA_Application_Copyright    , "Michael Neuweiler 1997,98",
               MUIA_Application_Description  , GetStr(MSG_AppDescription),
               MUIA_Application_Window       , win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE),
            End)
            {
               struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

               DoMethod(app, MUIM_Notify, MUIA_Application_DoubleStart, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_DOUBLESTART);

               connectspeed[0] = NULL;
               bzero(&Config, sizeof(struct Config));
               strcpy(config_file, DEFAULT_CONFIGFILE);
               DoMethod(win, MUIM_MainWindow_LoadConfig);

               DeleteFile("T:AmiTCP.log");
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

#ifdef DEMO
                  if(!check_date())
                     MUI_Request(app, 0, 0, 0, "*_Sigh..", "Sorry, program has become invalid !");
                  else
                  {
#endif
                  // put always_online online since it wasn't done in MainWindow_ChangeProvider cause AmiTCP wasn't running then
                  iterate_ifacelist(&data->isp.isp_ifaces, 1);
                  DoMethod(win, MUIM_MainWindow_PutOnline);

                  if(Config.cnf_startup)
                     exec_command(Config.cnf_startup, Config.cnf_startuptype);

                  while((id = DoMethod(app, MUIM_Application_NewInput, &sigs)) != MUIV_Application_ReturnID_Quit)
                  {
                     if(id == ID_DOUBLESTART)
                     {
                        if(!xget(win, MUIA_Window_Open))
                           set(win, MUIA_Window_Open, TRUE);
                        MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_Okay), "Genesis is already running");
                     }

                     if(sigs)
                     {
                        sigs = Wait(sigs | SIGBREAKF_CTRL_C | 1L << MainPort->mp_SigBit | 1L << LogNotifySignal | 1L << ConfigNotifySignal );

                        if(sigs & SIGBREAKF_CTRL_C)
                           break;
                        if(sigs & (1L << MainPort->mp_SigBit))
                           HandleMainMethod(MainPort);
                        if(sigs & (1L << LogNotifySignal))
                           DoMethod(win, MUIM_MainWindow_UpdateLog);
                        if(sigs & (1L << ConfigNotifySignal))
                        {
                           Delay(20);  // to give some time when saving
                           DoMethod(win, MUIM_MainWindow_LoadConfig);
                        }
                     }
                  }
#ifdef DEMO
                  }
#endif
                  set(win, MUIA_Window_Open, FALSE);

                  if(Config.cnf_shutdown)
                     exec_command(Config.cnf_shutdown, Config.cnf_shutdowntype);
   
                  iterate_ifacelist(&data->isp.isp_ifaces, 0);
                  DoMethod(win, MUIM_MainWindow_PutOffline);

                  if(LockSocketBase)      // gets opened in launch_amitcp()
                     CloseLibrary(LockSocketBase);
                  LockSocketBase = NULL;

                  amirexx_do_command("KILL");

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
   
               MUI_DisposeObject(app);
               app = NULL;
            }
            exit_ports();
         }
         exit_classes();
      }
      else
         MUI_Request(NULL, NULL, NULL, NULL, GetStr(MSG_BT_Abort), "Could not create MUI classes.");

      SetCurrentUser(-1);
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
      static UBYTE AlertData[] = "\0\214\020Genesis requires kickstart v37+ !!!\0\0";

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

