#include "main.h"

/*
 * close the libraries
 */

VOID exit_libs(VOID)
{
   if(cat)              CloseCatalog(cat);
   if(SoundObject)      DisposeDTObject(SoundObject);

   if(DataTypesBase)    CloseLibrary(DataTypesBase);
   if(IFFParseBase)     CloseLibrary(IFFParseBase);
   if(UtilityBase)      CloseLibrary(UtilityBase);
   if(MUIMasterBase)    CloseLibrary(MUIMasterBase);
   if(IconBase)         CloseLibrary(IconBase);

   cat         = NULL;
   SoundObject = NULL;
   IFFParseBase   = UtilityBase     = MUIMasterBase  = DataTypesBase   =
   IconBase       = NULL;
}


/*
 * open all needed libraries
 */

BOOL init_libs(VOID)
{
   if(LocaleBase)
      cat = OpenCatalog(NULL, "netconnect.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

   UtilityBase    = OpenLibrary("utility.library"     , 0);
   IFFParseBase   = OpenLibrary("iffparse.library"    , 0);
   DataTypesBase  = OpenLibrary("datatypes.library"   , 0);
   IconBase       = OpenLibrary("icon.library"        , 0);
   MUIMasterBase  = OpenLibrary("muimaster.library"   , 11);

   if(MUIMasterBase && UtilityBase && IFFParseBase && DataTypesBase && IconBase)
      return(TRUE);

   exit_libs();
   return(FALSE);
}

VOID exit_classes(VOID)
{
   if(CL_MainWindow)    MUI_DeleteCustomClass(CL_MainWindow);
   if(CL_PagerList)     MUI_DeleteCustomClass(CL_PagerList);
   if(CL_ProgramList)   MUI_DeleteCustomClass(CL_ProgramList);
   if(CL_MenuPrefs)     MUI_DeleteCustomClass(CL_MenuPrefs);
   if(CL_IconList)      MUI_DeleteCustomClass(CL_IconList);
   if(CL_DockPrefs)     MUI_DeleteCustomClass(CL_DockPrefs);
   if(CL_EditIcon)      MUI_DeleteCustomClass(CL_EditIcon);
   if(CL_Editor)        MUI_DeleteCustomClass(CL_Editor);
   if(CL_About)         MUI_DeleteCustomClass(CL_About);

   CL_DockPrefs      = CL_IconList  =
   CL_About          = CL_EditIcon  =
   CL_Editor         = CL_MenuPrefs =
   CL_ProgramList    = CL_PagerList =
   CL_MainWindow     = NULL;
}


/*
 * initialize our custom classes
 */

BOOL init_classes(VOID)
{
   CL_About       = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct About_Data)         , About_Dispatcher);
   CL_Editor      = MUI_CreateCustomClass(NULL, MUIC_List   , NULL, sizeof(struct Editor_Data)        , Editor_Dispatcher);
   CL_EditIcon    = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct EditIcon_Data)      , EditIcon_Dispatcher);
   CL_IconList    = MUI_CreateCustomClass(NULL, MUIC_List   , NULL, sizeof(struct IconList_Data)      , IconList_Dispatcher);
   CL_DockPrefs   = MUI_CreateCustomClass(NULL, MUIC_Group  , NULL, sizeof(struct DockPrefs_Data)     , DockPrefs_Dispatcher);
   CL_MenuPrefs   = MUI_CreateCustomClass(NULL, MUIC_Group  , NULL, sizeof(struct MenuPrefs_Data)     , MenuPrefs_Dispatcher);
   CL_ProgramList = MUI_CreateCustomClass(NULL, MUIC_List   , NULL, sizeof(struct ProgramList_Data)   , ProgramList_Dispatcher);
   CL_PagerList   = MUI_CreateCustomClass(NULL, MUIC_List   , NULL, sizeof(struct PagerList_Data)     , PagerList_Dispatcher);
   CL_MainWindow  = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct MainWindow_Data)    , MainWindow_Dispatcher);

   if(CL_About && CL_DockPrefs && CL_IconList && CL_EditIcon && CL_Editor && CL_MenuPrefs &&
      CL_ProgramList && CL_PagerList && CL_MainWindow)
      return(TRUE);

   exit_classes();
   return(FALSE);
}

