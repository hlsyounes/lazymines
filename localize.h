#ifndef LOCALIZE_H
#define LOCALIZE_H

/*
 * localize.h
 * ==========
 * Functions for localization.
 */

#include <exec/types.h>

#define CATCOMP_NUMBERS
#include "strings.h"


void
init_locale (
   char  *catalog);

void
finalize_locale (void);

char *
localized_string (
   LONG   string_num);

struct Menu *
CreateLocMenus (
   struct NewMenu  *new_menus,
   APTR             vis_info,
   ULONG            tag,
   ...);

#endif
