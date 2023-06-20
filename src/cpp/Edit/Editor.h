//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Editor.h
//
// Purpose-
//       Editor: Compilation and prerequisite controls.
//
// Last change date-
//       2023/06/20 (Version 2, Release 2)
//
//----------------------------------------------------------------------------
#ifndef EDITOR_H_INCLUDED
#define EDITOR_H_INCLUDED

#include <com/Color.h>
#include <com/Debug.h>
#include <com/FileInfo.h>
#include <com/FileList.h>
#include <com/FileName.h>
#include <com/Handler.h>
#include <com/List.h>
#include <com/Pool.h>
#include <com/Terminal.h>

#ifndef EDDRAW_H_INCLUDED
#include "EdDraw.h"
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define EDIT_VERSION    "EDIT V2.1" // Editor version
#define SHOULD_NOT_OCCUR      FALSE // Used in asserts

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Operating system controls
//----------------------------------------------------------------------------
#ifdef _OS_WIN
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Active;                       // Active line
class EdDraw;                       // Drawing base class
class EdHand;                       // Terminal exception handler
class EdLine;                       // Line
class EdMark;                       // Mark
class EdPool;                       // Pool
class EdRing;                       // Ring
class EdView;                       // View
class Status;                       // Status

//----------------------------------------------------------------------------
//
// Class-
//       Editor
//
// Purpose-
//       Editor object.
//
//----------------------------------------------------------------------------
class Editor : public EdDraw {      // Editor object
//----------------------------------------------------------------------------
// Edit::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum
{  MAX_ACTIVE= 512                  // Largest built-in active string
,  MAX_CHANGE= 512                  // Largest change string
,  MAX_LOCATE= 512                  // Largest locate string
,  MAX_TABS= 32                     // Number of possible tabs
}; // enum

//----------------------------------------------------------------------------
// Editor::Constructors
//----------------------------------------------------------------------------
public:
   ~Editor( void );                 // Destructor
   Editor( void );                  // Constructor

//----------------------------------------------------------------------------
// Editor::Methods
//----------------------------------------------------------------------------
public:
const char*                         // Return message (NULL OK)
   activate(                        // Activate Line
     EdLine*           edLine);     // Using this Line

const char*                         // Return message (NULL OK)
   activate(                        // Activate Ring
     EdRing*           edRing);     // Using this Ring

const char*                         // Return message (NULL OK)
   change( void );                  // Change

const char*                         // Return message (NULL OK)
   commit( void );                  // Commit data changes

const char*                         // Return message (NULL)
   defer(                           // Deferred reshow
     ReshowType        type);       // Reshow type

const char*                         // Return message (NULL)
   display( void );                 // Physically reshow all views

const char*                         // Return message (NULL OK)
   execute( void );                 // Execute command

const char*                         // Return message (NULL)
   focus(                           // Set focus
     EdView*           edView);     // Using this View

const char*                         // Return message (NULL OK)
   histInsert( void );              // Insert a history Line

#if 0
const char*                         // Return message (NULL OK)
   insertLine(                      // Insert lines into Ring
     EdRing*           edRing,      // The Ring containing the lines
     EdLine*           head,        // The first line to insert
     EdLine*           tail);       // The final line to insert
#endif

EdLine*
   insertLine( void );              // Insert a new, empty Line into dataView

const char*                         // Return message (NULL OK)
   insertRing(                      // Insert a new ring
     const char*       fileName);   // Using this filename

const char*                         // Return message (NULL OK)
   lineJoin( void );                // Join data Line

const char*                         // Return message (NULL OK)
   lineSplit( void );               // Split data Line

const char*                         // Return message (NULL OK)
   locate(                          // Locate
     int               change= 0);  // Non-zero to locate for change

const char*                         // Updated position pointer
   parser(                          // Command parser
     const char*       ptrchs,      // Current position
     char*             string);     // Output string

const char*                         // Return message (NULL OK)
   redo( void );                    // Redo last undo (NOT IMPLEMENTED)

const char*                         // Return message (NULL OK)
   removeLine(                      // Remove lines from Ring
     EdRing*           edRing,      // The Ring containing the lines
     EdLine*           head,        // The first line to remove
     EdLine*           tail);       // The final line to remove

const char*                         // Return message (NULL OK)
   removeLine( void );              // Remove the current dataView line

const char*                         // Return message (NULL OK)
   removeRing(                      // Remove (and delete) the Ring
     EdRing*           edRing);     // The Ring to remove

const char*                         // Return message (NULL OK)
   resize(                          // Resize screen
     unsigned          cols,        // Number of columns
     unsigned          rows);       // Number of rows

void
   resize( void );                  // Resize screen

void
   run( void );                     // Run in interactive mode

const char*                         // Return message (NULL OK)
   safeExit(                        // Safely remove a ring
     EdRing*           edRing);     // This ring

const char*                         // Return message (NULL OK)
   safeSave(                        // Safely write a ring
     EdRing*           edRing);     // This ring

unsigned                            // Next tab stop
   tabLeft(                         // Tab left
     unsigned          column);     // From this column

unsigned                            // Next tab stop
   tabRight(                        // Tab right
     unsigned          column);     // From this column

const char*                         // Return message (NULL OK)
   undo( void );                    // Undo last remove

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
   viewChange(                      // Update views after change
     EdRing*           edRing,      // For this ring
     EdLine*           head,        // First change line
     EdLine*           tail);       // Last change line

const char*                         // (The input format string)
   warning(                         // Write warning message
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF arguments

//----------------------------------------------------------------------------
// Editor::Debugging methods
//----------------------------------------------------------------------------
public:
void
   check( void ) const;             // Debugging check

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// Editor::Attributes
//----------------------------------------------------------------------------
public:
   //-------------------------------------------------------------------------
   // Active controls
   Active*             dataActive;  // The data Active object
   Active*             histActive;  // The history Active object
   Active*             workActive;  // The working Active object

   //-------------------------------------------------------------------------
   // Ring Controls
   List<EdRing>        ringList;    // Ring list anchor
   EdRing*             histRing;    // The history Ring
   EdRing*             utilRing;    // The utility Ring

   //-------------------------------------------------------------------------
   // Viewing controls
   List<EdView>        viewList;    // View list anchor
   EdView*             dataView;    // The current data View
   EdView*             histView;    // The history View
   EdView*             workView;    // The view with focus
   unsigned            viewCount;   // The number of data Views

   EdHand*             handler;     // Terminal exception handler
   EdMark*             mark;        // The mark
   Status*             status;      // The Status

   //-------------------------------------------------------------------------
   // Locate/Change strings
   size_t              changeLength;// Change string length
   size_t              locateLength;// Locate string length
   char                changeString[MAX_CHANGE]; // Change string
   char                locateString[MAX_LOCATE]; // Locate string

   //-------------------------------------------------------------------------
   // Controls
   unsigned int        online;      // TRUE iff operational
   unsigned int        marginLeft;  // Left margin (0 origin)
   unsigned int        marginRight; // Right margin (0 origin)
   unsigned int        tabUsed;     // Number of tabs used
   unsigned int        tabStop[MAX_TABS]; // Tab stop array
}; // class Editor

#endif // EDITOR_H_INCLUDED
