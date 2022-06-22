#define MUIM_Provider_NewISP                  (TAGBASE_PREFS | 0x1040)
#define MUIM_Provider_EditISP                 (TAGBASE_PREFS | 0x1042)
#define MUIM_Provider_EditISPFinish           (TAGBASE_PREFS | 0x1043)
#define MUIM_Provider_SetStates               (TAGBASE_PREFS | 0x1044)

struct MUIP_Provider_EditFinish       { ULONG MethodID; Object *win; LONG ok; };

struct Provider_Data
{
   Object *LV_ISP;
   Object *LI_ISP;

   Object *BT_New;
   Object *BT_Edit;
   Object *BT_Delete;
};

