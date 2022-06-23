#ifndef RPC_SVC_AUTH_H
#define RPC_SVC_AUTH_H
/*
 * $Id: svc_auth.h,v 1.1.1.1 1999/08/08 14:20:31 zapek Exp $
 *
 * Service side of rpc authentication.
 * 
 * Copyright © 1994 AmiTCP/IP Group,
 *                  Network Solutions Development Inc.
 *                  All rights reserved.
 *
 */
/* @(#)svc_auth.h	2.1 88/07/29 4.0 RPCSRC */
/*      @(#)svc_auth.h 1.6 86/07/16 SMI      */

/*
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

/*
 * Server side authenticator
 */
extern enum auth_stat _authenticate(struct svc_req * rqst,
				    struct rpc_msg * msg);

#endif /* !RPC_SVC_AUTH_H */
