#include "mui_IconList.h"

extern Object *win;
extern struct MUI_CustomClass *CL_MainWindow, *CL_DockPrefs;

///

ULONG IconList_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
   if(msg->obj == obj)                             /* If somebody tried to drag ourselves onto ourselves, we let our superclass (the list class) handle the necessary actions.*/
      return(DoSuperMethodA(cl, obj, msg));
   else
   {
       if(msg->obj == (Object *)muiUserData(obj))  /* If our predefined source object (the other list) wants us to become active, we politely accept it. */
         return(MUIV_DragQuery_Accept);
      else
         return(MUIV_DragQuery_Refuse);            /* Everything else is beeing rejected */
   }
}


/*
 * An object was dropped over the list.
 * Do the necessary actions (move from
 * one list to the other).
 */

ULONG IconList_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
   struct MainWindow_Data *main_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct DockPrefs_Data *dock_data = INST_DATA(CL_DockPrefs->mcc_Class, main_data->GR_Dock);
   struct Icon *icon, *icon_ptr;

   if(msg->obj == obj)                             /* the source is the same as the destination => */
   {                                               /*  let MUI handle the moving of objects within the list */
      ULONG ret;

      ret = DoSuperMethodA(cl, obj, msg);

      /** if it was the list that contains only pointers then also rearrange the list that contains the original data **/
      if((obj == dock_data->LI_ActiveIcons) && dock_data->dock)
      {
         int i = 0;
         FOREVER
         {
            DoMethod(obj, MUIM_List_GetEntry, i, &icon);
            if(!icon)
               break;
            DoMethod(dock_data->dock->LI_Buttons, MUIM_List_GetEntry, i, &icon_ptr);
            while((icon != icon_ptr) && icon_ptr)
            {
               DoMethod(dock_data->dock->LI_Buttons, MUIM_List_Move, i, MUIV_List_Move_Bottom);
               DoMethod(dock_data->dock->LI_Buttons, MUIM_List_GetEntry, i, &icon_ptr);
            }
            i++;
         }
      }

      return(ret);
   }
   else
   {
      LONG dropmark, pos, state;

      set(obj     , MUIA_List_Quiet, TRUE);
      set(msg->obj, MUIA_List_Quiet, TRUE);

      /* copy the entries to the dest. by iterating through the source's selected entries */
      pos = MUIV_List_NextSelected_Start;          /* copy all selected entries and the active entry before we delete anything */
      dropmark = xget(obj, MUIA_List_DropMark);    /* find out where to include the entries in the destination */
      FOREVER
      {
         DoMethod(msg->obj, MUIM_List_NextSelected, &pos);
         if(pos == MUIV_List_NextSelected_End)
            break;

         DoMethod(msg->obj, MUIM_List_GetEntry, pos, &icon);
         if(icon->list)
            DoMethod(msg->obj, MUIM_List_DeleteImage, icon->list);
         if(icon->bodychunk)
            icon->list = (APTR)DoMethod(obj, MUIM_List_CreateImage, icon->bodychunk, 0);


         if(obj == dock_data->LI_InactiveIcons)
            DoMethod(obj, MUIM_List_InsertSingle, icon, dropmark++);
         else
         {
            DoMethod(dock_data->dock->LI_Buttons, MUIM_List_InsertSingle, icon, dropmark);
            DoMethod(dock_data->dock->LI_Buttons, MUIM_List_GetEntry, dropmark, &icon_ptr);
            DoMethod(obj, MUIM_List_InsertSingle, icon_ptr, dropmark++);
         }

         icon->Name                 = NULL;  // these entries are copied an reused
         icon->Hotkey               = NULL;  // if set to zero they won't be freed.
         icon->ImageFile            = NULL;
         icon->Sound                = NULL;
         icon->Program.File         = NULL;
         icon->Program.CurrentDir   = NULL;
         icon->Program.OutputFile   = NULL;
         icon->Program.PublicScreen = NULL;
         icon->bodychunk            = NULL;
         icon->body                 = NULL;
         icon->cols                 = NULL;
         icon->disk_object          = NULL;
      }

      /* delete entries in the source */
      pos = xget(msg->obj, MUIA_List_Active);      /* we have to delete the "active" entry first (because its state is not "selected") */
      if(pos != MUIV_List_Active_Off)
      {
         DoMethod(msg->obj, MUIM_List_Remove, pos);
         if(msg->obj == dock_data->LI_ActiveIcons)
            DoMethod(dock_data->dock->LI_Buttons, MUIM_List_Remove, pos);
      }
      pos = 0;
      FOREVER
      {
         DoMethod(msg->obj, MUIM_List_GetEntry, pos, &icon_ptr);
         if(!icon_ptr)
            break;

         DoMethod(msg->obj, MUIM_List_Select, pos, MUIV_List_Select_Ask, &state);
         if(state)
         {
            DoMethod(msg->obj, MUIM_List_Remove, pos);
            if(msg->obj == dock_data->LI_ActiveIcons)
               DoMethod(dock_data->dock->LI_Buttons, MUIM_List_Remove, pos);
            pos = 0;                               /* start over again because we lost an entry */
         }
         else
            pos++;                                 /* otherwise go to the next entry */
      }

      /* The above 2-way method is necessary because we can't delete an object in the list
      ** while we scan it for selected items. So deletion is done in the second part but
      ** we have to find out which entries are selected in a different way to get the
      ** correct entries deleted => MUIM_List_Select is used but this we we haven't got
      ** the active entry (the last one we clicked on) included so we have to delete the
      ** MUIV_List_GetEntry_Active entry first and then scan the list with
      ** MUIV_List_Select_Ask. A bit complicated but it works and if I were you I wouldn't
      ** change it :)
      */


      /*
      ** make the insterted object the active and make the source listviews
      ** active object inactive to give some more visual feedback to the user.
      */
      set(obj, MUIA_List_Active        , xget(obj, MUIA_List_InsertPosition));
      set(msg->obj, MUIA_List_Active   , MUIV_List_Active_Off);

      /* and now make the changes visible */
      set(obj     , MUIA_List_Quiet, FALSE);
      set(msg->obj, MUIA_List_Quiet, FALSE);

      return(NULL);
   }
}

