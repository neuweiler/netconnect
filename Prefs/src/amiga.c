/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_DataBase.h"
#include "mui_Dialer.h"
#include "mui_MainWindow.h"
#include "mui_Modem.h"
#include "mui_PasswdReq.h"
#include "mui_Provider.h"
#include "mui_User.h"
#include "protos.h"

///
/// external variables
extern Object *app;
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_User;
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
SAVEDS LONG sortfunc(register __a1 STRPTR str1, register __a2 STRPTR str2)
{
   return(stricmp(str1, str2));
}

///
/// des_func
VOID SAVEDS des_func(register __a2 APTR pool, register __a1 APTR ptr)
{
   if(ptr)
      FreeVec(ptr);
}

///
/// strobjfunc
SAVEDS LONG strobjfunc(register __a2 Object *list, register __a1 Object *str)
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
VOID SAVEDS objstrfunc(register __a2 Object *list,register __a1 Object *str)
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
/// get_file_size
LONG get_file_size(STRPTR file)
{
   struct FileInfoBlock *fib;
   BPTR lock;
   LONG size = -1;

   if(lock = Lock(file, ACCESS_READ))
   {
      if(fib = AllocDosObject(DOS_FIB, NULL))
      {
         if(Examine(lock, fib))
            size = (fib->fib_DirEntryType > 0 ? -2 : fib->fib_Size);

         FreeDosObject(DOS_FIB, fib);
      }
      UnLock(lock);
   }
   return(size);
}

///
/// ParseConfig
BOOL ParseConfig(STRPTR file, struct pc_Data *pc_data)
{
   LONG size;
   STRPTR buf = NULL;
   BPTR fh;
   BOOL success = FALSE;

   if((size = get_file_size(file)) > -1)
   {
      if(buf = AllocVec(size, MEMF_ANY))
      {
         if(fh = Open(file, MODE_OLDFILE))
         {
            if(Read(fh, buf, size) == size)
            {
               success = TRUE;

               pc_data->Buffer   = buf;
               pc_data->Size     = size;
               pc_data->Current  = buf;

               pc_data->Argument = NULL;
               pc_data->Contents = NULL;
            }

            Close(fh);
         }
      }
   }

   return(success);
}

///
/// ParseNext
BOOL ParseNext(struct pc_Data *pc_data)
{
   BOOL success = FALSE;
   STRPTR ptr_eol, ptr_tmp;

   if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
   {
      if(ptr_eol = strchr(pc_data->Current, '\n'))
      {
         *ptr_eol = NULL;

         if(pc_data->Contents = strchr(pc_data->Current, 34))              /* is the content between ""'s ? */
         {
            pc_data->Contents++;
            if(ptr_tmp = strchr(pc_data->Contents, 34))  /* find the ending '"' */
               *ptr_tmp = NULL;

            ptr_tmp = pc_data->Contents - 2;
            while(((*ptr_tmp == ' ') || (*ptr_tmp == 9)) && ptr_tmp >= pc_data->Current)
               ptr_tmp--;

            ptr_tmp++;
            *ptr_tmp = NULL;
         }
         else
         {
            pc_data->Contents = strchr(pc_data->Current, ' ');                   /* a space  */
            ptr_tmp           = strchr(pc_data->Current, 9);                     /* or a TAB */

            if((ptr_tmp < pc_data->Contents && ptr_tmp) || !pc_data->Contents)   /* which one comes first ? */
               pc_data->Contents = ptr_tmp;
            if(pc_data->Contents)
            {
               *pc_data->Contents++ = NULL;
               while((*pc_data->Contents == ' ') || (*pc_data->Contents == 9))
                  pc_data->Contents++;

               if(ptr_tmp = strchr(pc_data->Contents, ';')) /* cut out the comment */
                  *ptr_tmp = NULL;
            }
            else
               pc_data->Contents = "";
         }

         pc_data->Argument = pc_data->Current;
         pc_data->Current  = ptr_eol + 1;
         success = TRUE;
      }
      else
         pc_data->Current = NULL;
   }
   return(success);
}

///
/// ParseNextLine
BOOL ParseNextLine(struct pc_Data *pc_data)
{
   BOOL success = FALSE;
   STRPTR ptr_eol;

   if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
   {
      if(ptr_eol = strchr(pc_data->Current, '\n'))
      {
         *ptr_eol = NULL;

         pc_data->Argument = "";
         pc_data->Contents = pc_data->Current;
         pc_data->Current  = ptr_eol + 1;
         success = TRUE;
      }
      else
         pc_data->Current = NULL;
   }

   return(success);
}

