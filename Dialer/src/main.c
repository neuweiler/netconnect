/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "protos.h"

///

/// external variables
extern struct Catalog    *cat;
extern struct Library    *MUIMasterBase;
extern struct MUI_CustomClass *CL_MainWindow;
extern struct MUI_CustomClass *CL_Online;
extern struct MsgPort     *MainPort;
extern struct timerequest *TimeReq;
extern struct MsgPort     *TimePort;
extern ULONG  NotifySignal;
extern struct NotifyRequest nr;
extern struct NewMenu MainMenu[];
extern Object *app;
extern Object *win;
extern struct MinList dialscript;
extern struct MinList user_startnet;
extern struct MinList user_stopnet;
extern struct config Config;
extern ULONG sigs;

///

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)
      CloseCatalog(cat);
   cat = NULL;

   if(MUIMasterBase)
      CloseLibrary(MUIMasterBase);
   MUIMasterBase = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
   if(LocaleBase)
      cat = OpenCatalog(NULL, "Genesis.catalog", OC_BuiltInLanguage, "english", TAG_DONE);
   if(MUIMasterBase = OpenLibrary("muimaster.library", 11))
      return(TRUE);
   else
      Printf("Could not open muimaster.library\n");

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
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct MainWindow_Data), MainWindow_Dispatcher);
   CL_Online         = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct Online_Data), Online_Dispatcher);

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
   BOOL success = FALSE;

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
   if(BattClockBase = OpenResource("battclock.resource"))
   {
      if(ReadBattClock() < 624173854)
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
         message->MethodID == MUIM_MainWindow_WeAreOnline ||
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

/// main
VOID main(VOID)
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
               MUIA_Application_Version      , VERSTAG,
               MUIA_Application_Copyright    , "Michael Neuweiler 1997",
               MUIA_Application_Description  , GetStr(MSG_AppDescription),
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
                  /* got a dynamic resolv.conf ? => create empty file */
                  if(Config.cnf_dns1 == INADDR_ANY && Config.cnf_dns2 == INADDR_ANY && !*Config.cnf_domainname)
                  {
                     BPTR fh;

                     if(fh = Open("AmiTCP:db/resolv.conf", MODE_NEWFILE))
                     {
                        FPrintf(fh, "; This file is built dynamically - do not edit\n");
                        Close(fh);
                     }
                  }
                  if(Config.cnf_winstartup == 1)
                     set(win, MUIA_Window_Open, TRUE);
                  if(Config.cnf_startup)
                     SystemTags(Config.cnf_startup, TAG_DONE);
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
                  DeleteFile("T:AmiTCP.log");
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

      exit_libs();
   }
}

///
