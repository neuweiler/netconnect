#include "globals.c"
#include "protos.h"


/****************************************************************************/
/* AmiTCP Prefs class                                                       */
/****************************************************************************/

ULONG AmiTCPPrefs_ImportProvider(struct IClass *cl, Object *obj, Msg msg)
{
#ifndef DEMO
	struct AmiTCPPrefs_Data	*data					= INST_DATA(cl								, obj);
	struct Provider_Data		*provider_data		= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
	struct pc_Data				pc_data;
	STRPTR file;
	char location[81], buffer[MAXPATHLEN];
	BPTR lock;

	*location = NULL;
	if(file = getfilename(win, GetStr(MSG_TX_SelectProviderFile), NULL, FALSE))
	{
		if(ParseConfig(file, &pc_data))
		{
			while(ParseNextLine(&pc_data))
			{
				if(*pc_data.Contents == '#')
				{
					pc_data.Contents += 2;
					if(pc_data.Contents = extract_arg(pc_data.Contents, location, 80, NULL))	// skip "Location:"
					{
						if(!stricmp(location, "Location:") && pc_data.Contents)
						{
							strncpy(location, pc_data.Contents, 80);
							break;
						}
						else
							*location = NULL;
					}
				}
			}
			ParseEnd(&pc_data);
		}

		if(*location)
		{
			if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_InstallProvider), GetStr(MSG_TX_InstallProvider), location))
			{
				sprintf(buffer, "NetConnect:Data/Providers/%ls", location);
				if(!(lock = Lock(buffer, ACCESS_READ)))
					lock = CreateDir(buffer);
				if(lock)
					UnLock(lock);

				AddPart(buffer, FilePart(file), MAXPATHLEN);
				CopyFile(file, buffer);

				DoMethod(data->GR_Provider, MUIM_Provider_PopList_Update, "NetConnect:Data/Providers", MUIV_Provider_PopString_Country);
				set(provider_data->PO_Country, MUIA_Text_Contents, location);
				sprintf(buffer, "NetConnect:Data/Providers/%ls", location);
				DoMethod(data->GR_Provider, MUIM_Provider_PopList_Update, buffer, MUIV_Provider_PopString_Provider);
				set(provider_data->PO_Provider, MUIA_Text_Contents, FilePart(file));
				AddPart(buffer, FilePart(file), MAXPATHLEN);
				DoMethod(obj, MUIM_Provider_PopList_Update, buffer, MUIV_Provider_PopString_PoP);
				nnset(provider_data->PO_PoP, MUIA_Text_Contents, GetStr(MSG_TX_SelectPoP));
			}
		}
	}
#endif

	return(NULL);
}

ULONG AmiTCPPrefs_ExportProvider(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data	*data				= INST_DATA(cl								, obj);
	struct Provider_Data		*provider_data	= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
	STRPTR file, ptr;
	BPTR fh;
	BOOL cr;
	int i;

	if(file = getfilename(win, GetStr(MSG_TX_SelectProviderFile), NULL, FALSE))
	{
		if(fh = Open(file, MODE_NEWFILE))
		{
			FPrintf(fh, "# Provider description file for AmiTCP Prefs\n# Location: %ls\n\n", xget(provider_data->PO_Country, MUIA_Text_Contents));
			FPrintf(fh, "# POPList\n%ls     \"%ls\"\n\n", xget(provider_data->PO_PoP, MUIA_Text_Contents), xget(provider_data->STR_Phone, MUIA_String_Contents));
			FPrintf(fh, "# provider.conf\nName               %ls\n", xget(provider_data->PO_Provider, MUIA_Text_Contents));
			FPrintf(fh, "Interface          %ls\n"		, (xget(provider_data->CY_Interface	, MUIA_Cycle_Active) ? "slip" : "ppp"));
			switch(xget(provider_data->CY_Header, MUIA_Cycle_Active))
			{
				case 1:
					FPrintf(fh, "InterfaceConfig    \"VJCMODE=2\"\n");
					break;
				case 2:
					FPrintf(fh, "InterfaceConfig    \"NOVJC\"\n");
					break;
				default:
					FPrintf(fh, "InterfaceConfig    \n");
			}
			FPrintf(fh, "IPDynamic          %ld\n"		, (xget(provider_data->CY_Address, MUIA_Cycle_Active) ? 0 : 1));
			FPrintf(fh, "NameServer         %ls\n"		, xget(provider_data->STR_NameServer1, MUIA_String_Contents));
			FPrintf(fh, "NameServer         %ls\n"		, xget(provider_data->STR_NameServer2, MUIA_String_Contents));
			FPrintf(fh, "DomainName         \"%ls\"\n", xget(provider_data->STR_DomainName, MUIA_String_Contents));
			FPrintf(fh, "UseBootP           %ld\n"		, (xget(provider_data->CH_BOOTP		, MUIA_Selected) ? 1 : 0));
			FPrintf(fh, "MTU                %ld\n"		, xget(provider_data->SL_MTU			, MUIA_Numeric_Value));
			switch(xget(provider_data->CY_Authentication, MUIA_Cycle_Active))
			{
				case 1:
					FPrintf(fh, "Authentication     chap\n");
					break;
				case 2:
					FPrintf(fh, "Authentication     pap\n");
					break;
				default:
					FPrintf(fh, "Authentication     none\n");
			}
			FPrintf(fh, "MailServer         \"%ls\"\n", xget(provider_data->STR_MailServer	, MUIA_String_Contents));
			FPrintf(fh, "POPServer          \"%ls\"\n", xget(provider_data->STR_POPServer		, MUIA_String_Contents));
			FPrintf(fh, "NewsServer         \"%ls\"\n", xget(provider_data->STR_NewsServer	, MUIA_String_Contents));
			FPrintf(fh, "WWWServer          \"%ls\"\n", xget(provider_data->STR_WWWServer		, MUIA_String_Contents));
			FPrintf(fh, "FTPServer          \"%ls\"\n", xget(provider_data->STR_FTPServer		, MUIA_String_Contents));
			FPrintf(fh, "IRCServer          \"%ls\"\n", xget(provider_data->STR_IRCServer		, MUIA_String_Contents));
			FPrintf(fh, "IRCPort            \"%ld\"\n", xget(provider_data->STR_IRCPort		, MUIA_String_Integer));
			FPrintf(fh, "TimeServer         \"%ls\"\n", xget(provider_data->STR_TimeServer	, MUIA_String_Contents));
			FPrintf(fh, "ProxyServer        \"%ls\"\n", xget(provider_data->STR_ProxyServer	, MUIA_String_Contents));
			FPrintf(fh, "ProxyPort          \"%ld\"\n", xget(provider_data->STR_ProxyPort		, MUIA_String_Integer));

			FPrintf(fh, "\n# LoginScript\n");
			for(i = 0; i < 8; i++)
			{
				cr = xget(provider_data->CH_CR[i], MUIA_Selected);
				ptr = (STRPTR)xget(provider_data->STR_Line[i], MUIA_String_Contents);

				switch(xget(provider_data->CY_Action[i], MUIA_Cycle_Active))
				{
					case 0:
						if(strlen(ptr) || cr)
							FPrintf(fh, "WaitFor	\"%ls%ls\"\n", ptr, (cr ? "\\r" : ""));
						break;
					case 1:
						if(strlen(ptr) || cr)
							FPrintf(fh, "%ls	\"%ls\"\n", (cr ? "SendLn" : "Send"), ptr);
						break;
					case 2:
						FPrintf(fh, "/* \"send username\" */\n%ls	\"\"\n", (cr ? "SendLn" : "Send"));
						break;
					case 3:
						FPrintf(fh, "/* \"send password\" */\n%ls	\"\"\n", (cr ? "SendLn" : "Send"));
						break;
				}
			}
			FPrintf(fh, "# EOP\n");

			Close(fh);
		}
	}

	MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_Okay), GetStr(MSG_TX_ExportProvider));

	return(NULL);
}

