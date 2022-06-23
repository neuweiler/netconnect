#ifndef USERPREFS_H
#define USERPREFS_H

/*********************************************************
**                                                      **
**      UserPrefs.mcc                                   **
**                                                      **
**              ©1998 Simone Tellini                    **
**                                                      **
*********************************************************/


/// Support stuff
#ifndef REG
#ifdef _DCC
#define REG(x) __ ## x
#else
#define REG(x) register __ ## x
#endif
#endif

#ifndef ASM
#if defined __MAXON__ || defined __STORM__ || defined _DCC
#define ASM
#else
#define ASM __asm
#endif
#endif

#ifndef SAVEDS
#ifdef __MAXON__
#define SAVEDS
#endif
#if defined __STORM__ || defined __SASC
#define SAVEDS __saveds
#endif
#if defined _GCC || defined _DCC
#define SAVEDS __geta4
#endif
#endif


#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d)    ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

#ifndef MIN
#define MIN(a,b)    (( a < b ) ? ( a ) : ( b ))
#endif

#ifndef MAX
#define MAX(a,b)    (( a > b ) ? ( a ) : ( b ))
#endif

#ifndef ABS
#define ABS(a) (((a) > 0) ? (a) : -(a))
#endif
///


#define MUIC_UserPrefs  "AmiTCP:MUI/UserPrefs.mcc"
#define UserPrefsObject MUI_NewObject( MUIC_UserPrefs


/*************************
**      Methods         **
*************************/

#define MUIM_UserPrefs_Load         0xF76B0001
#define MUIM_UserPrefs_Save         0xF76B0002
#define MUIM_UserPrefs_Edit         0xF76B0003  /*  PRIVATE     */
#define MUIM_UserPrefs_New          0xF76B0004  /*  PRIVATE     */
#define MUIM_UserPrefs_DisposeObj   0xF76B0005  /*  PRIVATE     */


/*************************
**      Attributes      **
*************************/

#define MUIA_UserPrefs_Changed      0xF76B0001  /*  ..g  BOOL                           */


/*************************
**      Structures      **
*************************/

struct MUIP_UserPrefs_DisposeObj { ULONG MethodID; Object *Object; };    /*  PRIVATE     */


#endif /* USERPREFS_H */
