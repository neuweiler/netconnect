#define MUISERIALNR_SETUP 1
#define TAGBASE_SETUP (TAG_USER | (MUISERIALNR_SETUP << 16))

#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)

#define NUM_PAGES 8

enum { ID_NODIALTONE_REQ = 21 };

/*** MUIM, MUIA, MUIV ***/
/// MainWindow
#define MUIM_MainWindow_SetPage                (TAGBASE_SETUP | 0x1000)
#define MUIM_MainWindow_NextPage               (TAGBASE_SETUP | 0x1001)
#define MUIM_MainWindow_BackPage               (TAGBASE_SETUP | 0x1002)
#define MUIM_MainWindow_About                  (TAGBASE_SETUP | 0x1003)
#define MUIM_MainWindow_Dial                   (TAGBASE_SETUP | 0x1004)
#define MUIM_MainWindow_GoOnline               (TAGBASE_SETUP | 0x1005)
#define MUIM_MainWindow_HangUp                 (TAGBASE_SETUP | 0x1006)
#define MUIM_MainWindow_SendLogin              (TAGBASE_SETUP | 0x1007)
#define MUIM_MainWindow_SendPassword           (TAGBASE_SETUP | 0x1008)
#define MUIM_MainWindow_SendBreak              (TAGBASE_SETUP | 0x1009)
#define MUIM_MainWindow_DisposeWindow          (TAGBASE_SETUP | 0x100a)
#define MUIM_MainWindow_Input                  (TAGBASE_SETUP | 0x100b)
#define MUIM_MainWindow_Trigger                (TAGBASE_SETUP | 0x100c)
#define MUIM_MainWindow_ChangeConfig           (TAGBASE_SETUP | 0x100d)
#define MUIM_MainWindow_ShowConfig             (TAGBASE_SETUP | 0x100e)
#define MUIM_MainWindow_Abort                  (TAGBASE_SETUP | 0x100f)

///
/// ModemDetect
#define MUIM_ModemDetect_Trigger                (TAGBASE_SETUP | 0x1020)
#define MUIM_ModemDetect_FindModem              (TAGBASE_SETUP | 0x1021)
#define MUIM_ModemDetect_CheckATI               (TAGBASE_SETUP | 0x1022)
#define MUIM_ModemDetect_Abort                  (TAGBASE_SETUP | 0x1023)
#define MUIM_ModemDetect_GetModem               (TAGBASE_SETUP | 0x1024)

///
/// ModemProtocol
#define MUIM_ModemProtocol_Use                  (TAGBASE_SETUP | 0x1030)
#define MUIM_ModemProtocol_InitFromPC           (TAGBASE_SETUP | 0x1031)

///
/// ModemWindow
#define MUIM_ModemWindow_Init                   (TAGBASE_SETUP | 0x1040)
#define MUIM_ModemWindow_Finish                 (TAGBASE_SETUP | 0x1041)
#define MUIM_ModemWindow_UpdateProtocolList     (TAGBASE_SETUP | 0x1042)
#define MUIM_ModemWindow_PopString              (TAGBASE_SETUP | 0x1043)

#define MUIV_ModemWindow_PopString_SerialDevice  1
#define MUIV_ModemWindow_PopString_Modem         2
#define MUIV_ModemWindow_PopString_Protocol      3
///
/// Online
#define MUIM_Online_GoOnline                    (TAGBASE_SETUP | 0x1050)
#define MUIM_Online_Abort                       (TAGBASE_SETUP | 0x1051)

///

/*** MUIP ***     ***** DO NOT USE BOOL *****/
/// MainWindow
struct MUIP_MainWindow_DisposeWindow      { ULONG MethodID; Object *window; };

///
/// ModemDetect
struct MUIP_ModemDetect_GetModem          { ULONG MEthodID; STRPTR modemname; };

///
/// ModemWindow
struct MUIP_MainWindow_Dial               { ULONG MethodID; LONG restart; };

///
/// ModemProtocol
struct MUIP_ModemProtocol_InitFromPC      { ULONG MethodID; struct pc_Data *pc_data; };

///
/// ModemWindow
struct MUIP_ModemWindow_PopString         { ULONG MethodID; LONG flags; };
struct MUIP_ModemWindow_Finish            { ULONG MethodID; LONG okay; };

///

/*** Object structures ***/
/// MainWindow
struct MainWindow_Data
{
   struct MUI_InputHandlerNode ihnode;
   int ihnode_added;

   LONG Page;
   Object *ARR_Pages[NUM_PAGES];
   Object *ARR_Pictures[NUM_PAGES];
   STRPTR ARR_FT_Infos[NUM_PAGES];

   Object *MN_Strip;

   Object *GR_Picture;
   Object *GR_Pager;
   Object *FT_Info;
   Object *GR_Active;
   Object *BC_Active;

   Object *BT_Back;
   Object *BT_Next;
   Object *BT_Abort;

   Object *RA_Interface;

   Object *TX_Device;
   Object *TX_Modem;
   Object *TX_InitString;
   Object *TX_DialPrefix;
   Object *BT_ChangeConfig;

   Object *STR_FullName;
   Object *STR_UserName;
   Object *STR_Password;

   Object *STR_PhoneNumber;
   Object *STR_IPAddress;
   Object *CY_IPAddress;
   Object *CY_Protocol;
   Object *CY_Script;

   Object *TR_Terminal;
   Object *SB_Terminal;
   Object *BT_Dial;
   Object *BT_GoOnline;
   Object *BT_HangUp;
   Object *BT_SendLogin;
   Object *BT_SendPassword;
   Object *BT_SendBreak;

   Object *STR_Config;
   Object *CH_Config;
   Object *STR_Info;
   Object *CH_Info;
   Object *STR_Printer;
   Object *CH_Printer;
   Object *BT_ShowConfig;

   Object *STR_SanaDevice;
   Object *SL_SanaUnit;
};

///
/// ModemDetect
struct ModemDetect_Data
{
   struct MUI_InputHandlerNode ihnode;
   int ihnode_added;

   char buffer[81];  // for serial input
   int buf_pos;

   struct MsgPort *time_port;
   struct timerequest *time_req;

   int action;    // what we are doing at the moment

   STRPTR *devices;
   int device_nr;
   int unit;

   char ATI[10][81];
   int ATI_nr;

   Object *TX_Info;
   Object *BU_Busy;
   Object *BT_Abort;
};

///
/// Modem Protocol
struct ModemProtocol_Data
{
   Object *LV_Protocols;
   Object *BT_Use;
};

///
/// ModemWindow
struct ModemWindow_Data
{
   Object *PO_SerialDevice;
   Object *STR_SerialDevice;
   Object *LV_SerialDevices;
   Object *SL_SerialUnit;

   Object *PO_ModemName;
   Object *LV_Modems;
   Object *TX_ModemName;
   Object *PO_InitString;
   Object *LV_Protocols;
   Object *STR_InitString;
   Object *STR_DialPrefix;
   Object *STR_DialSuffix;

   Object *BT_Okay;
   Object *BT_Cancel;
};

///
/// Online
struct Online_Data
{
   struct Process *TCPHandler;
   struct SignalSemaphore HandlerSemaphore;
   STRPTR login_ptr;       // required since TCPHandler must not call xget() !!
   STRPTR passwd_ptr;
   STRPTR addr_ptr;
   int abort;

   Object *TX_Info;
   Object *BU_Busy;
   Object *BT_Abort;
};

///

