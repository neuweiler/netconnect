#define MUIM_Dock_DockPrefs            (TAGBASE_NETCONNECT | 0x1010)
#define MUIM_Dock_GenesisPrefs         (TAGBASE_NETCONNECT | 0x1011)
#define MUIM_Dock_Help                 (TAGBASE_NETCONNECT | 0x1012)

struct Dock_Data
{
   struct Dock dock;
   Object *MN_Strip;

   CxObj *CX_Filter;
   struct TextAttr TxtAttr;
   struct TextFont *TxtFont;
};


