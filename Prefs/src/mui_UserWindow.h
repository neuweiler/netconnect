#define MUIM_UserWindow_Init           (TAGBASE_PREFS | 0x1090)
#define MUIM_UserWindow_CopyData       (TAGBASE_PREFS | 0x1091)

struct MUIP_UserWindow_Init                { ULONG MethodID; struct User *user; };

struct UserWindow_Data
{
   struct User *user;

   Object *STR_Name;
   Object *STR_RealName;
   Object *STR_EMail;
   Object *STR_MailLogin;
   Object *STR_MailPassword;
   Object *STR_MailServer;

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

