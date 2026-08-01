#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "localize.h"
#include "requesters.h"
#include "tooltypes.h"
#include "highscores.h"
#include "field.h"
#include "counter.h"
#include "layout.h"
#include "images.h"
#include "game.h"
#include "timer.h"

#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>


#ifdef SAS_C
void __regargs _CXBRK (void) {}
#endif


#define PRG_NAME        "LazyMines"
#define VERSION_NO      "2.3"
#define CREATION_YEAR   "1994-1995"
#define AUTHOR          "Lorens Younes"
#define MAIL_ADDRESS    "(d93-hyo@nada.kth.se)"

STRPTR version = "$VER: LazyMines 2.3 (8.2.95)";


void event_loop (void);
BOOL process_menus (UWORD);
void win_game (void);
void game_over (void);
void new_game (UWORD);
void load_high_score (void);
void save_high_score (void);
BOOL initialize (void);
BOOL init_display (void);
BOOL init_menu (void);
void finalize (void);
void finalize_display (void);


struct Library  *IntuitionBase = NULL;
struct GfxBase  *GfxBase = NULL;
struct Library  *GadToolsBase = NULL;

struct TextAttr   topaz8 = {
   "topaz.font", 8, 0, FPF_ROMFONT
};

struct NewMenu new_menu[] = {
   { NM_TITLE, (STRPTR)MSG_GAME_MENU, 0, 0, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_GAME_NEW, 0, 0, 0, 0 },
   {  NM_ITEM, NM_BARLABEL, 0, 0, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_GAME_NOVICE, 0, CHECKIT, ~0x04, 0 },
   {  NM_ITEM, (STRPTR)MSG_GAME_AMATURE, 0, CHECKIT, ~0x08, 0 },
   {  NM_ITEM, (STRPTR)MSG_GAME_EXPERT, 0, CHECKIT, ~0x10, 0 },
   {  NM_ITEM, NM_BARLABEL, 0, 0, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_GAME_HIGHSCORE, 0, 0, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_GAME_ABOUT, 0, 0, 0, 0 },
   {  NM_ITEM, NM_BARLABEL, 0, 0, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_GAME_QUIT, 0, 0, 0, 0 },
   { NM_TITLE, (STRPTR)MSG_SETTINGS_MENU, 0, 0, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_SETTINGS_WARNINGS, 0, CHECKIT | MENUTOGGLE, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_SETTINGS_AUTOLOCK, 0, CHECKIT | MENUTOGGLE, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_SETTINGS_SAFEOPEN, 0, CHECKIT | MENUTOGGLE, 0, 0 },
   {  NM_ITEM, NM_BARLABEL, 0, 0, 0, 0 },
   {  NM_ITEM, (STRPTR)MSG_SETTINGS_SAVE, 0, 0, 0, 0 },
   { NM_END,   NULL, 0, 0, 0, 0 }
};
#define MENU_Game       0
#define ITEM_New        0
#define ITEM_Novice     2
#define ITEM_Amature    3
#define ITEM_Expert     4
#define ITEM_High       6
#define ITEM_About      7
#define ITEM_Quit       9
#define MENU_Settings       1
#define ITEM_Warnings       0
#define ITEM_Autolock       1
#define ITEM_Safeopen       2
#define ITEM_SaveSettings   4

char   pubscr_name[129];
char   scr_title[81];
struct Screen  *pub_screen = NULL;
struct DrawInfo  *draw_info = NULL;
struct TextFont  *romfont = NULL;
APTR   vis_info = NULL;
struct Window  *main_win = NULL;
struct Menu  *main_menu = NULL;

#define LEFTDOWN    0x01
#define RIGHTDOWN   0x02

struct level   levels[] = {
   { 8, 8, 10 },
   { 16, 16, 40 },
   { 30, 16, 99 },
   { 30, 16, 99 }
};

UBYTE   current_level = EXPERT_LEVEL;
BOOL    place_warnings = FALSE;
BOOL    safe_opening = FALSE;
BOOL    auto_lock = FALSE;