ULONG AmiTCPPrefs_LoadProvider(struct IClass *cl, Object *obj, struct MUIP_AmiTCPPrefs_LoadProvider *msg)
{
	struct AmiTCPPrefs_Data	*data					= INST_DATA(cl								, obj);
	struct Provider_Data		*provider_data		= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
	struct pc_Data				pc_data;
	int i;

	if(ParseConfig(msg->file, &pc_data))
	{
		while(ParseNext(&pc_data))
		{
			if(!stricmp(pc_data.Argument, msg->pop))
				break;
		}
		if(!stricmp(pc_data.Argument, msg->pop))
		{
			setstring(provider_data->STR_Phone, pc_data.Contents);

// clear all provider dependant fields.

			setstring(provider_data->STR_DomainName	, "");
			setstring(provider_data->STR_NameServer1	, "");
			setstring(provider_data->STR_NameServer2	, "");
			setstring(provider_data->STR_ProxyServer	, "");
			setstring(provider_data->STR_HostName		, "");
			setstring(provider_data->STR_IP_Address	, "0.0.0.0");
			setcycle(provider_data->CY_Address			, 0);

			setcycle(provider_data->CY_Interface		, 0);
			setcycle(provider_data->CY_Header			, 0);
			setcycle(provider_data->CY_Authentication	, 0);
			setslider(provider_data->SL_MTU				, 1500);
			setcheckmark(provider_data->CH_BOOTP		, 0);

			setstring(provider_data->STR_MailServer	, "");
			setstring(provider_data->STR_POPServer		, "");
			setstring(provider_data->STR_NewsServer	, "");
			setstring(provider_data->STR_WWWServer		, "");
			setstring(provider_data->STR_FTPServer		, "");
			setstring(provider_data->STR_TimeServer	, "");
			setstring(provider_data->STR_IRCServer		, "");
			setstring(provider_data->STR_IRCPort		, "");

			for(i = 0; i < 8; i++)
			{
				setcycle(provider_data->CY_Action[i], 0);
				setstring(provider_data->STR_Line[i], "");
				setcheckmark(provider_data->CH_CR[i], FALSE);
			}

// read the pop's configuration

			FOREVER
			{
				while(*pc_data.Argument != '#')
				{
					if(!ParseNext(&pc_data))
					{
						ParseEnd(&pc_data);
						return(FALSE);
					}
				}
				if(!stricmp(pc_data.Contents, "EOP"))
				{
					ParseEnd(&pc_data);
					return(TRUE);
				}
				else
				if(!stricmp(pc_data.Contents, "provider.conf"))
				{
					i = 0;
					while(ParseNext(&pc_data))
					{
						if(*pc_data.Argument == '#')
							break;

						if(!stricmp(pc_data.Argument, "Interface"))
								setcycle(provider_data->CY_Interface, (stricmp(pc_data.Contents, "ppp") ? 1 : 0));
						if(!stricmp(pc_data.Argument, "InterfaceConfig"))
						{
							if(strstr(pc_data.Contents, "NOVJC"))
								setcycle(provider_data->CY_Header, 2);
							else	if(strstr(pc_data.Contents, "VJCMODE=2"))
								setcycle(provider_data->CY_Header, 1);
							else
								setcycle(provider_data->CY_Header, 0);
						}
						if(!stricmp(pc_data.Argument, "IPDynamic"))
							setcycle(provider_data->CY_Address, (atol(pc_data.Contents) ? 0 : 1));
						if(!stricmp(pc_data.Argument, "IPAddr"))
							setstring(provider_data->STR_IP_Address, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "NameServer"))
							setstring((i++ ? provider_data->STR_NameServer2 : provider_data->STR_NameServer1), pc_data.Contents);
						if(!stricmp(pc_data.Argument, "HostName"))
							setstring(provider_data->STR_HostName, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "DomainName"))
							setstring(provider_data->STR_DomainName, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "UseBootP"))
							setcheckmark(provider_data->CH_BOOTP, atol(pc_data.Contents));
						if(!stricmp(pc_data.Argument, "MTU"))
							set(provider_data->SL_MTU, MUIA_Numeric_Value, atol(pc_data.Contents));
						if(!stricmp(pc_data.Argument, "Authentication"))
						{
							if(stricmp(pc_data.Contents, "chap"))
							{
								if(stricmp(pc_data.Contents, "pap"))
									setcycle(provider_data->CY_Authentication, 0);
								else
									setcycle(provider_data->CY_Authentication, 2);
							}
							else
								setcycle(provider_data->CY_Authentication, 1);
						}
						if(!stricmp(pc_data.Argument, "MailServer"))
							setstring(provider_data->STR_MailServer, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "POPServer"))
							setstring(provider_data->STR_POPServer, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "NewsServer"))
							setstring(provider_data->STR_NewsServer, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "WWWServer"))
							setstring(provider_data->STR_WWWServer, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "FTPServer"))
							setstring(provider_data->STR_FTPServer, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "IRCServer"))
							setstring(provider_data->STR_IRCServer, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "IRCPort"))
							set(provider_data->STR_IRCPort, MUIA_String_Integer, atol(pc_data.Contents));
						if(!stricmp(pc_data.Argument, "ProxyServer"))
							setstring(provider_data->STR_ProxyServer, pc_data.Contents);
						if(!stricmp(pc_data.Argument, "ProxyPort"))
							set(provider_data->STR_ProxyPort, MUIA_String_Integer, atol(pc_data.Contents));
						if(!stricmp(pc_data.Argument, "TimeServer"))
							setstring(provider_data->STR_TimeServer, pc_data.Contents);
					}
				}
				else
				if(!stricmp(pc_data.Contents, "LoginScript"))
				{
					BYTE next = 0, gadget = 0, action;
					BOOL cr;
					STRPTR ptr;

					while(ParseNext(&pc_data))
					{
						if(*pc_data.Argument == '#')
							break;

						action = -1;
						cr = FALSE;

						if(!stricmp(pc_data.Argument, "WaitFor"))
						{
							action = 0;
							if(ptr = strstr(pc_data.Contents, "\\r"))
							{
								*ptr = NULL;
								cr = TRUE;
							}
						}
						if(!stricmp(pc_data.Argument, "SendLn"))
						{
							action = 1;
							cr = TRUE;
						}
						if(!stricmp(pc_data.Argument, "Send"))
							action = 1;
						if(next == 1)
						{
							action = 2;
							next = 0;
							if(!stricmp(pc_data.Argument, "SendLn"))
								cr = TRUE;
							*pc_data.Contents = NULL;
						}
						if(next == 2)
						{
							action = 3;
							next = 0;
							if(!stricmp(pc_data.Argument, "SendLn"))
								cr = TRUE;
							*pc_data.Contents = NULL;
						}
						if(strstr(pc_data.Contents, "send username"))
							next = 1;
						if(strstr(pc_data.Contents, "send password"))
							next = 2;
						if((gadget < 8) && (action != -1))
						{
							setcycle(provider_data->CY_Action[gadget], action);
							setstring(provider_data->STR_Line[gadget], pc_data.Contents);
							setcheckmark(provider_data->CH_CR[gadget], cr);
							gadget++;
						}
					}
				}
				else
					ParseNext(&pc_data);
			}
		}
		ParseEnd(&pc_data);
	}
	return(FALSE);
}


