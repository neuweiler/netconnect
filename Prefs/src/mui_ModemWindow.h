#define MUIM_ModemWindow_Init                      (TAGBASE_PREFS | 0x1070)
#define MUIM_ModemWindow_CopyData                  (TAGBASE_PREFS | 0x1071)
#define MUIM_ModemWindow_PopString_Close           (TAGBASE_PREFS | 0x1073)
#define MUIM_ModemWindow_UpdateProtocolList        (TAGBASE_PREFS | 0x1074)

#define MUIV_ModemWindow_PopString_Modem        1
#define MUIV_ModemWindow_PopString_Protocol     2

struct MUIP_ModemWindow_Init              { ULONG MethodID; struct Modem *modem; };
struct MUIP_ModemWindow_PopString_Close   { ULONG MethodID; LONG flags; };

struct ModemWindow_Data
{
   struct Modem *modem;

   Object *PO_Modem;
   Object *LV_Modems;
   Object *LI_Modems;
   Object *STR_Modem;
   Object *STR_Comment;
   Object *PO_Init;
   Object *LV_Protocols;
   Object *LI_Protocols;
   Object *STR_Init;
   Object *PO_DialPrefix;
   Object *STR_DialPrefix;
   Object *LV_DialPrefix;
   Object *STR_DialSuffix;
   Object *STR_Answer;
   Object *STR_Hangup;

   Object *PO_Device;
   Object *STR_Device;
   Object *LV_Devices;
   Object *LI_Devices;
   Object *STR_Unit;
   Object *PO_BaudRate;
   Object *STR_BaudRate;
   Object *LV_BaudRate;
   Object *STR_SerBufLen;
   Object *SL_RedialAttempts;
   Object *SL_RedialDelay;
   Object *SL_CommandDelay;

   Object *CH_IgnoreDSR;
   Object *CH_RadBoogie;
   Object *CH_OwnDevUnit;
   Object *CH_DropDTR;
   Object *CY_Handshake;

   Object *STR_Ring;
   Object *STR_Connect;
   Object *STR_NoCarrier;
   Object *STR_NoDialtone;
   Object *STR_Busy;
   Object *STR_Ok;
   Object *STR_Error;

   Object *BT_Okay;
   Object *BT_Cancel;
};

