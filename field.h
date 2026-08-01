#ifndef FIELD_H
#define FIELD_H
/*
 * field.h
 * =======
 * Interface to minefield.
 *
 * Copyright © 1994-1995 Lorens Younes (d93-hyo@nada.kth.se)
 */

#include <exec/types.h>
#include <graphics/rastport.h>


typedef struct field  *field_ptr;


field_ptr
field_init (
   struct RastPort  *rp,
   WORD              left,
   WORD              top,
   UBYTE             rows,
   UBYTE             columns,
   UWORD             mines);

void
field_free (
   field_ptr   field);

WORD
field_left (
   field_ptr   field);

WORD
field_top (
   field_ptr   field);

BOOL
field_size (
   field_ptr   field,
   UBYTE       rows,
   UBYTE       columns,
   UWORD       mines);

BOOL
field_inside (
   field_ptr   field,
   WORD        row,
   WORD        column);

BOOL
field_reset (
   field_ptr   field);

void
field_draw (
   field_ptr   field);

void
field_delete (
   field_ptr   field);

BOOL
reveal_this (
   field_ptr   field,
   UBYTE       row,
   UBYTE       col);

BOOL
sweep_this (
   field_ptr   field,
   UBYTE       row,
   UBYTE       col);

void
toggle_lock (
   field_ptr   field,
   UBYTE       row,
   UBYTE       col);

void
press_this (
   field_ptr   field,
   UBYTE       row,
   UBYTE       col);

void
press_around (
   field_ptr   field,
   UBYTE       row,
   UBYTE       col);

void
release_this (
   field_ptr   field,
   UBYTE       row,
   UBYTE       col);

void
release_around (
   field_ptr   field,
   UBYTE       row,
   UBYTE       col);

BOOL
field_swept (
   field_ptr   field);

void
field_win (
   field_ptr   field);

void
field_lose (
   field_ptr   field);

#endif
