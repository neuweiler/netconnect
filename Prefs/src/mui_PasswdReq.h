#define MUIM_PasswdReq_OldActive             (TAGBASE_PREFS | 0x10a0)
#define MUIM_PasswdReq_Pw2Active             (TAGBASE_PREFS | 0x10a1)
#define MUIA_PasswdReq_OldPassword           (TAGBASE_PREFS | 0x10a2)

struct PasswdReq_Data
{
   Object *originator;
   STRPTR old_password;
   int tries;

   Object *GR_old_pw;
   Object *STR_old_pw;
   Object *STR_pw1;
   Object *STR_pw2;
   Object *BT_Okay;
   Object *BT_Cancel;
};

