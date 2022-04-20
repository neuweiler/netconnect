/*
**	FixPath.c
**
**	Fix the current Process search patch list by faking a CLI
**
**	Copyright © 1990-1995 by Olaf `Olsen' Barthel
**		All Rights Reserved
**
**	:ts=4
*/

#include "globals.c"
#include "protos.h"

	/* This is how a linked list of directory search paths looks like. */

struct Path
{
	BPTR path_Next;	// Pointer to next entry
	BPTR path_Lock;	// The drawer in question
};

	/* CloneBString(BSTR Contents,WORD Len):
	 *
	 *	Reserve space for a BString, optionally copy an
	 *	existing BString into the new string.
	 */

STATIC BSTR __regargs
CloneBString(BSTR Contents,WORD Len)
{
	UBYTE	*Source,
			*String;

	if(Contents)
	{
		Source = BADDR(Contents);

		if(*Source > Len)
			Len = *Source;
	}
	else
		Source = NULL;

	if(String = AllocVec(Len + 1,MEMF_ANY))
	{
		if(Source)
			CopyMem(Source,String,((WORD)*String) + 1);
		else
			*String = 0;
	}

	return(MKBADDR(String));
}

	/* FreeBString(BSTR String):
	 *
	 *	Free the memory allocated for a BString.
	 */

STATIC VOID __regargs
FreeBString(BSTR String)
{
	APTR Mem = BADDR(String);

	if(String)
		FreeVec(Mem);
}

	/* DeletePath(BPTR StartPath):
	 *
	 *	Delete a path list as created by CreatePath().
	 */

STATIC VOID __regargs
DeletePath(BPTR StartPath)
{
	struct Path	*First,
				*Next;

	for(First = BADDR(StartPath) ; First ; First = Next)
	{
		Next = BADDR(First -> path_Next);

		UnLock(First -> path_Lock);

		FreeVec(First);
	}
}

	/* ClonePath(BPTR StartPath):
	 *
	 *	Clone a given path list.
	 */

STATIC BPTR __regargs
ClonePath(BPTR StartPath)
{
	struct Path	*First	= NULL,
				*Last	= NULL,
				*List,
				*New;

	for(List = BADDR(StartPath) ; List ; List = BADDR(List -> path_Next))
	{
		if(New = AllocVec(sizeof(struct Path),MEMF_ANY))
		{
			if(!(New -> path_Lock = DupLock(List -> path_Lock)))
			{
				FreeVec(New);

				DeletePath(MKBADDR(First));

				return(NULL);
			}
			else
				New -> path_Next = NULL;
		}
		else
		{
			DeletePath(MKBADDR(First));

			return(NULL);
		}

		if(Last)
			Last -> path_Next = MKBADDR(New);

		if(!First)
			First = New;

		Last = New;
	}

	return(MKBADDR(First));
}

	/* DeleteCLI(struct CommandLineInterface *CLI):
	 *
	 *	Delete a CLI structure as created by CloneCLI.
	 */

VOID __regargs
DeleteCLI(struct CommandLineInterface *CLI)
{
	if(CLI)
	{
		DeletePath(CLI -> cli_CommandDir);

		Close(CLI -> cli_CurrentOutput);
		Close(CLI -> cli_CurrentInput);

		FreeBString(CLI -> cli_CommandFile);
		FreeBString(CLI -> cli_Prompt);
		FreeBString(CLI -> cli_CommandName);
		FreeBString(CLI -> cli_SetName);

		FreeVec(CLI);
	}
}

	/* CloneCLI(struct Message *Message):
	 *
	 *	Clone a CLI structure, as supplied by another process.
	 */

struct CommandLineInterface * __regargs
CloneCLI(struct Message *Message)
{
	struct Process *Source;

	if(!Message -> mn_ReplyPort || (Message -> mn_ReplyPort -> mp_Flags & PF_ACTION) != PA_SIGNAL || !TypeOfMem(Message -> mn_ReplyPort -> mp_SigTask))
		return(NULL);
	else
		Source = Message -> mn_ReplyPort -> mp_SigTask;

	if(Source -> pr_Task . tc_Node . ln_Type == NT_PROCESS && Source -> pr_CLI)
	{
		struct CommandLineInterface	*SourceCLI = BADDR(Source -> pr_CLI),
									*DestCLI;

		if(DestCLI = AllocVec(sizeof(struct CommandLineInterface),MEMF_ANY))
		{
			CopyMem(SourceCLI,DestCLI,sizeof(struct CommandLineInterface));

			if(DestCLI -> cli_SetName = CloneBString(SourceCLI -> cli_SetName,255))
			{
				if(DestCLI -> cli_CommandName = CloneBString(SourceCLI -> cli_CommandName,255))
				{
					if(DestCLI -> cli_Prompt = CloneBString(SourceCLI -> cli_Prompt,255))
					{
						if(DestCLI -> cli_CommandFile = CloneBString(NULL,255))
						{
							DestCLI -> cli_Result2		= 0;
							DestCLI -> cli_ReturnCode	= 0;
							DestCLI -> cli_Interactive	= DOSFALSE;
							DestCLI -> cli_Background	= DOSFALSE;
							DestCLI -> cli_Module		= NULL;

							if(DestCLI -> cli_CurrentInput = Open("NIL:",MODE_NEWFILE))
							{
								if(DestCLI -> cli_CurrentOutput = Open("NIL:",MODE_NEWFILE))
								{
									DestCLI -> cli_StandardInput	= DestCLI -> cli_CurrentInput;
									DestCLI -> cli_StandardOutput	= DestCLI -> cli_CurrentOutput;
									DestCLI -> cli_CommandDir		= ClonePath(DestCLI -> cli_CommandDir);

									return(DestCLI);
								}

								Close(DestCLI -> cli_CurrentInput);
							}

							FreeBString(DestCLI -> cli_CommandFile);
						}

						FreeBString(DestCLI -> cli_Prompt);
					}

					FreeBString(DestCLI -> cli_CommandName);
				}

				FreeBString(DestCLI -> cli_SetName);
			}

			FreeVec(DestCLI);
		}
	}

	return(NULL);
}
