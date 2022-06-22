/// includes
#include "/includes.h"
#pragma header

#include "rev.h"
#include "Strings.h"
#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/genesis_lib.h"
#include "mui.h"
#include "mui_Finished.h"
#include "mui_ISPInfo1.h"
#include "mui_ISPInfo2.h"
#include "mui_LoginScript.h"
#include "mui_MainWindow.h"
#include "mui_ModemDetect.h"
#include "mui_ModemStrings.h"
#include "mui_Online.h"
//#include "mui_Sana2.h"
#include "mui_SerialModem.h"
//#include "mui_SerialSana.h"
#include "mui_Advanced.h"
#include "mui_UserInfo.h"
#include "mui_Welcome.h"
#include "protos.h"

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
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_ModemDetect;
extern struct MUI_CustomClass  *CL_Online;
extern struct MUI_CustomClass  *CL_Welcome , *CL_SerialSana, *CL_SerialModem, *CL_ModemStrings, *CL_Advanced;
extern struct MUI_CustomClass  *CL_UserInfo, *CL_ISPInfo1, *CL_ISPInfo2, *CL_LoginScript, *CL_Finished, *CL_Sana2;
extern struct NewMenu MainMenu[];
extern struct MsgPort *MainPort;
extern Object *app;
extern Object *win;
extern char serial_buffer[], serial_buffer_old1[], serial_buffer_old2[];
extern WORD ser_buf_pos;
extern struct Config Config;
extern ULONG sigs;
extern BOOL no_picture, use_loginscript, easy_ppp;
extern struct ISP ISP;
extern struct Interface Iface;

///

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)              CloseCatalog(cat);
   if(GenesisBase)      CloseLibrary(GenesisBase);
   if(MUIMasterBase)    CloseLibrary(MUIMasterBase);

   cat            = NULL;
   MUIMasterBase  = GenesisBase = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
//   if(LocaleBase)
//      cat = OpenCatalog(NULL, "GenesisWizard.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

   if(!(MUIMasterBase  = OpenLibrary("muimaster.library"   , 11)))
      Printf("Couldn't open muimaster.library.\n");
   if(!(GenesisBase    = OpenLibrary(GENESISNAME, 0)))
      Printf("Couldn't open %ls.\n", GENESISNAME);

   if(MUIMasterBase && GenesisBase)
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
   if(CL_Online)              MUI_DeleteCustomClass(CL_Online);

   if(CL_Welcome)             MUI_DeleteCustomClass(CL_Welcome);
//   if(CL_SerialSana)          MUI_DeleteCustomClass(CL_SerialSana);
   if(CL_Advanced)            MUI_DeleteCustomClass(CL_Advanced);
   if(CL_SerialModem)         MUI_DeleteCustomClass(CL_SerialModem);
   if(CL_ModemStrings)        MUI_DeleteCustomClass(CL_ModemStrings);
   if(CL_UserInfo)            MUI_DeleteCustomClass(CL_UserInfo);
   if(CL_ISPInfo1)            MUI_DeleteCustomClass(CL_ISPInfo1);
   if(CL_ISPInfo2)            MUI_DeleteCustomClass(CL_ISPInfo2);
   if(CL_LoginScript)         MUI_DeleteCustomClass(CL_LoginScript);
   if(CL_Finished)            MUI_DeleteCustomClass(CL_Finished);
//   if(CL_Sana2)               MUI_DeleteCustomClass(CL_Sana2);

   CL_MainWindow = CL_ModemDetect = CL_Online = CL_Advanced =
   CL_Welcome = CL_SerialSana = CL_SerialModem = CL_ModemStrings =
   CL_UserInfo = CL_ISPInfo1 = CL_ISPInfo2 = CL_LoginScript = CL_Finished = CL_Sana2 = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct MainWindow_Data)    , MainWindow_Dispatcher);
   CL_ModemDetect    = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct ModemDetect_Data)   , ModemDetect_Dispatcher);
   CL_Online         = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct Online_Data)        , Online_Dispatcher);

   CL_Welcome         = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct Welcome_Data)       , Welcome_Dispatcher);
//   CL_SerialSana      = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct SerialSana_Data)    , SerialSana_Dispatcher);
   CL_Advanced        = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct Advanced_Data)      , Advanced_Dispatcher);
   CL_SerialModem     = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct SerialModem_Data)   , SerialModem_Dispatcher);
   CL_ModemStrings    = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct ModemStrings_Data)  , ModemStrings_Dispatcher);
   CL_UserInfo        = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct UserInfo_Data)      , UserInfo_Dispatcher);
   CL_ISPInfo1        = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct ISPInfo1_Data)      , ISPInfo1_Dispatcher);
   CL_ISPInfo2        = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct ISPInfo2_Data)      , ISPInfo2_Dispatcher);
   CL_LoginScript     = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct LoginScript_Data)   , LoginScript_Dispatcher);
   CL_Finished        = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct Finished_Data)      , Finished_Dispatcher);
//   CL_Sana2           = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct Sana2_Data)         , Sana2_Dispatcher);

   if(CL_MainWindow && CL_ModemDetect && CL_Online && CL_Advanced &&
      CL_Welcome && /*CL_SerialSana &&*/ CL_SerialModem && CL_ModemStrings &&
      CL_UserInfo && CL_ISPInfo1 && CL_ISPInfo2 && CL_LoginScript && CL_Finished /*&& CL_Sana2*/)
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
}

