#define MUIM_Online_GoOnline                    (TAGBASE_WIZARD | 0x1050)
#define MUIM_Online_Abort                       (TAGBASE_WIZARD | 0x1051)

struct Online_Data
{
   struct Process *TCPHandler;
   int abort;

   Object *TX_Info;
   Object *BU_Busy;
   Object *BT_Abort;
};