ULONG AmiTCPPrefs_LoadPrefs(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data	*data					= INST_DATA(cl								, obj);
	struct Provider_Data		*provider_data		= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
	struct User_Data			*user_data			= INST_DATA(CL_User->mcc_Class		, data->GR_User);
	struct Modem_Data			*modem_data			= INST_DATA(CL_Modem->mcc_Class		, data->GR_Modem);
	struct Paths_Data			*paths_data			= INST_DATA(CL_Paths->mcc_Class		, data->GR_Paths);
	struct Users_Data			*users_data			= INST_DATA(CL_Users->mcc_Class		, data->GR_Users);
	struct Databases_Data	*db_data				= INST_DATA(CL_Databases->mcc_Class	, data->GR_Databases);
	struct pc_Data pc_data;
	char buffer[41];
	STRPTR ptr;
	int i = 0;

	/**** load provider.conf ****/

	if(ParseConfig("ENV:NetConfig/provider.conf", &pc_data))
	{
		while(ParseNext(&pc_data))
		{
			if(!stricmp(pc_data.Argument, "Interface"))
					setcycle(provider_data->CY_Interface, (stricmp(pc_data.Contents, "ppp") ? 1 : 0));

			if(!stricmp(pc_data.Argument, "InterfaceConfig"))
			{
				if(strstr(pc_data.Contents, "NOVJC"))
					setcycle(provider_data->CY_Header, 2);
				else	if(strstr(pc_data.Contents, "VJCMODE=2"))
					setcycle(provider_data->CY_Header, 1);
				else
					setcycle(provider_data->CY_Header, 0);
			}

			if(!stricmp(pc_data.Argument, "IPDynamic"))
				setcycle(provider_data->CY_Address, (atol(pc_data.Contents) ? 0 : 1));

			if(!stricmp(pc_data.Argument, "IPAddr"))
				setstring(provider_data->STR_IP_Address, pc_data.Contents);

			if(!stricmp(pc_data.Argument, "NameServer"))
				setstring((i++ ? provider_data->STR_NameServer2 : provider_data->STR_NameServer1), pc_data.Contents);

			if(!stricmp(pc_data.Argument, "HostName"))
				setstring(provider_data->STR_HostName, pc_data.Contents);

			if(!stricmp(pc_data.Argument, "DomainName"))
				setstring(provider_data->STR_DomainName, pc_data.Contents);

			if(!stricmp(pc_data.Argument, "UseBootP"))
				setcheckmark(provider_data->CH_BOOTP, atol(pc_data.Contents));

			if(!stricmp(pc_data.Argument, "MTU"))
				set(provider_data->SL_MTU, MUIA_Numeric_Value, atol(pc_data.Contents));

			if(!stricmp(pc_data.Argument, "Phone"))
				setstring(provider_data->STR_Phone, pc_data.Contents);

// these are options that will be found in env:netconfig/provider.conf only :

			if(!stricmp(pc_data.Argument, "Authentication"))
			{
				if(stricmp(pc_data.Contents, "chap"))
				{
					if(stricmp(pc_data.Contents, "pap"))
						setcycle(provider_data->CY_Authentication, 0);
					else
						setcycle(provider_data->CY_Authentication, 2);
				}
				else
					setcycle(provider_data->CY_Authentication, 1);
			}
			if(!stricmp(pc_data.Argument, "MailServer"))
				setstring(provider_data->STR_MailServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "POPServer"))
				setstring(provider_data->STR_POPServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "NewsServer"))
				setstring(provider_data->STR_NewsServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "WWWServer"))
				setstring(provider_data->STR_WWWServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "FTPServer"))
				setstring(provider_data->STR_FTPServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "IRCServer"))
				setstring(provider_data->STR_IRCServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "IRCPort"))
				set(provider_data->STR_IRCPort, MUIA_String_Integer, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "ProxyServer"))
				setstring(provider_data->STR_ProxyServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "ProxyPort"))
				set(provider_data->STR_ProxyPort, MUIA_String_Integer, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "TimeServer"))
				setstring(provider_data->STR_TimeServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Country"))
			{
				char file[MAXPATHLEN];

				nnset(provider_data->PO_Country, MUIA_Text_Contents, pc_data.Contents);
				strcpy(file, "NetConnect:Data/Providers");
				AddPart(file, pc_data.Contents, MAXPATHLEN);
				DoMethod(data->GR_Provider, MUIM_Provider_PopList_Update, file, MUIV_Provider_PopString_Provider);
			}
			if(!stricmp(pc_data.Argument, "Provider"))
			{
				char file[MAXPATHLEN];

				nnset(provider_data->PO_Provider, MUIA_Text_Contents, pc_data.Contents);
				strcpy(file, "NetConnect:Data/Providers");
				AddPart(file, (STRPTR)xget(provider_data->PO_Country, MUIA_Text_Contents), MAXPATHLEN);
				AddPart(file, pc_data.Contents, MAXPATHLEN);
				DoMethod(data->GR_Provider, MUIM_Provider_PopList_Update, file, MUIV_Provider_PopString_PoP);
			}
			if(!stricmp(pc_data.Argument, "PoP"))
				set(provider_data->PO_PoP, MUIA_Text_Contents, pc_data.Contents);
		}
		ParseEnd(&pc_data);
	}

	/**** load the loginscript ****/

	for(i = 0; i < 8; i++)
	{
		setcycle(provider_data->CY_Action[i], 0);
		setstring(provider_data->STR_Line[i], "");
		setcheckmark(provider_data->CH_CR[i], FALSE);
	}

	if(ParseConfig("ENV:NetConfig/LoginScript", &pc_data))
	{
		BYTE next = 0, gadget = 0, action;
		BOOL cr;
		STRPTR ptr;

		while(ParseNext(&pc_data))
		{
			action = -1;
			cr = FALSE;

			if(!stricmp(pc_data.Argument, "WaitFor"))
			{
				action = 0;
				if(ptr = strstr(pc_data.Contents, "\\r"))
				{
					*ptr = NULL;
					cr = TRUE;
				}
			}

			if(!stricmp(pc_data.Argument, "SendLn"))
			{
				action = 1;
				cr = TRUE;
			}

			if(!stricmp(pc_data.Argument, "Send"))
				action = 1;

			if(next == 1)
			{
				action = 2;
				next = 0;
				if(!stricmp(pc_data.Argument, "SendLn"))
					cr = TRUE;
				*pc_data.Contents = NULL;
			}

			if(next == 2)
			{
				action = 3;
				next = 0;
				if(!stricmp(pc_data.Argument, "SendLn"))
					cr = TRUE;
				*pc_data.Contents = NULL;
			}

			if(strstr(pc_data.Contents, "send username"))
				next = 1;
			if(strstr(pc_data.Contents, "send password"))
				next = 2;

			if((gadget < 8) && (action != -1))
			{
				setcycle(provider_data->CY_Action[gadget], action);
				setstring(provider_data->STR_Line[gadget], pc_data.Contents);
				setcheckmark(provider_data->CH_CR[gadget], cr);
				gadget++;
			}
		}
		ParseEnd(&pc_data);
	}
	else
	{
		setstring(provider_data->STR_Line[0], "sername:");
		setcycle(provider_data->CY_Action[1], 2);
		setstring(provider_data->STR_Line[2], "assword:");
		setcycle(provider_data->CY_Action[3], 3);
	}

	/**** load user-startnet ****/

	editor_load("ENV:NetConfig/User-Startnet", user_data->LV_UserStartnet);
	DoMethod(data->GR_User, MUIM_User_UserStartnetList_Active);


	/**** load user-stopnet ****/

	editor_load("ENV:NetConfig/User-Stopnet", user_data->LV_UserStopnet);
	DoMethod(data->GR_User, MUIM_User_UserStopnetList_Active);


	/**** load the user data ****/

	if(ParseConfig("ENV:NetConfig/User.conf", &pc_data))
	{
		while(ParseNext(&pc_data))
		{
			if(!stricmp(pc_data.Argument, "UserLevel"))
				DoMethod(data->MN_Strip, MUIM_SetUData, MEN_EXPERT, MUIA_Menuitem_Checked, atol(pc_data.Contents));

			if(!stricmp(pc_data.Argument, "LoginName"))
				setstring(user_data->STR_LoginName		, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Password"))
				setstring(user_data->STR_Password		, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "EMail"))
				setstring(user_data->STR_EMail			, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "RealName"))
				setstring(user_data->STR_RealName		, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Organisation"))
				setstring(user_data->STR_Organisation	, pc_data.Contents);

			if(!stricmp(pc_data.Argument, "Modem"))
				set(modem_data->TX_Modem, MUIA_Text_Contents, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "ModemInit"))
				setstring(modem_data->STR_ModemInit, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "DialPrefix"))
				setstring(modem_data->PO_DialPrefix, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Device"))
				setstring(modem_data->PO_SerialDriver, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Unit"))
				set(modem_data->STR_SerialUnit, MUIA_String_Integer, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "Baud"))
				set(modem_data->PO_BaudRate, MUIA_String_Integer, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "RedialAttempts"))
				setslider(modem_data->SL_RedialAttempts, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "RedialDelay"))
				setslider(modem_data->SL_RedialDelay, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "CarrierDetect"))
				setcheckmark(modem_data->CH_Carrier, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "7Wire"))
				setcheckmark(modem_data->CH_7Wire, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "OwnDevUnit"))
				setcheckmark(modem_data->CH_OwnDevUnit, atol(pc_data.Contents));

			if(!stricmp(pc_data.Argument, "MailIn"))
				setstring(paths_data->PA_MailIn, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "MailOut"))
				setstring(paths_data->PA_MailOut, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "NewsIn"))
				setstring(paths_data->PA_NewsIn, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "NewsOut"))
				setstring(paths_data->PA_NewsOut, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "FileIn"))
				setstring(paths_data->PA_FileIn, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "FileOut"))
				setstring(paths_data->PA_FileOut, pc_data.Contents);
		}
		ParseEnd(&pc_data);
	}


	/**** load the ModemSettings into the List ****/

	if(ParseConfig("NetConnect:Data/Misc/ModemSettings", &pc_data))
	{
		struct Modem *modem;

		if(modem = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY))
		{
			set(modem_data->LV_Modem, MUIA_List_Quiet, TRUE);
			while(ParseNext(&pc_data))
			{
				strncpy(modem->Name, pc_data.Argument, 80);
				strncpy(modem->InitString, pc_data.Contents, 80);
				DoMethod(modem_data->LV_Modem, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
			}
			FreeVec(modem);
			set(modem_data->LV_Modem, MUIA_List_Quiet, FALSE);
		}
		ParseEnd(&pc_data);
	}


	/** put devices into device list of modem page **/

	set(modem_data->LV_Devices, MUIA_List_Quiet, TRUE);
	{
		BPTR lock;
		struct FileInfoBlock *fib;

		if(lock = Lock("DEVS:", ACCESS_READ))
		{
			if(fib = AllocDosObject(DOS_FIB, NULL))
			{
				if(Examine(lock, fib))
				{
					while(ExNext(lock, fib))
					{
						if((fib->fib_DirEntryType < 0) && strstr(fib->fib_FileName, ".device"))
							DoMethod(modem_data->LV_Devices, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
					}
				}
				FreeDosObject(DOS_FIB, fib);
			}
			UnLock(lock);
		}
	}
	set(modem_data->LV_Devices, MUIA_List_Quiet, FALSE);


	/**** parse the passwd file ****/

	set(users_data->LV_User, MUIA_List_Quiet, TRUE);
	if(ParseConfig("AmiTCP:db/passwd", &pc_data))
	{
		struct User *user;

		if(user = AllocVec(sizeof(struct User), MEMF_ANY))
		{
			while(ParseNextLine(&pc_data))
			{
				if(ptr = strchr(pc_data.Contents, '|'))
				{
					*ptr = NULL;
					strncpy(user->Login, pc_data.Contents, 40);
					pc_data.Contents = ptr + 1;
					if(ptr = strchr(pc_data.Contents, '|'))
					{
						*ptr = NULL;
						strncpy(user->Password, pc_data.Contents, 20);
						pc_data.Contents = ptr + 1;
						if(ptr = strchr(pc_data.Contents, '|'))
						{
							*ptr = NULL;
							user->UserID = atol(pc_data.Contents);
							pc_data.Contents = ptr + 1;
							if(ptr = strchr(pc_data.Contents, '|'))
							{
								*ptr = NULL;
								user->GroupID = atol(pc_data.Contents);
								pc_data.Contents = ptr + 1;
								if(ptr = strchr(pc_data.Contents, '|'))
								{
									*ptr = NULL;
									strncpy(user->Name, pc_data.Contents, 80);
									pc_data.Contents = ptr + 1;
									if(ptr = strchr(pc_data.Contents, '|'))
									{
										*ptr = NULL;
										strncpy(user->HomeDir, pc_data.Contents, MAXPATHLEN - 1);
										strncpy(user->Shell, ptr + 1, 80);
										if(!strcmp(user->Password, "*"))
										{
											user->Disabled = TRUE;
											*user->Password = NULL;
										}
										else
											user->Disabled = FALSE;
										DoMethod(users_data->LV_User, MUIM_List_InsertSingle, user, MUIV_List_Insert_Bottom);
									}
								}
							}
						}
					}
				}
			}
			FreeVec(user);
		}
		ParseEnd(&pc_data);
	}
	set(users_data->LV_User, MUIA_List_Quiet, FALSE);
	DoMethod(data->GR_Users, MUIM_Users_SetUserStates);


	/**** parse the group file ****/

	set(users_data->LV_Groups, MUIA_List_Quiet, TRUE);
	if(ParseConfig("AmiTCP:db/group", &pc_data))
	{
		struct Group *group;

		if(group = AllocVec(sizeof(struct Group), MEMF_ANY))
		{
			while(ParseNextLine(&pc_data))
			{
				if(ptr = strchr(pc_data.Contents, '|'))
				{
					*ptr = NULL;
					strncpy(group->Name, pc_data.Contents, 40);
					pc_data.Contents = ptr + 1;
					if(ptr = strchr(pc_data.Contents, '|'))
					{
						*ptr = NULL;
						strncpy(group->Password, pc_data.Contents, 20);
						pc_data.Contents = ptr + 1;
						if(ptr = strchr(pc_data.Contents, '|'))
						{
							*ptr = NULL;
							group->ID = atol(pc_data.Contents);
							strncpy(group->Members, ptr + 1, 400);
							DoMethod(users_data->LV_Groups, MUIM_List_InsertSingle, group, MUIV_List_Insert_Bottom);
						}
					}
				}
			}
			FreeVec(group);
		}
		ParseEnd(&pc_data);
	}
	set(users_data->LV_Groups, MUIA_List_Quiet, FALSE);
	DoMethod(data->GR_Users, MUIM_Users_SetGroupStates);


	/**** parse the protocols file ****/

	set(db_data->LV_Protocols, MUIA_List_Quiet, TRUE);
	if(ParseConfig("AmiTCP:db/protocols", &pc_data))
	{
		struct Protocol *protocol;

		if(protocol = AllocVec(sizeof(struct Protocol), MEMF_ANY))
		{
			while(ParseNextLine(&pc_data))
			{
				if(ptr = strchr(pc_data.Contents, '#'))
					*ptr = NULL;
				if(ptr = strchr(pc_data.Contents, ';'))
					*ptr = NULL;

				if(pc_data.Contents = extract_arg(pc_data.Contents, protocol->Name, 40, NULL))
				{
					pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL);
					protocol->ID = atol(buffer);

					if(pc_data.Contents)
						strncpy(protocol->Aliases, pc_data.Contents, 80);
					else
						*protocol->Aliases = NULL;

					DoMethod(db_data->LV_Protocols, MUIM_List_InsertSingle, protocol, MUIV_List_Insert_Bottom);
				}
			}
			FreeVec(protocol);
		}
		ParseEnd(&pc_data);
	}
	set(db_data->LV_Protocols, MUIA_List_Quiet, FALSE);
	DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Protocols);


	/**** parse the services file ****/

	set(db_data->LV_Services, MUIA_List_Quiet, TRUE);
	if(ParseConfig("AmiTCP:db/services", &pc_data))
	{
		struct Service *service;

		if(service = AllocVec(sizeof(struct Service), MEMF_ANY))
		{
			while(ParseNextLine(&pc_data))
			{
				if(ptr = strchr(pc_data.Contents, '#'))
					*ptr = NULL;
				if(ptr = strchr(pc_data.Contents, ';'))
					*ptr = NULL;

				if(pc_data.Contents = extract_arg(pc_data.Contents, service->Name, 40, NULL))
				{
					if(pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, '/'))
					{
						service->Port = atol(buffer);

						if(pc_data.Contents = extract_arg(pc_data.Contents, service->Protocol, 40, NULL))
							strncpy(service->Aliases, pc_data.Contents, 80);
						else
							*service->Aliases = NULL;

						DoMethod(db_data->LV_Services, MUIM_List_InsertSingle, service, MUIV_List_Insert_Bottom);
					}
				}
			}
			FreeVec(service);
		}
		ParseEnd(&pc_data);
	}
	set(db_data->LV_Services, MUIA_List_Quiet, FALSE);
	DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Services);


	/**** parse the inetd.conf file ****/

	set(db_data->LV_Inetd, MUIA_List_Quiet, TRUE);
	if(ParseConfig("AmiTCP:db/inetd.conf", &pc_data))
	{
		struct Inetd *inetd;

		if(inetd = AllocVec(sizeof(struct Inetd), MEMF_ANY))
		{
			while(ParseNextLine(&pc_data))
			{
				if(*pc_data.Contents == '#')
				{
					inetd->Active = FALSE;
					pc_data.Contents++;
					if(*pc_data.Contents == ' ' || *pc_data.Contents == 9)
						continue;
				}
				else
					inetd->Active = TRUE;

				if(ptr = strchr(pc_data.Contents, '#'))
					*ptr = NULL;
				if(ptr = strchr(pc_data.Contents, ';'))
					*ptr = NULL;

				if(pc_data.Contents = extract_arg(pc_data.Contents, inetd->Service, 40, NULL))
				{
					if(pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL))
					{
						inetd->Socket = !stricmp(buffer, "dgram");
						if(pc_data.Contents = extract_arg(pc_data.Contents, inetd->Protocol, 40, NULL))
						{
							if(pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL))
							{
								inetd->Wait = (stricmp(buffer, "wait") ? (stricmp(buffer, "dos") ? 0 : 2) : 1);
								if(pc_data.Contents = extract_arg(pc_data.Contents, inetd->User, 40, NULL))
								{
									if(pc_data.Contents = extract_arg(pc_data.Contents, inetd->Server, 80, NULL))
										strncpy(inetd->Args, pc_data.Contents, 80);
									else
										*inetd->Args = NULL;

									DoMethod(db_data->LV_Inetd, MUIM_List_InsertSingle, inetd, MUIV_List_Insert_Bottom);
								}
							}
						}
					}
				}
			}
			FreeVec(inetd);
		}
		ParseEnd(&pc_data);
	}
	set(db_data->LV_Inetd, MUIA_List_Quiet, FALSE);
	DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Inetd);


	/**** parse the hosts file ****/

	set(db_data->LV_Hosts, MUIA_List_Quiet, TRUE);
	if(ParseConfig("AmiTCP:db/Hosts", &pc_data))
	{
		struct Host *host;

		if(host = AllocVec(sizeof(struct Host), MEMF_ANY))
		{
			while(ParseNextLine(&pc_data))
			{
				if(ptr = strchr(pc_data.Contents, '#'))
					*ptr = NULL;
				if(ptr = strchr(pc_data.Contents, ';'))
					*ptr = NULL;

				if(pc_data.Contents = extract_arg(pc_data.Contents, host->Addr, 20, NULL))
				{
					if(pc_data.Contents = extract_arg(pc_data.Contents, host->Name, 40, NULL))
						strncpy(host->Aliases, pc_data.Contents, 80);
					else
						*host->Aliases = NULL;

					DoMethod(db_data->LV_Hosts, MUIM_List_InsertSingle, host, MUIV_List_Insert_Bottom);
				}
			}
			FreeVec(host);
		}
		ParseEnd(&pc_data);
	}
	set(db_data->LV_Hosts, MUIA_List_Quiet, FALSE);
	DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Hosts);


	/**** parse the networks file ****/

	set(db_data->LV_Networks, MUIA_List_Quiet, TRUE);
	if(ParseConfig("AmiTCP:db/Networks", &pc_data))
	{
		struct Network *network;

		if(network = AllocVec(sizeof(struct Network), MEMF_ANY))
		{
			while(ParseNextLine(&pc_data))
			{
				if(ptr = strchr(pc_data.Contents, '#'))
					*ptr = NULL;
				if(ptr = strchr(pc_data.Contents, ';'))
					*ptr = NULL;

				if(pc_data.Contents = extract_arg(pc_data.Contents, network->Name, 40, NULL))
				{
					pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL);
					network->Number = atol(buffer);

					if(pc_data.Contents)
						strncpy(network->Aliases, pc_data.Contents, 80);
					else
						*network->Aliases = NULL;

					DoMethod(db_data->LV_Networks, MUIM_List_InsertSingle, network, MUIV_List_Insert_Bottom);
				}
			}
			FreeVec(network);
		}
		ParseEnd(&pc_data);
	}
	set(db_data->LV_Networks, MUIA_List_Quiet, FALSE);
	DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Networks);


	/**** parse the rpc file ****/

	set(db_data->LV_Rpcs, MUIA_List_Quiet, TRUE);
	if(ParseConfig("AmiTCP:db/rpc", &pc_data))
	{
		struct Rpc *rpc;

		if(rpc = AllocVec(sizeof(struct Rpc), MEMF_ANY))
		{
			while(ParseNextLine(&pc_data))
			{
				if(ptr = strchr(pc_data.Contents, '#'))
					*ptr = NULL;
				if(ptr = strchr(pc_data.Contents, ';'))
					*ptr = NULL;

				if(pc_data.Contents = extract_arg(pc_data.Contents, rpc->Name, 40, NULL))
				{
					pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL);
					rpc->Number = atol(buffer);

					if(pc_data.Contents)
						strncpy(rpc->Aliases, pc_data.Contents, 80);
					else
						*rpc->Aliases = NULL;

					DoMethod(db_data->LV_Rpcs, MUIM_List_InsertSingle, rpc, MUIV_List_Insert_Bottom);
				}
			}
			FreeVec(rpc);
		}
		ParseEnd(&pc_data);
	}
	set(db_data->LV_Rpcs, MUIA_List_Quiet, FALSE);
	DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Rpcs);

	return(NULL);
}

