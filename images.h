#ifndef IMAGES_H
#define IMAGES_H

/*
 * images.h
 * ========
 * Interface to images.
 */

#include <exec/types.h>
#include <intuition/screens.h>


#define IMAGESIZE(w, h) (((((w) + 15) >> 4) << 4) * (h))

#define FLAGIMAGE_WIDTH    8
#define FLAGIMAGE_HEIGHT   8

extern UBYTE   flagimage[];

#define BOMBIMAGE_WIDTH    8
#define BOMBIMAGE_HEIGHT   6

extern UBYTE   bombimage[];

#define DIGITIMAGE_WIDTH    13
#define DIGITIMAGE_HEIGHT   21

extern UBYTE digitimages[];

#define EMPTYDIGIT   10
#define DIGITIMAGE(n) (&digitimages[(n) * IMAGESIZE (DIGITIMAGE_WIDTH, DIGITIMAGE_HEIGHT)])

#define MAX_IMAGEWIDTH     13
#define MAX_IMAGEHEIGHT    21


BOOL
init_images (
   struct Window  *win);

void
finalize_images (void);

void
draw_image (
   struct RastPort  *rp,
   UWORD             x_pos,
   UWORD             y_pos,
   UWORD             width,
   UWORD             height,
   UBYTE            *image);

#endif
