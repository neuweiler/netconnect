#include "globals.c"
#include "protos.h"

/****************************************************************************/
/* Member List class (LIST)                                                 */
/****************************************************************************/

ULONG MemberList_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == obj)
		return(DoSuperMethodA(cl, obj, msg));
	else
	{
		 if(msg->obj == (Object *)muiUserData(obj))
			return(MUIV_DragQuery_Accept);
		else
			return(MUIV_DragQuery_Refuse);
	}
}

ULONG MemberList_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	struct MemberList_Data *data = INST_DATA(cl, obj);
	ULONG ret;

	if(msg->obj == obj)
		ret = DoSuperMethodA(cl, obj, msg);
	else
	{
		struct User *user;

		DoMethod(msg->obj, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
		DoMethod(obj, MUIM_List_InsertSingle, user->Login, xget(obj, MUIA_List_DropMark));
		set(obj, MUIA_List_Active, xget(obj, MUIA_List_InsertPosition));
		ret = NULL;
	}
	if(data->Originator)
		DoMethod(data->Originator, MUIM_Users_GetGroupStates);

	return(ret);
}

ULONG MemberList_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Frame					, MUIV_Frame_InputList,
		MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
		MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
		MUIA_List_DragSortable	, TRUE,
		TAG_MORE, msg->ops_AttrList);

	return((ULONG)obj);
}

SAVEDS ASM ULONG MemberList_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW				: return(MemberList_New			(cl, obj, (APTR)msg));
		case MUIM_DragQuery	: return(MemberList_DragQuery	(cl, obj, (APTR)msg));
		case MUIM_DragDrop	: return(MemberList_DragDrop	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}




/****************************************************************************/
/* GroupID String class (STRING)                                            */
/****************************************************************************/

ULONG GroupIDString_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == (Object *)muiUserData(obj))
		return(MUIV_DragQuery_Accept);

	return(MUIV_DragQuery_Refuse);
}

ULONG GroupIDString_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	struct Group *group;

	DoMethod(msg->obj, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &group);
	set(obj, MUIA_String_Integer, group->ID);

	return(NULL);
}

ULONG GroupIDString_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	obj = (Object *)DoSuperNew(cl, obj,
		StringFrame,
		MUIA_String_Integer	, 0,
		MUIA_String_Accept	, "1234567890",
		MUIA_CycleChain, 1,
		TAG_MORE, msg->ops_AttrList);

	return((ULONG)obj);
}

SAVEDS ASM ULONG GroupIDString_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW				: return(GroupIDString_New			(cl, obj, (APTR)msg));
		case MUIM_DragQuery	: return(GroupIDString_DragQuery	(cl, obj, (APTR)msg));
		case MUIM_DragDrop	: return(GroupIDString_DragDrop	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}


/****************************************************************************/
/* Users class (GROUP)                                                      */
/****************************************************************************/

SAVEDS ASM struct User *UserList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct User *src)
{
	struct User *new;

	if((new = (struct User *)AllocVec(sizeof(struct User), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct User));
	return(new);
}

ULONG Users_SetUserStates(struct IClass *cl, Object *obj, Msg msg)
{
	struct Users_Data *data = INST_DATA(cl, obj);
	struct User *user;

	DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
	if(user)
	{
		set(data->BT_ChangePassword, MUIA_Disabled, user->Disabled);
		set(data->BT_RemovePassword, MUIA_Disabled, !*user->Password || user->Disabled);

		nnset(data->STR_User		, MUIA_String_Contents	, user->Login);
		nnset(data->STR_Name		, MUIA_String_Contents	, user->Name);
		nnset(data->PA_HomeDir	, MUIA_String_Contents	, user->HomeDir);
		nnset(data->STR_Shell	, MUIA_String_Contents	, user->Shell);
		set(data->CH_Disabled	, MUIA_Selected			, user->Disabled);
		nnset(data->STR_UserID	, MUIA_String_Integer	, user->UserID);
		nnset(data->STR_GroupID	, MUIA_String_Integer	, user->GroupID);
	}
	else
	{
		set(data->BT_ChangePassword, MUIA_Disabled, TRUE);
		set(data->BT_RemovePassword, MUIA_Disabled, TRUE);

		nnset(data->STR_User		, MUIA_String_Contents	, "");
		nnset(data->STR_Name		, MUIA_String_Contents	, "");
		nnset(data->PA_HomeDir	, MUIA_String_Contents	, "");
		nnset(data->STR_Shell	, MUIA_String_Contents	, "");
		nnset(data->CH_Disabled	, MUIA_Selected			, FALSE);
		nnset(data->STR_UserID	, MUIA_String_Integer	, 0);
		nnset(data->STR_GroupID	, MUIA_String_Integer	, 0);
	}
	set(data->BT_RemoveUser	, MUIA_Disabled, !user);
	set(data->STR_User		, MUIA_Disabled, !user);
	set(data->STR_Name		, MUIA_Disabled, !user);
	set(data->PA_HomeDir		, MUIA_Disabled, !user);
	set(data->STR_Shell		, MUIA_Disabled, !user);
	set(data->CH_Disabled	, MUIA_Disabled, !user);
	set(data->STR_UserID		, MUIA_Disabled, !user);
	set(data->STR_GroupID	, MUIA_Disabled, !user);

	return(NULL);
}

SAVEDS ASM struct Group *GroupList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Group *src)
{
	struct Group *new;

	if((new = (struct Group *)AllocVec(sizeof(struct Group), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Group));
	return(new);
}

ULONG Users_SetGroupStates(struct IClass *cl, Object *obj, Msg msg)
{
	struct Users_Data *data = INST_DATA(cl, obj);
	struct Group *group;

	DoMethod(data->LV_Groups, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &group);
	if(group)
	{
		STRPTR ptr1, ptr2;
		char group_member[41];

		nnset(data->STR_Group		, MUIA_String_Contents	, group->Name);
		nnset(data->STR_GroupNumber, MUIA_String_Integer	, group->ID);
		set(data->LV_GroupMembers, MUIA_List_Quiet, TRUE);
		DoMethod(data->LV_GroupMembers, MUIM_List_Clear);
		ptr1 = group->Members;
		while(ptr1 && *ptr1)
		{
			strncpy(group_member, ptr1, 40);

			if(ptr2 = strchr(ptr1, ','))
				group_member[(ptr2 - ptr1)] = NULL;

			DoMethod(data->LV_GroupMembers, MUIM_List_InsertSingle, group_member, MUIV_List_Insert_Bottom);
			ptr1 = (ptr2 ? ptr2 + 1 : NULL);
		}
		set(data->LV_GroupMembers, MUIA_List_Quiet, FALSE);
	}
	else
	{
		nnset(data->STR_Group	, MUIA_String_Contents	, "");
		nnset(data->STR_GroupNumber, MUIA_String_Integer	, 0);
		DoMethod(data->LV_GroupMembers, MUIM_List_Clear);
	}
	set(data->BT_RemoveGroup		, MUIA_Disabled, !group);
	set(data->STR_Group				, MUIA_Disabled, !group);
	set(data->STR_GroupNumber		, MUIA_Disabled, !group);
	set(data->LV_GroupMembers		, MUIA_Disabled, !group);
	set(data->BT_RemoveGroupMember, MUIA_Disabled, !group || (xget(data->LV_GroupMembers, MUIA_List_Active) == MUIV_List_Active_Off));

	return(NULL);
}

ULONG Users_GetGroupStates(struct IClass *cl, Object *obj, Msg msg)
{
	struct Users_Data *data = INST_DATA(cl, obj);
	struct Group *group;

	DoMethod(data->LV_Groups, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &group);
	if(group)
	{
		int i;
		STRPTR ptr = NULL;

		*group->Members = NULL;
		i = 0;
		FOREVER
		{
			DoMethod(data->LV_GroupMembers, MUIM_List_GetEntry, i++, &ptr);
			if(!ptr)
				break;

			if(strlen(ptr) + strlen(group->Members) < 400)
			{
				if(i > 1)
					strcpy(&group->Members[strlen(group->Members)], ",");
				strcpy(&group->Members[strlen(group->Members)], ptr);
			}
		}
		set(data->LV_GroupMembers, MUIA_List_Active, MUIV_List_Active_Top);
	}

	return(NULL);
}

ULONG Users_Modification(struct IClass *cl, Object *obj, struct MUIP_Users_Modification *msg)
{
	struct Users_Data *data = INST_DATA(cl, obj);
	struct User *user;
	struct Group *group;

	switch(msg->flags)
	{
		case MUIV_Users_Modification_NewUser:
			DoMethod(data->LV_User, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_User, MUIA_List_Active, MUIV_List_Active_Bottom);
			set(win, MUIA_Window_ActiveObject, data->STR_User);
			break;
		case MUIV_Users_Modification_User:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
			{
				strcpy(user->Login, (STRPTR)xget(data->STR_User, MUIA_String_Contents));
				DoMethod(data->LV_User, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Users_Modification_FullName:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
				strcpy(user->Name, (STRPTR)xget(data->STR_Name, MUIA_String_Contents));
			break;
		case MUIV_Users_Modification_HomeDir:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
				strcpy(user->HomeDir, (STRPTR)xget(data->PA_HomeDir, MUIA_String_Contents));
			break;
		case MUIV_Users_Modification_Shell:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
				strcpy(user->Shell, (STRPTR)xget(data->STR_Shell, MUIA_String_Contents));
			break;
		case MUIV_Users_Modification_UserID:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
				user->UserID = xget(data->STR_UserID, MUIA_String_Integer);
			break;
		case MUIV_Users_Modification_GroupID:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
				user->GroupID = xget(data->STR_GroupID, MUIA_String_Integer);
			break;
		case MUIV_Users_Modification_Disable:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
			{
				user->Disabled = xget(data->CH_Disabled, MUIA_Selected);
				set(data->BT_ChangePassword, MUIA_Disabled, user->Disabled);
				set(data->BT_RemovePassword, MUIA_Disabled, !*user->Password || user->Disabled);
			}
			break;
		case MUIV_Users_Modification_ChangePassword:
MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_Okay), "Sorry, this function isn't implemented yet");
			break;
		case MUIV_Users_Modification_RemovePassword:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
			{
				if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_ReallyRemovePasswd), GetStr(MSG_TX_ReallyRemovePasswd)))
				{
					MUI_Request(app, win, NULL, NULL, GetStr(MSG_BT_Okay), GetStr(MSG_TX_PasswordWarning));
					*user->Password = NULL;
					set(data->BT_RemovePassword, MUIA_Disabled, TRUE);
				}
			}
			break;

		case MUIV_Users_Modification_NewGroup:
			DoMethod(data->LV_Groups, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_Groups, MUIA_List_Active, MUIV_List_Active_Bottom);
			set(win, MUIA_Window_ActiveObject, data->STR_Group);
			break;
		case MUIV_Users_Modification_Group:
			DoMethod(data->LV_Groups, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &group);
			if(group)
			{
				strcpy(group->Name, (STRPTR)xget(data->STR_Group, MUIA_String_Contents));
				DoMethod(data->LV_Groups, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Users_Modification_GroupNumber:
			DoMethod(data->LV_Groups, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &group);
			if(group)
				group->ID = xget(data->STR_GroupNumber, MUIA_String_Integer);
			break;
		case MUIV_Users_Modification_RemoveGroupMember:
			DoMethod(data->LV_GroupMembers, MUIM_List_Remove, MUIV_List_Remove_Active);
			DoMethod(obj, MUIM_Users_GetGroupStates);
			break;
		case MUIV_Users_Modification_MembersActive:
			set(data->BT_RemoveGroupMember, MUIA_Disabled, (xget(data->LV_GroupMembers, MUIA_List_Active) == MUIV_List_Active_Off));
			break;
	}
	return(NULL);
}

