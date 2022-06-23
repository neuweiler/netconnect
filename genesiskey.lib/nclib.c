#define __USE_SYSBASE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <libraries/mui.h>
#include <dos/dostags.h>
#include <dos.h>
#include <exec/execbase.h>

#include <mui/textinput_mcc.h>

#include <string.h>
#include <stdlib.h>

#include "idea68k.h"
#include "rev.h"

struct IntuitionBase *IntuitionBase;
struct DosLibrary *DOSBase;
struct ExecBase *SysBase;
struct Library *UtilityBase;

#define MAKE_ID(a,b,c,d)	\
	((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))

#define AMITCP_NEW_NAMES
#define CLIB_USERGROUP_PROTOS_H // to avoid name clashes
#include <bsdsocket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <amitcp/socketbasetags.h>
#include <sys/ioctl.h>

#define KEYNAMECODE 0xfe
#define CH(x) x^KEYNAMECODE

//#define NCLITE

static char nckeyname[] = { 
	CH('S'),
	CH(':'),
	CH('G'),
	CH('e'),
	CH('n'),
	CH('e'),
	CH('s'),
	CH('i'),
	CH('s'),
	CH('.'),
	CH('K'),
	CH('E'),
	CH('Y'),
	0
};

//
// Information about NC
//
struct ncinfo {
	char owner[ 256 ];
	char street[ 256 ];
	char city_zip[ 256 ];
	char country[ 256 ];

	char area[ 256 ];
	char email[ 256 ];
	char bought[ 256 ];
	char serial[ 256 ];

	char comment[ 512 ];

	unsigned long magic;
	time_t wasregged;

} ncinfo;

#define MAGIC 0xac7daffe

static BPTR keylock;

static UWORD mykey[] = {
	0x7133, 0x4144, 0x135f, 0x1722, 0x9491, 0xfc00, 0xab44, 0x39fa
};

static UWORD fmtfunc[] = { 0x16c0, 0x4e75 };

#ifndef NCLITE

struct Library *MUIMasterBase;

static UBYTE sched[ IDEA_SIZE_SCHEDULE ], ivec[ IDEA_SIZE_IVEC ];

static void writencinfo( void )
{
	struct ncinfo ncinfo2;
	BPTR f;

	memset( ivec, 0, sizeof( ivec ) );
	idea_key_schedule( mykey, sched );
	idea_cbc_encrypt( &ncinfo, &ncinfo2, sizeof( ncinfo ), sched, ivec, IDEA_MODE_ENCRYPT );

	f = Open( nckeyname, MODE_NEWFILE );
	if( f )
	{
		Write( f, &ncinfo2, sizeof( ncinfo2 ) );
		Close( f );
	}
}

static APTR win, app, grp_page;
static APTR str_name, str_street, str_city, str_country, str_email, str_bought, str_serial, str_zip, bt_noreg, str_comment, str_area;
static APTR txt_status, txt_status2, cyc_reg;

static APTR stro( void )
{
	APTR o = TextinputObject, StringFrame,
		MUIA_String_AdvanceOnCR, TRUE,
		MUIA_CycleChain, 1,
	End;
	return( o );
}

#define USE_WIZ_COLORS
#define USE_WIZ_BODY
#include "wiz.h"

#undef SimpleButton

static APTR SimpleButton( char *s )
{
	APTR o = MUI_MakeObject( MUIO_Button, s );
	set( o, MUIA_CycleChain, 1 );
	return( o );
}

