#define MUIM_IfaceWindow_CopyData              (TAGBASE_PREFS | 0x1080)
#define MUIM_IfaceWindow_Init                  (TAGBASE_PREFS | 0x1081)
#define MUIM_IfaceWindow_PopString_Close       (TAGBASE_PREFS | 0x1082)
#define MUIM_IfaceWindow_SanaActive            (TAGBASE_PREFS | 0x1083)

#define MUIV_IfaceWindow_PopString_IfaceName     1

struct MUIP_IfaceWindow_Init                { ULONG MethodID; struct Interface *iface; };
struct MUIP_IfaceWindow_PopString_Close     { ULONG MethodID; LONG flags; };


struct IfaceWindow_Data
{
   struct Interface *iface;

   Object *CY_Sana2Device;
   Object *PA_Sana2Device;
   Object *STR_Sana2Device;
   Object *STR_Sana2Unit;
   Object *PA_Sana2ConfigFile;
   Object *STR_Sana2ConfigFile;
   Object *TI_Sana2ConfigText;

   Object *PO_IfaceName;
   Object *STR_IfaceName;
   Object *LV_IfaceNames;
   Object *STR_ConfigParams;
   Object *STR_MTU;
   Object *STR_Address;
   Object *CY_Address;
   Object *STR_Netmask;
   Object *CY_Netmask;
   Object *STR_Dest;
   Object *CY_Dest;
   Object *STR_Gateway;
   Object *CY_Gateway;

   Object *SL_KeepAlive;
   Object *CH_AlwaysOnline;

   Object *CH_CarrierDetect;
   Object *SL_ConnectTimeout;
   Object *STR_Callback;
   Object *CH_Callback;
   Object *CH_MPPCompression;
   Object *CH_VJCompression;
   Object *CH_BSDCompression;
   Object *CH_DeflateCompression;
   Object *CY_EOF;

   Object *BT_Okay;
   Object *BT_Cancel;
};

