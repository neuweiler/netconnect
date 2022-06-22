/// includes
#include "/includes.h"
#pragma header

#include "rev.h"
#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_Finished.h"
#include "mui_ISPInfo.h"
#include "mui_LoginScript.h"
#include "mui_MainWindow.h"
#include "mui_ModemDetect.h"
#include "mui_ModemStrings.h"
#include "mui_Online.h"
//#include "mui_Sana2.h"
#include "mui_SerialModem.h"
//#include "mui_SerialSana.h"
#include "mui_UserInfo.h"
#include "mui_Welcome.h"
#include "protos.h"

///
/// external variables
extern struct Catalog *cat;
extern struct Library *MUIMasterBase;
#ifdef DEMO
extern struct Library *BattClockBase;
#endif
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_ModemDetect;
extern struct MUI_CustomClass  *CL_Online;
extern struct MUI_CustomClass  *CL_Welcome , *CL_SerialSana, *CL_SerialModem, *CL_ModemStrings;
extern struct MUI_CustomClass  *CL_UserInfo, *CL_ISPInfo, *CL_LoginScript, *CL_Finished, *CL_Sana2;
extern struct NewMenu MainMenu[];
extern struct MsgPort *MainPort;
extern Object *app;
extern Object *win;
extern Object *group;
extern Object *li_script;
extern char serial_buffer[], serial_buffer_old1[], serial_buffer_old2[];
extern WORD ser_buf_pos;
extern struct config Config;
extern ULONG sigs;
extern BOOL no_picture, use_loginscript;
extern char ip[];

///

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)              CloseCatalog(cat);
   if(MUIMasterBase)    CloseLibrary(MUIMasterBase);

   cat            = NULL;
   MUIMasterBase  = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
   if(LocaleBase)
      cat = OpenCatalog(NULL, "GenesisWizard.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

   MUIMasterBase  = OpenLibrary("muimaster.library"   , 11);

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
   if(CL_ModemDetect)         MUI_DeleteCustomClass(CL_ModemDetect);
   if(CL_Online)              MUI_DeleteCustomClass(CL_Online);

   if(CL_Welcome)             MUI_DeleteCustomClass(CL_Welcome);
//   if(CL_SerialSana)          MUI_DeleteCustomClass(CL_SerialSana);
   if(CL_SerialModem)         MUI_DeleteCustomClass(CL_SerialModem);
   if(CL_ModemStrings)        MUI_DeleteCustomClass(CL_ModemStrings);
   if(CL_UserInfo)            MUI_DeleteCustomClass(CL_UserInfo);
   if(CL_ISPInfo)             MUI_DeleteCustomClass(CL_ISPInfo);
   if(CL_LoginScript)         MUI_DeleteCustomClass(CL_LoginScript);
   if(CL_Finished)            MUI_DeleteCustomClass(CL_Finished);
//   if(CL_Sana2)               MUI_DeleteCustomClass(CL_Sana2);

   CL_MainWindow = CL_ModemDetect = CL_Online =
   CL_Welcome = CL_SerialSana = CL_SerialModem = CL_ModemStrings =
   CL_UserInfo = CL_ISPInfo = CL_LoginScript = CL_Finished = CL_Sana2 = NULL;
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
   CL_SerialModem     = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct SerialModem_Data)   , SerialModem_Dispatcher);
   CL_ModemStrings    = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct ModemStrings_Data)  , ModemStrings_Dispatcher);
   CL_UserInfo        = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct UserInfo_Data)      , UserInfo_Dispatcher);
   CL_ISPInfo         = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct ISPInfo_Data)       , ISPInfo_Dispatcher);
   CL_LoginScript     = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct LoginScript_Data)   , LoginScript_Dispatcher);
   CL_Finished        = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct Finished_Data)      , Finished_Dispatcher);
//   CL_Sana2           = MUI_CreateCustomClass(NULL, MUIC_Group , NULL, sizeof(struct Sana2_Data)         , Sana2_Dispatcher);

   if(CL_MainWindow && CL_ModemDetect && CL_Online &&
      CL_Welcome && /*CL_SerialSana &&*/ CL_SerialModem && CL_ModemStrings &&
      CL_UserInfo && CL_ISPInfo && CL_LoginScript && CL_Finished /*&& CL_Sana2*/)
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
      if(ReadBattClock() < 626581854)
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
      if(message->MethodID == TCM_INIT ||
         message->MethodID == MUIM_MainWindow_SetPage ||
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
              message->MethodID == MUIM_MainWindow_MUIRequest ||
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
               MUIA_Application_Base         , "GenesisWizard",
               MUIA_Application_Title        , "Genesis Wizard",
               MUIA_Application_Version      , VERSTAG,
               MUIA_Application_Copyright    , "Michael Neuweiler 1997",
               MUIA_Application_Description  , GetStr(MSG_AppDescription),
               MUIA_Application_Window       , win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE),
            End)
            {
               struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
               Object *window;

               if(window = WindowObject,
                  WindowContents, group = VGroup,
                     Child, li_script = ListObject,
                        MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                        MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
                     End,
                  End,
               End)
               {
                  DoMethod(app, OM_ADDMEMBER, window);

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

                  DoMethod(win, MUIM_MainWindow_About);

                  // init variables

                  ser_buf_pos = 0;
                  serial_buffer_old1[0] = NULL;
                  serial_buffer_old2[0] = NULL;
                  bzero(&Config, sizeof(struct config));
                  Config.cnf_baudrate = 38400;
                  Config.cnf_carrierdetect = TRUE;
                  Config.cnf_7wire = TRUE;
                  Config.cnf_serbuflen = 16384;
                  Config.cnf_redialattempts = 10;
                  Config.cnf_redialdelay = 3;
                  strcpy(Config.cnf_initstring, "AT&F&D2");
                  strcpy(Config.cnf_dialprefix, "ATDT");
                  strcpy(Config.cnf_modemname, "Generic");
                  strcpy(Config.cnf_serialdevice, "serial.device");
                  strcpy(Config.cnf_ifname, "ppp");
                  strcpy(ip, "0.0.0.0");
                  use_loginscript = TRUE;

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
               }
               MUI_DisposeObject(app);
               app = NULL;

               serial_delete();
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

