#define MUIM_IfaceReq_BuildList           (TAGBASE_GENESIS | 0x1030)
#define MUIM_IfaceReq_Finished            (TAGBASE_GENESIS | 0x1031)

struct MUIP_IfaceReq_BuildList            { ULONG MethodID; struct MinList *list; LONG online; };
struct MUIP_IfaceReq_Finished             { ULONG MethodID; LONG okay; };

struct IfaceReq_Data
{
   struct MinList *list;
   BOOL online;  // will ifaces be put offline or online ?

   Object *LV_Interfaces;
   Object *LI_Interfaces;
   Object *BT_Okay;
   Object *BT_Cancel;
};

