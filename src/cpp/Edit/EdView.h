//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdView.h
//
// Purpose-
//       Editor: Viewer.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDVIEW_H_INCLUDED
#define EDVIEW_H_INCLUDED

#ifndef EDITOR_H_INCLUDED
#include "Editor.h"
#endif

#ifndef EDDRAW_H_INCLUDED
#include "EdDraw.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       EdView
//
// Purpose-
//       Editor viewer object.
//
//----------------------------------------------------------------------------
class EdView : public List<EdView>::Link, public EdDraw { // Editor viewer object
//----------------------------------------------------------------------------
// EdView::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~EdView( void );                 // Destructor
   EdView(                          // Constructor
     Editor*           parent,      // Editor object
     Active*           active);     // Initial Active object

//----------------------------------------------------------------------------
// EdView::Accessor methods
//----------------------------------------------------------------------------
public:
inline Active*                      // The associated Active
   getActive( void ) const;         // Get associated Active

inline void
   setActive(                       // Set associated Active
     Active*           active);     // The associated Active

inline unsigned                     // The current column
   getColumn( void ) const;         // Get current column

inline EdLine*                      // The current line
   getLine( void ) const;           // Get current line

inline EdRing*                      // The current ring
   getRing( void ) const;           // Get current ring

inline unsigned                     // The current row
   getRow( void ) const;            // Get current row

inline int                          // True iff data view
   isDataView( void ) const;        // Is this the data view?

inline int                          // True iff history view
   isHistView( void ) const;        // Is this the history view?

//----------------------------------------------------------------------------
// EdView::Methods
//----------------------------------------------------------------------------
public:
EdLine*                             // The new current Line
   activate(                        // Activate Line
     EdLine*           edLine);     // Using this EdLine

EdLine*                             // The new current Line
   activate(                        // Activate Ring
     EdRing*           edRing);     // Using this EdRing

const char*                         // Return message (NULL OK)
   column(                          // Set column position
     int               left,        // Left column
     int               right= (-1));// Right column (precedence if specified)

const char*                         // Return message (NULL)
   defer(                           // Deferred reshow
     unsigned int      row);        // This row

const char*                         // Return message (NULL)
   defer(                           // Deferred reshow
     ReshowType        type);       // Reshow type

const char*                         // Return message (NULL)
   display( void );                 // Physical display

EdLine*                             // The new current Line
   moveDown( void );                // Move the view down one row

EdLine*                             // The new current Line
   moveFirst( void );               // Move view to top of ring

EdLine*                             // The new current Line
   moveLast( void );                // Move view to bottom of ring

const char*                         // Return message (NULL OK)
   moveLeft( void );                // Move the view left one column

const char*                         // Return message (NULL OK)
   moveRight( void );               // Move the view right one column

EdLine*                             // The new current Line
   moveUp( void );                  // Move the view up one row

const char*                         // Return message (NULL OK)
   resize(                          // Resize screen
     unsigned          cols,        // Number of columns
     unsigned          rows);       // Number of rows

EdLine*                             // The new current Line
   screenDown( void );              // Move the view down one screen

EdLine*                             // The new current Line
   screenTop( void );               // Move the current row to the top

EdLine*                             // The new current Line
   screenUp( void );                // Move the view up one screen

void
   synch( void );                   // Internal synchronization

EdLine*                             // The new current Line
   synchFetch(                      // Restore settings
     EdRing*           edRing);     // Using this EdRing

const char*                         // Return message (NULL)
   synchStore( void );              // Save settings

const char*                         // Return message (NULL)
   viewChange(                      // Update views after change
     const EdRing*     edRing,      // For this Ring
     const EdLine*     edLine);     // For this Line

const char*                         // Return message (NULL)
   viewChange(                      // Update views after change
     const EdRing*     edRing,      // For this Ring
     const EdLine*     edLine,      // For this Line
     const unsigned    column);     // For this column

const char*                         // Return message (NULL)
   viewChange(                      // Update view after line change
     const EdRing*     edRing,      // For this ring
     const EdLine*     head,        // First change line
     const EdLine*     tail);       // Last change line

//----------------------------------------------------------------------------
// EdView::Debugging methods
//----------------------------------------------------------------------------
public:
void
   check( void ) const;             // Debugging check

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// EdView::Internal methods
//----------------------------------------------------------------------------
protected:
const char*                         // Return message (NULL)
   display(                         // Physical reshow of data row
     unsigned          row,         // Row number
     const EdLine*     edLine) const; // -> EdLine

//----------------------------------------------------------------------------
// EdView::Attributes
//----------------------------------------------------------------------------
protected:
   //-------------------------------------------------------------------------
   // Constructor objects
   Editor*             const edit;  // -> Editor

   //-------------------------------------------------------------------------
   // View controls (static)
   unsigned int        vid;         // View identifier
   unsigned int        zcol;        // Physical display column zero
   unsigned int        zrow;        // Physical display row zero
   unsigned int        cols;        // Number of physical display columns
   unsigned int        rows;        // Number of physical display rows

   unsigned int        rowMin;      // Row index: Minimum data line
   unsigned int        rowMax;      // Row index: Maximum data line

   Color::VGA          bufNorm[CS_MAX]; // File line normal attribute
   Color::VGA          bufMark[CS_MAX]; // File line marked attribute

   //-------------------------------------------------------------------------
   // View controls (dynamic)
   Active*             active;      // -> Active Object

   unsigned int        firstCol;    // First screen column (0 origin)
   unsigned int        firstRow;    // First screen row    (0 origin)
   EdLine*             firstLine;   // First screen line

   unsigned int        curCol;      // Current file column offset
   unsigned int        curRow;      // Current file row    offset from minRow
   EdLine*             curLine;     // Current line
   EdRing*             curRing;     // Current ring

   unsigned int        deferRow;    // row+1 if reshow(line)
   unsigned char       deferBuf;    // TRUE if reshow(buffer)
}; // class EdView

#include "EdView.i"

#endif // EDVIEW_H_INCLUDED
