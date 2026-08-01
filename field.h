#ifndef FIELD_H
#define FIELD_H
/*
 * field.h
 * =======
 * Gränssnitt till minfältet.
 *
 * Copyright (C) 1994 Lorens Younes (d93-hyo@nada.kth.se)
 */

#include <exec/types.h>


/*
 * define_field
 * ------------
 * Definierar minfältet.
 *
 * Argument:
 *  rows    - Antal rader i minfältet.
 *  columns - Antal kolumner i minfältet.
 *  mines   - Antal minor i minfältet.
 * Returvärde:
 *  TRUE om det gick att skapa ett minfält med de givna dimensionerna,
 *  FALSE annars.
 */
BOOL
define_field (
   WORD    rows,
   WORD    columns,
   ULONG   mines);


/*
 * free_field
 * ----------
 * Frigör minfältet.
 *
 * Argument:
 *  inga
 * Returvärde:
 *  inget
 */
void
free_field (void);

#endif
