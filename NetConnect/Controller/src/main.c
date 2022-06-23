#include "main.h"

/// exit_libs
VOID exit_libs(VOID)
{
   if(cat)              CloseCatalog(cat);
   if(SoundObject)      DisposeDTObject(SoundObject);

   if(NetConnectBase)   CloseLibrary(NetConnectBase);
   if(DiskfontBase)     CloseLibrary(DiskfontBase);
   if(DataTypesBase)    CloseLibrary(DataTypesBase);
   if(IFFParseBase)     CloseLibrary(IFFParseBase);
   if(UtilityBase)      CloseLibrary(UtilityBase);
   if(MUIMasterBase)    CloseLibrary(MUIMasterBase);
   if(CxBase)           CloseLibrary(CxBase);
   if(WorkbenchBase)    CloseLibrary(WorkbenchBase);
   if(IconBase)         CloseLibrary(IconBase);

   cat         = NULL;
   SoundObject = NULL;
   IFFParseBase   = UtilityBase     = MUIMasterBase  = DataTypesBase   =
   CxBase         = WorkbenchBase   = IconBase       = DiskfontBase    =
   NetConnectBase = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
   if(LocaleBase)
      cat = OpenCatalog(NULL, "netconnect.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

   if(!(NetConnectBase = OpenLibrary("netconnect.library", 5)))
      Printf("could not open netconnect.library\n");

   MUIMasterBase  = OpenLibrary("muimaster.library"   , 11);
   UtilityBase    = OpenLibrary("utility.library"     , 0);
   IFFParseBase   = OpenLibrary("iffparse.library"    , 0);
   DataTypesBase  = OpenLibrary("datatypes.library"   , 0);
   CxBase         = OpenLibrary("commodities.library" , 0);
   WorkbenchBase  = OpenLibrary(WORKBENCH_NAME        , 0);
   IconBase       = OpenLibrary("icon.library"        , 0);
   DiskfontBase   = OpenLibrary("diskfont.library"    , 0);

   if(MUIMasterBase  && UtilityBase    && GfxBase  && IFFParseBase   &&
      WorkbenchBase  && DataTypesBase  && IconBase && NetConnectBase)
   {
//TODO      if(NCL_GetOwner())
      {
#ifdef DEMO
         CloseLibrary(NetConnectBase);
         if(NetConnectBase = (struct Library *)FindName(&SysBase->LibList, "netconnect.library"))
            RemLibrary(NetConnectBase);
         NetConnectBase = NULL;
#endif
         return(TRUE);
      }
      Printf("NetConnect registration failed.\n");
   }

   /* the program will still work without locale.library, commodity.library or diskfont.library */

   exit_libs();
   return(FALSE);
}

///
/// exit_classes
VOID exit_classes(VOID)
{
	if(CL_About)      MUI_DeleteCustomClass(CL_About);
   if(CL_Button)     MUI_DeleteCustomClass(CL_Button);
   if(CL_Dock)       MUI_DeleteCustomClass(CL_Dock);
   if(CL_MenuList)   MUI_DeleteCustomClass(CL_MenuList);

   CL_Dock  = CL_Button    =
   CL_About = CL_MenuList  = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
   CL_About    = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct About_Data)   , About_Dispatcher);
   CL_Button   = MUI_CreateCustomClass(NULL, MUIC_Group  , NULL, sizeof(struct Button_Data)  , Button_Dispatcher);
   CL_Dock     = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct Dock_Data)    , Dock_Dispatcher);
   CL_MenuList = MUI_CreateCustomClass(NULL, MUIC_List   , NULL, sizeof(struct MenuList_Data), MenuList_Dispatcher);

   if(CL_Dock && CL_Button && CL_About && CL_MenuList)
      return(TRUE);

   exit_classes();
   return(FALSE);
}

///
/// LocalizeNewMenu
VOID LocalizeNewMenu(struct NewMenu *nm)
{
   for (;nm->nm_Type!=NM_END;nm++)
      if (nm->nm_Label != NM_BARLABEL)
         nm->nm_Label = GetStr(nm->nm_Label);
}

