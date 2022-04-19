/***********************************************************/
/* you should set the tabulator spacing to 3 for this file */
/***********************************************************/

#include "globals.c"

extern STRPTR GetStr(STRPTR idstr);


ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...)
{
	return(DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}

LONG xget(Object *obj, ULONG attribute)
{
	LONG x;
	get(obj, attribute, &x);
	return(x);
}

Object *create_bodychunk(struct Icon *icon)
{
	Object *bodychunk = NULL;
	struct BitMapHeader *bmhd;
	struct IFFHandle *Handle;
	struct ContextNode *cn;
	struct StoredProperty *sp;
	int size;
	UBYTE *cols = NULL;

	if(Handle=AllocIFF())
	{
		if(Handle->iff_Stream = Open(icon->ImageFile, MODE_OLDFILE))
		{
			InitIFFasDOS(Handle);
			if(!OpenIFF(Handle, IFFF_READ))
			{
				if(!ParseIFF(Handle, IFFPARSE_STEP))
				{
					if((cn = CurrentChunk(Handle)) && (cn->cn_ID == ID_FORM))
					{
						if(cn->cn_Type == ID_ILBM)
						{
							if(!PropChunk(Handle, ID_ILBM, ID_BMHD) &&
								!PropChunk(Handle, ID_ILBM, ID_CMAP) &&
								!StopChunk(Handle, ID_ILBM, ID_BODY) &&
								!StopOnExit(Handle, ID_ILBM, ID_FORM) &&
								!ParseIFF(Handle, IFFPARSE_SCAN))
							{
								if(sp = FindProp(Handle, ID_ILBM, ID_CMAP))
									cols = sp->sp_Data;

								if(sp = FindProp(Handle, ID_ILBM, ID_BMHD))
								{
									bmhd = (struct BitMapHeader *)sp->sp_Data;
									size = CurrentChunk(Handle)->cn_Size;

									if(icon->body = AllocVec(size, MEMF_ANY))
									{
										if(ReadChunkBytes(Handle, icon->body, size) == size)
										{
											bodychunk = BodychunkObject,
												ButtonFrame,
//												MUIA_Bitmap_SourceColors	, cols,
												MUIA_Bitmap_Width				, bmhd->bmh_Width,
												MUIA_Bitmap_Height			, bmhd->bmh_Height,
												MUIA_FixWidth					, bmhd->bmh_Width,
												MUIA_FixHeight					, bmhd->bmh_Height,
												MUIA_Background				, MUII_ButtonBack,
												MUIA_InputMode					, MUIV_InputMode_RelVerify,
												MUIA_Bodychunk_Depth			, bmhd->bmh_Depth,
												MUIA_Bodychunk_Body			, icon->body,
												MUIA_Bodychunk_Compression	, bmhd->bmh_Compression,
												MUIA_Bodychunk_Masking		, bmhd->bmh_Masking,
												MUIA_Bitmap_Transparent		, 0,
												End;
										}
									}
								}
							}
						}
					}
				}
				CloseIFF(Handle);
			}
			Close(Handle->iff_Stream);
		}
		FreeIFF(Handle);
	}
	return(bodychunk);
}

VOID init_icon(struct Icon *icon, Object *list)
{
	icon->body		= NULL;
	icon->list		= NULL;
	if(icon->bodychunk = create_bodychunk(icon))
	{
		if(list)
			icon->list = (APTR)DoMethod(list, MUIM_List_CreateImage, icon->bodychunk, 0);
	}
}



/****************************************************************************/
/* User Prefs class (GROUP)                                                 */
/****************************************************************************/

ULONG UserPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct UserPrefs_Data tmp;

	STR_RA_Connection[0] = GetStr(MSG_RA_Connection0);
	STR_RA_Connection[1] = GetStr(MSG_RA_Connection1);
	STR_RA_Connection[2] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_User",
		Child, HGroup,
			MUIA_HelpNode	, "GR_Connection",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_ConnectionTitle),
			Child, HVSpace,
			Child, tmp.RA_Connection = RadioObject,
				MUIA_HelpNode		, "RA_Connection",
				MUIA_Radio_Entries, STR_RA_Connection,
			End,
			Child, HVSpace,
		End,
		Child, ColGroup(4),
			MUIA_HelpNode	, "GR_UserDetails",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_UserDetailsTitle),
			Child, Label(GetStr(MSG_LA_UserName)),
			Child, tmp.STR_UserName = String("enquiries", 80),
			Child, Label(GetStr(MSG_LA_NodeName)),
			Child, tmp.STR_NodeName = String("active2", 80),
			Child, Label(GetStr(MSG_LA_RealName)),
			Child, tmp.STR_RealName = String("Chris Wiles", 80),
			Child, Label(GetStr(MSG_LA_DomainName)),
			Child, tmp.STR_DomainName = String("demon.co.uk", 80),
			Child, Label(GetStr(MSG_LA_Organisation)),
			Child, tmp.STR_Organisation = String("Active Software", 80),
			Child, Label(GetStr(MSG_LA_Address)),
			Child, tmp.STR_IP_Address = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_HelpNode			, "STR_IP_Address",
				MUIA_String_Contents	, "0.0.0.0",
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
			Child, Label(GetStr(MSG_LA_Password)),
			Child, tmp.STR_Password = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_HelpNode			, "STR_Password",
				MUIA_String_Contents	, "password",
				MUIA_String_Secret	, TRUE,
			End,
			Child, HVSpace,
			Child, HVSpace,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct UserPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG UserPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(UserPrefs_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Server Prefs class (GROUP)                                               */
/****************************************************************************/

ULONG ServerPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct ServerPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Server",
		Child, ColGroup(2),
			MUIA_HelpNode	, "GR_Country",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_CountryTitle),
			Child, Label(GetStr(MSG_LA_Country)),
			Child, tmp.PO_Country = PopobjectObject,
				MUIA_Popstring_String, String("", 80),
				MUIA_Popstring_Button, PopButton(MUII_PopUp),
				MUIA_Popobject_Object, tmp.LV_Country = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, ListObject,
						MUIA_Frame			, MUIV_Frame_InputList,
						MUIA_List_Active	, MUIV_List_Active_Top,
					End,
				End,
			End,
		End,
		Child, ColGroup(2),
			MUIA_HelpNode	, "GR_Provider",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_ProviderTitle),
			Child, Label(GetStr(MSG_LA_Provider)),
			Child, tmp.PO_Provider = PopobjectObject,
				MUIA_Popstring_String, String("", 80),
				MUIA_Popstring_Button, PopButton(MUII_PopUp),
				MUIA_Popobject_Object, tmp.LV_Provider = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, ListObject,
						MUIA_Frame			, MUIV_Frame_InputList,
						MUIA_List_Active	, MUIV_List_Active_Top,
					End,
				End,
			End,
			Child, Label(GetStr(MSG_LA_PoP)),
			Child, tmp.PO_PoP = PopobjectObject,
				MUIA_Popstring_String, String("", 80),
				MUIA_Popstring_Button, PopButton(MUII_PopUp),
				MUIA_Popobject_Object, tmp.LV_PoP = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, ListObject,
						MUIA_Frame			, MUIV_Frame_InputList,
						MUIA_List_Active	, MUIV_List_Active_Top,
					End,
				End,
			End,
			Child, Label(GetStr(MSG_LA_Phone)),
			Child, tmp.STR_Phone = String("01642 33 76 76", 80),
		End,
		Child, ColGroup(4),
			MUIA_HelpNode	, "GR_Details",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_DetailsTitle),
			Child, Label(GetStr(MSG_LA_NameServer1)),
			Child, tmp.STR_NameServer1 = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_String_Contents	, "158.152.1.65",
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
			Child, Label(GetStr(MSG_LA_MailServer)),
			Child, tmp.STR_MailServer = String("mail", 80),
			Child, Label(GetStr(MSG_LA_NameServer2)),
			Child, tmp.STR_NameServer2 = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
			Child, Label(GetStr(MSG_LA_NewsServer)),
			Child, tmp.STR_NewsServer = String("news", 80),
			Child, Label(GetStr(MSG_LA_Netmask)),
			Child, tmp.STR_Netmask = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_String_Contents	, "255.255.255.0",
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
			Child, Label(GetStr(MSG_LA_IRCServer)),
			Child, tmp.STR_IRCServer = String("dismayl", 80),
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct ServerPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG ServerPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(ServerPrefs_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Misc Prefs class (GROUP)                                                 */
/****************************************************************************/

ULONG MiscPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct MiscPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Misc",
		Child, ColGroup(2),
			MUIA_HelpNode	, "GR_Drawers",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_DrawersTitle),
			Child, Label(GetStr(MSG_LA_MailIn)),
			Child, tmp.PA_MailIn = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
			End,
			Child, Label(GetStr(MSG_LA_MailOut)),
			Child, tmp.PA_MailOut = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
			End,
			Child, Label(GetStr(MSG_LA_FilesIn)),
			Child, tmp.PA_FilesIn = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
			End,
			Child, Label(GetStr(MSG_LA_FilesOut)),
			Child, tmp.PA_FilesOut = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
			End,
		End,
		Child, ColGroup(4),
			MUIA_HelpNode	, "GR_Modem",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_ModemTitle),
			Child, Label(GetStr(MSG_LA_SerialDriver)),
			Child, tmp.PA_SerialDriver = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("serial.device", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopFile),
			End,
			Child, Label(GetStr(MSG_LA_ModemInit)),
			Child, tmp.STR_ModemInit = String("ATZ", 80),
			Child, Label(GetStr(MSG_LA_SerialUnit)),
			Child, tmp.STR_SerialUnit = StringObject,
				StringFrame,
				MUIA_String_MaxLen	, 5,
				MUIA_String_Integer	, 0,
				MUIA_String_Accept	, "1234567890",
			End,
			Child, Label(GetStr(MSG_LA_DialString)),
			Child, tmp.STR_DialString = String("ATDT", 80),
			Child, Label(GetStr(MSG_LA_BaudRate)),
			Child, tmp.PO_BaudRate = PopobjectObject,
				MUIA_Popstring_String, StringObject,
					StringFrame,
					MUIA_String_MaxLen	, 5,
					MUIA_String_Integer	, 56700,
					MUIA_String_Accept	, "1234567890",
				End,
				MUIA_Popstring_Button, PopButton(MUII_PopUp),
				MUIA_Popobject_Object, tmp.LV_BaudRate = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, ListObject,
						MUIA_Frame			, MUIV_Frame_InputList,
						MUIA_List_Active	, MUIV_List_Active_Top,
					End,
				End,
			End,
			Child, HVSpace,
			Child, HVSpace,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct MiscPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG MiscPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(MiscPrefs_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* AmiTCP Prefs class                                                       */
/****************************************************************************/

ULONG AmiTCPPrefs_Finish(struct IClass *cl, Object *obj, struct MUIP_AmiTCPPrefs_Finish *msg)
{
//	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
//	struct Button *icon;

	set(obj, MUIA_Window_Open, FALSE);
	set(win, MUIA_Window_Sleep, FALSE);
	DoMethod(app, OM_REMMEMBER, obj);
	MUI_DisposeObject(obj);

	return(NULL);
}

ULONG AmiTCPPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct AmiTCPPrefs_Data tmp;

	STR_GR_Register[0] = GetStr(MSG_GR_Register0);
	STR_GR_Register[1] = GetStr(MSG_GR_Register1);
	STR_GR_Register[2] = GetStr(MSG_GR_Register2);
	STR_GR_Register[3] = GetStr(MSG_GR_Register3);
	STR_GR_Register[4] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title	, GetStr(MSG_WI_AmiTCPPrefs),
		MUIA_Window_ID		, MAKE_ID('A','R','E','F'),
		WindowContents		, VGroup,
			Child, tmp.GR_Register = RegisterObject,
				MUIA_Register_Titles, STR_GR_Register,
				MUIA_HelpNode, "GR_Register",
				Child, tmp.GR_User		= NewObject(CL_UserPrefs->mcc_Class			, NULL, TAG_DONE),
				Child, tmp.GR_Server		= NewObject(CL_ServerPrefs->mcc_Class		, NULL, TAG_DONE),
				Child, tmp.GR_Misc		= NewObject(CL_MiscPrefs->mcc_Class			, NULL, TAG_DONE),
//				Child, tmp.GR_DialScript= NewObject(CL_DialScriptPrefs->mcc_Class	, NULL, TAG_DONE),
			End,
			Child, HGroup,
				MUIA_HelpNode			, "GR_PrefsControl",
				MUIA_Group_SameSize	, TRUE,
				Child, tmp.BT_Save	= SimpleButton(GetStr(MSG_BT_Save)),
				Child, HSpace(0),
				Child, tmp.BT_Use		= SimpleButton(GetStr(MSG_BT_Use)),
				Child, HSpace(0),
				Child, tmp.BT_Cancel	= SimpleButton(GetStr(MSG_BT_Cancel)),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);

		*data = tmp;

		DoMethod(obj				, MUIM_Notify, MUIA_Window_CloseRequest, TRUE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 0);
		DoMethod(tmp.BT_Cancel	, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 0);
		DoMethod(tmp.BT_Use		, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 1);
		DoMethod(tmp.BT_Save		, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 2);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG AmiTCPPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW							: return(AmiTCPPrefs_New		(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_Finish	: return(AmiTCPPrefs_Finish	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Icon List class                                                          */
/****************************************************************************/

/*
 * Decide what to do when MUI tells us an
 * object is beeing dragged into the list.
 */

ULONG IconList_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == obj)										/* If somebody tried to drag ourselves onto ourselves, we let our superclass (the list class) handle the necessary actions.*/
		return(DoSuperMethodA(cl, obj, msg));
	else
	{
		 if(msg->obj == (Object *)muiUserData(obj))	/* If our predefined source object (the other list) wants us to become active, we politely accept it. */
			return(MUIV_DragQuery_Accept);
		else
			return(MUIV_DragQuery_Refuse);				/* Everything else is beeing rejected */
	}
}


/*
 * An object was dropped over the list.
 * Do the necessary actions (move from
 * one list to the other).
 */

ULONG IconList_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == obj)										/* the source is the same as the destination => */
		return(DoSuperMethodA(cl, obj, msg));			/*  MUI shall handle the moving of objects within the list */
	else
	{
		struct Icon *icon;
		LONG dropmark;
		LONG pos, state;

		/* make the lists silent while the entries are beeing moved */
		set(obj		, MUIA_List_Quiet, TRUE);
		set(msg->obj, MUIA_List_Quiet, TRUE);

		/* copy the entries to the dest. by iterating through the source's selected entries */
		pos = MUIV_List_NextSelected_Start;				/* copy all selected entries and the active entry before we delete anything */
		dropmark = xget(obj, MUIA_List_DropMark);		/* find out where to include the entries in the destination */
		FOREVER
		{
			DoMethod(msg->obj, MUIM_List_NextSelected, &pos);
			if(pos == MUIV_List_NextSelected_End)
				break;

			DoMethod(msg->obj, MUIM_List_GetEntry, pos, &icon);
			DoMethod(obj, MUIM_List_InsertSingle, icon, dropmark++);
		}

		/* delete entries in the source */
		pos	= MUIV_List_GetEntry_Active;				/* we have to delete the "active" entry first (because its state is not "selected") */
		state	= 1;												/* because we use the active entry first we have to fake the selection state */
		FOREVER
		{
			DoMethod(msg->obj, MUIM_List_GetEntry, pos, &icon);
			if(!icon)
				break;

			if(state)											/* is the current entry selected */
			{
				icon->bodychunk	= NULL;						/* zero these pointers so they don't get cleared */
				icon->list			= NULL;						/* by PrefsIconList_Desctruct */
				icon->body			= NULL;
				DoMethod(msg->obj, MUIM_List_Remove, pos);
				pos = 0;											/* start over again because we lost an entry */
			}
			else
				pos++;											/* otherwise go to the next entry */

			DoMethod(msg->obj, MUIM_List_Select, pos, MUIV_List_Select_Ask, &state);	/* is it selected ? */
		}

		/* The above 2-way method is necessary because we can't delete an object in the list
		**	while we scan it for selected items. So deletion is done in the second part but
		**	we have to find out which entries are selected in a different way to get the
		**	correct entries deleted => MUIM_List_Select is used but this we we haven't got
		**	the active entry (the last one we clicked on) included so we have to delete the
		**	MUIV_List_GetEntry_Active entry first and then scan the list with
		**	MUIV_List_Select_Ask. A bit complicated but it works and if I were you I wouldn't
		** change it :)
		*/


		/*
		** make the insterted object the active and make the source listviews
		** active object inactive to give some more visual feedback to the user.
		*/
		get(obj, MUIA_List_InsertPosition, &dropmark);
		set(obj, MUIA_List_Active			, dropmark);
		set(msg->obj, MUIA_List_Active	, MUIV_List_Active_Off);

		/* and now make the changes visible */
		set(obj		, MUIA_List_Quiet, FALSE);
		set(msg->obj, MUIA_List_Quiet, FALSE);

		return(NULL);
	}
}


