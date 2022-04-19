#include "globals.c"
#include "protos.h"

static struct Hook sorthook = { {NULL, NULL}, (VOID *)sortfunc, NULL, NULL};
static struct Hook strobjhook = { {NULL, NULL}, (VOID *)strobjfunc, NULL, NULL};
static struct Hook txtobjhook = { {NULL, NULL}, (VOID *)txtobjfunc, NULL, NULL};

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
	if(msg->obj == obj)
		return(DoSuperMethodA(cl, obj, msg));
	else
	{
		struct User *user;

		DoMethod(msg->obj, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
		DoMethod(obj, MUIM_List_InsertSingle, user->Login, xget(obj, MUIA_List_DropMark));
		set(obj, MUIA_List_Active, xget(obj, MUIA_List_InsertPosition));

		return(NULL);
	}
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
	switch (msg->MethodID)
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
		MUIA_CycleChain, 1,
		TAG_MORE, msg->ops_AttrList);

	return((ULONG)obj);
}

SAVEDS ASM ULONG GroupIDString_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW				: return(GroupIDString_New			(cl, obj, (APTR)msg));
		case MUIM_DragQuery	: return(GroupIDString_DragQuery	(cl, obj, (APTR)msg));
		case MUIM_DragDrop	: return(GroupIDString_DragDrop	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}


