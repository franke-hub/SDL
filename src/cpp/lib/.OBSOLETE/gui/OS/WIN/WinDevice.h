//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/WinDevice.h
//
// Purpose-
//       Graphical User Interface: Windows Device helpers
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_OSWIN_WINDEVICE_H_INCLUDED
#define GUI_OSWIN_WINDEVICE_H_INCLUDED

#include <windows.h>

#include <com/Mutex.h>
#include <com/Status.h>
#include <gui/Event.h>
#include <gui/Types.h>

#include "gui/Device.h"             // Base class
#include "com/Thread.h"             // Base class

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Status;

#include "gui/namespace.gui"
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class WinFont;

//----------------------------------------------------------------------------
//
// Class-
//       WinDevice
//
// Purpose-
//       Define system dependent implementation methods.
//
//----------------------------------------------------------------------------
class WinDevice : public Device, public Thread { // Windows Device
//----------------------------------------------------------------------------
// WinDevice::Attributes
//----------------------------------------------------------------------------
friend class WinFont;               // For full access

protected:                          // Friend not working
   // Prefix area
   char                prefix[16];  // Prefix area

   // Initialization/termination controls
   Status*             started;     // Run method initialization complete
   int                 operational; // TRUE while operational
   int                 operating;   // TRUE while thread operating

   // Offsets and lengths
   struct {
     XYValues            dpi;       // Resolution in dots per inch
     XYOffset            offset;    // The offset of the window (from move)
     XYLength            length;    // The size of the window (available)
     XYLength            window;    // The size of the window (actual)
   }                   device;      // The physical device

   struct {
     XYLength            bitmap;    // The length of the bitmap buffer
     XYOffset            offset;    // The offset of the window
     XYLength            length;    // The size of the window
     XYLength            window;    // The size of the window w/decoration
   }                   client;      // Client window

   struct {
     BOOL                active;    // TRUE iff scroll bars present
     XYOffset            offset;    // The offset (from the client origin)
     XYLength            length;    // The size of the window
     XYLength            window;    // The size of the window w/decoration
   }                   scroll;      // Scroll

   // Mutexes in priority order (e.g. if both needed, get status before bitmap)
   Mutex               eventsMutex; // Events access Mutex
   Mutex               statusMutex; // Status access Mutex
   Mutex               bitmapMutex; // Bitmap access Mutex

   // Windows device control
   HDC                 hdc;         // Device connection handle
   HINSTANCE           hinst;       // Application instance
   HBITMAP             hmap;        // Bitmap handle
   HPALETTE            hpal;        // Palette handle
   HWND                hwnd;        // Window handle

   // Suffix area
   char                suffix[16];  // Suffix area

//----------------------------------------------------------------------------
// WinDevice::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~WinDevice( void );              // Destructor
   WinDevice(                       // Constructor
     Window*           window);     // -> Source Window

//----------------------------------------------------------------------------
// WinDevice::Accessor methods
//----------------------------------------------------------------------------
public:
static WinDevice*                   // THE WinDevice singleton
   get( void );                     // Get WinDevice singleton

//----------------------------------------------------------------------------
// WinDevice::Overridden Thread methods
//----------------------------------------------------------------------------
public:
virtual long                        // Application completion code
   run( void );                     // Work processor

virtual long                        // The Thread completion code
   wait( void );                    // Wait for termination

//----------------------------------------------------------------------------
// WinDevice::Device methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   adjust(                          // Adjust the Device
     const XYOffset&   offset,      // Offset
     const XYLength&   length);     // Length

virtual void
   change(                          // Reflect buffer change
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

virtual const char*                 // Exception message (NULL OK)
   setAttribute(                    // Set attribute
     int               attribute,   // Attribute identifier
     int               value);      // Attribute value

//----------------------------------------------------------------------------
// WinDevice::Methods
//----------------------------------------------------------------------------
public:
virtual LRESULT                     // Resultant
   callback(                        // Windows callback routine
     HWND              hwnd,        // Window handle
     UINT              event,       // Event
     WPARAM            wParam,      // First message
     LPARAM            lParam);     // Second message

virtual void
   debug(                           // Diagnostic log
     int               line,        // Caller's line number
     const char*       message = NULL); // Associated message

virtual void
   handleEvent(                     // Handle event
     Event::EC         code,        // Event code
     unsigned          data,        // Event data
     const RECT&       rect);       // Location

virtual void
   initialize( void );              // Initialize the WinDevice

virtual int                         // The event code
   poll( void );                    // Poll and drive a single event

virtual void
   terminate( void );               // Terminate the WinDevice

protected:
virtual void
   render( void );                  // Render (paint) the window

virtual void
   resizeBitmap(                    // Update the bitmap size
     const XYLength&   length);     // To this size

virtual void
   updateScrolling(                 // Update scrolling
     const RECT&       rect);       // New scroll client window

virtual void
   updateScrollOffset(              // Update the scroll offset
     int               dx,          // X (width) adjustment
     int               dy);         // Y (height) adjustment
}; // class WinDevice

#include "gui/namespace.end"
#endif // GUI_OSWIN_WINDEVICE_H_INCLUDED
