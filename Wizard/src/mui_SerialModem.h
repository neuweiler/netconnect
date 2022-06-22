#define MUIM_SerialModem_LoadData            (TAGBASE_WIZARD | 0x1040)
#define MUIM_SerialModem_ModemActive         (TAGBASE_WIZARD | 0x1041)

struct MUIP_SerialModem_ModemActive          { ULONG MethodID; ULONG popstring; };

struct SerialModem_Data
{
   Object *PO_SerialDevice;
   Object *STR_SerialDevice;
   Object *LV_SerialDevices;
   Object *SL_SerialUnit;
   Object *PO_ModemName;
   Object *LV_Modems;
   Object *TX_ModemName;
};
