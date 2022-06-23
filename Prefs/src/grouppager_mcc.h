/*
** GroupPager.mcc
** -------------
**
** A generic group pager
**
** (C) 1997 Michael Neuweiler <dolphin@zool.unizh.ch>
** All Rights Reserved
**
** Many thanks to Oliver Wagner
**
*/

#ifndef GROUPPAGER_MCC_H
#define GROUPPAGER_MCC_H

#ifndef LIBRARIES_MUI_H
#include "libraries/mui.h"
#endif

/*
** Class name, object macros
*/

#define MUIC_Grouppager "Grouppager.mcc"
#define GrouppagerObject MUI_NewObject(MUIC_Grouppager

#define MCC_GP_TAGBASE 0x7d990000
#define MCC_GP_ID(x) (MCC_GP_TAGBASE + x)

/*
** Methods
*/

#define MUIM_Grouppager_PagerActive      MCC_GP_ID(1)      /* V1 (private) */
#define MUIM_Grouppager_InsertGroup      MCC_GP_ID(2)      /* V1 */
#define MUIM_Grouppager_InsertImageGroup MCC_GP_ID(3)      /* V1 */
#define MUIM_Grouppager_RemoveGroup      MCC_GP_ID(4)      /* V1 */

/*
** Messages
*/

//struct MUIP_Grouppager_PagerActive      { ULONG MethodID; };
struct MUIP_Grouppager_InsertGroup      { ULONG MethodID; STRPTR name; APTR group; APTR parent; LONG flags; };
struct MUIP_Grouppager_InsertImageGroup { ULONG MethodID; STRPTR name; APTR group; APTR parent; LONG flags; APTR bitmap_obj; };
struct MUIP_Grouppager_RemoveGroup      { ULONG MethodID; APTR group; };

/*
** Special Values
*/

/* MUIP_Grouppager_InsertGroup.flags */
#define GPE_LIST 1         /* create entry with list that can contain sub entries */
#define GPE_OPEN 2         /* open the list by default */
#define GPE_IMAGE_LATE 4   /* If an entry is added with MUIM_Grouppager_InsertImageGroup
                              and the window the GrouppagerObject belongs to is already open,
                              this flag has to be set. Otherwise the image won't be drawn. */

#define MUIV_Grouppager_Type_List             0
#define MUIV_Grouppager_Type_Listtree         1
#define MUIV_Grouppager_Type_NList            2
#define MUIV_Grouppager_Type_NListtree        3
#define MUIV_Grouppager_Active_Off            0
#define MUIV_Grouppager_DestructHook_Dispose -1

/*
** Attributes
*/

#define MUIA_Grouppager_DefaultGroup      MCC_GP_ID(20)  /* V1 i.g APTR */
#define MUIA_Grouppager_DestructHook      MCC_GP_ID(21)  /* V1 i.. struct Hook * */
#define MUIA_Grouppager_SwitchHook        MCC_GP_ID(22)  /* V1 i.. struct Hook * */
#define MUIA_Grouppager_Active            MCC_GP_ID(23)  /* V1 .sg LONG */
#define MUIA_Grouppager_ActiveGroup       MCC_GP_ID(24)  /* V1 .sg APTR */
#define MUIA_Grouppager_Quiet             MCC_GP_ID(25)  /* V1 isg BOOL */
#define MUIA_Grouppager_Type              MCC_GP_ID(26)  /* V1 i.. BYTE */
#define MUIA_Grouppager_BalanceObject     MCC_GP_ID(27)  /* V1 i.. BOOL */
#define MUIA_Grouppager_ListMinLineHeight MCC_GP_ID(28)  /* V1 i.. LONG */
#define MUIA_Grouppager_ListAdjustWidth   MCC_GP_ID(29)  /* V1 i.. BOOL */
#define MUIA_Grouppager_ListHorizWeight   MCC_GP_ID(30)  /* V1 isg WORD */

#endif

