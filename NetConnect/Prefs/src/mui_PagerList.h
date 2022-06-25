#ifndef PAGERLIST_H_
#define PAGERLIST_H_

#include "includes.h"

#include "NetConnect.h"
#include "locale/NetConnect.h"
#include "mui.h"
#include "mui_PagerList.h"
#include "protos.h"
#include "images/information.h"
#include "images/menus.h"
#include "images/dock.h"

struct PagerList_Data
{
   Object *o_information;
   Object *o_menus;
   Object *o_dock;
   Object *i_information;
   Object *i_menus;
   Object *i_dock;

   STRPTR ptr_information;    // this is for pagerlist_displayfunc
   STRPTR ptr_menus;

   struct Hook DisplayHook;
};

#endif