ULONG AmiTCPPrefs_Finish(struct IClass *cl, Object *obj, struct MUIP_AmiTCPPrefs_Finish *msg)
{
	struct AmiTCPPrefs_Data	*data				= INST_DATA(cl								, obj);
	struct Provider_Data		*provider_data	= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
	struct User_Data			*user_data		= INST_DATA(CL_User->mcc_Class		, data->GR_User);
	struct Modem_Data			*modem_data		= INST_DATA(CL_Modem->mcc_Class		, data->GR_Modem);
	struct Paths_Data			*paths_data		= INST_DATA(CL_Paths->mcc_Class		, data->GR_Paths);
	struct Users_Data			*users_data		= INST_DATA(CL_Users->mcc_Class		, data->GR_Users);
	struct Databases_Data	*db_data			= INST_DATA(CL_Databases->mcc_Class	, data->GR_Databases);
	BPTR fh;
	int i;

	if(msg->level)
	{
		if(get_file_size("ENV:NetConfig") != -2)
		{
			BPTR lock;
			if(lock = CreateDir("ENV:NetConfig"))
				UnLock(lock);
		}
		if(fh = Open("ENV:NetConfig/Provider.conf", MODE_NEWFILE))
		{
			FPrintf(fh, "Country            \"%ls\"\n", xget(provider_data->PO_Country		, MUIA_Text_Contents));
			FPrintf(fh, "Provider           \"%ls\"\n", xget(provider_data->PO_Provider	, MUIA_Text_Contents));
			FPrintf(fh, "PoP                \"%ls\"\n", xget(provider_data->PO_PoP			, MUIA_Text_Contents));
			FPrintf(fh, "DialUp             1\n");
			FPrintf(fh, "Interface          %ls\n"		, (xget(provider_data->CY_Interface	, MUIA_Cycle_Active) ? "slip" : "ppp"));
			switch(xget(provider_data->CY_Header, MUIA_Cycle_Active))
			{
				case 1:
					FPrintf(fh, "InterfaceConfig    \"VJCMODE=2\"\n");
					break;
				case 2:
					FPrintf(fh, "InterfaceConfig    \"NOVJC\"\n");
					break;
				default:
					FPrintf(fh, "InterfaceConfig    \n");
			}
			FPrintf(fh, "NeedSerial         1\n");
			FPrintf(fh, "IPDynamic          %ld\n"		, (xget(provider_data->CY_Address, MUIA_Cycle_Active) ? 0 : 1));
			FPrintf(fh, "IPAddr             %ls\n"		, xget(provider_data->STR_IP_Address, MUIA_String_Contents));
			FPrintf(fh, "DestIP             \n");
			FPrintf(fh, "NSDynamic          1\n");
			FPrintf(fh, "HostName           \"%ls\"\n", xget(provider_data->STR_HostName	, MUIA_String_Contents));
			FPrintf(fh, "UseBootP           %ld\n"		, (xget(provider_data->CH_BOOTP		, MUIA_Selected) ? 1 : 0));
			FPrintf(fh, "MTU                %ld\n"		, xget(provider_data->SL_MTU			, MUIA_Numeric_Value));

#ifdef NETCOM
			FPrintf(fh, "Phone              \"08450798888\"\n");
			FPrintf(fh, "DomainName         \"netcomuk.co.uk\"\n");
			FPrintf(fh, "NameServer         194.42.224.130\n");
			FPrintf(fh, "NameServer         194.42.224.131\n");
			FPrintf(fh, "Authentication     pap\n");
			FPrintf(fh, "MailServer         \"smtp.netcomuk.co.uk\"\n");
			FPrintf(fh, "POPServer          \"popd.netcomuk.co.uk\"\n");
			FPrintf(fh, "NewsServer         \"nntp.netcomuk.co.uk\"\n");
			FPrintf(fh, "WWWServer          \"www.netcom.net.uk\"\n");
			FPrintf(fh, "FTPServer          \"ftp.netcom.net.uk\"\n");
			FPrintf(fh, "IRCServer          \"irc.netcomuk.co.uk\"\n");
			FPrintf(fh, "ProxyServer        \"www-cache.netcomuk.co.uk\"\n");
			FPrintf(fh, "ProxyPort          \"8080\"\n");
			FPrintf(fh, "TimeServer         \"ntp.netcomuk.co.uk\"\n");
#else
			FPrintf(fh, "Phone              \"%ls\"\n", xget(provider_data->STR_Phone		, MUIA_String_Contents));
			FPrintf(fh, "DomainName         \"%ls\"\n", xget(provider_data->STR_DomainName, MUIA_String_Contents));
			FPrintf(fh, "NameServer         %ls\n"		, xget(provider_data->STR_NameServer1, MUIA_String_Contents));
			FPrintf(fh, "NameServer         %ls\n"		, xget(provider_data->STR_NameServer2, MUIA_String_Contents));
			switch(xget(provider_data->CY_Authentication, MUIA_Cycle_Active))
			{
				case 1:
					FPrintf(fh, "Authentication     chap\n");
					break;
				case 2:
					FPrintf(fh, "Authentication     pap\n");
					break;
				default:
					FPrintf(fh, "Authentication     none\n");
			}
			FPrintf(fh, "MailServer         \"%ls\"\n", xget(provider_data->STR_MailServer	, MUIA_String_Contents));
			FPrintf(fh, "POPServer          \"%ls\"\n", xget(provider_data->STR_POPServer		, MUIA_String_Contents));
			FPrintf(fh, "NewsServer         \"%ls\"\n", xget(provider_data->STR_NewsServer	, MUIA_String_Contents));
			FPrintf(fh, "WWWServer          \"%ls\"\n", xget(provider_data->STR_WWWServer		, MUIA_String_Contents));
			FPrintf(fh, "FTPServer          \"%ls\"\n", xget(provider_data->STR_FTPServer		, MUIA_String_Contents));
			FPrintf(fh, "IRCServer          \"%ls\"\n", xget(provider_data->STR_IRCServer		, MUIA_String_Contents));
			FPrintf(fh, "ProxyServer        \"%ls\"\n", xget(provider_data->STR_ProxyServer	, MUIA_String_Contents));
			FPrintf(fh, "ProxyPort          \"%ld\"\n", xget(provider_data->STR_ProxyPort		, MUIA_String_Integer));
			FPrintf(fh, "TimeServer         \"%ls\"\n", xget(provider_data->STR_TimeServer	, MUIA_String_Contents));
#endif
			FPrintf(fh, "IRCPort            \"%ld\"\n", xget(provider_data->STR_IRCPort		, MUIA_String_Integer));

			Close(fh);
		}
		if(fh = Open("ENV:NetConfig/User.conf", MODE_NEWFILE))
		{
			FPrintf(fh, "UserLevel          \"%ld\"\n", expert);
			FPrintf(fh, "LoginName          \"%ls\"\n", xget(user_data->STR_LoginName		, MUIA_String_Contents));
			FPrintf(fh, "Password           \"%ls\"\n", xget(user_data->STR_Password		, MUIA_String_Contents));
			FPrintf(fh, "EMail              \"%ls\"\n", xget(user_data->STR_EMail			, MUIA_String_Contents));
			FPrintf(fh, "RealName           \"%ls\"\n", xget(user_data->STR_RealName		, MUIA_String_Contents));
			FPrintf(fh, "Organisation       \"%ls\"\n", xget(user_data->STR_Organisation	, MUIA_String_Contents));

			FPrintf(fh, "Modem              \"%ls\"\n", xget(modem_data->TX_Modem			, MUIA_Text_Contents));
			FPrintf(fh, "ModemInit          \"%ls\"\n", xget(modem_data->STR_ModemInit		, MUIA_String_Contents));
			FPrintf(fh, "DialPrefix         \"%ls\"\n", xget(modem_data->PO_DialPrefix		, MUIA_String_Contents));
			FPrintf(fh, "Device             \"%ls\"\n", xget(modem_data->PO_SerialDriver	, MUIA_String_Contents));
			FPrintf(fh, "Unit               %ld\n"		, xget(modem_data->STR_SerialUnit	, MUIA_String_Integer));
			FPrintf(fh, "Baud               %ld\n"		, xget(modem_data->PO_BaudRate		, MUIA_String_Integer));
			FPrintf(fh, "RedialAttempts     %ld\n"		, xget(modem_data->SL_RedialAttempts, MUIA_Numeric_Value));
			FPrintf(fh, "RedialDelay        %ld\n"		, xget(modem_data->SL_RedialDelay	, MUIA_Numeric_Value));
			FPrintf(fh, "CarrierDetect      %ld\n"		, xget(modem_data->CH_Carrier			, MUIA_Selected));
			FPrintf(fh, "7Wire              %ld\n"		, xget(modem_data->CH_7Wire			, MUIA_Selected));
			FPrintf(fh, "OwnDevUnit         %ld\n"		, xget(modem_data->CH_OwnDevUnit		, MUIA_Selected));

			FPrintf(fh, "MailIn             \"%ls\"\n", xget(paths_data->PA_MailIn			, MUIA_String_Contents));
			FPrintf(fh, "MailOut            \"%ls\"\n", xget(paths_data->PA_MailOut			, MUIA_String_Contents));
			FPrintf(fh, "NewsIn             \"%ls\"\n", xget(paths_data->PA_NewsIn			, MUIA_String_Contents));
			FPrintf(fh, "NewsOut            \"%ls\"\n", xget(paths_data->PA_NewsOut			, MUIA_String_Contents));
			FPrintf(fh, "FileIn             \"%ls\"\n", xget(paths_data->PA_FileIn			, MUIA_String_Contents));
			FPrintf(fh, "FileOut            \"%ls\"\n", xget(paths_data->PA_FileOut			, MUIA_String_Contents));

			Close(fh);
		}

		if(fh = Open("ENV:NetConfig/LoginScript", MODE_NEWFILE))
		{
			BOOL cr;
			STRPTR ptr;

			FPrintf(fh, "/* This file was generated by AmiTCP Prefs. Please do NOT modify manually ! */\n\n");

			/** write loginscript only when authentication = none !! **/
			if(!xget(provider_data->CY_Authentication, MUIA_Cycle_Active))
			{
				for(i = 0; i < 8; i++)
				{
					cr = xget(provider_data->CH_CR[i], MUIA_Selected);
					ptr = (STRPTR)xget(provider_data->STR_Line[i], MUIA_String_Contents);

					switch(xget(provider_data->CY_Action[i], MUIA_Cycle_Active))
					{
						case 0:
							if(strlen(ptr) || cr)
								FPrintf(fh, "WaitFor	\"%ls%ls\"\n", ptr, (cr ? "\\r" : ""));
							break;
						case 1:
							if(strlen(ptr) || cr)
								FPrintf(fh, "%ls	\"%ls\"\n", (cr ? "SendLn" : "Send"), ptr);
							break;
						case 2:
							FPrintf(fh, "/* \"send username\" */\n%ls	\"%ls\"\n", (cr ? "SendLn" : "Send"), xget(user_data->STR_LoginName, MUIA_String_Contents));
							break;
						case 3:
							FPrintf(fh, "/* \"send password\" */\n%ls	\"%ls\"\n", (cr ? "SendLn" : "Send"), xget(user_data->STR_Password	, MUIA_String_Contents));
							break;
					}
				}
			}
			FPrintf(fh, "\nexit 0;\n");
			Close(fh);
		}

		editor_save("ENV:NetConfig/User-Startnet", user_data->LV_UserStartnet);

		editor_save("ENV:NetConfig/User-Stopnet", user_data->LV_UserStopnet);


		if(fh = Open("ENV:NetConfig/AutoInterfaces", MODE_NEWFILE))
		{
			if(xget(provider_data->CY_Interface, MUIA_Cycle_Active))
				FPrintf(fh, "slip DEV=DEVS:Networks/aslip.device UNIT=0 DoOffline ConfigFileName=ENV:Sana2/aslip0.config ");
			else
				FPrintf(fh, "ppp DEV=DEVS:Networks/appp.device UNIT=0 DoOffline ConfigFileName=ENV:Sana2/appp0.config ");

			FPrintf(fh, "ConfigFileContents=\"");
			FPrintf(fh, "%ls "			, xget(modem_data->PO_SerialDriver	, MUIA_String_Contents));
			FPrintf(fh, "%ld "			, xget(modem_data->STR_SerialUnit	, MUIA_String_Integer));
			FPrintf(fh, "%ld Shared "	, xget(modem_data->PO_BaudRate		, MUIA_String_Integer));
			FPrintf(fh, "0.0.0.0 ");
			if(xget(modem_data->CH_Carrier		, MUIA_Selected))
				FPrintf(fh, "CD ");
			if(xget(modem_data->CH_7Wire			, MUIA_Selected))
				FPrintf(fh, "7Wire ");
			if(xget(modem_data->CH_OwnDevUnit	, MUIA_Selected))
				FPrintf(fh, "UseODU ");
			if(!xget(provider_data->CY_Interface, MUIA_Cycle_Active))
			{
				switch(xget(provider_data->CY_Authentication, MUIA_Cycle_Active))
				{
					case 1:
						FPrintf(fh, "ALLOWCHAPMS=YES ALLOWCHAPMD5=YES USERID=%ls PASSWORD=%ls ", xget(user_data->STR_LoginName, MUIA_String_Contents), xget(user_data->STR_Password, MUIA_String_Contents));
						break;
					case 2:
						FPrintf(fh, "ALLOWPAP=YES USERID=%ls PASSWORD=%ls ", xget(user_data->STR_LoginName, MUIA_String_Contents), xget(user_data->STR_Password, MUIA_String_Contents));
						break;
				}
			}
			FPrintf(fh, "MTU=%ld\"\n", (xget(provider_data->CY_Interface, MUIA_Cycle_Active) ? 1524 : xget(provider_data->SL_MTU, MUIA_Numeric_Value)));

			Close(fh);
		}

		if(fh = Open("ENV:NetConfig/ModemInitString", MODE_NEWFILE))
		{
			FPrintf(fh, "%ls", xget(modem_data->STR_ModemInit		, MUIA_String_Contents));
			Close(fh);
		}

		if(fh = Open("ENV:NetConfig/ModemDialPrefix", MODE_NEWFILE))
		{
			FPrintf(fh, "%ls", xget(modem_data->STR_DialPrefix		, MUIA_String_Contents));
			Close(fh);
		}

		if(fh = Open("ENV:NetConfig/ModemDialAttempts", MODE_NEWFILE))
		{
			FPrintf(fh, "%ld", xget(modem_data->SL_RedialAttempts	, MUIA_Numeric_Value));
			Close(fh);
		}

		if(fh = Open("ENV:NetConfig/ModemDialDelay", MODE_NEWFILE))
		{
			FPrintf(fh, "%ld", xget(modem_data->SL_RedialDelay		, MUIA_Numeric_Value));
			Close(fh);
		}

	}
	if(msg->level == 2)
	{
		if(get_file_size("ENVARC:NetConfig") != -2)
		{
			BPTR lock;
			if(lock = CreateDir("ENVARC:NetConfig"))
				UnLock(lock);
		}
		CopyFile("ENV:NetConfig/Provider.conf"		, "ENVARC:NetConfig/Provider.conf");
		CopyFile("ENV:NetConfig/AutoInterfaces"	, "ENVARC:NetConfig/AutoInterfaces");
		CopyFile("ENV:NetConfig/User.conf"			, "ENVARC:NetConfig/User.conf");
		CopyFile("ENV:NetConfig/LoginScript"		, "ENVARC:NetConfig/LoginScript");
		CopyFile("ENV:NetConfig/User-Startnet"		, "ENVARC:NetConfig/User-Startnet");
		CopyFile("ENV:NetConfig/User-Stopnet"		, "ENVARC:NetConfig/User-Stopnet");
		CopyFile("ENV:NetConfig/ModemInitString"	, "ENVARC:NetConfig/ModemInitString");
		CopyFile("ENV:NetConfig/ModemDialPrefix"	, "ENVARC:NetConfig/ModemDialPrefix");
		CopyFile("ENV:NetConfig/ModemDialAttempts", "ENVARC:NetConfig/ModemDialAttempts");
		CopyFile("ENV:NetConfig/ModemDialDelay"	, "ENVARC:NetConfig/ModemDialDelay");

		if(fh = Open("AmiTCP:db/protocols", MODE_NEWFILE))
		{
			struct Protocol *protocol;

			FPrintf(fh, "## This file was generated by AmiTCP Prefs\n##\n");
			i = 0;
			FOREVER
			{
				DoMethod(db_data->LV_Protocols, MUIM_List_GetEntry, i++, &protocol);
				if(!protocol)
					break;
				FPrintf(fh, "%ls	%ld", protocol->Name, protocol->ID);
				if(*protocol->Aliases)
					FPrintf(fh, "	%ls", protocol->Aliases);
				FPrintf(fh, "\n");
			}
			Close(fh);
		}

		if(fh = Open("AmiTCP:db/services", MODE_NEWFILE))
		{
			struct Service *service;

			FPrintf(fh, "## This file was generated by AmiTCP Prefs\n##\n");
			i = 0;
			FOREVER
			{
				DoMethod(db_data->LV_Services, MUIM_List_GetEntry, i++, &service);
				if(!service)
					break;
				FPrintf(fh, "%ls		%ld/%ls", service->Name, service->Port, service->Protocol);
				if(*service->Aliases)
					FPrintf(fh, "		%ls", service->Aliases);
				FPrintf(fh, "\n");
			}
			Close(fh);
		}

		if(fh = Open("AmiTCP:db/inetd.conf", MODE_NEWFILE))
		{
			struct Inetd *inetd;

			FPrintf(fh, "## This file was generated by AmiTCP Prefs\n##\n");
			i = 0;
			FOREVER
			{
				DoMethod(db_data->LV_Inetd, MUIM_List_GetEntry, i++, &inetd);
				if(!inetd)
					break;
				if(!inetd->Active)
					FPrintf(fh, "#");
				FPrintf(fh, "%ls		%ls	%ls	%ls	%ls	%ls", inetd->Service, (inetd->Socket ? "dgram" : "stream"), inetd->Protocol, (inetd->Wait ? (inetd->Wait == 2 ? "dos" : "wait") : "nowait"), inetd->User, inetd->Server);
				if(*inetd->Args)
					FPrintf(fh, "	%ls", inetd->Args);
				FPrintf(fh, "\n");
			}
			Close(fh);
		}
		if(fh = Open("AmiTCP:db/hosts", MODE_NEWFILE))
		{
			struct Host *host;

			FPrintf(fh, "## This file was generated by AmiTCP Prefs\n##\n");
			i = 0;
			FOREVER
			{
				DoMethod(db_data->LV_Hosts, MUIM_List_GetEntry, i++, &host);
				if(!host)
					break;
				FPrintf(fh, "%ls		%ls", host->Addr, host->Name);
				if(*host->Aliases)
					FPrintf(fh, "	%ls", host->Aliases);
				FPrintf(fh, "\n");
			}
			Close(fh);
		}
		if(fh = Open("AmiTCP:db/networks", MODE_NEWFILE))
		{
			struct Network *network;

			FPrintf(fh, "## This file was generated by AmiTCP Prefs\n##\n");
			i = 0;
			FOREVER
			{
				DoMethod(db_data->LV_Networks, MUIM_List_GetEntry, i++, &network);
				if(!network)
					break;
				FPrintf(fh, "%ls		%ld", network->Name, network->Number);
				if(*network->Aliases)
					FPrintf(fh, "	%ls", network->Aliases);
				FPrintf(fh, "\n");
			}
			Close(fh);
		}
		if(fh = Open("AmiTCP:db/rpc", MODE_NEWFILE))
		{
			struct Rpc *rpc;

			FPrintf(fh, "## This file was generated by AmiTCP Prefs\n##\n");
			i = 0;
			FOREVER
			{
				DoMethod(db_data->LV_Rpcs, MUIM_List_GetEntry, i++, &rpc);
				if(!rpc)
					break;
				FPrintf(fh, "%ls		%ld", rpc->Name, rpc->Number);
				if(*rpc->Aliases)
					FPrintf(fh, "	%ls", rpc->Aliases);
				FPrintf(fh, "\n");
			}
			Close(fh);
		}

		if(fh = Open("AmiTCP:db/passwd", MODE_NEWFILE))
		{
			struct User *user;

			i = 0;
			FOREVER
			{
				DoMethod(users_data->LV_User, MUIM_List_GetEntry, i++, &user);
				if(!user)
					break;
				FPrintf(fh, "%ls|%ls|%ld|%ld|%ls|%ls|%ls\n", user->Login, (user->Disabled ? "*" : user->Password), user->UserID, user->GroupID, user->Name, user->HomeDir, user->Shell);
			}
			Close(fh);
		}

		if(fh = Open("AmiTCP:db/group", MODE_NEWFILE))
		{
			struct Group *group;

			i = 0;
			FOREVER
			{
				DoMethod(users_data->LV_Groups, MUIM_List_GetEntry, i++, &group);
				if(!group)
					break;
				FPrintf(fh, "%ls|%ls|%ld|%ls\n", group->Name, /*group->Password*/ "*", group->ID, group->Members);
			}
			Close(fh);
		}

	}

	DoMethod((Object *)xget(obj, MUIA_ApplicationObject), MUIM_Application_PushMethod, (Object *)xget(obj, MUIA_ApplicationObject), 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	return(NULL);
}

ULONG AmiTCPPrefs_About(struct IClass *cl, Object *obj, Msg msg)
{
	Object *req;

	set(win, MUIA_Window_Sleep, TRUE);
	if(req = (APTR)NewObject(CL_About->mcc_Class, NULL, TAG_DONE))
	{
		DoMethod(app, OM_ADDMEMBER, req);
		set(req, MUIA_Window_Open, TRUE);
	}
	else
		set(win, MUIA_Window_Sleep, FALSE);

	return(NULL);
}

ULONG AmiTCPPrefs_About_Finish(struct IClass *cl, Object *obj, struct MUIP_AmiTCPPrefs_About_Finish *msg)
{
	Object *window = msg->window;

	set(window, MUIA_Window_Open, FALSE);
	set(win, MUIA_Window_Sleep, FALSE);
	DoMethod(app, OM_REMMEMBER, window);
	MUI_DisposeObject(window);

#ifdef DEMO
	if(msg->level)
	{
		BPTR ofh, ifh;

		if(ofh = Open("NIL:", MODE_NEWFILE))
		{
			if(ifh = Open("NIL:", MODE_OLDFILE))
			{
				if(SystemTags("NetConnect:Update",
					SYS_Output		, ofh,
					SYS_Input		, ifh,
					SYS_Asynch		, TRUE,
					SYS_UserShell	, TRUE,
					NP_StackSize	, 12288,
					NP_Priority		, 0,
					TAG_DONE) == -1)
				{
					Close(ifh);
					Close(ofh);
				}
			}
			else
				Close(ofh);
		}
	}
#endif

	return(NULL);
}

ULONG AmiTCPPrefs_SetPage(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);
	LONG active;

	active = xget(data->LV_Pager, MUIA_List_Active);
	if(active == MUIV_List_Active_Off)
		set(data->LV_Pager, MUIA_List_Active, MUIV_List_Active_Top);
	else
	{
		Object *new = NULL;

		switch(active)
		{
			case 0:
				new = data->GR_Info;
				break;
			case 1:
				new = data->GR_Provider;
				break;
			case 2:
				new = data->GR_User;
				break;
			case 3:
				new = data->GR_Modem;
				break;
			case 4:
				new = data->GR_Users;
				break;
			case 5:
				new = data->GR_Databases;
				break;
			case 6:
				new = data->GR_Paths;
				break;
			case 7:
				new = data->GR_Events;
				break;
		}

		if(new)
		{
			DoMethod(data->GR_Pager, MUIM_Group_InitChange);
			DoMethod(group, MUIM_Group_InitChange);

			DoMethod(data->GR_Pager, OM_REMMEMBER, data->GR_Active);
			DoMethod(group, OM_ADDMEMBER, data->GR_Active);
			DoMethod(group, OM_REMMEMBER, new);
			DoMethod(data->GR_Pager, OM_ADDMEMBER, new);
			data->GR_Active = new;

			DoMethod(data->GR_Pager, MUIM_Group_ExitChange);
			DoMethod(group, MUIM_Group_ExitChange);
		}
	}

	return(NULL);
}

