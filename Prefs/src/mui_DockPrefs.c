#include "mui_DockPrefs.h"

extern Object *app, *win;
extern struct MUI_CustomClass *CL_MainWindow, *CL_DockPrefs, *CL_EditIcon, *CL_IconList;
extern struct Hook IconList_DestructHook, IconList_ConstructHook;

ULONG DockPrefs_GetDock(struct IClass *cl, Object *obj, Msg msg)
{
   struct DockPrefs_Data *data = INST_DATA(cl, obj);

   if(data->dock)
   {
      data->dock->Name     = update_string(data->dock->Name    , (STRPTR)xget(data->STR_Name, MUIA_String_Contents));
      data->dock->Hotkey   = update_string(data->dock->Hotkey  , (STRPTR)xget(data->STR_Hotkey, MUIA_String_Contents));
      data->dock->Font     = update_string(data->dock->Font    , (STRPTR)xget(data->PA_Font, MUIA_String_Contents));
      data->dock->Rows     = xget(data->SL_Rows       , MUIA_Numeric_Value);
      data->dock->Flags    = 0;
      data->dock->Flags    |= (xget(data->CH_Activate    , MUIA_Selected) ? DFL_Activate  : 0);
      data->dock->Flags    |= (xget(data->CH_PopUp       , MUIA_Selected) ? DFL_PopUp     : 0);
      data->dock->Flags    |= (xget(data->CH_Backdrop    , MUIA_Selected) ? DFL_Backdrop  : 0);
      data->dock->Flags    |= (xget(data->CH_Frontmost   , MUIA_Selected) ? DFL_Frontmost : 0);
      data->dock->Flags    |= (xget(data->CH_DragBar     , MUIA_Selected) ? DFL_DragBar   : 0);
      data->dock->Flags    |= (xget(data->CH_Borderless  , MUIA_Selected) ? DFL_Borderless: 0);
      switch(xget(data->CY_ButtonType, MUIA_Cycle_Active))
      {
         case 1:
            data->dock->Flags |= DFL_Icon;
            break;
         case 2:
            data->dock->Flags |= DFL_Text;
            break;
         default:
            data->dock->Flags |= DFL_Icon | DFL_Text;
            break;
      }
   }

   return(NULL);
}

ULONG DockPrefs_SetDock(struct IClass *cl, Object *obj, struct MUIP_DockPrefs_SetDock *msg)
{
   struct DockPrefs_Data *data = INST_DATA(cl, obj);
   struct Icon *icon = NULL;
   struct Dock *dock;
   int i;
   BOOL redraw = FALSE;

   set(data->LI_ActiveIcons, MUIA_List_Quiet, TRUE);
   /** make sure all inactive icons have got icon->list initialised **/
   i = 0;
   FOREVER
   {
      DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, i++, &icon);
      if(!icon)
         break;
      if(!icon->list && icon->bodychunk)
      {
         icon->list = (APTR)DoMethod(data->LI_InactiveIcons, MUIM_List_CreateImage, icon->bodychunk, 0);
         redraw = TRUE;
      }
   }
   if(redraw)
      DoMethod(data->LI_InactiveIcons, MUIM_List_Redraw, MUIV_List_Redraw_All);


   /** clean up the old entries first **/
   DoMethod(data->LI_ActiveIcons, MUIM_IconList_DeleteAllImages);
   DoMethod(data->LI_ActiveIcons, MUIM_List_Clear);
   set(data->SL_Rows, MUIA_Slider_Max, 1);

   DoMethod(data->LI_Docks, MUIM_List_GetEntry, msg->entry, &dock);
   if(dock)
   {
      i = 0;
      FOREVER
      {
         DoMethod(dock->LI_Buttons, MUIM_List_GetEntry, i++, &icon);
         if(!icon)
            break;
         if(!icon->list && icon->bodychunk)
            icon->list = (APTR)DoMethod(data->LI_ActiveIcons, MUIM_List_CreateImage, icon->bodychunk, 0);
         DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
      }
      set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));

      setstring(data->STR_Name, dock->Name);
      setstring(data->STR_Hotkey, dock->Hotkey);
      setstring(data->PA_Font, dock->Font);
      setslider(data->SL_Rows, dock->Rows);
      setcycle(data->CY_ButtonType, (dock->Flags & DFL_Icon ? (dock->Flags & DFL_Text ? 0 : 1) : 2));
      set(data->CH_Activate   , MUIA_Selected, (dock->Flags & DFL_Activate));
      set(data->CH_PopUp      , MUIA_Selected, (dock->Flags & DFL_PopUp));
      set(data->CH_Backdrop   , MUIA_Selected, (dock->Flags & DFL_Backdrop));
      set(data->CH_Frontmost  , MUIA_Selected, (dock->Flags & DFL_Frontmost));
      set(data->CH_DragBar    , MUIA_Selected, (dock->Flags & DFL_DragBar));
      set(data->CH_Borderless , MUIA_Selected, (dock->Flags & DFL_Borderless));
   }
   set(data->LI_ActiveIcons, MUIA_List_Quiet, FALSE);

   data->dock = dock;

   return(NULL);
}