field_ptr     field = NULL;
counter_ptr   flag_counter = NULL;
counter_ptr   time_counter = NULL;
timer_ptr     timer_obj;

BOOL   playing = TRUE;
BOOL   time_on = FALSE;


void
main (
   int    argc,
   char  *argv[])
{
   init_locale ("LazyMines.catalog");
   
   pubscr_name[0] = '\0';
   
   handle_startup_msg (argv, argc == 0);
   new_menu[ITEM_Novice + current_level + 1].nm_Flags |= CHECKED;
   if (place_warnings)
      new_menu[ITEM_Quit + ITEM_Warnings + 3].nm_Flags |= CHECKED;
   if (safe_opening)
      new_menu[ITEM_Quit + ITEM_Safeopen + 3].nm_Flags |= CHECKED;
   if (auto_lock)
      new_menu[ITEM_Quit + ITEM_Autolock + 3].nm_Flags |= CHECKED;
   
   if (initialize ())
   {
      srand48 (time (NULL));
      load_high_scores (AUTHOR);
      event_loop ();
      save_high_scores ();
   }
   
   finalize ();
   finalize_locale ();
}

void
event_loop (void)
{
   ULONG   winsig, timersig, sigmask;
   WORD    row, col, old_row = -1, old_col = -1;
   UBYTE   mouse_stat = 0;
   BOOL    ignore_click = TRUE;
   BOOL    quit = FALSE;
   ULONG   class;
   UWORD   code;
   struct IntuiMessage  *msg;
   
   new_game (current_level);
   row = (main_win->MouseY >= field_top (field)) ?
         (main_win->MouseY - field_top (field)) / cell_h : -1;
   col = (main_win->MouseX >= field_left (field)) ?
         (main_win->MouseX >= field_left (field)) / cell_w : -1;
   if (field_inside (field, row, col) && playing)
      main_win->Flags |= WFLG_RMBTRAP;
   else
      main_win->Flags &= ~WFLG_RMBTRAP;
   
   winsig = 1L << main_win->UserPort->mp_SigBit;
   timersig = timer_signal (timer_obj);
   while (!quit)
   {
      sigmask = Wait (winsig | timersig);
      if (sigmask & winsig)
      {
         while (msg = (struct IntuiMessage *)GetMsg (main_win->UserPort))
         {
            class = msg->Class;
            code = msg->Code;
            row = (msg->MouseY >= field_top (field)) ?
                  (msg->MouseY - field_top (field)) / cell_h : -1;
            col = (msg->MouseX >= field_left (field)) ?
                  (msg->MouseX - field_left (field)) / cell_w : -1;
            ReplyMsg ((struct Message *)msg);
            switch (class)
            {
            case IDCMP_MOUSEBUTTONS:
               if (ignore_click)
                  ignore_click = FALSE;
               else if (playing)
               {
                  switch (code)
                  {
                  case SELECTDOWN:
                     if (field_inside (field, row, col))
                     {
                        mouse_stat |= LEFTDOWN;
                        if (mouse_stat & RIGHTDOWN)
                           press_around (field, row, col);
                        else
                           press_this (field, row, col);
                     }
                     break;
                  case MENUDOWN:
                     mouse_stat |= RIGHTDOWN;
                     if (mouse_stat & LEFTDOWN)
                     {
                        release_this (field, row, col);
                        press_around (field, row, col);
                     }
                     break;
                  case SELECTUP:
                     if (mouse_stat & LEFTDOWN)
                     {
                        if (!time_on)
                        {
                           time_on = TRUE;
                           timer_start (timer_obj, 0L, 1000000L);
                        }
                        if (mouse_stat & RIGHTDOWN)
                           playing = sweep_this (field, row, col);
                        else
                           playing = reveal_this (field, row, col);
                        
                        mouse_stat = 0;
                        if (!playing)
                           game_over ();
                        else if (field_swept (field))
                        {
                           win_game ();
                           playing = FALSE;
                        }
                        time_on = playing;
                        if (!time_on)
                           timer_stop (timer_obj);
                     }
                     break;
                  case MENUUP:
                     if (mouse_stat & RIGHTDOWN)
                     {
                        if (mouse_stat & LEFTDOWN)
                        {
                           if (!time_on)
                           {
                              time_on = TRUE;
                              timer_start (timer_obj, 0L, 1000000L);
                           }
                           time_on = playing = sweep_this (field, row, col);
                           if (!time_on)
                              timer_stop (timer_obj);
                        }
                        else
                           toggle_lock (field, row, col);
                        
                        mouse_stat = 0;
                        if (!playing)
                           game_over ();
                        else if (field_swept (field))
                        {
                           win_game ();
                           time_on = playing = FALSE;
                           timer_stop (timer_obj);
                        }
                     }
                     break;
                  }
               }
               break;
            case IDCMP_MENUPICK:
               quit = process_menus (code);
               break;
            case IDCMP_MOUSEMOVE:
               ignore_click = FALSE;
               if (mouse_stat != 0)
               {
                  if (old_row != row || old_col != col)
                  {
                     if (mouse_stat & LEFTDOWN)
                     {
                        if (mouse_stat & RIGHTDOWN)
                        {
                           release_around (field, old_row, old_col);
                           press_around (field, row, col);
                        }
                        else
                        {
                           release_this (field, old_row, old_col);
                           press_this (field, row, col);
                        }
                     }
                  }
               }
               else
               {
                  if (field_inside (field, row, col) && playing)
                     main_win->Flags |= WFLG_RMBTRAP;
                  else
                     main_win->Flags &= ~WFLG_RMBTRAP;
               }
               break;
            case IDCMP_ACTIVEWINDOW:
               ignore_click = !ignore_click;
               break;
            case IDCMP_CLOSEWINDOW:
               quit = TRUE;
               break;
            }
            old_row = row;
            old_col = col;
         }
      }
      if (sigmask & timersig)
      {
         if (time_on)
         {
            timer_continue (timer_obj, 0L, 1000000L);
            counter_update (time_counter, counter_value (time_counter) + 1);
         }
         else
            timer_stop (timer_obj);
      }
      row = (main_win->MouseY >= field_top (field)) ?
            (main_win->MouseY - field_top (field)) / cell_h : -1;
      col = (main_win->MouseX >= field_left (field)) ?
            (main_win->MouseX >= field_left (field)) / cell_w : -1;
      if (field_inside (field, row, col) && playing)
         main_win->Flags |= WFLG_RMBTRAP;
      else
         main_win->Flags &= ~WFLG_RMBTRAP;
   }
}

