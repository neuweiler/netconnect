#define MUIM_NetInfo_Update          (TAGBASE_GENESIS | 0x1040)
#define MUIM_NetInfo_Close           (TAGBASE_GENESIS | 0x1041)
#define MUIM_NetInfo_SetStates       (TAGBASE_GENESIS | 0x1042)
#define MUIM_NetInfo_OnOffline       (TAGBASE_GENESIS | 0x1043)

struct MUIP_NetInfo_OnOffline        { ULONG MethodID; LONG online; };

struct NetInfo_Data
{
   Object *MN_Strip;
   Object *LV_Ifaces;
   Object *LI_Ifaces;
   Object *BT_Online;
   Object *BT_Offline;
};

