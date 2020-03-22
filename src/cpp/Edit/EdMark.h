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
//       EdMark.h
//
// Purpose-
//       Editor: Mark functions.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDMARK_H_INCLUDED
#define EDMARK_H_INCLUDED

#ifndef EDITOR_H_INCLUDED
#include "Editor.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       EdMark
//
// Purpose-
//       Editor mark.
//
//----------------------------------------------------------------------------
class EdMark                        // Editor mark descriptor
{
//----------------------------------------------------------------------------
// Edit::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum State                          // Mark state
{  FSM_RESET                        // No marker exists
,  FSM_LINES                        // Lines are marked
,  FSM_BLOCK                        // A block is marked
}; // enum State

//----------------------------------------------------------------------------
// EdMark::Constructors
//----------------------------------------------------------------------------
public:
   ~EdMark( void );                 // Destructor
   EdMark(                          // Constructor
     Editor*           editor);     // Editor object

//----------------------------------------------------------------------------
// EdMark::Methods
//----------------------------------------------------------------------------
public:
const char*                         // Return message (NULL OK)
   copy(                            // Copy the mark
     EdRing*           edRing,      // To this ring
     EdLine*           edLine,      // After this line
     unsigned          column);     // Using this column

const char*                         // Return message (NULL OK)
   format( void );                  // Format the mark

const char*                         // Return message (NULL OK)
   mark(                            // Create/expand/contract block mark
     EdRing*           edRing,      // In this ring
     EdLine*           edLine,      // Using this line
     int               column= (-1)); // Using this column

const char*                         // Return message (NULL OK)
   move(                            // Move the mark
     EdRing*           edRing,      // To this ring
     EdLine*           edLine,      // Using this line
     unsigned          column);     // Using this column

const char*                         // Return message (NULL OK)
   remove( void );                  // Remove the mark

void
   removeLine(                      // Prepare to remove lines
     const EdRing*     edRing,      // The Ring containing the lines
     const EdLine*     head,        // The first line to remove
     const EdLine*     tail);       // The final line to remove

void
   removeRing(                      // Remove a ring
     EdRing*           edRing);     // Using this ring

void
   reset( void );                   // Reset (undo) the mark

//----------------------------------------------------------------------------
// EdMark::Internal methods
//----------------------------------------------------------------------------
protected:
void
   removePrior(                     // Delete the old or current mark
     EdRing*           edRing,      // The ring that was marked
     EdLine*           head,        // The first line that was marked
     EdLine*           tail,        // The last line that was marked
     unsigned          left,        // Old left column
     unsigned          right);      // Old right column

const char*                         // Return message (NULL OK)
   verifyCopy(                      // Verify copy parameters
     EdRing*           edRing,      // To this ring
     EdLine*           edLine,      // After this line
     unsigned          column);     // Using this column

const char*                         // Return message (NULL OK)
   verifyMove(                      // Verify move parameters
     EdRing*           edRing,      // To this ring
     EdLine*           edLine,      // After this line
     unsigned          column);     // Using this column

//----------------------------------------------------------------------------
// EdMark::Debugging methods
//----------------------------------------------------------------------------
public:
void
   check( void ) const;             // Debugging check

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// EdMark::Attributes
//----------------------------------------------------------------------------
public:                             // (EdView)
   Editor*             const edit;  // Associated Editor
   unsigned            state;       // Current State
   EdRing*             ring;        // Marked ring
   EdLine*             first;       // First marked line
   unsigned            left;        // Mark left-hand column
   unsigned            right;       // Mark right-hand column

   EdLine*             touchLine;   // Last line marked
   unsigned            touchCol;    // Last column marked
}; // class EdMark

#endif // EDMARK_H_INCLUDED
