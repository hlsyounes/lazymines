#ifndef HIGHSCORES_H
#define HIGHSCORES_H

/* highscores.h
 * ============
 * Interface to highscore handling.
 *
 * Copyright © 1994 Lorens Younes (d93-hyo@nada.kth.se)
 */

#include <exec/types.h>


void
load_high_scores (
   char  *default_name);

void
save_high_scores (void);

BOOL
update_high_score (
   UWORD   score);

void
display_high_scores (void);

#endif