BOOL
process_menus (
   UWORD   code)
{
   char    buf_1[81], buf_2[256];
   UWORD   menu_no, item_no, sub_no;
   struct MenuItem  *item;
   BOOL   quit = FALSE;
   
   while (code != MENUNULL)
   {
      item = ItemAddress (main_menu, code);
      menu_no = MENUNUM (code);
      item_no = ITEMNUM (code);
      sub_no = SUBNUM (code);
      
      if (!quit)
      {
         switch (menu_no)
         {
         case MENU_Game:
            switch (item_no)
            {
            case ITEM_New:
               new_game (current_level);
               break;
            case ITEM_Novice:
               new_game (NOVICE_LEVEL);
               break;
            case ITEM_Amature:
               new_game (AMATURE_LEVEL);
               break;
            case ITEM_Expert:
               new_game (EXPERT_LEVEL);
               break;
            case ITEM_High:
               display_high_scores (current_level);
               break;
            case ITEM_About:
               sprintf (buf_1,
                        localized_string (MSG_ABOUT_REQTITLE), PRG_NAME);
               sprintf (buf_2, localized_string (MSG_ABOUT_REQMSG),
                        PRG_NAME, VERSION_NO, AUTHOR, MAIL_ADDRESS,
                        CREATION_YEAR, AUTHOR);
               msg_requester (main_win, buf_1,
                              localized_string (MSG_CONTINUE_GAD), buf_2);
               break;
            case ITEM_Quit:
               quit = TRUE;
               break;
            }
            break;
         case MENU_Settings:
            switch (item_no)
            {
            case ITEM_Warnings:
               place_warnings = !place_warnings;
               break;
            case ITEM_Autolock:
               auto_lock = !auto_lock;
               break;
            case ITEM_Safeopen:
               safe_opening = !safe_opening;
               break;
            case ITEM_SaveSettings:
               save_tooltypes ();
               break;
            }
            break;
         }
      }
      code = item->NextSelect;
   }
   
   return quit;
}