SAVEDS ASM struct Icon *IconList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Icon *src)
{
	struct Icon *new;

	if(new = (struct Icon *)AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
	{
		if(src)
			memcpy(new, src, sizeof(struct Icon));
	}
	return(new);
}

SAVEDS ASM VOID IconList_DestructFunc(REG(a2) APTR pool, REG(a1) struct Icon *icon)
{
	if(icon)
	{
		if(icon->bodychunk)
			MUI_DisposeObject(icon->bodychunk);
		if(icon->body)
			FreeVec(icon->body);
		FreeVec(icon);
	}
}

/*
 * This function tells MUI how to display
 * the list's entries
 */

SAVEDS ASM LONG IconList_DisplayFunc(REG(a0) struct Hook *hook, REG(a2) char **array, REG(a1) struct Icon *icon)
{
	if(icon)
	{
		static char buf[MAXPATHLEN];

		sprintf(buf,"\33O[%08lx]", icon->list);	/* show the image */
		*array++ = buf;
		*array   = icon->Name;							/* show the name */
	}
	return(NULL);
}


ULONG IconList_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
	static const struct Hook IconList_DisplayHook	= { { 0,0 }, (VOID *)IconList_DisplayFunc		, NULL, NULL };
	static const struct Hook IconList_ConstructHook	= { { 0,0 }, (VOID *)IconList_ConstructFunc	, NULL, NULL };
	static const struct Hook IconList_DestructHook	= { { 0,0 }, (VOID *)IconList_DestructFunc	, NULL, NULL };

	obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Frame					, MUIV_Frame_InputList,
		MUIA_List_DisplayHook	, &IconList_DisplayHook,
		MUIA_List_ConstructHook	, &IconList_ConstructHook,
		MUIA_List_DestructHook	, &IconList_DestructHook,
		MUIA_List_Format			, ",",
MUIA_List_MinLineHeight	, 40,
		MUIA_List_AutoVisible	, TRUE,
		MUIA_List_DragSortable	, TRUE,
		TAG_MORE, msg->ops_AttrList);

	return((ULONG)obj);
}

