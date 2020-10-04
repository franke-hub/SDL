//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Device.cpp
//
// Purpose-
//       Graphical User Interface: Device implementation
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/Types.h"
#include "gui/Event.h"
#include "gui/KeyCode.h"
#include "gui/Object.h"
#include "gui/Window.h"

#include "gui/Device.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal constants
//----------------------------------------------------------------------------
static const char*     NOT_CAPABLE= "NotCapable";

#include "gui/namespace.gui"
//----------------------------------------------------------------------------
//
// Method-
//       Device::~Device
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Device::~Device( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Device(%p)::~Device()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::Device
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Device::Device(                  // Default constructor
     Window*           window)      // Source Window
:  Attributes()
,  window(window)
{
   #ifdef HCDM
     Logger::log("%4d: Device(%p)::Device(%p)\n", __LINE__, this, window);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::callback
//
// Purpose-
//       Window callback
//
//----------------------------------------------------------------------------
void
   Device::callback(                // Window callback
     const Event&      event)       // For this Event
{
   #ifdef HCDM
     Logger::log("%4d: Device(%p)::callback(%p)\n", __LINE__, this, &event);
     event.debug();
   #endif

   window->callback(event);
}

void
   Device::callback(                // Window callback
     Event::EC         code,        // The event code
     int               data,        // The event data
     const XYOffset&   offset,      // Set Offset
     const XYLength&   length)      // Set Length
{
   Event event(code, data, offset, length);
   callback(event);
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::change
//
// Purpose-
//       Change the Device
//
//----------------------------------------------------------------------------
void
   Device::change(                  // Device change
     const XYOffset&   offset,      // Offset
     const XYLength&   length)      // Length
{
   #ifdef HCDM
     Logger::log("%4d: Device(%p)::change({%d,%d},{%d,%d})\n",
                 __LINE__, this, offset.x, offset.y, length.x, length.y);
   #else                            // Parameters only used if HCDM defined
     (void)offset; (void)length;
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::debug
//
// Purpose-
//       Debug the Device
//
//----------------------------------------------------------------------------
void
   Device::debug( void )            // Debugging display
{
   Logger::log("%4d: Device(%p)::debug()\n", __LINE__, this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::move
//
// Purpose-
//       Move the Device
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Device::move(                    // Move the Device
     const XYOffset&   offset)      // Offset
{
   #ifdef HCDM
     Logger::log("%4d: Device(%p)::move({%d,%d})\n",
                 __LINE__, this, offset.x, offset.y);
   #else                            // Parameters only used if HCDM defined
     (void)offset;
   #endif

   return NOT_CAPABLE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::resize
//
// Purpose-
//       Resize the Device
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Device::resize(                  // Resize the Device
     const XYLength&   length)      // Length
{
   #ifdef HCDM
     Logger::log("%4d: Device(%p)::resize({%d,%d})\n",
                 __LINE__, this, length.x, length.y);
   #else                            // Parameters only used if HCDM defined
     (void)length;
   #endif

   return NOT_CAPABLE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::wait
//
// Purpose-
//       Wait while operational
//
//----------------------------------------------------------------------------
long                                // (UNUSED) Return code
   Device::wait( void )             // Wait for Device termination
{
   #ifdef HCDM
     Logger::log("%4d: Device(%p)::wait()\n", __LINE__, this);
   #endif

   return 0;
}
#include "gui/namespace.end"

//----------------------------------------------------------------------------
//
// Section-
//       System Dependent Include
//
// Purpose-
//       Operating System dependent Device driver.
//
//----------------------------------------------------------------------------
#if   defined(_OS_WIN)
#include "OS/WIN/Device.cpp"

#elif defined(_OS_BSD)
#include "OS/BSD/Device.cpp"

#else
#error "Invalid OS"
#endif

