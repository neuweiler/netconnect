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

   if(obj = tmp.GR_Misc = (Object *)DoSuperNew(cl, obj,
#ifndef DO_LISTTREE
      InnerSpacing(0,0),
#endif
      MUIA_Group_Columns, 3,
      Child, HVSpace,
      Child, VGroup,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Connection"),
         Child, ColGroup(5),
            Child, tmp.CH_QuickReconnect  = MakeKeyCheckMark(FALSE, "  q"),
            Child, KeyLLabel1(GetStr("  Quick reconnect"), *GetStr("  q")),
            Child, HVSpace,
            Child, tmp.CH_ConfirmOffline  = MakeKeyCheckMark(FALSE, "  o"),
            Child, KeyLLabel1(GetStr("  Confirm Disconnection"), *GetStr("  o")),
            Child, tmp.CH_StartupOpenWin  = MakeKeyCheckMark(FALSE, "  w"),
            Child, KeyLLabel1(GetStr("  Open main window at startup"), *GetStr("  w")),
            Child, HVSpace,
            Child, tmp.CH_StartupIconify  = MakeKeyCheckMark(FALSE, "  i"),
            Child, KeyLLabel1(GetStr("  Iconified at startup"), *GetStr("  i")),
            Child, tmp.CH_Debug           = MakeKeyCheckMark(FALSE, "  d"),
            Child, KeyLLabel1(GetStr("  Debug mode"), *GetStr("  d")),
            Child, HVSpace,
            Child, VVSpace,
            Child, VVSpace,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Display options"),
         Child, ColGroup(5),
            Child, tmp.CH_ShowLog         = MakeKeyCheckMark(FALSE, "  l"),
            Child, KeyLLabel1(GetStr("  Show log"), *GetStr("  l")),
            Child, HVSpace,
            Child, tmp.CH_ShowLamps       = MakeKeyCheckMark(FALSE, "  e"),
            Child, KeyLLabel1(GetStr("  Show interface leds"), *GetStr("  e")),
            Child, tmp.CH_ShowConnect     = MakeKeyCheckMark(FALSE, "  p"),
            Child, KeyLLabel1(GetStr("  Show speed"), *GetStr("  p")),
            Child, HVSpace,
            Child, tmp.CH_ShowOnlineTime  = MakeKeyCheckMark(FALSE, "  o"),
            Child, KeyLLabel1(GetStr("  Show online time"), *GetStr("  o")),
            Child, tmp.CH_ShowButtons     = MakeKeyCheckMark(FALSE, "  b"),
            Child, KeyLLabel1(GetStr("  Show buttons"), *GetStr("  b")),
            Child, HVSpace,
            Child, tmp.CH_ShowNetwork     = MakeKeyCheckMark(FALSE, "  n"),
            Child, KeyLLabel1(GetStr("  Show network"), *GetStr("  n")),
            Child, tmp.CH_ShowUser        = MakeKeyCheckMark(FALSE, "  u"),
            Child, KeyLLabel1(GetStr("  Show user"), *GetStr("  u")),
            Child, HVSpace,
            Child, tmp.CH_ShowStatusWin   = MakeKeyCheckMark(FALSE, "  w"),
            Child, KeyLLabel1(GetStr("  Show status windows"), *GetStr("  w")),
            Child, tmp.CH_ShowSerialInput = MakeKeyCheckMark(FALSE, "  r"),
            Child, KeyLLabel1(GetStr("  Show serial input"), *GetStr("  r")),
            Child, HVSpace,
            Child, VVSpace,
            Child, VVSpace,
         End,
         Child, HVSpace,
      End,
      Child, HVSpace,
   TAG_MORE, msg->ops_AttrList))
   {
      struct Dialer_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      data->GR_Events = VGroup,
#ifndef DO_LISTTREE
         InnerSpacing(0,0),
#endif
         Child, HVSpace,
         Child, ColGroup(2),
            Child, MUI_MakeObject(MUIO_BarTitle, "Event"),
            Child, MUI_MakeObject(MUIO_BarTitle, "Execute"),
            Child, MakeKeyLabel2("  Startup:", "  t"),
            Child, data->PA_Startup = MakePopAsl(data->STR_Startup = MakeKeyString(NULL, MAXPATHLEN, "  t"), "  Choose file", FALSE),
            Child, MakeKeyLabel2("  Shutdown:", "  h"),
            Child, data->PA_Shutdown = MakePopAsl(data->STR_Shutdown = MakeKeyString(NULL, MAXPATHLEN, "  t"), "  Choose file", FALSE),
         End,
         Child, HVSpace,
      End;

      if(data->GR_Events)
      {
set(data->CH_ConfirmOffline, MUIA_Disabled, TRUE);
set(data->CH_QuickReconnect, MUIA_Disabled, TRUE);
      }
      else
         obj = NULL;
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

