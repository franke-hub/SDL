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
//       Status.h
//
// Purpose-
//       Editor: Status, display Ring status/message wrapper.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

#ifndef EDITOR_H_INCLUDED
#include "Editor.h"
#endif

#ifndef EDDRAW_H_INCLUDED
#include "EdDraw.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Status
//
// Purpose-
//       Editor control object.
//
// Notes-
//       The Status object determines whether the underlying status has
//       changed and, except for RESHOW_ALL, does not need to be called when
//       status changes occur.
//
//----------------------------------------------------------------------------
class Status : public EdDraw {      // Editor control object
//----------------------------------------------------------------------------
// Status::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
// Status::Constructors
//----------------------------------------------------------------------------
public:
   ~Status( void );                 // Destructor
   Status(                          // Constructor
     Editor*           parent);     // Editor object

//----------------------------------------------------------------------------
// Status::Methods
//----------------------------------------------------------------------------
public:
const char*                         // Return message (NULL)
   defer(                           // Deferred reshow
     ReshowType        type);       // Reshow type

const char*                         // Return message (NULL)
   display( void );                 // Physically reshow all views

int                                 // TRUE iff ring or active changed
   isChanged( void );               // Is the file in a changed state?

int                                 // Reply response character
   message(                         // Put message on status line
     MsgFsm            level,       // Message level
     const char*       string);     // The message

const char*                         // Return message (NULL OK)
   resize(                          // Resize event
     unsigned          cols,        // Number of columns
     unsigned          rows);       // Number of rows

const char*                         // (The format string)
   warning(                         // Put warning on status line
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF arguments

//----------------------------------------------------------------------------
// Status::Debugging methods
//----------------------------------------------------------------------------
public:
void
   check( void ) const;             // Debugging check

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// Status::Attributes
//----------------------------------------------------------------------------
protected:
   //-------------------------------------------------------------------------
   // View controls (static)
   Editor*             const edit;  // Editor object

   unsigned int        cols;        // Number of physical display columns
   unsigned int        rowSts;      // Row index: Status line
   unsigned int        rowMsg;      // Row index: Help/message line

   Color::VGA          msgDisp[CS_MAX]; // Display line attribute
   Color::VGA          msgInfo[CS_MAX]; // Message line info  attribute
   Color::VGA          msgWarn[CS_MAX]; // Message line warn  attribute
   Color::VGA          msgErrs[CS_MAX]; // Message line error attribute

   Color::VGA          stsNorm[CS_MAX]; // Status line normal attribute
   Color::VGA          stsChng[CS_MAX]; // Status line change attribute
   Color::VGA          stsErrs[CS_MAX]; // Status line damage attribute

   //-------------------------------------------------------------------------
   // Message controls (dynamic)
   EdRing*             ring;        // Status Ring
   unsigned int        column;      // Ring Column position
   unsigned int        row;         // Ring Row position
   int                 mode;        // Ring mode
   unsigned int        rows;        // Ring Row count
   unsigned char       changed;     // Ring was changed
   unsigned char       damaged;     // Ring was damaged
   unsigned char       insertKey;   // Insert key status

   unsigned char       deferMsg;    // TRUE if reshow(message)
   unsigned char       deferSts;    // TRUE if reshow(status)
   MsgFsm              msgState;    // Message/status line state
   char                msgLine[128];// Message line
}; // class Status

#include "Status.i"

#endif // STATUS_H_INCLUDED
