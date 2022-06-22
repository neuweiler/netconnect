/// general defines
#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define UC(b) (((int)b)&0xff)

#define MAXPATHLEN   256
#define BUFSIZE      512

#define IO_SIGBIT(req)  ((LONG)(((struct IORequest *)req)->io_Message.mn_ReplyPort->mp_SigBit))
#define IO_SIGMASK(req) ((LONG)(1L<<IO_SIGBIT(req)))
#define SIG_SER   (1L << ReadPortSER->mp_SigBit)
#define SIG_TIME  (1L << DoTime->tr_node.io_Message.mn_ReplyPort->mp_SigBit)

///


struct pc_Data
{
	STRPTR Buffer;    /* buffer holding the file (internal use only) */
	LONG Size;        /* variable holding the size of the buffer (internal use only) */
	STRPTR Current;   /* pointer to the current position (internal use only) */

	STRPTR Argument;  /* pointer to the argument */
	STRPTR Contents;  /* pointer to its contents */
};


struct Modem
{
	char Name[80];
	char InitString[80];
};

struct PoP
{
	char Name[81];
	char Phone[81];
};