static int initapp( void )
{
	APTR bt_next0, bt_prev1, bt_next1, bt_prev2, bt_next2, grp_comment, bt_done;
	static STRPTR regopts[] = { "Register online", "Register by post card", NULL };

	app = ApplicationObject,
		MUIA_Application_Title, "GenesisReg",
		MUIA_Application_Version, "$VER: Genesis  " VERTAG,
		MUIA_Application_Copyright, "© 1997-98 by Active Technologies, All Rights Reserved",
		MUIA_Application_Author, "Oliver Wagner",
		MUIA_Application_Description, "Genesis Registrar",
		MUIA_Application_Base, "GENESIS-REG",
		MUIA_Application_UseRexx, FALSE,

		SubWindow, win = WindowObject,
			MUIA_Window_Title, "Genesis Registration",
			MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
			MUIA_Window_TopEdge, MUIV_Window_TopEdge_Centered,
			MUIA_Window_CloseGadget, FALSE,
			MUIA_Window_RootObject, HGroup,

				Child, VGroup,

					Child, VSpace( 0 ),
					Child, BodychunkObject, TextFrame,
						MUIA_Background, MUII_BACKGROUND,
						MUIA_Bodychunk_Body, wiz_body,
						MUIA_Bodychunk_Compression, 1,
						MUIA_Bodychunk_Depth, WIZ_DEPTH,
						MUIA_FixWidth, WIZ_WIDTH,
						MUIA_FixHeight, WIZ_HEIGHT,
						MUIA_Bitmap_Width, WIZ_WIDTH,
						MUIA_Bitmap_Height, WIZ_HEIGHT,
						MUIA_Bitmap_SourceColors, wiz_colors,
						MUIA_Bitmap_UseFriend, TRUE,
						MUIA_Bitmap_Transparent, 0,
					End,
					Child, VSpace( 0 ),
				End,

				Child, MUI_MakeObject( MUIO_VBar, 0 ),

				Child, grp_page = PageGroup, MUIA_Weight, 10000,

					Child, VGroup,
						Child, VSpace( 0 ),

						Child, TextObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Text_Contents, 
"\n\033c\033uWelcome to Genesis!\033n\n\n" 
"In order to create your keyfile, we need you to\n" 
"enter some personal information including your\n" 
"unique serial code, as found in your registration\n"
"mail or card. This tool will create your key.",
						End,

						Child, VSpace( 0 ),

						Child, VGroup, MUIA_VertWeight, 0,
							Child, MUI_MakeObject( MUIO_HBar, 0 ),

							Child, HGroup,
								Child, HSpace( 0 ),
								Child, bt_next0 = SimpleButton( "Next >>" ),
							End,
						End,
					End,

					Child, VGroup,

						Child, VSpace( 0 ),

						Child, ColGroup( 2 ),

							Child, Label2( "Your full name:" ),
							Child, str_name = stro(),
							Child, Label2( "Street:" ),
							Child, str_street = stro(),
							Child, Label2( "Town/City:" ),
							Child, str_city = stro(),
							Child, Label2( "Area/County:" ),
							Child, str_area = stro(),
							Child, Label2( "Post/ZIP Code:" ),
							Child, str_zip = stro(),
							Child, Label2( "Country:" ),
							Child, str_country = stro(),
							Child, Label2( "Your E-Mail address:" ),
							Child, str_email = stro(),
						End,
	
						Child, VSpace( 0 ),

						Child, txt_status = TextObject, NoFrame, MUIA_Text_PreParse, "\033c\033b", End,

						Child, VGroup, MUIA_VertWeight, 0,
							Child, MUI_MakeObject( MUIO_HBar, 0 ),

							Child, HGroup,
								Child, bt_prev1 = SimpleButton( "<< Prev" ),
								Child, bt_next1 = SimpleButton( "Next >>" ),
							End,
						End,
					End,
	
					Child, VGroup, // 2nd page

						Child, ColGroup( 2 ),
							Child, Label2( "Where bought from:" ),
							Child, str_bought = stro(),
							Child, Label2( "Serial Code:" ),
							Child, str_serial = stro(),
						End,
						Child, grp_comment = PageGroup,
							Child, HGroup, MUIA_VertWeight, 1000,
								Child, VGroup,
									Child, VSpace( 0 ),
									Child, Label2( "Comments:" ),
									Child, VSpace( 0 ),
								End,
								Child, str_comment = TextinputscrollObject, StringFrame, MUIA_CycleChain, 1, MUIA_Textinput_Multiline, TRUE, MUIA_VertWeight, 300, End,
							End,

							Child, HVSpace,
						End,

						Child, cyc_reg = Cycle( regopts ),

						Child, txt_status2 = TextObject, NoFrame, MUIA_Text_PreParse, "\033c\033b", End,

						//Child, VSpace( 0 ),

						Child, VGroup, MUIA_Weight, 0,

							Child, MUI_MakeObject( MUIO_HBar, 0 ),

							Child, HGroup, MUIA_VertWeight, 0,
								Child, bt_prev2 = SimpleButton( "<< Prev" ),
								Child, bt_next2 = SimpleButton( "Finish" ),
							End,
						End,
					End,

					Child, VGroup,
						Child, VSpace( 0 ),

						Child, TextObject, TextFrame, MUIA_Background, MUII_TextBack, MUIA_Text_Contents, 

"\n\033c\033uRegistration completed!\033n\n\n"
"Your Genesis keyfile, containing\n"
"your \033bunique\033n serial code and\n"
"personal information was created\n"
"successfully and stored in SYS:S.\n"
"We suggest you make a backup copy of\n"
"this key, in case of hard disk problems.\n",

						End,

						Child, VSpace( 0 ),

						Child, VGroup, MUIA_VertWeight, 0,
							Child, MUI_MakeObject( MUIO_HBar, 0 ),

							Child, HGroup,
								Child, HSpace( 0 ),
								Child, bt_done = SimpleButton( "Done" ),
							End,
						End,
					End,

				End,
			End,
		End,
	End;
	
	if( !app )
		return( FALSE );

	DoMethod( bt_next0, MUIM_Notify, MUIA_Pressed, FALSE,
		grp_page, 3, MUIM_Set, MUIA_Group_ActivePage, 1
	);
	DoMethod( bt_next0, MUIM_Notify, MUIA_Pressed, FALSE,
		win, 3, MUIM_Set, MUIA_Window_ActiveObject, str_name
	);
	DoMethod( bt_prev1, MUIM_Notify, MUIA_Pressed, FALSE,
		grp_page, 3, MUIM_Set, MUIA_Group_ActivePage, 0
	);
	DoMethod( bt_next1, MUIM_Notify, MUIA_Pressed, FALSE,
		grp_page, 3, MUIM_Set, MUIA_Group_ActivePage, 2
	);
	DoMethod( bt_next1, MUIM_Notify, MUIA_Pressed, FALSE,
		win, 3, MUIM_Set, MUIA_Window_ActiveObject, str_bought
	);
	DoMethod( bt_prev2, MUIM_Notify, MUIA_Pressed, FALSE,
		grp_page, 3, MUIM_Set, MUIA_Group_ActivePage, 1
	);
	DoMethod( bt_prev2, MUIM_Notify, MUIA_Pressed, FALSE,
		win, 3, MUIM_Set, MUIA_Window_ActiveObject, str_name
	);
	DoMethod( bt_next2, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MUIM_Application_ReturnID, 1
	);
	DoMethod( bt_done, MUIM_Notify, MUIA_Pressed, FALSE,
		app, 2, MUIM_Application_ReturnID, 2
	);

	set( bt_prev1, MUIA_CycleChain, 0 );
	set( bt_prev2, MUIA_CycleChain, 0 );
	set( cyc_reg, MUIA_CycleChain, 1 );

	DoMethod( cyc_reg, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		grp_comment, 3, MUIM_Set, MUIA_Group_ActivePage, MUIV_TriggerValue
	);

	set( win, MUIA_Window_ActiveObject, str_name );
	set( win, MUIA_Window_Open, TRUE );

	return( TRUE );
}

