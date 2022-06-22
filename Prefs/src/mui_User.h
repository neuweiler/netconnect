#define MUIM_User_InsertFile                    (TAGBASE_PREFS | 0x1040)

struct MUIP_User_InsertFile            { ULONG MethodID; LONG stopnet; };

struct User_Data
{
   Object *GR_User;
   Object *STR_LoginName;
   Object *STR_RealName;
   Object *STR_Password;
   Object *STR_EMail;
   Object *STR_Organisation;
};

