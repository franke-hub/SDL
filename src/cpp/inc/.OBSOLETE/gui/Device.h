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
//       Device.h
//
// Purpose-
//       Graphical User Interface: Device
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_DEVICE_H_INCLUDED
#define GUI_DEVICE_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"
#endif

#ifndef GUI_EVENT_H_INCLUDED
#include "Event.h"                  // For Event::EC
#endif

#ifndef GUI_OBJECT_H_INCLUDED
#include "Object.h"                 // For Object::Attribute
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Window;

//----------------------------------------------------------------------------
//
// Class-
//       Device
//
// Purpose-
//       Abstract Device.
//
// Description-
//       The Device object is used internally within the GUI subsystem.
//       It is the mechanism used to physically expose a Window's content.
//
//----------------------------------------------------------------------------
class Device : public Attributes {  // System Dependent Implementation
//----------------------------------------------------------------------------
// Device::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum Attribute                      // The attribute list
{  VISIBLE                          // Visible
,  TRANSPARENT                      // Transparent
,  ATTRIBUTE_COUNT                  // Number of attributes
}; // enum Attribute

//----------------------------------------------------------------------------
// Device::Attributes
//----------------------------------------------------------------------------
protected:
Window*                window;      // -> Window

//----------------------------------------------------------------------------
// Device::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Device( void );                 // Destructor

protected:
   Device(                          // Default constructor
     Window*           window);     // Source Window

public:
static Device*                      // -> Device
   make(                            // Factory method
     Window*           window);     // Source Window

//----------------------------------------------------------------------------
// Device::Methods
//----------------------------------------------------------------------------
protected:
virtual void
   callback(                        // Window callback
     const Event&      event);      // For this Event

virtual void
   callback(                        // Window callback
     Event::EC         code,        // The event code
     int               data,        // The event data
     const XYOffset&   offset,      // Set Offset
     const XYLength&   length);     // Set Length

public:
virtual void
   change(                          // Device change
     const XYOffset&   offset,      // Offset
     const XYLength&   length);     // Length

virtual void
   debug( void );                   // Diagnostic debug

virtual const char*                 // Exception message (NULL OK)
   move(                            // Reposition the Device
     const XYOffset&   offset);     // Offset (from origin)

virtual const char*                 // Exception message (NULL OK)
   resize(                          // Resize the Device
     const XYLength&   length);     // Resize length

virtual long                        // (UNUSED) Return code
   wait( void );                    // Wait for Device termination
}; // class Device
#include "namespace.end"

#endif // GUI_DEVICE_H_INCLUDED
