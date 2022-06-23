#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <amitcp/socketbasetags.h>
#include <pragmas/socket_pragmas.h>
#include <pragmas/ifconfig_pragmas.h>
#include <pragmas/usergroup_pragmas.h>
#include <libraries/ifconfig.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/rexxsyslib.h>
#include <proto/utility.h>
#include <proto/muimaster.h>

#include <clib/macros.h>
#include <clib/alib_protos.h>
#include <clib/battclock_protos.h>

#include <libraries/iffparse.h>
#include <libraries/gadtools.h>
#include <libraries/mui.h>
#include <libraries/asl.h>

#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/devices.h>

#include <dos/dos.h>
#include <dos/dostags.h>

#include <intuition/icclass.h>

#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>

#include <workbench/workbench.h>
#include <resources/battclock.h>

#include <prefs/locale.h>
#include <prefs/prefhdr.h>

#include <rexx/storage.h>
#include <rexx/rxslib.h>

#include <mui/Listtree_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/NFloattext_mcc.h>
#include <mui/Busy_mcc.h>
#include <mui/Term_mcc.h>
#include <mui/textinput_mcc.h>

