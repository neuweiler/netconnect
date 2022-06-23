#pragma libcall VUPBase VUP_BeginCheckUpdate 1e 81003
#pragma libcall VUPBase VUP_Quit 24 801
#pragma libcall VUPBase VUP_NewBeginCheckUpdate 2a 9281005
#pragma libcall VUPBase VUP_TestMOTD 30 801

APTR VUP_BeginCheckUpdate( ULONG prodcode, ULONG vercode, char *verstring );
APTR VUP_NewBeginCheckUpdate( ULONG prodcode, ULONG vercode, char *verstring, int force, struct Screen *scr );
VUP_Quit( APTR );

