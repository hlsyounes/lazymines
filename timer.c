/*
 * timer.c
 * =======
 * Implementation of a timer.
 *
 * Copyright © 1994 Lorens Younes (d93-hyo@nada.kth.se)
 */

#include <exec/memory.h>
#include <devices/timer.h>
#include "timer.h"

#include <clib/exec_protos.h>


struct timer
{
   struct MsgPort      *mp;
   struct timerequest  *io;
   BOOL                 used;
};


timer_ptr
timer_create (void)
{
   timer_ptr   timer;
   
   if (timer = AllocVec (sizeof (*timer), MEMF_PUBLIC))
   {
      timer->mp = NULL;
      timer->io = NULL;
      timer->used = FALSE;
      if (timer->mp = CreateMsgPort ())
      {
         if (timer->io = (struct timerequest *)
                         CreateIORequest (timer->mp, sizeof (*timer->io)))
         {
            if (0 == OpenDevice (TIMERNAME, UNIT_VBLANK,
                                 (struct IORequest *)timer->io, 0L))
            {
               return timer;
            }
            DeleteIORequest (timer->io);
         }
         DeleteMsgPort (timer->mp);
      }
      FreeVec (timer);
   }
   
   return NULL;
}

void
timer_destroy (
   timer_ptr   timer)
{
   if (timer)
   {
      timer_stop (timer);
      CloseDevice ((struct IORequest *)timer->io);
      DeleteIORequest ((struct IORequest *)timer->io);
      DeleteMsgPort (timer->mp);
      FreeVec (timer);
   }
}

ULONG
timer_signal (
   timer_ptr   timer)
{
   return (ULONG)(1L << timer->mp->mp_SigBit);
}

void
timer_start (
   timer_ptr   timer,
   ULONG       secs,
   ULONG       micro)
{
   timer_stop (timer);
   timer->io->tr_node.io_Command = TR_ADDREQUEST;
   timer->io->tr_time.tv_secs = secs;
   timer->io->tr_time.tv_micro = micro;
   SendIO ((struct IORequest *)timer->io);
   timer->used = TRUE;
}

void
timer_continue (
   timer_ptr   timer,
   ULONG       secs,
   ULONG       micro)
{
   while (GetMsg (timer->mp))
      ;
   timer_start (timer, secs, micro);
}

void
timer_stop (
   timer_ptr   timer)
{
   if (timer->used)
   {
      AbortIO ((struct IORequest *)timer->io);
      WaitIO ((struct IORequest *)timer->io);
   }
}
