#define MUIM_User_SetStates               (TAGBASE_PREFS | 0x1030)
#define MUIM_User_NewUser                 (TAGBASE_PREFS | 0x1031)
#define MUIM_User_Edit                    (TAGBASE_PREFS | 0x1032)
#define MUIM_User_EditFinish              (TAGBASE_PREFS | 0x1033)

struct MUIP_User_EditFinish   { ULONG MethodID; Object *win; LONG ok; };

struct User_Data
{
   Object *LV_User;
   Object *LI_User;
   Object *BT_New;
   Object *BT_Delete;
   Object *BT_Edit;
};

