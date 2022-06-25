#include "mui_Dock.h"

///
/// external variables
extern struct NewMenu DockMenu[];
extern Object *app;

///

/// Dock_DockPrefs
ULONG Dock_DockPrefs(struct IClass *cl, Object *obj, Msg msg)
{
   struct Program program;

   bzero(&program, sizeof(struct Program));
   program.File   = "NetConnect2:NetConnectPrefs";
   program.Flags  = PRG_Asynch | PRG_CLI;
   program.Stack  = 12288;

   StartProgram(&program, NULL);
   return(NULL);
}

///
/// Dock_GenesisPrefs
ULONG Dock_GenesisPrefs(struct IClass *cl, Object *obj, Msg msg)
{
   struct Program program;

   bzero(&program, sizeof(struct Program));
   program.File   = "AmiTCP:GenesisPrefs";
   program.Flags  = PRG_Asynch | PRG_CLI;
   program.Stack  = 9216;

   StartProgram(&program, NULL);
   return(NULL);
}

///
/// Dock_Help
ULONG Dock_Help(struct IClass *cl, Object *obj, Msg msg)
{
   struct Program program;

   bzero(&program, sizeof(struct Program));
   program.File   = "NetConnect2:Docs/Documentation";
   program.Flags  = PRG_Asynch | PRG_SCRIPT;
   program.Stack  = 9216;

   StartProgram(&program, NULL);
   return(NULL);
}

///
/// Dock_Hotkey_Trigger
ULONG Dock_Hotkey_Trigger(struct IClass *cl, Object *obj, Msg msg)
{
   set(obj, MUIA_Window_Open, !(xget(obj, MUIA_Window_Open)));
   return(NULL);
}

///
/// Dock_Dispose
ULONG Dock_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
   struct Dock_Data *data = INST_DATA(cl,obj);

   if(data->TxtFont && DiskfontBase)
      CloseFont(data->TxtFont);
   if(data->CX_Filter && CxBase)
      DeleteCxObjAll(data->CX_Filter);

   if(data->dock.Name)
      FreeVec(data->dock.Name);
   if(data->dock.Hotkey)
      FreeVec(data->dock.Hotkey);
   if(data->dock.Font)
      FreeVec(data->dock.Font);

   return(DoSuperMethodA(cl, obj, msg));
}

///
/// Dock_New
ULONG Dock_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Dock *src;
   Object *MN_Strip = NULL;

   if(src = (struct Dock *)GetTagData(MUIA_NetConnect_Dock, (ULONG)"", msg->ops_AttrList))
   {
      if(obj = (Object *)DoSuperNew(cl, obj,
         MUIA_Window_AppWindow   , TRUE,
         MUIA_Window_ID          , src->WindowID,
         MUIA_Window_Menustrip   , MN_Strip = MUI_MakeObject(MUIO_MenustripNM, DockMenu, 0),
         MUIA_Window_Borderless  , src->Flags & DFL_Borderless,
         MUIA_Window_DepthGadget , !(src->Flags & DFL_Borderless),
         MUIA_Window_DragBar     , src->Flags & DFL_DragBar || !(src->Flags & DFL_Borderless),
         MUIA_Window_CloseGadget , !(src->Flags & DFL_Borderless),
         MUIA_Window_SizeGadget  , !(src->Flags & DFL_Borderless),
         MUIA_Window_Activate    , src->Flags & DFL_Activate,
         MUIA_Window_Backdrop    , src->Flags & DFL_Backdrop,
         MUIA_Window_Title       , (src->Flags & DFL_Borderless ? NULL : src->Name),
         TAG_MORE, msg->ops_AttrList))
      {
         struct Dock_Data *data = INST_DATA(cl,obj);

         memcpy(&data->dock, src, sizeof(struct Dock));
         data->MN_Strip = MN_Strip;

         data->CX_Filter = NULL;
         if(data->dock.Hotkey && CxBase && xget(app, MUIA_Application_Broker) && xget(app, MUIA_Application_BrokerPort))
         {
            if(data->CX_Filter = CxFilter(data->dock.Hotkey))
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
         data->TxtFont = NULL;
         if(data->dock.Font && DiskfontBase)
         {
            STRPTR ptr;

            if(ptr = AllocVec(strlen(data->dock.Font) + 10, MEMF_ANY))
            {
               strcpy(ptr, data->dock.Font);
               data->TxtAttr.ta_Name = ptr;

               if(ptr = strchr(data->TxtAttr.ta_Name, '/'))
               {
                  data->TxtAttr.ta_YSize = atol(ptr + 1);
                  strcpy(ptr, ".font");
                  data->TxtAttr.ta_Style = FS_NORMAL;
                  data->TxtAttr.ta_Flags = NULL;
                  data->TxtFont = OpenDiskFont(&data->TxtAttr);
               }
            }
         }

         DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
            obj, 3, MUIM_Set, MUIA_Window_Open, FALSE);
         DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
            MUIV_Notify_Application, 5, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, ID_CHECK_OPEN);

         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
            MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_ABOUT);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
            MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_CONTROLLER), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
            obj, 1, MUIM_Dock_DockPrefs);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_GENESIS) , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
            obj, 1, MUIM_Dock_GenesisPrefs);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_HELP) , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
            obj, 1, MUIM_Dock_Help);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
            MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
      }
   }

   return((ULONG)obj);
}

///
/// Dock_Dispatcher
#ifdef __SASC
SAVEDS ASM ULONG Dock_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg) {
#else /* gcc */
ULONG Dock_Dispatcher()
{
   register struct IClass *cl __asm("a0");
   register Object *obj __asm("a2");
   register Msg msg __asm("a1");
#endif

   switch (msg->MethodID)
   {
      case OM_NEW                : return(Dock_New             (cl, obj, (APTR)msg));
      case OM_DISPOSE            : return(Dock_Dispose         (cl, obj, (APTR)msg));
      case MUIM_Hotkey_Trigger   : return(Dock_Hotkey_Trigger  (cl, obj, (APTR)msg));
      case MUIM_Dock_DockPrefs   : return(Dock_DockPrefs       (cl, obj, (APTR)msg));
      case MUIM_Dock_GenesisPrefs: return(Dock_GenesisPrefs    (cl, obj, (APTR)msg));
      case MUIM_Dock_Help        : return(Dock_Help            (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}
///

