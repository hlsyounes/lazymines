/*
 * Copyright (C) 2026 Haakan Younes
 * SPDX-License-Identifier: MIT
 */

#include "random.h"

#include "timer.h"

static ULONG next = 1;

/*
 * Generates the next pseudo-random number.
 */
static ULONG Rand(void) {
  /*
   * This implementation is based on the POSIX.1-2008 (IEEE Std 1003.1-2008)
   * specification for Linear Congruential Generators (LCGs).  Specification
   * details: https://opengroup.org
   */
  next = next * 1103515245UL + 12345UL;
  return (next >> 16) & 0x7FFF;
}

void RandSeed(ULONG seed) {
  next = seed;
}

void RandSeedFromSysTime(void) {
  struct timeval sys_time = {0};

  /*
   * Seed LCG via bit-mixed system time.
   *
   * Shifts secs by a prime (<<7) to scramble slow-moving upper bits, then XORs
   * with raw micro to drive the critical lower 31 bits.  Prevents time-of-day
   * synchronization and ensures safe 32-bit overflow.
   */
  GetSysTime(&sys_time);
  next = (sys_time.tv_secs ^ (sys_time.tv_secs << 7)) ^ sys_time.tv_micro;
}

WORD RandUniform(WORD min, WORD max) {
  return (WORD)(((Rand() * (max - min + 1UL)) >> 15) + min);
}
