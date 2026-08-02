/*
 * Utility functions for working with the timer device.
 *
 * Copyright (C) 2026 Haakan Younes
 * SPDX-License-Identifier: MIT
 */

#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

#include <devices/timer.h>

/*
 * Fills sys_time with the system time. Does nothing if sys_time is NULL or
 * there is an error readin system time from the timer device.
 */
void GetSysTime(struct timeval *sys_time);

#endif  /* UTIL_TIMER_H */