void
win_game (void)
{
   field_win (field);
   if (update_high_score (current_level, counter_value (time_counter)))
      display_high_scores (current_level);
}

void
game_over (void)
{
   field_lose (field);
}

void
new_game (
   UWORD   level)
{
   if (level != current_level)
   {
      UWORD   win_w, win_h;
      ULONG   win_idcmp = main_win->IDCMPFlags;
      BOOL    done = FALSE;
      struct IntuiMessage  *msg;
      
      field_delete (field);
      if (digital_display);
      counter_delete (time_counter);
      
      SetAPen (main_win->RPort, gui_pens[BACKGROUNDPEN]);
      Move (main_win->RPort, main_win->BorderLeft + LINEWIDTH,
            main_win->Height - main_win->BorderBottom - 1);
      Draw (main_win->RPort, main_win->Width - main_win->BorderRight - 1,
            main_win->Height - main_win->BorderBottom - 1);
      Draw (main_win->RPort, main_win->Width - main_win->BorderRight - 1,
            main_win->BorderTop + LINEHEIGHT);
      Move (main_win->RPort, main_win->Width - main_win->BorderRight - 2,
            main_win->BorderTop + LINEHEIGHT);
      Draw (main_win->RPort, main_win->Width - main_win->BorderRight - 2,
            main_win->Height - main_win->BorderBottom - 2);
      
      ModifyIDCMP (main_win, IDCMP_CHANGEWINDOW);
      window_extent (pub_screen, level, main_win->RPort->TxWidth,
                     main_win->RPort->TxHeight, &win_w, &win_h);
      ChangeWindowBox (main_win,
                       (win_w > pub_screen->Width - main_win->LeftEdge) ?
                       pub_screen->Width - win_w : main_win->LeftEdge,
                       (win_h > pub_screen->Height - main_win->TopEdge) ?
                       pub_screen->Height - win_h : main_win->TopEdge,
                       win_w, win_h);
      while (!done)
      {
         WaitPort (main_win->UserPort);
         while (msg = (struct IntuiMessage *)GetMsg (main_win->UserPort))
         {
            if (msg->Class == IDCMP_CHANGEWINDOW)
               done = TRUE;
            ReplyMsg ((struct Message *)msg);
         }
      }
      ModifyIDCMP (main_win, win_idcmp);
      
      field_size (field, levels[level].rows, levels[level].columns,
                  levels[level].bombs);
      if (digital_display)
      {
         counter_move (time_counter,
                       main_win->Width - main_win->BorderRight -
                       LINEWIDTH - INTERWIDTH - COUNTERWIDTH,
                       main_win->BorderTop + LINEHEIGHT + INTERHEIGHT);
         counter_draw (time_counter);
      }
      DrawBevelBox (main_win->RPort, main_win->BorderLeft, main_win->BorderTop,
                    main_win->Width - main_win->BorderLeft -
                    main_win->BorderRight, main_win->Height -
                    main_win->BorderTop - main_win->BorderBottom,
                    GT_VisualInfo, vis_info,
                    TAG_DONE);
      
      current_level = level;
   }
   field_clear (field);
   field_reset (field);
   counter_update (flag_counter, levels[current_level].bombs);
   counter_update (time_counter, 0);
   playing = TRUE;
   time_on = FALSE;
   timer_stop (timer_obj);
}

