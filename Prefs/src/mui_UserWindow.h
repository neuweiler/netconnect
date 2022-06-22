#define MUIM_UserWindow_Init           (TAGBASE_PREFS | 0x1090)
#define MUIM_UserWindow_CopyData       (TAGBASE_PREFS | 0x1091)
#define MUIM_UserWindow_ChangePassword (TAGBASE_PREFS | 0x1092)
#define MUIM_UserWindow_DisableActive  (TAGBASE_PREFS | 0x1093)
#define MUIM_UserWindow_RemovePassword (TAGBASE_PREFS | 0x1094)
#define MUIM_UserWindow_NameActive     (TAGBASE_PREFS | 0x1095)
#define MUIM_UserWindow_HomeDirActive  (TAGBASE_PREFS | 0x1096)

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