/*

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

SAVEDS ASM VOID UserList_DestructFunc(REG(a2) APTR pool, REG(a1) struct User *user)
{
	if(user)
		FreeVec(user);
}

ULONG Users_SetUserStates(struct IClass *cl, Object *obj, Msg msg)
{
	struct Users_Data *data = INST_DATA(cl, obj);
	struct User *user;

	DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
	if(user)
	{
		set(data->BT_RemoveUser, MUIA_Disabled, FALSE);
		set(data->STR_User, MUIA_Disabled, FALSE);
		set(data->STR_Name, MUIA_Disabled, FALSE);
		set(data->PA_HomeDir, MUIA_Disabled, FALSE);
		set(data->STR_Shell, MUIA_Disabled, FALSE);
		set(data->CH_Disabled, MUIA_Disabled, FALSE);
		set(data->STR_UserID, MUIA_Disabled, FALSE);
		set(data->STR_GroupID, MUIA_Disabled, FALSE);
		set(data->BT_ChangePassword, MUIA_Disabled, FALSE);
		set(data->BT_RemovePassword, MUIA_Disabled, FALSE);

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
		set(data->BT_RemoveUser, MUIA_Disabled, TRUE);
		set(data->STR_User, MUIA_Disabled, TRUE);
		set(data->STR_Name, MUIA_Disabled, TRUE);
		set(data->PA_HomeDir, MUIA_Disabled, TRUE);
		set(data->STR_Shell, MUIA_Disabled, TRUE);
		set(data->CH_Disabled, MUIA_Disabled, TRUE);
		set(data->STR_UserID, MUIA_Disabled, TRUE);
		set(data->STR_GroupID, MUIA_Disabled, TRUE);
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

	return(NULL);
}

SAVEDS ASM struct Group *GroupList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Group *src)
{
	struct Group *new;

	if((new = (struct Group *)AllocVec(sizeof(struct Group), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Group));
	return(new);
}

SAVEDS ASM VOID GroupList_DestructFunc(REG(a2) APTR pool, REG(a1) struct Group *group)
{
	if(group)
		FreeVec(group);
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

		set(data->BT_RemoveGroup, MUIA_Disabled, FALSE);
		set(data->STR_Group, MUIA_Disabled, FALSE);
		set(data->STR_GroupNumber, MUIA_Disabled, FALSE);
		set(data->LV_GroupMembers, MUIA_Disabled, FALSE);
		set(data->BT_RemoveGroupMember, MUIA_Disabled, FALSE);

		nnset(data->STR_Group	, MUIA_String_Contents	, group->Name);
		nnset(data->STR_GroupNumber, MUIA_String_Integer	, group->ID);
		DoMethod(data->LV_GroupMembers, MUIM_List_Clear);
		ptr1 = group->Members;
Printf("parse members: %ls\n", ptr1);
		while(ptr1 && *ptr1)
		{
			if(ptr2 = strchr(ptr1, ','))
				strncpy(group_member, ptr1, ((ptr2 - ptr1) < 41 ? (ptr2 - ptr1) : 40));
			else
				strncpy(group_member, ptr1, 40);

			DoMethod(data->LV_GroupMembers, MUIM_List_InsertSingle, group_member, MUIV_List_Insert_Bottom);
Printf("member: %ls\n", group_member);
			ptr1 = (ptr2 ? ptr2 + 1 : NULL);
		}
	}
	else
	{
		set(data->BT_RemoveGroup, MUIA_Disabled, TRUE);
		set(data->STR_Group, MUIA_Disabled, TRUE);
		set(data->STR_GroupNumber, MUIA_Disabled, TRUE);
		set(data->LV_GroupMembers, MUIA_Disabled, TRUE);
		set(data->BT_RemoveGroupMember, MUIA_Disabled, TRUE);

		nnset(data->STR_Group	, MUIA_String_Contents	, "");
		nnset(data->STR_GroupNumber, MUIA_String_Integer	, 0);
		DoMethod(data->LV_GroupMembers, MUIM_List_Clear);
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
			break;
		case MUIV_Users_Modification_RemoveUser:
			DoMethod(data->LV_User, MUIM_List_Remove, MUIV_List_Remove_Active);
			break;
		case MUIV_Users_Modification_User:
			DoMethod(data->LV_User, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &user);
			if(user)
			{
				strcpy(user->Login, (STRPTR)xget(data->STR_User, MUIA_String_Contents));
				DoMethod(data->LV_User, MUIM_List_Redraw, MUIV_List_Redraw_Active);
			}
			break;
		case MUIV_Users_Modification_UserName:
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
				user->Disabled = xget(data->CH_Disabled, MUIA_Selected);
			break;
		case MUIV_Users_Modification_ChangePassword:
			break;
		case MUIV_Users_Modification_RemovePassword:
			break;

		case MUIV_Users_Modification_NewGroup:
			DoMethod(data->LV_Groups, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
			set(data->LV_Groups, MUIA_List_Active, MUIV_List_Active_Bottom);
			break;
		case MUIV_Users_Modification_RemoveGroup:
			DoMethod(data->LV_Groups, MUIM_List_Remove, MUIV_List_Remove_Active);
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
		case MUIV_Users_Modification_GroupMembers:
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
Printf("group->members: %ls\n", group->Members);
			}
			break;
		case MUIV_Users_Modification_RemoveGroupMember:
			DoMethod(data->LV_GroupMembers, MUIM_List_Remove, MUIV_List_Remove_Active);
			break;
	}
	return(NULL);
}

ULONG Users_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct Users_Data tmp;
	static const struct Hook UserList_ConstructHook	= { { 0,0 }, (VOID *)UserList_ConstructFunc	, NULL, NULL };
	static const struct Hook UserList_DestructHook	= { { 0,0 }, (VOID *)UserList_DestructFunc	, NULL, NULL };
	static const struct Hook GroupList_ConstructHook	= { { 0,0 }, (VOID *)GroupList_ConstructFunc	, NULL, NULL };
	static const struct Hook GroupList_DestructHook	= { { 0,0 }, (VOID *)GroupList_DestructFunc	, NULL, NULL };
	Object *LI_User, *LI_GroupMembers, *LI_Groups;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Users",
		Child, HGroup,
			GroupFrameT("Users"),
			Child, VGroup,
				MUIA_Weight, 50,
				MUIA_Group_Spacing, 0,
				Child, tmp.LV_User = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_DragType		, 1,
					MUIA_Listview_List			, LI_User = ListObject,
						MUIA_Frame					, MUIV_Frame_InputList,
						MUIA_List_ConstructHook	, &UserList_ConstructHook,
						MUIA_List_DestructHook	, &UserList_DestructHook,
						MUIA_List_DragSortable	, TRUE,
					End,
				End,
				Child, HGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.BT_NewUser = SimpleButton("_New"),
					Child, tmp.BT_RemoveUser = SimpleButton("_Remove"),
				End,
				Child, tmp.STR_User = StringObject,
					StringFrame,
					MUIA_String_MaxLen		, 40,
					MUIA_String_AttachedList, LI_User, 
				End,
			End,
			Child, VGroup,
				Child, ColGroup(2),
					Child, Label("Full Name :"),
					Child, tmp.STR_Name = String("", 80),
					Child, Label("Home Dir :"),
					Child, tmp.PA_HomeDir = MakePopAsl(String("", MAXPATHLEN), "Choose home directory", TRUE),
					Child, Label("Shell :"),
					Child, HGroup,
						Child, tmp.STR_Shell = String("", 80),
						Child, Label("Disable User"),
						Child, tmp.CH_Disabled = CheckMark(FALSE),
					End,
					Child, Label("User ID :"),
					Child, HGroup,
						Child, tmp.STR_UserID = String("", 7),
						Child, tmp.BT_ChangePassword = SimpleButton("_Change Password"),
					End,
					Child, Label("Group ID :"),
					Child, HGroup,
						Child, tmp.STR_GroupID = NewObject(CL_GroupIDString->mcc_Class, NULL, MUIA_String_MaxLen, 7, TAG_DONE),
						Child, tmp.BT_RemovePassword = SimpleButton("_Remove Password"),
					End,
					Child, HVSpace,
					Child, HVSpace,
				End,
			End,
		End,
		Child, HGroup,
			GroupFrameT("Groups"),
			Child, VGroup,
				MUIA_Group_Spacing, 0,
				Child, tmp.LV_Groups = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_DragType		, 1,
					MUIA_Listview_List			, LI_Groups = ListObject,
						MUIA_Frame					, MUIV_Frame_InputList,
						MUIA_List_ConstructHook	, &GroupList_ConstructHook,
						MUIA_List_DestructHook	, &GroupList_DestructHook,
						MUIA_List_DragSortable	, TRUE,
					End,
				End,
				Child, HGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.BT_NewGroup = SimpleButton("N_ew"),
					Child, tmp.BT_RemoveGroup = SimpleButton("Re_move"),
				End,
			End,
			Child, ColGroup(2),
				Child, Label("Name :"),
				Child, tmp.STR_Group = StringObject,
					StringFrame,
					MUIA_String_MaxLen		, 40,
					MUIA_String_AttachedList, LI_Groups, 
				End,
				Child, Label("ID :"),
				Child, tmp.STR_GroupNumber = String("100", 6),
				Child, HVSpace,
				Child, HVSpace,
			End,
			Child, VGroup,
				MUIA_Group_Spacing, 0,
				Child, tmp.LV_GroupMembers = ListviewObject,
					MUIA_FrameTitle				, "Members",
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_DragType		, 1,
					MUIA_Listview_List			, LI_GroupMembers = NewObject(CL_MemberList->mcc_Class, NULL, TAG_DONE),
				End,
				Child, tmp.BT_RemoveGroupMember = SimpleButton("Remo_ve"),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Users_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(LI_GroupMembers, MUIA_UserData, LI_User);	/* show the list from whom it has to accept drag requests */
		set(tmp.STR_GroupID, MUIA_UserData, LI_Groups);

		DoMethod(tmp.LV_User			, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 1, MUIM_Users_SetUserStates);
		DoMethod(tmp.BT_NewUser		, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_NewUser);
		DoMethod(tmp.BT_RemoveUser	, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_RemoveUser);
		DoMethod(tmp.STR_User		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_User);
		DoMethod(tmp.STR_Name		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_UserName);
		DoMethod(tmp.PA_HomeDir		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_HomeDir);
		DoMethod(tmp.STR_Shell		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_Shell);
		DoMethod(tmp.STR_UserID		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_UserID);
		DoMethod(tmp.STR_GroupID	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_GroupID);
		DoMethod(tmp.CH_Disabled	, MUIM_Notify, MUIA_Selected			, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_Disable);
		DoMethod(tmp.CH_Disabled	, MUIM_Notify, MUIA_Selected			, MUIV_EveryTime	, tmp.CH_Disabled, 5, MUIM_MultiSet, MUIA_Disabled, MUIV_TriggerValue, tmp.BT_ChangePassword, tmp.BT_RemovePassword);
		DoMethod(tmp.BT_ChangePassword, MUIM_Notify, MUIA_Pressed		, FALSE				, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_ChangePassword);
		DoMethod(tmp.BT_RemovePassword, MUIM_Notify, MUIA_Pressed		, FALSE				, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_RemovePassword);

		DoMethod(tmp.LV_Groups		, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 1, MUIM_Users_SetGroupStates);
		DoMethod(tmp.BT_NewGroup	, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_NewGroup);
		DoMethod(tmp.BT_RemoveGroup, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_RemoveGroup);
		DoMethod(tmp.STR_Group		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_Group);
		DoMethod(tmp.STR_GroupNumber, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_GroupNumber);
		DoMethod(tmp.LV_GroupMembers, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_GroupMembers);
		DoMethod(tmp.BT_RemoveGroupMember, MUIM_Notify, MUIA_Pressed	, FALSE				, obj, 2, MUIM_Users_Modification, MUIV_Users_Modification_RemoveGroupMember);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG Users_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW											: return(Users_New				(cl, obj, (APTR)msg));
		case MUIM_Users_SetUserStates	: return(Users_SetUserStates	(cl, obj, (APTR)msg));
		case MUIM_Users_SetGroupStates: return(Users_SetGroupStates	(cl, obj, (APTR)msg));
		case MUIM_Users_Modification	: return(Users_Modification	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}

