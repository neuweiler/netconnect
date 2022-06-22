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

