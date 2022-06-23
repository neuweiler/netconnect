/// includes
#include "includes.h"

#include "../../locale/Strings.h"
#include "protos.h"
#include "../../images/default_icon.h"
#include "../../../WBStart.h"
#include "mui_Dock.h"

//#include <proto/muimaster.h>
#include <clib/muimaster_protos.h>

//#include <libraries/mui.h>

///
/// external variables
extern struct Catalog *cat;
extern Object *SoundObject, *app, *menu_list, *group;
extern struct MUI_CustomClass *CL_Button;
extern ULONG default_icon_colors[];
extern UBYTE default_icon_body[];
extern struct MUI_CustomClass *CL_Dock;

///

/// DoSuperMethod
ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...)
{
   return(DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}

///
/// xget
LONG xget(Object *obj, ULONG attribute)
{
   LONG x;
   get(obj, attribute, &x);
   return(x);
}

///
/// GetStr
STRPTR GetStr(STRPTR idstr)
{
   STRPTR local;

   local = idstr + 2;

   if(LocaleBase)
      return((STRPTR)GetCatalogStr(cat, *(UWORD *)idstr, local));

   return(local);
}

///
/// play_sound
VOID play_sound(STRPTR file, LONG volume)
{
   if(*file && DataTypesBase)
   {
      if(SoundObject)
         DisposeDTObject(SoundObject);

      if(SoundObject = NewDTObject(file,
         DTA_SourceType , DTST_FILE,
         DTA_GroupID    , GID_SOUND,
         SDTA_Volume    , volume,
         SDTA_Cycles    , 1,
      TAG_DONE))
      {
         DoMethod(SoundObject, DTM_TRIGGER, NULL, STM_PLAY, NULL);
      }
   }
}

///
/// BuildCommandLine
ULONG BuildCommandLine(char *buf, struct Program *program, BPTR curdir, struct AppMessage *msg)
{
   ULONG cmdlen;      /* Command line length */
   char *lp;          /* Pointer to current cmdline pos. */
   STRPTR com = program->File;

   *buf = NULL;
   if(program->Flags & PRG_SCRIPT)
      strcpy(buf, "c:Execute ");
   if(program->Flags & PRG_AREXX)
      strcpy(buf, "SYS:Rexxc/rx ");

   if(com)
      strcat(buf, com);

   if(lp = strchr(buf, '['))
   {
      *lp = NULL;
      lp--;
      if(*lp == ' ')
         *lp = NULL;
   }

   cmdlen = strlen(buf);
   lp = buf + cmdlen;

   if(msg)
   {
      STRPTR dir; /* Buffer for file names */

      if(dir = AllocVec(MAXPATHLEN, MEMF_ANY))
      {
         struct WBArg *wa = msg->am_ArgList;    /* Pointer to WBArgs */
         int i;                                 /* Counter for WBArgs */

         for(i = msg->am_NumArgs; i; i--, wa++)
         {
            char *name, *space;
            ULONG namelen;

            if(!wa->wa_Lock)
               continue;

            if(cmdlen > CMDLINELEN - 2)
               break;
            *lp++=' ';
            cmdlen++;

            /* Build parameter from Lock & name */
            if(*(wa->wa_Name))
            {
               if(SameLock(curdir, wa->wa_Lock) == LOCK_SAME)
                  name=wa->wa_Name;
               else
               {
                  if(!NameFromLock(wa->wa_Lock, dir, MAXPATHLEN))
                     continue;
                  if(!AddPart(dir, wa->wa_Name, MAXPATHLEN))
                     continue;
                  name = dir;
               }
            }
            else     // no filename => drawer
            {
               if(!NameFromLock(wa->wa_Lock, dir, MAXPATHLEN))
                  continue;
               name = dir;
            }
            namelen = strlen(name);

            if(space = strchr(name,' '))
               namelen += 2;

            if(cmdlen + namelen > CMDLINELEN - 2)
               break;

            if(space)
               *lp++ = '"';
            strcpy(lp, name);
            lp += namelen;
            if(space)
            {
               lp--;
               *(lp-1) = '"';
            }
            cmdlen += namelen;
         }
         FreeVec(dir);
      }
   }

   if(com)
   {
      if((com = strchr(com, ']')) && (cmdlen + strlen(++com) < CMDLINELEN - 1))
      {
         strcpy(lp, com);
         lp = lp + strlen(lp);
      }
      else
         *lp = NULL;
   }
   else
      *lp = NULL;

   return((ULONG)(lp - buf));
}

///
/// StartCLIProgram
BOOL StartCLIProgram(struct Program *program, struct AppMessage *msg)
{
   char *cmd;     /* Buffer for command line */
   BOOL success = FALSE;

   if(cmd = AllocVec(CMDLINELEN, MEMF_ANY))
   {
      BPTR newcd = NULL;

      if(program->CurrentDir)
         newcd = Lock(program->CurrentDir, SHARED_LOCK);
      if(!newcd)
         newcd = Lock("SYS:", SHARED_LOCK);
      if(newcd)
      {
         BPTR ifh, ofh;
         BPTR oldcd = CurrentDir(newcd);

         BuildCommandLine(cmd, program, newcd, msg);

         if(ofh = Open((program->OutputFile ? program->OutputFile : (STRPTR)"NIL:"), MODE_NEWFILE))
         {
            if(ifh = Open("NIL:", MODE_OLDFILE))
            {
               if(SystemTags(cmd,
                  SYS_Output     , ofh,
                  SYS_Input      , ifh,
                  SYS_Asynch     , program->Flags & PRG_Asynch,
                  SYS_UserShell  , TRUE,
                  NP_StackSize   , program->Stack,
                  NP_Priority    , program->Priority,
                  TAG_DONE) != -1)
                     success = TRUE;

               if(!success || !(program->Flags & PRG_Asynch))
                  Close(ifh);
            }
            if(!success || !(program->Flags & PRG_Asynch))
               Close(ofh);
         }
         CurrentDir(oldcd);
         UnLock(newcd);
      }
      FreeVec(cmd);
   }
   return(success);
}

///
/// StartWBProgram
BOOL StartWBProgram(struct Program *program, struct AppMessage *msg)
{
   struct MsgPort *hp, *mp;
   struct WBStartMsg wbsm;
   BOOL success = FALSE;

   if(mp = CreateMsgPort())
   {
      if(program->CurrentDir)
         wbsm.wbsm_DirLock = Lock(program->CurrentDir, SHARED_LOCK);
      else
      {
         STRPTR buf, ptr;

         wbsm.wbsm_DirLock = NULL;
         if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY | MEMF_CLEAR))
         {
            if(program->File)
               strncpy(buf, program->File, MAXPATHLEN);
            if(ptr = FilePart(buf))
               *ptr = NULL;
            wbsm.wbsm_DirLock = Lock(buf, SHARED_LOCK);

            FreeVec(buf);
         }
      }

      wbsm.wbsm_Msg.mn_Node.ln_Pri  = 0;
      wbsm.wbsm_Msg.mn_ReplyPort    = mp;
      wbsm.wbsm_Name                = program->File;
      wbsm.wbsm_Stack               = program->Stack;
      wbsm.wbsm_Prio                = program->Priority;
      wbsm.wbsm_NumArgs             = msg ? msg->am_NumArgs : NULL;
      wbsm.wbsm_ArgList             = msg ? msg->am_ArgList : NULL;

      Forbid();
      if(hp = FindPort(WBS_PORTNAME))
         PutMsg(hp, (struct Message *)&wbsm);
      Permit();

      /* No WBStart-Handler, try to start it! */
      if(!hp)
      {
         BPTR ifh = Open("NIL:", MODE_NEWFILE);
         BPTR ofh = Open("NIL:", MODE_OLDFILE);

         if(SystemTags(WBS_LOADNAME,
            SYS_Input      , ifh,
            SYS_Output     , ofh,
            SYS_Asynch     , TRUE,
            SYS_UserShell  , TRUE,
            NP_ConsoleTask , NULL,
            NP_WindowPtr   , NULL,
            TAG_DONE) != -1)
         {
            int i;

            for(i = 0; i < 10; i++)
            {
               Forbid();
               if(hp = FindPort(WBS_PORTNAME))
                  PutMsg(hp, (struct Message *)&wbsm);
               Permit();
               if(hp)
                  break;
               Delay(25);
            }
         }
         else
         {
            Close(ofh);
            Close(ifh);
         }
      }

      if(hp)
      {
         WaitPort(mp);
         GetMsg(mp);
         success = wbsm.wbsm_Stack;    // Has tool been started?
      }

      if(wbsm.wbsm_DirLock)
         UnLock(wbsm.wbsm_DirLock);
      DeleteMsgPort(mp);
   }

   return(success);
}

