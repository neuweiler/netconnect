#define MUIM_Online_GoOnline                    (TAGBASE_GENESIS | 0x1010)
#define MUIM_Online_Abort                       (TAGBASE_GENESIS | 0x1011)

struct Online_Data
{
   struct Process *TCPHandler;
   BOOL abort;

   Object *TX_Info;
   Object *BU_Busy;
   Object *GR_Terminal;
   Object *TR_Terminal;
   Object *SB_Terminal;
   Object *BT_Abort;
};