ULONG Users_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct Users_Data tmp;
	static const struct Hook UserList_ConstructHook		= { { 0,0 }, (VOID *)UserList_ConstructFunc	, NULL, NULL };
	static const struct Hook GroupList_ConstructHook	= { { 0,0 }, (VOID *)GroupList_ConstructFunc	, NULL, NULL };

	if(obj = (Object *)DoSuperNew(cl, obj,
		Child, VGroup,
			GroupFrame,
			MUIA_Background, MUII_RegisterBack,
			Child, HGroup,
				Child, VGroup,
					MUIA_Weight, 50,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_User = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_FrameTitle				, GetStr(MSG_LA_Users),
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_User = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_ConstructHook	, &UserList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_DragSortable	, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewUser		= MakeButton(MSG_BT_New),
						Child, tmp.BT_RemoveUser	= MakeButton(MSG_BT_Remove),
					End,
					Child, tmp.STR_User = String("", 40),
				End,
				Child, VGroup,
					Child, ColGroup(2),
						Child, MakeKeyLabel2(MSG_LA_FullName, MSG_CC_FullName),
						Child, tmp.STR_Name = MakeKeyString("", 80, MSG_CC_FullName),
						Child, MakeKeyLabel2(MSG_LA_HomeDir, MSG_CC_HomeDir),
						Child, tmp.PA_HomeDir = MakePopAsl(MakeKeyString("", MAXPATHLEN, MSG_CC_FullName), GetStr(MSG_TX_ChooseHomeDir), TRUE),
						Child, MakeKeyLabel2(MSG_LA_Shell, MSG_CC_Shell),
						Child, HGroup,
							Child, tmp.STR_Shell = MakeKeyString("", 80, MSG_CC_Shell),
							Child, MakeKeyLabel2(MSG_LA_DisableUser, MSG_CC_DisableUser),
							Child, tmp.CH_Disabled = KeyCheckMark(FALSE, *GetStr(MSG_CC_DisableUser)),
						End,
						Child, MakeKeyLabel2(MSG_LA_UserID, MSG_CC_UserID),
						Child, HGroup,
							Child, tmp.STR_UserID = StringObject,
								StringFrame,
								MUIA_ControlChar		, *GetStr(MSG_CC_UserID),
								MUIA_CycleChain		, 1,
								MUIA_String_MaxLen	, 7,
								MUIA_String_Integer	, 0,
								MUIA_String_Accept	, "1234567890",
							End,
							Child, tmp.BT_ChangePassword = MakeButton(MSG_BT_ChangePassword),
						End,
						Child, MakeKeyLabel2(MSG_LA_UserGID, MSG_CC_UserGID),
						Child, HGroup,
							Child, tmp.STR_GroupID			= NewObject(CL_GroupIDString->mcc_Class, NULL, MUIA_String_MaxLen, 7, MUIA_ControlChar, *GetStr(MSG_CC_UserGID), TAG_DONE),
							Child, tmp.BT_RemovePassword	= MakeButton(MSG_BT_RemovePassword),
						End,
						Child, VVSpace,
						Child, HVSpace,
					End,
				End,
			End,
			Child, BalanceObject, End,
			Child, HGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Groups = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_FrameTitle				, GetStr(MSG_LA_Groups),
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Groups = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_ConstructHook	, &GroupList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_DragSortable	, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewGroup		= MakeButton(MSG_BT_New2),
						Child, tmp.BT_RemoveGroup	= MakeButton(MSG_BT_Remove2),
					End,
				End,
				Child, ColGroup(2),
					Child, VVSpace,
					Child, HVSpace,
					Child, MakeKeyLabel2(MSG_LA_GroupID, MSG_CC_GroupID),
					Child, tmp.STR_GroupNumber	= MakeKeyString("0", 6, MSG_CC_GroupID),
					Child, MakeKeyLabel2(MSG_LA_GroupName, MSG_CC_GroupName),
					Child, tmp.STR_Group			= MakeKeyString("", 40, MSG_CC_GroupName),
				End,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_GroupMembers = ListviewObject,
						MUIA_FrameTitle				, GetStr(MSG_LA_GroupMembers),
						MUIA_CycleChain				, 1,
						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_GroupMembers = NewObject(CL_MemberList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, tmp.BT_RemoveGroupMember = MakeButton(MSG_BT_Remove3),
				End,
			End,
		End,
		Child, TextObject,
			TextFrame,
			MUIA_Background, MUII_TextBack,
			MUIA_Font, MUIV_Font_Tiny,
			MUIA_Text_PreParse, "\033c",
			MUIA_Text_Contents, GetStr(MSG_TX_ChangesSavedOnly),
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Users_Data *data = INST_DATA(cl,obj);
		struct MemberList_Data *ml_data = INST_DATA(CL_MemberList->mcc_Class, tmp.LI_GroupMembers);

		*data = tmp;
		ml_data->Originator = obj;

		set(tmp.LI_GroupMembers, MUIA_UserData, tmp.LI_User);	/* show the list from whom it has to accept drag requests */
		set(tmp.STR_GroupID, MUIA_UserData, tmp.LI_Groups);

		set(tmp.STR_User		, MUIA_String_AttachedList, tmp.LV_User);
		set(tmp.STR_Group		, MUIA_String_AttachedList, tmp.LV_Groups);
		set(tmp.CH_Disabled	, MUIA_CycleChain, 1);
		set(tmp.BT_ChangePassword, MUIA_Weight, 10);
		set(tmp.BT_RemovePassword, MUIA_Weight, 10);

		set(tmp.LV_User				, MUIA_ShortHelp, GetStr(MSG_Help_Users_List));
		set(tmp.BT_NewUser			, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_RemoveUser		, MUIA_ShortHelp, GetStr(MSG_Help_Remove));
		set(tmp.STR_User				, MUIA_ShortHelp, GetStr(MSG_Help_Users_Username));
		set(tmp.STR_Name				, MUIA_ShortHelp, GetStr(MSG_Help_Users_Fullname));
		set(tmp.PA_HomeDir			, MUIA_ShortHelp, GetStr(MSG_Help_Users_HomeDir));
		set(tmp.STR_Shell				, MUIA_ShortHelp, GetStr(MSG_Help_Users_Shell));
		set(tmp.CH_Disabled			, MUIA_ShortHelp, GetStr(MSG_Help_Users_Disable));
		set(tmp.STR_UserID			, MUIA_ShortHelp, GetStr(MSG_Help_Users_UserID));
		set(tmp.BT_ChangePassword	, MUIA_ShortHelp, GetStr(MSG_Help_Users_ChangePassword));
		set(tmp.BT_RemovePassword	, MUIA_ShortHelp, GetStr(MSG_Help_Users_RemovePassword));
		set(tmp.STR_GroupID			, MUIA_ShortHelp, GetStr(MSG_Help_Users_GroupID));
		set(tmp.LV_Groups				, MUIA_ShortHelp, GetStr(MSG_Help_Groups_List));
		set(tmp.BT_NewGroup			, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_RemoveGroup		, MUIA_ShortHelp, GetStr(MSG_Help_Remove));
		set(tmp.STR_Group				, MUIA_ShortHelp, GetStr(MSG_Help_Groups_Name));
		set(tmp.STR_GroupNumber		, MUIA_ShortHelp, GetStr(MSG_Help_Groups_Number));
		set(tmp.LV_GroupMembers		, MUIA_ShortHelp, GetStr(MSG_Help_Groups_Members));
		set(tmp.BT_RemoveGroupMember, MUIA_ShortHelp, GetStr(MSG_Help_Remove));

		DoMethod(tmp.LV_User				, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj				, 1, MUIM_Users_SetUserStates);
		DoMethod(tmp.BT_NewUser			, MUIM_Notify, MUIA_Pressed			, FALSE				, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_NewUser);
		DoMethod(tmp.BT_RemoveUser		, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LV_User	, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.STR_User			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_User);
		DoMethod(tmp.STR_Name			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_FullName);
		DoMethod(tmp.PA_HomeDir			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_HomeDir);
		DoMethod(tmp.STR_Shell			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_Shell);
		DoMethod(tmp.STR_UserID			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_UserID);
		DoMethod(tmp.STR_GroupID		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_GroupID);
		DoMethod(tmp.CH_Disabled		, MUIM_Notify, MUIA_Selected			, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_Disable);
		DoMethod(tmp.BT_ChangePassword, MUIM_Notify, MUIA_Pressed			, FALSE				, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_ChangePassword);
		DoMethod(tmp.BT_RemovePassword, MUIM_Notify, MUIA_Pressed			, FALSE				, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_RemovePassword);

		DoMethod(tmp.LV_Groups			, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj				, 1, MUIM_Users_SetGroupStates);
		DoMethod(tmp.BT_NewGroup		, MUIM_Notify, MUIA_Pressed			, FALSE				, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_NewGroup);
		DoMethod(tmp.BT_RemoveGroup	, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LV_Groups, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.STR_Group			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_Group);
		DoMethod(tmp.STR_GroupNumber	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_GroupNumber);
		DoMethod(tmp.BT_RemoveGroupMember, MUIM_Notify, MUIA_Pressed		, FALSE				, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_RemoveGroupMember);
		DoMethod(tmp.LV_GroupMembers	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj				, 2, MUIM_Users_Modification, MUIV_Users_Modification_MembersActive);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG Users_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW							: return(Users_New				(cl, obj, (APTR)msg));
		case MUIM_Users_SetUserStates	: return(Users_SetUserStates	(cl, obj, (APTR)msg));
		case MUIM_Users_SetGroupStates: return(Users_SetGroupStates	(cl, obj, (APTR)msg));
		case MUIM_Users_GetGroupStates: return(Users_GetGroupStates	(cl, obj, (APTR)msg));
		case MUIM_Users_Modification	: return(Users_Modification	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}


/****************************************************************************/
/* Databases class (GROUP)                                                  */
/****************************************************************************/

SAVEDS ASM struct Protocol *ProtocolList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Protocol *src)
{
	struct Protocol *new;

	if((new = (struct Protocol *)AllocVec(sizeof(struct Protocol), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Protocol));
	return(new);
}

SAVEDS ASM LONG ProtocolList_DisplayFunc(REG(a2) char **array, REG(a1) struct Protocol *protocol)
{
	if(protocol)
	{
		static char buf[7];

		sprintf(buf, "%ld", protocol->ID);
		*array++	= protocol->Name;
		*array++	= buf;
		*array	= protocol->Aliases;
	}
	else
	{
		*array++ = GetStr(MSG_TX_Name);
		*array++ = GetStr(MSG_TX_ID);
		*array	= GetStr(MSG_TX_Aliases);
	}
	return(NULL);
}

SAVEDS ASM struct Service *ServiceList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Service *src)
{
	struct Service *new;

	if((new = (struct Service *)AllocVec(sizeof(struct Service), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Service));
	return(new);
}

SAVEDS ASM LONG ServiceList_DisplayFunc(REG(a2) char **array, REG(a1) struct Service *service)
{
	if(service)
	{
		static char buf[7];

		sprintf(buf, "%ld", service->Port);
		*array++	= service->Name;
		*array++	= buf;
		*array++	= service->Protocol;
		*array	= service->Aliases;
	}
	else
	{
		*array++ = GetStr(MSG_TX_Name);
		*array++ = GetStr(MSG_TX_Port);
		*array++ = GetStr(MSG_TX_Protocol);
		*array	= GetStr(MSG_TX_Aliases);
	}
	return(NULL);
}

SAVEDS ASM struct Inetd *InetdList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Inetd *src)
{
	struct Inetd *new;

	if((new = (struct Inetd *)AllocVec(sizeof(struct Inetd), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Inetd));
	return(new);
}

SAVEDS ASM LONG InetdList_DisplayFunc(REG(a2) char **array, REG(a1) struct Inetd *inetd)
{
	if(inetd)
	{
		*array++	= inetd->Service;
		*array++	= (inetd->Socket ? "dgram" : "stream");
		*array++	= inetd->Protocol;
		*array++	= (inetd->Wait ? (inetd->Wait == 2 ? "dos" : "wait") : "nowait");
		*array++	= inetd->User;
		*array++	= inetd->Server;
		*array++	= inetd->Args;
		*array	= (inetd->Active ? GetStr(MSG_TX_Enabled) : GetStr(MSG_TX_Disabled));
	}
	else
	{
		*array++ = GetStr(MSG_TX_Service);
		*array++ = GetStr(MSG_TX_Socket);
		*array++ = GetStr(MSG_TX_Protocol);
		*array++ = GetStr(MSG_TX_Wait);
		*array++ = GetStr(MSG_TX_User);
		*array++ = GetStr(MSG_TX_Server);
		*array++ = GetStr(MSG_TX_Args);
		*array	= GetStr(MSG_TX_Status);
	}
	return(NULL);
}

SAVEDS ASM struct Host *HostList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Host *src)
{
	struct Host *new;

	if((new = (struct Host *)AllocVec(sizeof(struct Host), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Host));
	return(new);
}

SAVEDS ASM LONG HostList_DisplayFunc(REG(a2) char **array, REG(a1) struct Host *host)
{
	if(host)
	{
		*array++	= host->Addr;
		*array++	= host->Name;
		*array	= host->Aliases;
	}
	else
	{
		*array++ = GetStr(MSG_TX_IPAddr);
		*array++ = GetStr(MSG_TX_Name);
		*array	= GetStr(MSG_TX_Aliases);
	}
	return(NULL);
}

SAVEDS ASM struct Network *NetworkList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Network *src)
{
	struct Network *new;

	if((new = (struct Network *)AllocVec(sizeof(struct Network), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Network));
	return(new);
}

SAVEDS ASM LONG NetworkList_DisplayFunc(REG(a2) char **array, REG(a1) struct Network *network)
{
	if(network)
	{
		static char buf[11];

		sprintf(buf, "%ld", network->Number);
		*array++	= network->Name;
		*array++	= buf;
		*array	= network->Aliases;
	}
	else
	{
		*array++ = GetStr(MSG_TX_Name);
		*array++ = GetStr(MSG_TX_ID);
		*array	= GetStr(MSG_TX_Aliases);
	}
	return(NULL);
}

SAVEDS ASM struct Rpc *RpcList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Rpc *src)
{
	struct Rpc *new;

	if((new = (struct Rpc *)AllocVec(sizeof(struct Rpc), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Rpc));
	return(new);
}

SAVEDS ASM LONG RpcList_DisplayFunc(REG(a2) char **array, REG(a1) struct Rpc *rpc)
{
	if(rpc)
	{
		static char buf[11];

		sprintf(buf, "%ld", rpc->Number);
		*array++	= rpc->Name;
		*array++	= buf;
		*array	= rpc->Aliases;
	}
	else
	{
		*array++ = GetStr(MSG_TX_Name);
		*array++ = GetStr(MSG_TX_ID);
		*array	= GetStr(MSG_TX_Aliases);
	}
	return(NULL);
}

LONG protocol_pos(Object *list, STRPTR string)
{
	LONG pos = 0;
	struct Protocol *protocol;

	FOREVER
	{
		DoMethod(list, MUIM_List_GetEntry, pos, &protocol);
		if(!protocol)
			break;
		if(!stricmp(protocol->Name, string))
			break;
		pos++;
	}
	return(protocol ? pos : MUIV_List_Active_Off);
}

ULONG Databases_SetStates(struct IClass *cl, Object *obj, struct MUIP_Databases_SetStates *msg)
{
	struct Databases_Data *data = INST_DATA(cl, obj);

	switch(msg->page)
	{
		case MUIV_Databases_SetStates_Protocols:
		{
			struct Protocol *protocol;
			LONG pos;

			DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &protocol);
			if(protocol)
			{
				set(data->STR_ProtocolName, MUIA_String_Contents, protocol->Name);
				set(data->STR_ProtocolID, MUIA_String_Integer, protocol->ID);
				set(data->STR_ProtocolAliases, MUIA_String_Contents, protocol->Aliases);
			}
			else
			{
				set(data->STR_ProtocolName, MUIA_String_Contents, "");
				set(data->STR_ProtocolID, MUIA_String_Integer, 0);
				set(data->STR_ProtocolAliases, MUIA_String_Contents, "");
			}
			set(data->BT_RemoveProtocol, MUIA_Disabled, !protocol);
			set(data->STR_ProtocolName, MUIA_Disabled, !protocol);
			set(data->STR_ProtocolID, MUIA_Disabled, !protocol);
			set(data->STR_ProtocolAliases, MUIA_Disabled, !protocol);

			// copy the protocol names to the two other lists
			set(data->LV_Services, MUIA_List_Active, MUIV_List_Active_Off);
			set(data->LV_Inetd, MUIA_List_Active, MUIV_List_Active_Off);
			DoMethod(data->LV_ServiceProtocol, MUIM_List_Clear);
			DoMethod(data->LV_InetdProtocol, MUIM_List_Clear);
			pos = 0;
			FOREVER
			{
				DoMethod(data->LV_Protocols, MUIM_List_GetEntry, pos++, &protocol);
				if(!protocol)
					break;
				DoMethod(data->LV_ServiceProtocol, MUIM_List_InsertSingle, protocol->Name, MUIV_List_Insert_Bottom);
				DoMethod(data->LV_InetdProtocol, MUIM_List_InsertSingle, protocol->Name, MUIV_List_Insert_Bottom);
			}
		}
			break;

		case MUIV_Databases_SetStates_Services:
		{
			struct Service *service;

			DoMethod(data->LV_Services, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &service);
			if(service)
			{
				set(data->STR_ServiceName, MUIA_String_Contents, service->Name);
				set(data->STR_ServicePort, MUIA_String_Integer, service->Port);
				set(data->STR_ServiceAliases, MUIA_String_Contents, service->Aliases);
				set(data->LV_ServiceProtocol, MUIA_List_Active, protocol_pos(data->LV_ServiceProtocol, service->Protocol));
			}
			else
			{
				set(data->STR_ServiceName, MUIA_String_Contents, "");
				set(data->STR_ServicePort, MUIA_String_Integer, 0);
				set(data->STR_ServiceAliases, MUIA_String_Contents, "");
				set(data->LV_ServiceProtocol, MUIA_List_Active, MUIV_List_Active_Off);
			}
			set(data->BT_RemoveService, MUIA_Disabled, !service);
			set(data->STR_ServiceName, MUIA_Disabled, !service);
			set(data->STR_ServicePort, MUIA_Disabled, !service);
			set(data->STR_ServiceAliases, MUIA_Disabled, !service);
			set(data->LV_ServiceProtocol, MUIA_Disabled, !service);
		}
			break;

		case MUIV_Databases_SetStates_Inetd:
		{
			struct Inetd *inetd;

			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				set(data->STR_InetdService, MUIA_String_Contents, inetd->Service);
				set(data->STR_InetdUser, MUIA_String_Contents, inetd->User);
				set(data->PA_InetdServer, MUIA_String_Contents, inetd->Server);
				set(data->STR_InetdArgs, MUIA_String_Contents, inetd->Args);
				set(data->CH_InetdActive, MUIA_Selected, inetd->Active);
				set(data->LV_InetdProtocol, MUIA_List_Active, protocol_pos(data->LV_InetdProtocol, inetd->Protocol));
				set(data->CY_InetdSocket, MUIA_Cycle_Active, inetd->Socket);
				set(data->CY_InetdWait, MUIA_Cycle_Active, inetd->Wait);
			}
			else
			{
				set(data->STR_InetdService, MUIA_String_Contents, "");
				set(data->STR_InetdUser, MUIA_String_Contents, "");
				set(data->PA_InetdServer, MUIA_String_Contents, "");
				set(data->STR_InetdArgs, MUIA_String_Contents, "");
				set(data->CH_InetdActive, MUIA_Selected, FALSE);
				set(data->LV_InetdProtocol, MUIA_List_Active, MUIV_List_Active_Off);
				set(data->CY_InetdSocket, MUIA_Cycle_Active, 0);
				set(data->CY_InetdWait, MUIA_Cycle_Active, 0);
			}
			set(data->BT_RemoveInetd, MUIA_Disabled, !inetd);
			set(data->STR_InetdService, MUIA_Disabled, !inetd);
			set(data->STR_InetdUser, MUIA_Disabled, !inetd);
			set(data->PA_InetdServer, MUIA_Disabled, !inetd);
			set(data->STR_InetdArgs, MUIA_Disabled, !inetd);
			set(data->CH_InetdActive, MUIA_Disabled, !inetd);
			set(data->LV_InetdProtocol, MUIA_Disabled, !inetd);
			set(data->CY_InetdSocket, MUIA_Disabled, !inetd);
			set(data->CY_InetdWait, MUIA_Disabled, !inetd);
		}
			break;

		case MUIV_Databases_SetStates_Hosts:
		{
			struct Host *host;

			DoMethod(data->LV_Hosts, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &host);
			if(host)
			{
				set(data->STR_HostAddr, MUIA_String_Contents, host->Addr);
				set(data->STR_HostName, MUIA_String_Contents, host->Name);
				set(data->STR_HostAliases, MUIA_String_Contents, host->Aliases);
			}
			else
			{
				set(data->STR_HostAddr, MUIA_String_Contents, 0);
				set(data->STR_HostName, MUIA_String_Contents, "");
				set(data->STR_HostAliases, MUIA_String_Contents, "");
			}
			set(data->BT_RemoveHost, MUIA_Disabled, !host);
			set(data->STR_HostAddr, MUIA_Disabled, !host);
			set(data->STR_HostName, MUIA_Disabled, !host);
			set(data->STR_HostAliases, MUIA_Disabled, !host);
		}
			break;

		case MUIV_Databases_SetStates_Networks:
		{
			struct Network *network;

			DoMethod(data->LV_Networks, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &network);
			if(network)
			{
				set(data->STR_NetworkName, MUIA_String_Contents, network->Name);
				set(data->STR_NetworkID, MUIA_String_Integer, network->Number);
				set(data->STR_NetworkAliases, MUIA_String_Contents, network->Aliases);
			}
			else
			{
				set(data->STR_NetworkName, MUIA_String_Contents, "");
				set(data->STR_NetworkID, MUIA_String_Integer, 0);
				set(data->STR_NetworkAliases, MUIA_String_Contents, "");
			}
			set(data->BT_RemoveNetwork, MUIA_Disabled, !network);
			set(data->STR_NetworkName, MUIA_Disabled, !network);
			set(data->STR_NetworkID, MUIA_Disabled, !network);
			set(data->STR_NetworkAliases, MUIA_Disabled, !network);
		}
			break;

		case MUIV_Databases_SetStates_Rpcs:
		{
			struct Rpc *rpc;

			DoMethod(data->LV_Rpcs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &rpc);
			if(rpc)
			{
				set(data->STR_RpcName, MUIA_String_Contents, rpc->Name);
				set(data->STR_RpcID, MUIA_String_Integer, rpc->Number);
				set(data->STR_RpcAliases, MUIA_String_Contents, rpc->Aliases);
			}
			else
			{
				set(data->STR_RpcName, MUIA_String_Contents, "");
				set(data->STR_RpcID, MUIA_String_Integer, 0);
				set(data->STR_RpcAliases, MUIA_String_Contents, "");
			}
			set(data->BT_RemoveRpc, MUIA_Disabled, !rpc);
			set(data->STR_RpcName, MUIA_Disabled, !rpc);
			set(data->STR_RpcID, MUIA_Disabled, !rpc);
			set(data->STR_RpcAliases, MUIA_Disabled, !rpc);
		}
			break;
	}
	return(NULL);
}

ULONG Databases_Modification(struct IClass *cl, Object *obj, struct MUIP_Databases_Modification *msg)
{
	struct Databases_Data *data = INST_DATA(cl, obj);
	struct Protocol *protocol;
	struct Service *service;
	struct Inetd *inetd;
	struct Host *host;
	struct Network *network;
	struct Rpc *rpc;

	switch(msg->what)
	{
		case MUIV_Databases_Modification_ProtocolName:
			DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &protocol);
			if(protocol)
			{
				strcpy(protocol->Name, (STRPTR)xget(data->STR_ProtocolName, MUIA_String_Contents));
				DoMethod(data->LV_Protocols, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_ProtocolID:
			DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &protocol);
			if(protocol)
			{
				protocol->ID = xget(data->STR_ProtocolID, MUIA_String_Integer);
				DoMethod(data->LV_Protocols, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_ProtocolAliases:
			DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &protocol);
			if(protocol)
			{
				strcpy(protocol->Aliases, (STRPTR)xget(data->STR_ProtocolAliases, MUIA_String_Contents));
				DoMethod(data->LV_Protocols, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_ServiceName:
			DoMethod(data->LV_Services, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &service);
			if(service)
			{
				strcpy(service->Name, (STRPTR)xget(data->STR_ServiceName, MUIA_String_Contents));
				DoMethod(data->LV_Services, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_ServicePort:
			DoMethod(data->LV_Services, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &service);
			if(service)
			{
				service->Port = xget(data->STR_ServicePort, MUIA_String_Integer);
				DoMethod(data->LV_Services, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_ServiceAliases:
			DoMethod(data->LV_Services, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &service);
			if(service)
			{
				strcpy(service->Aliases, (STRPTR)xget(data->STR_ServiceAliases, MUIA_String_Contents));
				DoMethod(data->LV_Services, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_ServiceProtocol:
			DoMethod(data->LV_Services, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &service);
			if(service)
			{
				DoMethod(data->LV_ServiceProtocol, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &protocol);
				if(protocol)
				{
					strcpy(service->Protocol, protocol->Name);
					DoMethod(data->LV_Services, MUIM_List_Redraw, MUIV_List_Redraw_Active);
				}
			}
			break;
		case MUIV_Databases_Modification_InetdService:
			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				strcpy(inetd->Service, (STRPTR)xget(data->STR_InetdService, MUIA_String_Contents));
				DoMethod(data->LV_Inetd, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_InetdUser:
			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				strcpy(inetd->User, (STRPTR)xget(data->STR_InetdUser, MUIA_String_Contents));
				DoMethod(data->LV_Inetd, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_InetdServer:
			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				strcpy(inetd->Server, (STRPTR)xget(data->PA_InetdServer, MUIA_String_Contents));
				DoMethod(data->LV_Inetd, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_InetdArgs:
			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				strcpy(inetd->Args, (STRPTR)xget(data->STR_InetdArgs, MUIA_String_Contents));
				DoMethod(data->LV_Inetd, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_InetdActive:
			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				inetd->Active = xget(data->CH_InetdActive, MUIA_Selected);
				DoMethod(data->LV_Inetd, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_InetdProtocol:
			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				DoMethod(data->LV_InetdProtocol, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &protocol);
				if(protocol)
				{
					strcpy(inetd->Protocol, protocol->Name);
					DoMethod(data->LV_Inetd, MUIM_List_Redraw, MUIV_List_Redraw_Active);
				}
			}
			break;
		case MUIV_Databases_Modification_InetdSocket:
			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				inetd->Socket = xget(data->CY_InetdSocket, MUIA_Cycle_Active);
				DoMethod(data->LV_Inetd, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_InetdWait:
			DoMethod(data->LV_Inetd, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &inetd);
			if(inetd)
			{
				inetd->Wait = xget(data->CY_InetdWait, MUIA_Cycle_Active);
				DoMethod(data->LV_Inetd, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_HostAddr:
			DoMethod(data->LV_Hosts, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &host);
			if(host)
			{
				strcpy(host->Addr, (STRPTR)xget(data->STR_HostAddr, MUIA_String_Contents));
				DoMethod(data->LV_Hosts, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_HostName:
			DoMethod(data->LV_Hosts, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &host);
			if(host)
			{
				strcpy(host->Name, (STRPTR)xget(data->STR_HostName, MUIA_String_Contents));
				DoMethod(data->LV_Hosts, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_HostAliases:
			DoMethod(data->LV_Hosts, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &host);
			if(host)
			{
				strcpy(host->Aliases, (STRPTR)xget(data->STR_HostAliases, MUIA_String_Contents));
				DoMethod(data->LV_Hosts, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_NetworkName:
			DoMethod(data->LV_Networks, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &network);
			if(network)
			{
				strcpy(network->Name, (STRPTR)xget(data->STR_NetworkName, MUIA_String_Contents));
				DoMethod(data->LV_Networks, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_NetworkID:
			DoMethod(data->LV_Networks, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &network);
			if(network)
			{
				network->Number = xget(data->STR_NetworkID, MUIA_String_Integer);
				DoMethod(data->LV_Networks, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_NetworkAliases:
			DoMethod(data->LV_Networks, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &network);
			if(network)
			{
				strcpy(network->Aliases, (STRPTR)xget(data->STR_NetworkAliases, MUIA_String_Contents));
				DoMethod(data->LV_Networks, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_RpcName:
			DoMethod(data->LV_Rpcs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &rpc);
			if(rpc)
			{
				strcpy(rpc->Name, (STRPTR)xget(data->STR_RpcName, MUIA_String_Contents));
				DoMethod(data->LV_Rpcs, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_RpcID:
			DoMethod(data->LV_Rpcs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &rpc);
			if(rpc)
			{
				rpc->Number = xget(data->STR_RpcID, MUIA_String_Integer);
				DoMethod(data->LV_Rpcs, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_RpcAliases:
			DoMethod(data->LV_Rpcs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &rpc);
			if(rpc)
			{
				strcpy(rpc->Aliases, (STRPTR)xget(data->STR_RpcAliases, MUIA_String_Contents));
				DoMethod(data->LV_Rpcs, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Databases_Modification_NewProtocol:
			DoMethod(data->LV_Protocols, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_Protocols, MUIA_List_Active, MUIV_List_Active_Bottom);
			break;
		case MUIV_Databases_Modification_NewService:
			DoMethod(data->LV_Services, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_Services, MUIA_List_Active, MUIV_List_Active_Bottom);
			break;
		case MUIV_Databases_Modification_NewInetd:
			DoMethod(data->LV_Inetd, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_Inetd, MUIA_List_Active, MUIV_List_Active_Bottom);
			break;
		case MUIV_Databases_Modification_NewHost:
			DoMethod(data->LV_Hosts, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_Hosts, MUIA_List_Active, MUIV_List_Active_Bottom);
			break;
		case MUIV_Databases_Modification_NewNetwork:
			DoMethod(data->LV_Networks, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_Networks, MUIA_List_Active, MUIV_List_Active_Bottom);
			break;
		case MUIV_Databases_Modification_NewRpc:
			DoMethod(data->LV_Rpcs, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_Rpcs, MUIA_List_Active, MUIV_List_Active_Bottom);
			break;
	}
	return(NULL);
}

ULONG Databases_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook ProtocolList_ConstructHook= { { 0,0 }, (VOID *)ProtocolList_ConstructFunc	, NULL, NULL };
	static const struct Hook ProtocolList_DisplayHook	= { { 0,0 }, (VOID *)ProtocolList_DisplayFunc	, NULL, NULL };
	static const struct Hook ServiceList_ConstructHook	= { { 0,0 }, (VOID *)ServiceList_ConstructFunc	, NULL, NULL };
	static const struct Hook ServiceList_DisplayHook	= { { 0,0 }, (VOID *)ServiceList_DisplayFunc		, NULL, NULL };
	static const struct Hook InetdList_ConstructHook	= { { 0,0 }, (VOID *)InetdList_ConstructFunc	, NULL, NULL };
	static const struct Hook InetdList_DisplayHook	= { { 0,0 }, (VOID *)InetdList_DisplayFunc		, NULL, NULL };
	static const struct Hook HostList_ConstructHook= { { 0,0 }, (VOID *)HostList_ConstructFunc	, NULL, NULL };
	static const struct Hook HostList_DisplayHook	= { { 0,0 }, (VOID *)HostList_DisplayFunc	, NULL, NULL };
	static const struct Hook NetworkList_ConstructHook= { { 0,0 }, (VOID *)NetworkList_ConstructFunc	, NULL, NULL };
	static const struct Hook NetworkList_DisplayHook	= { { 0,0 }, (VOID *)NetworkList_DisplayFunc	, NULL, NULL };
	static const struct Hook RpcList_ConstructHook= { { 0,0 }, (VOID *)RpcList_ConstructFunc	, NULL, NULL };
	static const struct Hook RpcList_DisplayHook	= { { 0,0 }, (VOID *)RpcList_DisplayFunc	, NULL, NULL };
	static STRPTR STR_GR_DatabasesRegister[7];
	static STRPTR STR_CY_Socket[3];
	static STRPTR STR_CY_Wait[4];
	struct Databases_Data tmp;

	STR_GR_DatabasesRegister[0] = GetStr(MSG_DatabasesRegister1);
	STR_GR_DatabasesRegister[1] = GetStr(MSG_DatabasesRegister2);
	STR_GR_DatabasesRegister[2] = GetStr(MSG_DatabasesRegister3);
	STR_GR_DatabasesRegister[3] = GetStr(MSG_DatabasesRegister4);
	STR_GR_DatabasesRegister[4] = GetStr(MSG_DatabasesRegister5);
	STR_GR_DatabasesRegister[5] = GetStr(MSG_DatabasesRegister6);
	STR_GR_DatabasesRegister[6] = NULL;

	STR_CY_Socket[0] = "stream";
	STR_CY_Socket[1] = "dgram";
	STR_CY_Socket[2] = NULL;

	STR_CY_Wait[0] = "nowait";
	STR_CY_Wait[1] = "wait";
	STR_CY_Wait[2] = "dos";
	STR_CY_Wait[3] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		Child, tmp.GR_Register = RegisterObject,
			MUIA_Background		, MUII_RegisterBack,
			MUIA_Register_Titles	, STR_GR_DatabasesRegister,
			MUIA_CycleChain		, 1,
			Child, VGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Hosts = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Hosts = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_DragSortable	, TRUE,
							MUIA_List_DisplayHook	, &HostList_DisplayHook,
							MUIA_List_ConstructHook	, &HostList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_Format			, "BAR,BAR,",
							MUIA_List_Title			, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewHost = MakeButton(MSG_BT_New),
						Child, tmp.BT_RemoveHost = MakeButton(MSG_BT_Remove),
					End,
				End,
				Child, ColGroup(2),
					Child, MakeKeyLabel2(MSG_LA_IPAddr, MSG_CC_IPAddr),
					Child, tmp.STR_HostAddr = StringObject,
						MUIA_ControlChar		, *GetStr(MSG_CC_IPAddr),
						MUIA_CycleChain		, 1,
						StringFrame,
						MUIA_String_MaxLen	, 16,
						MUIA_String_Accept	, "1234567890.",
					End,
					Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
					Child, tmp.STR_HostName = MakeKeyString("", 40, MSG_CC_Name),
					Child, MakeKeyLabel2(MSG_LA_Aliases, MSG_CC_Aliases),
					Child, tmp.STR_HostAliases = MakeKeyString("", 80, MSG_CC_Aliases),
				End,
			End,
			Child, VGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Protocols = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Protocols = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_DragSortable	, TRUE,
							MUIA_List_DisplayHook	, &ProtocolList_DisplayHook,
							MUIA_List_ConstructHook	, &ProtocolList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_Format			, "BAR,BAR,",
							MUIA_List_Title			, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewProtocol = MakeButton(MSG_BT_New),
						Child, tmp.BT_RemoveProtocol = MakeButton(MSG_BT_Remove),
					End,
				End,
				Child, ColGroup(2),
					Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
					Child, tmp.STR_ProtocolName = MakeKeyString("", 40, MSG_CC_Name),
					Child, MakeKeyLabel2(MSG_LA_ID, MSG_CC_ID),
					Child, tmp.STR_ProtocolID = StringObject,
						MUIA_ControlChar		, *GetStr(MSG_CC_ID),
						MUIA_CycleChain		, 1,
						StringFrame,
						MUIA_String_MaxLen	, 7,
						MUIA_String_Integer	, 0,
						MUIA_String_Accept	, "1234567890",
					End,
					Child, MakeKeyLabel2(MSG_LA_Aliases, MSG_CC_Aliases),
					Child, tmp.STR_ProtocolAliases = MakeKeyString("", 80, MSG_CC_Aliases),
				End,
			End,
			Child, VGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Services = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Services = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_DragSortable	, TRUE,
							MUIA_List_DisplayHook	, &ServiceList_DisplayHook,
							MUIA_List_ConstructHook	, &ServiceList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_Format			, "BAR,BAR,BAR,",
							MUIA_List_Title			, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewService = MakeButton(MSG_BT_New),
						Child, tmp.BT_RemoveService = MakeButton(MSG_BT_Remove),
					End,
				End,
				Child, HGroup,
					Child, ColGroup(2),
						Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
						Child, tmp.STR_ServiceName = MakeKeyString("", 40, MSG_CC_Name),
						Child, MakeKeyLabel2(MSG_LA_Port, MSG_CC_Port),
						Child, tmp.STR_ServicePort = StringObject,
							MUIA_ControlChar		, *GetStr(MSG_CC_Port),
							MUIA_CycleChain		, 1,
							StringFrame,
							MUIA_String_MaxLen	, 7,
							MUIA_String_Integer	, 0,
							MUIA_String_Accept	, "1234567890",
						End,
						Child, MakeKeyLabel2(MSG_LA_Aliases, MSG_CC_Aliases),
						Child, tmp.STR_ServiceAliases = MakeKeyString("", 80, MSG_CC_Aliases),
					End,
					Child, BalanceObject, End,
					Child, VGroup,
						MUIA_Weight, 20,
						Child, tmp.LV_ServiceProtocol = ListviewObject,
							MUIA_CycleChain	, 1,
							MUIA_Listview_List, tmp.LI_ServiceProtocol = ListObject,
								MUIA_Frame, MUIV_Frame_InputList,
							End,
						End,
					End,
				End,
			End,
			Child, VGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Inetd = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Inetd = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_DragSortable	, TRUE,
							MUIA_List_DisplayHook	, &InetdList_DisplayHook,
							MUIA_List_ConstructHook	, &InetdList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_Format			, "BAR,BAR,BAR,BAR,BAR,BAR,BAR,",
							MUIA_List_Title			, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewInetd = MakeButton(MSG_BT_New),
						Child, tmp.BT_RemoveInetd = MakeButton(MSG_BT_Remove),
					End,
				End,
				Child, HGroup,
					Child, ColGroup(2),
						Child, MakeKeyLabel2(MSG_LA_Service, MSG_CC_Service),
						Child, tmp.STR_InetdService	= MakeKeyString("", 40, MSG_CC_Service),
						Child, MakeKeyLabel2(MSG_LA_User, MSG_CC_User),
						Child, tmp.STR_InetdUser		= MakeKeyString("", 40, MSG_CC_User),
						Child, MakeKeyLabel2(MSG_LA_Server, MSG_CC_Server),
						Child, tmp.PA_InetdServer		= MakePopAsl(MakeKeyString("", 80, MSG_CC_Server), GetStr(MSG_LA_Server), FALSE),
						Child, MakeKeyLabel2(MSG_LA_Args, MSG_CC_Args),
						Child, tmp.STR_InetdArgs		= MakeKeyString("", 80, MSG_CC_Args),
						Child, MakeKeyLabel1(MSG_LA_Enabled, MSG_CC_Enabled),
						Child, HGroup,
							Child, tmp.CH_InetdActive = KeyCheckMark(TRUE, *GetStr(MSG_CC_Enabled)),
							Child, HVSpace,
						End,
					End,
					Child, BalanceObject, End,
					Child, VGroup,
						MUIA_Weight, 20,
						Child, tmp.LV_InetdProtocol = ListviewObject,
							MUIA_CycleChain	, 1,
							MUIA_Listview_List, tmp.LI_InetdProtocol = ListObject,
								MUIA_Frame, MUIV_Frame_InputList,
							End,
						End,
						Child, tmp.CY_InetdSocket	= Cycle(STR_CY_Socket),
						Child, tmp.CY_InetdWait		= Cycle(STR_CY_Wait),
					End,
				End,
			End,
			Child, VGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Networks = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Networks = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_DragSortable	, TRUE,
							MUIA_List_DisplayHook	, &NetworkList_DisplayHook,
							MUIA_List_ConstructHook	, &NetworkList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_Format			, "BAR,BAR,",
							MUIA_List_Title			, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewNetwork = MakeButton(MSG_BT_New),
						Child, tmp.BT_RemoveNetwork = MakeButton(MSG_BT_Remove),
					End,
				End,
				Child, ColGroup(2),
					Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
					Child, tmp.STR_NetworkName = MakeKeyString("", 40, MSG_CC_Name),
					Child, MakeKeyLabel2(MSG_LA_ID, MSG_CC_ID),
					Child, tmp.STR_NetworkID = StringObject,
						MUIA_ControlChar		, *GetStr(MSG_CC_ID),
						MUIA_CycleChain		, 1,
						StringFrame,
						MUIA_String_MaxLen	, 5,
						MUIA_String_Integer	, 0,
						MUIA_String_Accept	, "1234567890",
					End,
					Child, MakeKeyLabel2(MSG_LA_Aliases, MSG_CC_Aliases),
					Child, tmp.STR_NetworkAliases = MakeKeyString("", 80, MSG_CC_Aliases),
				End,
			End,
			Child, VGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Rpcs = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Rpcs = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_DragSortable	, TRUE,
							MUIA_List_DisplayHook	, &RpcList_DisplayHook,
							MUIA_List_ConstructHook	, &RpcList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_Format			, "BAR,BAR,",
							MUIA_List_Title			, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewRpc = MakeButton(MSG_BT_New),
						Child, tmp.BT_RemoveRpc = MakeButton(MSG_BT_Remove),
					End,
				End,
				Child, ColGroup(2),
					Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
					Child, tmp.STR_RpcName = MakeKeyString("", 40, MSG_CC_Name),
					Child, MakeKeyLabel2(MSG_LA_ID, MSG_CC_ID),
					Child, tmp.STR_RpcID = StringObject,
						MUIA_ControlChar		, *GetStr(MSG_CC_ID),
						MUIA_CycleChain		, 1,
						StringFrame,
						MUIA_String_MaxLen	, 10,
						MUIA_String_Integer	, 0,
						MUIA_String_Accept	, "1234567890",
					End,
					Child, MakeKeyLabel2(MSG_LA_Aliases, MSG_CC_Aliases),
					Child, tmp.STR_RpcAliases = MakeKeyString("", 80, MSG_CC_Aliases),
				End,
			End,
		End,
		Child, tmp.TX_Info = TextObject,
			TextFrame,
			MUIA_Background, MUII_TextBack,
			MUIA_Font, MUIV_Font_Tiny,
			MUIA_Text_PreParse, "\033c",
			MUIA_Text_Contents, GetStr(MSG_TX_ChangesSavedOnly),
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Databases_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.STR_ProtocolName, MUIA_String_AttachedList, tmp.LV_Protocols);
		set(tmp.STR_HostAddr		, MUIA_String_AttachedList, tmp.LV_Hosts);
		set(tmp.STR_ServiceName	, MUIA_String_AttachedList, tmp.LV_Services);
		set(tmp.STR_InetdService, MUIA_String_AttachedList, tmp.LV_Inetd);
		set(tmp.STR_NetworkName	, MUIA_String_AttachedList, tmp.LV_Networks);
		set(tmp.STR_RpcName		, MUIA_String_AttachedList, tmp.LV_Rpcs);

		set(tmp.CY_InetdSocket	, MUIA_CycleChain, 1);
		set(tmp.CY_InetdWait		, MUIA_CycleChain, 1);
		set(tmp.CH_InetdActive	, MUIA_CycleChain, 1);

		set(tmp.LV_Hosts				, MUIA_ShortHelp, GetStr(MSG_Help_Hosts));
		set(tmp.BT_NewHost			, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_RemoveHost		, MUIA_ShortHelp, GetStr(MSG_Help_Remove));
		set(tmp.STR_HostAddr			, MUIA_ShortHelp, GetStr(MSG_Help_Hosts_IPAddr));
		set(tmp.STR_HostName			, MUIA_ShortHelp, GetStr(MSG_Help_Name));
		set(tmp.STR_HostAliases		, MUIA_ShortHelp, GetStr(MSG_Help_Aliases));

		set(tmp.LV_Protocols			, MUIA_ShortHelp, GetStr(MSG_Help_Protocols));
		set(tmp.BT_NewProtocol		, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_RemoveProtocol	, MUIA_ShortHelp, GetStr(MSG_Help_Remove));
		set(tmp.STR_ProtocolName	, MUIA_ShortHelp, GetStr(MSG_Help_Name));
		set(tmp.STR_ProtocolID		, MUIA_ShortHelp, GetStr(MSG_Help_ID));
		set(tmp.STR_ProtocolAliases, MUIA_ShortHelp, GetStr(MSG_Help_Aliases));

		set(tmp.LV_Services			, MUIA_ShortHelp, GetStr(MSG_Help_Services));
		set(tmp.BT_NewService		, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_RemoveService	, MUIA_ShortHelp, GetStr(MSG_Help_Remove));
		set(tmp.STR_ServiceName		, MUIA_ShortHelp, GetStr(MSG_Help_Name));
		set(tmp.STR_ServicePort		, MUIA_ShortHelp, GetStr(MSG_Help_Port));
		set(tmp.STR_ServiceAliases	, MUIA_ShortHelp, GetStr(MSG_Help_Aliases));
		set(tmp.LV_ServiceProtocol	, MUIA_ShortHelp, GetStr(MSG_Help_Protocol));

		set(tmp.LV_Inetd				, MUIA_ShortHelp, GetStr(MSG_Help_Inetd));
		set(tmp.BT_NewInetd			, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_RemoveInetd		, MUIA_ShortHelp, GetStr(MSG_Help_Remove));
		set(tmp.STR_InetdService	, MUIA_ShortHelp, GetStr(MSG_Help_Service));
		set(tmp.STR_InetdUser		, MUIA_ShortHelp, GetStr(MSG_Help_Inetd_User));
		set(tmp.PA_InetdServer		, MUIA_ShortHelp, GetStr(MSG_Help_Inetd_Server));
		set(tmp.STR_InetdArgs		, MUIA_ShortHelp, GetStr(MSG_Help_Args));
		set(tmp.CH_InetdActive		, MUIA_ShortHelp, GetStr(MSG_Help_Enabled));
		set(tmp.LV_InetdProtocol	, MUIA_ShortHelp, GetStr(MSG_Help_Protocol));
		set(tmp.CY_InetdSocket		, MUIA_ShortHelp, GetStr(MSG_Help_Socket));
		set(tmp.CY_InetdWait			, MUIA_ShortHelp, GetStr(MSG_Help_Wait));

		set(tmp.LV_Networks			, MUIA_ShortHelp, GetStr(MSG_Help_Networks));
		set(tmp.BT_NewNetwork		, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_RemoveNetwork	, MUIA_ShortHelp, GetStr(MSG_Help_Remove));
		set(tmp.STR_NetworkName		, MUIA_ShortHelp, GetStr(MSG_Help_Name));
		set(tmp.STR_NetworkID		, MUIA_ShortHelp, GetStr(MSG_Help_ID));
		set(tmp.STR_NetworkAliases	, MUIA_ShortHelp, GetStr(MSG_Help_Aliases));

		set(tmp.LV_Rpcs				, MUIA_ShortHelp, GetStr(MSG_Help_Rpcs));
		set(tmp.BT_NewRpc				, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_RemoveRpc			, MUIA_ShortHelp, GetStr(MSG_Help_Remove));
		set(tmp.STR_RpcName			, MUIA_ShortHelp, GetStr(MSG_Help_Name));
		set(tmp.STR_RpcID				, MUIA_ShortHelp, GetStr(MSG_Help_ID));
		set(tmp.STR_RpcAliases		, MUIA_ShortHelp, GetStr(MSG_Help_Aliases));

		DoMethod(tmp.LV_Protocols	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Protocols);
		DoMethod(tmp.LV_Services	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Services);
		DoMethod(tmp.LV_Inetd		, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Inetd);
		DoMethod(tmp.LV_Hosts		, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Hosts);
		DoMethod(tmp.LV_Networks	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Networks);
		DoMethod(tmp.LV_Rpcs			, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Rpcs);

		DoMethod(tmp.STR_ProtocolName		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_ProtocolName);
		DoMethod(tmp.STR_ProtocolID		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_ProtocolID);
		DoMethod(tmp.STR_ProtocolAliases	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_ProtocolAliases);
		DoMethod(tmp.STR_ServiceName		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_ServiceName);
		DoMethod(tmp.STR_ServicePort		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_ServicePort);
		DoMethod(tmp.STR_ServiceAliases	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_ServiceAliases);
		DoMethod(tmp.LV_ServiceProtocol	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_ServiceProtocol);
		DoMethod(tmp.STR_InetdService		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_InetdService);
		DoMethod(tmp.STR_InetdUser			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_InetdUser);
		DoMethod(tmp.PA_InetdServer		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_InetdServer);
		DoMethod(tmp.STR_InetdArgs			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_InetdArgs);
		DoMethod(tmp.CH_InetdActive		, MUIM_Notify, MUIA_Selected			, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_InetdActive);
		DoMethod(tmp.LV_InetdProtocol		, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_InetdProtocol);
		DoMethod(tmp.CY_InetdSocket		, MUIM_Notify, MUIA_Cycle_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_InetdSocket);
		DoMethod(tmp.CY_InetdWait			, MUIM_Notify, MUIA_Cycle_Active		, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_InetdWait);
		DoMethod(tmp.STR_HostAddr			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_HostAddr);
		DoMethod(tmp.STR_HostName			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_HostName);
		DoMethod(tmp.STR_HostAliases		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_HostAliases);
		DoMethod(tmp.STR_NetworkName		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NetworkName);
		DoMethod(tmp.STR_NetworkID			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NetworkID);
		DoMethod(tmp.STR_NetworkAliases	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NetworkAliases);
		DoMethod(tmp.STR_RpcName			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_RpcName);
		DoMethod(tmp.STR_RpcID				, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_RpcID);
		DoMethod(tmp.STR_RpcAliases		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_RpcAliases);
		DoMethod(tmp.BT_NewProtocol		, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NewProtocol);
		DoMethod(tmp.BT_NewService			, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NewService);
		DoMethod(tmp.BT_NewInetd			, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NewInetd);
		DoMethod(tmp.BT_NewHost				, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NewHost);
		DoMethod(tmp.BT_NewNetwork			, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NewNetwork);
		DoMethod(tmp.BT_NewRpc				, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Databases_Modification, MUIV_Databases_Modification_NewRpc);
		DoMethod(tmp.BT_RemoveProtocol	, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LV_Protocols, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_RemoveService		, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LV_Services, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_RemoveInetd		, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LV_Inetd, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_RemoveHost			, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LV_Hosts, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_RemoveNetwork		, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LV_Networks, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_RemoveRpc			, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LV_Rpcs, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG Databases_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(Databases_New					(cl, obj, (APTR)msg));
		case MUIM_Databases_SetStates			: return(Databases_SetStates			(cl, obj, (APTR)msg));
		case MUIM_Databases_Modification		: return(Databases_Modification		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}


/****************************************************************************/
/* Events class (GROUP)                                                  */
/****************************************************************************/

ULONG Events_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct Events_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		GroupFrame,
		MUIA_Background, MUII_RegisterBack,
		Child, ColGroup(2),
			Child, VVSpace,
			Child, HVSpace,
			Child, VVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_Start, MSG_CC_Start),
			Child, MakeKeyString("", 80, MSG_CC_Start),
			Child, MakeKeyLabel2(MSG_LA_Stop, MSG_CC_Stop),
			Child, MakeKeyString("", 80, MSG_CC_Stop),
			Child, VVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_Online, MSG_CC_Online),
			Child, MakeKeyString("", 80, MSG_CC_Online),
			Child, MakeKeyLabel2(MSG_LA_OnlineFailed, MSG_CC_OnlineFailed),
			Child, MakeKeyString("", 80, MSG_CC_OnlineFailed),
			Child, VVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_OfflineActive, MSG_CC_OfflineActive),
			Child, MakeKeyString("", 80, MSG_CC_OfflineActive),
			Child, MakeKeyLabel2(MSG_LA_OfflinePassive, MSG_CC_OfflinePassive),
			Child, MakeKeyString("", 80, MSG_CC_OfflinePassive),
			Child, VVSpace,
			Child, HVSpace,
			Child, VVSpace,
			Child, HVSpace,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Events_Data *data = INST_DATA(cl, obj);

		set(obj, MUIA_Disabled, TRUE);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG Events_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(Events_New					(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}
