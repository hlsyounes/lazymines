/*
 * field.c
 * =======
 * Implements the minefield.
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <libraries/gadtools.h>
#include <math.h>
#include <string.h>
#include "counter.h"
#include "layout.h"
#include "images.h"
#include "game.h"
#include "field.h"

#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>


struct field {
   struct RastPort  *rp;
   WORD              left;
   WORD              top;
   UBYTE             rows;
   UBYTE             columns;
   UWORD             bombs;
   UBYTE            *data;
   UWORD             num_swept;
};


#define BOMB     0x10
#define LOCKED   0x20
#define WARNED   0x40
#define SWEPT    0x80

#define FIELD(f, r, c) ((f)->data[(r) * (f)->columns + (c)])

#define SET_FIELD(f, r, c, v) (FIELD ((f), (r), (c)) = (v))

#define CELL_VAL(f, r, c) (FIELD ((f), (r), (c)) & 0x0F)

#define IS_BOMB(f, r, c) (FIELD ((f), (r), (c)) & BOMB)

#define IS_LOCKED(f, r, c) (FIELD ((f), (r), (c)) & LOCKED)

#define IS_WARNED(f, r, c) (FIELD ((f), (r), (c)) & WARNED)

#define IS_SWEPT(f, r, c) (FIELD ((f), (r), (c)) & SWEPT)

#define PLACE_BOMB(f, r, c) (FIELD ((f), (r), (c)) |= BOMB)

#define LOCK_CELL(f, r, c) (FIELD ((f), (r), (c)) |= LOCKED)

#define WARN_CELL(f, r, c) (FIELD ((f), (r), (c)) |= WARNED)

#define UNLOCK_CELL(f, r, c) (FIELD ((f), (r), (c)) &= ~(LOCKED | WARNED))

#define SWEEP_CELL(f, r, c) (FIELD ((f), (r), (c)) |= SWEPT)

#define FIELDWIDTH(f) ((f)->columns * cell_w + 2 * LINEWIDTH)
#define FIELDHEIGHT(f) ((f)->rows * cell_h + 2 * LINEHEIGHT)


extern APTR   vis_info;


static UBYTE
count_neighbors (
   field_ptr   field,
   WORD        row,
   WORD        col,
   UBYTE       filter)
{
   register WORD    r, c;
   register UBYTE   count = 0;
   
   for (r = row - 1; r <= row + 1; ++r)
   {
      for (c = col - 1; c <= col + 1; ++c)
      {
         if (field_inside (field, r, c) && !(r == row && c == col) &&
             FIELD (field, r, c) & filter)
         {
            ++count;
         }
      }
   }
   
   return count;
}

static BOOL
reveal_around (
   field_ptr   field,
   WORD        row,
   WORD        col)
{
   register WORD   r, c;
   register BOOL   success = TRUE;
   
   for (r = row - 1; r <= row + 1 && success; ++r)
      for (c = col - 1; c <= col + 1 && success; ++c)
         if (!(r == row && c == col))
            success = reveal_this (field, r, c);
   
   return success;
}

static void
draw_box(
   struct RastPort  *rp,
   WORD              left,
   WORD              top,
   BOOL              recessed)
{
   if (recessed)
   {
      SetAPen (rp, gui_pens[SHADOWPEN]);
      Move (rp, left, top);
      Draw (rp, left + cell_w - 1, top);
      Draw (rp, left + cell_w - 1, top + cell_h - 1);
      Draw (rp, left, top + cell_h - 1);
      Draw (rp, left, top);
      SetAPen (rp, gui_pens[BACKGROUNDPEN]);
      Move (rp, left + 1, top + 1);
      Draw (rp, left + 1, top + cell_h - 2);
      Move (rp, left + cell_w - 2, top + 1);
      Draw (rp, left + cell_w - 2, top + cell_h - 2);
   }
   else
   {
      SetAPen(rp, gui_pens[SHINEPEN]);
      Move (rp, left + cell_w - 2, top);
      Draw (rp, left, top);
      Draw (rp, left, top + cell_h - 1);
      Move (rp, left + 1, top + cell_h - 2);
      Draw (rp, left + 1, top + 1);
      SetAPen(rp, gui_pens[SHADOWPEN]);
      Move (rp, left + 1, top + cell_h - 1);
      Draw (rp, left + cell_w - 1, top + cell_h - 1);
      Draw (rp, left + cell_w - 1, top);
      Move (rp, left + cell_w - 2, top + 1);
      Draw (rp, left + cell_w - 2, top + cell_h - 2);
   }
}

static void
draw_cell (
   struct RastPort  *rp,
   WORD              left,
   WORD              top,
   UBYTE             value)
{
   draw_box (rp, left, top, value & SWEPT);
   SetAPen (rp, gui_pens[BACKGROUNDPEN]);
   RectFill (rp, left + LINEWIDTH, top + LINEHEIGHT,
             left + cell_w - LINEWIDTH - 1, top + cell_h - LINEHEIGHT - 1);
   if (value & SWEPT)
   {
      if (value & BOMB)
      {
         draw_image (rp, left + (cell_w - BOMBIMAGE_WIDTH) / 2,
                     top + (cell_h - BOMBIMAGE_HEIGHT) / 2,
                     BOMBIMAGE_WIDTH, BOMBIMAGE_HEIGHT, bombimage);
      }
      else
      {
         value &= 0x0F;
         if (value > 0)
         {
            char   ch = value + '0';
            
            SetAPen (rp, (game_pens[value - 1] != -1) ?
                     game_pens[value - 1] : gui_pens[TEXTPEN]);
            Move (rp, left + (cell_w - rp->TxWidth) / 2,
                  top + (cell_h - rp->TxHeight) / 2 + rp->TxBaseline);
            Text (rp, &ch, 1);
         }
      }
   }
   else if (value & LOCKED)
   {
      if (value & WARNED)
      {
         SetAPen (rp, gui_pens[TEXTPEN]);
         Move (rp, left + (cell_w - rp->TxWidth) / 2,
               top + (cell_h - rp->TxHeight) / 2 + rp->TxBaseline);
         Text (rp, "?", 1);
      }
      else
      {
         draw_image (rp, left + (cell_w - FLAGIMAGE_WIDTH) / 2,
                     top + (cell_h - FLAGIMAGE_HEIGHT) / 2,
                     FLAGIMAGE_WIDTH, FLAGIMAGE_WIDTH, flagimage);
      }
   }
}

static void
mutate_neighbors (
   field_ptr   field,
   WORD        row,
   WORD        col,
   UBYTE       filter,
   UBYTE       mask)
{
   register WORD   r, c;
   
   for (r = row - 1; r <= row + 1; ++r)
   {
      for (c = col - 1; c <= col + 1; ++c)
      {
         if (field_inside (field, r, c) && !(r == row && r == col) &&
             FIELD (field, r, c) & filter)
         {
            FIELD (field, r, c) &= ~mask;
            draw_cell (field->rp, field->left + LINEWIDTH + c * cell_w,
                       field->top + LINEHEIGHT + r * cell_h,
                       FIELD (field, r, c));
         }
      }
   }
}

field_ptr
field_init (
   struct RastPort  *rp,
   WORD              left,
   WORD              top,
   UBYTE             rows,
   UBYTE             columns,
   UWORD             bombs)
{
   field_ptr   field;
   
   if (field = AllocVec (sizeof (*field), MEMF_PUBLIC))
   {
      field->rp = rp;
      field->left = left;
      field->top = top;
      field->rows = rows;
      field->columns = columns;
      field->bombs = bombs;
      if (!(field->data = AllocVec (field->rows * field->columns *
                                    sizeof (*field->data), MEMF_PUBLIC)))
      {
         field_free (field);
      }
   }
   
   return field;
}

void
field_free (
   field_ptr   field)
{
   if (field != NULL)
   {
      if (field->data != NULL)
         FreeVec (field->data);
      
      FreeVec (field);
   }
}

WORD
field_left (
   field_ptr   field)
{
   return field->left;
}

WORD
field_top (
   field_ptr   field)
{
   return field->top;
}

BOOL
field_swept (
   field_ptr   field)
{
   return (BOOL)(field->rows * field->columns - field->bombs ==
                 field->num_swept);
}

void
field_win (
   field_ptr   field)
{
   register UBYTE   r, c;
   
   for (r = 0; r < field->rows; ++r)
   {
      for (c = 0; c < field->columns; ++c)
      {
         if (IS_BOMB (field, r, c) && !IS_LOCKED (field, r, c))
         {
            LOCK_CELL (field, r, c);
            draw_cell (field->rp, field->left + LINEWIDTH + c * cell_w,
                       field->top + LINEHEIGHT + r * cell_h,
                       FIELD (field, r, c));
         }
      }
   }
   counter_update (flag_counter, 0);
}

void
field_lose (
   field_ptr   field)
{
   register UBYTE   r, c;
   
   for (r = 0; r < field->rows; ++r)
   {
      for (c = 0; c < field->columns; ++c)
      {
         if (IS_BOMB (field, r, c))
         {
            SWEEP_CELL (field, r, c);
            draw_cell (field->rp, field->left + LINEWIDTH + c * cell_w,
                       field->top + LINEHEIGHT + r * cell_h,
                       FIELD (field, r, c));
         }
      }
   }
}

void
field_move (
   field_ptr   field,
   WORD        left,
   WORD        top)
{
   field->left = left;
   field->top = top;
}

BOOL
field_size (
   field_ptr   field,
   UBYTE       rows,
   UBYTE       columns,
   UWORD       bombs)
{
   if (rows != field->rows || columns != field->columns && field->data != NULL)
   {
      FreeVec (field->data);
      if (!(field->data = AllocVec (rows * columns *
                                    sizeof (*field->data), MEMF_PUBLIC)))
      {
         return FALSE;
      }
   }
   field->rows = rows;
   field->columns = columns;
   field->bombs = bombs;
}

BOOL
field_inside (
   field_ptr   field,
   WORD        r,
   WORD        c)
{
   return (BOOL)(r >= 0 && r < field->rows && c >= 0 && c < field->columns);
}

void
field_reset (
   field_ptr   field)
{
   register UBYTE   r, c;
   register ULONG   n = field->bombs;
   
   field->num_swept = 0;
   
   memset (field->data, 0,
           field->rows * field->columns * sizeof (*(field->data)));
   
   while (n > 0)
   {
      r = drand48 () * field->rows;
      c = drand48 () * field->columns;
      if (!IS_BOMB (field, r, c))
      {
         PLACE_BOMB (field, r, c);
         --n;
      }
   }
   
   for (r = 0; r < field->rows; ++r)
      for (c = 0; c < field->columns; ++c)
         if (!IS_BOMB (field, r, c))
            SET_FIELD (field, r, c, count_neighbors (field, r, c, BOMB));
}

void
field_clear (
   field_ptr   field)
{
   register UBYTE   r, c;
   
   DrawBevelBox (field->rp, field->left, field->top,
                 FIELDWIDTH (field), FIELDHEIGHT (field),
                 GT_VisualInfo, vis_info,
                 GTBB_Recessed, TRUE,
                 TAG_DONE);
   
   SetAPen (field->rp, gui_pens[BACKGROUNDPEN]);
   RectFill (field->rp, field->left + LINEWIDTH, field->top + LINEHEIGHT,
             field->left + LINEWIDTH + cell_w - 1,
             field->top + LINEHEIGHT + cell_h - 1);
   draw_box (field->rp, field->left + LINEWIDTH, field->top + LINEHEIGHT, FALSE);
   
   for (r = 0; r < field->rows; ++r)
   {
      for (c = 0; c < field->columns; ++c)
      {
         if (r || c)
         {
            ClipBlit (field->rp, field->left + LINEWIDTH,
                      field->top + LINEHEIGHT,
                      field->rp, field->left + LINEWIDTH + c * cell_w,
                      field->top + LINEHEIGHT + r * cell_h,
                      cell_w, cell_h, 0x00C0);
         }
      }
   }
}

void
field_delete (
   field_ptr   field)
{
   SetAPen (field->rp, gui_pens[BACKGROUNDPEN]);
   RectFill (field->rp, field->left, field->top,
             field->left + FIELDWIDTH (field) - 1,
             field->top + FIELDHEIGHT (field) - 1);
}

BOOL
reveal_this (
   field_ptr   field,
   WORD        row,
   WORD        col)
{
   if (field_inside (field, row, col) &&
       !IS_SWEPT (field, row, col) && !IS_LOCKED (field, row, col))
   {
      if (IS_BOMB (field, row, col))
      {
         if (field->num_swept == 0 && safe_opening)
         {
            release_this (field, row, col);
            return TRUE;
         }
         else
         {
            SWEEP_CELL (field, row, col);
            draw_cell (field->rp, field->left + LINEWIDTH + col * cell_w,
                       field->top + LINEHEIGHT + row * cell_h,
                       FIELD (field, row, col));
            
            return FALSE;
         }
      }
      SWEEP_CELL (field, row, col);
      ++field->num_swept;
      draw_cell (field->rp, field->left + LINEWIDTH + col * cell_w,
                 field->top + LINEHEIGHT + row * cell_h,
                 FIELD (field, row, col));
      if (CELL_VAL (field, row, col) == 0)
         reveal_around (field, row, col);
   }
   
   return TRUE;
}

BOOL
sweep_this (
   field_ptr   field,
   WORD        row,
   WORD        col)
{
   register WORD    r, c;
   register UBYTE   lock_count = 0;
   register UBYTE   warn_count = 0;
   register UBYTE   free_count = 0;
   register BOOL    success = TRUE;
   
   if (field_inside (field, row, col) && IS_SWEPT (field, row, col))
   {
      for (r = row - 1; r <= row + 1; ++r)
      {
         for (c = col - 1; c <= col + 1; ++c)
         {
            if (field_inside (field, r, c) && !(r == row && c == col))
            {
               if (IS_WARNED (field, r, c))
                  ++warn_count;
               else if (IS_LOCKED (field, r, c))
                  ++lock_count;
               else if (!IS_SWEPT (field, r, c))
                  ++free_count;
            }
         }
      }
      
      if (lock_count >= CELL_VAL (field, row, col))
      {
         if (warn_count)
            mutate_neighbors (field, row, col, WARNED, WARNED | LOCKED);
         success = reveal_around (field, row, col);
      }
      else if (free_count + warn_count + lock_count ==
               CELL_VAL (field, row, col))
      {
         if (auto_lock)
         {
            if (warn_count)
               mutate_neighbors (field, row, col, WARNED, WARNED | LOCKED);
            for (r = row - 1; r <= row + 1; ++r)
            {
               for (c = col - 1; c <= col + 1; ++c)
               {
                  if (field_inside (field, r, c) && !(r == row && c == col) &&
                      !(IS_SWEPT (field, r, c) || IS_LOCKED (field, r, c)))
                  {
                     toggle_lock (field, r, c);
                  }
               }
            }
         }
      }
   }
   release_around (field, row, col);
   
   return success;
}

void
toggle_lock (
   field_ptr   field,
   WORD        row,
   WORD        col)
{
   if (field_inside (field, row, col) && !IS_SWEPT (field, row, col))
   {
      if (IS_LOCKED (field, row, col))
      {
         if (IS_WARNED (field, row, col))
            UNLOCK_CELL (field, row, col);
         else
         {
            if (place_warnings)
               WARN_CELL (field, row, col);
            else
               UNLOCK_CELL (field, row, col);
            counter_update (flag_counter, counter_value (flag_counter) + 1);
         }
         draw_cell (field->rp, field->left + LINEWIDTH + col * cell_w,
                    field->top + LINEHEIGHT + row * cell_h,
                    FIELD (field, row, col));
      }
      else if (counter_value (flag_counter) > 0)
      {
         LOCK_CELL (field, row, col);
         draw_cell (field->rp, field->left + LINEWIDTH + col * cell_w,
                    field->top + LINEHEIGHT + row * cell_h,
                    FIELD (field, row, col));
         counter_update (flag_counter, counter_value (flag_counter) - 1);
      }
   }
}

void
press_this (
   field_ptr   field,
   WORD        row,
   WORD        col)
{
   if (field_inside (field, row, col) && 
       !(IS_SWEPT (field, row, col) || IS_LOCKED (field, row, col)))
   {
      draw_box (field->rp, field->left + LINEWIDTH + col * cell_w,
                field->top + LINEHEIGHT + row * cell_h,
                TRUE);
   }
}

void
press_around (
   field_ptr   field,
   WORD        row,
   WORD        col)
{
   register WORD   r, c;
   
   if (field_inside (field, row, col) && IS_SWEPT (field, row, col))
      for (r = row - 1; r <= row + 1; ++r)
         for (c = col - 1; c <= col + 1; ++c)
            if (!(r == row && c == col))
               press_this (field, r, c);
}

void
release_this (
   field_ptr   field,
   WORD        row,
   WORD        col)
{
   if (field_inside (field, row, col) &&
       !(IS_SWEPT (field, row, col) || IS_LOCKED (field, row, col)))
   {
      draw_box (field->rp, field->left + LINEWIDTH + col * cell_w,
                field->top + LINEHEIGHT + row * cell_h,
                FALSE);
   }
}

void
release_around (
   field_ptr   field,
   WORD        row,
   WORD        col)
{
   register WORD   r, c;
   
   if (field_inside (field, row, col) && IS_SWEPT (field, row, col))
      for (r = row - 1; r <= row + 1; ++r)
         for (c = col - 1; c <= col + 1; ++c)
            if (!(r == row && c == col))
               release_this (field, r, c);
}
