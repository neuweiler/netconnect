#ifndef MUI_H_
#define MUI_H_

#define MUISERIALNR_NETCONNECT 1
#define TAGBASE_NETCONNECT (TAG_USER | (MUISERIALNR_NETCONNECT << 16))

enum {ID_REFRESH = 10, ID_ABOUT, ID_ABOUTFINISH, ID_DOUBLESTART , ID_CHECK_OPEN };
enum { MEN_ABOUT = 1, MEN_QUIT, MEN_CONTROLLER, MEN_GENESIS, MEN_MUI, MEN_HELP };

#define MUIA_NetConnect_Icon           (TAGBASE_NETCONNECT | 0x1000)
#define MUIA_NetConnect_Dock           (TAGBASE_NETCONNECT | 0x1001)
#define MUIA_NetConnect_Originator     (TAGBASE_NETCONNECT | 0x1002)

#define MUIM_Hotkey_Trigger            (TAGBASE_NETCONNECT | 0x1020)

#endif