ULONG AmiTCPPrefs_Expert(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);
//	struct Provider_Data		*provider_data	= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
	struct User_Data			*user_data		= INST_DATA(CL_User->mcc_Class		, data->GR_User);
	LONG value;

	set(obj, MUIA_Window_Sleep, TRUE);

	DoMethod(data->MN_Strip, MUIM_GetUData, MEN_EXPERT, MUIA_Menuitem_Checked, &value);
	if(value != expert)
	{
		expert = value;
		set(data->LV_Pager, MUIA_List_Active, MUIV_List_Active_Top);
		set(data->LV_Pager, MUIA_List_Quiet, TRUE);
		if(expert)
		{
			DoMethod(data->LV_Pager, MUIM_List_InsertSingle, GetStr(MSG_Pages5), MUIV_List_Insert_Bottom);
			DoMethod(data->LV_Pager, MUIM_List_InsertSingle, GetStr(MSG_Pages6), MUIV_List_Insert_Bottom);
			DoMethod(data->LV_Pager, MUIM_List_InsertSingle, GetStr(MSG_Pages7), MUIV_List_Insert_Bottom);
		}
		else
		{
			DoMethod(data->LV_Pager, MUIM_List_Remove, MUIV_List_Remove_Last);
			DoMethod(data->LV_Pager, MUIM_List_Remove, MUIV_List_Remove_Last);
			DoMethod(data->LV_Pager, MUIM_List_Remove, MUIV_List_Remove_Last);
		}
		set(data->LV_Pager, MUIA_List_Quiet, FALSE);
	}
	// these need to be set anyway (see call in main.c)
	set(user_data->GR_UserStartnet, MUIA_Disabled, !expert);
	set(user_data->GR_UserStopnet, MUIA_Disabled, !expert);

	set(obj, MUIA_Window_Sleep, FALSE);

	return(NULL);
}

