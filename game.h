#ifndef GAME_H
#define GAME_H

/*
 * game.h
 * ======
 * Interface to game module.
 */

#include <exec/types.h>
#include "counter.h"

struct level {
   UWORD   columns;
   UWORD   rows;
   UWORD   bombs;
};

#define NOVICE_LEVEL     0
#define AMATURE_LEVEL    1
#define EXPERT_LEVEL     2
#define OPTIONAL_LEVEL   3

extern struct level   levels[];

extern BOOL   place_warnings;
extern BOOL   safe_opening;
extern BOOL   auto_lock;

extern counter_ptr   time_counter;
extern counter_ptr   flag_counter;

#endif