ULONG DockPrefs_NewIcon(struct IClass *cl, Object *obj, Msg msg)
{
#ifndef DEMO
   struct DockPrefs_Data *data = INST_DATA(cl, obj);
   struct Icon *icon;

   if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
   {
      icon->Name        = update_string(NULL, GetStr(MSG_TX_New));
      icon->Program.File= update_string(NULL, PROGRAM_PATH);
      icon->ImageFile   = update_string(NULL, IMAGE_PATH);
      icon->Program.Flags = PRG_Arguments;
      icon->Program.Stack = 8192;
      icon->Volume = 64;
      icon->Flags = IFL_DrawFrame;
      init_icon(icon);
      if(icon->bodychunk)
         icon->list = (APTR)DoMethod(data->LI_InactiveIcons, MUIM_List_CreateImage, icon->bodychunk, 0);
      DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
      set(data->LI_InactiveIcons, MUIA_List_Active, MUIV_List_Active_Bottom);
      FreeVec(icon);

      DoMethod(obj, MUIM_DockPrefs_EditIcon);
   }
#endif

   return(NULL);
}

#ifdef __SASC
SAVEDS ASM LONG IconList_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x) {
#else /* gcc */
LONG IconList_AppMsgFunc()
{
   register APTR obj __asm("a2");
   register struct AppMessage **x __asm("a1");
#endif

   struct MainWindow_Data *main_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct DockPrefs_Data *data = INST_DATA(CL_DockPrefs->mcc_Class, main_data->GR_Dock);
   struct WBArg *ap;
   struct AppMessage *amsg = *x;
   STRPTR buf;
   struct Icon icon, *icon_ptr;

   bzero(&icon, sizeof(struct Icon));
   ap = amsg->am_ArgList;
   if(amsg->am_NumArgs)
   {
      if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
      {
         NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
         AddPart(buf, ap->wa_Name, MAXPATHLEN);

         if(buf[strlen(buf) - 1] == '/')
            buf[strlen(buf) - 1] = NULL;
         if(buf[strlen(buf) - 1] == ':')
            AddPart(buf, "Disk.info", MAXPATHLEN);

         icon.Name            = update_string(NULL, (*ap->wa_Name ? (STRPTR)ap->wa_Name : FilePart(buf)));
         icon.Program.File    = update_string(NULL, buf);
         icon.ImageFile       = update_string(NULL, buf);
         icon.Program.Flags   = PRG_Arguments | PRG_WORKBENCH | PRG_Asynch;
         icon.Program.Stack   = 8192;
         icon.Volume          = 64;
         icon.Flags           = IFL_DrawFrame;

         init_icon(&icon);
         if(icon.bodychunk)
            icon.list = (APTR)DoMethod(obj, MUIM_List_CreateImage, icon.bodychunk, 0);

         if(obj == data->LI_InactiveIcons)
            DoMethod(obj, MUIM_List_InsertSingle, &icon, MUIV_List_Insert_Bottom);
         else
         {
            DoMethod(data->dock->LI_Buttons, MUIM_List_InsertSingle, &icon, MUIV_List_Insert_Bottom);
            DoMethod(data->dock->LI_Buttons, MUIM_List_GetEntry, xget(data->dock->LI_Buttons, MUIA_List_Entries) - 1, &icon_ptr);
            DoMethod(obj, MUIM_List_InsertSingle, icon_ptr, MUIV_List_Insert_Bottom);
         }

         set(obj, MUIA_List_Active, MUIV_List_Active_Bottom);

         FreeVec(buf);
      }
   }
   return(NULL);
}

ULONG DockPrefs_CopyIcon(struct IClass *cl, Object *obj, Msg msg)
{
#ifndef DEMO
   struct DockPrefs_Data *data = INST_DATA(cl, obj);
   struct Icon *icon = NULL, *new_icon;
   Object *list = NULL;
   int active;

   find_list(data, &list, &icon);
   if(icon && list)
   {
      active = xget(list, MUIA_List_Active) + 1;

      if(new_icon = AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
      {
         memcpy(new_icon, icon, sizeof(struct Icon));

         new_icon->Name                = update_string(NULL, icon->Name);
         new_icon->Hotkey              = update_string(NULL, icon->Hotkey);
         new_icon->ImageFile           = update_string(NULL, icon->ImageFile);
         new_icon->Sound               = update_string(NULL, icon->Sound);
         new_icon->Program.File        = update_string(NULL, icon->Program.File);
         new_icon->Program.CurrentDir  = update_string(NULL, icon->Program.CurrentDir);
         new_icon->Program.OutputFile  = update_string(NULL, icon->Program.OutputFile);
         new_icon->Program.PublicScreen= update_string(NULL, icon->Program.PublicScreen);

         init_icon(new_icon);
         if(new_icon->bodychunk)
            new_icon->list = (APTR)DoMethod(list, MUIM_List_CreateImage, new_icon->bodychunk, 0);
         if(list == data->LI_InactiveIcons)
            DoMethod(list, MUIM_List_InsertSingle, new_icon, active);
         else
         {
            DoMethod(data->dock->LI_Buttons, MUIM_List_InsertSingle, new_icon, active);
            DoMethod(data->dock->LI_Buttons, MUIM_List_GetEntry, active, &icon);
            DoMethod(list, MUIM_List_InsertSingle, icon, active);
         }
         set(list, MUIA_List_Active, active);
         FreeVec(new_icon);
      }
   }
#endif

   return(NULL);
}

ULONG DockPrefs_DeleteIcon(struct IClass *cl, Object *obj, Msg msg)
{
   struct DockPrefs_Data *data = INST_DATA(cl, obj);
   struct Icon *icon = NULL;
   Object *list = NULL;
   int active;

   find_list(data, &list, &icon);
   if(icon && list)
   {
      active = xget(list, MUIA_List_Active);

      if(icon->list)
         DoMethod(list, MUIM_List_DeleteImage, icon->list);
      icon->list = NULL;
      DoMethod(list, MUIM_List_Remove, MUIV_List_Remove_Active);

      if((list == data->LI_ActiveIcons) && data->dock)
         DoMethod(data->dock->LI_Buttons, MUIM_List_Remove, active);
   }

   return(NULL);
}

ULONG DockPrefs_EditIcon(struct IClass *cl, Object *obj, Msg msg)
{
   struct DockPrefs_Data *data = INST_DATA(cl, obj);
   struct Icon *icon = NULL;
   Object *list = NULL;

   find_list(data, &list, &icon);
   if(icon && list)
   {
      set(app, MUIA_Application_Sleep, TRUE);
      if(icon->edit_window = (Object *)NewObject(CL_EditIcon->mcc_Class, NULL,
         MUIA_NetConnect_Icon       , icon,
         MUIA_NetConnect_List       , list,
         MUIA_NetConnect_Originator , obj,
         TAG_DONE))
      {
         struct EditIcon_Data *ei_data = INST_DATA(CL_EditIcon->mcc_Class, icon->edit_window);

         DoMethod(app, OM_ADDMEMBER, icon->edit_window);

         /** do the necessary stuff for initialisation **/
// is this allowed ???
         set(ei_data->CY_Type, MUIA_Cycle_Active, (icon->Program.Flags & PRG_AREXX ? 3 : (icon->Program.Flags & PRG_SCRIPT ? 2 : (icon->Program.Flags & PRG_WORKBENCH ? 1 : 0))));
         DoMethod(icon->edit_window, MUIM_EditIcon_Sound_Active);

         set(icon->edit_window, MUIA_Window_Open, TRUE);
      }
      set(app, MUIA_Application_Sleep, FALSE);
      set(win, MUIA_Window_Sleep, TRUE);
   }

   return(NULL);
}

ULONG DockPrefs_EditIcon_Finish(struct IClass *cl, Object *obj, struct MUIP_DockPrefs_EditIcon_Finish *msg)
{
   struct Icon *icon = msg->icon;
   Object *list = msg->list, *window = icon->edit_window;
   struct EditIcon_Data *data = INST_DATA(CL_EditIcon->mcc_Class, window);

   if(!icon || !list || !data || !window)
      return(NULL);

   editor_checksave((STRPTR)xget(data->PA_Program, MUIA_String_Contents), data->LI_Editor);

   if(msg->use)
   {
      icon->Name                 = update_string(icon->Name                   , (STRPTR)xget(data->STR_Name          , MUIA_String_Contents));
      icon->Hotkey               = update_string(icon->Hotkey                 , (STRPTR)xget(data->STR_Hotkey        , MUIA_String_Contents));
      icon->Sound                = update_string(icon->Sound                  , (STRPTR)xget(data->PA_Sound          , MUIA_String_Contents));
      icon->Program.File         = update_string(icon->Program.File           , (STRPTR)xget(data->PA_Program        , MUIA_String_Contents));
      icon->Program.CurrentDir   = update_string(icon->Program.CurrentDir     , (STRPTR)xget(data->PA_CurrentDir     , MUIA_String_Contents));
      icon->Program.OutputFile   = update_string(icon->Program.OutputFile     , (STRPTR)xget(data->PA_OutputFile     , MUIA_String_Contents));
      icon->Program.PublicScreen = update_string(icon->Program.PublicScreen   , (STRPTR)xget(data->STR_PublicScreen  , MUIA_String_Contents));

      icon->Program.Stack     = xget(data->STR_Stack  , MUIA_String_Integer);
      icon->Program.Priority  = xget(data->SL_Priority, MUIA_Numeric_Value);
      icon->Program.Flags     = (xget(data->CH_WBArgs, MUIA_Selected) ? PRG_Arguments : 0) | (xget(data->CH_ToFront, MUIA_Selected) ? PRG_ToFront : 0);
      icon->Volume            = xget(data->SL_Volume  , MUIA_Numeric_Value);
      switch(xget(data->CY_Type, MUIA_Cycle_Active))
      {
         case 1:
            icon->Program.Flags |= PRG_WORKBENCH;
            break;
         case 2:
            icon->Program.Flags |= PRG_SCRIPT;
            break;
         case 3:
            icon->Program.Flags |= PRG_AREXX;
            break;
         default:
            icon->Program.Flags |= PRG_CLI;
            break;
      }

      if((!icon->ImageFile && strlen((STRPTR)xget(data->PA_Image, MUIA_String_Contents))) || (strcmp(icon->ImageFile, (STRPTR)xget(data->PA_Image, MUIA_String_Contents))) || (xget(data->CH_DrawFrame, MUIA_Selected) != (icon->Flags & IFL_DrawFrame)))
      {
         set(list, MUIA_List_Quiet, TRUE);
         if(icon->list)
            DoMethod(list, MUIM_List_DeleteImage, icon->list);
         if(icon->bodychunk)
            MUI_DisposeObject(icon->bodychunk);
         if(icon->body)
            FreeVec(icon->body);
         if(icon->cols)
            FreeVec(icon->cols);
         if(icon->disk_object)
            FreeDiskObject(icon->disk_object);

         icon->Flags       = (xget(data->CH_DrawFrame, MUIA_Selected) ? IFL_DrawFrame : NULL);
         icon->ImageFile   = update_string(icon->ImageFile, (STRPTR)xget(data->PA_Image, MUIA_String_Contents));
         init_icon(icon);
         if(icon->bodychunk)
            icon->list = (APTR)DoMethod(list, MUIM_List_CreateImage, icon->bodychunk, 0);

         set(list, MUIA_List_Quiet, FALSE);
         DoMethod(list, MUIM_List_Redraw, MUIV_List_Redraw_All);
      }
      else
         DoMethod(list, MUIM_List_Redraw, MUIV_List_Redraw_Active);
   }

   set(window, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, window);
   MUI_DisposeObject(window);
   set(win, MUIA_Window_Sleep, FALSE);

   return(NULL);
}


ULONG DockPrefs_List_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct DockPrefs_Data *data = INST_DATA(cl, obj);
   struct Icon *icon = NULL;
   Object *list = NULL;
   LONG max;

   max = xget(data->LI_ActiveIcons, MUIA_List_Entries);
   if(max != xget(data->SL_Rows, MUIA_Slider_Max))
   {
      LONG pos;

      pos = xget(data->SL_Rows, MUIA_Numeric_Value);
      set(data->SL_Rows, MUIA_Slider_Max, (max > 0 ? max : 1));
      setslider(data->SL_Rows, pos);
   }

   if(find_list(data, &list, &icon))
   {
      set(data->BT_Delete, MUIA_Disabled, FALSE);
      set(data->BT_Edit, MUIA_Disabled, FALSE);
#ifndef DEMO
      set(data->BT_Copy, MUIA_Disabled, FALSE);
#endif
   }
   else
   {
      set(data->BT_Delete, MUIA_Disabled, TRUE);
      set(data->BT_Edit, MUIA_Disabled, TRUE);
      set(data->BT_Copy, MUIA_Disabled, TRUE);
   }

   return(NULL);
}

ULONG DockPrefs_Name_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct DockPrefs_Data *data = INST_DATA(cl, obj);
   struct MainWindow_Data *main_data = INST_DATA(CL_MainWindow->mcc_Class, win);

   data->dock->Name = update_string(data->dock->Name, (STRPTR)xget(data->STR_Name, MUIA_String_Contents));
   DoMethod(main_data->LI_Pager, MUIM_List_Redraw, MUIV_List_Redraw_Active);
   return(NULL);
}

