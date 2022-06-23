/// includes
#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;

///

/// provider_rxfunc
SAVEDS ASM APTR provider_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg)
{
   DoMethod(win, MUIM_MainWindow_ChangeProvider, arg[0], TRUE);

   return(RETURN_OK);
}

///
/// user_rxfunc
SAVEDS ASM APTR user_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg)
{
   DoMethod(win, MUIM_MainWindow_ChangeUser, arg[0], arg[1]);

   return(RETURN_OK);
}

///

/// connect_rxfunc
SAVEDS ASM APTR connect_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

   if(arg[1])
      iterate_ifacelist(&data->isp.isp_ifaces, 2);
   else
   {
      if(arg[0] && (data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp.isp_ifaces))
      {
         struct Interface *iface;
         char **ifnames;
         int i;

         ifnames   = (char **)arg[0];

         i = 0;
         while(ifnames[i])
         {
            iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
            while(iface->if_node.mln_Succ)
            {
               if(!stricmp(iface->if_name, ifnames[i]) && !(iface->if_flags & IFL_IsOnline))
                  iface->if_flags |= IFL_PutOnline;
               iface = (struct Interface *)iface->if_node.mln_Succ;
            }
            i++;
         }
      }
      else
      {
         DoMethod(win, MUIM_MainWindow_OnOffline, TRUE);
         return(RETURN_OK);
      }
   }

   DoMethod(win, MUIM_MainWindow_PutOnline);

   return(RETURN_OK);
}

///
/// disconnect_rxfunc
SAVEDS ASM APTR disconnect_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

   if(arg[1])
      iterate_ifacelist(&data->isp.isp_ifaces, 0);
   else
   {
      if(arg[0] && (data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp.isp_ifaces))
      {
         struct Interface *iface;
         char **ifnames;
         int i;

         ifnames   = (char **)arg[0];

         i = 0;
         while(ifnames[i])
         {
            iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
            while(iface->if_node.mln_Succ)
            {
               if(!stricmp(iface->if_name, ifnames[i]) && (iface->if_flags & IFL_IsOnline))
                  iface->if_flags |= IFL_PutOffline;
               iface = (struct Interface *)iface->if_node.mln_Succ;
            }
            i++;
         }
      }
      else
      {
         DoMethod(win, MUIM_MainWindow_OnOffline, FALSE);
         return(RETURN_OK);
      }
   }

   DoMethod(win, MUIM_MainWindow_PutOffline);

   return(RETURN_OK);
}

///

/// isonline_rxfunc
SAVEDS ASM APTR isonline_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
   int online = 0;  // 0 = offline, 1 = online, 2 = connecting

   if(data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp.isp_ifaces)
   {
      struct Interface *iface;

      if(arg[1])  // is any iface online ?
      {
         iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
         while(iface->if_node.mln_Succ && !online)
         {
            if(iface->if_flags & IFL_IsOnline)
               online = 1;
            else if(iface->if_flags & IFL_PutOnline)
               online = 2;

            iface = (struct Interface *)iface->if_node.mln_Succ;
         }
      }
      else if(arg[0])
      {
         iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
         while(iface->if_node.mln_Succ)
         {
            if(!stricmp(iface->if_name, (char *)arg[0]))
            {
               if(iface->if_flags & IFL_IsOnline)
                  online = 1;
               else if(iface->if_flags & IFL_PutOnline)
                  online = 2;
               break;
            }
            iface = (struct Interface *)iface->if_node.mln_Succ;
         }
      }
      else
      {
         iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
         if(iface->if_node.mln_Succ)
         {
            if(iface->if_flags & IFL_IsOnline)
               online = 1;
            else if(iface->if_flags & IFL_PutOnline)
               online = 2;
         }
      }
   }

   return((APTR)online);
}

///
/// window_rxfunc
SAVEDS ASM APTR window_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg)
{
   if(arg[1])
      set(win, MUIA_Window_Open, FALSE);
   if(arg[0])
      set(win, MUIA_Window_Open, TRUE);

   return(RETURN_OK);
}

///