///
/// check_date
#ifdef DEMO
#include <resources/battclock.h>
#include <clib/battclock_protos.h>
ULONG check_date(VOID)
{
   struct FileInfoBlock *fib;
   ULONG days_running = 0;
   BOOL set_comment = FALSE;
   char file[50];
   BPTR lock;
   ULONG clock = NULL;

   strcpy(file, "libs:locale.library");

   if(BattClockBase = OpenResource("battclock.resource"))
   {
      clock = ReadBattClock();

      if(fib = AllocDosObject(DOS_FIB, NULL))
      {
         if(lock = Lock(file, ACCESS_READ))
         {
            Examine(lock, fib);
            UnLock(lock);

            if(strlen(fib->fib_Comment) > 4)
            {
               ULONG inst;

               inst = atol((STRPTR)(fib->fib_Comment + 2));
               if(inst > 8000000)   // did we put something in there once ?
               {
                  if((fib->fib_Comment[0] == '0') && (fib->fib_Comment[1] == '2'))
                     days_running = (clock - inst)/86400;
                  else
                     set_comment = TRUE;
               }
               else
                  set_comment = TRUE;
            }
            else
               set_comment = TRUE;
         }
         FreeDosObject(DOS_FIB, fib);
      }

      if(set_comment)
      {
         char buffer[21];

         sprintf(buffer, "02%ld", clock);
         SetComment(file, buffer);
      }
   }

   return(days_running);
}
#endif

///

/// BuildApp
BOOL BuildApp(VOID)
{
   BOOL success = FALSE;

   if(app = ApplicationObject,
      MUIA_Application_SingleTask   , TRUE,
      MUIA_Application_Author       , "Michael Neuweiler",
      MUIA_Application_Base         , "NetConnect",
      MUIA_Application_Title        , "NetConnect",
#ifdef DEMO
      MUIA_Application_Version      , "$VER:NetConnect "VERTAG" (DEMO)",
#else
      MUIA_Application_Version      , "$VER:NetConnect "VERTAG,
#endif
      MUIA_Application_Copyright    , GetStr(MSG_AppCopyright),
      MUIA_Application_Description  , GetStr(MSG_ControllerAppDescription),
      MUIA_Application_BrokerHook   , &BrokerHook,
      MUIA_Application_Commands     , arexx_list,
      MUIA_Application_Window       , WindowObject,
         WindowContents    , group = VGroup,
            Child, HVSpace,
         End,
      End,
      End)
   {
      if(menu_list = (Object *)NewObject(CL_MenuList->mcc_Class, NULL, TAG_DONE))
      {
         DoMethod(group, OM_ADDMEMBER, menu_list);

         ihnode.ihn_Object    = menu_list;
         ihnode.ihn_Signals   = (LONG)(1L << appmenu_port->mp_SigBit);
         ihnode.ihn_Method    = MUIM_MenuList_TriggerMenu;
         ihnode.ihn_Flags     = 0;
         DoMethod(app, MUIM_Application_AddInputHandler, &ihnode);

         if(load_config())
            success = TRUE;

         DoMethod(app, MUIM_Notify, MUIA_Application_DoubleStart, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_DOUBLESTART);
      }
   }
   return(success);
}

///
/// rem_inputhandler
VOID rem_inputhandler(VOID)
{
   struct Message *message;

   DoMethod(app, MUIM_Application_RemInputHandler, &ihnode);
   while(message = GetMsg(appmenu_port))
      ReplyMsg(message);
}

///
/// refresh
BOOL refresh(VOID)
{
   BOOL success = FALSE;

   rem_inputhandler();
   MUI_DisposeObject(app);
   app = NULL;
   if(BuildApp())
      success = TRUE;

   return(success);
}

