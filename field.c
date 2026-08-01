/*
 * field.c
 * =======
 * Implementerar minfältet.
 *
 * Copyright (C) 1994 Lorens Younes (d93-hyo@nada.kth.se)
 */

#include <exec/types.h>
#include <clib/exec_protos.h>
#include <math.h>
#include "globals.h"
#include "field.h"

void NewGame(void);
void WinTheGame(void);
void GameOver(void);
void InitField(void);
BOOL IsInside(short row, short col);
short Cellvalue(short row, short col);
BOOL RevealThis(short row, short col);
BOOL RevealAround(short row, short col);
void ToggleLock(short row, short col);
BOOL SweepThis(short row, short col);
void RemoveWarnings(short row, short col);
void PressThis(short row, short col);
void PressAround(short row, short col);
void ReleaseThis(short row, short col);
void ReleaseAround(short row, short col);

void ClearField(void);
void DrawCell(short row, short col);
void DrawBox(short row, short col, BOOL recessed);
void draw_flagcounter (ULONG);

extern short num_rows, num_columns, num_mines;
extern short revealed;
extern short flagsLeft;
extern BOOL playing;
extern BOOL openSafe, warnings;
extern BOOL autoLock, autoOpen;


/* Variabler som definierar minfältet. */
UBYTE  *field = NULL;   /* minfältet */


BOOL
define_field (
   WORD    rows,
   WORD    columns,
   ULONG   mines)
{
   num_mines = mines;
   if (field != NULL && rows == num_rows && columns == num_columns)
      return TRUE;
   
   num_rows = rows;
   num_columns = columns;
   free_field ();
   field = AllocVec (num_rows * num_columns * sizeof (*field), 0L);
   
   return (BOOL)(field != NULL);
}


void
free_field (void)
{
   if (field != NULL)
      FreeVec (field);
}


void
NewGame(void)
{
   register short   r, c;
   register BOOL    success = FALSE;
   revealed = 0;
   flagsLeft = num_mines;
   draw_flagcounter (flagsLeft);
   InitField();
   ClearField();
   playing = TRUE;

   if (autoOpen)
   {
      for (r = 0; r < num_rows && !success; ++r)
         for (c = 0; c < num_columns && !success; ++c)
         if (success = (Cellvalue (r, c) == 0))
            RevealThis (r, c);
   }
}

void
WinTheGame(void)
{
   register short r, c;
   
   for (r = 0; r < num_rows; r++)
      for (c = 0; c < num_columns; c++)
         if (Cellvalue(r, c) == MINE)
         {
            field[r * num_columns + c] |= LOCKED;
            DrawCell(r, c);
         }
   draw_flagcounter (0);
}

void
GameOver(void)
{
   register short r, c;
   
   for (r = 0; r < num_rows; r++)
      for (c = 0; c < num_columns; c++)
         if (Cellvalue(r, c) == MINE)
         {
            field[r * num_columns + c] |= REVEALED;
            DrawCell(r, c);
         }
}

void
InitField(void)
{
   register short r, c, j, i = 0;
   
   for (r = 0; r < num_rows; r++)
      for (c = 0; c < num_columns; c++)
         field[r * num_columns + c] = 0;
   while (i < num_mines)
   {
      r = drand48() * num_rows;
      c = drand48() * num_columns;
      if (field[r * num_columns + c] == 0)
      {
         i++;
         field[r * num_columns + c] = MINE;
      }
   }
   for (r = 0; r < num_rows; r++)
      for (c = 0; c < num_columns; c++)
         for (i = r - 1; i <= r + 1; i++)
            for (j = c - 1; j <= c + 1; j++)
               if (IsInside(i, j) &&
                   field[r * num_columns + c] != MINE &&
                   field[i * num_columns + j] == MINE)
               {
                  field[r * num_columns + c]++;
               }
}

BOOL
IsInside(
   short row,
   short col)
{
   return (BOOL)(row >= 0 && col >= 0 && row < num_rows && col < num_columns);
}

short
Cellvalue(
   short row,
   short col)
{
   return (short)(field[row * num_columns + col] & 0x0F);
}

