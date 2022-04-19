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

struct	Library					*IntuitionBase	= NULL;
struct	Library					*MUIMasterBase	= NULL;
struct	Library					*UtilityBase	= NULL;
struct	Library					*LocaleBase		= NULL;
struct	Library					*IFFParseBase	= NULL; /* this one's for the config-file */
struct	Library					*DataTypesBase	= NULL;
struct	Catalog					*cat				= NULL; /* pointer to our locale catalog */

STATIC LONG Stops[] =			/* IFF ID's for our config file	*/
{
	ID_NTCN, ID_AICN,				/* Active ICoN		: Holds a "struct Icon", an icon for the Icon Bar		*/
	ID_NTCN, ID_IICN,				/* Inactive ICoN	: Holds a "struct Icon", an icon fot the Icon Bank	*/
	ID_NTCN, ID_ROWS
};

Object *app						= NULL;	/* our MUI application												*/
Object *win						= NULL;	/* a global pointer to our main window (the icon bar) 	*/

struct MUI_CustomClass  *CL_IconBar;			/* class for the main (icon bar) window									*/
struct MUI_CustomClass  *CL_IconBarPrefs;		/* icon bar preferences window class										*/
struct MUI_CustomClass  *CL_IconList;			/* list class for the two listviews in the icon bar prefs			*/
struct MUI_CustomClass	*CL_Button;
struct MUI_CustomClass	*CL_About;

LONG Rows = 1;
Object *SoundObject = NULL;

enum { MEN_RESET = 1 };
struct NewMenu IconBarPrefsMenu[] =
{
	{ NM_TITLE, (STRPTR)MSG_MENU_SETTINGS			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_RESET_DEFAULT	,"D", 0, 0, (APTR)MEN_RESET	},

	{ NM_END  , NULL								, 0 , 0, 0, (APTR)0				},

};

enum { MEN_ABOUT = 1, MEN_QUIT, MEN_ICONBAR, MEN_AMITCP, MEN_TIME, MEN_MUI };
struct NewMenu IconBarMenu[] =
{
	{ NM_TITLE, (STRPTR)MSG_MENU_PROJECT	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_ABOUT		,"?", 0, 0, (APTR)MEN_ABOUT	},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_QUIT		,"Q", 0, 0, (APTR)MEN_QUIT		},

	{ NM_TITLE, (STRPTR)MSG_MENU_SETTINGS	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_ICONBAR	,"I", 0, 0, (APTR)MEN_ICONBAR	},
	{ NM_ITEM , (STRPTR)MSG_MENU_AMITCP		,"A", 0, 0, (APTR)MEN_AMITCP	},
	{ NM_ITEM , (STRPTR)MSG_MENU_TIME		,"T", 0, 0, (APTR)MEN_TIME		},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_MUI			,"M", 0, 0, (APTR)MEN_MUI		},

	{ NM_END  , NULL								, 0 , 0, 0, (APTR)0				},

};
