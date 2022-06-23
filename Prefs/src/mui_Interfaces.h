#define MUIM_Interfaces_NewIface             (TAGBASE_PREFS | 0x1050)
#define MUIM_Interfaces_Edit                 (TAGBASE_PREFS | 0x1051)
#define MUIM_Interfaces_EditFinish           (TAGBASE_PREFS | 0x1052)
#define MUIM_Interfaces_SetStates            (TAGBASE_PREFS | 0x1053)

struct MUIP_Interfaces_NewIface         { ULONG MethodID; LONG copy; };
struct MUIP_Interfaces_EditFinish       { ULONG MethodID; Object *win; LONG ok; };

struct Interfaces_Data
{
   Object *LV_Interfaces;
   Object *LI_Interfaces;

   Object *BT_New;
   Object *BT_Copy;
   Object *BT_Edit;
   Object *BT_Delete;
};

