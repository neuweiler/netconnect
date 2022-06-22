#define MUIM_Modem_PopString_Close              (TAGBASE_PREFS | 0x1020)
#define MUIM_Modem_UpdateProtocolList           (TAGBASE_PREFS | 0x1021)

#define MUIV_Modem_PopString_Modem        1
#define MUIV_Modem_PopString_Protocol     2

struct MUIP_Modem_PopString_Close      { ULONG MethodID; LONG flags; };

struct Modem_Data
{
   Object *GR_Modem;
   Object *PO_Modem;
   Object *LV_Modems;
   Object *STR_Modem;
   Object *PO_ModemInit;
   Object *LV_Protocols;
   Object *STR_ModemInit;
   Object *PO_DialPrefix;
   Object *STR_DialPrefix;
   Object *LV_DialPrefix;
   Object *STR_DialSuffix;

   Object *GR_Serial;
   Object *PO_SerialDriver;
   Object *STR_SerialDriver;
   Object *LV_Devices;
   Object *STR_SerialUnit;
   Object *PO_BaudRate;
   Object *STR_BaudRate;
   Object *LV_BaudRate;
   Object *STR_SerBufLen;
   Object *SL_RedialAttempts;
   Object *SL_RedialDelay;
   Object *CH_Carrier;
   Object *CH_7Wire;
};
