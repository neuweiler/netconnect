/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "mui.h"
#include "mui_Led.h"
#include "protos.h"

///
/// images
#define USE_BLACK_HEADER
#define USE_BLACK_BODY
#define USE_BLACK_COLORS
#include "images/black.h"

#define USE_GREEN_HEADER
#define USE_GREEN_BODY
#define USE_GREEN_COLORS
#include "images/green.h"

#define USE_YELLOW_HEADER
#define USE_YELLOW_BODY
#define USE_YELLOW_COLORS
#include "images/yellow.h"

#define USE_ORANGE_HEADER
#define USE_ORANGE_BODY
#define USE_ORANGE_COLORS
#include "images/orange.h"

#define USE_RED_HEADER
#define USE_RED_BODY
#define USE_RED_COLORS
#include "images/red.h"

///

/// Led_New
ULONG ASM Led_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Group_PageMode, TRUE,
      Child, BodychunkObject,
         MUIA_FixWidth             , BLACK_WIDTH,
         MUIA_FixHeight            , BLACK_HEIGHT,
         MUIA_Bitmap_Width         , BLACK_WIDTH ,
         MUIA_Bitmap_Height        , BLACK_HEIGHT,
         MUIA_Bodychunk_Depth      , BLACK_DEPTH ,
         MUIA_Bodychunk_Body       , (UBYTE *)black_body,
         MUIA_Bodychunk_Compression, BLACK_COMPRESSION,
         MUIA_Bodychunk_Masking    , BLACK_MASKING,
         MUIA_Bitmap_SourceColors  , (ULONG *)black_colors,
         MUIA_Bitmap_Transparent   , 0,
      End,
      Child, BodychunkObject,
         MUIA_FixWidth             , GREEN_WIDTH,
         MUIA_FixHeight            , GREEN_HEIGHT,
         MUIA_Bitmap_Width         , GREEN_WIDTH ,
         MUIA_Bitmap_Height        , GREEN_HEIGHT,
         MUIA_Bodychunk_Depth      , GREEN_DEPTH ,
         MUIA_Bodychunk_Body       , (UBYTE *)green_body,
         MUIA_Bodychunk_Compression, GREEN_COMPRESSION,
         MUIA_Bodychunk_Masking    , GREEN_MASKING,
         MUIA_Bitmap_SourceColors  , (ULONG *)green_colors,
         MUIA_Bitmap_Transparent   , 0,
      End,
      Child, BodychunkObject,
         MUIA_FixWidth             , YELLOW_WIDTH,
         MUIA_FixHeight            , YELLOW_HEIGHT,
         MUIA_Bitmap_Width         , YELLOW_WIDTH ,
         MUIA_Bitmap_Height        , YELLOW_HEIGHT,
         MUIA_Bodychunk_Depth      , YELLOW_DEPTH ,
         MUIA_Bodychunk_Body       , (UBYTE *)yellow_body,
         MUIA_Bodychunk_Compression, YELLOW_COMPRESSION,
         MUIA_Bodychunk_Masking    , YELLOW_MASKING,
         MUIA_Bitmap_SourceColors  , (ULONG *)yellow_colors,
         MUIA_Bitmap_Transparent   , 0,
      End,
      Child, BodychunkObject,
         MUIA_FixWidth             , ORANGE_WIDTH,
         MUIA_FixHeight            , ORANGE_HEIGHT,
         MUIA_Bitmap_Width         , ORANGE_WIDTH ,
         MUIA_Bitmap_Height        , ORANGE_HEIGHT,
         MUIA_Bodychunk_Depth      , ORANGE_DEPTH ,
         MUIA_Bodychunk_Body       , (UBYTE *)orange_body,
         MUIA_Bodychunk_Compression, ORANGE_COMPRESSION,
         MUIA_Bodychunk_Masking    , ORANGE_MASKING,
         MUIA_Bitmap_SourceColors  , (ULONG *)orange_colors,
         MUIA_Bitmap_Transparent   , 0,
      End,
      Child, BodychunkObject,
         MUIA_FixWidth             , RED_WIDTH,
         MUIA_FixHeight            , RED_HEIGHT,
         MUIA_Bitmap_Width         , RED_WIDTH ,
         MUIA_Bitmap_Height        , RED_HEIGHT,
         MUIA_Bodychunk_Depth      , RED_DEPTH ,
         MUIA_Bodychunk_Body       , (UBYTE *)red_body,
         MUIA_Bodychunk_Compression, RED_COMPRESSION,
         MUIA_Bodychunk_Masking    , RED_MASKING,
         MUIA_Bitmap_SourceColors  , (ULONG *)red_colors,
         MUIA_Bitmap_Transparent   , 0,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Led_Data *data = INST_DATA(cl, obj);

      data->page = 0;
   }

   return((ULONG)obj);
}

///

/// Led Dispatcher
SAVEDS ULONG Led_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(Led_New(cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

