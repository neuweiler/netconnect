#define MUIM_Modems_NewModem             (TAGBASE_PREFS | 0x1060)
#define MUIM_Modems_Edit                 (TAGBASE_PREFS | 0x1061)
#define MUIM_Modems_EditFinish           (TAGBASE_PREFS | 0x1062)
#define MUIM_Modems_SetStates            (TAGBASE_PREFS | 0x1063)

struct MUIP_Modems_EditFinish       { ULONG MethodID; Object *win; LONG ok; };
struct MUIP_Modems_NewModem         { ULONG MethodID; LONG copy; };

struct Modems_Data
{
   Object *LV_Modems;
   Object *LI_Modems;

   Object *BT_New;
   Object *BT_Copy;
   Object *BT_Edit;
   Object *BT_Delete;
};


