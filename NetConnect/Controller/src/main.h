/*
 * main.h
 *
 *  Created on: 23 Jun 2022
 *      Author: michael
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "includes.h"

#include "../../NetConnect.h"
#include "../../locale/Strings.h"
#include "../../../genesis.lib/src/pragmas/nc_lib.h"
#include "mui.h"
#include "mui_About.h"
#include "mui_Button.h"
#include "mui_Dock.h"
#include "mui_MenuList.h"
#include "protos.h"
#include "rev.h"
//#include <workbench/WBStart.h>
#include "../../../WBStart.h"

#define NEWSTACK_SIZE 16384

extern struct ExecBase *SysBase;
extern struct Catalog *cat;
extern Object *SoundObject, *app, *group, *menu_list;
extern struct Library *NetConnectBase;
extern struct Library *MUIMasterBase;

extern struct MUI_CustomClass *CL_Dock, *CL_Button, *CL_About, *CL_MenuList;
extern struct Hook BrokerHook;
extern struct MUI_InputHandlerNode ihnode;
extern struct MsgPort *appmenu_port;
extern ULONG NotifySignal;
extern struct NotifyRequest nr;
extern struct Process *proc;
extern struct NewMenu DockMenu[];
extern struct CommandLineInterface *LocalCLI;
extern BPTR OldCLI;
extern struct StackSwapStruct StackSwapper;
extern struct MUI_Command arexx_list[];
extern struct WBStartup *_WBenchMsg; //TODO how does this get initialized?
#ifdef DEMO
extern struct Library *BattClockBase;
#endif


#endif /* MAIN_H_ */