static STRPTR getstrp( APTR obj )
{
	char *s;

	get( obj, MUIA_String_Contents, &s );
	return( s );
}

static int checkfield( APTR str, int page )
{
	STRPTR s = getstrp( str );

	s = stpblk( s );

	if( !*s )
	{
		set( page ? txt_status2 : txt_status, MUIA_Text_Contents, "Please fill in this field as well." );
		set( page ? txt_status : txt_status2, MUIA_Text_Contents, "" );
		set( grp_page, MUIA_Group_ActivePage, page + 1 );
		set( win, MUIA_Window_ActiveObject, str );
		return( TRUE );
	}

	return( FALSE );
}

static int checkserial( char *p )
{
	int v1, v2;
	int digits[ 10 ];
	int chk, c;

	v1 = atoi( p );
	srand( v1 );

	p = strchr( p, '-' );
	if( !p )
		return( -1 );
	p = strchr( p + 1, '-' );
	if( !p )
		return( -1 );

	for( c = 0, ++p; c < 10; c++ )
	{
		if( !*p )
			return( -2 );
		digits[ c ] = *p++ - '0';
	}

	for( chk = 0, c = 0; c < 9; c++ )
		chk += digits[ c ];

	if( chk % 10 != digits[ 9 ] )
		return( -3 );

	for( c = 0; c < 9; c++ )
	{
		int v3 = rand();
		if( c == 5 )
		{
			if( digits[ c ] != ( v1 % 10 ) )
				return( -4 );
			continue;
		}
		else if( c == 7 )
			continue;
		if( digits[ c ] != ( v3 % 10 ) )
			return( -5 - c );
	}

	return( 0 );
}

