#define MUIM_UserWindow_Init           (TAGBASE_PREFS | 0x10b0)
#define MUIM_UserWindow_CopyData       (TAGBASE_PREFS | 0x10b1)
#define MUIM_UserWindow_ChangePassword (TAGBASE_PREFS | 0x10b2)
#define MUIM_UserWindow_DisableActive  (TAGBASE_PREFS | 0x10b3)
#define MUIM_UserWindow_RemovePassword (TAGBASE_PREFS | 0x10b4)
#define MUIM_UserWindow_NameActive     (TAGBASE_PREFS | 0x10b5)
#define MUIM_UserWindow_HomeDirActive  (TAGBASE_PREFS | 0x10b6)

struct MUIP_UserWindow_Init                { ULONG MethodID; struct Prefs_User *p_user; };

struct UserWindow_Data
{
   struct Prefs_User *p_user;
   char password[41];

   Object *STR_Name;
   Object *STR_RealName;

   Object *CH_Disabled;
   Object *PA_HomeDir;
   Object *STR_HomeDir;
   Object *STR_Shell;
   Object *STR_UserID;
   Object *STR_GroupID;
   Object *BT_ChangePassword;
   Object *BT_RemovePassword;

   Object *BT_Okay;
   Object *BT_Cancel;
};