///
/// Handler
VOID Handler(VOID)
{
   ULONG sigs = NULL;
   BOOL running;
   Object *req = NULL;

   if(init_libs())
   {
      if((NotifySignal = AllocSignal(-1)) != -1)
      {
         nr.nr_stuff.nr_Signal.nr_Task = (struct Task *)proc;
         nr.nr_stuff.nr_Signal.nr_SignalNum = NotifySignal;
         StartNotify(&nr);
      }
      if(init_classes())
      {
         if(appmenu_port = CreateMsgPort())
         {
            LocalizeNewMenu(DockMenu);

            if(BuildApp())
            {
               running = TRUE;
#ifdef DEMO
               {
                  ULONG days_running;

                  days_running = check_date();

                  if(days_running > MAX_DAYS)
                     MUI_Request(app, NULL, NULL, NULL, GetStr(MSG_BT_Okay), "Sorry, this demo version has timed out !");
                  else
#endif
               while(running)
               {
                  switch(DoMethod(app, MUIM_Application_NewInput, &sigs))
                  {
                     case MUIV_Application_ReturnID_Quit:
                        running = FALSE;
                        break;
                     case ID_REFRESH:
                        running = refresh();
                        break;

                     case ID_CHECK_OPEN:
                     {
                        APTR object_state;
                        Object *window_ptr;
                        struct List *list;

                        if(list = (struct List *)xget(app, MUIA_Application_WindowList))
                        {
                           object_state = list->lh_Head;
                           while(window_ptr = NextObject(&object_state))
                           {
                              if(xget(window_ptr, MUIA_Window_Open))
                                 break;
                           }
                           if(!window_ptr)
                              running = !MUI_Request(app, NULL, NULL, NULL, GetStr(MSG_BT_Want_Quit), GetStr(MSG_TX_Want_Quit));
                        }
                     }
                     break;

                     case ID_ABOUT:
                        if(!req)
                        {
                           set(app, MUIA_Application_Sleep, TRUE);
                           if(req = (APTR)NewObject(CL_About->mcc_Class, NULL, TAG_DONE))
                           {
                              DoMethod(app, OM_ADDMEMBER, req);
                              set(req, MUIA_Window_Open, TRUE);
                           }
                           else
                              set(app, MUIA_Application_Sleep, FALSE);
                        }
                        break;
                     case  ID_ABOUTFINISH:
                        if(req)
                        {
                           set(req, MUIA_Window_Open, FALSE);
                           DoMethod(app, OM_REMMEMBER, req);
                           MUI_DisposeObject(req);
                           set(app, MUIA_Application_Sleep, FALSE);
                           req = NULL;
                        }
                        break;
                     case ID_DOUBLESTART:
                        MUI_Request(app, NULL, NULL, NULL, GetStr(MSG_BT_Okay), GetStr(MSG_TX_Already_Running));
                        break;
                  }
                  if(sigs)
                  {
                     sigs = Wait(sigs | SIGBREAKF_CTRL_C | (1L << NotifySignal));
                     if(sigs & SIGBREAKF_CTRL_C)
                        running = FALSE;
                     if(sigs & (1L << NotifySignal))
                        running = refresh();
                  }
               }
#ifdef DEMO
               }
#endif
            }
            if(app)
            {
               rem_inputhandler();
               MUI_DisposeObject(app);
               app = NULL;
            }
            DeleteMsgPort(appmenu_port);
         }
         exit_classes();
      }
      if(NotifySignal != -1)
      {
         EndNotify(&nr);
         FreeSignal(NotifySignal);
      }
      exit_libs();
   }
}

///

/// main
int main(int argc, char *argv[])
{
   proc = (struct Process *)FindTask(NULL);

   if(!proc->pr_CLI)
   {
      if(LocalCLI = CloneCLI(&_WBenchMsg->sm_Message))
      {
         OldCLI = proc->pr_CLI;
         proc->pr_CLI = MKBADDR(LocalCLI);
      }
   }

   if(((ULONG)proc->pr_Task.tc_SPUpper - (ULONG)proc->pr_Task.tc_SPLower) < NEWSTACK_SIZE)
   {
      if(!(StackSwapper.stk_Lower = AllocVec(NEWSTACK_SIZE, MEMF_ANY)))
         exit(20);
      StackSwapper.stk_Upper   = (ULONG)StackSwapper.stk_Lower + NEWSTACK_SIZE;
      StackSwapper.stk_Pointer = (APTR)StackSwapper.stk_Upper;
      StackSwap(&StackSwapper);

      Handler();

      StackSwap(&StackSwapper);
      FreeVec(StackSwapper.stk_Lower);
   }
   else
      Handler();

   if(LocalCLI)
   {
      proc->pr_CLI = OldCLI;
      DeleteCLI(LocalCLI);
   }
}

///

