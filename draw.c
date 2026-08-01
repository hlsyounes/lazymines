#include <exec/types.h>
#include <graphics/text.h>
#include <libraries/gadtools.h>
#include <intuition/intuition.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <string.h>
#include "globals.h"
#include "images.h"

void DrawBox(short row, short col, BOOL recessed);
void DrawField(void);
void DrawCell(short row, short col);
void ClearField(void);

short Cellvalue(short row, short col);

extern struct Window *mineWin;
extern char windowTitle[];
extern char winTitleEnd;
extern struct VisualInfo *visualInfo;
extern struct TextFont *defaultFont;
extern UWORD textColor, backColor;
extern UWORD shineColor, shadowColor;
extern LONG numberColors[];
extern UWORD xOffs, yOffs;

extern short num_rows, num_columns;
extern short cellH, cellW;

void
DrawBox(
   short row,
   short col,
   BOOL recessed)
{
   if (recessed)
   {
      SetAPen (mineWin->RPort, shadowColor);
      Move (mineWin->RPort,
            xOffs + col * cellW, yOffs + row * cellH);
      Draw (mineWin->RPort,
            xOffs + (col + 1) * cellW - 1, yOffs + row * cellH);
      Draw (mineWin->RPort,
            xOffs + (col + 1) * cellW - 1, yOffs + (row + 1) * cellH - 1);
      Draw (mineWin->RPort,
            xOffs + col * cellW, yOffs + (row + 1) * cellH - 1);
      Draw (mineWin->RPort,
            xOffs + col * cellW, yOffs + row * cellH);
      SetAPen (mineWin->RPort, backColor);
      Move (mineWin->RPort,
            xOffs + col * cellW + 1, yOffs + row * cellH + 1);
      Draw (mineWin->RPort,
            xOffs + col * cellW + 1, yOffs + (row + 1) * cellH - 2);
      Move (mineWin->RPort,
            xOffs + (col + 1) * cellW - 2, yOffs + row * cellH + 1);
      Draw (mineWin->RPort,
            xOffs + (col + 1) * cellW - 2, yOffs + (row + 1) * cellH - 2);
   }
   else
   {
      SetAPen(mineWin->RPort, shineColor);
      Move(mineWin->RPort,
           xOffs + (col + 1) * cellW - 2, yOffs + row * cellH);
      Draw(mineWin->RPort,
           xOffs + col * cellW, yOffs + row * cellH);
      Draw(mineWin->RPort,
           xOffs + col * cellW, yOffs + (row + 1) * cellH - 1);
      Move(mineWin->RPort,
           xOffs + col * cellW + 1, yOffs + (row + 1) * cellH - 2);
      Draw(mineWin->RPort,
           xOffs + col * cellW + 1, yOffs + row * cellH + 1);
      SetAPen(mineWin->RPort, shadowColor);
      Move(mineWin->RPort,
           xOffs + col * cellW + 1, yOffs + (row + 1) * cellH - 1);
      Draw(mineWin->RPort,
           xOffs + (col + 1) * cellW - 1, yOffs + (row + 1) * cellH - 1);
      Draw(mineWin->RPort,
           xOffs + (col + 1) * cellW - 1, yOffs + row * cellH);
      Move(mineWin->RPort,
           xOffs + (col + 1) * cellW - 2, yOffs + row * cellH + 1);
      Draw(mineWin->RPort,
           xOffs + (col + 1) * cellW - 2, yOffs + (row + 1) * cellH - 2);
   }
}

void
DrawField(void)
{
   register short r, c;
   
   for (r = 0; r < num_rows; r++)
      for (c = 0; c < num_columns; c++)
         DrawCell(r, c);
}

void
DrawCell(
   short row,
   short col)
{
   char value = Cellvalue(row, col) + '0';
   
   DrawBox(row, col, field[row * num_columns + col] & REVEALED);
   if (field[row * num_columns + col] & REVEALED)
   {
      if (value - '0' == MINE)
         DrawImage(mineWin->RPort, &mineImage,
                   xOffs + col * cellW + (cellW - mineImage.Width) / 2,
                   yOffs + row * cellH + (cellH - mineImage.Height) / 2);
      else if (value != '0')
      {
         SetAPen(mineWin->RPort, (numberColors[value - '0' - 1] != -1) ? numberColors[value - '0' - 1] : textColor);
         Move(mineWin->RPort,
              xOffs + col * cellW + (cellW - defaultFont->tf_XSize) / 2,
              yOffs + row * cellH + (cellH - defaultFont->tf_YSize) / 2 +
                 defaultFont->tf_Baseline);
         Text(mineWin->RPort, &value, 1);
      }
   }
   else if (field[row * num_columns + col] & LOCKED)
      if (field[row * num_columns + col] & WARNING)
      {
         SetAPen(mineWin->RPort, backColor);
         RectFill(mineWin->RPort,
                  xOffs + col * cellW + 2, yOffs + row * cellH + 1,
                  xOffs + (col + 1) * cellW - 3,
                  yOffs + (row + 1) * cellH - 2);
         SetAPen(mineWin->RPort, textColor);
         Move(mineWin->RPort,
              xOffs + col * cellW + (cellW - defaultFont->tf_XSize) / 2,
              yOffs + row * cellH + (cellH - defaultFont->tf_YSize) / 2 +
                 defaultFont->tf_Baseline);
         Text(mineWin->RPort, "?", 1);
      }
      else
         DrawImage(mineWin->RPort, &flagImage,
                   xOffs + col * cellW + (cellW - flagImage.Width) / 2,
                   yOffs + row * cellH + (cellH - flagImage.Height) / 2);
   else
   {
      SetAPen(mineWin->RPort, backColor);
      RectFill(mineWin->RPort,
               xOffs + col * cellW + 2, yOffs + row * cellH + 1,
               xOffs + (col + 1) * cellW - 3,
               yOffs + (row + 1) * cellH - 2);
   }
}

void
ClearField(void)
{
   register short r, c;
   
   SetAPen(mineWin->RPort, backColor);
   RectFill(mineWin->RPort, xOffs + 2, yOffs + 1,
            xOffs + cellW - 3, yOffs + cellH - 2);
   DrawBox(0, 0, FALSE);
   for (r = 0; r < num_rows; r++)
      for (c = 0; c < num_columns; c++)
         if (r || c)
            ClipBlit(mineWin->RPort, xOffs, yOffs, mineWin->RPort,
                     xOffs + (c * cellW), yOffs + (r * cellH),
                     cellW, cellH, 0x0C0);
}

void
draw_flagcounter (
   ULONG   flags)
{
   register int   i = 2;
   
   windowTitle[winTitleEnd] = '\0';
   strncat (windowTitle, "           Mines Left:    ", 81);
   windowTitle[winTitleEnd + 26 + 3] = '\0';
   while (i >= 0)
   {
      windowTitle[winTitleEnd + 23 + i] = flags % 10 + '0';
      --i;
      flags /= 10;
   }
   SetWindowTitles (mineWin, windowTitle, (UBYTE *)(~0));
}