BOOL
RevealThis(
   short row,
   short col)
{
   register short value;
   
   if (IsInside(row, col) &&
       !(field[row * num_columns + col] & (REVEALED | LOCKED)))
   {
      value = Cellvalue(row, col);
      if (value == MINE && revealed == 0 && openSafe)
      {
         ReleaseThis(row, col);
         return TRUE;
      }
      field[row * num_columns + col] |= REVEALED;
      revealed++;
      DrawCell(row, col);
      if (value == 0)
         RevealAround(row, col);
      return (BOOL)(value != MINE);
   }
   return TRUE;
}

BOOL
RevealAround(
   short row,
   short col)
{
   register short i, j;
   register BOOL success = TRUE;
   
   for (i = -1; i <= 1 && success; i++)
      for (j = -1; j <= 1 && success; j++)
         if (i || j)
            success = RevealThis(row + i, col + j);
   
   return success;
}

void
ToggleLock(
   short row,
   short col)
{
   if (IsInside(row, col) && !(field[row * num_columns + col] & REVEALED))
      if (field[row * num_columns + col] & LOCKED)
      {
         if (field[row * num_columns + col] & WARNING)
            field[row * num_columns + col] &= ~(WARNING | LOCKED);
         else
         {
            if (warnings)
               field[row * num_columns + col] |= WARNING;
            else
               field[row * num_columns + col] &= ~LOCKED;
            flagsLeft++;
            draw_flagcounter (flagsLeft);
         }
         DrawCell(row, col);
      }
      else
         if (flagsLeft > 0)
         {
            field[row * num_columns + col] |= LOCKED;
            flagsLeft--;
            draw_flagcounter (flagsLeft);
            DrawCell(row, col);
         }
}

BOOL
SweepThis(
   short row,
   short col)
{
   register short i, j;
   short lockCount = 0;
   short warnCount = 0;
   short freeCount = 0;
   BOOL success = TRUE;
   
   if (IsInside(row, col) && field[row * num_columns + col] & REVEALED)
   {
      for (i = row - 1; i <= row + 1; i++)
         for (j = col - 1; j <= col + 1; j++)
            if (IsInside (i, j))
            {
               if (field[i * num_columns + j] & WARNING)
                  warnCount++;
               else if (field[i * num_columns + j] & LOCKED)
                  lockCount++;
               else if (!(field[i * num_columns + j] & REVEALED))
                  freeCount++;
            }
      if (lockCount >= Cellvalue (row, col))
      {
         if (warnCount)
            RemoveWarnings (row, col);
         success = RevealAround (row, col);
      }
      else if (freeCount + warnCount + lockCount == Cellvalue (row, col))
      {
         if (autoLock)
         {
            if (warnCount)
               RemoveWarnings (row, col);
            for (i = row - 1; i <= row + 1; ++i)
               for (j = col - 1; j <= col + 1; ++j)
                  if (IsInside (i, j) &&
                      !(field[i * num_columns + j] & (REVEALED | LOCKED)))
                  {
                     ToggleLock (i, j);
                  }
         }
      }
      ReleaseAround (row, col);
   }
   
   return success;
}

void
RemoveWarnings(
   short row,
   short col)
{
   register short i, j;
   
      for (i = row - 1; i <= row + 1; i++)
         for (j = col - 1; j <= col + 1; j++)
            if (IsInside(i, j) && field[i * num_columns + j] & WARNING)
            {
               field[i * num_columns + j] &= ~(WARNING | LOCKED);
               DrawCell(i, j);
            }
}

void
PressThis(
   short row,
   short col)
{
   if (IsInside(row, col) &&
       !(field[row * num_columns + col] & (REVEALED | LOCKED)))
   {
      DrawBox(row, col, TRUE);
   }
}

void
PressAround(
   short row,
   short col)
{
   register short i, j;
   
   if (IsInside(row, col) && field[row * num_columns + col] & REVEALED)
      for (i = -1; i <= 1; i++)
         for (j = -1; j <= 1; j++)
            if (i || j)
               PressThis(row + i, col + j);
}

void
ReleaseThis(
   short row,
   short col)
{
   if (IsInside(row, col) &&
       !(field[row * num_columns + col] & (REVEALED | LOCKED)))
   {
      DrawBox(row, col, FALSE);
   }
}

void
ReleaseAround(
   short row,
   short col)
{
   register short i, j;
   
   if (IsInside(row, col) && field[row * num_columns + col] & REVEALED)
      for (i = -1; i <= 1; i++)
         for (j = -1; j <= 1; j++)
            if (i || j)
               ReleaseThis(row + i, col + j);
}