/*
 * extract the correct string from a locale made by "cat2h"
 */

STRPTR GetStr(STRPTR idstr)
{
   STRPTR local;

   local = idstr + 2;

   if(LocaleBase)
      return((STRPTR)GetCatalogStr(cat, *(UWORD *)idstr, local));

   return(local);
}

VOID LocalizeNewMenu(struct NewMenu *nm)
{
   for (;nm->nm_Type!=NM_END;nm++)
      if (nm->nm_Label != NM_BARLABEL)
         nm->nm_Label = GetStr(nm->nm_Label);
}

VOID Handler(VOID)
{
   ULONG sigs = NULL;

   if(init_libs())
   {
      if(init_classes())
      {
         LocalizeNewMenu(MainWindowMenu);

         if(app = ApplicationObject,
//            MUIA_Application_SingleTask   , TRUE,
            MUIA_Application_Author       , "Michael Neuweiler",
            MUIA_Application_Base         , "NetConnectPrefs",
            MUIA_Application_Title        , "NetConnect Preferences",
#ifdef DEMO
            MUIA_Application_Version      , "$VER:NetConnectPrefs " VERTAG " (DEMO)",
#else
            MUIA_Application_Version      , "$VER:NetConnectPrefs " VERTAG,
#endif
            MUIA_Application_Copyright    , GetStr(MSG_AppCopyright),
            MUIA_Application_Description  , GetStr(MSG_PrefsAppDescription),
            MUIA_Application_Window       , WindowObject,
               WindowContents    , group = VGroup,
                  Child, HVSpace,
               End,
            End,
         End)
         {
            if(win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE))
            {
               DoMethod(app, OM_ADDMEMBER, win);
               if(DoMethod(win, MUIM_MainWindow_InitGroups))
               {
                  set(win, MUIA_Window_Open, TRUE);
#ifdef DEMO
                  DoMethod(win, MUIM_MainWindow_About);
#endif
                  DoMethod(app, MUIM_Notify, MUIA_Application_DoubleStart, MUIV_EveryTime, win, 1, MUIM_MainWindow_DoubleStart);
                  DoMethod(win, MUIM_MainWindow_LoadPrefs, DEFAULT_CONFIGFILE);

                  while(DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
                  {
                     if(sigs)
                     {
                        sigs = Wait(sigs | SIGBREAKF_CTRL_C);
                        if(sigs & SIGBREAKF_CTRL_C)
                           break;
                     }
                  }
                  {
                     struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
                     struct DockPrefs_Data *dock_data = INST_DATA(CL_DockPrefs->mcc_Class, data->GR_Dock);

                     /** this can only be done while the window is still open !!! **/
                     DoMethod(dock_data->LI_ActiveIcons, MUIM_IconList_DeleteAllImages);
                     DoMethod(dock_data->LI_InactiveIcons, MUIM_IconList_DeleteAllImages);
                  }
                  set(win, MUIA_Window_Open, FALSE);
               }
            }
            MUI_DisposeObject(app);
            app = NULL;
         }
         exit_classes();
      }
      exit_libs();
   }
}

/// main
int main(int argc, char *argv[])
{
   proc = (struct Process *)FindTask(NULL);

   if(((ULONG)proc->pr_Task.tc_SPUpper - (ULONG)proc->pr_Task.tc_SPLower) < NEWSTACK_SIZE)
   {
      if(!(StackSwapper.stk_Lower = AllocVec(NEWSTACK_SIZE, MEMF_ANY)))
         exit(20);
      StackSwapper.stk_Upper   = (ULONG)StackSwapper.stk_Lower + NEWSTACK_SIZE;
      StackSwapper.stk_Pointer = (APTR)StackSwapper.stk_Upper;
      StackSwap(&StackSwapper);

      Handler();

      StackSwap(&StackSwapper);
      FreeVec(StackSwapper.stk_Lower);
   }
   else
      Handler();
}

///

