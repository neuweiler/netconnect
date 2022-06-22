#define MUIM_Online_GoOnline                    (TAGBASE_WIZARD | 0x10d0)
#define MUIM_Online_Abort                       (TAGBASE_WIZARD | 0x10d1)

struct Online_Data
{
   struct Process *TCPHandler;
   int abort;

   Object *TX_Info;
   Object *BU_Busy;
   Object *BT_Abort;
};

