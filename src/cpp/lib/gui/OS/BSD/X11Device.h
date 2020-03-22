//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/X11Device.h
//
// Purpose-
//       Graphical User Interface: Device implementation (X11).
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
// Only included from OS/BSD/Device.cpp

//----------------------------------------------------------------------------
//
// Class-
//       X11Device
//
// Purpose-
//       Define system dependent implementation methods.
//
//----------------------------------------------------------------------------
class X11Device : public Device  {  // X11 Device
friend class X11Thread;
//----------------------------------------------------------------------------
// X11Device::Attributes
//----------------------------------------------------------------------------
protected:
// Controls
int                    operational; // TRUE iff operational
X11Thread              thread;      // The client control thread
Mutex                  unitMutex;   // Prevents Device/Thread interaction

// Attributes
int                    kb_state;    // Keyboard state
Atom                   wm_delete;   // "WM_DELETE_WINDOW" Atom

// Offsets and lengths
struct {
  XYOffset               offset;    // The GUI::Window offset (from root)
  XYLength               length;    // The GUI::Window length
}                      device;      // The GUI::Window offset and length

struct {
  XYLength               length;    // The screen length
}                      screen;      // The screen length

// X11 Graphics
Display*               disp;        // Display handle
int                    xscr;        // Display screen
Visual*                xvis;        // Visual properties
Window                 xwin;        // X11 Window
GC                     xgco;        // Graphic context
XImage*                ximg;        // Image

//----------------------------------------------------------------------------
// X11Device::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~X11Device( void );              // Destructor
   X11Device(                       // Constructor
     GUI::Window*      window);     // -> Source Window

//----------------------------------------------------------------------------
// X11Device::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   setAttribute(                    // Device set attribute
     int               attribute,   // Attribute identifier
     int               value);      // Attribute value

virtual void
   change(                          // Device change
     const XYOffset&   offset,      // Offset
     const XYLength&   length);     // Length

virtual const char*                 // Exception message (NULL OK)
   move(                            // Device move
     const XYOffset&   offset);     // Offset

virtual const char*                 // Exception message (NULL OK)
   resize(                          // Device resize
     const XYLength&   length);     // Length

virtual long                        // (UNUSED) Return code
   wait( void );                    // Wait for Device termination

//----------------------------------------------------------------------------
// X11Device::Helper methods
//----------------------------------------------------------------------------
protected:
void
   config(                          // Configure the device
     const XYLength&   length);     // Length (of backing store)

void
   flush( void );                   // Wait for all pending XEvents

void
   expose(                          // Expose change
     const XYOffset&   offset,      // Offset
     const XYLength&   length);     // Length

void
   nextEvent(                       // XSelectInput, XNextEvent (diagnostic)
     XEvent&           e);          // (OUTPUT) Event
}; // class X11Device