static int checkdata( void )
{
	int val;
	char *p;

	if( checkfield( str_name, 0 ) )
		return( FALSE );
	if( checkfield( str_street, 0 ) )
		return( FALSE );
	if( checkfield( str_city, 0 ) )
		return( FALSE );
	if( checkfield( str_zip, 0 ) )
		return( FALSE );
	if( checkfield( str_country, 0 ) )
		return( FALSE );
	if( checkfield( str_bought, 1 ) )
		return( FALSE );
	if( checkfield( str_serial, 1 ) )
		return( FALSE );

	get( str_serial, MUIA_String_Contents, &p );

	if( !( val = checkserial( p ) ) )
	{
		// copy data
		strcpy( ncinfo.owner, getstrp( str_name ) );
		strcpy( ncinfo.street, getstrp( str_street ) );
		strcpy( ncinfo.city_zip, getstrp( str_city ) );
		strcat( ncinfo.city_zip, " " );
		strcat( ncinfo.city_zip, getstrp( str_city ) );
		strcpy( ncinfo.country, getstrp( str_country ) );
		strcpy( ncinfo.area, getstrp( str_area ) );
		strcpy( ncinfo.email, getstrp( str_email ) );
		strcpy( ncinfo.bought, getstrp( str_bought ) );
		strcpy( ncinfo.serial, getstrp( str_serial ) );

		ncinfo.magic = MAGIC;

		get( cyc_reg, MUIA_Cycle_Active, &ncinfo.wasregged );

		return( TRUE );
	}

	set( txt_status, MUIA_Text_Contents, "Invalid registration number!" );
	set( grp_page, MUIA_Group_ActivePage, 2 );
	set( win, MUIA_Window_ActiveObject, str_serial );
	
	return( FALSE );
}

static int doloop( void )
{
	LONG id;
	ULONG msig = 0;
	int Done = FALSE;

	while( !Done )
	{
		id = DoMethod( app, MUIM_Application_NewInput, &msig );

		switch( id )
		{
			case MUIV_Application_ReturnID_Quit:
				Done = 1;
				break;

			case 1:
				if( checkdata() )
					set( grp_page, MUIA_Group_ActivePage, 3 );
				else
					DisplayBeep( 0 );
				break;

			case 2:
				Done = 2;
				break;
		}

		if( msig && !Done )
			if( ( msig = Wait( msig | SIGBREAKF_CTRL_C ) ) & SIGBREAKF_CTRL_C )
				break;
	}

	return( Done == 2 );
}

static int getinfo( void )
{
	int rc = FALSE;

	if( !( MUIMasterBase = OpenLibrary( "muimaster.library", 8 ) ) )
		return( FALSE );

	if( initapp() )
	{
		rc = doloop();
		MUI_DisposeObject( app );
	}
	CloseLibrary( MUIMasterBase );

	if( rc )
	{
		writencinfo();
	}
	return( rc );
}

static int checkload( void )
{
	BPTR f;
	struct ncinfo ncinfo2;

	if( keylock )
		return( TRUE );

	// try to load ncinfo into memory
	for(;;)
	{
		f = Open( nckeyname, MODE_OLDFILE );
		if( f )
			break;
		if( !getinfo() )
			return( FALSE );
	}

	if( Read( f, &ncinfo2, sizeof( ncinfo ) ) != sizeof( ncinfo ) )
	{
		// key file is hosed
		Close( f );
		return( FALSE );
	}

	// decrypt key
	memset( ivec, 0, sizeof( ivec ) );
	idea_key_schedule( mykey, sched );
	idea_cbc_encrypt( &ncinfo2, &ncinfo, sizeof( ncinfo ), sched, ivec, IDEA_MODE_DECRYPT );

	Close( f );

	if( ncinfo.magic != MAGIC )
	{
		return( FALSE );
	}

	if( checkserial( ncinfo.serial ) )
	{
		return( FALSE );
	}

/*	switch( atoi( ncinfo.serial ) )
	{
		case 1:
			return( FALSE );
	}*/


	// seems we have loaded and decoded fine
	keylock = Lock( nckeyname, EXCLUSIVE_LOCK );

	return( TRUE );
}

#if 0
static struct Process *regproc;
struct Library *SocketBase;

static char linebuffer[ 256 ], readbuffer[ 256 ];
static int lbp;

static void initreadline( void )
{
	lbp = 0;
	memset( linebuffer, 0, sizeof( linebuffer ) );
}

