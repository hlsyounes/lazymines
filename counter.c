/*
 * counter.c
 * =========
 * Implements digital counter.
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <stdio.h>
#include "layout.h"
#include "images.h"
#include "counter.h"

#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>


struct counter {
   struct Window  *win;
   WORD            left;
   WORD            top;
   UWORD           value;
   BOOL            digital;
};


extern APTR   vis_info;


counter_ptr
counter_init (
   struct Window  *win,
   WORD            left,
   WORD            top,
   UWORD           value,
   BOOL            digital)
{
   counter_ptr   counter;
   
   if (counter = AllocVec (sizeof (*counter), MEMF_PUBLIC))
   {
      counter->win = win;
      counter->left = left;
      counter->top = top;
      counter->value = value;
      counter->digital = digital;
   }
   
   return counter;
}

void
counter_free (
   counter_ptr   counter)
{
   if (counter)
      FreeVec (counter);
}

UWORD
counter_value (
   counter_ptr   counter)
{
   return counter->value;
}

void
counter_move (
   counter_ptr   counter,
   WORD          left,
   WORD          top)
{
   counter->left = left;
   counter->top = top;
}

void
counter_draw (
   counter_ptr       counter)
{
   if (counter->digital)
   {
      DrawBevelBox (counter->win->RPort, counter->left, counter->top,
                    3 * DIGITIMAGE_WIDTH + 6 * LINEWIDTH,
                    DIGITIMAGE_HEIGHT + 4 * LINEHEIGHT,
                    GT_VisualInfo, vis_info,
                    GTBB_Recessed, TRUE,
                    TAG_DONE);
      SetAPen (counter->win->RPort, (game_pens[DIGITBACKGROUNDPEN] = NOPEN) ?
                   gui_pens[TEXTPEN] : game_pens[DIGITBACKGROUNDPEN]);
      RectFill (counter->win->RPort, counter->left + LINEWIDTH,
                counter->top + LINEHEIGHT,
                counter->left + 3 * DIGITIMAGE_WIDTH + 5 * LINEWIDTH - 1,
                counter->top + DIGITIMAGE_HEIGHT + 3 * LINEHEIGHT - 1);
   }
   counter_update (counter, counter->value);
}

void
counter_delete (
   counter_ptr       counter)
{
   if (counter->digital)
   {
      SetAPen (counter->win->RPort, gui_pens[BACKGROUNDPEN]);
      RectFill (counter->win->RPort, counter->left, counter->top,
                counter->left + 3 * DIGITIMAGE_WIDTH + 6 * LINEWIDTH - 1,
                counter->top + DIGITIMAGE_HEIGHT + 4 * LINEHEIGHT - 1);
   }
}

void
counter_update (
   counter_ptr       counter,
   UWORD             value)
{
   char   value_str[4];
   register UBYTE   i;
   
   counter->value = (value > 999) ? 999 : value;
   sprintf (value_str, "%3d", counter->value);
   for (i = 0; i < 3; ++i)
   {
      if (counter->digital)
      {
         draw_image (counter->win->RPort,
               counter->left + 2 * LINEWIDTH +
               i * (DIGITIMAGE_WIDTH + LINEWIDTH),
               counter->top + 2 * LINEHEIGHT,
               DIGITIMAGE_WIDTH, DIGITIMAGE_HEIGHT,
               (value_str[i] == ' ') ?
               DIGITIMAGE (10) : DIGITIMAGE (value_str[i] - '0'));
      }
      else
      {
         counter->win->Title[counter->left + i] = value_str[i];
         SetWindowTitles (counter->win, counter->win->Title, (UBYTE *)(~0));
      }
   }
}