BOOL
initialize (void)
{
   if (IntuitionBase = OpenLibrary ("intuition.library", 37L))
   {
      if (GfxBase = (struct GfxBase *)OpenLibrary ("graphics.library", 37L))
      {
         if (GadToolsBase = OpenLibrary ("gadtools.library", 37L))
            return init_display ();
         else
         {
            msg_requester (NULL, "Init Error", "OK",
                           "Couldn't open gadtools.library!");
         }
      }
      else
      {
         msg_requester (NULL, "Init Error", "OK",
                        "Couldn't open graphics.library!");
      }
   }
   else
      printf ("Couldn't open intuition.library!\n");
   
   return FALSE;
}

BOOL
init_display (void)
{
   BOOL    use_romfont;
   UWORD   win_w, win_h;
   
   strncpy (scr_title, PRG_NAME, 80);
   strncat (scr_title, " v", 80);
   strncat (scr_title, VERSION_NO, 80);
   strncat (scr_title, " - ©", 80);
   strncat (scr_title, CREATION_YEAR, 80);
   strncat (scr_title, " ", 80);
   strncat (scr_title, AUTHOR, 80);
   
   if ((pub_screen =
        LockPubScreen ((pubscr_name[0] == '\0') ? NULL : pubscr_name)) ||
       (pub_screen = LockPubScreen (NULL)))
   {
      if (draw_info = GetScreenDrawInfo (pub_screen))
      {
         gui_pens = draw_info->dri_Pens;
         init_pens (pub_screen);
         if (layout_display (pub_screen, &use_romfont))
         {
            if (use_romfont)
            {
               if (!(romfont = OpenFont (&topaz8)))
               {
                  msg_requester (NULL, "Init Error", "OK",
                                 "Couldn't open topaz.font!");
                  return FALSE;
               }
               window_extent (pub_screen, current_level, ROMFONT_WIDTH,
                              ROMFONT_HEIGHT, &win_w, &win_h);
            }
            else
            {
               window_extent (pub_screen, current_level,
                              GfxBase->DefaultFont->tf_XSize,
                              GfxBase->DefaultFont->tf_YSize, &win_w, &win_h);
            }
            if (vis_info = GetVisualInfo (pub_screen, TAG_DONE))
            {
               main_win = OpenWindowTags (NULL,
                           WA_Left, (pub_screen->Width - win_w) / 2,
                           WA_Top, (pub_screen->Height - win_h) / 2,
                           WA_Width, win_w,
                           WA_Height, win_h,
                           WA_AutoAdjust, FALSE,
                           WA_Activate, TRUE,
                           WA_CloseGadget, TRUE,
                           WA_DepthGadget, TRUE,
                           WA_DragBar, TRUE,
                           WA_Title, (digital_display) ?
                                     PRG_NAME : "    :    ",
                           WA_ScreenTitle, scr_title,
                           WA_PubScreen, pub_screen,
                           WA_NewLookMenus, TRUE,
                           WA_ReportMouse, TRUE,
                           WA_IDCMP, IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK |
                                     IDCMP_MOUSEMOVE |
                                     IDCMP_ACTIVEWINDOW | IDCMP_CLOSEWINDOW,
                           TAG_DONE);
               if (main_win)
               {
                  if (!init_menu ())
                     return FALSE;
                  
                  if (use_romfont)
                     SetFont (main_win->RPort, romfont);
                  
                  if (init_images (main_win))
                  {
                     if (digital_display)
                     {
                        flag_counter = counter_init (main_win,
                                                  main_win->BorderLeft +
                                                  LINEWIDTH + INTERWIDTH,
                                                  main_win->BorderTop +
                                                  LINEHEIGHT + INTERHEIGHT,
                                                  levels[current_level].bombs,
                                                  TRUE);
                        time_counter = counter_init (main_win,
                                                  main_win->Width -
                                                  main_win->BorderRight -
                                                  LINEWIDTH - INTERWIDTH -
                                                  COUNTERWIDTH,
                                                  main_win->BorderTop +
                                                  LINEHEIGHT + INTERHEIGHT,
                                                  0, TRUE);
                     }
                     else
                     {
                        flag_counter = counter_init (main_win, 0, 0,
                                                  levels[current_level].bombs,
                                                  FALSE);
                        time_counter = counter_init (main_win, 6, 0,
                                                  0, FALSE);
                     }
                     if (flag_counter && time_counter)
                     {
                        counter_draw (flag_counter);
                        counter_draw (time_counter);
                        
                        DrawBevelBox (main_win->RPort,
                                  main_win->BorderLeft, main_win->BorderTop,
                                  main_win->Width - main_win->BorderLeft -
                                  main_win->BorderRight, main_win->Height -
                                  main_win->BorderTop - main_win->BorderBottom,
                                  GT_VisualInfo, vis_info,
                                  TAG_DONE);
                     
                        if (field  = field_init (main_win->RPort,
                                                 main_win->BorderLeft +
                                                 LINEWIDTH + INTERWIDTH,
                                                 main_win->BorderTop +
                                                 LINEHEIGHT + INTERHEIGHT +
                                                 ((digital_display) ?
                                                 COUNTERHEIGHT + INTERHEIGHT :
                                                 0),
                                                 levels[current_level].rows,
                                                 levels[current_level].columns,
                                                 levels[current_level].bombs))
                        {
                           field_clear (field);
                           return (BOOL)(timer_obj = timer_create ());
                        }
                     }
                     else
                     {
                        msg_requester (NULL, "Init Error", "OK",
                                       "Couldn't create minefield!");
                     }
                  }
                  else
                  {
                     msg_requester (NULL, "Init Error", "OK",
                                    "Couldn't create counters!");
                  }
               }
               else
               {
                  msg_requester (NULL, "Init Error", "OK",
                                 "Couldn't open window!");
               }
            }
            else
            {
               msg_requester (NULL, "Init Error", "OK",
                              "Couldn't get VisualInfo!");
            }
         }
         else
         {
            msg_requester (NULL, "Init Error", "OK",
                           "Screen is too small!");
         }
      }
      else
      {
         msg_requester (NULL, "Init Error", "OK",
                        "Couldn't get DrawInfo!");
      }
   }
   else
   {
      msg_requester (NULL, "Init Error", "OK",
                     "Couldn't lock public screen!");
   }
   
   return FALSE;
}

