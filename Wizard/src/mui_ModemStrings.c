/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_ModemStrings.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "images/setup_page2.h"

///
/// external variables
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct config Config;
extern BOOL no_picture;

extern ULONG setup_page2_colors[];
extern UBYTE setup_page2_body[];
extern struct Hook strobjhook, sorthook, txtobjhook, deshook, objstrhook;

///

/// ProtocolList_ConstructFunc
struct ModemProtocol * SAVEDS ProtocolList_ConstructFunc(register __a2 APTR pool, register __a1 struct ModemProtocol *src)
{
   struct ModemProtocol *new;

   if((new = (struct ModemProtocol *)AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR)) && src)
      memcpy(new, src, sizeof(struct ModemProtocol));
   return(new);
}

///
/// ProtocolList_DisplayFunc
SAVEDS LONG ProtocolList_DisplayFunc(register __a2 char **array, register __a1 struct ModemProtocol *modem_protocol)
{
   if(modem_protocol)
   {
      *array++ = modem_protocol->Name;
      *array   = modem_protocol->InitString;
   }
   else
   {
      *array++ = GetStr(MSG_BLA_Protocol);
      *array   = GetStr(MSG_BLA_InitString2);
   }
   return(NULL);
}

///
/// initstring_objstrfunc
VOID SAVEDS initstring_objstrfunc(register __a2 Object *list,register __a1 Object *str)
{
   struct ModemProtocol *modem_protocol;

   DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem_protocol);
   if(modem_protocol)
      set(str, MUIA_String_Contents, modem_protocol->InitString);
}

///

/// ModemStrings_LoadData
ULONG ModemStrings_LoadData(struct IClass *cl, Object *obj, Msg msg)
{
   struct ModemStrings_Data *data = INST_DATA(cl, obj);
   struct pc_Data pc_data;
   struct ModemProtocol *modem_protocol;
   int counter = 0;

   // load the Modem InitStrings into the List
   if(ParseConfig("AmiTCP:db/modems", &pc_data))
   {
      set(data->LV_Protocols, MUIA_List_Quiet, TRUE);

      if(modem_protocol = AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR))
      {
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.Argument, "Modem"))
            {
               if(!strcmp(pc_data.Contents, Config.cnf_modemname))
                  break;
            }
         }
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.Argument, "Modem"))
               break;

            if(!stricmp(pc_data.Argument, "Protocol"))
               strncpy(modem_protocol->Name, pc_data.Contents, 40);

            if(!stricmp(pc_data.Argument, "InitString"))
            {
               if(!counter++)
                  set(data->STR_InitString, MUIA_String_Contents, pc_data.Contents);

               strncpy(modem_protocol->InitString, pc_data.Contents, 40);
               if(!*modem_protocol->Name)
                  strcpy(modem_protocol->Name, "Default");
               DoMethod(data->LV_Protocols, MUIM_List_InsertSingle, modem_protocol, MUIV_List_Insert_Bottom);
               modem_protocol->Name[0] = NULL;
            }
         }
         FreeVec(modem_protocol);
      }
      set(data->LV_Protocols, MUIA_List_Quiet, FALSE);
      ParseEnd(&pc_data);
   }

   return(NULL);
}

///

/// ModemStrings_New
ULONG ModemStrings_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct ModemStrings_Data tmp;
   static const struct Hook ProtocolList_ConstructHook= { { 0,0 }, (VOID *)ProtocolList_ConstructFunc , NULL, NULL };
   static const struct Hook ProtocolList_DisplayHook= { { 0,0 }, (VOID *)ProtocolList_DisplayFunc , NULL, NULL };
   static const struct Hook initstring_objstrhook= { { 0,0 }, (VOID *)initstring_objstrfunc , NULL, NULL };
   static STRPTR ARR_DialPrefix[] = { "ATDT", "ATDP", "ATD0w", "ATD0,", NULL };

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Group_Horiz, TRUE,
      Child, tmp.GR_Picture = VGroup,
         MUIA_ShowMe, !no_picture,
         Child, BodychunkObject,
            GroupFrame,
            InnerSpacing(0, 0),
            MUIA_FixWidth             , SETUP_PAGE2_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE2_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE2_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE2_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE2_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page2_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE2_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE2_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page2_colors,
         End,
         Child, HVSpace,
      End,
      Child, VGroup,   // modem strings
         GroupFrame,
         MUIA_Background, MUII_TextBack,
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoInitString)),
         Child, tmp.PO_InitString = PopobjectObject,
            MUIA_Popstring_String      , tmp.STR_InitString = MakeString(Config.cnf_initstring, 80),
            MUIA_Popstring_Button      , PopButton(MUII_PopUp),
            MUIA_Popobject_StrObjHook  , &strobjhook,
            MUIA_Popobject_ObjStrHook  , &initstring_objstrhook,
            MUIA_Popobject_Object      , tmp.LV_Protocols = ListviewObject,
               MUIA_Listview_DoubleClick  , TRUE,
               MUIA_Listview_List         , ListObject,
                  MUIA_Frame              , MUIV_Frame_InputList,
                  MUIA_List_ConstructHook , &ProtocolList_ConstructHook,
                  MUIA_List_DestructHook  , &deshook,
                  MUIA_List_DisplayHook   , &ProtocolList_DisplayHook,
                  MUIA_List_CompareHook   , &sorthook,
                  MUIA_List_Format        , "BAR,",
                  MUIA_List_AutoVisible   , TRUE,
                  MUIA_List_Title         , TRUE,
               End,
            End,
         End,
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoDialPrefix)),
         Child, tmp.PO_DialPrefix = PopobjectObject,
            MUIA_Popstring_String      , tmp.STR_DialPrefix = MakeString(Config.cnf_dialprefix, 80),
            MUIA_Popstring_Button      , PopButton(MUII_PopUp),
            MUIA_Popobject_StrObjHook  , &strobjhook,
            MUIA_Popobject_ObjStrHook  , &objstrhook,
            MUIA_Popobject_Object      , tmp.LV_DialPrefix = ListviewObject,
               MUIA_Listview_DoubleClick  , TRUE,
               MUIA_Listview_List         , ListObject,
                  MUIA_Frame              , MUIV_Frame_InputList,
                  MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                  MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
                  MUIA_List_SourceArray   , ARR_DialPrefix,
               End,
            End,
         End,
         Child, HVSpace,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct ModemStrings_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

      *data = tmp;

      set(data->STR_InitString   , MUIA_ShortHelp, GetStr(MSG_HELP_InitString));
      set(data->STR_DialPrefix   , MUIA_ShortHelp, GetStr(MSG_HELP_DialPrefix));

      DoMethod(data->LV_Protocols        , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime, data->PO_InitString, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->LV_DialPrefix       , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime, data->PO_DialPrefix, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->STR_InitString      , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_DialPrefix);
      DoMethod(data->STR_DialPrefix      , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, mw_data->BT_Next);
   }

   return((ULONG)obj);
}

///
/// ModemString_Dispatcher
SAVEDS ULONG ModemStrings_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(ModemStrings_New         (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_ModemStrings_LoadData)
      return(ModemStrings_LoadData    (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

