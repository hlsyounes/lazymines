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


extern struct LocaleInfo   li;


STRPTR __asm
GetString (
   register __a0 struct LocaleInfo *li,
   register __d0 LONG stringNum);

void
init_locale (
   char  *catalog);

void
finalize_locale (void);

struct Menu *
CreateLocMenus (
   struct NewMenu  *new_menus,
   APTR             vis_info,
   ULONG            tag,
   ...);

#endif
