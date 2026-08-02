/*
 * Copyright (C) 2026 Haakan Younes
 * SPDX-License-Identifier: MIT
 */

#include "timer.h"

#include <devices/timer.h>
#include <exec/types.h>
#include <proto/exec.h>

/*
 * Deletes the given timer, which should have been created with CreateTimer().
 * Does noting if timer is NULL.
 */
static void DeleteTimer(struct timerequest *timer) {
  if (timer == NULL) {
    return;
  }

  DeleteMsgPort(timer->tr_node.io_Message.mn_ReplyPort);
  CloseDevice((struct IORequest *)timer);
  DeleteExtIO((struct IORequest *)timer);
}

/*
 * Creates a timer with the given timer device unit. Returns NULL of failure.
 */
static struct timerequest *CreateTimer(ULONG unit) {
  struct MsgPort *port;
  struct timerequest *timer;
  BYTE error;

  if ((port = CreateMsgPort()) == NULL) {
    return NULL;
  }

  timer = (struct timerequest *)CreateExtIO(port, sizeof *timer);
  if (timer == NULL) {
    DeleteMsgPort(port);
    return NULL;
  }

  error = OpenDevice(TIMERNAME, unit, (struct IORequest *)timer, /*flags=*/0);
  if (error != 0) {
    DeleteTimer(timer);
    return NULL;
  }

  return timer;
}

void GetSysTime(struct timeval *sys_time) {
  struct timerequest *timer;

  if (sys_time == NULL) {
    return;
  }

  if ((timer = CreateTimer(UNIT_MICROHZ)) == NULL) {
    return;
  }

  timer->tr_node.io_Command = TR_GETSYSTIME;
  if (DoIO((struct IORequest *)timer) == 0) {
    *sys_time = timer->tr_time;
  }

  DeleteTimer(timer);
}
