/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Dialer.h"
#include "protos.h"

///
/// external variables
extern struct Hook des_hook;
extern struct Hook strobjhook;
extern struct Hook objstrhook;
extern Object *win;
extern Object *app;

///

/// Dialer_New
ULONG Dialer_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Dialer_Data tmp;
   static STRPTR ARR_CY_Window[] = { "-", "open", "close", "iconify", NULL };
   static STRPTR ARR_Dialer_Register[] = { "Misc", "Display", "Execute", NULL };
   static STRPTR ARR_LaunchMode[] = { "CLI", "WB", "Script", "AREXX", NULL };

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Register_Titles, ARR_Dialer_Register,
      Child, VGroup,
         Child, HVSpace,
         Child, ColGroup(7),
            Child, HVSpace,
            Child, tmp.CH_QuickReconnect  = MakeKeyCheckMark(FALSE, "  q"),
            Child, KeyLLabel1(GetStr("  Quick reconnect"), *GetStr("  q")),
            Child, HVSpace,
            Child, tmp.CH_ConfirmOffline  = MakeKeyCheckMark(FALSE, "  o"),
            Child, KeyLLabel1(GetStr("  Confirm Disconnection"), *GetStr("  o")),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_StartupOpenWin  = MakeKeyCheckMark(TRUE, "  w"),
            Child, KeyLLabel1(GetStr("  Open main window"), *GetStr("  w")),
            Child, HVSpace,
            Child, tmp.CH_StartupIconify  = MakeKeyCheckMark(FALSE, "  i"),
            Child, KeyLLabel1(GetStr("  Iconified at startup"), *GetStr("  i")),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_ShowStatusWin   = MakeKeyCheckMark(TRUE, "  w"),
            Child, KeyLLabel1(GetStr("  Status windows"), *GetStr("  w")),
            Child, HVSpace,
            Child, tmp.CH_Debug           = MakeKeyCheckMark(FALSE, "  d"),
            Child, KeyLLabel1(GetStr("  Debug mode"), *GetStr("  d")),
            Child, HVSpace,
         End,
         Child, HVSpace,
      End,

      Child, VGroup,
         Child, HVSpace,
         Child, ColGroup(7),
            Child, HVSpace,
            Child, tmp.CH_ShowLog         = MakeKeyCheckMark(TRUE, "  l"),
            Child, KeyLLabel1(GetStr("  Show log"), *GetStr("  l")),
            Child, HVSpace,
            Child, tmp.CH_ShowLamps       = MakeKeyCheckMark(TRUE, "  e"),
            Child, KeyLLabel1(GetStr("  Show interface leds"), *GetStr("  e")),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_ShowConnect     = MakeKeyCheckMark(TRUE, "  p"),
            Child, KeyLLabel1(GetStr("  Show speed"), *GetStr("  p")),
            Child, HVSpace,
            Child, tmp.CH_ShowOnlineTime  = MakeKeyCheckMark(TRUE, "  o"),
            Child, KeyLLabel1(GetStr("  Show online time"), *GetStr("  o")),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_ShowButtons     = MakeKeyCheckMark(TRUE, "  b"),
            Child, KeyLLabel1(GetStr("  Show buttons"), *GetStr("  b")),
            Child, HVSpace,
            Child, tmp.CH_ShowNetwork     = MakeKeyCheckMark(TRUE, "  n"),
            Child, KeyLLabel1(GetStr("  Show network"), *GetStr("  n")),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_ShowUser        = MakeKeyCheckMark(TRUE, "  u"),
            Child, KeyLLabel1(GetStr("  Show user"), *GetStr("  u")),
            Child, HVSpace,
            Child, tmp.CH_ShowSerialInput = MakeKeyCheckMark(TRUE, "  r"),
            Child, KeyLLabel1(GetStr("  Show serial input"), *GetStr("  r")),
            Child, HVSpace,
         End,
         Child, HVSpace,
      End,

      Child, VGroup,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Startup"),
         Child, HGroup,
            Child, tmp.PA_Startup = MakePopAsl(tmp.STR_Startup = MakeKeyString(NULL, MAXPATHLEN, "  t"), "  Choose file", FALSE),
            Child, tmp.CY_Startup = Cycle(ARR_LaunchMode),
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Shutdown"),
         Child, HGroup,
            Child, tmp.PA_Shutdown = MakePopAsl(tmp.STR_Shutdown = MakeKeyString(NULL, MAXPATHLEN, "  t"), "  Choose file", FALSE),
            Child, tmp.CY_Shutdown = Cycle(ARR_LaunchMode),
         End,
         Child, HVSpace,
      End,
   TAG_MORE, msg->ops_AttrList))
   {
      struct Dialer_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(data->CY_Startup , MUIA_Weight, 0);
      set(data->CY_Shutdown, MUIA_Weight, 0);
set(data->CH_ConfirmOffline, MUIA_Disabled, TRUE);
set(data->CH_QuickReconnect, MUIA_Disabled, TRUE);
   }
   return((ULONG)obj);
}

///
/// Dialer_Dispatcher
SAVEDS ULONG Dialer_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(Dialer_New               (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

