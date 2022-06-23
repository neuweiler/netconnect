#define MUIM_NetInfo_Update          (TAGBASE_GENESIS | 0x1040)
#define MUIM_NetInfo_Redraw          (TAGBASE_GENESIS | 0x1041)

struct NetInfo_Data
{
   Object *LV_Ifaces;
   Object *LI_Ifaces;
};

