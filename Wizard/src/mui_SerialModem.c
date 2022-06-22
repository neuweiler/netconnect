/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/genesis_lib.h"
#include "mui.h"
#include "mui_SerialModem.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "images/setup_page2.h"
///
/// external variables
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct Config Config;
extern BOOL no_picture;

extern ULONG setup_page2_colors[];
extern UBYTE setup_page2_body[];
extern struct Hook strobjhook, sorthook, txtobjhook, deshook, objstrhook, objtxthook;

///

/// SerialModem_LoadData
ULONG SerialModem_LoadData(struct IClass *cl, Object *obj, Msg msg)
{
   struct SerialModem_Data *data = INST_DATA(cl, obj);
   struct pc_Data pc_data;

   // load devices into serial.dev list

   set(data->LV_SerialDevices, MUIA_List_Quiet, TRUE);
   {
      BPTR lock;
      struct FileInfoBlock *fib;

      if(lock = Lock("DEVS:", ACCESS_READ))
      {
         if(fib = AllocDosObject(DOS_FIB, NULL))
         {
            if(Examine(lock, fib))
            {
               while(ExNext(lock, fib))
               {
                  if((fib->fib_DirEntryType < 0) && strstr(fib->fib_FileName, ".device"))
                     DoMethod(data->LV_SerialDevices, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
               }
            }
            FreeDosObject(DOS_FIB, fib);
         }
         UnLock(lock);
      }
   }
   set(data->LV_SerialDevices, MUIA_List_Quiet, FALSE);

   // load the ModemSettings into the List
   if(ParseConfig("AmiTCP:db/modems", &pc_data))
   {
      set(data->LV_Modems, MUIA_List_Quiet, TRUE);
      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.Argument, "Modem"))
            DoMethod(data->LV_Modems, MUIM_List_InsertSingle, pc_data.Contents, MUIV_List_Insert_Sorted);
      }
      DoMethod(data->LV_Modems, MUIM_List_InsertSingle, "Generic", MUIV_List_Insert_Top);
      set(data->LV_Modems, MUIA_List_Quiet, FALSE);
      ParseEnd(&pc_data);
   }

   return(NULL);
}

///
/// SerialModem_New
ULONG SerialModem_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct SerialModem_Data tmp;

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
      Child, VGroup,   // serial options
         GroupFrame,
         MUIA_Background, MUII_TextBack,
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoSerialDevice)),
         Child, HGroup,
            Child, tmp.PO_SerialDevice = PopobjectObject,
               MUIA_Popstring_String      , tmp.STR_SerialDevice = MakeString(Config.cnf_serialdevice, MAXPATHLEN),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_ObjStrHook  , &objstrhook,
               MUIA_Popobject_Object      , tmp.LV_SerialDevices = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                     MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
                     MUIA_List_CompareHook   , &sorthook,
                     MUIA_List_AutoVisible   , TRUE,
                  End,
               End,
            End,
            Child, Label(GetStr(MSG_LA_Unit)),
            Child, tmp.SL_SerialUnit = NumericbuttonObject,
               MUIA_CycleChain      , 1,
               MUIA_Numeric_Min     , 0,
               MUIA_Numeric_Max     , 20,
               MUIA_Numeric_Value   , Config.cnf_serialunit,
            End,
         End,
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoModemName)),
         Child, tmp.PO_ModemName = PopobjectObject,
            MUIA_Popstring_String      , tmp.TX_ModemName = TextObject,
               TextFrame,
               MUIA_Background, MUII_TextBack,
               MUIA_Text_Contents, Config.cnf_modemname,
            End,
            MUIA_Popstring_Button      , PopButton(MUII_PopUp),
            MUIA_Popobject_StrObjHook  , &txtobjhook,
            MUIA_Popobject_ObjStrHook  , &objtxthook,
            MUIA_Popobject_Object      , tmp.LV_Modems = ListviewObject,
               MUIA_Listview_DoubleClick  , TRUE,
               MUIA_Listview_List         , ListObject,
                  InputListFrame,
                  MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                  MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
                  MUIA_List_CompareHook   , &sorthook,
                  MUIA_List_AutoVisible   , TRUE,
               End,
            End,
         End,
         Child, HVSpace,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct SerialModem_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

      *data = tmp;

      set(data->STR_SerialDevice , MUIA_ShortHelp, GetStr(MSG_HELP_SerialDevice));
      set(data->SL_SerialUnit    , MUIA_ShortHelp, GetStr(MSG_HELP_SerialUnit));
      set(data->TX_ModemName     , MUIA_ShortHelp, GetStr(MSG_HELP_Modem));

      DoMethod(data->LV_SerialDevices , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime, data->PO_SerialDevice, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->LV_Modems        , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime, data->PO_ModemName, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->STR_SerialDevice , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, mw_data->BT_Next);
   }

   return((ULONG)obj);
}

///
/// SerialModem_Dispatcher
SAVEDS ULONG SerialModem_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(SerialModem_New         (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_SerialModem_LoadData)
      return(SerialModem_LoadData    (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///
