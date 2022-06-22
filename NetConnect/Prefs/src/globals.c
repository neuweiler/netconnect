/// includes
#include "/includes.h"

#include "/NetConnect.h"
#include "/locale/Strings.h"
#include "mui.h"
#include "protos.h"

#define USE_DEFAULT_ICON_HEADER
#define USE_DEFAULT_ICON_BODY
#define USE_DEFAULT_ICON_COLORS
#include "/images/default_icon.h"

#define USE_LOGO_HEADER
#define USE_LOGO_BODY
#define USE_LOGO_COLORS
#include "/images/logo.h"

#define USE_INFORMATION_HEADER
#define USE_INFORMATION_BODY
#define USE_INFORMATION_COLORS
#include "/images/information.h"

#define USE_MENUS_HEADER
#define USE_MENUS_BODY
#define USE_MENUS_COLORS
#include "/images/menus.h"

#define USE_DOCK_HEADER
#define USE_DOCK_BODY
#define USE_DOCK_COLORS
#include "/images/dock.h"
///

/// Libraries
struct   Library     *MUIMasterBase = NULL;
struct   Library     *UtilityBase   = NULL;
struct   Library     *IFFParseBase  = NULL;
struct   Library     *DataTypesBase = NULL;
struct   Library     *IconBase      = NULL;

///

/// other data
struct   Catalog     *cat           = NULL;
struct StackSwapStruct StackSwapper;
struct Process       *proc;
Object *SoundObject  = NULL;
BOOL is_test = FALSE;
LONG left=-1, top=-1, width=-1, height=-1;

///
/// default programs
STRPTR default_names[] = {
   "Genesis",
   "Voyager",
   "Microdot-II",
   "AmFTP",
   "AmIRC",
   "AmTelnet",
   "AmTerm",
   "AmTalk",
   "NetInfo",
   "X-Arc",
   "CManager",
   "Docs",
   "Misc",
   NULL };
STRPTR default_imagefiles[] = {
   IMAGE_PATH"Start",
   IMAGE_PATH"WWW2",
   IMAGE_PATH"Mail",
   IMAGE_PATH"FTP",
   IMAGE_PATH"IRC",
   IMAGE_PATH"Telnet",
   IMAGE_PATH"Colour/Comms",
   IMAGE_PATH"Talk",
   IMAGE_PATH"Colour/Search",
   IMAGE_PATH"Packers/Lha",
   IMAGE_PATH"General/Clipboard",
   IMAGE_PATH"Docs",
   IMAGE_PATH"Misc",
   NULL };
STRPTR default_programfiles[] = {
   "AmiTCP:Genesis",
   PROGRAM_PATH"Voyager/V",
   PROGRAM_PATH"Microdot-II/Microdot",
   PROGRAM_PATH"AmFTP/AmFTP",
   PROGRAM_PATH"AmIrc/AmIRC",
   PROGRAM_PATH"AmTelnet/AmTelnet",
   PROGRAM_PATH"AmTerm/AmTerm",
   PROGRAM_PATH"AmTalk/AmTalk",
   PROGRAM_PATH"NetInfo/NetInfo",
   PROGRAM_PATH"X-Arc/X-Arc",
   PROGRAM_PATH"CManager/CManager",
   "NetConnect2:Docs/Documentation",
   PROGRAM_PATH"misc",
   NULL };

STRPTR default_menus[] = {
   "MultiView",
   "LZX extract",
   "Play Sound",
   "NewShell",
   NULL };
STRPTR default_menu_programs[]= {
   "SYS:Utilities/MultiView",
   "lzx x [] ram: <>CON:0/0/640/200/LZXoutput/AUTO/CLOSE/WAIT",
   "PlaySound",
   "NewShell",
   NULL };

///

/// MainWindowMenu
struct NewMenu MainWindowMenu[] =
{
   { NM_TITLE, (STRPTR)MSG_MENU_PROJECT         , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)MSG_MENU_ABOUT           ,"?", 0, 0, (APTR)MEN_ABOUT   },
   { NM_ITEM , (STRPTR)MSG_MENU_HELP            ,"H", 0, 0, (APTR)MEN_HELP},
   { NM_ITEM , (STRPTR)NM_BARLABEL              , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)MSG_MENU_QUIT            ,"Q", 0, 0, (APTR)MEN_QUIT    },

   { NM_TITLE, (STRPTR)MSG_MENU_SETTINGS        , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)MSG_MENU_LOAD            ,"L", 0, 0, (APTR)MEN_LOAD    },
   { NM_ITEM , (STRPTR)MSG_MENU_SAVE_AS         ,"S", 0, 0, (APTR)MEN_SAVE_AS },
   { NM_ITEM , (STRPTR)MSG_MENU_RESET_DEFAULT   ,"D", 0, 0, (APTR)MEN_RESET   },
   { NM_ITEM , (STRPTR)NM_BARLABEL              , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)MSG_MENU_MUI             ,"M", 0, 0, (APTR)MEN_MUI     },

   { NM_END  , NULL                             , 0 , 0, 0, (APTR)0           }

};

///

/// hooks
extern SAVEDS ASM VOID DestructFunc(REG(a2) APTR pool, REG(a1) APTR ptr);
struct Hook DestructHook  = { { 0,0 }, (VOID *)DestructFunc   , NULL, NULL };

extern SAVEDS ASM LONG AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x);
struct Hook AppMsgHook = { {NULL, NULL}, (VOID *)AppMsgFunc, NULL, NULL };

extern SAVEDS ASM LONG Editor_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x);
struct Hook Editor_AppMsgHook = { {NULL, NULL}, (VOID *)Editor_AppMsgFunc, NULL, NULL };
///

/// MUI Custom Classes
struct MUI_CustomClass  *CL_MainWindow    = NULL;
struct MUI_CustomClass  *CL_DockPrefs     = NULL;
struct MUI_CustomClass  *CL_Editor        = NULL;
struct MUI_CustomClass  *CL_EditIcon      = NULL;
struct MUI_CustomClass  *CL_IconList      = NULL;
struct MUI_CustomClass  *CL_About         = NULL;
struct MUI_CustomClass  *CL_MenuPrefs     = NULL;
struct MUI_CustomClass  *CL_ProgramList   = NULL;
struct MUI_CustomClass  *CL_PagerList  = NULL;

///
/// MUI stuff
Object *app = NULL;
Object *win = NULL;
Object *group  = NULL;

///

