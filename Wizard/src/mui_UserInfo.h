#define MUIM_UserInfo_AddPhone              (TAGBASE_WIZARD | 0x1060)

struct MUIP_UserInfo_AddPhone  { ULONG MethodID; LONG doit; };

struct UserInfo_Data
{
   Object *STR_LoginName;
   Object *STR_Password;
   Object *PO_PhoneNumber;
   Object *STR_PhoneNumber;
   Object *STR_AddPhone;
   Object *BT_AddPhone;
   Object *BT_CancelPhone;
};
