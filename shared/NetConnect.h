#ifndef NETCONNECT_H_
#define NETCONNECT_H_

#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define to32(c) (((c)<<24)|((c)<<16)|((c)<<8)|(c))
#define MAXPATHLEN 256
#define CMDLINELEN 4096

#define PATH "NetConnect2:"
#define IMAGE_PATH PATH"Data/Images/"
#define PROGRAM_PATH PATH"Programs/"
#define DEFAULT_CONFIGFILE "ENV:NC2.prefs"

#ifdef DEMO
#define MAX_DAYS 100
#define WARN_DAYS 90
#endif

#define ID_NTCN   MAKE_ID('N','T','C','N')   /* `NetConnect' data chunk. */

#define ID_PRGM   MAKE_ID('P','R','G','M')   /* the program name */
#define ID_PRGF   MAKE_ID('P','R','G','F')   /* the program flags */
#define ID_PRGS   MAKE_ID('P','R','G','S')   /* the program stack */
#define ID_PRGP   MAKE_ID('P','R','G','P')   /* the program priority */
#define ID_PRGC   MAKE_ID('P','R','G','C')   /* the program current dir */
#define ID_PRGO   MAKE_ID('P','R','G','O')   /* the program output file */
#define ID_PRGR   MAKE_ID('P','R','G','R')   /* the program public screen */

#define ID_MENU   MAKE_ID('M','E','N','U')   /* the menu name */

#define ID_ICON   MAKE_ID('I','C','O','N')   /* the icon name */
#define ID_ICOH   MAKE_ID('I','C','O','H')   /* the icon hotkey */
#define ID_ICOI   MAKE_ID('I','C','O','I')   /* the icon image file */
#define ID_ICOS   MAKE_ID('I','C','O','S')   /* the icon sound file */
#define ID_ICOV   MAKE_ID('I','C','O','V')   /* the icon volume */
#define ID_ICOF   MAKE_ID('I','C','O','F')   /* the icon flags */

#define ID_DOCK   MAKE_ID('D','O','C','K')   /* the dock name */
#define ID_DOCH   MAKE_ID('D','O','C','H')   /* the dock hotkey */
#define ID_DOCO   MAKE_ID('D','O','C','O')   /* the dock font */
#define ID_DOCR   MAKE_ID('D','O','C','R')   /* the dock rows */
#define ID_DOCI   MAKE_ID('D','O','C','I')   /* the dock ID */
#define ID_DOCF   MAKE_ID('D','O','C','F')   /* the dock flags */

#define ID_END    MAKE_ID('E','N','D',' ')   /* the end of an entry*/

#define NUM_STOPS (sizeof(Stops) / (2 * sizeof(ULONG)))

STATIC LONG Stops[] =         /* IFF ID's for our config file  */
{
   ID_NTCN, ID_PRGM,
   ID_NTCN, ID_PRGF,
   ID_NTCN, ID_PRGS,
   ID_NTCN, ID_PRGP,
   ID_NTCN, ID_PRGC,
   ID_NTCN, ID_PRGO,
   ID_NTCN, ID_PRGR,

   ID_NTCN, ID_MENU,

   ID_NTCN, ID_ICON,
   ID_NTCN, ID_ICOH,
   ID_NTCN, ID_ICOI,
   ID_NTCN, ID_ICOS,
   ID_NTCN, ID_ICOV,
   ID_NTCN, ID_ICOF,

   ID_NTCN, ID_DOCK,
   ID_NTCN, ID_DOCH,
   ID_NTCN, ID_DOCR,
   ID_NTCN, ID_DOCI,
   ID_NTCN, ID_DOCF,
   ID_NTCN, ID_DOCO,

   ID_NTCN, ID_END
};

#define PRG_CLI         1
#define PRG_WORKBENCH   2
#define PRG_SCRIPT      4
#define PRG_AREXX       8
#define PRG_Arguments   16
#define PRG_ToFront     32
#define PRG_Asynch      64

struct Program
{
   STRPTR File;
   UWORD Flags;         // PRG_Arguments = pass wb-icons as argument    PRG_ToFront = Put pubscreen to front
   LONG Stack;
   BYTE Priority;
   STRPTR CurrentDir;
   STRPTR OutputFile;
   STRPTR PublicScreen;
};


#define IFL_DrawFrame   1

struct Icon
{
   STRPTR Name;
   STRPTR Hotkey;
   STRPTR ImageFile;
   STRPTR Sound;
   UBYTE Volume;
   UWORD Flags;
   struct Program Program;

   // only for internal use
   Object   *bodychunk;
   UBYTE    *body;
   ULONG    *cols;   // pointer to color array (MUST be ULONG !!)
   struct   DiskObject *disk_object;
   Object   *edit_window;
   Object   *list;
};


struct MenuEntry
{
   STRPTR Name;

   LONG Id;
   Object *LI_Programs;
   struct AppMenuItem *AppMenuItem;
};

#define DFL_Icon           1
#define DFL_Text           2
#define DFL_Activate       4
#define DFL_PopUp          8
#define DFL_Backdrop       16
#define DFL_Frontmost      32
#define DFL_Borderless     64
#define DFL_DragBar        128

struct Dock
{
   STRPTR Name;      // this one's size is fixed to 81 in ncprefs (otherwise problem with li_pager)
   STRPTR Hotkey;
   STRPTR Font;
   UWORD Rows;
   LONG WindowID;
   UWORD Flags;
   Object *LI_Buttons;
};

#endif
