/// includes
#include "/includes.h"

#include "/NetConnect.h"
#include "/locale/Strings.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_DockPrefs.h"
#include "mui_MenuPrefs.h"
#include "mui_IconList.h"
#include "protos.h"
#include "rev.h"
#include "/images/logo.h"

///
/// external veriables
extern struct MUI_CustomClass *CL_MenuPrefs, *CL_DockPrefs, *CL_MainWindow,
                              *CL_About, *CL_PagerList;
extern BOOL is_test;
extern Object *win, *app, *group;
extern STRPTR default_names[], default_imagefiles[], default_programfiles[],
              default_menus[], default_menu_programs[];
extern struct Hook DockList_ConstructHook, DockList_DestructHook;
extern struct NewMenu MainWindowMenu[];
extern ULONG logo_colors[];
extern UBYTE logo_body[];

///

VOID load_program(struct IFFHandle *Handle, struct ContextNode *Chunk, struct Program *program)
{
   bzero(program, sizeof(struct Program));

   if(program->File = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
      ReadChunkBytes(Handle, program->File, Chunk->cn_Size);

   while(!ParseIFF(Handle, IFFPARSE_SCAN))
   {
      Chunk = CurrentChunk(Handle);

      if(Chunk->cn_ID == ID_END)
         break;
      if(Chunk->cn_ID == ID_PRGF)
         ReadChunkBytes(Handle, &program->Flags, MIN(sizeof(UWORD), Chunk->cn_Size));
      if(Chunk->cn_ID == ID_PRGS)
         ReadChunkBytes(Handle, &program->Stack, MIN(sizeof(LONG), Chunk->cn_Size));
      if(Chunk->cn_ID == ID_PRGP)
         ReadChunkBytes(Handle, &program->Priority, MIN(sizeof(BYTE), Chunk->cn_Size));
      if(Chunk->cn_ID == ID_PRGC)
      {
         if(program->CurrentDir = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
            ReadChunkBytes(Handle, program->CurrentDir, Chunk->cn_Size);
      }
      if(Chunk->cn_ID == ID_PRGO)
      {
         if(program->OutputFile = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
            ReadChunkBytes(Handle, program->OutputFile, Chunk->cn_Size);
      }
      if(Chunk->cn_ID == ID_PRGR)
      {
         if(program->PublicScreen = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
            ReadChunkBytes(Handle, program->PublicScreen, Chunk->cn_Size);
      }
   }
}


VOID load_icon(struct IFFHandle *Handle, struct ContextNode *Chunk, struct Icon *icon)
{
   bzero(icon, sizeof(struct Icon));

   if(icon->Name = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
      ReadChunkBytes(Handle, icon->Name, Chunk->cn_Size);

   while(!ParseIFF(Handle, IFFPARSE_SCAN))
   {
      Chunk = CurrentChunk(Handle);

      if(Chunk->cn_ID == ID_END)
         break;
      if(Chunk->cn_ID == ID_ICOH)
      {
         if(icon->Hotkey = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
            ReadChunkBytes(Handle, icon->Hotkey, Chunk->cn_Size);
      }
      if(Chunk->cn_ID == ID_ICOI)
      {
         if(icon->ImageFile = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
            ReadChunkBytes(Handle, icon->ImageFile, Chunk->cn_Size);
      }
      if(Chunk->cn_ID == ID_ICOS)
      {
         if(icon->Sound = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
            ReadChunkBytes(Handle, icon->Sound, Chunk->cn_Size);
      }
      if(Chunk->cn_ID == ID_ICOV)
         ReadChunkBytes(Handle, &icon->Volume, MIN(sizeof(UWORD), Chunk->cn_Size));
      if(Chunk->cn_ID == ID_ICOF)
         ReadChunkBytes(Handle, &icon->Flags, MIN(sizeof(UWORD), Chunk->cn_Size));
      if(Chunk->cn_ID == ID_PRGM)
         load_program(Handle, Chunk, &icon->Program);
   }
}

ULONG MainWindow_LoadPrefs(struct IClass *cl, Object *obj, struct MUIP_MainWindow_LoadPrefs *msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct MenuPrefs_Data *menu_data = INST_DATA(CL_MenuPrefs->mcc_Class, data->GR_Menus);
   struct DockPrefs_Data *dock_data = INST_DATA(CL_DockPrefs->mcc_Class, data->GR_Dock);
   struct IFFHandle  *Handle;
   struct ContextNode   *Chunk;
   UWORD menu_id = 0;
   BOOL anything = FALSE;

   set(obj, MUIA_Window_Sleep, TRUE);

   if(Handle = AllocIFF())
   {
      if(Handle->iff_Stream = Open(msg->file, MODE_OLDFILE))
      {
         InitIFFasDOS(Handle);
         if(!(OpenIFF(Handle, IFFF_READ)))
         {
            if(!(StopChunks(Handle, Stops, NUM_STOPS)))
            {
               set(data->LV_Pager, MUIA_List_Active, MUIV_List_Active_Top);
               set(dock_data->LI_ActiveIcons    , MUIA_List_Quiet, TRUE);
               set(dock_data->LI_InactiveIcons  , MUIA_List_Quiet, TRUE);
               set(data->LV_Pager               , MUIA_List_Quiet, TRUE);

               DoMethod(dock_data->LI_ActiveIcons, MUIM_IconList_DeleteAllImages);
               DoMethod(dock_data->LI_InactiveIcons, MUIM_IconList_DeleteAllImages);
               DoMethod(dock_data->LI_ActiveIcons  , MUIM_List_Clear);
               DoMethod(dock_data->LI_InactiveIcons, MUIM_List_Clear);
               DoMethod(dock_data->LI_Docks        , MUIM_List_Clear);
               DoMethod(menu_data->LI_Menus        , MUIM_List_Clear);
               while(xget(data->LV_Pager, MUIA_List_Entries) > 2)
                  DoMethod(data->LV_Pager, MUIM_List_Remove, MUIV_List_Remove_Last);

               while(!ParseIFF(Handle, IFFPARSE_SCAN))
               {
                  Chunk = CurrentChunk(Handle);

                  if(Chunk->cn_ID == ID_MENU)
                  {
                     struct MenuEntry menu, *menu_ptr;
                     struct Program program;

                     if(menu.Name = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
                        ReadChunkBytes(Handle, menu.Name, Chunk->cn_Size);

                     menu.Id = menu_id++;
                     DoMethod(menu_data->LI_Menus, MUIM_List_InsertSingle, &menu, MUIV_List_Insert_Bottom);

                     DoMethod(menu_data->LI_Menus, MUIM_List_GetEntry, xget(menu_data->LI_Menus, MUIA_List_Entries) - 1, &menu_ptr);
                     while(!ParseIFF(Handle, IFFPARSE_SCAN))
                     {
                        Chunk = CurrentChunk(Handle);

                        if(Chunk->cn_ID == ID_END)
                           break;
                        if(Chunk->cn_ID == ID_PRGM)
                        {
                           load_program(Handle, Chunk, &program);
                           if(program.File && menu_ptr)
                              DoMethod(menu_ptr->LI_Programs, MUIM_List_InsertSingle, &program, MUIV_List_Insert_Bottom);
                        }
                     }
                     anything = TRUE;
                  }

                  if(Chunk->cn_ID == ID_DOCK)
                  {
                     struct Dock dock, *dock_ptr;
                     struct Icon icon;
                     bzero(&dock, sizeof(struct Dock));

                     if(dock.Name = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
                        ReadChunkBytes(Handle, dock.Name, Chunk->cn_Size);

                     while(!ParseIFF(Handle, IFFPARSE_SCAN))
                     {
                        Chunk = CurrentChunk(Handle);

                        if(Chunk->cn_ID == ID_END || Chunk->cn_ID == ID_ICON)
                           break;
                        if(Chunk->cn_ID == ID_DOCH)
                        {
                           if(dock.Hotkey = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
                              ReadChunkBytes(Handle, dock.Hotkey, Chunk->cn_Size);
                        }
                        if(Chunk->cn_ID == ID_DOCR)
                           ReadChunkBytes(Handle, &dock.Rows, MIN(sizeof(UWORD), Chunk->cn_Size));
                        if(Chunk->cn_ID == ID_DOCI)
                           ReadChunkBytes(Handle, &dock.WindowID, MIN(sizeof(LONG), Chunk->cn_Size));
                        if(Chunk->cn_ID == ID_DOCF)
                           ReadChunkBytes(Handle, &dock.Flags, MIN(sizeof(UWORD), Chunk->cn_Size));
                        if(Chunk->cn_ID == ID_DOCO)
                        {
                           if(dock.Font = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
                              ReadChunkBytes(Handle, dock.Font, Chunk->cn_Size);
                        }
                     }
                     DoMethod(dock_data->LI_Docks, MUIM_List_InsertSingle, &dock, MUIV_List_Insert_Bottom);
                     DoMethod(dock_data->LI_Docks, MUIM_List_GetEntry, xget(dock_data->LI_Docks, MUIA_List_Entries) - 1, &dock_ptr);
                     DoMethod(data->LV_Pager, MUIM_List_InsertSingle, dock_ptr, MUIV_List_Insert_Bottom);
                     while(Chunk->cn_ID == ID_ICON)
                     {
                        load_icon(Handle, Chunk, &icon);
                        if(dock_ptr)
                        {
                           init_icon(&icon);
                           DoMethod(dock_ptr->LI_Buttons, MUIM_List_InsertSingle, &icon, MUIV_List_Insert_Bottom);
                        }
                        if(ParseIFF(Handle, IFFPARSE_SCAN))
                           break;
                        Chunk = CurrentChunk(Handle);
                     }
                     anything = TRUE;
                  }

                  if(Chunk->cn_ID == ID_ICON)
                  {
                     struct Icon icon;

                     load_icon(Handle, Chunk, &icon);
                     init_icon(&icon);
                     DoMethod(dock_data->LI_InactiveIcons, MUIM_List_InsertSingle, &icon, MUIV_List_Insert_Bottom);

                     anything = TRUE;
                  }
               }
               set(dock_data->LI_ActiveIcons    , MUIA_List_Quiet, FALSE);
               set(dock_data->LI_InactiveIcons  , MUIA_List_Quiet, FALSE);
               set(data->LV_Pager               , MUIA_List_Quiet, FALSE);
            }
            CloseIFF(Handle);
         }
         Close(Handle->iff_Stream);
      }
      FreeIFF(Handle);
   }

   set(obj, MUIA_Window_Sleep, FALSE);

   if(!anything)
      DoMethod(obj, MUIM_MainWindow_Reset);

   return(NULL);
}


VOID save_program(struct IFFHandle *Handle, struct Program *program)
{
   if(!PushChunk(Handle, ID_NTCN, ID_PRGM, IFFSIZE_UNKNOWN))
   {
      if(program->File)
         WriteChunkBytes(Handle, program->File, strlen(program->File) + 1);
      PopChunk(Handle);

      if(!PushChunk(Handle, ID_NTCN, ID_PRGF, IFFSIZE_UNKNOWN))
      {
         WriteChunkBytes(Handle, &program->Flags, sizeof(UWORD));
         PopChunk(Handle);
      }
      if(!PushChunk(Handle, ID_NTCN, ID_PRGS, IFFSIZE_UNKNOWN))
      {
         WriteChunkBytes(Handle, &program->Stack, sizeof(LONG));
         PopChunk(Handle);
      }
      if(!PushChunk(Handle, ID_NTCN, ID_PRGP, IFFSIZE_UNKNOWN))
      {
         WriteChunkBytes(Handle, &program->Priority, sizeof(BYTE));
         PopChunk(Handle);
      }
      if(program->CurrentDir)
      {
         if(!PushChunk(Handle, ID_NTCN, ID_PRGC, IFFSIZE_UNKNOWN))
         {
            WriteChunkBytes(Handle, program->CurrentDir, strlen(program->CurrentDir) + 1);
            PopChunk(Handle);
         }
      }
      if(program->OutputFile)
      {
         if(!PushChunk(Handle, ID_NTCN, ID_PRGO, IFFSIZE_UNKNOWN))
         {
            WriteChunkBytes(Handle, program->OutputFile, strlen(program->OutputFile) + 1);
            PopChunk(Handle);
         }
      }
      if(program->PublicScreen)
      {
         if(!PushChunk(Handle, ID_NTCN, ID_PRGR, IFFSIZE_UNKNOWN))
         {
            WriteChunkBytes(Handle, program->PublicScreen, strlen(program->PublicScreen) + 1);
            PopChunk(Handle);
         }
      }

      if(!PushChunk(Handle, ID_NTCN, ID_END, IFFSIZE_UNKNOWN))
         PopChunk(Handle);
   }
}

VOID save_icon(struct IFFHandle *Handle, struct Icon *icon)
{
   if(!PushChunk(Handle, ID_NTCN, ID_ICON, IFFSIZE_UNKNOWN))
   {
      if(icon->Name)
         WriteChunkBytes(Handle, icon->Name, strlen(icon->Name) + 1);
      PopChunk(Handle);

      if(icon->Hotkey)
      {
         if(!PushChunk(Handle, ID_NTCN, ID_ICOH, IFFSIZE_UNKNOWN))
         {
            WriteChunkBytes(Handle, icon->Hotkey, strlen(icon->Hotkey) + 1);
            PopChunk(Handle);
         }
      }
      if(icon->ImageFile)
      {
         if(!PushChunk(Handle, ID_NTCN, ID_ICOI, IFFSIZE_UNKNOWN))
         {
            WriteChunkBytes(Handle, icon->ImageFile, strlen(icon->ImageFile) + 1);
            PopChunk(Handle);
         }
      }
      if(icon->Sound)
      {
         if(!PushChunk(Handle, ID_NTCN, ID_ICOS, IFFSIZE_UNKNOWN))
         {
            WriteChunkBytes(Handle, icon->Sound, strlen(icon->Sound) + 1);
            PopChunk(Handle);
         }
      }
      if(!PushChunk(Handle, ID_NTCN, ID_ICOV, IFFSIZE_UNKNOWN))
      {
         WriteChunkBytes(Handle, &icon->Volume, sizeof(UBYTE));
         PopChunk(Handle);
      }
      if(!PushChunk(Handle, ID_NTCN, ID_ICOF, IFFSIZE_UNKNOWN))
      {
         WriteChunkBytes(Handle, &icon->Flags, sizeof(UWORD));
         PopChunk(Handle);
      }

      save_program(Handle, &icon->Program);

      if(!PushChunk(Handle, ID_NTCN, ID_END, IFFSIZE_UNKNOWN))
         PopChunk(Handle);
   }
}

VOID save_dock(struct IFFHandle *Handle, struct Dock *dock)
{
   int i;
   struct Icon *icon;

   if(!PushChunk(Handle, ID_NTCN, ID_DOCK, IFFSIZE_UNKNOWN))
   {
      if(dock->Name)
         WriteChunkBytes(Handle, dock->Name, strlen(dock->Name) + 1);
      PopChunk(Handle);

      if(dock->Hotkey)
      {
         if(!PushChunk(Handle, ID_NTCN, ID_DOCH, IFFSIZE_UNKNOWN))
         {
            WriteChunkBytes(Handle, dock->Hotkey, strlen(dock->Hotkey) + 1);
            PopChunk(Handle);
         }
      }
      if(dock->Font)
      {
         if(!PushChunk(Handle, ID_NTCN, ID_DOCO, IFFSIZE_UNKNOWN))
         {
            WriteChunkBytes(Handle, dock->Font, strlen(dock->Font) + 1);
            PopChunk(Handle);
         }
      }
      if(!PushChunk(Handle, ID_NTCN, ID_DOCR, IFFSIZE_UNKNOWN))
      {
         WriteChunkBytes(Handle, &dock->Rows, sizeof(UWORD));
         PopChunk(Handle);
      }
      if(!PushChunk(Handle, ID_NTCN, ID_DOCI, IFFSIZE_UNKNOWN))
      {
         WriteChunkBytes(Handle, &dock->WindowID, sizeof(LONG));
         PopChunk(Handle);
      }
      if(!PushChunk(Handle, ID_NTCN, ID_DOCF, IFFSIZE_UNKNOWN))
      {
         WriteChunkBytes(Handle, &dock->Flags, sizeof(UWORD));
         PopChunk(Handle);
      }

      i = 0;
      FOREVER
      {
         DoMethod(dock->LI_Buttons, MUIM_List_GetEntry, i++, &icon);
         if(!icon)
            break;
         save_icon(Handle, icon);
      }

      if(!PushChunk(Handle, ID_NTCN, ID_END, IFFSIZE_UNKNOWN))
         PopChunk(Handle);
   }
}

VOID save_menu(struct IFFHandle *Handle, struct MenuEntry *menu)
{
   int i;
   struct Program *program;

   if(!PushChunk(Handle, ID_NTCN, ID_MENU, IFFSIZE_UNKNOWN))
   {
      if(menu->Name)
         WriteChunkBytes(Handle, menu->Name, strlen(menu->Name) + 1);
      PopChunk(Handle);

      i = 0;
      FOREVER
      {
         DoMethod(menu->LI_Programs, MUIM_List_GetEntry, i++, &program);
         if(!program)
            break;
         save_program(Handle, program);
      }

      if(!PushChunk(Handle, ID_NTCN, ID_END, IFFSIZE_UNKNOWN))
         PopChunk(Handle);
   }
}

VOID save_prefs(STRPTR file)
{
   struct MainWindow_Data *data     = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct MenuPrefs_Data *menu_data = INST_DATA(CL_MenuPrefs->mcc_Class, data->GR_Menus);
   struct DockPrefs_Data *dock_data = INST_DATA(CL_DockPrefs->mcc_Class, data->GR_Dock);
   struct IFFHandle  *Handle;
   struct Icon *icon;
   struct Dock *dock;
   struct MenuEntry *menu;
   int i, j;

   if(Handle = AllocIFF())
   {
      if(Handle->iff_Stream = Open(file, MODE_NEWFILE))
      {
         InitIFFasDOS(Handle);
         if(!(OpenIFF(Handle, IFFF_WRITE)))
         {
            if(!(PushChunk(Handle, ID_NTCN, ID_FORM, IFFSIZE_UNKNOWN)))
            {
               i = 0;
               FOREVER
               {
                  DoMethod(dock_data->LI_InactiveIcons, MUIM_List_GetEntry, i++, &icon);
                  if(!icon)
                     break;
                  save_icon(Handle, icon);
               }
               j = 0;
               FOREVER
               {
                  DoMethod(dock_data->LI_Docks, MUIM_List_GetEntry, j++, &dock);
                  if(!dock)
                     break;
                  save_dock(Handle, dock);
               }
               j = 0;
               FOREVER
               {
                  DoMethod(menu_data->LI_Menus, MUIM_List_GetEntry, j++, &menu);
                  if(!menu)
                     break;
                  save_menu(Handle, menu);
               }
               PopChunk(Handle);
            }
            CloseIFF(Handle);
         }
         Close(Handle->iff_Stream);
      }
      FreeIFF(Handle);
   }
}

ULONG MainWindow_Finish(struct IClass *cl, Object *obj, struct MUIP_MainWindow_Finish *msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   if(data->GR_Active == data->GR_Dock)
      DoMethod(data->GR_Dock, MUIM_DockPrefs_GetDock);

   if(msg->level)
   {
      save_prefs(DEFAULT_CONFIGFILE);
      if(msg->level == 2)
         CopyFile(DEFAULT_CONFIGFILE, "ENVARC:NC2.prefs");
   }
   else
   {
      if(is_test)
         CopyFile("ENV:NC2.prefs.tmp", DEFAULT_CONFIGFILE);
   }
   DeleteFile("ENV:NC2.prefs.tmp");

   DoMethod(app, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

   return(NULL);
}

ULONG MainWindow_Test(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   if(data->GR_Active == data->GR_Dock)
      DoMethod(data->GR_Dock, MUIM_DockPrefs_GetDock);

   if(!is_test)
   {
      CopyFile(DEFAULT_CONFIGFILE, "ENV:NC2.prefs.tmp");
      is_test = TRUE;
   }
   save_prefs(DEFAULT_CONFIGFILE);

   return(NULL);
}

ULONG MainWindow_Help(struct IClass *cl, Object *obj, Msg msg)
{
   SystemTags("C:Execute NetConnect2:Docs/Documentation", TAG_DONE);
   return(NULL);
}

ULONG MainWindow_Reset(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct DockPrefs_Data *dock_data = INST_DATA(CL_DockPrefs->mcc_Class, data->GR_Dock);
   struct MenuPrefs_Data *menu_data = INST_DATA(CL_MenuPrefs->mcc_Class, data->GR_Menus);
   struct Icon icon;
   struct Dock dock, *dock_ptr;
   struct MenuEntry menu, *menu_ptr;
   struct Program program;
   LONG pos;

   set(obj, MUIA_Window_Sleep, TRUE);
   set(data->LV_Pager, MUIA_List_Active, MUIV_List_Active_Top);

   set(dock_data->LI_ActiveIcons    , MUIA_List_Quiet, TRUE);
   set(dock_data->LI_InactiveIcons  , MUIA_List_Quiet, TRUE);
   set(data->LV_Pager               , MUIA_List_Quiet, TRUE);

   /** clear all lists: active_icons, inactive_icons, docks and pager **/
   DoMethod(dock_data->LI_ActiveIcons     , MUIM_IconList_DeleteAllImages);
   DoMethod(dock_data->LI_ActiveIcons     , MUIM_List_Clear);
   DoMethod(dock_data->LI_InactiveIcons   , MUIM_IconList_DeleteAllImages);
   DoMethod(dock_data->LI_InactiveIcons   , MUIM_List_Clear);
   DoMethod(dock_data->LI_Docks           , MUIM_List_Clear);
   while(xget(data->LV_Pager, MUIA_List_Entries) > 2)
      DoMethod(data->LV_Pager, MUIM_List_Remove, MUIV_List_Remove_Last);

   bzero(&dock, sizeof(struct Dock));
   if(dock.Name = AllocVec(strlen(GetStr(MSG_TX_InternetDock)), MEMF_ANY))
   {
      strcpy(dock.Name, GetStr(MSG_TX_InternetDock));
      dock.WindowID = MAKE_ID('D', 'O', 'A', 'A');
      dock.Flags = DFL_PopUp | DFL_DragBar | DFL_Text | DFL_Icon;
      DoMethod(dock_data->LI_Docks, MUIM_List_InsertSingle, &dock, MUIV_List_Insert_Bottom);

      DoMethod(dock_data->LI_Docks, MUIM_List_GetEntry, xget(dock_data->LI_Docks, MUIA_List_Entries) - 1, &dock_ptr);
      if(dock_ptr)
      {
         pos = 0;
         bzero(&icon, sizeof(struct Icon));
         while(default_names[pos])
         {
            if(icon.Name = AllocVec(strlen(default_names[pos]) + 1, MEMF_ANY))
               strcpy(icon.Name           , default_names[pos]);
            if(icon.ImageFile = AllocVec(strlen(default_imagefiles[pos]) + 1, MEMF_ANY))
               strcpy(icon.ImageFile      , default_imagefiles[pos]);
            if(icon.Program.File = AllocVec(strlen(default_programfiles[pos]) + 1, MEMF_ANY))
               strcpy(icon.Program.File   , default_programfiles[pos]);
            icon.Program.Flags   = (pos != 7 ? PRG_CLI : PRG_SCRIPT);
            icon.Program.Flags   |=  PRG_Asynch;
            icon.Volume          = 64;
            icon.Flags           = IFL_DrawFrame;

            init_icon(&icon);
            DoMethod((pos < DEFAULT_ACTIVE ? dock_ptr->LI_Buttons : dock_data->LI_InactiveIcons), MUIM_List_InsertSingle, &icon, MUIV_List_Insert_Bottom);

            pos++;
         }
      }
      DoMethod(data->LV_Pager, MUIM_List_InsertSingle, dock_ptr, MUIV_List_Insert_Bottom);
   }

   set(dock_data->LI_ActiveIcons    , MUIA_List_Quiet, FALSE);
   set(dock_data->LI_InactiveIcons  , MUIA_List_Quiet, FALSE);
   set(data->LV_Pager               , MUIA_List_Quiet, FALSE);

   pos = xget(dock_data->SL_Rows, MUIA_Numeric_Value);
   set(dock_data->SL_Rows, MUIA_Slider_Max, (xget(dock_ptr->LI_Buttons, MUIA_List_Entries) ? xget(dock_ptr->LI_Buttons, MUIA_List_Entries) : 1));
   setslider(dock_data->SL_Rows, pos);

   /** and the menus **/
   DoMethod(menu_data->LI_Menus, MUIM_List_Clear);
   bzero(&menu, sizeof(struct MenuEntry));
   bzero(&program, sizeof(struct Program));
   pos = 0;
   while(default_menus[pos])
   {
      if(menu.Name = AllocVec(strlen(default_menus[pos]) + 1, MEMF_ANY))
         strcpy(menu.Name, default_menus[pos]);
      DoMethod(menu_data->LI_Menus, MUIM_List_InsertSingle, &menu, MUIV_List_Insert_Bottom);

      if(program.File = AllocVec(strlen(default_menu_programs[pos]) + 1, MEMF_ANY))
         strcpy(program.File, default_menu_programs[pos]);
      program.Flags = PRG_Asynch;
      program.Stack = 8192;
      DoMethod(menu_data->LI_Menus, MUIM_List_GetEntry, xget(menu_data->LI_Menus, MUIA_List_Entries) - 1, &menu_ptr);
      if(menu_ptr)
         DoMethod(menu_ptr->LI_Programs, MUIM_List_InsertSingle, &program, MUIV_List_Insert_Bottom);
      pos++;
   }

   set(obj, MUIA_Window_Sleep, FALSE);

   return(NULL);
}

ULONG MainWindow_Load(struct IClass *cl, Object *obj, Msg msg)
{
   STRPTR file;

   if(file = getfilename(obj, "Select file to load", DEFAULT_CONFIGFILE, FALSE))
   {
      if(*file)
         DoMethod(obj, MUIM_MainWindow_LoadPrefs, file);
   }
   return(NULL);
}

ULONG MainWindow_SaveAs(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   STRPTR file;

   if(data->GR_Active == data->GR_Dock)
      DoMethod(data->GR_Dock, MUIM_DockPrefs_GetDock);

   if(file = getfilename(obj, "Select a filename for settings", DEFAULT_CONFIGFILE, TRUE))
   {
      if(*file)
         save_prefs(file);
   }

   return(NULL);
}

ULONG MainWindow_NewDock(struct IClass *cl, Object *obj, Msg msg)
{
#ifndef DEMO
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct DockPrefs_Data *dock_data = INST_DATA(CL_DockPrefs->mcc_Class, data->GR_Dock);
   struct Dock *dock;
   BOOL stop;
   int pos;
   char i, j;

   DoMethod(dock_data->LI_Docks, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);

   /** give the window a unique ID **/

   stop = FALSE;
   for(i = 'A'; i <= 'Z', !stop; i++)
   {
      for(j = 'A'; j <= 'Z', !stop; j++)
      {
         pos = 0;
         stop = TRUE;
         while(stop)
         {
            DoMethod(dock_data->LI_Docks, MUIM_List_GetEntry, pos++, &dock);
            if(!dock)
               break;
            if(dock->WindowID == MAKE_ID('D', 'O', i, j))
               stop = FALSE;
         }
      }
   }
   i--;
   j--;
   DoMethod(dock_data->LI_Docks, MUIM_List_GetEntry, xget(dock_data->LI_Docks, MUIA_List_Entries) - 1, &dock);
   if(dock)
   {
      dock->WindowID = MAKE_ID('D', 'O', i, j);
      dock->Name = update_string(dock->Name, GetStr(MSG_TX_Dock));
      dock->Flags |= DFL_PopUp | DFL_DragBar | DFL_Icon | DFL_Text;
      DoMethod(data->LV_Pager, MUIM_List_InsertSingle, dock, MUIV_List_Insert_Bottom);
   }

   set(data->LV_Pager, MUIA_List_Active, MUIV_List_Active_Bottom);

#endif

   return(NULL);
}

ULONG MainWindow_RemoveDock(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct DockPrefs_Data *dock_data = INST_DATA(CL_DockPrefs->mcc_Class, data->GR_Dock);
   int active = xget(data->LV_Pager, MUIA_List_Active);

   if(active > 1)
   {
      DoMethod(data->GR_Pager, MUIM_Group_InitChange);
      set(data->LV_Pager, MUIA_List_Active, MUIV_List_Active_Top);

      DoMethod(dock_data->LI_ActiveIcons  , MUIM_IconList_DeleteAllImages);
      DoMethod(dock_data->LI_ActiveIcons  , MUIM_List_Clear);
      DoMethod(dock_data->LI_Docks        , MUIM_List_Remove, active - 2);
      DoMethod(data->LV_Pager             , MUIM_List_Remove, active);

      set(data->LV_Pager, MUIA_List_Active, active - 1);
      DoMethod(data->GR_Pager, MUIM_Group_ExitChange);
   }
   else
      DisplayBeep(NULL);

   return(NULL);
}

ULONG MainWindow_About(struct IClass *cl, Object *obj, Msg msg)
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

ULONG MainWindow_About_Finish(struct IClass *cl, Object *obj, struct MUIP_MainWindow_About_Finish *msg)
{
   Object *window = msg->window;

   set(window, MUIA_Window_Open, FALSE);
   set(win, MUIA_Window_Sleep, FALSE);
   DoMethod(app, OM_REMMEMBER, window);
   MUI_DisposeObject(window);

   return(NULL);
}

ULONG MainWindow_DoubleStart(struct IClass *cl, Object *obj, Msg msg)
{
   if(!xget(win, MUIA_Window_Open))
      set(win, MUIA_Window_Open, TRUE);
   DoMethod(win, MUIM_Window_ScreenToFront);
   DoMethod(win, MUIM_Window_ToFront);
   set(win, MUIA_Window_Activate, TRUE);
   return(NULL);
}

ULONG MainWindow_SetPage(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   LONG active;

   if(data->GR_Active == data->GR_Dock)
      DoMethod(data->GR_Dock, MUIM_DockPrefs_GetDock);

   active = xget(data->LV_Pager, MUIA_List_Active);
   if((active == MUIV_List_Active_Off) || (active < 0))
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
            new = data->GR_Menus;
            break;
         default:
            new = data->GR_Dock;
            break;
      }

      if(new && new != data->GR_Active)
      {
         DoMethod(data->GR_Pager, MUIM_Group_InitChange);

         DoMethod(data->GR_Pager, OM_REMMEMBER, data->GR_Active);
         DoMethod(group, OM_ADDMEMBER, data->GR_Active);
         DoMethod(group, OM_REMMEMBER, new);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, new);
         data->GR_Active = new;

         DoMethod(data->GR_Pager, MUIM_Group_ExitChange);
      }

      /** we MUST do this after gr_dock has been added because of MUIM_List_CreateImage in DockPrefs_SetDock() !! */
      if(active > 1)
      {
         set(data->BT_Remove, MUIA_Disabled, FALSE);
         DoMethod(data->GR_Dock, MUIM_DockPrefs_SetDock, active - 2);
      }
      else
         set(data->BT_Remove, MUIA_Disabled, TRUE);
   }

   return(NULL);
}

ULONG MainWindow_InitGroups(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   BOOL success = FALSE;

   data->GR_Menus = data->GR_Dock   = NULL;
   if(data->GR_Menus    = NewObject(CL_MenuPrefs->mcc_Class , NULL, TAG_DONE))
      DoMethod(group, OM_ADDMEMBER, data->GR_Menus);
   if(data->GR_Dock     = NewObject(CL_DockPrefs->mcc_Class , NULL,
      MUIA_DockPrefs_MinLineHeight, find_max(DEFAULT_CONFIGFILE),
      TAG_DONE))
   {
      struct DockPrefs_Data *dock_data = INST_DATA(CL_DockPrefs->mcc_Class, data->GR_Dock);

      DoMethod(group, OM_ADDMEMBER, data->GR_Dock);

      if(dock_data->LI_Docks = (ListObject,
         MUIA_List_ConstructHook , &DockList_ConstructHook,
         MUIA_List_DestructHook  , &DockList_DestructHook,
         End))
         DoMethod(group, OM_ADDMEMBER, dock_data->LI_Docks);
   }

   if(data->GR_Menus && data->GR_Dock)
   {
      success = TRUE;
   }
   return(success);
}

ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static STRPTR ARR_Pages[3];
   struct MainWindow_Data tmp;

   ARR_Pages[0] = GetStr(MSG_GR_Information);
   ARR_Pages[1] = GetStr(MSG_GR_Menus);
   ARR_Pages[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , "NetConnectPrefs © 1997-98 by Michael Neuweiler, Active Technologies",
      MUIA_Window_ID       , MAKE_ID('M','A','I','N'),
      MUIA_Window_AppWindow, TRUE,
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainWindowMenu,0),
      WindowContents       , VGroup,
         Child, tmp.GR_Pager = HGroup,
            Child, VGroup,
               MUIA_Group_Spacing, 0,
               Child, tmp.LV_Pager = ListviewObject,
                  MUIA_CycleChain            , 1,
                  MUIA_Listview_List         , tmp.LI_Pager = NewObject(CL_PagerList->mcc_Class, NULL,
                     MUIA_List_SourceArray   , ARR_Pages,
                     MUIA_List_AdjustWidth   , TRUE,
                     TAG_DONE),
               End,
               Child, HGroup,
                  MUIA_Group_Spacing, 0,
                  Child, tmp.BT_New = MakeButton(MSG_BT_New1),
                  Child, tmp.BT_Remove = MakeButton(MSG_BT_Remove1),
               End,
            End,
            Child, tmp.GR_Active = tmp.GR_Info = VGroup,
               GroupFrame,
               MUIA_Background, "2:ffffffff,ffffffff,ffffffff",
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
//                     MUIA_Bitmap_Transparent   , 0,
                  End,
                  Child, HVSpace,
               End,
               Child, HVSpace,
               Child, CLabel("\033bNetConnectPrefs " VERTAG),
               Child, HVSpace,
#ifdef DEMO
               Child, CLabel("\33bTHIS IS A DEMO VERSION !"),
               Child, HVSpace,
#endif
               Child, HVSpace,
            End,
         End,
         Child, MUI_MakeObject(MUIO_HBar, 2),
         Child, HGroup,
            Child, tmp.BT_Save   = MakeButton(MSG_BT_Save),
            Child, HSpace(0),
            Child, tmp.BT_Use    = MakeButton(MSG_BT_Use),
            Child, HSpace(0),
            Child, tmp.BT_Test   = MakeButton(MSG_BT_Test),
            Child, HSpace(0),
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct MainWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(tmp.LV_Pager, MUIA_List_Active, MUIV_List_Active_Top);
      set(tmp.BT_Remove, MUIA_Disabled, TRUE);
#ifdef DEMO
      set(tmp.BT_New, MUIA_Disabled, TRUE);
#endif

      set(tmp.LV_Pager  , MUIA_ShortHelp, GetStr(MSG_HELP_Pager));
      set(tmp.BT_New    , MUIA_ShortHelp, GetStr(MSG_HELP_NewDock));
      set(tmp.BT_Remove , MUIA_ShortHelp, GetStr(MSG_HELP_RemoveDock));
      set(tmp.BT_Save   , MUIA_ShortHelp, GetStr(MSG_HELP_Save));
      set(tmp.BT_Use    , MUIA_ShortHelp, GetStr(MSG_HELP_Use));
      set(tmp.BT_Test   , MUIA_ShortHelp, GetStr(MSG_HELP_Test));
      set(tmp.BT_Cancel , MUIA_ShortHelp, GetStr(MSG_HELP_Cancel));

      DoMethod(obj            , MUIM_Notify, MUIA_Window_CloseRequest, TRUE   , obj, 2, MUIM_MainWindow_Finish, 0);
      DoMethod(tmp.LV_Pager   , MUIM_Notify, MUIA_List_Active, MUIV_EveryTime , obj, 1, MUIM_MainWindow_SetPage);
#ifndef DEMO
      DoMethod(tmp.BT_New     , MUIM_Notify, MUIA_Pressed            , FALSE  , obj, 1, MUIM_MainWindow_NewDock);
#endif
      DoMethod(tmp.BT_Remove  , MUIM_Notify, MUIA_Pressed            , FALSE  , obj, 1, MUIM_MainWindow_RemoveDock);
      DoMethod(tmp.BT_Cancel  , MUIM_Notify, MUIA_Pressed            , FALSE  , obj, 2, MUIM_MainWindow_Finish, 0);
      DoMethod(tmp.BT_Use     , MUIM_Notify, MUIA_Pressed            , FALSE  , obj, 2, MUIM_MainWindow_Finish, 1);
      DoMethod(tmp.BT_Test    , MUIM_Notify, MUIA_Pressed            , FALSE  , obj, 1, MUIM_MainWindow_Test);
      DoMethod(tmp.BT_Save    , MUIM_Notify, MUIA_Pressed            , FALSE  , obj, 2, MUIM_MainWindow_Finish, 2);

      DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_ABOUT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_About);
      DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_QUIT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 2, MUIM_MainWindow_Finish, 0);
      DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_LOAD)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 2, MUIM_MainWindow_Load, 0);
      DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_SAVE_AS)  , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 2, MUIM_MainWindow_SaveAs, 0);
      DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_RESET)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 2, MUIM_MainWindow_Reset, 0);
      DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_MUI)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
      DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_HELP)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_Help);
   }

   return((ULONG)obj);
}

SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                         : return(MainWindow_New          (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Finish         : return(MainWindow_Finish       (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Test           : return(MainWindow_Test         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_LoadPrefs      : return(MainWindow_LoadPrefs    (cl, obj, (APTR)msg));
      case MUIM_MainWindow_NewDock        : return(MainWindow_NewDock      (cl, obj, (APTR)msg));
      case MUIM_MainWindow_RemoveDock     : return(MainWindow_RemoveDock   (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About          : return(MainWindow_About        (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About_Finish   : return(MainWindow_About_Finish (cl, obj, (APTR)msg));
      case MUIM_MainWindow_DoubleStart    : return(MainWindow_DoubleStart  (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SetPage        : return(MainWindow_SetPage      (cl, obj, (APTR)msg));
      case MUIM_MainWindow_InitGroups     : return(MainWindow_InitGroups   (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Reset          : return(MainWindow_Reset        (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Load           : return(MainWindow_Load         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SaveAs         : return(MainWindow_SaveAs       (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Help           : return(MainWindow_Help         (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}


