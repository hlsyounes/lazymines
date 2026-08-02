/*
 * Utility functions for generating pseudo-random numbers.
 *
 * Copyright (C) 2026 Haakan Younes
 * SPDX-License-Identifier: MIT
 */

#ifndef UTIL_RANDOM_H
#define UTIL_RANDOM_H

#include <exec/types.h>

/*
 * Sets the seed for the random number generator.
 */
void RandSeed(ULONG seed);

/*
 * Seeds the random number generator from system time.
 */
void RandSeedFromSysTime(void);

/*
 * Returns a pseudo-random number in the range [min, max].
 */
WORD RandUniform(WORD min, WORD max);

#endif  /* UTIL_RANDOM_H */
