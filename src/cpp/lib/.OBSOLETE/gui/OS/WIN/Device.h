//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/Device.h
//
// Purpose-
//       Graphical User Interface: Windows Device helpers
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_OSWIN_DEVICE_H_INCLUDED
#define GUI_OSWIN_DEVICE_H_INCLUDED

#include <windows.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef VERBOSE
#define VERBOSE 1                   // Verbosity
#endif

#define ID_MENU_ABOUT           101 // ABOUT menu item
#define ID_ABOUT_BOX           1001 // ABOUT dialog item

#define MIN_WINDOW_X            128 // Minimum window width
#define MIN_WINDOW_Y            128 // Minimum window height

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define VERSION_ID "2.001"
#define WINCHECK(rc, function) wincheck(__LINE__, (void*)(intptr_t)rc, function)
#define WINDEBUG(rc, function) windebug(__LINE__, (void*)(intptr_t)rc, function)

#include "gui/namespace.gui"
//----------------------------------------------------------------------------
//
// Class-
//       GUI::ObjectSelector
//
// Purpose-
//       Implement MSWindows SelectObject, restoring the selection when
//       the ObjectSelector is deleted.
//
//----------------------------------------------------------------------------
class ObjectSelector {
public:
   HDC                 hdc;         // The device context
   HGDIOBJ             hobj;        // The originally selected object

   ~ObjectSelector( void )
{
   SelectObject(hdc, hobj);         // Restore
}

   ObjectSelector(
     HDC               hdc,         // The device context
     HGDIOBJ           hobj)        // The object to be selected
:  hdc(hdc)
{
   this->hobj= SelectObject(hdc, hobj); // Select
}
}; // class ObjectSelector

//----------------------------------------------------------------------------
//
// Class-
//       GUI::PaletteSelector
//
// Purpose-
//       Implement MSWindows SelectPalette, restoring the selection when
//       the PaletteSelector is deleted.
//
//----------------------------------------------------------------------------
class PaletteSelector
{
public:
   HDC                 hdc;         // The device context
   HPALETTE            hpal;        // The originally selected palette

   ~PaletteSelector()
{
   SelectPalette(hdc, hpal, FALSE); // Restore the Palette
   RealizePalette(hdc);
}

   PaletteSelector(
     HDC               hdc,         // The device context
     HPALETTE          hpal)        // The palette to select
:  hdc(hdc)
{
   this->hpal= SelectPalette(hdc, hpal, FALSE); // Select the Palette
   RealizePalette(hdc);
}
}; // class PaletteSelector

//----------------------------------------------------------------------------
//
// Class-
//       GUI::TempDC
//
// Purpose-
//       Create a temporary device context handle, releasing it when
//       the TempDC is deleted.
//
//----------------------------------------------------------------------------
class TempDC {
public:
   HDC                 hdc;         // The temporary device context
   HWND                hwnd;        // The associated window handle

   ~TempDC()
{
   ReleaseDC(hwnd, hdc);            // Release the DC handle
}

   TempDC(
     HWND              hwnd)        // The associated window handle
:  hwnd(hwnd)
{
   hdc= GetDC(hwnd);                // Create a temporary DC handle
}

   operator HDC() const             // Cast to HDC
{
   return hdc;
}
}; // class TempDC

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::checkstop
//
// Purpose-
//       Handle checkstop (abort) condition.
//
//----------------------------------------------------------------------------
extern void
   checkstop(                       // Check stop
     int               line,        // Line number
     const char*       name,        // Function name
     int               code);       // Error code

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::code2name
//
// Purpose-
//       Convert callback code to callback name.
//
//----------------------------------------------------------------------------
extern const char*                  // Resultant (codename)
   code2name(                       // Get name
     int               code);       // For this WM_code

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::logRect
//
// Purpose-
//       Log a rectangle.
//
//----------------------------------------------------------------------------
extern void
   logRect(                         // Log a rectangle
     int               line,        // Caller's line number
     RECT&             rect);       // The rectangle

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::max
//
// Purpose-
//       Integer maxiumum.
//
//----------------------------------------------------------------------------
#undef max
static inline int                   // Resultant
   max(                             // Integer maximum
     int               a,           // Integer
     int               b)           // Integer
{
   if( a > b )
     return a;
   return b;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::min
//
// Purpose-
//       Integer miniumum.
//
//----------------------------------------------------------------------------
#undef min
static inline int                   // Resultant
   min(                             // Integer minimum
     int               a,           // Integer
     int               b)           // Integer
{
   if( a < b )
     return a;
   return b;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::wincheck
//
// Purpose-
//       Check the status of a windows function, abort if error.
//
//----------------------------------------------------------------------------
extern void
   wincheck(                        // Check windows function
     int               line,        // Line number
     void*             rc,          // Return code
     const char*       name);       // Function name

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::windebug
//
// Purpose-
//       Check the status of a windows function.
//
//----------------------------------------------------------------------------
extern int                          // The error code on input
   windebug(                        // Check windows function
     int               line,        // Line number
     void*             rc,          // Return code
     const char*       name);       // Function name

#include "gui/namespace.end"
#endif // GUI_OSWIN_DEVICE_H_INCLUDED
