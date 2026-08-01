/*
 * Minesweeper
 *
 * by Lorens Younes (d93-hyo@nada.kth.se)
 *
 * v1.0d3   (93-03-19)  Lots of colors!
 * v1.0d4   (93-11-30)  Looks better on Workbench.
 */

#define INTUI_V36_NAMES_ONLY

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <exec/types.h>
#include <libraries/gadtools.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/icon_protos.h>
#include "field.h"

#define INTUI_REV 37L

#ifdef LATTICE
	int CXBRK(void) { return(0); }
	int chkabort(void) { return(0); }
#endif

#define PRG_NAME "LazyMines"
#define VERSION_NO "1.0"
#define CREATION_YEAR "1994"
#define AUTHOR "Lorens Younes"

void Handle(void);
BOOL ProcessMenus(UWORD code);
void ClearMessages(void);
void MessageReq(struct Window *win, char *title, char *message);
VOID HandleStartupMsg(char **, BOOL), ReadToolTypes(char *);
BOOL Initialize(void);
BOOL LibraryInit(void);
BOOL DisplayInit(void);
BOOL EnvInit(void);
void Finalize(void);

void NewGame(void);
void WinTheGame(void);
void GameOver(void);
BOOL IsInside(short row, short col);
BOOL RevealThis(short row, short col);
BOOL RevealAround(short row, short col);
void ToggleLock(short row, short col);
BOOL SweepThis(short row, short col);
void PressThis(short row, short col);
void PressAround(short row, short col);
void ReleaseThis(short row, short col);
void ReleaseAround(short row, short col);

#define LEFTDOWN 0x01
#define RIGHTDOWN 0x02

