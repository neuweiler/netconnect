/*
 * main.h
 *
 *  Created on: 25 Jun 2022
 *      Author: michael
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "includes.h"

#include "NetConnect.h"
#include "locale/NetConnect.h"
#include "mui.h"
#include "mui_About.h"
#include "mui_DockPrefs.h"
#include "mui_EditIcon.h"
#include "mui_Editor.h"
#include "mui_IconList.h"
#include "mui_MainWindow.h"
#include "mui_MenuPrefs.h"
#include "mui_PagerList.h"
#include "mui_ProgramList.h"
#include "protos.h"
#include "rev.h"

#define NEWSTACK_SIZE 16384

extern struct Catalog *cat;
extern Object *SoundObject, *app, *group, *win;
extern struct MUI_CustomClass *CL_MainWindow, *CL_PagerList, *CL_ProgramList, *CL_MenuPrefs,
                              *CL_IconList, *CL_DockPrefs, *CL_EditIcon, *CL_Editor, *CL_About;
extern struct StackSwapStruct StackSwapper;
extern struct Process *proc;
extern struct NewMenu MainWindowMenu[];
extern struct Library *MUIMasterBase;

#endif /* MAIN_H_ */
