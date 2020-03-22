//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/TextScreen.cpp
//
// Purpose-
//       Text Screen control.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef HCDM                        // INLINE HDCM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <ctype.h>                  // Used in debug traces
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#define  NCURSES_INTERNALS 1        // Uses WINDOW
#include <curses.h>                 // Uses CURSES

#include <com/Color.h>
#include <com/Debug.h>
#include <com/syslib.h>
#include <com/Unconditional.h>

#include "com/Terminal.h"
#include "com/TextScreen.h"

#undef  inline                      // INLINEs are inline

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define Attr TextScreenAttr         // Qualify local class
#define SUBSTITUTE '~'              // Substitute character

//----------------------------------------------------------------------------
//
// Subroutine-
//       VGA2BSD
//
// Purpose-
//       Convert VGA::Color to curses color
//
//----------------------------------------------------------------------------
inline unsigned                     // The BSD Color
   VGA2BSD(                         // Convert VGA to BSD color
     unsigned          vga)         // The VGA color
{
   switch( vga )
   {
     case VGAColor::Black:
       return COLOR_BLACK;

     case VGAColor::Blue:
       return COLOR_BLUE;

     case VGAColor::Green:
       return COLOR_GREEN;

     case VGAColor::Cyan:
       return COLOR_CYAN;

     case VGAColor::Red:
       return COLOR_RED;

     case VGAColor::Magenta:
       return COLOR_MAGENTA;

     case VGAColor::Brown:
       return COLOR_YELLOW;

     case VGAColor::Grey:
       return COLOR_WHITE;
   }

   return vga; // Should not occur
}

