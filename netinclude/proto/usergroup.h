#ifndef PROTO_USERGROUP_H
#define PROTO_USERGROUP_H \
       "$Id: usergroup.h,v 1.1.1.1 1999/08/08 14:20:32 zapek Exp $"
/*
 *	SAS C prototypes for usergroup.library
 *
 *      Copyright © 1994 AmiTCP/IP Group,
 *                       Network Solutions Development, Inc.
 *                       All rights reserved.
 */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef __NOLIBBASE__       
extern struct Library *UserGroupBase;
#endif

#include <libraries/usergroup.h>
#include <clib/usergroup_protos.h>
#if __SASC
#include <pragmas/usergroup_pragmas.h>
#elif __GNUC__
#include <inline/usergroup.h>
#endif

#endif /* PROTO_USERGROUP_H */
