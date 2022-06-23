#define MUIM_User_SetStates               (TAGBASE_PREFS | 0x10a0)
#define MUIM_User_NewUser                 (TAGBASE_PREFS | 0x10a1)
#define MUIM_User_Edit                    (TAGBASE_PREFS | 0x10a2)
#define MUIM_User_EditFinish              (TAGBASE_PREFS | 0x10a3)

struct MUIP_User_EditFinish   { ULONG MethodID; Object *win; LONG ok; };

struct Prefs_User
{
   char   pu_login[41];
   char   pu_realname[81];
   char   pu_password[41];
   LONG   pu_uid;
   LONG   pu_gid;
   char   pu_homedir[MAXPATHLEN];
   char   pu_shell[81];
};

struct User_Data
{
   Object *LV_User;
   Object *LI_User;
   Object *BT_New;
   Object *BT_Delete;
   Object *BT_Edit;
};

