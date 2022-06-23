/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "protos.h"

///
/// external variables
extern Object *app;
extern Object *win;
extern struct Library *GenesisLibrary;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_ProviderWindow;
extern struct MUI_CustomClass  *CL_Provider;
extern struct MUI_CustomClass  *CL_Dialer;
extern struct MUI_CustomClass  *CL_Users;
extern struct MUI_CustomClass  *CL_Databases;
extern struct MUI_CustomClass  *CL_Modem;
extern struct MUI_CustomClass  *CL_About;
extern struct MUI_CustomClass  *CL_PasswdReq;

///

/// DoSuperNew
ULONG DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...)
{
   return(DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}

///
/// xget
LONG xget(Object *obj, ULONG attribute)
{
   LONG x;
   get(obj, attribute, &x);
   return(x);
}

///
/// sortfunc
SAVEDS ASM LONG sortfunc(register __a1 STRPTR str1, register __a2 STRPTR str2)
{
   return(stricmp(str1, str2));
}

///
/// des_func
SAVEDS ASM VOID des_func(register __a2 APTR pool, register __a1 APTR ptr)
{
   if(ptr)
      FreeVec(ptr);
}

///
/// strobjfunc
SAVEDS ASM LONG strobjfunc(register __a2 Object *list, register __a1 Object *str)
{
   char *x, *s;
   int i;

   get(str, MUIA_String_Contents, &s);

   i = 0;
   FOREVER
   {
      DoMethod(list, MUIM_List_GetEntry, i, &x);
      if(!x)
      {
         set(list, MUIA_List_Active, MUIV_List_Active_Off);
         break;
      }
      else
      {
         if(!stricmp(x, s))
         {
            set(list, MUIA_List_Active, i);
            break;
         }
      }

      i++;
   }
   return(TRUE);
}

///
/// objstrfunc
SAVEDS ASM VOID objstrfunc(register __a2 Object *list,register __a1 Object *str)
{
   char *x;

   DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
   if(x)
      set(str, MUIA_String_Contents, x);
}

///
/// MakeKeyLabel1
Object *MakeKeyLabel1(STRPTR label, STRPTR control_char)
{
   return(KeyLabel1(GetStr(label), *GetStr(control_char)));
}

///
/// MakeKeyLabel2
Object *MakeKeyLabel2(STRPTR label, STRPTR control_char)
{
   return(KeyLabel2(GetStr(label), *GetStr(control_char)));
}

///
/// MakeButton
Object *MakeButton(STRPTR string)
{
   Object *obj = SimpleButton(GetStr(string));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

///
/// MakeKeyString
Object *MakeKeyString(STRPTR string, LONG len, STRPTR c)
{
   return(TextinputObject,
      StringFrame,
      MUIA_CycleChain         , 1,
      MUIA_Textinput_Multiline, FALSE,
      MUIA_Textinput_Contents , string,
      MUIA_Textinput_MaxLen   , len,
      MUIA_ControlChar        , *GetStr(c),
      End);
}

///
/// MakeKeyCycle
Object *MakeKeyCycle(STRPTR *array, STRPTR control_char)
{
   Object *obj = KeyCycle(array, *(GetStr(control_char)));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

///
/// MakeKeySlider
Object *MakeKeySlider(LONG min, LONG max, LONG level, STRPTR control_char)
{
   Object *obj = KeySlider(min, max, level, *(GetStr(control_char)));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

///
/// MakeKeyCheckmark
Object *MakeKeyCheckMark(BOOL state, STRPTR control_char)
{
   Object *obj = KeyCheckMark(state, *(GetStr(control_char)));
   if(obj)
      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

///
/// MakePopAsl
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only)
{
   Object *obj = PopaslObject,
      MUIA_Popstring_String, string,
      MUIA_Popstring_Button, PopButton((drawers_only ? MUII_PopDrawer : MUII_PopFile)),
      MUIA_Popasl_Type     , ASL_FileRequest,
      ASLFR_TitleText      , GetStr(title),
      ASLFR_DrawersOnly    , drawers_only,
   End;
//   if(obj)
//      set(obj, MUIA_CycleChain, 1);
   return(obj);
}

///
/// extract_arg
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep)
{
   STRPTR ptr1, ptr2;

   strncpy(buffer, string, len);

   ptr1 = strchr(buffer, (sep ? sep : ' '));
   ptr2 = strchr(buffer, 9);

   if(ptr2 && ((ptr2 < ptr1) || !ptr1))
      ptr1 = ptr2;
   if(ptr1)
      *ptr1 = NULL;

   string += strlen(buffer);

   while(*string == ' ' || *string == 9 || (sep ? *string == sep : NULL))
      string++;

   return((*string ? string : NULL));
}

///
/// IntuiMsgFunc
SAVEDS ASM VOID IntuiMsgFunc(register __a1 struct IntuiMessage *imsg,register __a2 struct FileRequester *req)
{
   if(imsg->Class == IDCMP_REFRESHWINDOW)
      DoMethod(req->fr_UserData, MUIM_Application_CheckRefresh);
}

///
/// getfilename
char *getfilename(Object *win, STRPTR title, STRPTR file, BOOL save)
{
   static char buf[512];
   struct FileRequester *req;
   struct Window *w;
   static LONG left=-1,top=-1,width=-1,height=-1;
   STRPTR res = NULL, ptr = NULL;
   static const struct Hook IntuiMsgHook = { { 0,0 }, (VOID *)IntuiMsgFunc, NULL, NULL };

   get(win, MUIA_Window_Window, &w);
   if(left == -1)
   {
      left     = w->LeftEdge+w->BorderLeft + 2;
      top      = w->TopEdge+w->BorderTop + 2;
      width    = w->Width-w->BorderLeft-w->BorderRight - 4;
      height   = w->Height-w->BorderTop-w->BorderBottom - 4;
   }

   strcpy(buf, file);
   if(ptr = PathPart(buf))
      *ptr++ = NULL;

   if(req = MUI_AllocAslRequestTags(ASL_FileRequest,
      ASLFR_Window         , w,
      ASLFR_TitleText      , title,
      ASLFR_InitialLeftEdge, left,
      ASLFR_InitialTopEdge , top,
      ASLFR_InitialWidth   , width,
      ASLFR_InitialHeight  , height,
      ASLFR_InitialDrawer  , buf,
      ASLFR_InitialFile    , (ptr ? ptr : file),
      ASLFR_DoSaveMode     , save,
      ASLFR_RejectIcons    , TRUE,
      ASLFR_UserData       , app,
      ASLFR_IntuiMsgFunc   , &IntuiMsgHook,
      TAG_DONE))
   {
      set(app, MUIA_Application_Sleep, TRUE);
      if(MUI_AslRequestTags(req, TAG_DONE))
      {
         if(*req->fr_File)
         {
            res = buf;
            strncpy(buf, req->fr_Drawer, sizeof(buf));
            AddPart(buf, req->fr_File, sizeof(buf));
         }
         left     = req->fr_LeftEdge;
         top      = req->fr_TopEdge;
         width    = req->fr_Width;
         height   = req->fr_Height;
      }
      MUI_FreeAslRequest(req);
      set(app, MUIA_Application_Sleep, FALSE);
   }

   return((char *)res);
}

///

/// clear_list
VOID clear_list(struct MinList *list)
{
   if(list->mlh_TailPred != (struct MinNode *)list)
   {
      struct MinNode *e1, *e2;

      e1 = list->mlh_Head;
      while(e2 = e1->mln_Succ)
      {
         Remove((struct Node *)e1);
         FreeVec(e1);
         e1 = e2;
      }
   }
}

///

#define ENCODE(c) (c ? (c & 0x3F) + 0x20 : 0x60)
#define DECODE(c) ((c - 0x20) & 0x3F)

/// encrypt
VOID encrypt(STRPTR in, STRPTR out)
{
   LONG n, i;
   UBYTE c;
   STRPTR s, t;

   n = strlen(in);
   if (n > 0)
   {
      s = out;
      *s++ = ENCODE(n);

      for (i = 0; i < n; i += 3)
      {
         t = &in[i];

         c = t[0] >> 2;
         *s++ = ENCODE(c);
         c = (t[0] << 4) & 0x30 | (t[1] >> 4) & 0x0F;
         *s++ = ENCODE(c);
         c = (t[1] << 2) & 0x3C | (t[2] >> 6) & 0x03;
         *s++ = ENCODE(c);
         c = t[2] & 0x3F;
         *s++ = ENCODE(c);
      }
      *s = NULL;
   }
}

///
/// decrypt
VOID decrypt(STRPTR in, STRPTR out)
{
   STRPTR s, t;
   LONG l, c;

   s = in;
   t = out;
   c = *s++;
   l = DECODE(c);
   if (c != '\n' && l > 0)
   {
      while (l >= 4)
      {
         c = DECODE(s[0]) << 2 | DECODE(s[1]) >> 4;
         *t++ = c;
         c = DECODE(s[1]) << 4 | DECODE(s[2]) >> 2;
         *t++ = c;
         c = DECODE(s[2]) << 6 | DECODE(s[3]);
         *t++ = c;

         s += 4;
         l -= 3;
      }
      c = DECODE(s[0]) << 2 | DECODE(s[1]) >> 4;
      if (l >= 1)
         *t++ = c;
      c = DECODE(s[1]) << 4 | DECODE(s[2]) >> 2;
      if (l >= 2)
         *t++ = c;
      c = DECODE(s[2]) << 6 | DECODE(s[3]);
      if (l >= 3)
         *t++ = c;
      s += 4;
   }
   *t = NULL;
}

///