//----------------------------------------------------------------------------
//
// Class-
//       Attr
//
// Purpose-
//       Define hidden attributes.
//
//----------------------------------------------------------------------------
class Attr {                        // Hidden attributes
//----------------------------------------------------------------------------
// Attr::Attributes
//----------------------------------------------------------------------------
private:
   WINDOW*             dspH;        // Standard display handle
   unsigned            xSize;       // Horizontal size of buffer
   unsigned            ySize;       // Vertical size of buffer
   attr_t              initAttr;    // Initial attribute character

public:
   TextScreen&         screen;      // Associated TextScreen
   attr_t              currAttr;    // Current attribute character
   attr_t              attrArray[256]; // Attribute array
   Color::Char*        buffer;      // -> Screen buffer
   unsigned int        columns;     // Number of columns
   unsigned int        rows;        // Number of rows
   unsigned long       size;        // columns * rows

//----------------------------------------------------------------------------
//
// Methode-
//       Attr::~Attr
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
inline
   ~Attr( void )                    // Destructor
{
   #ifdef HCDM
     tracef("%8s= TextScreenAttr(%p)::~TextScreenAttr()\n", "", this);
     tracef("%p= TextScreenAttr(%p).buffer\n", buffer, this);
   #endif

   if( buffer != NULL )             // If initialized
   {
     currAttr= initAttr;            // Restore initial attribute
     clearScreen();                 // Clear the screen
     setCursorMode(TextScreen::REPLACE); // Set replace cursor mode
   }

   resetty();                       // Reset the Window
   endwin();                        // Terminate the Window

   if( buffer == NULL )             // If not initialized
     return;                        // Nothing to do

   free(buffer);                    // Release the buffer
   buffer= NULL;                    // Indicate uninitialized

   #ifdef HCDM
     tracef("%8s= TextScreenAttr(%p)::~TextScreenAttr()\n", "Done", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Methode-
//       Attr::Attr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
inline
   Attr(
     TextScreen&     screen)
:  screen(screen)
,  buffer(NULL)
{
   unsigned            fg, bg;      // Foreground, background color index
   attr_t              color;       // The current color

   #ifdef HCDM
     tracef("%8s= TextScreenAttr(%p)::TextScreenAttr(%p)\n",
            "", this, &screen);
   #endif

   // Get screen handle
   dspH= initscr();                 // Initialize the Window
   initAttr= dspH->_attrs;          // Save initial attribute
   currAttr= initAttr;

   // Initialize the screen
   xSize= 0;
   ySize= 0;

   handleResizeEvent();

   // Initialize colors
   memset(attrArray, 0, sizeof(attrArray));
   if( has_colors() )               // If color is supported
   {
     start_color();                 // Initialize colors
     for(fg= 0; fg < 8; fg++)
     {
       for(bg= 0; bg < 8; bg++)
       {
         init_pair(bg*8+fg, fg, bg);
       }
     }

     // Initialize the color array
     // We basically have two choices on how to initialize attrArray
     // 1) Ignore the high order background bit,
     //    making all background colors dim. (Black is Black, White is grey)
     // 2) For background colors > 8, ignore the high order foreground bit,
     //    making all foreground colors bright.,
     #define USE_DUPLICATE_FOREGROUND true // Select (2)
     for(fg= 0; fg < 16; fg++)             // marker
     {
       for(bg= 0; bg < 16; bg++)
       {
         unsigned index= bg*16 + fg;
         int bx= VGA2BSD(bg & 0x0000007);
         int fx= VGA2BSD(fg & 0x0000007);

         color= A_BOLD;           // All colors default bold
         if( fg <= VGAColor::DarkGrey )
         {
           if( fg == bg )
             color= A_NORMAL;
           else
             color |= A_DIM;
         }

         if( bx == VGAColor::Black )
         {
           if( fg == VGAColor::Black )
             color= A_DIM;
           else if( fg == VGAColor::Grey )
             color= A_NORMAL;
         }

         color |= COLOR_PAIR(8*bx + fx);
         #if USE_DUPLICATE_FOREGROUND
           // We can't use DarkGrey as a background because the reversed
           // colors differ, leading to a motly background.
           if( bg&8 && bg != VGAColor::DarkGrey )
           {
             color= A_REVERSE | A_BOLD; // All colors reverse bold
             color |= COLOR_PAIR(8*fx + bx);
           }
         #endif

         attrArray[index]= color;
       }
     }
   }
}

   // Bitwise copy prohibited
   Attr(const Attr&) = delete;      // Disallowed copy constructor
   Attr& operator=(const Attr&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       Attr::addrXY
//
// Purpose-
//       Get screen buffer address
//
//----------------------------------------------------------------------------
inline Color::Char*                 // -> Screen buffer character
   addrXY(                          // Get screen buffer address
     unsigned int      col,         // Column position (X)
     unsigned int      row)         // Row position (Y)
{
   return buffer + (xSize*row) + col; // Return the buffer address
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::clearScreen
//
// Purpose-
//       Clear the screen.
//
//----------------------------------------------------------------------------
inline void
   clearScreen( void )              // Clear the screen.
{
   unsigned int        i;

   // Clear the logical buffer
   for(i=0; i<size; i++)            // Clear the screen
   {
     buffer[i].attr= currAttr;      // Set attribute
     buffer[i].data= ' ';           // Set data character
   }

   // Reposition the cursor
   physicalXY(0,0);                 // Reposition the cursor

   // Clear the (entire) physical buffer
   write(0, 0, columns-1, rows-1);  // Write the screen
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::handleResizeEvent
//
// Purpose-
//       Resize the buffer (and clear the screen).
//
//----------------------------------------------------------------------------
inline void
   handleResizeEvent( void )        // Resize the buffer
{
   WINDOW&             W= *dspH;    // Working Window

   int                 oldX;        // Old X-size
   int                 oldY;        // Old Y-size

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreenAttr(%p)::handleResizeEvent()\n", "", this);
   #endif

   // Save the old buffer size
   oldX= xSize;
   oldY= ySize;

   // Extract the buffer information
   xSize= W._maxx + 1;              // Use < rather than <=
   ySize= W._maxy + 1;

   //Just use the space visible in the window
   columns= xSize;
   rows=    ySize;
   size=    columns*rows;

   // If the backing buffer has expanded, free the old one
   if( oldX < xSize || oldY < ySize )
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
     buffer= (Color::Char*)must_malloc(sizeof(Color::Char)*ySize*xSize);
     memset(buffer, 0, sizeof(Color::Char)*ySize*xSize);

     #ifdef HCDM
       tracef("%p= TextScreenAttr(%p).buffer\n", buffer, this);
       tracef(">>  size(%4ld)\n", (long)sizeof(Color::Char)*ySize*xSize);
       tracef(">> xSize(%4d)\n", xSize);
       tracef(">> ySize(%4d)\n", ySize);
     #endif
   }

   clearScreen();
}

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
inline void
   physicalXY(                      // Set the cursor position
     unsigned int      col,         // Column number (X)
     unsigned int      row)         // Row number (Y)
{
   WINDOW&             W= *dspH;    // Working Window

   #if defined(HCDM) && FALSE
     tracef("%8s= TextScreenAttr(%p)::physicalXY(%d,%d)\n", "", this,
            col, row);
   #endif

   wmove(&W, row, col);             // Set the new cursor position
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::resume
//
// Purpose-
//       Resume operation.
//
//----------------------------------------------------------------------------
void
   resume( void )                   // Resume operation
{
   WINDOW&             W= *dspH;    // Working Window

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreenAttr(%p)::resume()\n", "", this);
   #endif

   wrefresh(&W);                    // Restore the Window
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::setAttribute
//
// Purpose-
//       Set the attribute character.
//
//----------------------------------------------------------------------------
inline void
   setAttribute(                    // Set screen attribute character
     Color::VGA        fg,          // Set this foreground color
     Color::VGA        bg)          // Set this background color
{
   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreenAttr(%p)::setAttribute(%d,%d)\n", "", this,
            fg, bg);
   #endif

   if( fg > VGAColor::MAXVGA || bg > VGAColor::MAXVGA )
   {
     screen.error(Terminal::ErrorColor);
     return;
   }
   currAttr= Color::Char::retAttribute(fg, bg);
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::setCursorMode
//
// Purpose-
//       Set the cursor mode.
//
//----------------------------------------------------------------------------
inline void
   setCursorMode(                   // Set the cursor mode
     TextScreen::CursorMode
                       mode)        // Use this mode
{
   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreenAttr(%p)::setCursorMode(%d)\n", "", this, mode);
   #endif

   switch( mode )
   {
     case TextScreen::INSERT:
       curs_set(2);
       break;

     case TextScreen::REPLACE:
       curs_set(1);
       break;

     default:
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::suspend
//
// Purpose-
//       Suspend operation.
//
//----------------------------------------------------------------------------
void
   suspend( void )                  // Suspend operation
{
   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreenAttr(%p)::suspend()\n", "", this);
   #endif

   endwin();                        // Suspend the Window
}

//----------------------------------------------------------------------------
//
// Method-
//       Attr::write
//
// Purpose-
//       Write the buffer on the screen.
//
//----------------------------------------------------------------------------
inline void                         // No return value
   write(                           // Write buffer to screen
     unsigned int      Lcol,        // Left column number
     unsigned int      Trow,        // Top row number
     unsigned int      Rcol,        // Right column number
     unsigned int      Brow)        // Bottom row number
{
   WINDOW&             W= *dspH;    // Working Window

   Color::Char*        buffer;      // Working buffer pointer
   int                 nextAttr;    // Next Attribute character
   int                 workChar;    // Working character
   int                 row;         // Working row
   int                 col;         // Working column

   #ifdef HCDM
     tracef("%8s= TextScreenAttr(%p)::write(%d,%d,%d,%d)\n", "", this,
            Lcol, Trow, Rcol, Brow);
   #endif

   for(row= Trow; row<=Brow; row++)
   {
     physicalXY(Lcol, row);
     buffer= addrXY(Lcol, row);

     #if defined(HCDM) && TRUE
       tracef("[%4d,%4d]", Lcol, row);
     #endif
     for(col= Lcol; col<=Rcol; col++)
     {
       nextAttr= attrArray[buffer->attr&0x00ff]; // Marker
       workChar= buffer->data & 0x00ff;
       if( workChar < ' ' )
         workChar= SUBSTITUTE;
       workChar |= nextAttr;
       #if defined(HCDM) && TRUE
         tracef(" [%.6x='%c']", workChar, workChar&0x00ff);
       #endif
       waddch(&W, workChar);
       buffer++;
     }
     #if defined(HCDM) && TRUE
       tracef("\n");
     #endif
   }

   wrefresh(&W);                    // Refresh the Window
}
}; // class Attr

//----------------------------------------------------------------------------
//
// Method-
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
// Method-
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
// Method-
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
// Method-
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
// Method-
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
// Method-
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
// Method-
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
// Method-
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
// Method-
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
// Method-
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
// Method-
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
// Method-
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
   L= vsnprintf(out, sizeof(out)-1, fmt, argptr); // Format the string
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
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::resume()\n", "", this);
   #endif

   attr.resume();
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
// Method-
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
   Attr&               attr= *(Attr*)this->attr;

   #if defined(HCDM) && TRUE
     tracef("%8s= TextScreen(%p)::suspend()\n", "", this);
   #endif

   attr.suspend();
}

//----------------------------------------------------------------------------
//
// Method-
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
     c= SUBSTITUTE;

   buff= attr.addrXY(currentCol, currentRow); // Get current position
   buff[0].attr= attr.currAttr;     // Set attribute
   buff[0].data= c;                 // Set data character

   attr.write(currentCol, currentRow, currentCol, currentRow);
   next();                          // Update the cursor position
}

//----------------------------------------------------------------------------
//
// Method-
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
         buff[0].attr= attr.currAttr; // Set attribute
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
// Method-
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
     buff[i].attr= attr.currAttr;   // Set the attribute character
     buff[i].data= buffer[i];       // Set the data character
   }

   for(i=size; i<attr.columns; i++) // Blank fill
   {
     buff[i].attr= attr.currAttr;   // Set attribute
     buff[i].data= ' ';             // Set data character
   }

   attr.write(0, row, attr.columns-1, row);
}

//----------------------------------------------------------------------------
//
// Method-
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
     buff[i].attr= attr.currAttr;   // Set attribute (from current)
     buff[i].data= ' ';             // Set data character
   }

   attr.write(0, row, attr.columns-1, row);
}

