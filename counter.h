#ifndef COUNTER_H
#define COUNTER_H

/*
 * counter.h
 * =========
 * Interface to digital counter.
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include "images.h"


#define COUNTERWIDTH    (3 * DIGITIMAGE_WIDTH + 6 * LINEWIDTH)
#define COUNTERHEIGHT   (DIGITIMAGE_HEIGHT + 4 * LINEHEIGHT)


typedef struct counter  *counter_ptr;


counter_ptr
counter_init (
   struct Window  *win,
   WORD    left,
   WORD    top,
   UWORD   value,
   BOOL    digital);

void
counter_free (
   counter_ptr   counter);

UWORD
counter_value (
   counter_ptr   counter);

void
counter_move (
   counter_ptr   counter,
   WORD          left,
   WORD          top);

void
counter_draw (
   counter_ptr       counter);

void
counter_delete (
   counter_ptr       counter);

void
counter_update (
   counter_ptr       counter,
   UWORD             value);

#endif
