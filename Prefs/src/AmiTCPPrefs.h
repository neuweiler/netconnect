#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define MAXPATHLEN 256
#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)

struct pc_Data
{
	STRPTR Buffer;		/* buffer holding the file (internal use only) */
	LONG Size;			/* variable holding the size of the buffer (internal use only) */
	STRPTR Current;	/* pointer to the current position (internal use only) */

	STRPTR Argument;	/* pointer to the argument */
	STRPTR Contents;	/* pointer to its contents */
};


struct Modem
{
	char Name[80];
	char InitString[80];
};

struct User
{
	char Login[41];
	char Password[41];
	LONG UserID;
	LONG GroupID;
	char Name[81];
	char HomeDir[MAXPATHLEN];
	char Shell[81];
	BOOL Disabled;			// I can't overwrite the password immediately when "disable" is selected because it might get deselected again
};

struct Group
{
	char Name[41];
	char Password[21];
	LONG ID;
	char Members[401];
};

struct InfoLine
{
	char Label[41];
	char Contents[81];
};

struct PoP
{
	char Name[81];
	char Phone[81];
};

struct Protocol
{
	char Name[41];
	WORD ID;
	char Aliases[81];
};

struct Service
{
	char Name[41];
	WORD Port;
	char Protocol[41];
	char Aliases[81];
};

struct Inetd
{
	char Service[41];
	BYTE Socket;
	char Protocol[41];
	BYTE Wait;
	char User[41];
	char Server[81];
	char Args[81];

	BOOL Active;
};

struct Host
{
	char Addr[21];
	char Name[41];
	char Aliases[81];
};

struct Network
{
	char Name[41];
	ULONG Number;
	char Aliases[81];
};

struct Rpc
{
	char Name[41];
	ULONG Number;
	char Aliases[81];
};