ULONG DockPrefs_Type_Active(struct IClass *cl, Object *obj, Msg msg)
{
   struct DockPrefs_Data *data = INST_DATA(cl, obj);

   set(data->PA_Font, MUIA_Disabled, (xget(data->CY_ButtonType, MUIA_Cycle_Active) != 2));
   return(NULL);
}

#ifdef __SASC
SAVEDS ASM struct Dock *DockList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Dock *src) {
#else /* gcc */
struct Dock *DockList_ConstructFunc()
{
   register APTR pool __asm("a2");
   register struct Dock *src __asm("a1");
#endif

   struct Dock *new;

   if(new = (struct Dock *)AllocVec(sizeof(struct Dock), MEMF_ANY | MEMF_CLEAR))
   {
      if(src)
         memcpy(new, src, sizeof(struct Dock));

      new->LI_Buttons = ListObject,
         MUIA_List_ConstructHook , &IconList_ConstructHook,
         MUIA_List_DestructHook  , &IconList_DestructHook,
         End;
   }
   return(new);
}
struct Hook DockList_ConstructHook = { { 0,0 }, (VOID *)DockList_ConstructFunc  , NULL, NULL };

#ifdef __SASC
SAVEDS ASM VOID DockList_DestructFunc(REG(a2) APTR pool, REG(a1) struct Dock *dock) {
#else /* gcc */
VOID DockList_DestructFunc()
{
   register APTR pool __asm("a2");
   register struct Dock *dock __asm("a1");
#endif

   if(dock)
   {
      if(dock->Name)
         FreeVec(dock->Name);
      if(dock->Hotkey)
         FreeVec(dock->Hotkey);
      if(dock->Font)
         FreeVec(dock->Font);
      if(dock->LI_Buttons)
         MUI_DisposeObject(dock->LI_Buttons);
      FreeVec(dock);
   }
}
struct Hook DockList_DestructHook  = { { 0,0 }, (VOID *)DockList_DestructFunc   , NULL, NULL };

ULONG DockPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static STRPTR CY_ButtonTypes[4];
   struct DockPrefs_Data tmp;
   int max;

   CY_ButtonTypes[0] = GetStr(MSG_CY_IconText);
   CY_ButtonTypes[1] = GetStr(MSG_CY_Icon);
   CY_ButtonTypes[2] = GetStr(MSG_CY_Text);
   CY_ButtonTypes[3] = NULL;

   max = GetTagData(MUIA_DockPrefs_MinLineHeight, 0, msg->ops_AttrList) + 4;

   if(obj = (Object *)DoSuperNew(cl, obj,
      Child, VGroup,
         Child, HGroup,
            Child, VGroup,
               MUIA_FrameTitle   , GetStr(MSG_LV_DockTitle),
               MUIA_Group_Spacing, 0,
               Child, tmp.LV_ActiveIcons = ListviewObject,
                  MUIA_CycleChain            , 1,
                  MUIA_Listview_MultiSelect  , MUIV_Listview_MultiSelect_Default,
                  MUIA_Listview_DragType     , 1,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , tmp.LI_ActiveIcons = NewObject(CL_IconList->mcc_Class, NULL,
                     MUIA_List_MinLineHeight , max,
                  TAG_DONE),
               End,
               Child, HGroup,
                  MUIA_Group_Spacing, 0,
                  Child, tmp.BT_Edit = MakeButton(MSG_BT_Edit2),
                  Child, tmp.BT_Delete = MakeButton(MSG_BT_Delete1),
               End,
            End,
            Child, VGroup,
               MUIA_FrameTitle   , GetStr(MSG_LV_ButtonBankTitle),
               MUIA_Group_Spacing, 0,
               Child, tmp.LV_InactiveIcons = ListviewObject,
                  MUIA_CycleChain            , 1,
                  MUIA_Listview_MultiSelect  , MUIV_Listview_MultiSelect_Default,
                  MUIA_Listview_DragType     , 1,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , tmp.LI_InactiveIcons = NewObject(CL_IconList->mcc_Class, NULL,
                     MUIA_List_ConstructHook , &IconList_ConstructHook,
                     MUIA_List_DestructHook  , &IconList_DestructHook,
                     MUIA_List_MinLineHeight , max,
                  TAG_DONE),
               End,
               Child, HGroup,
                  MUIA_Group_Spacing, 0,
                  Child, tmp.BT_New    = MakeButton(MSG_BT_New2),
                  Child, tmp.BT_Copy   = MakeButton(MSG_BT_Copy),
               End,
            End,
         End,
         Child, HGroup,
            Child, ColGroup(2),
               Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
               Child, tmp.STR_Name        = MakeKeyString("", 81, MSG_CC_Name),
               Child, MakeKeyLabel2(MSG_LA_Hotkey, MSG_CC_Hotkey),
               Child, tmp.STR_Hotkey      = MakeKeyString("", 41, MSG_CC_Hotkey),
               Child, MakeKeyLabel2(MSG_LA_Font, MSG_CC_Font),
               Child, tmp.PA_Font = PopaslObject,
                  MUIA_Popstring_String, MakeKeyString("", 81, MSG_CC_Font),
                  MUIA_Popstring_Button, PopButton(MUII_PopUp),
                  MUIA_Popasl_Type     , ASL_FontRequest,
                  ASLFO_TitleText      , GetStr(MSG_TX_SelectFont),
               End,
               Child, MakeKeyLabel2(MSG_LA_Rows, MSG_CC_Rows),
               Child, tmp.SL_Rows = MakeKeySlider(1, 999, 1, MSG_CC_Rows),
            End,
            Child, VGroup,
               MUIA_Weight, 5,
               Child, tmp.CY_ButtonType   = MakeKeyCycle(CY_ButtonTypes, "   "),
               Child, ColGroup(5),
                  Child, tmp.CH_Activate     = MakeKeyCheckMark(FALSE, MSG_CC_Activate),
                  Child, MakeKeyLLabel2(MSG_LA_Activate, MSG_CC_Activate),
                  Child, HVSpace,
                  Child, tmp.CH_PopUp        = MakeKeyCheckMark(FALSE, MSG_CC_PopUp),
                  Child, MakeKeyLLabel2(MSG_LA_PopUp, MSG_CC_PopUp),
                  Child, tmp.CH_Backdrop     = MakeKeyCheckMark(FALSE, MSG_CC_Backdrop),
                  Child, MakeKeyLLabel2(MSG_LA_Backdrop, MSG_CC_Backdrop),
                  Child, HVSpace,
                  Child, tmp.CH_Frontmost    = MakeKeyCheckMark(FALSE, MSG_CC_Frontmost),
                  Child, MakeKeyLLabel2(MSG_LA_Frontmost, MSG_CC_Frontmost),
                  Child, tmp.CH_Borderless   = MakeKeyCheckMark(FALSE, MSG_CC_Borderless),
                  Child, MakeKeyLLabel2(MSG_LA_Borderless, MSG_CC_Borderless),
                  Child, HVSpace,
                  Child, tmp.CH_DragBar      = MakeKeyCheckMark(FALSE, MSG_CC_DragBar),
                  Child, MakeKeyLLabel2(MSG_LA_DragBar, MSG_CC_DragBar),
               End,
            End,
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      static const struct Hook IconList_AppMsgHook = { {NULL, NULL}, (VOID *)IconList_AppMsgFunc, NULL, NULL };
      struct DockPrefs_Data *data = INST_DATA(cl, obj);

      *data = tmp;

#ifdef DEMO
      set(tmp.BT_New    , MUIA_Disabled, TRUE);
#endif
      set(tmp.BT_Delete , MUIA_Disabled, TRUE);
      set(tmp.BT_Edit   , MUIA_Disabled, TRUE);
      set(tmp.BT_Copy   , MUIA_Disabled, TRUE);
      set(tmp.PA_Font   , MUIA_Disabled, TRUE);
      set(tmp.CH_Frontmost, MUIA_Disabled, TRUE);

      set(tmp.LI_InactiveIcons   , MUIA_UserData, tmp.LI_ActiveIcons);     /* show the lists from whom they have to accept drag requests */
      set(tmp.LI_ActiveIcons     , MUIA_UserData, tmp.LI_InactiveIcons);

      set(tmp.LV_InactiveIcons, MUIA_ShortHelp, GetStr(MSG_HELP_InactiveIcons));
      set(tmp.LV_ActiveIcons  , MUIA_ShortHelp, GetStr(MSG_HELP_ActiveIcons));
      set(tmp.BT_Edit         , MUIA_ShortHelp, GetStr(MSG_HELP_EditIcon));
      set(tmp.BT_Delete       , MUIA_ShortHelp, GetStr(MSG_HELP_RemoveIcon));
      set(tmp.BT_New          , MUIA_ShortHelp, GetStr(MSG_HELP_NewIcon));
      set(tmp.BT_Copy         , MUIA_ShortHelp, GetStr(MSG_HELP_CopyIcon));

      set(tmp.STR_Name        , MUIA_ShortHelp, GetStr(MSG_HELP_DockName));
      set(tmp.STR_Hotkey      , MUIA_ShortHelp, GetStr(MSG_HELP_DockHotkey));
      set(tmp.SL_Rows         , MUIA_ShortHelp, GetStr(MSG_HELP_Rows));
      set(tmp.PA_Font         , MUIA_ShortHelp, GetStr(MSG_HELP_Font));
      set(tmp.CY_ButtonType   , MUIA_ShortHelp, GetStr(MSG_HELP_ButtonType));
      set(tmp.CH_Activate     , MUIA_ShortHelp, GetStr(MSG_HELP_Activate));
      set(tmp.CH_PopUp        , MUIA_ShortHelp, GetStr(MSG_HELP_PopUp));
      set(tmp.CH_Backdrop     , MUIA_ShortHelp, GetStr(MSG_HELP_Backdrop));
      set(tmp.CH_Frontmost    , MUIA_ShortHelp, GetStr(MSG_HELP_Frontmost));
      set(tmp.CH_Borderless   , MUIA_ShortHelp, GetStr(MSG_HELP_Borderless));
      set(tmp.CH_DragBar      , MUIA_ShortHelp, GetStr(MSG_HELP_DragBar));

      DoMethod(tmp.LV_ActiveIcons   , MUIM_Notify, MUIA_List_Active        , MUIV_EveryTime  , tmp.LV_InactiveIcons  , 3, MUIM_NoNotifySet, MUIA_List_Active, MUIV_List_Active_Off);
      DoMethod(tmp.LV_InactiveIcons , MUIM_Notify, MUIA_List_Active        , MUIV_EveryTime  , tmp.LV_ActiveIcons    , 3, MUIM_NoNotifySet, MUIA_List_Active, MUIV_List_Active_Off);
      DoMethod(tmp.LV_ActiveIcons   , MUIM_Notify, MUIA_List_Active        , MUIV_EveryTime  , obj, 1, MUIM_DockPrefs_List_Active);
      DoMethod(tmp.LV_InactiveIcons , MUIM_Notify, MUIA_List_Active        , MUIV_EveryTime  , obj, 1, MUIM_DockPrefs_List_Active);
      DoMethod(tmp.LV_ActiveIcons   , MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime , obj, 1, MUIM_DockPrefs_EditIcon);
      DoMethod(tmp.LV_InactiveIcons , MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime , obj, 1, MUIM_DockPrefs_EditIcon);
      DoMethod(tmp.BT_Delete        , MUIM_Notify, MUIA_Pressed            , FALSE           , obj, 1, MUIM_DockPrefs_DeleteIcon);
      DoMethod(tmp.BT_Edit          , MUIM_Notify, MUIA_Pressed            , FALSE           , obj, 1, MUIM_DockPrefs_EditIcon);
#ifndef DEMO
      DoMethod(tmp.BT_New           , MUIM_Notify, MUIA_Pressed            , FALSE           , obj, 1, MUIM_DockPrefs_NewIcon);
      DoMethod(tmp.BT_Copy          , MUIM_Notify, MUIA_Pressed            , FALSE           , obj, 1, MUIM_DockPrefs_CopyIcon);
#endif
      DoMethod(tmp.STR_Name         , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj, 1, MUIM_DockPrefs_Name_Active);
      DoMethod(tmp.CY_ButtonType    , MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime  , obj, 1, MUIM_DockPrefs_Type_Active);

      DoMethod(tmp.LI_ActiveIcons   , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.LI_ActiveIcons    , 3, MUIM_CallHook, &IconList_AppMsgHook, MUIV_TriggerValue);
      DoMethod(tmp.LI_InactiveIcons , MUIM_Notify, MUIA_AppMessage         , MUIV_EveryTime  , tmp.LI_InactiveIcons  , 3, MUIM_CallHook, &IconList_AppMsgHook, MUIV_TriggerValue);
   }

   return((ULONG)obj);
}


#ifdef __SASC
SAVEDS ASM ULONG DockPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg) {
#else /* gcc */
ULONG DockPrefs_Dispatcher()
{
   register struct IClass *cl __asm("a0");
   register Object *obj __asm("a2");
   register Msg msg __asm("a1");
#endif

   switch (msg->MethodID)
   {
      case OM_NEW                         : return(DockPrefs_New              (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_GetDock         : return(DockPrefs_GetDock          (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_SetDock         : return(DockPrefs_SetDock          (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_NewIcon         : return(DockPrefs_NewIcon          (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_DeleteIcon      : return(DockPrefs_DeleteIcon       (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_CopyIcon        : return(DockPrefs_CopyIcon         (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_EditIcon        : return(DockPrefs_EditIcon         (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_EditIcon_Finish : return(DockPrefs_EditIcon_Finish  (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_List_Active     : return(DockPrefs_List_Active      (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_Name_Active     : return(DockPrefs_Name_Active      (cl, obj, (APTR)msg));
      case MUIM_DockPrefs_Type_Active     : return(DockPrefs_Type_Active      (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}


