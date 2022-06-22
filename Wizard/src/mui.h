#define MUISERIALNR_SETUP 1
#define TAGBASE_SETUP (TAG_USER | (MUISERIALNR_SETUP << 16))

#define NUM_PAGES 9

/*** MUIM, MUIA, MUIV ***/
/// MainWindow
#define MUIM_MainWindow_SetPage                (TAGBASE_SETUP | 0x1000)
#define MUIM_MainWindow_NextPage               (TAGBASE_SETUP | 0x1001)
#define MUIM_MainWindow_BackPage               (TAGBASE_SETUP | 0x1002)
#define MUIM_MainWindow_About                  (TAGBASE_SETUP | 0x1003)
#define MUIM_MainWindow_PopString              (TAGBASE_SETUP | 0x1004)
#define MUIM_MainWindow_ModemActive            (TAGBASE_SETUP | 0x1005)
#define MUIM_MainWindow_CreateProviderList     (TAGBASE_SETUP | 0x1006)
#define MUIM_MainWindow_ProviderSelect         (TAGBASE_SETUP | 0x1007)
#define MUIM_MainWindow_Dial                   (TAGBASE_SETUP | 0x1008)
#define MUIM_MainWindow_GoOnline               (TAGBASE_SETUP | 0x1009)
#define MUIM_MainWindow_HangUp                 (TAGBASE_SETUP | 0x100a)
#define MUIM_MainWindow_SendLogin              (TAGBASE_SETUP | 0x100b)
#define MUIM_MainWindow_SendPassword           (TAGBASE_SETUP | 0x100c)
#define MUIM_MainWindow_SendBreak              (TAGBASE_SETUP | 0x100d)
#define MUIM_MainWindow_CloseInfoWindow        (TAGBASE_SETUP | 0x100e)
#define MUIM_MainWindow_Input                  (TAGBASE_SETUP | 0x100f)

#define MUIV_MainWindow_PopString_SerialDevice  1

///
/// InfoWindow
#define MUIM_InfoWindow_Abort                (TAGBASE_SETUP | 0x1010)

///
/// InfoText
#define MUIM_InfoText_TimeTrigger            (TAGBASE_SETUP | 0x1020)

///

/*** MUIP ***     ***** DO NOT USE BOOL *****/
/// MainWindow
struct MUIP_MainWindow_PopString          { ULONG MethodID; LONG flags; };
struct MUIP_MainWindow_CreateProviderList { ULONG MethodID; STRPTR path; };
struct MUIP_MainWindow_Input              { ULONG MethodID; struct InputEvent *ie; };

///

/*** Object structures ***/
/// MainWindow
struct MainWindow_Data
{
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

	Object *PO_SerialDevice;
	Object *STR_SerialDevice;
	Object *LV_SerialDevices;
	Object *SL_SerialUnit;

	Object *LV_Modems;
	Object *STR_ModemName;
	Object *STR_ModemInit;
	Object *STR_DialPrefix;
	Object *STR_DialSuffix;

	Object *LT_Providers;
	Object *TX_PoP;
	Object *TX_Phone;
	Object *BT_ClearProvider;

	Object *STR_UserName;
	Object *STR_Password;
	Object *STR_PhoneNumber;
	Object *STR_IPAddress;
	Object *CY_IPAddress;
	Object *CY_Protocol;

	Object *LV_Terminal;
	Object *LI_Terminal;
	Object *BT_Dial;
	Object *BT_GoOnline;
	Object *BT_HangUp;
	Object *BT_SendLogin;
	Object *BT_SendPassword;
	Object *BT_SendBreak;

	Object *STR_Config;
	Object *STR_Info;
	Object *STR_Printer;

	Object *STR_SanaDevice;
	Object *SL_SanaUnit;
};

///
/// InfoWindow
struct InfoWindow_Data
{
	Object *TX_Info;
	Object *BU_Busy;
	Object *BT_Abort;
};

///
/// InfoText
struct InfoText_Data
{
	struct MUI_InputHandlerNode ihnode;
};

///