struct Library *IntuitionBase = NULL, *GadToolsBase = NULL, *IconBase;
struct GfxBase *GfxBase = NULL;
struct Screen *pubScreen = NULL;
char *pubScreenName = NULL;
char screenTitle[81];
char windowTitle[81];
UBYTE winTitleEnd;
struct Window *mineWin = NULL;
struct VisualInfo *visualInfo = NULL;
struct Menu *mineMenu = NULL;
struct TextFont *defaultFont;
UWORD textColor = 1, backColor = 0;
LONG numberColors[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
UWORD shineColor = 2, shadowColor = 1;
UWORD xOffs, yOffs;

ULONG colorValues[8][3] = {
   { 0x00000000, 0x00000000, 0xA7A7A7A7 },
   { 0x00000000, 0xA1A1A1A1, 0x15151515 },
   { 0xD8D8D8D8, 0x00000000, 0x50505050 },
   { 0x00000000, 0x00000000, 0x78787878 },
   { 0x9B9B9B9B, 0x00000000, 0x39393939 },
   { 0x00000000, 0xAAAAAAAA, 0xAAAAAAAA },
   { 0x00000000, 0x00000000, 0x00000000 },
   { 0x61616161, 0x61616161, 0x61616161 }
};

struct NewMenu newMineMenu[] = {
   { NM_TITLE, "Game",               0 , 0, 0, 0 },
   {  NM_ITEM, "New",               "N", 0, 0, 0 },
   {  NM_ITEM, NM_BARLABEL,          0 , 0, 0, 0 },
   {  NM_ITEM, "About...",          "?", 0, 0, 0 },
   {  NM_ITEM, NM_BARLABEL,          0 , 0, 0, 0 },
   {  NM_ITEM, "Quit LazyMines",    "Q", 0, 0, 0 },
   { NM_TITLE, "Settings",           0 , 0, 0, 0 },
   {  NM_ITEM, "Safe Opening",       0 , CHECKIT | MENUTOGGLE, 0, 0 },
   {  NM_ITEM, "Warnings",           0 , CHECKIT | MENUTOGGLE, 0, 0 },
   {  NM_ITEM, "Auto Open",          0 , CHECKIT | MENUTOGGLE, 0, 0 },
   {  NM_ITEM, "Auto Lock",          0 , CHECKIT | MENUTOGGLE | CHECKED, 0, 0 },
   { NM_END,   NULL,                 0 , 0, 0, 0 }
};

#define MENU_GAME       0
#define ITEM_NEW        0
#define ITEM_ABOUT      2
#define ITEM_QUIT       4
#define MENU_SETTINGS   1
#define ITEM_OPENSAFE   0
#define ITEM_WARNINGS   1
#define ITEM_AUTOOPEN   2
#define ITEM_AUTOLOCK   3

struct EasyStruct aboutBox = {
   sizeof(struct EasyStruct),
   0,
   "About LazyMines",
   "%s v%s\nBy %s\nCopyright © %s %s.",
   "OK"
};

short cellW, cellH;
short maxRows, maxCols;
short num_rows = 16, num_columns = 30;
short num_mines = 99;
short revealed;
short flagsLeft;
short playing = TRUE;
BOOL openSafe = FALSE, warnings = FALSE;
BOOL autoOpen = FALSE;
BOOL autoLock = TRUE;

void
main(int argc, char **argv)
{
   HandleStartupMsg (argv, (argc == 0));
   if (Initialize())
      Handle();
   Finalize();
}

void
Handle(void)
{
   struct IntuiMessage *message;
   ULONG class;
   UWORD code;
   UBYTE mouseStat = 0;
   BOOL ignoreClick = TRUE;
   short row, col;
   short oldRow = -1, oldCol = -1;
   BOOL quit = FALSE;
   
   NewGame();
   row = (mineWin->MouseY >= yOffs) ?
         (mineWin->MouseY - yOffs) / cellH : -1;
   col = (mineWin->MouseX >= xOffs) ?
         (mineWin->MouseY - xOffs) / cellW : -1;
   if (IsInside(row, col) && playing)
      mineWin->Flags |= WFLG_RMBTRAP;
   else
      mineWin->Flags &= ~WFLG_RMBTRAP;
   while (!quit)
   {
      Wait(1L << mineWin->UserPort->mp_SigBit);
      while (message = (struct IntuiMessage *)GetMsg(mineWin->UserPort))
      {
         class = message->Class;
         code = message->Code;
         row = (message->MouseY >= yOffs) ?
               (message->MouseY - yOffs) / cellH : -1;
         col = (message->MouseX >= xOffs) ?
               (message->MouseX - xOffs) / cellW : -1;
         ReplyMsg((struct Message *)message);
         switch (class)
         {
         case IDCMP_MOUSEBUTTONS:
            if (ignoreClick)
            {
               ignoreClick = FALSE;
               break;
            }
            if (playing)
               switch (code)
               {
               case SELECTDOWN:
                  mouseStat |= LEFTDOWN;
                  if (mouseStat & RIGHTDOWN)
                     PressAround(row, col);
                  else
                     PressThis(row, col);
                  break;
               case MENUDOWN:
                  mouseStat |= RIGHTDOWN;
                  if (mouseStat & LEFTDOWN)
                  {
                     ReleaseThis(row, col);
                     PressAround(row, col);
                  }
                  break;
               case SELECTUP:
                  if (mouseStat & RIGHTDOWN)
                     playing = SweepThis(row, col);
                  else if (mouseStat & LEFTDOWN)
                     playing = RevealThis(row, col);
                  mouseStat = 0;
                  if (!playing)
                     GameOver();
                  else if (revealed == num_rows * num_columns - num_mines)
                  {
                     WinTheGame();
                     playing = FALSE;
                  }
                  break;
               case MENUUP:
                  if (mouseStat & LEFTDOWN)
                     playing = SweepThis(row, col);
                  else if (mouseStat & RIGHTDOWN)
                     ToggleLock(row, col);
                  mouseStat = 0;
                  if (!playing)
                     GameOver();
                  else if (revealed == num_rows * num_columns - num_mines)
                  {
                     WinTheGame();
                     playing = FALSE;
                  }
                  break;
               }
            if (!playing)
               mineWin->Flags &= ~WFLG_RMBTRAP;
            break;
         case IDCMP_MOUSEMOVE:
            ignoreClick = FALSE;
            if (mouseStat)
            {
               if (oldRow != row || oldCol != col)
                  if (mouseStat & LEFTDOWN)
                     if (mouseStat & RIGHTDOWN)
                     {
                        ReleaseAround(oldRow, oldCol);
                        PressAround(row, col);
                     }
                     else
                     {
                        ReleaseThis(oldRow, oldCol);
                        PressThis(row, col);
                     }
            }
            else
               if (IsInside(row, col) && playing)
                  mineWin->Flags |= WFLG_RMBTRAP;
               else
                  mineWin->Flags &= ~WFLG_RMBTRAP;
            break;
         case IDCMP_MENUPICK:
            quit = ProcessMenus(code);
            break;
         case IDCMP_ACTIVEWINDOW:
            ignoreClick = !ignoreClick;
            break;
         case IDCMP_CLOSEWINDOW:
            quit = TRUE;
            break;
         }
         oldRow = row;
         oldCol = col;
      }
      row = (mineWin->MouseY >= yOffs) ?
            (mineWin->MouseY - yOffs) / cellH : -1;
      col = (mineWin->MouseX >= xOffs) ?
            (mineWin->MouseY - xOffs) / cellW : -1;
      if (IsInside(row, col) && playing)
         mineWin->Flags |= WFLG_RMBTRAP;
      else
         mineWin->Flags &= ~WFLG_RMBTRAP;
   }
}

BOOL
ProcessMenus(
   UWORD code)
{
   UWORD menuNum, itemNum, subNum;
   struct MenuItem *item;
   BOOL quit = FALSE;
   
   while (code != MENUNULL)
   {
      item = ItemAddress(mineMenu, code);
      menuNum = MENUNUM(code);
      itemNum = ITEMNUM(code);
      subNum = SUBNUM(code);
      switch (menuNum)
      {
      case MENU_GAME:
         switch (itemNum)
         {
         case ITEM_NEW:
            if (!quit)
               NewGame();
            break;
         case ITEM_ABOUT:
            if (!quit)
            {
               OffMenu(mineWin, FULLMENUNUM(0, NOITEM, NOSUB));
               OffMenu(mineWin, FULLMENUNUM(1, NOITEM, NOSUB));
               EasyRequest(mineWin, &aboutBox, NULL, PRG_NAME, VERSION_NO,
                           AUTHOR, CREATION_YEAR, AUTHOR);
               ClearMessages();
               OnMenu(mineWin, FULLMENUNUM(0, NOITEM, NOSUB));
               OnMenu(mineWin, FULLMENUNUM(1, NOITEM, NOSUB));
            }
            break;
         case ITEM_QUIT:
            quit = TRUE;
            break;
         }
         break;
      case MENU_SETTINGS:
         if (!quit)
         {
            switch (itemNum)
            {
            case ITEM_OPENSAFE:
               openSafe = !openSafe;
               break;
            case ITEM_WARNINGS:
               warnings = !warnings;
               break;
            case ITEM_AUTOOPEN:
               autoOpen = !autoOpen;
               if (autoOpen)
                  OffMenu (mineWin, FULLMENUNUM(MENU_SETTINGS, ITEM_OPENSAFE, NOSUB));
               else
                  OnMenu (mineWin, FULLMENUNUM(MENU_SETTINGS, ITEM_OPENSAFE, NOSUB));
               break;
            case ITEM_AUTOLOCK:
               autoLock = !autoLock;
               break;
            }
            break;
         }
         break;
      }
      code = item->NextSelect;
   }
   return quit;
}

void
ClearMessages(void)
{
   struct Message *message;
   
   while (message = GetMsg(mineWin->UserPort))
      ReplyMsg(message);
}

void
MessageReq(
   struct Window *win,
   char *title,
   char *message)
{
	struct EasyStruct messageBox;
	
	messageBox.es_StructSize = sizeof(struct EasyStruct);
	messageBox.es_Flags = 0;
	messageBox.es_Title = title;
	messageBox.es_TextFormat = message;
	messageBox.es_GadgetFormat = "OK";
	EasyRequest(win, &messageBox, NULL, NULL);
}

VOID HandleStartupMsg(char **arg, BOOL fromWB)
{
   BPTR oldDir = -1;
   struct WBStartup *wbArg;
   
   if (fromWB){
      wbArg = (struct WBStartup *)arg;
      if (wbArg->sm_ArgList->wa_Lock)
         oldDir = CurrentDir(wbArg->sm_ArgList->wa_Lock);
      ReadToolTypes(wbArg->sm_ArgList->wa_Name);
      if (oldDir != -1)
         CurrentDir(oldDir);
   }
   else
      ReadToolTypes(arg[0]);
}

VOID ReadToolTypes(char *prgName)
{
   struct DiskObject *myDiskObj;
   char *toolValue;
   
   if (IconBase = OpenLibrary("icon.library", 0L)){
      if (myDiskObj = GetDiskObject(prgName)){
         if (toolValue = FindToolType(myDiskObj->do_ToolTypes, "ROWS"))
            num_rows = atoi(toolValue);
         if (toolValue = FindToolType(myDiskObj->do_ToolTypes, "COLUMNS"))
            num_columns = atoi(toolValue);
         if (toolValue = FindToolType(myDiskObj->do_ToolTypes, "MINES"))
            num_mines = atoi(toolValue);
         FreeDiskObject(myDiskObj);
      }
      CloseLibrary(IconBase);
   }
}

BOOL
Initialize(void)
{
   return (BOOL)(LibraryInit() && DisplayInit() && EnvInit());
}

BOOL
LibraryInit(void)
{
   if (IntuitionBase = OpenLibrary("intuition.library", INTUI_REV))
      if (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 37L))
         if (GadToolsBase = OpenLibrary("gadtools.library", INTUI_REV))
            return TRUE;
         else
            MessageReq(NULL, "Error", "Couldn't open gadtools.library!");
      else
         MessageReq(NULL, "Error", "Couldn't open graphics.library!");
   else
      printf("Couldn't open intuition.library v%ld!\n", INTUI_REV);
   return FALSE;
}

