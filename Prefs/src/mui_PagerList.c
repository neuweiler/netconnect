#include "mui_PagerList.h"

extern ULONG information_colors[], menus_colors[], dock_colors[];
extern UBYTE information_body[], menus_body[], dock_body[];

///

#ifdef __SASC
SAVEDS ASM LONG PagerList_DisplayFunc(REG(a0) struct Hook *hook, REG(a2) char **array, REG(a1) STRPTR string) {
#else /* gcc */
LONG PagerList_DisplayFunc()
{
   register struct Hook *hook __asm("a0");
   register char **array __asm("a2");
   register STRPTR string __asm("a1");
#endif

   struct PagerList_Data *data = (APTR)hook->h_Data;

   if(string)
   {
      static char buf1[50];
      STRPTR text = string;

      if(!strcmp(string, data->ptr_information))
         sprintf(buf1,"\33c\33O[%08lx]", data->i_information);
      else if(!strcmp(string, data->ptr_menus))
         sprintf(buf1,"\33c\33O[%08lx]", data->i_menus);
      else
      {
         struct Dock *dock = (struct Dock *)string;

         sprintf(buf1,"\33c\33O[%08lx]", data->i_dock);
         text = (dock->Name ? dock->Name : (STRPTR)"");
      }

      *array++ = buf1;
      *array = text;
   }
   return(NULL);
}

ULONG PagerList_Setup(struct IClass *cl, Object *obj, Msg msg)
{
   struct PagerList_Data *data = INST_DATA(cl, obj);

   if(!DoSuperMethodA(cl, obj, msg))
      return(FALSE);

   data->o_information = BodychunkObject,
      MUIA_Background            , MUII_ButtonBack,
      MUIA_Bitmap_SourceColors   , (ULONG *)information_colors,
      MUIA_Bitmap_Width          , INFORMATION_WIDTH ,
      MUIA_Bitmap_Height         , INFORMATION_HEIGHT,
      MUIA_FixWidth              , INFORMATION_WIDTH ,
      MUIA_FixHeight             , INFORMATION_HEIGHT,
      MUIA_Bodychunk_Depth       , INFORMATION_DEPTH ,
      MUIA_Bodychunk_Body        , (UBYTE *)information_body,
      MUIA_Bodychunk_Compression , INFORMATION_COMPRESSION,
      MUIA_Bodychunk_Masking     , INFORMATION_MASKING,
      MUIA_Bitmap_Transparent    , 0,
   End;
   data->o_menus = BodychunkObject,
      MUIA_Background            , MUII_ButtonBack,
      MUIA_Bitmap_SourceColors   , (ULONG *)menus_colors,
      MUIA_Bitmap_Width          , MENUS_WIDTH ,
      MUIA_Bitmap_Height         , MENUS_HEIGHT,
      MUIA_FixWidth              , MENUS_WIDTH ,
      MUIA_FixHeight             , MENUS_HEIGHT,
      MUIA_Bodychunk_Depth       , MENUS_DEPTH ,
      MUIA_Bodychunk_Body        , (UBYTE *)menus_body,
      MUIA_Bodychunk_Compression , MENUS_COMPRESSION,
      MUIA_Bodychunk_Masking     , MENUS_MASKING,
      MUIA_Bitmap_Transparent    , 0,
   End;
   data->o_dock = BodychunkObject,
      MUIA_Background            , MUII_ButtonBack,
      MUIA_Bitmap_SourceColors   , (ULONG *)dock_colors,
      MUIA_Bitmap_Width          , DOCK_WIDTH ,
      MUIA_Bitmap_Height         , DOCK_HEIGHT,
      MUIA_FixWidth              , DOCK_WIDTH ,
      MUIA_FixHeight             , DOCK_HEIGHT,
      MUIA_Bodychunk_Depth       , DOCK_DEPTH ,
      MUIA_Bodychunk_Body        , (UBYTE *)dock_body,
      MUIA_Bodychunk_Compression , DOCK_COMPRESSION,
      MUIA_Bodychunk_Masking     , DOCK_MASKING,
      MUIA_Bitmap_Transparent    , 0,
   End;

   data->i_information  = (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_information, 0);
   data->i_menus        = (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_menus, 0);
   data->i_dock         = (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_dock, 0);

// MUI_RequestIDCMP(obj,IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);

   return(TRUE);
}


ULONG PagerList_Cleanup(struct IClass *cl,Object *obj,Msg msg)
{
   struct PagerList_Data *data = INST_DATA(cl,obj);

   DoMethod(obj, MUIM_List_DeleteImage, data->i_information);
   DoMethod(obj, MUIM_List_DeleteImage, data->i_menus);
   DoMethod(obj, MUIM_List_DeleteImage, data->i_dock);

   if(data->o_information)
      MUI_DisposeObject(data->o_information);
   if(data->o_menus)
      MUI_DisposeObject(data->o_menus);
   if(data->o_dock)
      MUI_DisposeObject(data->o_dock);

// MUI_RejectIDCMP(obj,IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);

   return(DoSuperMethodA(cl, obj ,msg));
}

ULONG PagerList_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
   obj = (Object *)DoSuperNew(cl, obj,
      InputListFrame,
      MUIA_List_Format        , ",",
      MUIA_List_MinLineHeight , MAX(DOCK_HEIGHT, MAX(MENUS_HEIGHT, INFORMATION_HEIGHT)),
      TAG_MORE, msg->ops_AttrList);

   if(obj)
   {
      struct PagerList_Data *data = INST_DATA(cl, obj);

      data->ptr_information   = GetStr(MSG_GR_Information);
      data->ptr_menus         = GetStr(MSG_GR_Menus);

      data->DisplayHook.h_Entry = (VOID *)PagerList_DisplayFunc;
      data->DisplayHook.h_Data  = (APTR)data;
      set(obj, MUIA_List_DisplayHook, &data->DisplayHook);
   }

   return((ULONG)obj);
}

#ifdef __SASC
SAVEDS ASM ULONG PagerList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg) {
#else /* gcc */
ULONG PagerList_Dispatcher()
{
   register struct IClass *cl __asm("a0");
   register Object *obj __asm("a2");
   register Msg msg __asm("a1");
#endif

   switch(msg->MethodID)
   {
      case OM_NEW       : return(PagerList_New     (cl, obj, (APTR)msg));
      case MUIM_Setup   : return(PagerList_Setup   (cl, obj, (APTR)msg));
      case MUIM_Cleanup : return(PagerList_Cleanup (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