ULONG IconList_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl,obj);
	struct Icon *icon;
	LONG pos;

	if(!DoSuperMethodA(cl, obj, msg))
		return(FALSE);

	/* Free the images which were created in the
	** setup. Zero the variables !! So they're not
	** used twice since there are two lists and
	** CreateImage was issued only on one.
	*/
	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(!icon)
			break;
		if(icon->list)
			DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
		icon->list = NULL;
	}
	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(!icon)
			break;
		if(icon->list)
			DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
		icon->list = NULL;
	}
	return(DoSuperMethodA(cl, obj, msg));
}

SAVEDS ASM ULONG IconList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW				: return(IconList_New  	  	(cl, obj, (APTR)msg));
		case MUIM_Cleanup		: return(IconList_Cleanup	(cl, obj, (APTR)msg));
		case MUIM_DragQuery	: return(IconList_DragQuery(cl, obj, (APTR)msg));
		case MUIM_DragDrop	: return(IconList_DragDrop	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Icon Bar Prefs class                                                     */
/****************************************************************************/

ULONG IconBarPrefs_LoadIcons(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	struct IFFHandle	*Handle;
	struct ContextNode *cn;
	BOOL anything = FALSE;

	set(data->LI_ActiveIcons	, MUIA_List_Quiet, TRUE);
	set(data->LI_InactiveIcons	, MUIA_List_Quiet, TRUE);

	if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
	{
		if(Handle = AllocIFF())
		{
			if(Handle->iff_Stream = Open("ENV:NetConnectPrefs", MODE_OLDFILE))
			{
				InitIFFasDOS(Handle);
				if(!(OpenIFF(Handle, IFFF_READ)))
				{
					if(!(StopChunks(Handle, Stops, NUM_STOPS)))
					{
						while(!ParseIFF(Handle, IFFPARSE_SCAN))
						{
							cn = CurrentChunk(Handle);

							/* is it an active or inactive icon ? */
							if(cn->cn_ID == ID_AICN || cn->cn_ID == ID_IICN)
							{
								if(ReadChunkBytes(Handle, icon, MIN(sizeof(struct Icon), cn->cn_Size)) == MIN(sizeof(struct Icon), cn->cn_Size))
								{
									init_icon(icon, data->LI_ActiveIcons);
									DoMethod((cn->cn_ID == ID_AICN ? data->LI_ActiveIcons : data->LI_InactiveIcons), MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
									anything = TRUE;
								}
							}
						}
					}
					CloseIFF(Handle);
				}
				Close(Handle->iff_Stream);
			}
			FreeIFF(Handle);
		}
		if(!anything)
		{
			/* if there's nothing in both lists create at least the start/stop icon */
			strcpy(icon->Name		, "Start / Stop");
			strcpy(icon->ImageFile, "Images/Start");
			strcpy(icon->Script	, "AmiTCP:bin/startnet");
			icon->Advanced	= TRUE;

			init_icon(icon, data->LI_ActiveIcons);
			DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		}
		FreeVec(icon);
	}
	set(data->LI_ActiveIcons, MUIA_List_Quiet, FALSE);
	set(data->LI_InactiveIcons, MUIA_List_Quiet, FALSE);

	return(NULL);
}

ULONG IconBarPrefs_Reset(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	LONG pos;

	set(data->LI_ActiveIcons	, MUIA_List_Quiet, TRUE);
	set(data->LI_InactiveIcons	, MUIA_List_Quiet, TRUE);

	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(!icon)
			break;
		if(icon->list)
			DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
		icon->list = NULL;
	}
	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(!icon)
			break;
		if(icon->list)
			DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
		icon->list = NULL;
	}
	DoMethod(data->LI_ActiveIcons		, MUIM_List_Clear);
	DoMethod(data->LI_InactiveIcons	, MUIM_List_Clear);

	if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
	{
		strcpy(icon->Name, "WWW");
		strcpy(icon->ImageFile, "Images/WWW");
		strcpy(icon->Program, "IBrowse");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Telnet");
		strcpy(icon->ImageFile, "Images/Telnet");
		strcpy(icon->Program, "Telnet");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "FTP");
		strcpy(icon->ImageFile, "Images/FTP");
		strcpy(icon->Program, "mFTP");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Ping");
		strcpy(icon->ImageFile, "Images/Ping");
		strcpy(icon->Program, "Ping");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Traceroute");
		strcpy(icon->ImageFile, "Images/Traceroute");
		strcpy(icon->Program, "Traceroute");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Finger");
		strcpy(icon->ImageFile, "Images/Finger");
		strcpy(icon->Program, "Finger");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "IRC");
		strcpy(icon->ImageFile, "Images/IRC");
		strcpy(icon->Program, "Irc");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Mail");
		strcpy(icon->ImageFile, "Images/Mail");
		strcpy(icon->Program, "Mail");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "News");
		strcpy(icon->ImageFile, "Images/News");
		strcpy(icon->Program, "News");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Search");
		strcpy(icon->ImageFile, "Images/Search");
		strcpy(icon->Program, "Search");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Docs");
		strcpy(icon->ImageFile, "Images/Docs");
		strcpy(icon->Program, "Docs");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Misc");
		strcpy(icon->ImageFile, "Images/Misc");
		strcpy(icon->Program, "Misc");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Start / Stop");
		strcpy(icon->ImageFile, "Images/Start");
		strcpy(icon->Script	, "AmiTCP:bin/startnet");
		*icon->Program	= NULL;
		icon->Advanced	= TRUE;
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Top);

		FreeVec(icon);
	}

	set(data->LI_ActiveIcons	, MUIA_List_Quiet, FALSE);
	set(data->LI_InactiveIcons	, MUIA_List_Quiet, FALSE);

	return(NULL);
}