*/


/****************************************************************************/
/* AmiTCP Prefs class                                                       */
/****************************************************************************/

ULONG AmiTCPPrefs_LoadConfig(struct IClass *cl, Object *obj, struct MUIP_AmiTCPPrefs_LoadConfig *msg)
{
	struct AmiTCPPrefs_Data	*data					= INST_DATA(cl								, obj);
	struct Provider_Data		*provider_data		= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
	struct User_Data			*user_data			= INST_DATA(CL_User->mcc_Class		, data->GR_User);
	struct Modem_Data			*modem_data			= INST_DATA(CL_Modem->mcc_Class		, data->GR_Modem);
	struct Paths_Data			*paths_data			= INST_DATA(CL_Paths->mcc_Class		, data->GR_Paths);
//	struct Users_Data			*users_data			= INST_DATA(CL_Users->mcc_Class		, data->GR_Users);
	struct InfoWindow_Data	*infowindow_data	= INST_DATA(CL_InfoWindow->mcc_Class, data->GR_InfoWindow);
	struct pc_Data pc_data;
	STRPTR file;
	int second;

	if(!msg->file)
	{
		if(ParseConfig((get_file_size("ENV:NetConfig/resolv.conf") > -1 ? "ENV:NetConfig/resolv.conf" : "AmiTCP:db/resolv.conf"), &pc_data))
		{
			second = 0;
			while(ParseNext(&pc_data))
			{
				if(!stricmp(pc_data.Argument, "NAMESERVER"))
					setstring((second++ ? provider_data->STR_NameServer2 : provider_data->STR_NameServer1), pc_data.Contents);

				if(!stricmp(pc_data.Argument, "DOMAIN"))
					setstring(provider_data->STR_DomainName, pc_data.Contents);
			}
			ParseEnd(&pc_data);
		}
	}

	if(msg->file)
		file = msg->file;
	else
		file = (get_file_size("ENV:NetConfig/provider.conf") > -1 ? "ENV:NetConfig/provider.conf" : "AmiTCP:db/provider.conf");

	second = 0;
	if(ParseConfig(file, &pc_data))
	{
		while(ParseNext(&pc_data))
		{
//			if(!stricmp(pc_data.Argument, "Name"))
//			if(!stricmp(pc_data.Argument, "DialUp"))

			if(!stricmp(pc_data.Argument, "Interface"))
					setmutex(provider_data->RA_Interface, (stricmp(pc_data.Contents, "ppp") ? 1 : 0));

			if(!stricmp(pc_data.Argument, "InterfaceConfig"))
			{
				if(strstr(pc_data.Contents, "NOVJC"))
					setcycle(provider_data->CY_Header, 2);
				else	if(strstr(pc_data.Contents, "VJCMODE=2"))
					setcycle(provider_data->CY_Header, 1);
				else
					setcycle(provider_data->CY_Header, 0);
			}

//			if(!stricmp(pc_data.Argument, "NeedSerial"))

			if(!stricmp(pc_data.Argument, "IPDynamic"))
				setmutex(provider_data->RA_Connection, (atol(pc_data.Contents) ? 0 : 1));

			if(!stricmp(pc_data.Argument, "IPAddr"))
				setstring(user_data->STR_IP_Address, pc_data.Contents);

//			if(!stricmp(pc_data.Argument, "DestIP"))

//			if(!stricmp(pc_data.Argument, "NSDynamic"))

			if(!stricmp(pc_data.Argument, "NameServer"))
				setstring((second++ ? provider_data->STR_NameServer2 : provider_data->STR_NameServer1), pc_data.Contents);

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
				if(!stricmp(pc_data.Contents, "chap"))
				{
					STRPTR buf, ptr1, ptr2;

					setcycle(provider_data->CY_Authentication, 1);

					/*** load the chap passwordfile ***/

					if(buf = LoadFile("ENV:NetConfig/CHAP_Passwordfile"))
					{
						if(ptr1 = strchr(buf, ' '))
						{
							*ptr1 = NULL;
							setstring(provider_data->STR_HostID, buf);
							ptr2 = ptr1 + 1;
							if(ptr1 = strchr(ptr2, ' '))
							{
								*ptr1 = NULL;
								setstring(provider_data->STR_Password, ptr2);
								ptr2 = ptr1 + 1;
								if(ptr1 = strchr(ptr2, '\n'))
								{
									*ptr1 = NULL;
									setstring(provider_data->STR_YourID, ptr2);
								}
							}
						}
						FreeVec(buf);
					}
				}
				else
				{
					if(!stricmp(pc_data.Contents, "pap"))
					{
						STRPTR buf, ptr1, ptr2;

						setcycle(provider_data->CY_Authentication, 2);

						/*** load the chap passwordfile ***/

						if(buf = LoadFile("ENV:NetConfig/PAP_Passwordfile"))
						{
							if(ptr1 = strchr(buf, ' '))
							{
								*ptr1 = NULL;
								setstring(provider_data->STR_YourID, buf);
								ptr2 = ptr1 + 1;

								if(ptr1 = strchr(ptr2, '\n'))
								{
									*ptr1 = NULL;
									setstring(provider_data->STR_Password, ptr2);
								}
							}
							FreeVec(buf);
						}
					}
					else
						setcycle(provider_data->CY_Authentication, 0);
				}
			}
			if(!stricmp(pc_data.Argument, "MailServer"))
				setstring(provider_data->STR_MailServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "POPServer"))
				setstring(provider_data->STR_POPServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "NewsServer"))
				setstring(provider_data->STR_NewsServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "IRCServer"))
				setstring(provider_data->STR_IRCServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "WWWServer"))
				setstring(provider_data->STR_WWWServer, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Country"))
			{
				char path[MAXPATHLEN];

				nnset(provider_data->PO_Country, MUIA_Text_Contents, pc_data.Contents);
				strcpy(path, "AmiTCP:Providers");
				AddPart(path, pc_data.Contents, MAXPATHLEN);
				DoMethod(data->GR_Provider, MUIM_Provider_PopList_Update, path, MUIV_Provider_PopString_Provider);
				DoMethod(provider_data->LV_PoP, MUIM_List_Clear);
				set(provider_data->PO_Provider, MUIA_Text_Contents, "");
				set(provider_data->PO_PoP, MUIA_Text_Contents, "");
			}
			if(!stricmp(pc_data.Argument, "Provider"))
			{
				char path[MAXPATHLEN];

				nnset(provider_data->PO_Provider, MUIA_Text_Contents, pc_data.Contents);
				strcpy(path, "AmiTCP:Providers");
				AddPart(path, (STRPTR)xget(provider_data->PO_Country, MUIA_Text_Contents), MAXPATHLEN);
				AddPart(path, pc_data.Contents, MAXPATHLEN);
				DoMethod(data->GR_Provider, MUIM_Provider_PopList_Update, path, MUIV_Provider_PopString_PoP);
				set(provider_data->PO_PoP, MUIA_Text_Contents, "");
			}
			if(!stricmp(pc_data.Argument, "PoP"))
				set(provider_data->PO_PoP, MUIA_Text_Contents, pc_data.Contents);
		}
		ParseEnd(&pc_data);



	/**** load the provider description ****/
		{
			char info_file[MAXPATHLEN];
			STRPTR ptr;

			strcpy(info_file, "AmiTCP:Providers");
			AddPart(info_file, (STRPTR)xget(provider_data->PO_Country, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(info_file, (STRPTR)xget(provider_data->PO_Provider, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(info_file, (STRPTR)xget(provider_data->PO_PoP, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(info_file, "provider.txt", MAXPATHLEN);
			DoMethod(data->GR_InfoWindow, MUIM_InfoWindow_LoadFile, info_file);

	/**** load the dialscript ****/

			if(ptr = FilePart(info_file))
				*ptr = NULL;
			AddPart(info_file, "DialScript", MAXPATHLEN);
			if(msg->file)
				file = info_file;
			else
			{
				if(get_file_size("ENV:NetConfig/DialScript") > -1)
					file = "ENV:NetConfig/DialScript";
				else
					file = info_file;
			}
			editor_load(file, provider_data->LV_DialScript);
		}
	}
	else
	{
		DoMethod(infowindow_data->LV_Info, MUIM_List_Clear);
	}

	/**** load user-startnet ****/

	editor_load(((get_file_size("ENV:NetConfig/User-Startnet") > -1) ? "ENV:NetConfig/User-Startnet" : "AmiTCP:db/User-Startnet"), user_data->LV_UserStartnet);


	/**** load the user data ****/

	if(ParseConfig("ENV:NetConfig/User.conf", &pc_data))
	{
		while(ParseNext(&pc_data))
		{
			if(!stricmp(pc_data.Argument, "UserName"))
				setstring(user_data->STR_UserName, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Password"))
				setstring(user_data->STR_Password, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "EMail"))
				setstring(user_data->STR_EMail, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "RealName"))
				setstring(user_data->STR_RealName, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Organisation"))
				setstring(user_data->STR_Organisation, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "HostName"))
				setstring(user_data->STR_HostName, pc_data.Contents);

			if(!stricmp(pc_data.Argument, "Modem"))
				set(modem_data->TX_Modem, MUIA_Text_Contents, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "ModemInit"))						// is in ENV:ModemInitString
				setstring(modem_data->STR_ModemInit, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "DialPrefix"))						// is in ENV:ModemDialPrefix
				setstring(modem_data->PO_DialPrefix, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "DialSuffix"))
				setstring(modem_data->STR_DialSuffix, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Device"))
				setstring(modem_data->PA_SerialDriver, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Unit"))
				set(modem_data->STR_SerialUnit, MUIA_String_Integer, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "Baud"))
				set(modem_data->PO_BaudRate, MUIA_String_Integer, atol(pc_data.Contents));
			if(!stricmp(pc_data.Argument, "RedialAttempts"))
				setslider(modem_data->SL_RedialAttempts, atol(pc_data.Contents));
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

	if(ParseConfig("NetConnect:Data/ModemSettings", &pc_data))
	{
		struct Modem *modem;

		if(modem = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY))
		{
			set(modem_data->LV_Modem, MUIA_List_Quiet, TRUE);
			DoMethod(modem_data->LV_Modem, MUIM_List_Clear);
			while(ParseNext(&pc_data))
			{
				strncpy(modem->Name, pc_data.Argument, 80);
				strncpy(modem->InitString, pc_data.Contents, 80);
				DoMethod(modem_data->LV_Modem, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
			}
			set(modem_data->LV_Modem, MUIA_List_Quiet, FALSE);
			FreeVec(modem);
		}
		ParseEnd(&pc_data);
	}

/*
	/**** parse the passwd file ****/

	DoMethod(users_data->LV_User, MUIM_List_Clear);
	if(ParseConfig("AmiTCP:db/passwd", &pc_data))
	{
		struct User *user;
		STRPTR ptr;

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
										user->Disabled = !(strcmp(user->Password, "*"));
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
	DoMethod(data->GR_Users, MUIM_Users_SetUserStates);


	/**** parse the group file ****/

	DoMethod(users_data->LV_Groups, MUIM_List_Clear);
	if(ParseConfig("AmiTCP:db/group", &pc_data))
	{
		struct Group *group;
		STRPTR ptr;

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
	DoMethod(data->GR_Users, MUIM_Users_SetGroupStates);
*/

	return(NULL);
}

ULONG AmiTCPPrefs_Finish(struct IClass *cl, Object *obj, struct MUIP_AmiTCPPrefs_Finish *msg)
{
	struct AmiTCPPrefs_Data	*data				= INST_DATA(cl								, obj);
	struct Provider_Data		*provider_data	= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
	struct User_Data			*user_data		= INST_DATA(CL_User->mcc_Class		, data->GR_User);
	struct Modem_Data			*modem_data		= INST_DATA(CL_Modem->mcc_Class		, data->GR_Modem);
	struct Paths_Data			*paths_data		= INST_DATA(CL_Paths->mcc_Class		, data->GR_Paths);
	BPTR fh;


	if(msg->level)
	{
		if(get_file_size("ENV:NetConfig") != -2)
		{
			BPTR lock;
			if(lock = CreateDir("ENV:NetConfig"))
				UnLock(lock);
		}
/*		if(fh = Open("ENV:NetConfig/Resolv.conf", MODE_NEWFILE))
		{
			FPrintf(fh, "; Name servers\n");
			FPrintf(fh, "NAMESERVER %ls\n", xget(provider_data->STR_NameServer1, MUIA_String_Contents));
			FPrintf(fh, "NAMESERVER %ls\n", xget(provider_data->STR_NameServer2, MUIA_String_Contents));
			FPrintf(fh, "; Domain names\n");
			FPrintf(fh, "DOMAIN %ls\n", xget(provider_data->STR_DomainName, MUIA_String_Contents));

			Close(fh);
		}
*/		if(fh = Open("ENV:NetConfig/Provider.conf", MODE_NEWFILE))
		{
			FPrintf(fh, "/* Provider Configuration Follows:\n");
			FPrintf(fh, "Country            \"%ls\"\n", xget(provider_data->PO_Country, MUIA_Text_Contents));
			FPrintf(fh, "Provider           \"%ls\"\n", xget(provider_data->PO_Provider, MUIA_Text_Contents));
			FPrintf(fh, "PoP                \"%ls\"\n", xget(provider_data->PO_PoP, MUIA_Text_Contents));
			FPrintf(fh, "DialUp             1\n");
			FPrintf(fh, "Interface          %ls\n", (xget(provider_data->RA_Interface		, MUIA_Radio_Active) ? "slip" : "ppp"));
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
			FPrintf(fh, "IPDynamic          %ld\n", (xget(provider_data->RA_Connection		, MUIA_Radio_Active) ? 0 : 1));
			FPrintf(fh, "IPAddr             %ls\n", xget(user_data->STR_IP_Address		, MUIA_String_Contents));
			FPrintf(fh, "DestIP             \n");
			FPrintf(fh, "NSDynamic          1\n");
			FPrintf(fh, "NameServer         %ls\n", xget(provider_data->STR_NameServer1, MUIA_String_Contents));
			FPrintf(fh, "NameServer         %ls\n", xget(provider_data->STR_NameServer2, MUIA_String_Contents));
			FPrintf(fh, "DomainName         \"%ls\"\n", xget(provider_data->STR_DomainName, MUIA_String_Contents));
			FPrintf(fh, "UseBootP           %ld\n", (xget(provider_data->CH_BOOTP				, MUIA_Selected) ? 1 : 0));
			FPrintf(fh, "MTU                %ld\n", xget(provider_data->SL_MTU			, MUIA_Numeric_Value));
			FPrintf(fh, "Phone              \"%ls\"\n", xget(provider_data->STR_Phone			, MUIA_String_Contents));

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
			FPrintf(fh, "POPServer          \"%ls\"\n", xget(provider_data->STR_POPServer	, MUIA_String_Contents));
			FPrintf(fh, "NewsServer         \"%ls\"\n", xget(provider_data->STR_NewsServer	, MUIA_String_Contents));
			FPrintf(fh, "IRCServer          \"%ls\"\n", xget(provider_data->STR_IRCServer	, MUIA_String_Contents));
			FPrintf(fh, "WWWServer          \"%ls\"\n", xget(provider_data->STR_WWWServer	, MUIA_String_Contents));
			FPrintf(fh, "*/\n");

			Close(fh);
		}
		if(fh = Open("ENV:NetConfig/User.conf", MODE_NEWFILE))
		{
			FPrintf(fh, "UserName           \"%ls\"\n", xget(user_data->STR_UserName		, MUIA_String_Contents));
			FPrintf(fh, "Password           \"%ls\"\n", xget(user_data->STR_Password		, MUIA_String_Contents));
			FPrintf(fh, "EMail              \"%ls\"\n", xget(user_data->STR_EMail			, MUIA_String_Contents));
			FPrintf(fh, "RealName           \"%ls\"\n", xget(user_data->STR_RealName		, MUIA_String_Contents));
			FPrintf(fh, "Organisation       \"%ls\"\n", xget(user_data->STR_Organisation, MUIA_String_Contents));
			FPrintf(fh, "HostName           \"%ls\"\n", xget(user_data->STR_HostName		, MUIA_String_Contents));

			FPrintf(fh, "Modem              \"%ls\"\n", xget(modem_data->TX_Modem			, MUIA_Text_Contents));
			FPrintf(fh, "ModemInit          \"%ls\"\n", xget(modem_data->STR_ModemInit	, MUIA_String_Contents));
			FPrintf(fh, "DialPrefix         \"%ls\"\n", xget(modem_data->PO_DialPrefix	, MUIA_String_Contents));
			FPrintf(fh, "DialSuffix         \"%ls\"\n", xget(modem_data->STR_DialSuffix	, MUIA_String_Contents));
			FPrintf(fh, "Device             \"%ls\"\n", xget(modem_data->PA_SerialDriver, MUIA_String_Contents));
			FPrintf(fh, "Unit               %ld\n", xget(modem_data->STR_SerialUnit	, MUIA_String_Integer));
			FPrintf(fh, "Baud               %ld\n", xget(modem_data->PO_BaudRate		, MUIA_String_Integer));
			FPrintf(fh, "RedialAttempts     %ld\n", xget(modem_data->SL_RedialAttempts, MUIA_Numeric_Value));
			FPrintf(fh, "CarrierDetect      %ld\n", xget(modem_data->CH_Carrier		, MUIA_Selected));
			FPrintf(fh, "7Wire              %ld\n", xget(modem_data->CH_7Wire			, MUIA_Selected));
			FPrintf(fh, "OwnDevUnit         %ld\n", xget(modem_data->CH_OwnDevUnit	, MUIA_Selected));

			FPrintf(fh, "MailIn             \"%ls\"\n", xget(paths_data->PA_MailIn		, MUIA_String_Contents));
			FPrintf(fh, "MailOut            \"%ls\"\n", xget(paths_data->PA_MailOut		, MUIA_String_Contents));
			FPrintf(fh, "NewsIn             \"%ls\"\n", xget(paths_data->PA_NewsIn		, MUIA_String_Contents));
			FPrintf(fh, "NewsOut            \"%ls\"\n", xget(paths_data->PA_NewsOut		, MUIA_String_Contents));
			FPrintf(fh, "FileIn             \"%ls\"\n", xget(paths_data->PA_FileIn		, MUIA_String_Contents));
			FPrintf(fh, "FileOut            \"%ls\"\n", xget(paths_data->PA_FileOut		, MUIA_String_Contents));

			Close(fh);
		}

		editor_save("ENV:NetConfig/DialScript", provider_data->LV_DialScript);
		editor_save("ENV:NetConfig/User-Startnet", user_data->LV_UserStartnet);

		switch(xget(provider_data->CY_Authentication, MUIA_Cycle_Active))
		{
			case 1:
				if(fh = Open("ENV:NetConfig/CHAP_Passwordfile", MODE_NEWFILE))
				{
					FPrintf(fh, "%ls %ls %ls\n", xget(provider_data->STR_HostID, MUIA_String_Contents), xget(provider_data->STR_Password, MUIA_String_Contents), xget(provider_data->STR_YourID, MUIA_String_Contents));
					Close(fh);
				}
				break;
			case 2:
				if(fh = Open("ENV:NetConfig/PAP_Passwordfile", MODE_NEWFILE))
				{
					FPrintf(fh, "%ls %ls\n", xget(provider_data->STR_YourID, MUIA_String_Contents), xget(provider_data->STR_Password, MUIA_String_Contents));
					Close(fh);
				}
				break;
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
		CopyFile("ENV:NetConfig/Resolv.conf"			, "ENVARC:NetConfig/Resolv.conf");
		CopyFile("ENV:NetConfig/Provider.conf"			, "ENVARC:NetConfig/Provider.conf");
		CopyFile("ENV:NetConfig/User.conf"				, "ENVARC:NetConfig/User.conf");
		CopyFile("ENV:NetConfig/DialScript"				, "ENVARC:NetConfig/DialScript");
		CopyFile("ENV:NetConfig/User-Startnet"			, "ENVARC:NetConfig/User-Startnet");
		CopyFile("ENV:NetConfig/PAP_Passwordfile"		, "ENVARC:NetConfig/PAP_Passwordfile");
		CopyFile("ENV:NetConfig/CHAP_Passwordfile"	, "ENVARC:NetConfig/CHAP_Passwordfile");
	}

// write necessary info to env:netconfig/autointerfaces

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
				new = data->GR_Paths;
				break;
//			case 5:
//				new = data->GR_Users;
//				break;
		}

		if(new)
		{
			DoMethod(data->GR_Pager, MUIM_Group_InitChange);
			DoMethod(data->GR_Temp, MUIM_Group_InitChange);

			DoMethod(data->GR_Pager, OM_REMMEMBER, data->GR_Active);
			DoMethod(data->GR_Temp, OM_ADDMEMBER, data->GR_Active);
			DoMethod(data->GR_Temp, OM_REMMEMBER, new);
			DoMethod(data->GR_Pager, OM_ADDMEMBER, new);
			data->GR_Active = new;

			DoMethod(data->GR_Pager, MUIM_Group_ExitChange);
			DoMethod(data->GR_Temp, MUIM_Group_ExitChange);
		}
	}

	return(NULL);
}

ULONG AmiTCPPrefs_InitGroups(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);
	BOOL success = FALSE;

	data->GR_Provider			= data->GR_User		=
	data->GR_Modem				= data->GR_Paths		=
	NULL;
//	data->GR_Users	= NULL;

	if(data->GR_Temp = GroupObject, End)
	{
		DoMethod(app, OM_ADDMEMBER, data->GR_Temp);
		
		if(data->GR_Provider		= NewObject(CL_Provider->mcc_Class	, NULL, TAG_DONE))
			DoMethod(data->GR_Temp, OM_ADDMEMBER, data->GR_Provider);
		if(data->GR_User			= NewObject(CL_User->mcc_Class		, NULL, TAG_DONE))
			DoMethod(data->GR_Temp, OM_ADDMEMBER, data->GR_User);
		if(data->GR_Modem			= NewObject(CL_Modem->mcc_Class		, NULL, TAG_DONE))
			DoMethod(data->GR_Temp, OM_ADDMEMBER, data->GR_Modem);
		if(data->GR_Paths			= NewObject(CL_Paths->mcc_Class		, NULL, TAG_DONE))
			DoMethod(data->GR_Temp, OM_ADDMEMBER, data->GR_Paths);
//		if(data->GR_Users			= NewObject(CL_Users->mcc_Class		, NULL, TAG_DONE))
//			DoMethod(data->GR_Temp, OM_ADDMEMBER, data->GR_Users);
		if(data->GR_InfoWindow	= NewObject(CL_InfoWindow->mcc_Class, NULL, TAG_DONE))
			DoMethod(app, OM_ADDMEMBER, data->GR_InfoWindow);

		if(data->GR_Provider	&& data->GR_User	&& data->GR_Modem &&
			data->GR_Paths		/* && data->GR_Users */)
		{
			struct Provider_Data		*provider_data		= INST_DATA(CL_Provider->mcc_Class	, data->GR_Provider);
			struct User_Data			*user_data			= INST_DATA(CL_User->mcc_Class		, data->GR_User);
//			struct Modem_Data			*modem_data			= INST_DATA(CL_Modem->mcc_Class		, data->GR_Modem);
//			struct Paths_Data			*paths_data			= INST_DATA(CL_Paths->mcc_Class		, data->GR_Paths);
//			struct Users_Data			*users_data			= INST_DATA(CL_Users->mcc_Class		, data->GR_Users);
//			struct InfoWindow_Data	*infowindow_data	= INST_DATA(CL_InfoWindow->mcc_Class, data->GR_InfoWindow);

			success = TRUE;

// Set the notification between the groups

			DoMethod(provider_data->RA_Connection, MUIM_Notify, MUIA_Radio_Active, MUIV_EveryTime , user_data->STR_IP_Address, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
			DoMethod(data->GR_InfoWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE , provider_data->CH_ProviderInfo, 3, MUIM_Set, MUIA_Selected, FALSE);
			DoMethod(provider_data->CH_ProviderInfo, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, data->GR_InfoWindow, 3, MUIM_Set, MUIA_Window_Open, MUIV_TriggerValue);
		}
	}

	return(success);
}

SAVEDS ASM VOID HelpFunc(REG(a2) Object *help,REG(a1) Object **objptr)
{
	if(*objptr)
	{
		if(xget(*objptr, MUIA_UserData))
			set(help, MUIA_Floattext_Text, xget(*objptr, MUIA_UserData));
	}
}

ULONG AmiTCPPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static struct Hook HelpHook = { {0,0}, (VOID *)HelpFunc, 0, 0 };
	struct AmiTCPPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title	, GetStr(MSG_WI_AmiTCPPrefs),
		MUIA_Window_ID		, MAKE_ID('A','R','E','F'),
		MUIA_Window_NeedsMouseObject, TRUE,
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, AmiTCPPrefsMenu,0),
		WindowContents		, VGroup,
			Child, tmp.GR_Pager = HGroup,
				Child, VGroup,
					Child, tmp.LV_Pager = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_List			, tmp.LI_Pager = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_SourceArray	, ARR_Pages,
						End,
					End,
					Child, HGroup,
						Child, MakeKeyLabel2("  Extra Help", "  x"),
						Child, tmp.CH_ExtraHelp = KeyCheckMark(FALSE, 'x'),
					End,
				End,
				Child, tmp.GR_Active = tmp.GR_Info = VGroup,
					GroupFrame,
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
					Child, CLabel("AmiTCP Config v0.53 -beta-  (20.07.96)"),
					Child, HVSpace,
					Child, CLabel("\33bTHIS IS A DEMO - DO NOT REDISTRIBUTE !!!"),
					Child, HVSpace,
				End,
			End,
			Child, tmp.BL_ExtraHelp = BalanceObject,
				MUIA_ShowMe, FALSE,
			End,
			Child, tmp.LV_ExtraHelp = ListviewObject,
				MUIA_CycleChain		, 1,
				MUIA_ShowMe				, FALSE,
				MUIA_Listview_Input	, FALSE,
				MUIA_Listview_List	, tmp.FT_ExtraHelp = FloattextObject,
					MUIA_Floattext_Justify	, TRUE,
					MUIA_Frame					, MUIV_Frame_InputList,
				End,
			End,
			Child, HGroup,
				MUIA_HelpNode			, "GR_PrefsControl",
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

		set(tmp.LV_Pager, MUIA_List_Active, MUIV_List_Active_Top);
		set(tmp.CH_ExtraHelp, MUIA_CycleChain, 1);

		set(tmp.LI_Pager, MUIA_UserData, "Here you can select the page");
		set(tmp.CH_ExtraHelp, MUIA_UserData, "Show/Hide this information box");
		set(tmp.BT_Save, MUIA_UserData, "Save all changes permanently");
		set(tmp.BT_Use, MUIA_UserData, "Save changes temporarely. They will be lost next time you reboot your Amiga.");
		set(tmp.BT_Cancel, MUIA_UserData, "Abandon all changes");

		DoMethod(obj				, MUIM_Notify, MUIA_Window_CloseRequest, TRUE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 0);
		DoMethod(tmp.LV_Pager	, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime	, obj, 1, MUIM_AmiTCPPrefs_SetPage);
		DoMethod(tmp.BT_Cancel	, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 0);
		DoMethod(tmp.BT_Use		, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 1);
		DoMethod(tmp.BT_Save		, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 2);
		DoMethod(tmp.CH_ExtraHelp, MUIM_Notify, MUIA_Selected, MUIV_EveryTime		, tmp.CH_ExtraHelp, 5, MUIM_MultiSet, MUIA_ShowMe, MUIV_TriggerValue, tmp.BL_ExtraHelp, tmp.LV_ExtraHelp);

		DoMethod(obj, MUIM_Notify, MUIA_Window_MouseObject, MUIV_EveryTime, tmp.FT_ExtraHelp, 3, MUIM_CallHook, &HelpHook, MUIV_TriggerValue);

		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_ABOUT)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_AmiTCPPrefs_About);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_QUIT)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_MUI)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
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
		case MUIM_AmiTCPPrefs_LoadConfig		: return(AmiTCPPrefs_LoadConfig	(cl, obj, (APTR)msg));
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



