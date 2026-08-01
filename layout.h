#ifndef LAYOUT_H
#define LAYOUT_H

/*
 * layout.h
 * ========
 * 
 */

#include <exec/types.h>


#define LINEWIDTH    2
#define LINEHEIGHT   1

#define ROMFONT_WIDTH    8
#define ROMFONT_HEIGHT   8

#define NORMAL_SPACE   0
#define NO_SPACE       1

extern UBYTE   cell_space;
extern UBYTE   cell_w, cell_h;
extern BOOL    digital_display;

#define NOPEN                -1
#define ONEPEN               0
#define TWOPEN               1
#define THREEPEN             2
#define FOURPEN              3
#define FIVEPEN              4
#define SIXPEN               5
#define SEVENPEN             6
#define EIGHTPEN             7
#define FLAGPEN              8
#define POLEPEN              9
#define GROUNDPEN            10
#define GROUNDSHADOWPEN      11
#define BOMBPEN              12
#define DIGITBACKGROUNDPEN   13
#define DIGITFILLPEN         14
#define DIGITSHADOWPEN       15

#define NUM_GAMEPENS   16

extern WORD    game_pens[];
extern UWORD  *gui_pens;


void
window_extent (
   struct Screen  *scr,
   UBYTE           level,
   UWORD           font_w,
   UWORD           font_h,
   UWORD          *used_w,
   UWORD          *used_h);

void
init_pens (
   struct Screen  *scr);

void
free_pens (
   struct Screen  *scr);

BOOL
layout_display (
   struct Screen  *scr,
   BOOL           *rom_font);

#endif