static int readline( int s )
{
	char *p;
	ULONG sig = SIGBREAKF_CTRL_C;
	int rc;

	for(;;)
	{
		p = strchr( linebuffer, '\n' );
		if( p )
		{
			int gc = p - linebuffer + 1;

			stccpy( readbuffer, linebuffer, gc );
			p = strchr( readbuffer, '\r' );
			if( p )
				*p = 0;

			memcpy( linebuffer, &linebuffer[ gc ], sizeof( linebuffer ) - gc );
			lbp -= gc;

			return( FALSE );
		}

		for(;;)
		{
			fd_set fds;

			FD_ZERO(&fds);
			FD_SET(s,&fds);

			rc = WaitSelect( s + 1, &fds, NULL, NULL, NULL, &sig );
			if( rc < 0 )
				return( -1 );
			else if( rc == 1 )
				break;
		}

		rc = Recv( s, &linebuffer[ lbp ], sizeof( linebuffer ) - lbp - 1, 0 );
		if( rc < 1 )
			return( -1 );

		lbp += rc;
		linebuffer[ lbp ] = 0;
	}
}

static int smtpcomm( char *send, int skipnum, int s )
{
	if( send )
	{
		Send( s, send, strlen( send ), 0 );
		Send( s, "\r\n", 2, 0 );
	}

	for(;;)
	{
		long rc;

		if( readline( s ) )
		{
			return( -1 );
		}

		// skip multi line responses
		if( readbuffer[ 3 ] == '-' )
			continue;

		StrToLong( readbuffer, &rc );
		//rc = atoi( readbuffer );

		if( rc == skipnum )
			continue;

		return( rc );
	}
	return( -1 );
}

void __stdargs sendfmt( int s, char *fmt, ... )
{
	char buffer[ 512 ];

	RawDoFmt( fmt, &fmt + 1, (APTR)fmtfunc, buffer );
	Send( s, buffer, strlen( buffer ), 0 );
}

static int domailserver2( void )
{
	int s;
	int rc = 1;
	struct hostent *hent;
	struct sockaddr_in sockadr = { 0 };

	initreadline();

	hent = GetHostByName( "ncreg.vapor.com" );
	if( !hent )
		return( -2  );

	s = Socket( AF_INET, SOCK_STREAM, 0 );
	if( s < 0 )
	{
		return( -2 );		
	}

	sockadr.sin_len = sizeof( sockadr );
	memcpy( &sockadr.sin_addr, *hent->h_addr_list, 4 );
	sockadr.sin_family = AF_INET;
	sockadr.sin_port = 25;

	if( !Connect( s, (APTR)&sockadr, sizeof( sockadr ) ) )
	{
		if( smtpcomm( "HELO ncreg.vapor.com", 220, s ) == 250 )
		{
			if( smtpcomm( "MAIL FROM:<ncreg@vapor.com>", 0, s ) == 250 )
			{
				if( smtpcomm( "RCPT TO:<ncreg@vapor.com>", 0, s ) == 250 )
				{
					if( smtpcomm( "DATA", 0, s ) == 354 )
					{
						struct DateStamp ds;

						DateStamp( &ds );

						// ready to send the mail
						sendfmt( s, "From: %s (%s)\n", ncinfo.email, ncinfo.owner );
						sendfmt( s, "To: ncreg@vapor.com\n" );
						sendfmt( s, "Precedence: special-delivery\n" );
						sendfmt( s, "Subject: NCRegV2\n" );
						sendfmt( s, "Message-Id: <R%lx.%lx.%lx.%lx@ncreg.vapor.com>\n", ds.ds_Tick, ds.ds_Minute, ds.ds_Days, FindTask( NULL ) );
						sendfmt( s, "\n" );

						sendfmt( s, "NCID " VERTAG "\n" );
						sendfmt( s, "NAME %s\n", ncinfo.owner );
						sendfmt( s, "STREET %s\n", ncinfo.street );
						sendfmt( s, "CITY %s\n", ncinfo.city_zip );
						sendfmt( s, "AREA %s\n", ncinfo.area );
						sendfmt( s, "COUNTRY %s\n", ncinfo.country );
						sendfmt( s, "EMAIL %s\n", ncinfo.email );
						sendfmt( s, "BOUGHT %s\n", ncinfo.bought );
						sendfmt( s, "SERIAL %s\n", ncinfo.serial );
						sendfmt( s, "COMMENT %s\n", ncinfo.comment );

						if( smtpcomm( ".", 0, s ) == 250 )
						{
							smtpcomm( "QUIT", 0, s );
							rc = 0;
						}
					}
				}
			}
		}
	}
	CloseSocket( s );
	return( rc );
}