ULONG AmiTCPPrefs_InitGroups(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);
	BOOL success = FALSE;

	data->GR_Provider			= data->GR_User		=
	data->GR_Modem				= data->GR_Paths		=
	data->GR_Users				= NULL;

	if(data->GR_Provider		= NewObject(CL_Provider->mcc_Class	, NULL, TAG_DONE))
		DoMethod(group, OM_ADDMEMBER, data->GR_Provider);
	if(data->GR_User			= NewObject(CL_User->mcc_Class		, NULL, TAG_DONE))
		DoMethod(group, OM_ADDMEMBER, data->GR_User);
	if(data->GR_Modem			= NewObject(CL_Modem->mcc_Class		, NULL, TAG_DONE))
		DoMethod(group, OM_ADDMEMBER, data->GR_Modem);
	if(data->GR_Paths			= NewObject(CL_Paths->mcc_Class		, NULL, TAG_DONE))
		DoMethod(group, OM_ADDMEMBER, data->GR_Paths);
	if(data->GR_Users			= NewObject(CL_Users->mcc_Class		, NULL, TAG_DONE))
		DoMethod(group, OM_ADDMEMBER, data->GR_Users);
	if(data->GR_Databases	= NewObject(CL_Databases->mcc_Class	, NULL, TAG_DONE))
		DoMethod(group, OM_ADDMEMBER, data->GR_Databases);
	if(data->GR_Events		= NewObject(CL_Events->mcc_Class		, NULL, TAG_DONE))
		DoMethod(group, OM_ADDMEMBER, data->GR_Events);
	if(data->GR_InfoWindow	= NewObject(CL_InfoWindow->mcc_Class, NULL, TAG_DONE))
		DoMethod(app, OM_ADDMEMBER, data->GR_InfoWindow);

	if(data->GR_Provider	&& data->GR_User	&& data->GR_Modem &&
		data->GR_Paths		&& data->GR_Users	&& data->GR_Databases &&
		data->GR_Events)
	{
		struct Provider_Data		*provider_data		= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
//		struct User_Data			*user_data			= INST_DATA(CL_User->mcc_Class		, data->GR_User);
//		struct Modem_Data			*modem_data			= INST_DATA(CL_Modem->mcc_Class		, data->GR_Modem);
//		struct Paths_Data			*paths_data			= INST_DATA(CL_Paths->mcc_Class		, data->GR_Paths);
//		struct Users_Data			*users_data			= INST_DATA(CL_Users->mcc_Class		, data->GR_Users);
//		struct Databases_Data	*datab_data			= INST_DATA(CL_Databases->mcc_Class	, data->GR_Databases);
//		struct Events_Data		*events_data		= INST_DATA(CL_Events->mcc_Class		, data->GR_Events);
//		struct InfoWindow_Data	*infowindow_data	= INST_DATA(CL_InfoWindow->mcc_Class, data->GR_InfoWindow);

		success = TRUE;

// Set the notification between the groups

		DoMethod(data->GR_InfoWindow				, MUIM_Notify, MUIA_Window_CloseRequest, TRUE				, provider_data->CH_ProviderInfo	, 3, MUIM_Set, MUIA_Selected, FALSE);
		DoMethod(provider_data->CH_ProviderInfo, MUIM_Notify, MUIA_Selected				, MUIV_EveryTime	, data->GR_InfoWindow				, 3, MUIM_Set, MUIA_Window_Open, MUIV_TriggerValue);
	}

	return(success);
}

