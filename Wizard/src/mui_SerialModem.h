#define MUIM_SerialModem_LoadData            (TAGBASE_WIZARD | 0x1040)

struct SerialModem_Data
{
   Object *GR_Picture;
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
   Object *PO_DialPrefix;
   Object *STR_DialPrefix;
   Object *LV_DialPrefix;
};
