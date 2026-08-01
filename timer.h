#ifndef TIMER_H
#define TIMER_H
/*
 * timer.h
 * =======
 * Interface to timer.
 *
 * Copyright © 1994-1995 Lorens Younes (d93-hyo@nada.kth.se)
 */

#include <exec/types.h>


/* Timer object */
typedef struct timer  *timer_ptr;


/* Creates a new timer object */
timer_ptr              /* created timer */
timer_create (void);

/* Frees a timer object */
void
timer_free (
   timer_ptr   timer);   /* timer to free */

/* Calculates the signal of a timer */
ULONG                    /* calculated signal */
timer_signal (
   timer_ptr   timer);   /* timer to calculate signal for */

/* Starts a timer */
void
timer_start (
   timer_ptr   timer,    /* timer to start */
   ULONG       secs,     /* number of seconds timer will count */
   ULONG       micro);   /* number of microseconds the timer will count */

/* Replies a timer */
void
timer_reply (
   timer_ptr   timer);   /* timer to reply */

/* Stops a timer */
void
timer_stop (
   timer_ptr   timer);   /* timer to stop */

#endif