ULONG AmiTCPPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct AmiTCPPrefs_Data tmp;
	static STRPTR ARR_Pages[9];

	ARR_Pages[0] = GetStr(MSG_Pages1);
	ARR_Pages[1] = GetStr(MSG_Pages2);
	ARR_Pages[2] = GetStr(MSG_Pages3);
	ARR_Pages[3] = GetStr(MSG_Pages4);
	ARR_Pages[4] = GetStr(MSG_Pages5);
	ARR_Pages[5] = GetStr(MSG_Pages6);
	ARR_Pages[6] = GetStr(MSG_Pages7);
	ARR_Pages[7] = GetStr(MSG_Pages8);
ARR_Pages[4] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, VERS "  © 1997 by Michael Neuweiler, Active Software",
		MUIA_Window_ID			, MAKE_ID('A','R','E','F'),
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, AmiTCPPrefsMenu,0),
		WindowContents			, VGroup,
			Child, tmp.GR_Pager = HGroup,
				Child, tmp.LV_Pager = ListviewObject,
					MUIA_CycleChain				, 1,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, tmp.LI_Pager = NewObject(CL_PagerList->mcc_Class, NULL,
						MUIA_List_SourceArray	, ARR_Pages,
						MUIA_List_AdjustWidth	, TRUE,
					TAG_DONE),
				End,
				Child, tmp.GR_Active = tmp.GR_Info = VGroup,
					GroupFrame,
					MUIA_Background, MUII_RegisterBack,
					Child, HVSpace,
					Child, HGroup,
						Child, HVSpace,
						Child, BodychunkObject,
							MUIA_FixWidth             , LOGO_WIDTH ,
							MUIA_FixHeight            , LOGO_HEIGHT,
							MUIA_Bitmap_Width         , LOGO_WIDTH ,
							MUIA_Bitmap_Height        , LOGO_HEIGHT,
							MUIA_Bodychunk_Depth      , LOGO_DEPTH ,
							MUIA_Bodychunk_Body       , (UBYTE *)logo_body,
							MUIA_Bodychunk_Compression, LOGO_COMPRESSION,
							MUIA_Bodychunk_Masking    , LOGO_MASKING,
							MUIA_Bitmap_SourceColors  , (ULONG *)logo_colors,
							MUIA_Bitmap_Transparent   , 0,
						End,
						Child, HVSpace,
					End,
					Child, HVSpace,
					Child, CLabel(VERS),
					Child, HVSpace,
