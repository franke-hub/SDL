//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Event.h
//
// Purpose-
//       Graphical User Interface: Event descriptor
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_EVENT_H_INCLUDED
#define GUI_EVENT_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"                  // This include is guaranteed
#endif

#include "namespace.gui"            // Graphical User Interface
//----------------------------------------------------------------------------
//
// Class-
//       Event
//
// Purpose-
//       Event descriptor.
//
//----------------------------------------------------------------------------
class Event {                       // Event descriptor
//----------------------------------------------------------------------------
// Event::Enumerations
//----------------------------------------------------------------------------
public:
enum EC                             // Event code
{  EC_TERMINATE= (-1)               // Window termination event
,  EC_SIGNAL= 0                     // GENERIC event
,  EC_RESIZE                        // RESIZE event
,  EC_GLOBAL= EC_RESIZE             // (Highest global event)
,  EC_KEYDOWN                       // Key press event
,  EC_KEYUP                         // Key release event
,  EC_MOUSEDOWN                     // Mouse press event
,  EC_MOUSEOVER                     // Mouse over event
,  EC_MOUSEUP                       // Mouse release event
};

enum KD                             // Key data
{  KD_LEFT_ALT=          0x00000101 // Left ALT key
,  KD_LEFT_CTRL=         0x00000102 // Left control key
,  KD_LEFT_SHIFT=        0x00000103 // Left shift key
,  KD_RIGHT_ALT=         0x00000201 // Right ALT key
,  KD_RIGHT_CTRL=        0x00000202 // Right control key
,  KD_RIGHT_SHIFT=       0x00000203 // Right shift key
};

enum MD                             // Mouse data
{  MD_LEFT= (-1)                    // LEFT mouse button
,  MD_MIDDLE= 0                     // MIDDLE mouse button
,  MD_RIGHT= (+1)                   // RIGHT mouse button
};

enum MO                             // Mouse over
{  MO_EXIT= (-1)                    // Mouse exit
,  MO_OVER= 0                       // Mouse position
,  MO_ENTER= (+1)                   // Mouse entry
};

//----------------------------------------------------------------------------
// Event::Attributes
//----------------------------------------------------------------------------
public:                             // PUBLIC attributes
EC                     code;        // The event code
int                    data;        // The event data
XYOffset               offset;      // Offset of Event
XYLength               length;      // Length of Event

//----------------------------------------------------------------------------
// Event::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Event( void );                  // Destructor
inline
   Event( void );                   // Default Constructor (No initialization)

inline
   Event(                           // Constructor
     EC                code,        // The event code
     int               data,        // The event data
     const XYOffset&   offset,      // Set Offset
     const XYLength&   length);     // Set Length

//----------------------------------------------------------------------------
// Event::Methods
//----------------------------------------------------------------------------
public:
inline EC                           // The Event code
   getCode( void ) const;           // Get Event code

inline int                          // The Event data
   getData( void ) const;           // Get Event data

inline const XYLength&              // XYLength
   getLength( void ) const;         // Get Length of Event

inline const XYOffset&              // XYOffset
   getOffset( void ) const;         // Get Offset of Event

inline void
   setCode(                         // Set the Event code
     EC                code);       // The event code

inline void
   setData(                         // Set the Event data
     int               data);       // The event data

inline void
   setEvent(                        // Set all Event attributes
     EC                code,        // The event code
     int               data,        // The event data
     const XYOffset&   offset,      // Set Offset
     const XYLength&   length);     // Set Length

inline void
   setLength(                       // Set Length of Event
     const XYLength&   length);     // Set Length

inline void
   setOffset(                       // Set Offset of Event
     const XYOffset&   offset);     // Set Offset

//----------------------------------------------------------------------------
//
// Public method-
//       Event::debug
//
// Purpose-
//       Diagnostic debug.
//
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Diagnostic debug
}; // class Event
#include "Event.i"
#include "namespace.end"

#endif // GUI_EVENT_H_INCLUDED
