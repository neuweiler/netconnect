#include "mui_Button.h"

///
/// external variables
extern struct MUI_CustomClass *CL_Button;
extern Object *app;

///

/// actio_button
VOID action_button(struct Icon *icon, struct AppMessage *msg)
{
   if(icon->Sound && icon->Volume)
      play_sound(icon->Sound, icon->Volume);

   if(icon->Program.PublicScreen && (icon->Program.Flags & PRG_ToFront))
   {
      struct Screen *scr;
      if(scr = LockPubScreen(icon->Program.PublicScreen))
      {
         ScreenToFront(scr);
         UnlockPubScreen(NULL, scr);
      }
   }

   icon->Program.Flags |= PRG_Asynch;
   StartProgram(&icon->Program, msg);
}

///
/// Button_AppMsgFunc
#ifdef __SASC
SAVEDS ASM LONG Button_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x) {
#else /* gcc */
LONG Button_AppMsgFunc()
{
   register APTR obj __asm("a2");
   register struct AppMEssage **x __asm("a1");
#endif

   struct Button_Data *data = INST_DATA(CL_Button->mcc_Class, obj);

   action_button(&data->icon, *x);
   return(NULL);
}

///
/// Button_Hotkey_Trigger
ULONG Button_Hotkey_Trigger(struct IClass *cl, Object *obj, Msg msg)
{
   struct Button_Data *data = INST_DATA(CL_Button->mcc_Class, obj);

   action_button(&data->icon, NULL);
   return(NULL);
}

///
/// Button_Dispose
ULONG Button_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
   struct Button_Data *data = INST_DATA(cl, obj);

   if(data->icon.Name)
      FreeVec(data->icon.Name);
   if(data->icon.Hotkey)
      FreeVec(data->icon.Hotkey);
   if(data->icon.ImageFile)
      FreeVec(data->icon.ImageFile);
   if(data->icon.Sound)
      FreeVec(data->icon.Sound);
   if(data->icon.Program.File)
      FreeVec(data->icon.Program.File);
   if(data->icon.Program.CurrentDir)
      FreeVec(data->icon.Program.CurrentDir);
   if(data->icon.Program.OutputFile)
      FreeVec(data->icon.Program.OutputFile);
   if(data->icon.Program.PublicScreen)
      FreeVec(data->icon.Program.PublicScreen);

   if(data->icon.body)
      FreeVec(data->icon.body);
   if(data->icon.cols)
      FreeVec(data->icon.cols);
   if(data->icon.disk_object)
      FreeDiskObject(data->icon.disk_object);

   if(data->CX_Filter && CxBase)
      DeleteCxObjAll(data->CX_Filter);

   return(DoSuperMethodA(cl, obj, msg));
}

///
/// Button_New
ULONG Button_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static const struct Hook Button_AppMsgHook = { {NULL, NULL}, (VOID *)Button_AppMsgFunc, NULL, NULL };
   struct Icon *src;

   if(src = (struct Icon *)GetTagData(MUIA_NetConnect_Icon, (ULONG)"", msg->ops_AttrList))
   {
      if(obj = (Object *)DoSuperNew(cl, obj,
         MUIA_Frame        , (src->Flags & IFL_DrawFrame ? MUIV_Frame_Button : MUIV_Frame_None),
         MUIA_Group_Spacing, 0,
         MUIA_InnerBottom  , 0,
         MUIA_InnerLeft    , 0,
         MUIA_InnerTop     , 0,
         MUIA_InnerRight   , 0,
         MUIA_CycleChain   , 1,
         MUIA_Background   , MUII_ButtonBack,
         MUIA_InputMode    , MUIV_InputMode_RelVerify,
         TAG_MORE, msg->ops_AttrList))
      {
         struct Button_Data *data = INST_DATA(cl, obj);

         memcpy(&data->icon, src, sizeof(struct Icon));

         if(data->icon.Name)
            set(obj, MUIA_ShortHelp, data->icon.Name);
         DoMethod(obj, MUIM_Notify, MUIA_Pressed      , FALSE           , obj, 1, MUIM_Hotkey_Trigger);
         DoMethod(obj, MUIM_Notify, MUIA_AppMessage   , MUIV_EveryTime  , obj, 3, MUIM_CallHook, &Button_AppMsgHook, MUIV_TriggerValue);

/*
if(data->icon.disk_object)
{
   struct Gadget *gad;

   if(gad = (struct Gadget *)xget(obj, MUIA_Gadget_Gadget))
      gad->SelectRender = data->icon.disk_object->do_Gadget.SelectRender;
}
*/

         data->CX_Filter = NULL;
         if(data->icon.Hotkey && CxBase && xget(app, MUIA_Application_Broker) && xget(app, MUIA_Application_BrokerPort))
         {
            if(data->CX_Filter = CxFilter(data->icon.Hotkey))
            {
               BOOL success = FALSE;
               CxObj *sender;
               CxObj *translator;

               AttachCxObj((CxObj *)xget(app, MUIA_Application_Broker), data->CX_Filter);
               if(sender = CxSender(xget(app, MUIA_Application_BrokerPort), obj))
               {
                  AttachCxObj(data->CX_Filter, sender);
                  if(translator = CxTranslate(NULL))
                  {
                     AttachCxObj(data->CX_Filter, translator);
                     if(!(CxObjError(data->CX_Filter)))
                        success = TRUE;
                  }
               }
               if(!success)
               {
                  DeleteCxObjAll(data->CX_Filter);
                  data->CX_Filter = NULL;
               }
            }
         }
      }
   }

   return((ULONG)obj);
}

///
/// Button_Dispatcher
#ifdef __SASC
SAVEDS ASM ULONG Button_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg) {
#else /* gcc */
ULONG Button_Dispatcher()
{
   register struct IClass *cl __asm("a0");
   register Object *obj __asm("a2");
   register Msg msg __asm("a1");
#endif
   
   switch (msg->MethodID)
   {
      case OM_NEW             : return(Button_New              (cl, obj, (APTR)msg));
      case OM_DISPOSE         : return(Button_Dispose          (cl, obj, (APTR)msg));
      case MUIM_Hotkey_Trigger: return(Button_Hotkey_Trigger   (cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}
///

