#ifndef GENESIS_H
#define GENESIS_H

#include <exec/libraries.h>

#define GENESISNAME "AmiTCP:libs/genesis.library"

struct GenesisBase
{
   struct Library         LibNode;
   long                   gb_CurrentUser;       /* read only */
   struct SignalSemaphore gb_UserListSemaphore; /* private */
   struct MinList         gb_UserList;          /* private */
};

struct UserData
{
   char ud_Name[41];          /* local login name, ENV:LOGNAME will contain this */
   char ud_RealName[41];      /* the real name of the above user */
   char ud_EMail[41];         /* the user's email address */
   char ud_MailLogin[41];     /* login for the user's mail account */
   char ud_MailPassword[41];  /* password for the mail account */
   char ud_MailServer[41];    /* if the user needs a different mailserver, it's in here - otherwise it's empty */
};

struct pc_Data
{
   STRPTR Buffer;    /* buffer holding the file (internal use only) */
   LONG Size;        /* variable holding the size of the buffer (internal use only) */
   STRPTR Current;   /* pointer to the current position (internal use only) */

   STRPTR Argument;  /* pointer to the argument name */
   STRPTR Contents;  /* pointer to the argument's contents */
};


#endif
