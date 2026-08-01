/*
 * highscores.c
 * ============
 * Handles highscores.
 *
 * Copyright © 1994 Lorens Younes (d93-hyo@nada.kth.se)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include "requesters.h"
#include "localize.h"
#include "layout_const.h"
#include "highscores.h"

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>


#define NUM_SCORES   10


extern struct Window  *main_win;
extern APTR   vis_info;
extern UWORD  *gui_pens;


static char    names[NUM_SCORES][3][31];
static UWORD   scores[NUM_SCORES][3];
static BOOL    need_save = FALSE;


void
load_high_scores (
   char  *default_name)
{
   register UWORD   i = 0, j = 0, k = 0, m;
   FILE  *score_file;
   int   ch;
   char   num_str[4];
   UBYTE   status = 0;
   
   if (score_file = fopen ("lazymines.hiscore", "r"))
   {
      while ((ch = fgetc (score_file)) != EOF && k < NUM_SCORES)
      {
         if (status == 0)
         {
            if (ch == '\n')
            {
               names[k][j][i] = '\0';
               ++status;
               i = 0;
            }
            else
            {
               names[k][j][i] = ch;
               ++i;
            }
         }
         else
         {
            if (ch == '\n')
            {
               num_str[i] = '\0';
               scores[k][j] = atoi (num_str);
               ++status;
               if (status > 1)
               {
                  status = 0;
                  ++j;
                  if (j > 2)
                  {
                     j = 0;
                     ++k;
                  }
               }
               i = 0;
            }
            else
            {
               num_str[i] = ch;
               ++i;
            }
         }
      }
      fclose (score_file);
   }
   
   for (m = k; m < NUM_SCORES; ++m)
   {
      for (i = j; i < 3; ++i)
      {
         switch (status)
         {
         case 0:
            strncpy (names[m][i], default_name, 30);
         case 1:
            scores[m][i] = 999;
            status = 0;
            break;
         }
      }
   }
}

void
save_high_scores (void)
{
   register UBYTE   i, j;
   FILE *score_file;
   
   if (need_save)
   {
      if (score_file = fopen ("lazymines.hiscore", "w"))
      {
         for (j = 0; j < NUM_SCORES; ++j)
         {
            for (i = 0; i < 3; ++i)
            {
               fputs (names[j][i], score_file);
               fputc ('\n', score_file);
               fprintf (score_file, "%d\n", scores[j][i]);
            }
         }
         fclose (score_file);
         SetProtection ("lazymines.hiscore", 2);
      }
   }
}

BOOL
update_high_score (
   UBYTE   game,
   UWORD   score)
{
   char   name[31];
   BYTE   n = NUM_SCORES - 1;
   BOOL   get_name = TRUE;
   
   name[0] = '\0';
   
   while (n >= 0)
   {
      if (score <= scores[n][game])
      {
         need_save = TRUE;
         
         if (n < NUM_SCORES - 1)
         {
            strcpy (names[n + 1][game], names[n][game]);
            scores[n + 1][game] = scores[n][game];
         }
         
         if (get_name)
         {
            string_requester (main_win, vis_info,
                              localized_string (MSG_NAME_REQTITLE),
                              localized_string (MSG_NAME_GAD),
                              name, 30);
            get_name = FALSE;
         }
         
         strcpy (names[n][game], name);
         scores[n][game] = score;
      }
      --n;
   }
   
   return (BOOL)(!get_name);
}

void
display_high_scores (
   UBYTE   game)
{
   register UBYTE   n;
   struct Window  *win;
   struct IntuiMessage  *msg;
   BOOL   done = FALSE;
   struct Requester   req;
   BOOL   win_sleep = FALSE;
   char   text_buf[81];
   UWORD   width, height;
   WORD    left, top;
   
   win_sleep = window_sleep (main_win, &req);
   width = main_win->BorderLeft + main_win->BorderRight +
           34 * main_win->RPort->TxWidth + 2 * INTERWIDTH;
   height = main_win->BorderTop + main_win->BorderBottom +
            NUM_SCORES * main_win->RPort->TxHeight + 2 * INTERHEIGHT;
   left = main_win->LeftEdge + (main_win->Width - width) / 2;
   top = main_win->TopEdge + (main_win->Height - height) / 2;
   win = OpenWindowTags (NULL,
                         WA_Left, (width > main_win->WScreen->Width - left) ?
                                  main_win->WScreen->Width - width : left,
                         WA_Top, (height > main_win->WScreen->Height - top) ?
                                 main_win->WScreen->Height - height : top,
                         WA_Width, width,
                         WA_Height, height,
                         WA_AutoAdjust, FALSE,
                         WA_Title, localized_string (MSG_HIGHSCORE_REQTITLE),
                         WA_ScreenTitle, main_win->ScreenTitle,
                         WA_PubScreen, main_win->WScreen,
                         WA_IDCMP, IDCMP_MOUSEBUTTONS,
                         WA_DragBar, TRUE,
                         WA_DepthGadget, TRUE,
                         WA_Activate, TRUE,
                         TAG_DONE);
   if (win != NULL)
   {
      SetAPen (win->RPort, gui_pens[TEXTPEN]);
      for (n = 0; n < NUM_SCORES; ++n)
      {
         Move (win->RPort, win->BorderLeft + INTERWIDTH,
               win->BorderTop + INTERHEIGHT + n * win->RPort->TxHeight +
               win->RPort->TxBaseline);
         sprintf (text_buf, "%-30s %3d", names[n][game], scores[n][game]);
         Text (win->RPort, text_buf, strlen (text_buf));
      }
      
      while (!done)
      {
         WaitPort (win->UserPort);
         while (msg = (struct IntuiMessage *)GetMsg (win->UserPort))
         {
            done = (msg->Class == IDCMP_MOUSEBUTTONS &&
                    msg->Code == SELECTDOWN);
            ReplyMsg ((struct Message *)msg);
         }
      }
      CloseWindow (win);
   }
   if (win_sleep)
      window_wakeup (main_win, &req);
}