#ifdef __SASC
SAVEDS ASM struct Icon *IconList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Icon *src) {
#else /* gcc */
struct Icon *IconList_ConstructFunc()
{
   register APTR pool __asm("a2");
   register struct Icon *src __asm("a1");
#endif

   struct Icon *new;

   if((new = (struct Icon *)AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct Icon));
   return(new);
}
struct Hook IconList_ConstructHook = { { 0,0 }, (VOID *)IconList_ConstructFunc  , NULL, NULL };

#ifdef __SASC
SAVEDS ASM VOID IconList_DestructFunc(REG(a2) APTR pool, REG(a1) struct Icon *icon) {
#else /* gcc */
VOID IconList_DestructFunc()
{
   register APTR pool __asm("a2");
   register struct Icon *icon __asm("a1");
#endif

   if(icon)
   {
      if(icon->Name)
         FreeVec(icon->Name);
      if(icon->Hotkey)
         FreeVec(icon->Hotkey);
      if(icon->ImageFile)
         FreeVec(icon->ImageFile);
      if(icon->Sound)
         FreeVec(icon->Sound);
      if(icon->Program.File)
         FreeVec(icon->Program.File);
      if(icon->Program.CurrentDir)
         FreeVec(icon->Program.CurrentDir);
      if(icon->Program.OutputFile)
         FreeVec(icon->Program.OutputFile);
      if(icon->Program.PublicScreen)
         FreeVec(icon->Program.PublicScreen);

      if(icon->bodychunk)
         MUI_DisposeObject(icon->bodychunk);
      if(icon->body)
         FreeVec(icon->body);
      if(icon->cols)
         FreeVec(icon->cols);
      if(icon->disk_object)
         FreeDiskObject(icon->disk_object);
      FreeVec(icon);
   }
}
struct Hook IconList_DestructHook  = { { 0,0 }, (VOID *)IconList_DestructFunc   , NULL, NULL };

#ifdef __SASC
SAVEDS ASM LONG IconList_DisplayFunc(REG(a0) struct Hook *hook, REG(a2) char **array, REG(a1) struct Icon *icon)
{
#else /* gcc */
LONG IconList_DisplayFunc()
{
   register struct Hook *hook __asm("a0");
   register char **array __asm("a2");
   register struct Icon *icon __asm("a1");
#endif

   if(icon)
   {
      static char buf[31];

      if(icon->list)
         sprintf(buf,"\33O[%08lx]", icon->list);   /* show the image */
      else
         strcpy(buf, "\33c[Icon] ");
      *array++ = buf;
      *array   = (icon->Name ? icon->Name : (STRPTR)"");                      /* show the name */
   }
   return(NULL);
}
struct Hook IconList_DisplayHook   = { { 0,0 }, (VOID *)IconList_DisplayFunc    , NULL, NULL };

ULONG IconList_DeleteAllImages(struct IClass *cl, Object *obj, Msg msg)
{
   int i = 0;
   struct Icon *icon;

   FOREVER
   {
      DoMethod(obj, MUIM_List_GetEntry, i++, &icon);
      if(!icon)
         break;
      if(icon->list)
         DoMethod(obj, MUIM_List_DeleteImage, icon->list);
      icon->list = NULL;
   }
   return(NULL);
}

ULONG IconList_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   obj = (Object *)DoSuperNew(cl, obj,
      InputListFrame,
      MUIA_List_DisplayHook   , &IconList_DisplayHook,
      /** the con/destruct hooks are only needed by one list (see dockprefs_new) **/
      MUIA_List_Format        , ",",
      MUIA_List_AutoVisible   , TRUE,
      MUIA_List_DragSortable  , TRUE,
      TAG_MORE, msg->ops_AttrList);

   return((ULONG)obj);
}

#ifdef __SASC
SAVEDS ASM ULONG IconList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg) {
#else /* gcc */
ULONG IconList_Dispatcher()
{
   register struct IClass *cl __asm("a0");
   register Object *obj __asm("a2");
   register Msg msg __asm("a1");
#endif

   switch(msg->MethodID)
   {
      case OM_NEW                         : return(IconList_New            (cl, obj, (APTR)msg));
      case MUIM_DragQuery                 : return(IconList_DragQuery      (cl, obj, (APTR)msg));
      case MUIM_DragDrop                  : return(IconList_DragDrop       (cl, obj, (APTR)msg));
      case MUIM_IconList_DeleteAllImages  : return(IconList_DeleteAllImages(cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

