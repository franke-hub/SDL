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
//       TextScreen.h
//
// Purpose-
//       Text screen control.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef TEXTSCREEN_H_INCLUDED
#define TEXTSCREEN_H_INCLUDED

#ifndef COLOR_H_INCLUDED
#include "Color.h"
#endif

#ifndef HANDLER_H_INCLUDED
#include "Handler.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       TextScreen
//
// Purpose-
//       Text screen control.
//
// Notes-
//       Rows    are numbered from 0 (top)  to getYSize()-1,
//       Columns are numbered from 0 (left) to getXSize()-1.
//
//----------------------------------------------------------------------------
class TextScreen : virtual public Handler { // Text screen control
//----------------------------------------------------------------------------
// TextScreen::Attributes
//----------------------------------------------------------------------------
private:
   void*               attr;        // Hidden attributes

   unsigned int        currentCol;  // Current column
   unsigned int        currentRow;  // Current row

//----------------------------------------------------------------------------
// TextScreen::Enumerations
//----------------------------------------------------------------------------
public:
enum CursorMode                     // Cursor mode
{
   REPLACE,                         // Small cursor
   INSERT                           // Large cursor
}; // enum CursorMode

//----------------------------------------------------------------------------
// TextScreen::Constructors
//----------------------------------------------------------------------------
public:
   ~TextScreen( void );             // Destructor
   TextScreen( void );              // Constructor

private:                            // Bitwise copy prohibited
   TextScreen(const TextScreen&);   // Disallowed copy constructor
   TextScreen& operator=(const TextScreen&); // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::alarm
//
// Purpose-
//       Sound the alarm.
//
//----------------------------------------------------------------------------
public:
void
   alarm( void );                   // Sound the alarm

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::clearScreen
//
// Purpose-
//       Clear the screen (to blanks).
//       The logical and physical positions are set to (top, left).
//
//----------------------------------------------------------------------------
void
   clearScreen( void );             // Clear the screen

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::deleteRow
//
// Purpose-
//       The screen is scrolled up, duplicating the last row.
//
//----------------------------------------------------------------------------
void
   deleteRow(                       // Delete screen row
     unsigned int      toprow);     // Delete this row

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::deleteRow
//
// Purpose-
//       The screen is scrolled up, duplicating botrow.
//
//----------------------------------------------------------------------------
void
   deleteRow(                       // Delete screen row
     unsigned int      toprow,      // Delete this row
     unsigned int      botrow);     // Last row to modify

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::getCurrentCol
//
// Purpose-
//       Return the current screen column.
//
//----------------------------------------------------------------------------
inline unsigned int                 // The current screen column
   getCurrentCol( void );           // Get the current screen column

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::getCurrentRow
//
// Purpose-
//       Return the current screen row.
//
//----------------------------------------------------------------------------
inline unsigned int                 // The current screen row
   getCurrentRow( void );           // Get the current screen row

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
   getXSize( void );                // Get number of screen columns

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
   getYSize( void );                // Get number of screen rows

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::handleResizeEvent
//
// Purpose-
//       Resize the buffer (and clear the screen).
//       This is required in environments where a resize event is not
//       detected by the TextScreen object.
//
//----------------------------------------------------------------------------
void
   handleResizeEvent( void );       // Resize the buffer

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::insertRow
//
// Purpose-
//       The screen is scrolled down, duplicating toprow.
//
//----------------------------------------------------------------------------
void
   insertRow(                       // Insert screen row
     unsigned int      toprow);     // Roll this row down one row

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::insertRow
//
// Purpose-
//       The screen is scrolled down to botrow, duplicating toprow.
//
//----------------------------------------------------------------------------
void
   insertRow(                       // Insert screen row
     unsigned int      toprow,      // Roll this row down one row
     unsigned int      botrow);     // Last row to modify

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::logicalXY
//
// Purpose-
//       Set the logical column and row position.
//
//----------------------------------------------------------------------------
void
   logicalXY(                       // Set logical column, row coordinate
     unsigned int      col,         // Column number
     unsigned int      row);        // Row number

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::physicalXY
//
// Purpose-
//       Set the cursor position.
//       This is the only mechanism for changing the cursor position.
//
//----------------------------------------------------------------------------
void
   physicalXY(                      // Set the cursor position
     unsigned int      col,         // Column number (X)
     unsigned int      row);        // Row number (Y)

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::printf
//
// Purpose-
//       Write to screen at the current logical position.
//
//----------------------------------------------------------------------------
void
   printf(                          // Screen printf facility
     const char*       fmt,         // The PRINTF format string
                       ...);        // The remaining arguments

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
   resume( void );                  // Resume operation

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::setAttribute
//
// Purpose-
//       Set the default attributes.
//
//----------------------------------------------------------------------------
public:
void
   setAttribute(                    // Set screen attribute character
     Color::VGA        fg,          // Set this foreground color
     Color::VGA        bg);         // Set this background color

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::setCursorMode
//
// Purpose-
//       Set the cursor mode.
//
//----------------------------------------------------------------------------
void
   setCursorMode(                   // Set screen cursor mode
     CursorMode        mode);       // Use this mode

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
   suspend( void );                 // Suspend operation

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::wr
//
// Purpose-
//       Write to screen at current logical position.
//       The logical position is updated.
//
//----------------------------------------------------------------------------
void
   wr(                              // Write screen character
     char              c);          // Data character

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::wr
//
// Purpose-
//       Write to screen at current logical position.
//       The logical position is updated.
//
//----------------------------------------------------------------------------
void
   wr(                              // Write screen
     const char*       buffer,      // -> Character string
     unsigned int      buflen);     // Sizeof(buffer)

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::wr
//
// Purpose-
//       Write to screen at current logical position.
//       The logical position is updated.
//
//----------------------------------------------------------------------------
void
   wr(                              // Write screen
     const Color::Char*
                       buffer,      // -> Color::Char string
     unsigned int      buflen);     // Number of buffer elements

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::wr
//
// Purpose-
//       Write to screen at current logical position.
//       The logical position is updated.
//
//----------------------------------------------------------------------------
void
   wr(                              // Write screen
     const char*       buffer);     // -> Character string

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::wr
//
// Purpose-
//       Write to screen at (row, 0).
//       Fills with blanks, truncates at end of row.
//       The logical position remains unchanged.
//
//----------------------------------------------------------------------------
void
   wr(                              // Write screen
     unsigned int      row,         // row position (column 0)
     const char*       buffer,      // -> Character string
     unsigned int      buflen);     // Sizeof(buffer)

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::wr
//
// Purpose-
//       Write to screen at (row, 0).
//       Fills with blanks, truncates at end of row.
//       The logical position remains unchanged.
//
//----------------------------------------------------------------------------
void
   wr(                              // Write screen
     unsigned int      row,         // row position (column 0)
     const char*       buffer);     // -> Character string

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::wr
//
// Purpose-
//       Write to screen at (row, 0).
//       Fills with blanks, truncates at end of row.
//       The logical position remains unchanged.
//
//----------------------------------------------------------------------------
void
   wr(                              // Write screen
     unsigned int      row,         // row position (column 0)
     const Color::Char*
                       buffer,      // -> Color::Char string
     unsigned int      buflen);     // Number of buffer elements

private:
inline void
   next( void );                    // Increment position
}; // class TextScreen

#include "TextScreen.i"

#endif // TEXTSCREEN_H_INCLUDED
