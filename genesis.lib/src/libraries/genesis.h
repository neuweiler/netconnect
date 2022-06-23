#ifndef GENESIS_H
#define GENESIS_H

#include <exec/libraries.h>

#define GENESISNAME "AmiTCP:libs/genesis.library"

#define GUF_TextObject  (1 << 0)  /* use a TextObject for loginname instead of StringObject *
                                   * => user can't change it                                */

#define IOC_AskUser   1
#define IOC_Force     2

struct RestrictedTime
{
   struct MinNode rt_node;
   UBYTE  rt_day;   /* weekday */
   ULONG  rt_start; /* in minutes from midnight */
   ULONG  rt_end;
};

#define USRF_TIMESERVER         (1 << 0)  /* use the time server */
#define USRF_WARNUSER           (1 << 1)  /* warn the user before disconnecting for time limit */

struct User
{
   UBYTE *us_name;      /* Username */
   UBYTE *us_passwd;    /* Encrypted password */
   LONG   us_uid;       /* User ID */
   LONG   us_gid;       /* Group ID */
   UBYTE *us_gecos;     /* Real name etc */
   UBYTE *us_dir;       /* Home directory */
   UBYTE *us_shell;     /* Shell */

   ULONG us_flags;      /* user flags */
   ULONG us_max_time;   /* max. minutes user can stay online */
   UBYTE *us_timeserver;/* name of timeserver */
   struct MinList us_restricted_times; /* list containing RestrictedTimes */
};

struct ParseConfig_Data
{
   STRPTR pc_buffer;    /* buffer holding the file (internal use only) */
   LONG   pc_size;      /* holding the size of the buffer (internal use only) */
   STRPTR pc_current;   /* pointer to the current position (internal use only) */

   STRPTR pc_argument;  /* pointer to the argument name */
   STRPTR pc_contents;  /* pointer to the argument's contents */
};


#endif