BOOL
DisplayInit(void)
{
   struct DrawInfo *scrDrawInfo;
   UWORD menuPen = 0;
   UWORD horizBorder, vertBorder;
   
   strncpy(screenTitle, PRG_NAME, 80);
   strncat(screenTitle, " v", 80);
   strncat(screenTitle, VERSION_NO, 80);
   strncat(screenTitle, " - ©", 80);
   strncat(screenTitle, CREATION_YEAR, 80);
   strncat(screenTitle, " ", 80);
   strncat(screenTitle, AUTHOR, 80);
   strncat(screenTitle, ".", 80);
   strncpy(windowTitle, PRG_NAME, 80);
   winTitleEnd = strlen (windowTitle);
   if ((pubScreen = LockPubScreen (pubScreenName)) ||
       pubScreenName && (pubScreen = LockPubScreen (NULL)))
   {
      if (scrDrawInfo = GetScreenDrawInfo(pubScreen))
      {
         textColor = scrDrawInfo->dri_Pens[TEXTPEN];
         backColor = scrDrawInfo->dri_Pens[BACKGROUNDPEN];
         shineColor = scrDrawInfo->dri_Pens[SHINEPEN];
         shadowColor = scrDrawInfo->dri_Pens[SHADOWPEN];
         if (scrDrawInfo->dri_Version >= 2)
            menuPen = scrDrawInfo->dri_Pens[BARDETAILPEN];
         FreeScreenDrawInfo(pubScreen, scrDrawInfo);
      }
      defaultFont = GfxBase->DefaultFont;
      horizBorder = pubScreen->WBorLeft + pubScreen->WBorRight;
      vertBorder = pubScreen->WBorTop + pubScreen->Font->ta_YSize +
                   pubScreen->WBorBottom + 1;
      cellH = max (defaultFont->tf_YSize, defaultFont->tf_XSize) + 4;
      cellW = cellH + 4;
      maxRows = ((pubScreen->Height - vertBorder) / cellH);
      maxCols = ((pubScreen->Width - horizBorder) / cellW);
      if (mineWin = OpenWindowTags(NULL,
                       WA_Left, (pubScreen->Width -
                                (num_columns * cellW + horizBorder)) / 2,
                       WA_Top, (pubScreen->Height -
                               (num_rows * cellH + vertBorder)) / 2,
                       WA_InnerWidth, num_columns * cellW,
                       WA_InnerHeight, num_rows * cellH,
                       WA_AutoAdjust, FALSE,
                       WA_Title, windowTitle,
                       WA_ScreenTitle, screenTitle,
                       WA_DragBar, TRUE,
                       WA_CloseGadget, TRUE,
                       WA_DepthGadget, TRUE,
                       WA_Activate, TRUE,
                       WA_PubScreen, pubScreen,
                       WA_NewLookMenus, TRUE,
                       WA_ReportMouse, TRUE,
                       WA_IDCMP, IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE |
                                 IDCMP_MENUPICK | IDCMP_ACTIVEWINDOW |
                                 IDCMP_CLOSEWINDOW,
                       TAG_END))
      {
         UnlockPubScreen(NULL, pubScreen);
         xOffs = mineWin->BorderLeft;
         yOffs = mineWin->BorderTop;
         if (visualInfo = GetVisualInfo(mineWin->WScreen, TAG_END))
            if (mineMenu = CreateMenus(newMineMenu,
                              GTMN_FrontPen, menuPen,
                              TAG_END))
            {
               if(LayoutMenus(mineMenu, visualInfo, TAG_END))
                  if (SetMenuStrip(mineWin, mineMenu))
                     return TRUE;
                  else
                     MessageReq(mineWin, "Error", "Couldn't set menustrip!");
               else
                  MessageReq(mineWin, "Error", "Couldn't layout menus!");
            }
            else
               MessageReq(mineWin, "Error", "Couldn't create menu!");
         else
            MessageReq(mineWin, "Error", "Couldn't get visual info!");
      }
      else
         MessageReq(NULL, "Error", "Couldn't open window!");
   }
   else
      MessageReq(NULL, "Error", "Couldn't lock public screen!");
   if (pubScreen)
      UnlockPubScreen(NULL, pubScreen);
   return(FALSE);
}