static void regfunc( void )
{
	putreg( REG_A4, ((struct Process*)((*((struct ExecBase**)4))->ThisTask))->pr_ExitData );

	SocketBase = OpenLibrary( "bsdsocket.library", 4 );

	if( SocketBase )
	{
		if( !domailserver2() )
		{
			struct DateStamp ds;

			DateStamp( &ds );
			ncinfo.wasregged = __datecvt( &ds );
			UnLock( keylock );
			writencinfo();
			keylock = Lock( nckeyname, EXCLUSIVE_LOCK );
			CloseLibrary( SocketBase );
			Forbid();
			regproc = NULL;
			return;			
		}
	}
	CloseLibrary( SocketBase );
	Forbid();
	regproc = NULL;
}

static void startreg( void )
{
	if( ncinfo.wasregged || regproc  )
		return;

	regproc = CreateNewProcTags( 
		NP_Entry, regfunc,
		NP_Name, "AmiTCP",
		NP_Priority, 1,
		NP_StackSize, 8192,
		NP_ExitData, getreg( REG_A4 ),
		TAG_DONE
	);
}
#endif

#else

static time_t starttime;

int checkload( void )
{
	struct DateStamp ds;
	time_t now;

	DateStamp( &ds );
	now = __datecvt( &ds );

	if( now - starttime > 3661 )
	{
		SetSignal( SIGBREAKF_CTRL_C, SIGBREAKF_CTRL_C );
		return( FALSE );
	}
	return( TRUE );
}

void startreg( void )
{

} 

#endif

struct Library * __saveds __asm NCL_OpenSocket( void )
{
	struct Library *l;

	if( !checkload() )
		return( NULL );

	l = OpenLibrary( "genesis.library", 2 );
	if( !l )
		return( NULL );
	CloseLibrary( l );

	//startreg();

	return( OpenLibrary( "bsdsocket.library", 4 ) );
}

char * __saveds __asm NCL_GetSerial( void )
{
	if( !checkload() )
		return( NULL );

	return( ncinfo.serial );
}

char * __saveds __asm NCL_GetOwner( void )
{
	if( !checkload() )
		return( NULL );

	return( ncinfo.owner );
}

long __saveds __asm __UserLibInit(register __a6 struct Library *libbase)
{
	char *p;
	struct DateStamp ds;

	SysBase=*((struct ExecBase**)4);
	DOSBase=(struct DosLibrary*)OpenLibrary("dos.library",37);
	if(!DOSBase)
		return(-1);

	IntuitionBase=(struct IntuitionBase*)OpenLibrary("intuition.library",37);
	UtilityBase=OpenLibrary("utility.library",37);

	libbase->lib_Node.ln_Pri = -64;

#ifndef NCLITE
	p = nckeyname;
	while( *p )
	{
		*p++ ^= KEYNAMECODE;
	}
#else
	strcpy( ncinfo.serial, "888888" );
	strcpy( ncinfo.owner, "NC LITE" );
	DateStamp( &ds );
	starttime = __datecvt( &ds );

#endif

	return( 0 );
}

void __saveds __asm __UserLibCleanup(void)
{
	if( keylock )
		UnLock( keylock );

	CloseLibrary( DOSBase );
	CloseLibrary( IntuitionBase );
	CloseLibrary( UtilityBase );
}

void __saveds __asm NCL_reserved1( void )
{
	Supervisor( __UserLibCleanup );
}

void __saveds __asm NCL_reserved2( void )
{
	Supervisor( __UserLibInit );
}

void __saveds __asm NCL_reserved3( void )
{
	Supervisor( NCL_OpenSocket );
}

void __saveds __asm NCL_reserved4( void )
{
	Supervisor( NCL_GetOwner );
}

void __saveds __asm NCL_reserved5( void )
{
	Supervisor( NCL_GetSerial );
}

void __saveds __asm NCL_reserved6( void )
{
	Supervisor( 0 );
}

void __saveds __asm NCL_CallMeFrequently( void )
{
#ifdef NCLITE
	checkload();
#endif
}

void __saveds __asm NCL_CallMeSometimes( void )
{
	checkload();
}
