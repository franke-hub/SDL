//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/TextScreen.cpp
//
// Purpose-
//       Text Screen control.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef HCDM                        // INLINE HDCM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <ctype.h>                  // Used in debug traces
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <com/Color.h>
#include <com/Debug.h>
#include <com/syslib.h>
#include <com/Unconditional.h>

#include "com/Terminal.h"
#include "com/TextScreen.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEXTSCRN" // Source file, for debugging
#define Attr TextScreenAttr         // Qualify local class

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define SUBSTITUTE '~'              // Substitute character

#ifdef _OS_WIN
  #define vsnprintf _vsnprintf
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Attr
//
// Purpose-
//       Define hidden attributes.
//
//----------------------------------------------------------------------------
class Attr                          // Hidden attributes
{
//----------------------------------------------------------------------------
// Attr::Constructors
//----------------------------------------------------------------------------
public:
inline virtual
   ~Attr( void );                   // Destructor

inline
   Attr(
     TextScreen&       screen);

private:                            // Bitwise copy prohibited
   Attr(const Attr&);               // Disallowed copy constructor
   Attr& operator=(const Attr&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       Attr::addrXY
//
// Purpose-
//       Get screen buffer address
//
//----------------------------------------------------------------------------
public:
inline Color::Char*                 // -> Screen buffer character
   addrXY(                          // Get screen buffer address
     unsigned int      col,         // Column position (X)
     unsigned int      row);        // Row position (Y)

//----------------------------------------------------------------------------
//
// Method-
//       Attr::clearScreen
//
// Purpose-
//       Clear the screen.
//
//----------------------------------------------------------------------------
public:
inline void
   clearScreen( void );             // Clear the screen.

//----------------------------------------------------------------------------
//
// Method-
//       Attr::handleResizeEvent
//
// Purpose-
//       Resize the buffer (and clear the screen).
//
//----------------------------------------------------------------------------
public:
inline void
   handleResizeEvent( void );       // Resize the buffer

//----------------------------------------------------------------------------
//
// Method-
//       Attr::setCursorMode
//
// Purpose-
//       Set the cursor mode.
//
//----------------------------------------------------------------------------
public:
inline void
   setCursorMode(                   // Set the cursor mode
     TextScreen::CursorMode
                       mode);       // Use this mode

//----------------------------------------------------------------------------
//
// Method-
//       Attr::physicalXY
//
// Purpose-
//       Set the (prevalidated) cursor position.
//
// Example-
//       physicalXY(0,0); // Position the cursor at the upper left corner
//
//----------------------------------------------------------------------------
public:
inline void
   physicalXY(                      // Set the cursor position
     unsigned int      col,         // Column number (X)
     unsigned int      row);        // Row number (Y)

//----------------------------------------------------------------------------
//
// Method-
//       Attr::setAttribute
//
// Purpose-
//       Set the attribute character.
//
//----------------------------------------------------------------------------
public:
inline void
   setAttribute(                    // Set screen attribute character
     Color::VGA        fg,          // Set this foreground color
     Color::VGA        bg);         // Set this background color

//----------------------------------------------------------------------------
//
// Method-
//       Attr::write
//
// Purpose-
//       Write the buffer on the screen.
//
//----------------------------------------------------------------------------
public:
inline void                         // No return value
   write(                           // Write buffer to screen
     unsigned int      Lcol,        // Left column number
     unsigned int      Trow,        // Top row number
     unsigned int      Rcol,        // Right column number
     unsigned int      Brow);       // Bottom row number

//----------------------------------------------------------------------------
// Attr::Attributes
//----------------------------------------------------------------------------
private:
   HANDLE              dspH;        // Standard display handle
   COORD               buffsize;    // Size of buffer
   short               initialAttr; // Initial attribute character

public:
   TextScreen&         screen;      // Assocated Screen
   short               attr;        // Current attribute character
   Color::Char*        buffer;      // -> Screen buffer
   unsigned int        columns;     // Number of columns
   unsigned int        rows;        // Number of rows
   unsigned long       size;        // columns * rows
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::~Attr
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Attr::~Attr( void )              // Destructor
{
   #ifdef HCDM
     tracef("%8s= TextScreenAttr(%p)::~TextScreenAttr()\n", "", this);
     tracef("%p= TextScreenAttr(%p).buffer\n", buffer, this);
   #endif

   if( buffer == NULL )             // If not initialized
     return;                        // Nothing to do

   attr= initialAttr;               // Restore initial attribute
   clearScreen();                   // Clear the screen

   setCursorMode(TextScreen::REPLACE); // Set replace cursor mode

   free(buffer);                    // Release the buffer
   buffer= NULL;                    // Indicate uninitialized

   #ifdef HCDM
     tracef("%8s= TextScreenAttr(%p)::~TextScreenAttr()\n", "Done", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::Attr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Attr::Attr(                      // Physically initialize the screen
     TextScreen&       screen)      // Associated screen
:  screen(screen)
,  buffer(NULL)
{
   #ifdef HCDM
     tracef("%8s= TextScreenAttr(%p)::TextScreenAttr(%p)\n",
            "", this, &screen);
   #endif

   // Get screen handle
   dspH= GetStdHandle(STD_OUTPUT_HANDLE);

   // Initialize the screen
   buffsize.X= 0;
   buffsize.Y= 0;

   handleResizeEvent();
   initialAttr= attr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::addrXY
//
// Purpose-
//       Get screen buffern address
//
//----------------------------------------------------------------------------
Color::Char*                        // -> Screen buffer character
   Attr::addrXY(                    // Get screen buffer address
     unsigned int      col,         // Column position (X)
     unsigned int      row)         // Row position (Y)
{
   return buffer + (buffsize.X*row) + col; // Return the buffer address
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::clearScreen
//
// Purpose-
//       Clear the screen.
//
//----------------------------------------------------------------------------
void
   Attr::clearScreen( void )        // Clear the screen
{
   COORD               coord;
   DWORD               cWritten;

   unsigned int        i;

   // Clear the logical buffer
   for(i=0; i<size; i++)            // Clear the screen
   {
     buffer[i].attr= attr;          // Set attribute
     buffer[i].data= ' ';           // Set data character
   }

   // Clear the (entire) physical buffer
   coord.X= 0;
   coord.Y= 0;
   FillConsoleOutputAttribute(      // Clear the entire buffer
       dspH,                        // screen buffer handle
       attr,                        // fill attribute
       buffsize.X*buffsize.Y,       // Entire buffer
       coord,                       // first cell to write to
       &cWritten);                  // Where to save actual number written to
   FillConsoleOutputCharacter(      // Clear the entire buffer
       dspH,                        // screen buffer handle
       ' ',                         // fill character
       buffsize.X*buffsize.Y,       // Entire buffer
       coord,                       // first cell to write to
       &cWritten);                  // Where to save actual number written to

   // Reposition the cursor
   physicalXY(0,0);                 // Reposition the cursor
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::handleResizeEvent
//
// Purpose-
//       Resize the buffer
//
//----------------------------------------------------------------------------
void
   Attr::handleResizeEvent( void )  // Resize the buffer
{
   int                 oldX;        // Old X-size
   int                 oldY;        // Old Y-size

   CONSOLE_SCREEN_BUFFER_INFO
                       buffinfo;    // New buffer information
   SMALL_RECT          window;      // New window position

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreenAttr(%p)::handleResizeEvent()\n", "", this);
   #endif

   // Save the old buffer size
   oldX= buffsize.X;
   oldY= buffsize.Y;

   // Extract the buffer information
   GetConsoleScreenBufferInfo(      // Locate the screen buffer
       dspH,                        // Screen handle
       &buffinfo);                  // Buffer information

   buffsize.X= buffinfo.dwSize.X;
   buffsize.Y= buffinfo.dwSize.Y;
   attr=       buffinfo.wAttributes;

   //Just use the space visible in the window
   columns= buffinfo.srWindow.Right-buffinfo.srWindow.Left+1;
   rows=    buffinfo.srWindow.Bottom-buffinfo.srWindow.Top+1;
   size=    columns*rows;

   // Position window to top-left portion of the screen buffer
   window.Top=    0;
   window.Left=   0;
   window.Right=  columns-1;
   window.Bottom= rows-1;
   SetConsoleWindowInfo(dspH, TRUE, &window);

   // If the backing buffer has expanded, free the old one
   if( oldX < buffsize.X
       ||oldY < buffsize.Y )
   {
     if( buffer != NULL )
     {
       free(buffer);
       buffer= NULL;
     }
   }

   // If required, allocate a new backing buffer
   if( buffer == NULL )
   {
     buffer= (Color::Char*)must_malloc(sizeof(Color::Char)*buffsize.Y*buffsize.X);

     #ifdef HCDM
       tracef("%p= TextScreenAttr(%p).buffer, size(%d)\n", buffer, this,
              sizeof(Color::Char)*buffsize.Y*buffsize.X);
     #endif
   }

   clearScreen();
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::setAttribute
//
// Purpose-
//       Set the default attributes.
//
//----------------------------------------------------------------------------
void
   Attr::setAttribute(              // Set screen attribute character
     Color::VGA        fg,          // Set this foreground color
     Color::VGA        bg)          // Set this background color
{
   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreenAttr(%p)::setAttribute(%d,%d)\n", "", this,
            fg, bg);
   #endif

   if( fg > VGAColor::MAXVGA
       ||bg > VGAColor::MAXVGA )
   {
     screen.error(Terminal::ErrorColor);
     return;
   }
   attr= Color::Char::retAttribute(fg, bg);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::setCursorMode
//
// Purpose-
//       Physically set the cursor mode.
//
//----------------------------------------------------------------------------
void
   Attr::setCursorMode(             // Set cursor mode
     TextScreen::CursorMode
                       mode)        // Use this mode
{
   CONSOLE_CURSOR_INFO cursor;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreenAttr(%p)::setCursorMode(%d)\n", "", this, mode);
   #endif

   cursor.dwSize= 15;               // Default, REPLACE cursor
   if( mode == TextScreen::INSERT ) // If INSERT mode
     cursor.dwSize= 50;             // Set INSERT cursor

   cursor.bVisible= TRUE;           // The cursor is *always* visible
   SetConsoleCursorInfo(            // Set the cursor shape
       dspH,                        // Screen buffer handle
       &cursor);                    // Cursor control object
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::physicalXY
//
// Purpose-
//       Set the (prevalidated) cursor position.
//
//----------------------------------------------------------------------------
void
   Attr::physicalXY(                // Set the cursor position
     unsigned int      col,         // Column number (X)
     unsigned int      row)         // Row number (Y)
{
   COORD               coord;

   #if defined(HCDM) && FALSE
     tracef("%8s= TextScreenAttr(%p)::physicalXY(%d,%d)\n", "", this,
            col, row);
   #endif

   coord.X= col;
   coord.Y= row;
   SetConsoleCursorPosition(        // Set the cursor position
       dspH,                        // Screen buffer handle
       coord);                      // Buffer position
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::write
//
// Purpose-
//       Physically write the screen.
//
//----------------------------------------------------------------------------
void
   Attr::write(                     // Write buffer to screen
     unsigned int      Lcol,        // Left column number
     unsigned int      Trow,        // Top row number
     unsigned int      Rcol,        // Right column number
     unsigned int      Brow)        // Bottom row number
{
   COORD               source;      // Source offset in buffer
   SMALL_RECT          target;      // Target rectangle

   source.X= Lcol;
   source.Y= Trow;

   #ifdef HCDM
     tracef("%8s= TextScreenAttr(%p)::write(%d,%d,%d,%d)\n", "", this,
            Lcol, Trow, Rcol, Brow);
   #endif

   #if defined(HCDM) && TRUE
     Color::Char*      buffer;      // Working buffer pointer
     int               nextAttr;    // Next Attribute character
     int               workChar;    // Working character
     int               row;         // Working row
     int               col;         // Working column

     for(row= Trow; row<=Brow; row++)
     {
       buffer= addrXY(Lcol, row);
       tracef("[%4d,%4d]", Lcol, row);
       for(col= Lcol; col<=Rcol; col++)
       {
         nextAttr= buffer->attr & 0x00ff;
         workChar= buffer->data & 0x00ff;
         if( workChar < ' ' )
           workChar= SUBSTITUTE;
         workChar |= (nextAttr << 8);
         tracef(" [%.4x='%c']", workChar, workChar&0x00ff);
         buffer++;
       }

       tracef("\n");
     }
   #endif

   target.Left= Lcol;
   target.Right= Rcol;
   target.Top= Trow;
   target.Bottom= Brow;
   WriteConsoleOutput(              // Write to the screen
       dspH,                        // Screen handle
       (const CHAR_INFO*)this->buffer, // Source buffer
       buffsize,                    // Buffer size
       source,                      // Offset in buffer
       &target);                    // Target rectangle
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::~TextScreen
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   TextScreen::~TextScreen( void )  // Destructor
{
   #ifdef HCDM
     tracef("%8s= TextScreen(%p)::~TextScreen()\n", "", this);
     tracef("%p= TextScreen(%p).attr\n", attr, this);
     tracef("%p= TextScreen(%p).buffer\n",
            ((Attr*)this->attr)->buffer, this);
   #endif

   if( attr != NULL )               // If attributes exist
   {
     delete (Attr*)attr;            // Delete them
     attr= NULL;
   }

   #ifdef HCDM
     tracef("%8s= TextScreen(%p)::~TextScreen()\n", "Done", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::TextScreen
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   TextScreen::TextScreen( void )   // Constructor
:  Handler()
,  currentCol(0)
,  currentRow(0)
{
   //-------------------------------------------------------------------------
   // Debugging Hook
   //-------------------------------------------------------------------------
   #ifdef HCDM
     tracef("%8s= TextScreen(%p)::TextScreen()\n", "", this);
   #endif

   attr= (void*)new Attr(*this);    // Create associated attributes

   #ifdef HCDM
     tracef("%p= TextScreen(%p).attr\n", attr, this);
     tracef("%p= TextScreen(%p).buffer\n",
            ((Attr*)this->attr)->buffer, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::next
//
// Purpose-
//       Increment cursor position.
//
//----------------------------------------------------------------------------
void
   TextScreen::next( void )         // Increment cursor position
{
   Attr&               attr= *(Attr*)this->attr;

   currentCol++;                    // Increment column position
   if (currentCol >= attr.columns)  // If wrap
   {
     currentCol= 0;                 // Wrap to column[0]
     currentRow++;                  // Increment row position
     if (currentRow >= attr.rows)   // If wrap
       currentRow= 0;               // Wrap to row[0]
   }

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::next() col(%d) row(%d)\n", "", this,
            currentCol, currentRow);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::alarm
//
// Purpose-
//       Sound the audible alarm.
//
//----------------------------------------------------------------------------
void
   TextScreen::alarm( void )        // Sound the alarm
{
   fprintf(stderr, "\a");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::clearScreen
//
// Purpose-
//       Clear the screen.
//
//----------------------------------------------------------------------------
void
   TextScreen::clearScreen( void )  // Clear the screen
{
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::clearScreen()\n", "", this);
   #endif

   attr.clearScreen();              // Clear the screen
   logicalXY(0,0);                  // Set the logical position
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::deleteRow
//
// Purpose-
//       Delete a row.
//
// Notes-
//       The screen is scrolled up, duplicating the bottom row.
//       (Deleting the bottom row has no effect.)
//
//----------------------------------------------------------------------------
void
   TextScreen::deleteRow(           // Delete row
     unsigned int      toprow)      // Delete this row
{
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::deleteRow(%d)\n", "", this, toprow);
   #endif

   deleteRow(toprow, attr.rows-1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::deleteRow
//
// Purpose-
//       Delete a row.
//
// Notes-
//       The screen is scrolled up, duplicating the bottom row.
//       (Deleting the bottom row has no effect.)
//
//----------------------------------------------------------------------------
void
   TextScreen::deleteRow(           // Delete row
     unsigned int      toprow,      // Delete this row
     unsigned int      botrow)      // Last row to modify
{
   Attr&               attr= *(Attr*)this->attr;

   Color::Char*        to;          // Address of row to delete
   Color::Char*        fr;          // Address of next row

   size_t              size;        // Move length

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::deleteRow(%d,%d)\n", "", this,
            toprow, botrow);
   #endif

   if (toprow > botrow
       ||botrow >= attr.rows)       // If invalid row value
   {
     error(Terminal::ErrorPosition);// Set error number
     return;                        // and return
   }

   if (toprow < botrow)             // If not last row on screen
   {
     to= attr.addrXY(0, toprow);
     fr= attr.addrXY(0, toprow+1);
     size= (botrow-toprow) * attr.columns * sizeof(Color::Char);
     memmove(to, fr, size);
     attr.write(0, toprow, attr.columns-1, botrow);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::getXSize
//
// Purpose-
//       Return the current number of screen columns.
//
//----------------------------------------------------------------------------
unsigned int
   TextScreen::getXSize( void )     // Get number of screen columns
{
   Attr&               attr= *(Attr*)this->attr;

   return attr.columns;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::getYSize
//
// Purpose-
//       Return the current number of screen rows.
//
//----------------------------------------------------------------------------
unsigned int
   TextScreen::getYSize( void )     // Get number of screen rows
{
   Attr&               attr= *(Attr*)this->attr;

   return attr.rows;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::handleResizeEvent
//
// Purpose-
//       Handle a resize event.
//
//----------------------------------------------------------------------------
void
   TextScreen::handleResizeEvent( void ) // Handle a resize event
{
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::handleResizeEvent()\n", "", this);
   #endif

   attr.handleResizeEvent();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::insertRow
//
// Purpose-
//       Insert a row.
//
// Notes-
//       The screen is scrolled down, duplicating the row.
//       (Inserting the bottom row has no effect.)
//
//----------------------------------------------------------------------------
void
   TextScreen::insertRow(           // Insert row
     unsigned int      toprow)      // Roll this row down one row,
{
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::insertRow(%d)\n", "", this, toprow);
   #endif

   insertRow(toprow, attr.rows-1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::insertRow
//
// Purpose-
//       Insert a row.
//
// Notes-
//       The screen is scrolled down, duplicating the row.
//       (Inserting the bottom row has no effect.)
//
//----------------------------------------------------------------------------
void
   TextScreen::insertRow(           // Insert row
     unsigned int      toprow,      // Roll this row down one row,
                                    // duplicating toprow.
     unsigned int      botrow)      // Last row to modify
{
   Attr&               attr= *(Attr*)this->attr;

   Color::Char*        to;          // Address of next row
   Color::Char*        fr;          // Address of row to insert

   size_t              size;        // Move length

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::insertRow(%d,%d)\n", "", this,
            toprow, botrow);
   #endif

   if (toprow > botrow
       ||botrow >= attr.rows)       // If invalid row value
   {
     error(Terminal::ErrorPosition);// Set error number
     return;                        // and return
   }

   if (toprow < botrow)             // If not bottom row
   {
     to= attr.addrXY(0, toprow+1);
     fr= attr.addrXY(0, toprow);
     size= (botrow-toprow) * attr.columns * sizeof(Color::Char);
     memmove(to, fr, size);
     attr.write(0, toprow, attr.columns-1, botrow);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::logicalXY
//
// Purpose-
//       Set logical column and row.
//
//----------------------------------------------------------------------------
void
   TextScreen::logicalXY(           // Set row, column coordinate
     unsigned int      col,         // Column number
     unsigned int      row)         // Row number
{
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::logicalXY(%d,%d)\n", "", this, col, row);
   #endif

   if (col >= attr.columns
      ||row >= attr.rows)           // If invalid position
   {
     error(Terminal::ErrorPosition);// Set error number
     return;                        // and return
   }

   currentCol= col;
   currentRow= row;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::physicalXY
//
// Purpose-
//       Set the cursor position.
//
//----------------------------------------------------------------------------
void
   TextScreen::physicalXY(          // Set the cursor position.
     unsigned int      col,         // Column number (X)
     unsigned int      row)         // Row number (Y)
{
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::physicalXY(%d,%d)\n", "", this, col, row);
   #endif

   if (col >= attr.columns
      ||row >= attr.rows)           // If invalid position
   {
     error(Terminal::ErrorPosition);// Set error number
     return;                        // and return
   }

   attr.physicalXY(col,row);        // Set the cursor position.
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::printf
//
// Function-
//       Screen printf facility.
//
//----------------------------------------------------------------------------
void
   TextScreen::printf(              // Screen printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   int                 L;           // strlen(out)
   char                out[512];    // sprintf() format area

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::printf(%s,...)\n", "", this, fmt);
   #endif

   va_start(argptr, fmt);           // Initialize va_ functions
   L= vsnprintf(out, sizeof(out), fmt, argptr); // Format the string
   if( L < 0 )
     L= sizeof(out);

   va_end(argptr);                  // Close va_ functions

   wr(out, L);                      // Write the string
}

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::resume
//
// Purpose-
//       Resume operation.
//
//----------------------------------------------------------------------------
void
   TextScreen::resume( void )       // Resume operation
{
   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::resume()\n", "", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::setAttribute
//
// Purpose-
//       Set the default attributes.
//
//----------------------------------------------------------------------------
void
   TextScreen::setAttribute(        // Set screen attribute character
     Color::VGA        fg,          // Set this foreground color
     Color::VGA        bg)          // Set this background color
{
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::setAttribute(%d,%d)\n", "", this, fg, bg);
   #endif

   attr.setAttribute(fg, bg);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::setCursorMode
//
// Purpose-
//       Set cursor mode.
//
//----------------------------------------------------------------------------
void
   TextScreen::setCursorMode(       // Set cursor mode
     CursorMode        mode)        // Use this mode
{
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::setCursorMode(%d)\n", "", this, mode);
   #endif

   attr.setCursorMode(mode);        // Physically set the cursor mode
}

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::suspend
//
// Purpose-
//       Suspend operation.
//
//----------------------------------------------------------------------------
void
   TextScreen::suspend( void )      // Suspend operation
{
   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::suspend()\n", "", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::wr
//
// Purpose-
//       Write character at logical position.
//
// Notes-
//       The logical position is updated.
//
//----------------------------------------------------------------------------
void
   TextScreen::wr(                  // Put character (at current),
     char              c)           // Data character
{
   Attr&               attr= *(Attr*)this->attr;
   Color::Char*        buff;        // Working buffer pointer

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::wr('%c'=%.2x)\n", "", this, c, c);
   #endif

   if( c == '\n' )
   {
     currentRow++;                  // Increment row position
     if (currentRow >= attr.rows)   // If wrap
       currentRow= 0;               // Wrap to row[0]
     currentCol= 0;                 // Set column number 0
     return;
   }

   if( c == '\r' )
   {
     currentCol= 0;                 // Set column number 0
     return;
   }

   if( c == '\t' )
     c= ' ';

   buff= attr.addrXY(currentCol, currentRow); // Get current position
   buff[0].attr= attr.attr;         // Set attribute
   buff[0].data= c;                 // Set data character

   attr.write(currentCol, currentRow, currentCol, currentRow);
   next();                          // Update the cursor position
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::wr
//
// Purpose-
//       Write string at current position.
//
// Notes-
//       The logical position is updated.
//
//----------------------------------------------------------------------------
void
   TextScreen::wr(                  // Write screen
     const char*       buffer)      // -> Character string
{
   wr(buffer, strlen(buffer));
}

void
   TextScreen::wr(                  // Put character (at current),
     const char*       buffer,      // -> Character string
     unsigned int      buflen)      // Sizeof(buffer)
{
   Attr&               attr= *(Attr*)this->attr;

   Color::Char*        buff;        // Working buffer pointer
   unsigned int        minCol= currentCol;
   unsigned int        minRow= currentRow;
   unsigned int        maxCol= currentCol;
   unsigned int        maxRow= currentRow;

   unsigned int        i;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::wr(\"%s\",%d)\n", "", this, buffer, buflen);
   #endif

   for(i=0; i<buflen; i++)          // Write the string
   {
     if (currentCol > maxCol)       // Adjust min/max
       maxCol= currentCol;
     if (currentRow > maxRow)
       maxRow= currentRow;
     if (currentCol < minCol )
       minCol= currentCol;
     if (currentRow < minRow)
       minRow= currentRow;

     buff= attr.addrXY(currentCol, currentRow); // Get current position
     switch (buffer[i])             // Process character
     {
       case '\n':                   // If newline
         currentRow++;
         if (currentRow >= attr.rows) // If wrap
         {
           currentRow= 0;           // Wrap to row[0]
           maxRow= attr.rows-1;
         }

         // Continue, set currentCol= 0

       case '\r':                   // If carriage return
         currentCol= 0;
         break;

       default:
         buff[0].attr= attr.attr;   // Set attribute
         buff[0].data= buffer[i];   // Set data character
         next();                    // Update the cursor position
         break;
     }
   }

   attr.write(minCol, minRow, maxCol, maxRow);
}

void
   TextScreen::wr(                  // Overwrite line
     const Color::Char*
                       buffer,      // -> Color::Char string
     unsigned int      buflen)      // Number of buffer elements
                                    // (size == columns)
{
   Attr&               attr= *(Attr*)this->attr;

   Color::Char*        buff;        // Working buffer pointer
   unsigned int        minCol= currentCol;
   unsigned int        minRow= currentRow;
   unsigned int        maxCol= currentCol;
   unsigned int        maxRow= currentRow;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::wr(%p,%d)\n", "", this, buffer, buflen);
   #endif

   while( buflen > 0 )              // Write into screen buffer
   {
     if (currentCol > maxCol)       // Adjust min/max
       maxCol= currentCol;
     if (currentRow > maxRow)
       maxRow= currentRow;
     if (currentCol < minCol )
       minCol= currentCol;
     if (currentRow < minRow)
       minRow= currentRow;

     buff= attr.addrXY(currentCol, currentRow); // Get current position
     *buff= *buffer;                // Set the screen character
     buffer++;
     buflen--;

     next();
   }

   attr.write(minCol, minRow, maxCol, maxRow);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::wr
//
// Purpose-
//       Overwrite one row on screen, blank fill if required.
//
// Notes-
//       The logical position is unchanged.
//
//----------------------------------------------------------------------------
void
   TextScreen::wr(                  // Write screen
     unsigned int      row,         // row position (column 0)
     const char*       buffer)      // -> Character string
{
   wr(row, buffer, strlen(buffer));
}

void
   TextScreen::wr(                  // Overwrite row
     unsigned int      row,         // Overwrite this row
     const char*       buffer,      // -> Character string
     unsigned int      buflen)      // Sizeof(buffer)
{
   Attr&               attr= *(Attr*)this->attr;

   Color::Char*        buff;        // Working buffer pointer
   unsigned int        size;        // Working buffer length
   unsigned int        i;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::wr(%d,\"%s\",%d)\n", "", this,
            row, buffer, buflen);
   #endif

   if (row >= attr.rows)            // If invalid row value
   {
     error(Terminal::ErrorPosition);// Set error number
     return;                        // and return
   }

   size= buflen;                    // Set data length
   if (size > attr.columns)
     size= attr.columns;

   buff= attr.addrXY(0, row);       // Address the row
   for(i=0; i<size; i++)            // Write into screen buffer
   {
     buff[i].attr= attr.attr;       // Set the attribute character
     buff[i].data= buffer[i];       // Set the data character
   }

   for(i=size; i<attr.columns; i++) // Blank fill
   {
     buff[i].attr= attr.attr;       // Set attribute
     buff[i].data= ' ';             // Set data character
   }

   attr.write(0, row, attr.columns-1, row);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TextScreen::wr
//
// Purpose-
//       Overwrite one row on screen, blank fill if required.
//
// Notes-
//       The logical position is unchanged.
//
//----------------------------------------------------------------------------
void
   TextScreen::wr(                  // Overwrite line
     unsigned int      row,         // Overwrite this line
     const Color::Char*
                       buffer,      // -> Color::Char string
     unsigned int      buflen)      // Number of buffer elements
                                    // (size == columns)
{
   Attr&               attr= *(Attr*)this->attr;

   Color::Char*        buff;        // Working buffer pointer
   unsigned int        size;        // Working buffer length
   unsigned int        i;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::wr(%d,%p,%d)\n", "", this,
            row, buffer, buflen);
   #endif

   if (row >= attr.rows)            // If invalid row value
   {
     error(Terminal::ErrorPosition);// Set error number
     return;                        // and return
   }

   size= buflen;                    // Set data length
   if (size > attr.columns)
     size= attr.columns;

   buff= attr.addrXY(0, row);       // Address the row
   for(i=0; i<size; i++)            // Write into screen buffer
     buff[i]= buffer[i];            // Set the screen character

   for(i=size; i<attr.columns; i++) // Blank fill
   {
     buff[i].attr= attr.attr;       // Set attribute (from current)
     buff[i].data= ' ';             // Set data character
   }

   attr.write(0, row, attr.columns-1, row);
}