///
/// StartProgram
VOID StartProgram(struct Program *program, struct AppMessage *msg)
{
   struct AppMessage *args = (program->Flags & PRG_Arguments) ? msg : NULL;
   BOOL success = FALSE;

   set(app, MUIA_Application_Sleep, TRUE);
   if(program->Flags & PRG_WORKBENCH)
      success = StartWBProgram(program, args);
   else
      success = StartCLIProgram(program, args);

   if(!success)
      DisplayBeep(NULL);
   set(app, MUIA_Application_Sleep, FALSE);
}

///
/// update_string
STRPTR update_string(STRPTR old, STRPTR source)
{
   if(source && *source)
   {
      STRPTR new = NULL;

      if(old)
      {
         if(strlen(old) >= strlen(source))
            new = old;
         else
            FreeVec(old);
      }
      if(!new)
         new = AllocVec(strlen(source) + 1, MEMF_ANY);

      if(new)
      {
         strcpy(new, source);
         return(new);
      }
   }
   else
   {
      if(old)
         FreeVec(old);
   }
   return(NULL);
}

///

/// load_image
UBYTE *load_image(STRPTR file, struct BitMapHeader **bmhd, ULONG **cols)
{
   struct IFFHandle *Handle;
   struct ContextNode *cn;
   struct StoredProperty *sp;
   UBYTE *body = NULL;

   if(!file || !cols || !bmhd)
      return(NULL);

   if(Handle = AllocIFF())
   {
      if(Handle->iff_Stream = Open(file, MODE_OLDFILE))
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
                        {
                           UBYTE *src;
                           int i;

                           src = sp->sp_Data;
                           if(*cols = AllocVec(sizeof(ULONG) * sp->sp_Size, MEMF_ANY))
                           {
                              for (i = 0; i < sp->sp_Size; i++)
                                 (*cols)[i] = to32(src[i]);
                           }
                        }

                        if(sp = FindProp(Handle, ID_ILBM, ID_BMHD))
                        {
                           LONG size = CurrentChunk(Handle)->cn_Size;

                           if(*bmhd = AllocVec(sizeof(struct BitMapHeader), MEMF_ANY))
                           {
                              memcpy(*bmhd, sp->sp_Data, sizeof(struct BitMapHeader));

                              if(body = AllocVec(size, MEMF_ANY))
                                 ReadChunkBytes(Handle, body, size);
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

   if(!body)
   {
      if(*cols)
         FreeVec(*cols);
      *cols = NULL;
      if(*bmhd)
         FreeVec(*bmhd);
      *bmhd = NULL;
   }

   return(body);
}

///
/// init_icon
VOID init_icon(struct Icon *icon)
{
   if(!icon)
      return;

   icon->body        = NULL;
   icon->cols        = NULL;
   icon->bodychunk   = NULL;
   icon->list        = NULL;
   icon->edit_window = NULL;
   icon->disk_object = NULL;

   if(icon->ImageFile)
   {
      struct BitMapHeader *bmhd = NULL;

      if(icon->body = load_image(icon->ImageFile, &bmhd, &(icon->cols)))
      {
         icon->bodychunk = BodychunkObject,
            MUIA_Background            , MUII_ButtonBack,
            MUIA_Bitmap_SourceColors   , icon->cols,
            MUIA_Bitmap_Width          , bmhd->bmh_Width,
            MUIA_Bitmap_Height         , bmhd->bmh_Height,
            MUIA_FixWidth              , bmhd->bmh_Width,
            MUIA_FixHeight             , bmhd->bmh_Height,
            MUIA_Bodychunk_Depth       , bmhd->bmh_Depth,
            MUIA_Bodychunk_Body        , icon->body,
            MUIA_Bodychunk_Compression , bmhd->bmh_Compression,
            MUIA_Bodychunk_Masking     , bmhd->bmh_Masking,
            MUIA_Bitmap_Transparent    , 0,
            End;
      }
      if(bmhd)
         FreeVec(bmhd);

      if(!icon->bodychunk)
      {
         char file[MAXPATHLEN];

         strncpy(file, icon->ImageFile, MAXPATHLEN);
         if(!strcmp(&file[strlen(file) - 5], ".info"))
            file[strlen(file) - 5] = NULL;
         if(icon->disk_object = GetDiskObject(file))
         {
            if(!(icon->bodychunk = ImageObject,
               MUIA_Background      , MUII_ButtonBack,
               MUIA_Image_OldImage  , icon->disk_object->do_Gadget.GadgetRender,
            End))
            {
               FreeDiskObject(icon->disk_object);
               icon->disk_object = NULL;
            }
         }
      }
   }
   if(!icon->bodychunk)
   {
      icon->body = NULL;
      icon->cols = NULL;
      icon->disk_object = NULL;
      icon->bodychunk = BodychunkObject,
         MUIA_Background            , MUII_ButtonBack,
         MUIA_Bitmap_SourceColors   , (ULONG *)default_icon_colors,
         MUIA_Bitmap_Width          , DEFAULT_ICON_WIDTH ,
         MUIA_Bitmap_Height         , DEFAULT_ICON_HEIGHT,
         MUIA_FixWidth              , DEFAULT_ICON_WIDTH ,
         MUIA_FixHeight             , DEFAULT_ICON_HEIGHT,
         MUIA_Bodychunk_Depth       , DEFAULT_ICON_DEPTH ,
         MUIA_Bodychunk_Body        , (UBYTE *)default_icon_body,
         MUIA_Bodychunk_Compression , DEFAULT_ICON_COMPRESSION,
         MUIA_Bodychunk_Masking     , DEFAULT_ICON_MASKING,
         MUIA_Bitmap_Transparent    , 0,
      End;
   }
}

///
/// create_button
Object *create_button(struct Icon *icon, Object *current_dock)
{
   Object *button = NULL;
   APTR font = NULL;
   UBYTE type = 2;

   if(current_dock)
   {
      struct Dock_Data *dock_data = INST_DATA(CL_Dock->mcc_Class, current_dock);

      type = (dock_data->dock.Flags & DFL_Icon ? (dock_data->dock.Flags & DFL_Text ? 0 : 1) : 2);
      font = dock_data->TxtFont;
   }

   switch(type)
   {
      case 1:
         init_icon(icon);
         if(icon->bodychunk)
         {
            button = NewObject(CL_Button->mcc_Class, NULL,
               MUIA_NetConnect_Icon, icon,
               Child, icon->bodychunk,
               TAG_DONE);
         }
         break;
      case 2:
         icon->Flags |= IFL_DrawFrame; // always draw a frame for text buttons
         button = NewObject(CL_Button->mcc_Class, NULL,
            MUIA_NetConnect_Icon, icon,
            Child, TextObject,
               MUIA_Font         , (font ? font : (APTR)MUIV_Font_Button),
               MUIA_Text_PreParse, "\33c",
               MUIA_Text_Contents, icon->Name,
            End,
            TAG_DONE);
         break;
      default:
         init_icon(icon);
         if(icon->bodychunk)
         {
            button = VGroup,
               MUIA_Group_Spacing, 0,
               MUIA_InnerBottom  , 0,
               MUIA_InnerLeft    , 0,
               MUIA_InnerTop     , 0,
               MUIA_InnerRight   , 0,
               Child, NewObject(CL_Button->mcc_Class, NULL,
                  MUIA_NetConnect_Icon, icon,
                  Child, icon->bodychunk,
               TAG_DONE),
               Child, RectangleObject, MUIA_Weight, 1, End,
               Child, TextObject,
                  MUIA_Font, MUIV_Font_Tiny,
                  MUIA_Text_PreParse, "\33c",
                  MUIA_Text_Contents, icon->Name,
               End,
            End;
         }
         break;
   }
   return(button);
}

///
/// load_program
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

///
/// load_icon
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

///
/// load_dock
VOID load_dock(struct IFFHandle *Handle, struct ContextNode *Chunk)
{
   struct Dock dock;
   Object *window = NULL, *dock_group = NULL;
#ifdef DEMO
   int buttons = 0;
#endif
   Object *button;
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
   if(window = NewObject(CL_Dock->mcc_Class, NULL,
      MUIA_NetConnect_Dock , &dock,
      WindowContents       , dock_group = RowGroup(dock.Rows),
         MUIA_Group_Spacing, 0,
         MUIA_InnerBottom  , 0,
         MUIA_InnerLeft    , 0,
         MUIA_InnerTop     , 0,
         MUIA_InnerRight   , 0,
      End,
      TAG_DONE))
   {
      DoMethod(app, OM_ADDMEMBER, window);
      DoMethod(dock_group, MUIM_Group_InitChange);
   }
   while(Chunk->cn_ID == ID_ICON)
   {
#ifdef DEMO
      if(buttons++ < 12)
#endif
      {
         load_icon(Handle, Chunk, &icon);
         if(dock_group)
         {
            if(button = create_button(&icon, window))
               DoMethod(dock_group, OM_ADDMEMBER, button);
         }
      }
      if(ParseIFF(Handle, IFFPARSE_SCAN))
         break;
      Chunk = CurrentChunk(Handle);
   }

   if(dock_group)
   {
      DoMethod(dock_group, MUIM_Group_ExitChange);
      if(dock.Flags & DFL_PopUp)
         set(window, MUIA_Window_Open, TRUE);
   }
}

///
/// load_config
BOOL load_config(VOID)
{
   BOOL success = FALSE;
   struct IFFHandle  *Handle;
   struct ContextNode   *Chunk;
   UWORD menu_id;
   int docks = 0;

   if(Handle = AllocIFF())
   {
      if(Handle->iff_Stream = Open(DEFAULT_CONFIGFILE, MODE_OLDFILE))
      {
         InitIFFasDOS(Handle);
         if(!(OpenIFF(Handle, IFFF_READ)))
         {
            if(!(StopChunks(Handle, Stops, NUM_STOPS)))
            {
               set(menu_list, MUIA_List_Quiet,TRUE);
               DoMethod(menu_list, MUIM_List_Clear);
               menu_id = 0;
               while(!ParseIFF(Handle, IFFPARSE_SCAN))
               {
                  Chunk = CurrentChunk(Handle);

                  if(Chunk->cn_ID == ID_MENU)
                  {
                     struct MenuEntry menu, *menu_ptr;
                     struct Program program;

                     bzero(&menu, sizeof(struct MenuEntry));

                     if(menu.Name = AllocVec(Chunk->cn_Size, MEMF_ANY | MEMF_CLEAR))
                        ReadChunkBytes(Handle, menu.Name, Chunk->cn_Size);

                     menu.Id = menu_id++;
                     DoMethod(menu_list, MUIM_List_InsertSingle, &menu, MUIV_List_Insert_Bottom);

                     DoMethod(menu_list, MUIM_List_GetEntry, xget(menu_list, MUIA_List_Entries) - 1, &menu_ptr);
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
                  }

                  if(Chunk->cn_ID == ID_DOCK)
                  {
#ifdef DEMO
                     if(docks < 1)
#endif
                     load_dock(Handle, Chunk);
                     docks++;
                  }

                  if(Chunk->cn_ID == ID_ICON && group)
                  {
                     struct Icon icon;
                     Object *button;

                     load_icon(Handle, Chunk, &icon);
                     if(icon.Hotkey)
                     {
                        if(button = create_button(&icon, NULL))
                           DoMethod(group, OM_ADDMEMBER, button);
                     }
                  }
               }
               set(menu_list, MUIA_List_Quiet, FALSE);
               success = TRUE;
            }
            CloseIFF(Handle);
         }
         Close(Handle->iff_Stream);
      }
      FreeIFF(Handle);
   }

   return(success);
}

/// BrokerFunc
int BrokerFunc() {
   register CxMsg *msg __asm("a1");

   if(CxMsgType(msg) == CXM_IEVENT)
      DoMethod((Object *)CxMsgID(msg), MUIM_Hotkey_Trigger);
   return(0);
}
