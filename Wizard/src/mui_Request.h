#define MUIM_Request_Finish           (TAGBASE_WIZARD | 0x10b0)
#define MUIA_Request_Process          (TAGBASE_WIZARD | 0x10b1)
#define MUIA_Request_Buffer           (TAGBASE_WIZARD | 0x10b2)
#define MUIA_Request_Text             (TAGBASE_WIZARD | 0x10b3)

struct MUIP_Request_Finish            { ULONG MethodID; LONG ok; };

struct Request_Data
{
   struct Process *proc;
   STRPTR buffer;

   Object *STR_Input;
   Object *BT_Okay;
   Object *BT_Cancel;
};