///
/// init_ports
BOOL init_ports(VOID)
{
   BOOL success = FALSE;

   if(MainPort = CreateMsgPort())
   {
      return(TRUE);
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
      if(ReadBattClock() < 641814503)
         return(TRUE);
   }
   return(FALSE);
}
#endif

///
/// LocalizeNewMenu
VOID LocalizeNewMenu(struct NewMenu *nm)
{
   while(nm && nm->nm_Type != NM_END)
   {
      if(nm->nm_Label != NM_BARLABEL)
         nm->nm_Label = GetStr(nm->nm_Label);
      if(nm->nm_CommKey)
         nm->nm_CommKey = GetStr(nm->nm_CommKey);
      nm++;
   }
}

///
/// HandleMainMethod
VOID HandleMainMethod(struct MsgPort *port)
{
   struct MainMessage *message;

   while(message = (struct MainMessage *)GetMsg(port))
   {
      switch((ULONG)message->MethodID)
      {
         case TCM_INIT:
         case MUIM_MainWindow_SetPage:
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
         case MUIM_MainWindow_MUIRequest:
         case MUIM_List_GetEntry:
            message->result = DoMethod(message->obj, message->MethodID, message->data1, message->data2);
            break;
         case MUIM_List_Select:
            message->result = DoMethod(message->obj, message->MethodID, message->data1, message->data2, message->data3);
            break;

         case MUIM_Genesis_Get:
            message->result = xget(message->obj, (ULONG)message->data1);
            break;
         case MUIM_Serial_Send:
            serial_send((STRPTR)message->obj, (LONG)message->data1);
            break;
         case MUIM_Serial_WaitFor:
            message->result = serial_waitfor(message->data1, message->data2, message->data3, (int)message->obj);
            break;
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
               MUIA_Application_Base         , "GenesisWizard",
               MUIA_Application_Title        , "Genesis Wizard",
               MUIA_Application_Version      , "$VER:GenesisWizard "VERTAG,
               MUIA_Application_Copyright    , "Michael Neuweiler 1997,98",
               MUIA_Application_HelpFile     , "PROGDIR:GenesisWizard.guide",
               MUIA_Application_Description  , GetStr(MSG_AppDescription),
               MUIA_Application_Window       , win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE),
            End)
            {
               struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

               set(win, MUIA_Window_Open, TRUE);
               if(!xget(win, MUIA_Window_Open))    // check if there was enogh space to open window
               {
                  struct Welcome_Data *w_data = INST_DATA(CL_Welcome->mcc_Class, data->GR_Active);

                  set(w_data->GR_Picture, MUIA_ShowMe, FALSE);
                  no_picture = TRUE;
                  set(win, MUIA_Window_Open, TRUE);
               }
               else
                  no_picture = FALSE;

               // init variables

               ser_buf_pos = 0;
               serial_buffer_old1[0] = NULL;
               serial_buffer_old2[0] = NULL;

               bzero(&Config, sizeof(struct Config));
               Config.cnf_baudrate = 38400;
               Config.cnf_flags = CFL_7Wire;
               Config.cnf_serbuflen = 16384;
               Config.cnf_redialattempts = 10;
               Config.cnf_redialdelay = 5;
               strcpy(Config.cnf_initstring, "AT&F&D2");
               strcpy(Config.cnf_dialprefix, "ATDT");
               strcpy(Config.cnf_modemname, "Generic");
               strcpy(Config.cnf_serialdevice, "serial.device");

               bzero(&ISP, sizeof(struct ISP));
               NewList((struct List *)&ISP.isp_loginscript);
               NewList((struct List *)&ISP.isp_nameservers);
               NewList((struct List *)&ISP.isp_domainnames);
               strcpy(ISP.isp_name, "GenesisWizard Config");
               strcpy(ISP.isp_comment, "Config generated by GenesisWizard");

               bzero(&Iface, sizeof(struct Interface));
               strcpy(Iface.if_netmask, "255.255.255.0");
               Iface.if_MTU    = 1500;
               use_loginscript = TRUE;
               easy_ppp = TRUE;

#ifdef DEMO
               if(!check_date())
                  MUI_Request(app, 0, 0, 0, "*_Sigh..", "Sorry, this demo version has expired !");
               else
#endif
               while((id = DoMethod(app, MUIM_Application_NewInput, &sigs)) != MUIV_Application_ReturnID_Quit)
               {
                  if(sigs)
                  {
                     sigs = Wait(sigs | SIGBREAKF_CTRL_C | 1L << MainPort->mp_SigBit);

                     if(sigs & SIGBREAKF_CTRL_C)
                        break;
                     if(sigs & (1L << MainPort->mp_SigBit))
                        HandleMainMethod(MainPort);
                  }
               }
               set(win, MUIA_Window_Open, FALSE);

               MUI_DisposeObject(app);
               app = NULL;

               serial_delete();
               clear_list(&ISP.isp_loginscript);
            }
            else
               MUI_Request(0,0,0,0, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorMUIApp));
            exit_ports();
         }
         else
            MUI_Request(0,0,0,0, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorMsgPorts));
         exit_classes();
      }
      else
         MUI_Request(0,0,0,0, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorMUICustomClasses));
      exit_libs();
   }
   else
   {
      Printf(GetStr(MSG_TX_ErrorOpenX), "muimaster.library");
      Printf("\n\n");
   }
}

///

#define NEWSTACK_SIZE 16384
/// main
int main(int argc, char *argv[])
{
   if(SysBase->LibNode.lib_Version < 37)
   {
      static UBYTE AlertData[] = "\0\214\020GenesisWizard requires kickstart v37+ !!!\0\0";

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

