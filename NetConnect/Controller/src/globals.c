#include "NetConnect.h"
#include "NetConnectStrings.h"

#define USE_DEFAULT_ICON_HEADER
#define USE_DEFAULT_ICON_BODY
#define USE_DEFAULT_ICON_COLORS
#include "default_icon.bh"

#define USE_LOGO_HEADER
#define USE_LOGO_BODY
#define USE_LOGO_COLORS
#include "logo.bh"

struct	ExecBase					*SysBase			= NULL;
struct	DosLibrary				*DOSBase			= NULL;
struct	Library					*IntuitionBase	= NULL;
struct	Library					*MUIMasterBase	= NULL;
struct	Library					*UtilityBase	= NULL;
struct	Library					*LocaleBase		= NULL;
struct	Library					*IFFParseBase	= NULL; /* this one's for the config-file */
struct	Library					*DataTypesBase	= NULL;
struct	Library					*CxBase			= NULL;
struct	Library					*WorkbenchBase	= NULL;
struct	Library					*IconBase		= NULL;
struct	Catalog					*cat				= NULL; /* pointer to our locale catalog */

STATIC LONG Stops[] =			/* IFF ID's for our config file	*/
{
	ID_NTCN, ID_AICN,				/* Active ICoN		: Holds a "struct Icon", an icon for the Icon Bar		*/
	ID_NTCN, ID_IICN,				/* Inactive ICoN	: Holds a "struct Icon", an icon fot the Icon Bank	*/
	ID_NTCN, ID_COLS,				/* howmany columns will be used to display the icons */
	ID_NTCN, ID_WINT,				/* type of window: 0=normal, 1=borderless, 2=borderless with dragbar */
	ID_NTCN, ID_BTTY,				/* how buttons are displayed: 0=text&icon, 1=icon only, 2=text only */

	ID_NTCN, ID_MENU,
	ID_NTCN, ID_CMND
};

Object *app						= NULL;	/* our MUI application												*/
Object *win						= NULL;	/* a global pointer to our main window (the icon bar) 	*/
Object *menu_win				= NULL;	/* pointer to MenuPrefs window */

struct MUI_CustomClass  *CL_IconBar			= NULL;	/* class for the main (icon bar) window									*/
struct MUI_CustomClass  *CL_IconBarPrefs	= NULL;	/* icon bar preferences window class										*/
struct MUI_CustomClass  *CL_Editor			= NULL;	/* list class																		*/
struct MUI_CustomClass  *CL_EditIcon		= NULL;	/* window class																	*/
struct MUI_CustomClass  *CL_IconList		= NULL;	/* list class for the two listviews in the icon bar prefs			*/
struct MUI_CustomClass	*CL_Button			= NULL;
struct MUI_CustomClass	*CL_About			= NULL;
struct MUI_CustomClass	*CL_MenuPrefs		= NULL;
struct MUI_CustomClass	*CL_ProgramList	= NULL;

Object *SoundObject = NULL;
STRPTR ARR_Asynch[] = { "synch", "asynch", NULL };
STRPTR ARR_ProgramTypes[] = { "CLI", "Workbench", "Script", "ARexx Script", NULL };
STRPTR ARR_ButtonTypes[] = { "Icon & Text", "Icon only", "Text only", NULL };
STRPTR ARR_WindowTypes[] = { "Normal", "Borderless", "Borderless with DragBar", NULL };
STRPTR STR_GR_Register[4];
struct MsgPort						*appmenu_port;
struct MUI_InputHandlerNode	ihnode;
struct Process						*ThisProcess;
struct WBStartup					*WBenchMsg;
BPTR									WBenchLock;
struct CommandLineInterface	*LocalCLI = NULL;
STATIC BPTR							OldCLI = NULL;


STRPTR default_names[]			= { "Start/Stop"				, "WWW"						, "Telnet"					, "FTP"					, "IRC"					, "Mail"										, "News"												, "Ping"												, "Traceroute"												, "Finger"												, "Search"												, "Docs"												, "Misc"												, NULL };
STRPTR default_imagefiles[]	= { IMAGE_PATH"Start"		, IMAGE_PATH"WWW"			, IMAGE_PATH"Telnet"		, IMAGE_PATH"FTP"		, IMAGE_PATH"IRC"		, IMAGE_PATH"Mail"		, IMAGE_PATH"News"		, IMAGE_PATH"Ping"		, IMAGE_PATH"Traceroute"		, IMAGE_PATH"Finger"		, IMAGE_PATH"Search"		, IMAGE_PATH"Docs"		, IMAGE_PATH"Misc"		, NULL };
STRPTR default_programfiles[]	= { "AmiTCP:bin/startnet"	, PROGRAM_PATH"Voyager"	, PROGRAM_PATH"Telnet"	, PROGRAM_PATH"mFTP"	, PROGRAM_PATH"Irc"	, PROGRAM_PATH"Mail", PROGRAM_PATH"News", PROGRAM_PATH"Ping", PROGRAM_PATH"Traceroute", PROGRAM_PATH"Finger", PROGRAM_PATH"Search", PROGRAM_PATH"Docs", PROGRAM_PATH"Misc", NULL };


enum { MEN_RESET = 1 , MEN_MUI2 };
struct NewMenu IconBarPrefsMenu[] =
{
	{ NM_TITLE, (STRPTR)MSG_MENU_SETTINGS			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_RESET_DEFAULT	,"D", 0, 0, (APTR)MEN_RESET	},
	{ NM_ITEM , (STRPTR)NM_BARLABEL					, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_MUI					,"M", 0, 0, (APTR)MEN_MUI2		},

	{ NM_END  , NULL								, 0 , 0, 0, (APTR)0				},

};

enum { MEN_ABOUT = 1, MEN_QUIT, MEN_ICONBAR, MEN_MENUS, MEN_AMITCP, MEN_TIMELOG, MEN_NETWATCH, MEN_MUI };
struct NewMenu IconBarMenu[] =
{
	{ NM_TITLE, (STRPTR)MSG_MENU_PROJECT	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_ABOUT		,"?", 0, 0, (APTR)MEN_ABOUT	},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_QUIT		,"Q", 0, 0, (APTR)MEN_QUIT		},

	{ NM_TITLE, (STRPTR)MSG_MENU_SETTINGS	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_ICONBAR	,"I", 0, 0, (APTR)MEN_ICONBAR	},
	{ NM_ITEM , (STRPTR)MSG_MENU_MENUS		,"M", 0, 0, (APTR)MEN_MENUS	},
	{ NM_ITEM , (STRPTR)MSG_MENU_AMITCP		,"A", 0, 0, (APTR)MEN_AMITCP	},
	{ NM_ITEM , (STRPTR)MSG_MENU_MODULES	, 0 , 0, 0, (APTR)0				},
	{ NM_SUB  , (STRPTR)MSG_MENU_TIMELOG	,"T", 0, 0, (APTR)MEN_TIMELOG	},
	{ NM_SUB  , (STRPTR)MSG_MENU_NETWATCH	,"N", 0, 0, (APTR)MEN_NETWATCH},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_MUI			,"U", 0, 0, (APTR)MEN_MUI		},

	{ NM_END  , NULL								, 0 , 0, 0, (APTR)0				},

};

SAVEDS ASM int BrokerFunc(REG(a1) CxMsg *msg);
struct Hook BrokerHook = { { 0,0 }, (VOID *)BrokerFunc, NULL, NULL };
