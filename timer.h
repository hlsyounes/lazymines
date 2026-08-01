#ifndef TIMER_H
#define TIMER_H
/*
 * timer.h
 * =======
 * Interface to timer.
 *
 * Copyright © 1994 Lorens Younes (d93-hyo@nada.kth.se)
 */

#include <exec/types.h>


typedef struct timer  *timer_ptr;


timer_ptr
timer_create (void);

void
timer_destroy (
   timer_ptr   timer);

ULONG
timer_signal (
   timer_ptr   timer);

void
timer_start (
   timer_ptr   timer,
   ULONG       secs,
   ULONG       micro);

void
timer_continue (
   timer_ptr   timer,
   ULONG       secs,
   ULONG       micro);

void
timer_stop (
   timer_ptr   timer);

#endif
