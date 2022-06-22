#include "microirc.h"

//
//	Fake IDENT
//

extern char *identd_username;
static struct Process *identdproc;

static void __saveds identproc( void )
{
	struct mysock *s, *temps;
	char buffer[ 256 ];
	ULONG remoteip, remoteport;
	struct MsgPort *ipcreply;
	struct ipcmsg_addwin ipcm;
	struct ipcmsg_queryport ipcqp;

	s = tcp_socket();
	if( !s )
	{
		identdproc = NULL;
		return;
	}

	ipcm.im.m.mn_ReplyPort = ipcreply = CreateMsgPort();
	ipcm.im.action = IPC_ADDWIN;
	ipcm.color = tc_server;
	ipcm.prefixcode = MSG_PREFIX_IDENTD;

	ipcqp.im.m.mn_ReplyPort = ipcreply;
	ipcqp.im.action = IPC_QUERYPORT;

	if( tcp_bindlisten( s, 113, 2 ) )
	{
		strcpy( ipcm.data, GS( IDENTD_BINDERROR ) );

		doipcmsg( &ipcm );
		DeleteMsgPort( ipcreply );

		//Printf( "bind-error!\n" );
		identdproc = NULL;
		tcp_close( s );
		return;
	}

	while( temps = tcp_accept( s, &remoteip, &remoteport ) )
	{
		while( tcp_recv( temps, buffer, 256 ) > 0 )
		{
			int porthere, portthere;

			char *p = stpbrk( buffer, "\r\n" );
			if( p )
				*p = 0;

			//Printf( "got req: %s\n", buffer );

			doipcmsg( &ipcqp );

			porthere = atoi( buffer );
			p = strchr( buffer, ',' );
			if( p )
				portthere = atoi( p + 1 );

			//kprintf( "my %ld gotreq %ld %ld\n", ipcqp.port, porthere, portthere );

			if( porthere == ipcqp.port ) 
			{
				tcp_sendfmt( temps, "%ld , %ld : USERID : UNIX : %s\r\n", porthere, portthere, identd_username );
				//kprintf( "%ld , %ld : USERID : UNIX : %s\r\n", porthere, portthere, identd_username );
			}
			else
			{
				tcp_sendfmt( temps, "%ld , %ld : ERROR : NO-USER\r\n", porthere, portthere );
			}
			sprintf( ipcm.data, GS( IDENTD_GOTREQ ), tcp_ntoa( temps, remoteip ) );
			doipcmsg( &ipcm );
		}
		tcp_close( temps );
	}

	//Printf( "terminating...\n" );

	tcp_close( s );
	identdproc = NULL;
	DeleteMsgPort( ipcreply );

	//Printf( "goodbye\n" );
}

void startidentd( void )
{
	if( identdproc )
		return;
	identdproc = CreateNewProcTags( 
		NP_Entry, identproc,
		NP_StackSize, 8 * 1024,
		NP_Name, "AmIRC's IdentD Server",
		//NP_Output, Open( "CON:////IDENTD/WAIT/CLOSE", MODE_NEWFILE ),
		NP_Output, NULL,
		NP_CloseOutput, FALSE,
		NP_Input, NULL,
		NP_CloseInput, FALSE,
		NP_CopyVars, FALSE,
		NP_WindowPtr, -1,
		NP_Priority, 1,
		TAG_DONE
	);
}

void stopidentd( void )
{
	if( identdproc )
		Signal( identdproc, SIGBREAKF_CTRL_C );
}

DESTRUCTOR_P(stopidentd,30000)
{
	while( identdproc )
	{
		Signal( identdproc, SIGBREAKF_CTRL_C );
		Delay( 10 );
	}
}

--=_=8<==MD234EC2DFC-49A68F70==8<=_=--
