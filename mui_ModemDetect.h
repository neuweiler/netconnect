#define MUISERIALNR_DETECT 1
#define TAGBASE_DETECT (TAG_USER | (MUISERIALNR_DETECT << 16))

SAVEDS ASM ULONG ModemDetect_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

/// MUIM
#define MUIM_ModemDetect_Trigger                (TAGBASE_DETECT | 0x1110)
#define MUIM_ModemDetect_FindModem              (TAGBASE_DETECT | 0x1111)
#define MUIM_ModemDetect_CheckATI               (TAGBASE_DETECT | 0x1112)
#define MUIM_ModemDetect_Abort                  (TAGBASE_DETECT | 0x1113)
#define MUIM_ModemDetect_GetModem               (TAGBASE_DETECT | 0x1114)

///
/// MUIP
struct MUIP_ModemDetect_GetModem          { ULONG MEthodID; STRPTR modemname; };

///
/// Data
struct ModemDetect_Data
{
   struct MUI_InputHandlerNode ihnode;
   int ihnode_added;

   char buffer[81];  // for serial input
   int buf_pos;

   struct MsgPort *time_port;
   struct timerequest *time_req;

   int action;    // what we are doing at the moment

   STRPTR *devices;
   int device_nr;
   int unit;

   char ATI[10][81];
   int ATI_nr;

   Object *TX_Info;
   Object *BU_Busy;
   Object *BT_Abort;
};

///
