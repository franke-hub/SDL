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
//       OS/BSD/Device.cpp
//
// Purpose-
//       Graphical User Interface: Device implementation (X11).
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <com/Interval.h>
#include <com/Mutex.h>
#include <com/Status.h>
#include <com/Thread.h>
#include <com/Unconditional.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

using GUI::Color_t;
using GUI::Device;
using GUI::Event;
using GUI::Object;
using GUI::Pixel;
using GUI::XYLength;
using GUI::XYOffset;
using GUI::XOffset_t;
using GUI::YOffset_t;
using GUI::XLength_t;
using GUI::YLength_t;

#define X11Device GUI ## _X11Device
#define X11Thread GUI ## _X11Thread

#include "X11Thread.h"
#include "X11Device.h"              // (Requires X11Thread.h)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef VERBOSE
#define VERBOSE 1                   // Verbosity
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       max
//
// Purpose-
//       Integer maxiumum.
//
//----------------------------------------------------------------------------
#undef max
static inline unsigned              // Resultant
   max(                             // Integer maximum
     unsigned          a,           // Integer
     unsigned          b)           // Integer
{
   if( a > b )
     return a;
   return b;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       min
//
// Purpose-
//       Integer miniumum.
//
//----------------------------------------------------------------------------
#undef min
static inline unsigned              // Resultant
   min(                             // Integer minimum
     unsigned          a,           // Integer
     unsigned          b)           // Integer
{
   if( a < b )
     return a;
   return b;
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::make
//
// Purpose-
//       Return a System Dependent Implementation
//
//----------------------------------------------------------------------------
Device*                             // -> Device
   Device::make(                    // Return System Dependent Implementation
     Window*           window)      // -> Window
{
   #if defined(HCDM) && TRUE
     debugSetIntensiveMode();
   #endif

   X11Device* device= new X11Device(window); // (Convienient gdb break point)
   return device;                   // (Exposes X11Device* in gdb)
}

//----------------------------------------------------------------------------
//
// Class-
//       X11Device
//       X11Thread
//
// Purpose-
//       Associated class Implementations
//
//----------------------------------------------------------------------------
#include "X11Device.cpp"
#include "X11Thread.cpp"            // (Requires X11Device macros)

