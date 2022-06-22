#define MUISERIALNR_WIZARD 1
#define TAGBASE_WIZARD (TAG_USER | (MUISERIALNR_WIZARD << 16))

#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)

enum { MEN_ABOUT=1, MEN_ABOUT_MUI, MEN_QUIT, MEN_MUI };

#define MUIM_Genesis_Handshake                 (TAGBASE_WIZARD | 0x1000)
#define MUIM_Genesis_Get                       (TAGBASE_WIZARD | 0x1001)

struct MainMessage
{
   struct Message msg;

   Object *obj;
   LONG MethodID;
   APTR data1;
   APTR data2;
   APTR data3;
   ULONG result;
};

/*
Genesis     : 0
Mainwindow  : 1
Welcome     : 2
SerialSana  : 3
SerialModem : 4
ModemStrings: 5
UserInfo    : 6
ISPInfo     : 7
LoginScript : 8
Finished    : 9
Sana2       : a
ModemDetect : b

*/