ULONG IconBarPrefs_NewIcon(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);

	DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
	set(data->LI_InactiveIcons, MUIA_List_Active, xget(data->LI_InactiveIcons, MUIA_List_InsertPosition));
	set(data->LI_ActiveIcons, MUIA_List_Active, MUIV_List_Active_Off);

	return(NULL);
}

BOOL find_list(struct IconBarPrefs_Data *data, Object **list, struct Icon **icon)
{
	struct Icon *tmp_icon;

	DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &tmp_icon);
	if(tmp_icon)
	{
		*icon = tmp_icon;
		*list = data->LI_ActiveIcons;
		return(TRUE);
	}
	else
	{
		DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &tmp_icon);
		if(tmp_icon)
		{
			*icon = tmp_icon;
			*list = data->LI_InactiveIcons;
			return(TRUE);
		}
	}
	return(FALSE);
}

ULONG IconBarPrefs_ModifyIcon(struct IClass *cl, Object *obj, struct MUIP_IconBarPrefs_ModifyIcon *msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	Object *list;

	if(find_list(data, &list, &icon))
	{
		switch(msg->flag)
		{
			case MUIV_IconBarPrefs_ModifyIcon_Remove:
				if(icon->list)
					DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
				icon->list = NULL;
				DoMethod(list, MUIM_List_Remove, MUIV_List_Remove_Active);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Name:
				strcpy(icon->Name, (STRPTR)xget(data->PO_Name, MUIA_String_Contents));
				DoMethod(list, MUIM_List_Redraw, MUIV_List_Redraw_Active);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Program:
				strcpy(icon->Program, (STRPTR)xget(data->PA_Program, MUIA_String_Contents));
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Image:
				{
					struct Icon *new_icon;
					LONG position = MUIV_List_Insert_Active;

					if(xget(list, MUIA_List_Entries))
					{
						DoMethod(list, MUIM_List_GetEntry, xget(list, MUIA_List_Entries) - 1, &new_icon);
						if(new_icon == icon)
							position = MUIV_List_Insert_Bottom;
					}

					if(new_icon = AllocVec(sizeof(struct Icon), MEMF_ANY))
					{
						nnset(list, MUIA_List_Quiet, TRUE);
						memcpy(new_icon, icon, sizeof(struct Icon));
						strcpy(new_icon->ImageFile, (STRPTR)xget(data->PA_Image, MUIA_String_Contents));

						if(icon->list)
							DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
						icon->list = NULL;
						DoMethod(list, MUIM_List_Remove, MUIV_List_Remove_Active);

						init_icon(new_icon, data->LI_ActiveIcons);
						DoMethod(list, MUIM_List_InsertSingle, new_icon, position);
						set(list, MUIA_List_Active, xget(list, MUIA_List_InsertPosition));
						nnset(list, MUIA_List_Quiet, FALSE);

						FreeVec(new_icon);
					}
				}
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Sound:
				strcpy(icon->Sound, (STRPTR)xget(data->PA_Sound, MUIA_String_Contents));
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Volume:
				icon->Volume = xget(data->SL_Volume, MUIA_Numeric_Value);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Advanced:
				icon->Advanced = xget(data->CH_Advanced, MUIA_Selected);
				set(data->GR_Script, MUIA_Disabled, (icon->Advanced ? FALSE : TRUE));
				set(data->PA_Program, MUIA_Disabled, (icon->Advanced ? TRUE : FALSE));
				break;
		}
	}
	return(NULL);
}