///
/// ParseEnd
VOID ParseEnd(struct pc_Data *pc_data)
{
   if(pc_data->Buffer)
      FreeVec(pc_data->Buffer);

   pc_data->Buffer   = NULL;
   pc_data->Size     = NULL;
   pc_data->Current  = NULL;

   pc_data->Argument = NULL;
   pc_data->Contents = NULL;
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
VOID SAVEDS IntuiMsgFunc(register __a1 struct IntuiMessage *imsg,register __a2 struct FileRequester *req)
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
   char *res = NULL;
   static const struct Hook IntuiMsgHook = { { 0,0 }, (VOID *)IntuiMsgFunc, NULL, NULL };

   get(win, MUIA_Window_Window, &w);
   if(left == -1)
   {
      left     = w->LeftEdge+w->BorderLeft + 2;
      top      = w->TopEdge+w->BorderTop + 2;
      width    = w->Width-w->BorderLeft-w->BorderRight - 4;
      height   = w->Height-w->BorderTop-w->BorderBottom - 4;
   }

   if(req = MUI_AllocAslRequestTags(ASL_FileRequest,
      ASLFR_Window         , w,
      ASLFR_TitleText      , title,
      ASLFR_InitialLeftEdge, left,
      ASLFR_InitialTopEdge , top,
      ASLFR_InitialWidth   , width,
      ASLFR_InitialHeight  , height,
//    ASLFR_InitialDrawer  , (drawer ? drawer : (STRPTR)"PROGDIR:"),
      ASLFR_InitialFile    , (file ? file : (STRPTR)""),
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

   return(res);
}

///
/// get_configcontents
STRPTR get_configcontents(BOOL ppp)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct Modem_Data *modem_data = INST_DATA(CL_Modem->mcc_Class, data->GR_Modem);
   struct Provider_Data *provider_data = INST_DATA(CL_Provider->mcc_Class, data->GR_Provider);
   struct User_Data *user_data = INST_DATA(CL_User->mcc_Class, data->GR_User);
   static char buffer[1024];

   if(ppp)
   {
      sprintf(buffer, "sername %ls\nserunit %ld\nserbaud %ld\nlocalipaddress %ls\n%lsuser %ls\nsecret %ls\n",
         xget(modem_data->STR_SerialDriver, MUIA_String_Contents),
         xget(modem_data->STR_SerialUnit, MUIA_String_Integer),
         xget(modem_data->STR_BaudRate, MUIA_String_Integer),
         xget(provider_data->STR_IP_Address, MUIA_String_Contents),
         (xget(modem_data->CH_Carrier, MUIA_Selected) ? "cd yes\n" : ""),
//  (xget(modem_data->CH_7Wire, MUIA_Selected) ? "7Wire " : ""),
         xget(user_data->STR_LoginName, MUIA_String_Contents),
         xget(user_data->STR_Password, MUIA_String_Contents));
//  xget(provider_data->STR_MTU, MUIA_String_Integer));

/*      sprintf(buffer, "%ls %ld %ld Shared %ls %ls%ls ALLOWPAP=YES ALLOWCHAPMS=YES ALLOWCHAPMD5=YES USERID=%ls PASSWORD=%ls MTU=%ld",
         xget(modem_data->STR_SerialDriver, MUIA_String_Contents),
         xget(modem_data->STR_SerialUnit, MUIA_String_Integer),
         xget(modem_data->STR_BaudRate, MUIA_String_Integer),
         xget(provider_data->STR_IP_Address, MUIA_String_Contents),
         (xget(modem_data->CH_Carrier, MUIA_Selected) ? "CD " : ""),
         (xget(modem_data->CH_7Wire, MUIA_Selected) ? "7Wire " : ""),
         xget(user_data->STR_LoginName, MUIA_String_Contents),
         xget(user_data->STR_Password, MUIA_String_Contents),
         xget(provider_data->STR_MTU, MUIA_String_Integer));
*/
   }
   else
   {
      sprintf(buffer, "%ls %ld %ld Shared %ls %ls%ls MTU=%ld",
         xget(modem_data->STR_SerialDriver, MUIA_String_Contents),
         xget(modem_data->STR_SerialUnit, MUIA_String_Integer),
         xget(modem_data->STR_BaudRate, MUIA_String_Integer),
         xget(provider_data->STR_IP_Address, MUIA_String_Contents),
         (xget(modem_data->CH_Carrier, MUIA_Selected) ? "CD " : ""),
         (xget(modem_data->CH_7Wire, MUIA_Selected) ? "7Wire " : ""),
         xget(provider_data->STR_MTU, MUIA_String_Integer));
   }

   return(buffer);
}

///

