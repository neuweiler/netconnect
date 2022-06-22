#define MUISERIALNR_PREFS 1
#define TAGBASE_PREFS (TAG_USER | (MUISERIALNR_PREFS << 16))

#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)

enum { MEN_ABOUT = 1, MEN_ABOUT_MUI, MEN_ICONIFY, MEN_QUIT, MEN_LOAD, MEN_SAVE, MEN_SAVEAS, MEN_MUI };

#define MUIA_Genesis_Originator                  (TAGBASE_PREFS | 0x1060)
#define MUIM_Genesis_Finish                      (TAGBASE_PREFS | 0x1061)

struct MUIP_Genesis_Finish              { ULONG MethodID; Object *obj; LONG status; };

