/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Modems.h"
#include "mui_ModemWindow.h"
#include "protos.h"

///
/// external variables
extern struct Hook des_hook;
extern Object *app;
extern struct MUI_CustomClass *CL_ModemWindow;

///

/// Modems_NewModem
ULONG Modems_NewModem(struct IClass *cl, Object *obj, struct MUIP_Modems_NewModem *msg)
{
   struct Modems_Data *data = INST_DATA(cl, obj);
   struct Modem *modem;

   if(msg->copy)
      DoMethod(data->LI_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
   else
      modem = (APTR)-1;

   DoMethod(data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Bottom);
   set(data->LI_Modems, MUIA_List_Active, MUIV_List_Active_Bottom);

   DoMethod(data->LI_Modems , MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
   if(modem)
   {
      modem->mo_id = 1;
      uniquify_modem_id(data->LI_Modems, modem);
   }

   DoMethod(obj, MUIM_Modems_Edit);

   return(NULL);
}

///
/// Modems_SetStates
ULONG Modems_SetStates(struct IClass *cl, Object *obj, Msg msg)
{
   struct Modems_Data *data = INST_DATA(cl, obj);
   struct Modem *modem;

   DoMethod(data->LI_Modems , MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
   DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, !modem, data->BT_Copy,
      data->BT_Edit, data->BT_Delete, NULL);

   return(NULL);
}

///
/// Modems_Edit
ULONG Modems_Edit(struct IClass *cl, Object *obj, Msg msg)
{
   struct Modems_Data *data = INST_DATA(cl, obj);
   struct Modem *modem;

   DoMethod(data->LI_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
   if(modem)
   {
      Object *window;

      set(app, MUIA_Application_Sleep, TRUE);
      if(window = NewObject(CL_ModemWindow->mcc_Class, NULL, MUIA_Genesis_Originator, obj, TAG_DONE))
      {
         DoMethod(app, OM_ADDMEMBER, window);
         DoMethod(window, MUIM_ModemWindow_Init, modem);
         set(window, MUIA_Window_Title, modem->mo_name);
         set(window, MUIA_Window_Open, TRUE);
      }
   }
   else
      DisplayBeep(NULL);

   return(NULL);
}

///
/// Modems_EditFinish
ULONG Modems_EditFinish(struct IClass *cl, Object *obj, struct MUIP_Modems_EditFinish *msg)
{
   struct Modems_Data *data = INST_DATA(cl, obj);

   set(msg->win, MUIA_Window_Open, FALSE);
   if(msg->ok)
      DoMethod(msg->win, MUIM_ModemWindow_CopyData);
   DoMethod(app, OM_REMMEMBER, msg->win);
   MUI_DisposeObject(msg->win);
   set(app, MUIA_Application_Sleep, FALSE);
   DoMethod(data->LI_Modems, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);

   return(NULL);
}

///

/// ModemList_ConstructFunc
SAVEDS ASM struct Modem *ModemList_ConstructFunc(register __a2 APTR pool, register __a1 struct Modem *src)
{
   struct Modem *new;

   if(new = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY | MEMF_CLEAR))
   {
      if(src && (src != (APTR)-1))
         memcpy(new, src, sizeof(struct Modem));
      else
      {
         strcpy(new->mo_name        , GetStr(MSG_TX_Generic));
         strcpy(new->mo_comment     , GetStr(MSG_TX_NewModem));
         strcpy(new->mo_device      , "serial.device");
         new->mo_flags     = MFL_7Wire | MFL_RadBoogie | MFL_DropDTR;
         new->mo_baudrate  = 38400;
         new->mo_serbuflen = 16384;
         strcpy(new->mo_init        , "AT&F&D2\\r");
         strcpy(new->mo_dialprefix  , "ATDT");
         strcpy(new->mo_dialsuffix  , "\\r");
         strcpy(new->mo_answer      , "ATA\\r");
         strcpy(new->mo_hangup      , "~~~+++~~~ATH0\\r");
         strcpy(new->mo_ring        , "RING");
         strcpy(new->mo_connect     , "CONNECT");
         strcpy(new->mo_nocarrier   , "NO CARRIER");
         strcpy(new->mo_nodialtone  , "NO DIALTONE");
         strcpy(new->mo_busy        , "BUSY");
         strcpy(new->mo_ok          , "OK");
         strcpy(new->mo_error       , "ERROR");
         new->mo_redialattempts  = 15;
         new->mo_redialdelay     = 5;
         new->mo_commanddelay    = 10;
      }
   }
   return(new);
}
struct Hook ModemList_ConstructHook= { { 0,0 }, (VOID *)ModemList_ConstructFunc , NULL, NULL };

///
/// ModemList_DisplayFunc
SAVEDS ASM LONG ModemList_DisplayFunc(register __a2 char **array, register __a1 struct Modem *modem)
{
   if(modem)
   {
      static char buf1[5], buf2[10];

      sprintf(buf1, "%ld", modem->mo_unit);
      sprintf(buf2, "%ld", modem->mo_baudrate);
      *array++ = modem->mo_name;
      *array++ = modem->mo_comment;
      *array++ = modem->mo_device;
      *array++ = buf1;
      *array   = buf2;
   }
   else
   {
      *array++ = GetStr(MSG_TX_Name);
      *array++ = GetStr(MSG_TX_Comment);
      *array++ = GetStr(MSG_TX_Device);
      *array++ = GetStr(MSG_TX_Unit);
      *array   = GetStr(MSG_TX_BaudRate);
   }
   return(NULL);
}
struct Hook ModemList_DisplayHook= { { 0,0 }, (VOID *)ModemList_DisplayFunc , NULL, NULL };

///

/// Modems_New
ULONG Modems_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Modems_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      Child, VGroup,
         GroupSpacing(0),
         Child, tmp.LV_Modems = NListviewObject,
            MUIA_CycleChain      , 1,
            MUIA_NListview_NList , tmp.LI_Modems = NListObject,
               InputListFrame,
               MUIA_NList_DragType     , MUIV_NList_DragType_Default,
               MUIA_NList_DragSortable , TRUE,
               MUIA_NList_DoubleClick  , TRUE,
               MUIA_NList_ConstructHook, &ModemList_ConstructHook,
               MUIA_NList_DisplayHook  , &ModemList_DisplayHook,
               MUIA_NList_DestructHook , &des_hook,
               MUIA_NList_Format       , "BAR,BAR,BAR,BAR,",
               MUIA_NList_Title        , TRUE,
            End,
         End,
         Child, HGroup,
            GroupSpacing(0),
            Child, tmp.BT_New    = MakeButton(MSG_BT_New),
            Child, tmp.BT_Copy    = MakeButton(MSG_BT_Copy),
            Child, tmp.BT_Delete = MakeButton(MSG_BT_Delete),
            Child, tmp.BT_Edit   = MakeButton(MSG_BT_Edit),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Modems_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->BT_Edit       , MUIA_Disabled, TRUE);
      set(data->BT_Delete     , MUIA_Disabled, TRUE);
      set(data->BT_Copy       , MUIA_Disabled, TRUE);

      set(data->LV_Modems     , MUIA_ShortHelp, GetStr(MSG_Help_ModemList));
      set(data->BT_New        , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_Copy       , MUIA_ShortHelp, GetStr(MSG_Help_Copy));
      set(data->BT_Delete     , MUIA_ShortHelp, GetStr(MSG_Help_Delete));
      set(data->BT_Edit       , MUIA_ShortHelp, GetStr(MSG_Help_Edit));

      DoMethod(data->BT_New      , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_Modems_NewModem, FALSE);
      DoMethod(data->BT_Copy     , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_Modems_NewModem, TRUE);
      DoMethod(data->BT_Edit     , MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_Modems_Edit);
      DoMethod(data->BT_Delete   , MUIM_Notify, MUIA_Pressed, FALSE, data->LI_Modems, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->LI_Modems   , MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_Modems_SetStates);
      DoMethod(data->LI_Modems   , MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 1, MUIM_Modems_Edit);
   }
   return((ULONG)obj);
}

///
/// Modems_Dispatcher
SAVEDS ASM ULONG Modems_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                  : return(Modems_New          (cl, obj, (APTR)msg));
      case MUIM_Modems_Edit        : return(Modems_Edit         (cl, obj, (APTR)msg));
      case MUIM_Modems_EditFinish  : return(Modems_EditFinish   (cl, obj, (APTR)msg));
      case MUIM_Modems_NewModem    : return(Modems_NewModem     (cl, obj, (APTR)msg));
      case MUIM_Modems_SetStates   : return(Modems_SetStates    (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

