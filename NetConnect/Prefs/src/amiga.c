#include "includes.h"

#include "NetConnect.h"
#include "locale/NetConnect.h"
#include "mui.h"
#include "mui_DockPrefs.h"
#include "protos.h"
#include "images/default_icon.h"

///
/// external variables
extern ULONG default_icon_colors[];
extern UBYTE default_icon_body[];
extern Object *app, *win, *SoundObject;
extern LONG left, top, width, height;

///

ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...)
{
   return(DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}

#ifdef __SASC
SAVEDS ASM VOID DestructFunc(REG(a2) APTR pool, REG(a1) APTR ptr) {
#else /* gcc */
VOID DestructFunc()
{
   register APTR pool __asm("a2");
   register APTR ptr __asm("a1");
#endif

   if(ptr)
      FreeVec(ptr);
}

LONG xget(Object *obj, ULONG attribute)
{
   LONG x;
   get(obj, attribute, &x);
   return(x);
}

Object *MakeKeyLabel2(STRPTR label, STRPTR control_char)
{
   return(KeyLabel2(GetStr(label), *GetStr(control_char)));
}

Object *MakeKeyLLabel2(STRPTR label, STRPTR control_char)
{
   return(KeyLabel2(GetStr(label), *GetStr(control_char)));
}

Object *MakeButton(STRPTR string)
{
   Object *obj = SimpleButton(GetStr(string));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

Object *MakeKeyString(STRPTR string, LONG len, STRPTR control_char)
{
   Object *obj = KeyString(string, len, *(GetStr(control_char)));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

Object *MakeKeyCycle(STRPTR *array, STRPTR control_char)
{
   Object *obj = KeyCycle(array, *(GetStr(control_char)));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

Object *MakeKeySlider(LONG min, LONG max, LONG level, STRPTR control_char)
{
   Object *obj = KeySlider(min, max, level, *(GetStr(control_char)));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

Object *MakeKeyCheckMark(BOOL selected, STRPTR control_char)
{
   Object *obj = KeyCheckMark(selected, *(GetStr(control_char)));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only)
{
   Object *obj = PopaslObject,
      MUIA_Popstring_String, string,
      MUIA_Popstring_Button, PopButton((drawers_only ? MUII_PopDrawer : MUII_PopFile)),
      MUIA_Popasl_Type     , ASL_FileRequest,
      ASLFR_TitleText      , GetStr(title),
      ASLFR_DrawersOnly    , drawers_only,
   End;
   return(obj);
}

/*
 * loads bitmap image of an icon.
 * cols, bmhd and body
 * are beeing allocated
 */

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

/*
 * calls load_image() and generates
 * a BodychunkObject.
 * nothing is changed in icon by
 * this routine (but load_image does !)
 */

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
            MUIA_Frame                 , (icon->Flags & IFL_DrawFrame ? MUIV_Frame_Button : MUIV_Frame_None),
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
               MUIA_Frame           , (icon->Flags & IFL_DrawFrame ? MUIV_Frame_Button : MUIV_Frame_None),
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
         MUIA_Frame                 , (icon->Flags & IFL_DrawFrame ? MUIV_Frame_Button : MUIV_Frame_None),
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

BOOL find_list(struct DockPrefs_Data *data, Object **list, struct Icon **icon)
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

LONG get_file_size(STRPTR file)
{
   struct FileInfoBlock *fib;
   BPTR lock;
   LONG size = -1;

   if(lock = Lock(file, ACCESS_READ))
   {
      if(fib = AllocDosObject(DOS_FIB, NULL))
      {
         if(Examine(lock, fib))
            size = (fib->fib_DirEntryType > 0 ? -2 : fib->fib_Size);

         FreeDosObject(DOS_FIB, fib);
      }
      UnLock(lock);
   }
   return(size);
}

STRPTR LoadFile(STRPTR file)
{
   LONG size;
   STRPTR buf = NULL;
   BPTR fh;
   BOOL success = FALSE;

   if((size = get_file_size(file)) > -1)
   {
      if(buf = AllocVec(size + 1, MEMF_ANY))
      {
         if(fh = Open(file, MODE_OLDFILE))
         {
            if(Read(fh, buf, size) == size)
               success = TRUE;
            Close(fh);
         }
         buf[size] = NULL;    // We need buffers that are terminated by a zero
      }
   }

   if(!success && buf)
   {
      FreeVec(buf);
      buf = NULL;
   }

   return(buf);
}

int find_max(STRPTR file)
{
   struct BitMapHeader *bmhd;
   struct IFFHandle *Handle1, *Handle2;
   struct ContextNode *cn1, *cn2;
   struct StoredProperty *sp;
   struct DiskObject *disk_object;
   int max_height = DEFAULT_ICON_HEIGHT;
   char buffer[MAXPATHLEN];

   if(Handle1 = AllocIFF())
   {
      if(Handle1->iff_Stream = Open(file, MODE_OLDFILE))
      {
         InitIFFasDOS(Handle1);
         if(!(OpenIFF(Handle1, IFFF_READ)))
         {
            if(!(StopChunk(Handle1, ID_NTCN, ID_ICOI)))
            {
               while(!ParseIFF(Handle1, IFFPARSE_SCAN))
               {
                  cn1 = CurrentChunk(Handle1);
                  if(cn1->cn_ID == ID_ICOI)
                  {
                     if(ReadChunkBytes(Handle1, buffer, MIN(MAXPATHLEN, cn1->cn_Size)) == MIN(MAXPATHLEN, cn1->cn_Size))
                     {
                        bmhd = NULL;         // this will indicate if the file was a valid iff brush
                        if(Handle2=AllocIFF())
                        {
                           if(Handle2->iff_Stream = Open(buffer, MODE_OLDFILE))
                           {
                              InitIFFasDOS(Handle2);
                              if(!OpenIFF(Handle2, IFFF_READ))
                              {
                                 if(!ParseIFF(Handle2, IFFPARSE_STEP))
                                 {
                                    if((cn2 = CurrentChunk(Handle2)) && (cn2->cn_ID == ID_FORM))
                                    {
                                       if(cn2->cn_Type == ID_ILBM)
                                       {
                                          if(!PropChunk(Handle2, ID_ILBM, ID_BMHD) &&
                                             !PropChunk(Handle2, ID_ILBM, ID_CMAP) &&
                                             !StopChunk(Handle2, ID_ILBM, ID_BODY) &&
                                             !StopOnExit(Handle2, ID_ILBM, ID_FORM) &&
                                             !ParseIFF(Handle2, IFFPARSE_SCAN))
                                          {
                                             if(sp = FindProp(Handle2, ID_ILBM, ID_BMHD))
                                             {
                                                bmhd = (struct BitMapHeader *)sp->sp_Data;
                                                max_height = MAX(max_height, bmhd->bmh_Height);
                                             }
                                          }
                                       }
                                    }
                                 }
                                 CloseIFF(Handle2);
                              }
                              Close(Handle2->iff_Stream);
                           }
                           FreeIFF(Handle2);
                        }
                        if(!bmhd)         // wasn't a iff brush, might be an icon
                        {
                           if(!strcmp(&buffer[strlen(buffer) - 5], ".info"))
                              buffer[strlen(buffer) - 5] = NULL;
                           if(disk_object = GetDiskObjectNew(buffer))
                           {
                              max_height = MAX(max_height, ((struct Image *)(disk_object->do_Gadget.GadgetRender))->Height);
                              FreeDiskObject(disk_object);
                           }
                        }
                     }
                  }
               }
            }
            CloseIFF(Handle1);
         }
         Close(Handle1->iff_Stream);
      }
      FreeIFF(Handle1);
   }

   return(max_height);
}

#ifdef __SASC
SAVEDS ASM LONG AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x) {
#else /* gcc */
LONG AppMsgFunc()
{
   register APTR obj __asm("a2");
   register struct AppMessage **x __asm("a1");
#endif
   struct WBArg *ap;
   struct AppMessage *amsg = *x;
   STRPTR buf;

   ap = amsg->am_ArgList;
   if(amsg->am_NumArgs)
   {
      if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
      {
         NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
         AddPart(buf, ap->wa_Name, MAXPATHLEN);
         setstring(obj, buf);
         FreeVec(buf);
      }
   }
   return(NULL);
}

#ifdef __SASC
SAVEDS ASM LONG Editor_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x) {
#else /* gcc */
LONG Editor_AppMsgFunc()
{
   register APTR obj __asm("a2");
   register struct AppMessage **x __asm("a1");
#endif

   struct WBArg *ap;
   struct AppMessage *amsg = *x;
   STRPTR buf;
   int i;

   if(amsg->am_NumArgs)
   {
      if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
      {
         for(ap = amsg->am_ArgList, i = 0; i < amsg->am_NumArgs; i++, ap++)
         {
            NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
            AddPart(buf, ap->wa_Name, MAXPATHLEN);
            DoMethod(obj, MUIM_List_InsertSingle, buf, MUIV_List_Insert_Active);
         }
         set(obj, MUIA_UserData, 1);   // contents have been changed
         FreeVec(buf);
      }
   }
   return(NULL);
}

BOOL editor_load(STRPTR file, Object *editor)
{
   STRPTR buf, ptr1, ptr2;
   LONG size;
   BOOL success = FALSE;

   if(file && *file)
   {
      set(editor, MUIA_List_Quiet, TRUE);
      DoMethod(editor, MUIM_List_Clear);

      size = get_file_size(file);
      if(buf = LoadFile(file))
      {
         ptr1 = buf;
         while(ptr1 && ptr1 < buf + size)
         {
            if(ptr2 = strchr(ptr1, '\n'))
            {
               *ptr2 = NULL;
               DoMethod(editor, MUIM_List_InsertSingle, ptr1, MUIV_List_Insert_Bottom);
               ptr1 = ptr2 + 1;
            }
            else
               ptr1 = NULL;
         }
         FreeVec(buf);
         success = TRUE;
      }
      set(editor, MUIA_List_Quiet, FALSE);
      set(editor, MUIA_List_Active, MUIV_List_Active_Top);
   }
   return(success);
}

BOOL editor_save(STRPTR file, Object *editor)
{
   BPTR fh;
   STRPTR ptr;
   int i;

   if(file && *file)
   {
      if(fh = Open(file, MODE_NEWFILE))
      {
         i = 0;
         FOREVER
         {
            DoMethod(editor, MUIM_List_GetEntry, i++, &ptr);
            if(!ptr)
               break;
            FPrintf(fh, "%ls\n", ptr);
         }

         Close(fh);
         set(editor, MUIA_UserData, NULL);
         return(TRUE);
      }
   }
   return(FALSE);
}

BOOL editor_checksave(STRPTR file, Object *editor)
{
   if(xget(editor, MUIA_UserData))
   {
      if(MUI_Request(app, (Object *)xget(editor, MUIA_WindowObject), 0, 0, GetStr(MSG_BT_SaveDiscardChanges), GetStr(MSG_LA_ScriptModified)))
         return(editor_save(file, editor));
   }
   return(FALSE);
}

VOID play_sound(STRPTR file, LONG volume)
{
   if(file && *file && DataTypesBase)
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

BOOL CopyFile(STRPTR infile, STRPTR outfile)
{
   BPTR in, out;
   char buf[100];
   LONG i;
   BOOL success = FALSE;

   if(in = Open(infile, MODE_OLDFILE))
   {
      if(out = Open(outfile, MODE_NEWFILE))
      {
         success = TRUE;
         while(i = Read(in, buf, 100))
         {
            if(Write(out, buf, i) != i)
            {
               success = FALSE;
               break;
            }
         }
         Close(out);
      }
      Close(in);
   }
   return(success);
}

#ifdef __SASC
SAVEDS ASM VOID IntuiMsgFunc(REG(a1) struct IntuiMessage *imsg,REG(a2) struct FileRequester *req) {
#else /* gcc */
VOID IntuiMsgFunc()
{
   register struct FileRequester *req __asm("a2");
   register struct IntuiMessage *imsg __asm("a1");
#endif

   if(imsg->Class == IDCMP_REFRESHWINDOW)
      DoMethod(req->fr_UserData, MUIM_Application_CheckRefresh);
}

char *getfilename(Object *win, STRPTR title, STRPTR file, BOOL save)
{
   static char buf[512];
   struct FileRequester *req;
   struct Window *w;
   char *res = NULL;
   static const struct Hook IntuiMsgHook = { { 0,0 }, (VOID *)IntuiMsgFunc, NULL, NULL };

   get(win, MUIA_Window_Window, &w);
   if(left == -1)
   {
      left     = w->LeftEdge+w->BorderLeft + 2;
      top      = w->TopEdge+w->BorderTop + 2;
      width    = w->Width-w->BorderLeft-w->BorderRight - 4;
      height   = w->Height-w->BorderTop-w->BorderBottom - 4;
   }

   if(req = MUI_AllocAslRequestTags(ASL_FileRequest,
      ASLFR_Window         , w,
      ASLFR_TitleText      , title,
      ASLFR_InitialLeftEdge, left,
      ASLFR_InitialTopEdge , top,
      ASLFR_InitialWidth   , width,
      ASLFR_InitialHeight  , height,
      ASLFR_InitialFile    , (file ? file : (STRPTR)""),
      ASLFR_DoSaveMode     , save,
      ASLFR_RejectIcons    , TRUE,
      ASLFR_UserData       , app,
      ASLFR_IntuiMsgFunc   , &IntuiMsgHook,
      TAG_DONE))
   {
      set(app, MUIA_Application_Sleep, TRUE);
      if(MUI_AslRequestTags(req, TAG_DONE))
      {
         if(*req->fr_File)
         {
            res = buf;
            strncpy(buf, req->fr_Drawer, sizeof(buf));
            AddPart(buf, req->fr_File, sizeof(buf));
         }
         left     = req->fr_LeftEdge;
         top      = req->fr_TopEdge;
         width    = req->fr_Width;
         height   = req->fr_Height;
      }
      MUI_FreeAslRequest(req);
      set(app, MUIA_Application_Sleep, FALSE);
   }
   return(res);
}

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