/*
 * close our custom classes
 */

VOID exit_classes(VOID)
{
//	if(CL_Users)			MUI_DeleteCustomClass(CL_Users);
	if(CL_Paths)			MUI_DeleteCustomClass(CL_Paths);
	if(CL_Modem)			MUI_DeleteCustomClass(CL_Modem);
	if(CL_User)				MUI_DeleteCustomClass(CL_User);
	if(CL_Provider)		MUI_DeleteCustomClass(CL_Provider);
	if(CL_AmiTCPPrefs)	MUI_DeleteCustomClass(CL_AmiTCPPrefs);
	if(CL_About)			MUI_DeleteCustomClass(CL_About);
	if(CL_MemberList)		MUI_DeleteCustomClass(CL_MemberList);
	if(CL_GroupIDString)	MUI_DeleteCustomClass(CL_GroupIDString);
	if(CL_InfoWindow)		MUI_DeleteCustomClass(CL_InfoWindow);

	CL_AmiTCPPrefs	= CL_User		= CL_Provider		=
	CL_Paths			= CL_Modem		= /*CL_Users			=*/
	CL_About			= CL_MemberList= CL_GroupIDString=
	CL_InfoWindow	= NULL;
}


/*
 * initialize our custom classes
 */

BOOL init_classes(VOID)
{
	CL_AmiTCPPrefs		= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct AmiTCPPrefs_Data), AmiTCPPrefs_Dispatcher);
	CL_User				= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct User_Data)			, User_Dispatcher);
	CL_Provider			= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Provider_Data)	, Provider_Dispatcher);
	CL_Paths				= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Paths_Data)		, Paths_Dispatcher);
	CL_Modem				= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Modem_Data)		, Modem_Dispatcher);
//	CL_Users				= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Users_Data)		, Users_Dispatcher);
	CL_About				= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct About_Data)		, About_Dispatcher);
	CL_MemberList		= MUI_CreateCustomClass(NULL, MUIC_List	, NULL, sizeof(struct About_Data)		, MemberList_Dispatcher);
	CL_GroupIDString	= MUI_CreateCustomClass(NULL, MUIC_String	, NULL, sizeof(struct About_Data)		, GroupIDString_Dispatcher);
	CL_InfoWindow		= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct InfoWindow_Data)	, InfoWindow_Dispatcher);

	if(CL_AmiTCPPrefs		&& CL_User			&& CL_Provider	&&
		CL_Paths				&& CL_Modem			&& CL_About 	&&
		/*CL_Users				&& */ CL_MemberList	&&
		CL_GroupIDString	&& CL_InfoWindow)
		return(TRUE);

	exit_classes();
	return(FALSE);
}