BOOL
init_menu (void)
{
   if (main_menu = CreateLocMenus (new_menu, vis_info, TAG_DONE))
   {
      if (LayoutMenus (main_menu, vis_info,
                       GTMN_NewLookMenus, TRUE, TAG_DONE))
      {
         if (SetMenuStrip (main_win, main_menu))
            return TRUE;
         else
         {
            msg_requester (main_win, "Init Error", "OK",
                           "Couldn't set menustrip!");
         }
      }
      else
      {
         msg_requester (main_win, "Init Error", "OK",
                        "Couldn't layout menus!");
      }
   }
   else
   {
      msg_requester (main_win, "Init Error", "OK",
                     "Couldn't create menus!");
   }
   
   return FALSE;
}

void
finalize (void)
{
   finalize_display ();
   CloseLibrary (GadToolsBase);
   CloseLibrary ((struct Library  *)GfxBase);
   CloseLibrary (IntuitionBase);
}

void
finalize_display (void)
{
   if (timer_obj)
      timer_destroy (timer_obj);
   if (field)
      field_free (field);
   if (flag_counter)
      counter_free (flag_counter);
   if (time_counter)
      counter_free (time_counter);
   if (main_win)
   {
      finalize_images ();
      ClearMenuStrip (main_win);
      CloseWindow (main_win);
   }
   FreeMenus (main_menu);
   if (romfont)
      CloseFont (romfont);
   if (vis_info)
      FreeVisualInfo (vis_info);
   if (draw_info)
      FreeScreenDrawInfo (pub_screen, draw_info);
   if (pub_screen)
   {
      free_pens (pub_screen);
      UnlockPubScreen (NULL, pub_screen);
   }
}
