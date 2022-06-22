#define MUIM_ModemDetect_FindModem              (TAGBASE_WIZARD | 0x10b0)


struct ModemDetect_Data
{
   struct Process *Handler;
   struct MsgPort *time_port;
   struct timerequest *time_req;
   BOOL abort;

   Object *TX_Info;
   Object *BU_Busy;
   Object *BT_Abort;
};