BOOL
EnvInit(void)
{
   register int   i;

   if (GfxBase->LibNode.lib_Version >= 39L)
   {
      for (i = 0; i < 8; ++i)
      {
         numberColors[i] = ObtainBestPen (pubScreen->ViewPort.ColorMap,
                                          colorValues[i][0],
                                          colorValues[i][1],
                                          colorValues[i][2],
                                          OBP_Precision, PRECISION_GUI,
                                          TAG_DONE);
         if (numberColors[i] == backColor)
            numberColors[i] = -1;
      }
   }
   srand48(time(NULL));
   return define_field (num_rows, num_columns, num_mines);
}

void
Finalize(void)
{
   register int   i;
   
   free_field ();
   for (i = 0; i < 8; ++i)
      if (numberColors[i] != -1)
         ReleasePen (pubScreen->ViewPort.ColorMap, numberColors[i]);
   if (visualInfo)
      FreeVisualInfo(visualInfo);
   if (mineWin)
   {
      ClearMenuStrip(mineWin);
      CloseWindow(mineWin);
   }
   FreeMenus (mineMenu);
   if (GadToolsBase)
      CloseLibrary(GadToolsBase);
   if (GfxBase)
      CloseLibrary((struct Library *)GfxBase);
   if (IntuitionBase)
      CloseLibrary(IntuitionBase);
}
