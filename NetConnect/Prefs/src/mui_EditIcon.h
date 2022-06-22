#define MUIM_EditIcon_Editor_Active    (TAGBASE_NETCONNECTPREFS | 0x1040)
#define MUIM_EditIcon_ChangeLine       (TAGBASE_NETCONNECTPREFS | 0x1041)
#define MUIM_EditIcon_Type_Active      (TAGBASE_NETCONNECTPREFS | 0x1042)
#define MUIM_EditIcon_Sound_Active     (TAGBASE_NETCONNECTPREFS | 0x1043)
#define MUIM_EditIcon_PlaySound        (TAGBASE_NETCONNECTPREFS | 0x1044)

struct EditIcon_Data
{
   Object *GR_Register;

   Object *GR_Button;
   Object *STR_Name;
   Object *STR_Hotkey;
   Object *CY_Type;
   Object *PA_Program;
   Object *PA_Image;
   Object *PA_Sound;
   Object *BT_PlaySound;
   Object *SL_Volume;

   Object *GR_Advanced;
   Object *STR_Stack;
   Object *SL_Priority;
   Object *PA_CurrentDir;
   Object *PA_OutputFile;
   Object *STR_PublicScreen;
   Object *CH_WBArgs;
   Object *CH_DrawFrame;
   Object *CH_ToFront;

   Object *GR_Script;
   Object *LV_Editor;
   Object *LI_Editor;
   Object *BT_New;
   Object *BT_Delete;
   Object *BT_Clear;
   Object *STR_Line;

   Object *BT_Okay;
   Object *BT_Cancel;
};