ULONG IconBarPrefs_SetStates(struct IClass *cl, Object *obj, struct MUIP_IconBarPrefs_SetStates *msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	LONG i;

	nnset((msg->level ? data->LI_InactiveIcons : data->LI_ActiveIcons), MUIA_List_Active, MUIV_List_Active_Off);
	DoMethod((msg->level ? data->LI_InactiveIcons : data->LI_ActiveIcons), MUIM_List_Select, MUIV_List_Select_All, MUIV_List_Select_Ask, &i);
	if(i)
		DoMethod((msg->level ? data->LI_InactiveIcons : data->LI_ActiveIcons), MUIM_List_Select, MUIV_List_Select_All, MUIV_List_Select_Off, NULL);

	DoMethod((msg->level ? data->LI_ActiveIcons : data->LI_InactiveIcons), MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &icon);
	if(icon)
	{
		set(data->BT_Remove, MUIA_Disabled, FALSE);
		set(data->GR_Button, MUIA_Disabled, FALSE);
		set(data->GR_Script, MUIA_Disabled, (icon->Advanced ? FALSE : TRUE));
		set(data->PA_Program, MUIA_Disabled, (icon->Advanced ? TRUE : FALSE));

		setstring(data->PO_Name			, icon->Name);
		setstring(data->PA_Program		, icon->Program);
		setstring(data->PA_Image		, icon->ImageFile);
		setstring(data->PA_Sound		, icon->Sound);
		setslider(data->SL_Volume		, icon->Volume);
		setcheckmark(data->CH_Advanced, icon->Advanced);
		setstring(data->PA_SaveScript	, icon->Script);
	}
	else
	{
		set(data->BT_Remove, MUIA_Disabled, TRUE);
		set(data->GR_Script, MUIA_Disabled, TRUE);
		set(data->GR_Button, MUIA_Disabled, TRUE);

		setstring(data->PO_Name		, "");
		setstring(data->PA_Program	, "");
		setstring(data->PA_Image	, "");
		setstring(data->PA_Sound	, "");
		setslider(data->SL_Volume	, 0);
		setcheckmark(data->CH_Advanced, FALSE);
		setstring(data->PA_SaveScript	, "");
	}
	return(NULL);
}

ULONG IconBarPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct IconBarPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, GetStr(MSG_WI_IconBarPrefs),
		MUIA_Window_ID			, MAKE_ID('I','P','R','F'),
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, IconBarPrefsMenu, 0),
		WindowContents		, VGroup,
			MUIA_HelpNode, "WI_IconBarPrefs",
			MUIA_Frame, MUIV_Frame_Group,
			Child, HGroup,
				MUIA_HelpNode, "GR_IconBarPrefs_top",
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_InactiveIcons = ListviewObject,
						MUIA_FrameTitle				, GetStr(MSG_LV_ButtonBankTitle),
						MUIA_Listview_MultiSelect	, MUIV_Listview_MultiSelect_Default,
//						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_InactiveIcons = NewObject(CL_IconList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_New		= SimpleButton(GetStr(MSG_BT_New)),
						Child, tmp.BT_Remove = SimpleButton(GetStr(MSG_BT_Remove)),
					End,
				End,
				Child, tmp.LV_ActiveIcons = ListviewObject,
					MUIA_FrameTitle				, GetStr(MSG_LV_IconBarTitle),
					MUIA_Listview_MultiSelect	, MUIV_Listview_MultiSelect_Default,
