#define MUIM_Modem_PopString_Close              (TAGBASE_PREFS | 0x1060)
#define MUIM_Modem_UpdateProtocolList           (TAGBASE_PREFS | 0x1061)

#define MUIV_Modem_PopString_Modem        1
#define MUIV_Modem_PopString_Protocol     2

struct MUIP_Modem_PopString_Close      { ULONG MethodID; LONG flags; };

struct Modem_Data
{
   Object *PO_Modem;
   Object *LV_Modems;
   Object *LI_Modems;
   Object *STR_Modem;
   Object *PO_InitString;
   Object *LV_Protocols;
   Object *LI_Protocols;
   Object *STR_InitString;
   Object *PO_DialPrefix;
   Object *STR_DialPrefix;
   Object *LV_DialPrefix;
   Object *STR_DialSuffix;

   Object *PO_SerialDevice;
   Object *STR_SerialDevice;
   Object *LV_Devices;
   Object *LI_Devices;
   Object *STR_SerialUnit;
   Object *PO_BaudRate;
   Object *STR_BaudRate;
   Object *LV_BaudRate;
   Object *STR_SerBufLen;
   Object *SL_RedialAttempts;
   Object *SL_RedialDelay;

   Object *CH_IgnoreDSR;
   Object *CH_RadBoogie;
   Object *CH_OwnDevUnit;
   Object *CY_Handshake;
};