#ifdef DEMO
					Child, CLabel(GetStr(MSG_TX_DemoVersion)),
					Child, HVSpace,
#endif
				End,
			End,
			Child, HGroup,
				MUIA_Group_SameSize	, TRUE,
				Child, tmp.BT_Save	= MakeButton(MSG_BT_Save),
				Child, HSpace(0),
				Child, tmp.BT_Use		= MakeButton(MSG_BT_Use),
				Child, HSpace(0),
				Child, tmp.BT_Cancel	= MakeButton(MSG_BT_Cancel),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);

		*data = tmp;

		nnset(tmp.LV_Pager, MUIA_List_Active, MUIV_List_Active_Top);

		set(tmp.LV_Pager	, MUIA_ShortHelp, GetStr(MSG_Help_Pager));
		set(tmp.BT_Save	, MUIA_ShortHelp, GetStr(MSG_Help_Save));
		set(tmp.BT_Use		, MUIA_ShortHelp, GetStr(MSG_Help_Use));
		set(tmp.BT_Cancel	, MUIA_ShortHelp, GetStr(MSG_Help_Cancel));

		DoMethod(obj				, MUIM_Notify, MUIA_Window_CloseRequest, TRUE				, obj, 2, MUIM_AmiTCPPrefs_Finish, 0);
		DoMethod(tmp.LV_Pager	, MUIM_Notify, MUIA_List_Active			, MUIV_EveryTime	, obj, 1, MUIM_AmiTCPPrefs_SetPage);
		DoMethod(tmp.BT_Cancel	, MUIM_Notify, MUIA_Pressed				, FALSE				, obj, 2, MUIM_AmiTCPPrefs_Finish, 0);
		DoMethod(tmp.BT_Use		, MUIM_Notify, MUIA_Pressed				, FALSE				, obj, 2, MUIM_AmiTCPPrefs_Finish, 1);
		DoMethod(tmp.BT_Save		, MUIM_Notify, MUIA_Pressed				, FALSE				, obj, 2, MUIM_AmiTCPPrefs_Finish, 2);

		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_ABOUT)			, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_AmiTCPPrefs_About);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_QUIT)			, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_EXPERT)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_AmiTCPPrefs_Expert);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_IMPORT)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_AmiTCPPrefs_ImportProvider);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_EXPORT)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_AmiTCPPrefs_ExportProvider);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_MUI)			, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_HELP_AMITCP)	, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 5, MUIM_Application_ShowHelp, obj, "NetConnect:Docs/AmiTCP.guide", NULL, NULL);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG AmiTCPPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(AmiTCPPrefs_New			(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_InitGroups		: return(AmiTCPPrefs_InitGroups	(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_SetPage			: return(AmiTCPPrefs_SetPage		(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_Finish			: return(AmiTCPPrefs_Finish		(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_Expert			: return(AmiTCPPrefs_Expert		(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_ImportProvider: return(AmiTCPPrefs_ImportProvider(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_ExportProvider: return(AmiTCPPrefs_ExportProvider(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_LoadProvider	: return(AmiTCPPrefs_LoadProvider(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_LoadPrefs		: return(AmiTCPPrefs_LoadPrefs	(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_About			: return(AmiTCPPrefs_About			(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_About_Finish	: return(AmiTCPPrefs_About_Finish(cl, obj, (APTR)msg));

		case MUIM_Provider_PopList_Update	:
		{
			struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);
		 	return(DoMethodA(data->GR_Provider, msg));
		}
	}
	return(DoSuperMethodA(cl, obj, msg));
}
