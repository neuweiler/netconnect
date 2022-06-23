/*
**	$Filename: pragmas/ifconfig_pragmas.h $
**	$Release$
**	$Revision: 1.1.1.1 $
**	$Date: 1999/08/08 14:20:33 $
**
**	Pragmas file for ifconfig.library
**
**	Copyright © 1995 AmiTCP/IP Group, 
**	                 Network Solutions Development Inc.
**	                 All rights reserved.
*/
/* ifconfig.library functions */
#pragma libcall IfConfigBase Reserved 1E 00
#pragma libcall IfConfigBase IfConfigFind 24 9802
#pragma libcall IfConfigBase IfConfigFree 2A 801
#pragma libcall IfConfigBase IfConfigFindAll 30 9802
#pragma libcall IfConfigBase IfConfigFreeList 36 801
#pragma libcall IfConfigBase IfConfigStrError 3C 18003
#pragma libcall IfConfigBase IfConfigGetTagInfo 42 001