//					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_DragType		, 1,
					MUIA_Listview_List			, tmp.LI_ActiveIcons = NewObject(CL_IconList->mcc_Class, NULL, TAG_DONE),
				End,
			End,
			Child, HGroup,
				MUIA_HelpNode, "GR_IconBarPrefs_bottom",
				Child, tmp.GR_Button = VGroup,
					MUIA_HelpNode	, "GR_IconConrtol",
					MUIA_Frame		, MUIV_Frame_Group,
					MUIA_FrameTitle, GetStr(MSG_GR_ButtonConrtolTitle),
					Child, ColGroup(2),
						Child, Label("Columns :"),
						Child, tmp.SL_Columns = Slider(1, 10, 1),
						Child, Label(GetStr(MSG_LA_Name)),
						Child, tmp.PO_Name = PopobjectObject,
							MUIA_Popstring_String, String("", 80),
							MUIA_Popstring_Button, PopButton(MUII_PopUp),
							MUIA_Popobject_Object, tmp.LV_Name = ListviewObject,
								MUIA_Listview_DoubleClick	, TRUE,
								MUIA_Listview_List			, ListObject,
									MUIA_Frame			, MUIV_Frame_InputList,
									MUIA_List_Active	, MUIV_List_Active_Top,
								End,
							End,
						End,
						Child, Label(GetStr(MSG_LA_Program)),
						Child, tmp.PA_Program = PopaslObject,
							MUIA_Popasl_Type		, 0,
							MUIA_Popstring_String, String("", MAXPATHLEN),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
						End,
						Child, Label(GetStr(MSG_LA_Image)),
						Child, tmp.PA_Image = PopaslObject,
							MUIA_Popasl_Type		, 0,
							MUIA_Popstring_String, String("", MAXPATHLEN),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
						End,
						Child, Label(GetStr(MSG_LA_Sound)),
						Child, tmp.PA_Sound = PopaslObject,
							MUIA_Popasl_Type		, 0,
							MUIA_Popstring_String, String("", MAXPATHLEN),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
						End,
					End,
					Child, HGroup,
						Child, ColGroup(2),
							Child, HVSpace,
							Child, HVSpace,
							Child, tmp.CH_Advanced = CheckMark(FALSE),
							Child, LLabel1(GetStr(MSG_LA_Advanced)),
							Child, HVSpace,
							Child, HVSpace,
						End,
						Child, HVSpace,
						Child, VGroup,
							Child, tmp.SL_Volume = KnobObject,
								MUIA_Numeric_Min		, 0,
								MUIA_Numeric_Max		, 64,
								MUIA_Numeric_Value	, 64,
								MUIA_Numeric_Format	, "%3ld",
							End,
							Child, TextObject,
								MUIA_Font, MUIV_Font_Tiny,
								MUIA_Text_Contents, GetStr(MSG_LA_Volume),
								MUIA_Text_PreParse, "\33c",
							End,
						End,
					End,
				End,
				Child, tmp.GR_Script = VGroup,
					MUIA_HelpNode	, "GR_ScriptEdit",
					MUIA_Frame		, MUIV_Frame_Group,
					MUIA_FrameTitle, GetStr(MSG_GR_ScriptEditTitle),
					Child, tmp.GR_Editor = ScrollgroupObject,
						MUIA_Scrollgroup_Contents, tmp.TF_Editor = HVSpace,
					End,
					Child, ColGroup(2),
						Child, Label(GetStr(MSG_LA_FindProgram)),
						Child, HGroup,
							MUIA_Group_HorizSpacing, 0,
							Child, tmp.PA_FindProgram = PopaslObject,
								MUIA_Popasl_Type		, 0,
								MUIA_Popstring_String, String("", MAXPATHLEN),
								MUIA_Popstring_Button, PopButton(MUII_PopFile),
							End,
							Child, tmp.BT_Add = SimpleButton(GetStr(MSG_BT_Add)),
						End,
						Child, Label(GetStr(MSG_LA_SaveScript)),
						Child, tmp.PA_SaveScript = PopaslObject,
							MUIA_Popasl_Type		, 0,
							MUIA_Popstring_String, String("", MAXPATHLEN),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
						End,
					End,
				End,
			End,
			Child, HGroup,
				MUIA_HelpNode			, "GR_IconBarPrefsControl",
				MUIA_Group_SameSize	, TRUE,
				Child, tmp.BT_Save	= SimpleButton(GetStr(MSG_BT_Save)),
				Child, HSpace(0),
				Child, tmp.BT_Use		= SimpleButton(GetStr(MSG_BT_Use)),
				Child, HSpace(0),
				Child, tmp.BT_Cancel	= SimpleButton(GetStr(MSG_BT_Cancel)),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct IconBarPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.BT_Add, MUIA_Weight, 1);

		set(tmp.BT_Remove, MUIA_Disabled, TRUE);
		set(tmp.GR_Script, MUIA_Disabled, TRUE);
		set(tmp.GR_Button, MUIA_Disabled, TRUE);

		set(tmp.LI_InactiveIcons, MUIA_UserData, tmp.LI_ActiveIcons);
		set(tmp.LI_ActiveIcons, MUIA_UserData, tmp.LI_InactiveIcons);

		DoMethod(tmp.LV_ActiveIcons	, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime			, obj, 2, MUIM_IconBarPrefs_SetStates, 1);
		DoMethod(tmp.LV_InactiveIcons	, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime			, obj, 2, MUIM_IconBarPrefs_SetStates, 0);
		DoMethod(tmp.BT_New				, MUIM_Notify, MUIA_Pressed				, FALSE			, obj, 1, MUIM_IconBarPrefs_NewIcon);
		DoMethod(tmp.BT_Remove			, MUIM_Notify, MUIA_Pressed				, FALSE			, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Remove);
		DoMethod(tmp.PO_Name				, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Name);
		DoMethod(tmp.PA_Program			, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Program);
		DoMethod(tmp.PA_Image			, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Image);
		DoMethod(tmp.PA_Sound			, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Sound);
		DoMethod(tmp.SL_Volume			, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime		, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Volume);
		DoMethod(tmp.CH_Advanced		, MUIM_Notify, MUIA_Selected, MUIV_EveryTime				, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Advanced);

		DoMethod(obj						, MUIM_Notify, MUIA_Window_CloseRequest, TRUE	, win, 2, MUIM_IconBar_IconBarPrefs_Finish, 0);
		DoMethod(tmp.BT_Cancel			, MUIM_Notify, MUIA_Pressed				, FALSE	, win, 2, MUIM_IconBar_IconBarPrefs_Finish, 0);
		DoMethod(tmp.BT_Use				, MUIM_Notify, MUIA_Pressed				, FALSE	, win, 2, MUIM_IconBar_IconBarPrefs_Finish, 1);
		DoMethod(tmp.BT_Save				, MUIM_Notify, MUIA_Pressed				, FALSE	, win, 2, MUIM_IconBar_IconBarPrefs_Finish, 2);

		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_RESET)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 2, MUIM_IconBarPrefs_Reset, 0);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG IconBarPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(IconBarPrefs_New			(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_LoadIcons		: return(IconBarPrefs_LoadIcons	(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_Reset			: return(IconBarPrefs_Reset		(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_NewIcon		: return(IconBarPrefs_NewIcon		(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_ModifyIcon	: return(IconBarPrefs_ModifyIcon	(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_SetStates		: return(IconBarPrefs_SetStates	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Button class                                                             */
/****************************************************************************/

ULONG Button_Dispose(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct Button_Data *data = INST_DATA(cl, obj);

	if(data->icon->body)					/* this one gets allocated in create_bodychunk */
		FreeVec(data->icon->body);

	if(data->icon)
		FreeVec(data->icon);

	return(DoSuperMethodA(cl, obj, msg));
}

ULONG Button_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct Button_Data tmp;
	struct Icon *icon;
	struct BitMapHeader *bmhd;
	struct IFFHandle *Handle;
	struct ContextNode *cn;
	struct StoredProperty *sp;
	int size;
	UBYTE *cols = NULL;

	if(icon = (struct Icon *)GetTagData(MUIA_NetConnect_Icon, (ULONG)"", msg->ops_AttrList))
	{
		if(Handle=AllocIFF())
		{
			if(Handle->iff_Stream = Open(icon->ImageFile, MODE_OLDFILE))
			{
				InitIFFasDOS(Handle);
				if(!OpenIFF(Handle, IFFF_READ))
				{
					if(!ParseIFF(Handle, IFFPARSE_STEP))
					{
						if((cn = CurrentChunk(Handle)) && (cn->cn_ID == ID_FORM))
						{
							if(cn->cn_Type == ID_ILBM)
							{
								if(!PropChunk(Handle, ID_ILBM, ID_BMHD) &&
									!PropChunk(Handle, ID_ILBM, ID_CMAP) &&
									!StopChunk(Handle, ID_ILBM, ID_BODY) &&
									!StopOnExit(Handle, ID_ILBM, ID_FORM) &&
									!ParseIFF(Handle, IFFPARSE_SCAN))
								{
									if(sp = FindProp(Handle, ID_ILBM, ID_CMAP))
										cols = sp->sp_Data;

									if(sp = FindProp(Handle, ID_ILBM, ID_BMHD))
									{
										bmhd = (struct BitMapHeader *)sp->sp_Data;
										size = CurrentChunk(Handle)->cn_Size;

										if(icon->body = AllocVec(size, MEMF_ANY))
										{
											if(ReadChunkBytes(Handle, icon->body, size) == size)
											{
												if(obj = (Object *)DoSuperNew(cl, obj,
													ButtonFrame,
//													MUIA_Bitmap_SourceColors	, cols,
													MUIA_Bitmap_Width				, bmhd->bmh_Width,
													MUIA_Bitmap_Height			, bmhd->bmh_Height,
													MUIA_FixWidth					, bmhd->bmh_Width,
													MUIA_FixHeight					, bmhd->bmh_Height,
													MUIA_Background				, MUII_ButtonBack,
													MUIA_InputMode					, MUIV_InputMode_RelVerify,
													MUIA_Bodychunk_Depth			, bmhd->bmh_Depth,
													MUIA_Bodychunk_Body			, icon->body,
													MUIA_Bodychunk_Compression	, bmhd->bmh_Compression,
													MUIA_Bodychunk_Masking		, bmhd->bmh_Masking,
													MUIA_Bitmap_Transparent		, 0,
													TAG_MORE, msg->ops_AttrList))
												{
													struct Button_Data *data = INST_DATA(cl, obj);
													*data = tmp;

													if(data->icon = AllocVec(sizeof(struct Icon), MEMF_ANY))
													{
														memcpy(data->icon, icon, sizeof(struct Icon));
													}
												}
											}
											else
												obj = NULL;
										}
										else
											obj = NULL;
									}
									else
										obj = NULL;
								}
								else
									obj = NULL;
							}
							else
								obj = NULL;
						}
						else
							obj = NULL;
					}
					else
						obj = NULL;
					CloseIFF(Handle);
				}
				else
					obj = NULL;
				Close(Handle->iff_Stream);
			}
			else
				obj = NULL;
			FreeIFF(Handle);
		}
		else
			obj = NULL;
	}
	else
		obj = NULL;

	return((ULONG)obj);
}

SAVEDS ASM ULONG Button_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(Button_New				(cl, obj, (APTR)msg));
		case OM_DISPOSE							: return(Button_Dispose			(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}


/****************************************************************************/
/* IconBar class                                                            */
/****************************************************************************/

ULONG IconBar_LoadButtons(struct IClass *cl, Object *obj, struct MUIP_IconBar_LoadButtons *msg)
{
	struct IconBar_Data *data = INST_DATA(cl,obj);
	Object *button, *group, *root;
	struct Icon *icon;
	BOOL anything = FALSE;
	struct IFFHandle *Handle;
	struct ContextNode *cn;

	if(group = ColGroup(3), End)
	{
		DoMethod(group, MUIM_Group_InitChange);
		if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY))
		{
			if(Handle = AllocIFF())
			{
				if(Handle->iff_Stream = Open("ENV:NetConnectPrefs", MODE_OLDFILE))
				{
					InitIFFasDOS(Handle);
					if(!(OpenIFF(Handle, IFFF_READ)))
					{
						if(!(StopChunks(Handle, Stops, NUM_STOPS)))
						{
							while(!ParseIFF(Handle, IFFPARSE_SCAN))
							{
								cn = CurrentChunk(Handle);

								/* is it an active icon ? */
								if(cn->cn_ID == ID_AICN)
								{
									/* load the data into the buffer */
									if(ReadChunkBytes(Handle, icon, MIN(sizeof(struct Icon), cn->cn_Size)) == MIN(sizeof(struct Icon), cn->cn_Size))
									{
										if(button = NewObject(CL_Button->mcc_Class, NULL,
											MUIA_NetConnect_Icon, icon,
											TAG_DONE))
										{
											DoMethod(group, OM_ADDMEMBER, button);
											anything = TRUE;
										}
									}
								}
							}
						}
						CloseIFF(Handle);
					}
					Close(Handle->iff_Stream);
				}
				FreeIFF(Handle);
			}
			if(!anything)
			{
				/* there's nothing in both lists => create at least the start/stop icon */
				strcpy(icon->Name		, "Start / Stop");
				strcpy(icon->ImageFile, "Images/Start");
				strcpy(icon->Script	, "AmiTCP:bin/startnet");
				icon->Advanced	= TRUE;

				if(button = NewObject(CL_Button->mcc_Class, NULL,
					MUIA_NetConnect_Icon, icon,
					TAG_DONE))
				{
					DoMethod(group, OM_ADDMEMBER, button);
				}
			}
			FreeVec(icon);
		}
		DoMethod(group, MUIM_Group_ExitChange);
		if(root = (Object *)xget(win, MUIA_Window_RootObject))
		{
			DoMethod(root, MUIM_Group_InitChange);

			DoMethod(root, OM_REMMEMBER, data->GR_Buttons);
			MUI_DisposeObject(data->GR_Buttons);
			data->GR_Buttons = group;
			DoMethod(root, OM_ADDMEMBER, group);

			DoMethod(root, MUIM_Group_ExitChange);
		}
	}
	return(NULL);
}


ULONG IconBar_IconBarPrefs(struct IClass *cl, Object *obj, Msg msg)
{
	Object *window;

	set(app, MUIA_Application_Sleep, FALSE);
	if(window = (Object *)NewObject(CL_IconBarPrefs->mcc_Class, NULL, TAG_DONE))
	{
		DoMethod(app, OM_ADDMEMBER, window);
		set(window, MUIA_Window_Open, TRUE);
	}
	set(app, MUIA_Application_Sleep, FALSE);
	set(win, MUIA_Window_Sleep, TRUE);
	DoMethod(window, MUIM_IconBarPrefs_LoadIcons);

	return(NULL);
}

ULONG IconBar_IconBarPrefs_Finish(struct IClass *cl, Object *obj, struct MUIP_IconBar_IconBarPrefs_Finish *msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	struct IFFHandle	*Handle;
	LONG i;

	i = msg->level;
	while(i > 0)
	{
		if(Handle = AllocIFF())
		{
			if(Handle->iff_Stream = Open((i == 2 ? "ENVARC:NetConnectPrefs" : "ENV:NetConnectPrefs"), MODE_NEWFILE))
			{
				InitIFFasDOS(Handle);
				if(!(OpenIFF(Handle, IFFF_WRITE)))
				{
					if(!(PushChunk(Handle, ID_NTCN, ID_FORM, IFFSIZE_UNKNOWN)))
					{
						LONG pos = 0;
						FOREVER
						{
							DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, pos++, &icon);
							if(!icon)
								break;
							if(PushChunk(Handle, ID_NTCN, ID_AICN, IFFSIZE_UNKNOWN))
								break;
							if(WriteChunkBytes(Handle, icon, sizeof(struct Icon)) != sizeof(struct Icon))
								break;
							if(PopChunk(Handle))
								break;
						}
						pos = 0;
						FOREVER
						{
							DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, pos++, &icon);
							if(!icon)
								break;
							if(PushChunk(Handle, ID_NTCN, ID_IICN, IFFSIZE_UNKNOWN))
								break;
							if(WriteChunkBytes(Handle, icon, sizeof(struct Icon)) != sizeof(struct Icon))
								break;
							if(PopChunk(Handle))
								break;
						}
						PopChunk(Handle);
					}
					CloseIFF(Handle);
				}
				Close(Handle->iff_Stream);
			}
			FreeIFF(Handle);
		}
		i--;
	}

	if(msg->level)
		DoMethod(win, MUIM_IconBar_LoadButtons);

	set(obj, MUIA_Window_Open, FALSE);
	set(win, MUIA_Window_Sleep, FALSE);
	DoMethod(app, OM_REMMEMBER, obj);
	MUI_DisposeObject(obj);

	return(NULL);
}

ULONG IconBar_AmiTCPPrefs(struct IClass *cl, Object *obj, Msg msg)
{
	Object *window;

	set(app, MUIA_Application_Sleep, FALSE);
	if(window = (Object *)NewObject(CL_AmiTCPPrefs->mcc_Class, NULL, TAG_DONE))
	{
		DoMethod(app, OM_ADDMEMBER, window);
		set(window, MUIA_Window_Open, TRUE);
	}
	set(app, MUIA_Application_Sleep, FALSE);
	set(win, MUIA_Window_Sleep, TRUE);

	return(NULL);
}

ULONG IconBar_Finish(struct IClass *cl, Object *obj, struct MUIP_IconBar_Finish *msg)
{
	DoMethod((Object *)xget(obj, MUIA_ApplicationObject), MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
	return(NULL);
}

ULONG IconBar_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct IconBar_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, GetStr(MSG_WI_IconBar),
		MUIA_Window_ID			, MAKE_ID('I','C','O','N'),
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, IconBarMenu,0),
		WindowContents			, VGroup,
			Child, tmp.GR_Buttons = HGroup,
//tmp.GR_Buttons = ColGroup(xget(cp_data->SL_Columns, MUIA_Numeric_Value)),
				MUIA_Group_Spacing, 0,
				MUIA_InnerBottom	, 0,
				MUIA_InnerLeft		, 0,
				MUIA_InnerTop		, 0,
				MUIA_InnerRight	, 0,
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct IconBar_Data *data = INST_DATA(cl,obj);

		*data = tmp;


		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2, MUIM_IconBar_Finish, 0);

		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_QUIT)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 2, MUIM_IconBar_Finish, 0);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_ICONBAR)	, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 2, MUIM_IconBar_IconBarPrefs);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_AMITCP)	, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 2, MUIM_IconBar_AmiTCPPrefs);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_MUI)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG IconBar_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(IconBar_New						(cl, obj, (APTR)msg));
		case MUIM_IconBar_Finish					: return(IconBar_Finish					(cl, obj, (APTR)msg));
		case MUIM_IconBar_LoadButtons				: return(IconBar_LoadButtons			(cl, obj, (APTR)msg));
		case MUIM_IconBar_IconBarPrefs			: return(IconBar_IconBarPrefs			(cl, obj, (APTR)msg));
		case MUIM_IconBar_IconBarPrefs_Finish	: return(IconBar_IconBarPrefs_Finish(cl, obj, (APTR)msg));
		case MUIM_IconBar_AmiTCPPrefs				: return(IconBar_AmiTCPPrefs			(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}






/*
	GR_ScriptEditor = VirtgroupObject,
		VirtualFrame,
		MUIA_HelpNode, "GR_ScriptEditor",
		MUIA_Background, MUII_BACKGROUND,
		MUIA_Frame, MUIV_Frame_InputList,
	End;

	GR_ScriptEditor = ScrollgroupObject,
		MUIA_Scrollgroup_Contents, GR_ScriptEditor,
	End;

	GR_DialScript = GroupObject,
		MUIA_HelpNode, "GR_DialScript",
		Child, GR_ScriptEditor,
	End;
*/




/*
 * close our custom classes
 */

VOID exit_classes(VOID)
{
	if(CL_IconBarPrefs)	MUI_DeleteCustomClass(CL_IconBarPrefs);
	if(CL_IconList)		MUI_DeleteCustomClass(CL_IconList);
	if(CL_Button)			MUI_DeleteCustomClass(CL_Button);
	if(CL_IconBar)			MUI_DeleteCustomClass(CL_IconBar);

	if(CL_AmiTCPPrefs)	MUI_DeleteCustomClass(CL_AmiTCPPrefs);
	if(CL_UserPrefs)		MUI_DeleteCustomClass(CL_UserPrefs);
	if(CL_ServerPrefs)	MUI_DeleteCustomClass(CL_ServerPrefs);
	if(CL_MiscPrefs)		MUI_DeleteCustomClass(CL_MiscPrefs);

	CL_IconBarPrefs	= CL_IconList		= CL_IconBar		=
	CL_AmiTCPPrefs		= CL_UserPrefs		= CL_ServerPrefs	=
	CL_MiscPrefs		= CL_Button					= NULL;
}


/*
 * initialize our custom classes
 */

BOOL init_classes(VOID)
{
	CL_IconBarPrefs	= MUI_CreateCustomClass(NULL, MUIC_Window		, NULL, sizeof(struct IconBarPrefs_Data)	, IconBarPrefs_Dispatcher);
	CL_IconList			= MUI_CreateCustomClass(NULL, MUIC_List		, NULL, sizeof(struct IconList_Data)		, IconList_Dispatcher);
	CL_Button			= MUI_CreateCustomClass(NULL, MUIC_Bodychunk	, NULL, sizeof(struct Button_Data)			, Button_Dispatcher);
	CL_IconBar			= MUI_CreateCustomClass(NULL, MUIC_Window		, NULL, sizeof(struct IconBar_Data)			, IconBar_Dispatcher);

	CL_AmiTCPPrefs		= MUI_CreateCustomClass(NULL, MUIC_Window		, NULL, sizeof(struct AmiTCPPrefs_Data)	, AmiTCPPrefs_Dispatcher);
	CL_UserPrefs		= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct UserPrefs_Data)		, UserPrefs_Dispatcher);
	CL_ServerPrefs		= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct ServerPrefs_Data)	, ServerPrefs_Dispatcher);
	CL_MiscPrefs		= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct MiscPrefs_Data)		, MiscPrefs_Dispatcher);

	if(CL_IconBarPrefs	&& CL_IconBar		&& CL_IconList 	&& CL_Button &&
		CL_AmiTCPPrefs		&& CL_UserPrefs	&& CL_ServerPrefs	&& CL_MiscPrefs)
		return(TRUE);

	exit_classes();
	return(FALSE);
}
