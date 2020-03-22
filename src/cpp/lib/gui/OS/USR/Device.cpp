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
//       OS/USR/Device.cpp
//
// Purpose-
//       Graphical User Interface: Device implementation (sample).
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       UsrDevice
//
// Purpose-
//       Define system dependent implementation methods.
//
//----------------------------------------------------------------------------
#include "gui/namespace.gui"
class UsrDevice : public Device  {  // Device System Dependent Implementation
//----------------------------------------------------------------------------
// UsrDevice::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~UsrDevice( void );              // Destructor
   UsrDevice(                       // Constructor
     Window*           window);     // -> Source Window

//----------------------------------------------------------------------------
// Device::Methods
//----------------------------------------------------------------------------
public:
virtual void
   change(                          // Device change
     const XYOffset&   offset,      // Offset
     const XYLength&   length);     // Length

//----------------------------------------------------------------------------
// UsrDevice::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class UsrDevice
#include "gui/namespace.end"
using GUI::UsrDevice;

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
   return new UsrDevice(window);
}

//----------------------------------------------------------------------------
//
// Method-
//       UsrDevice::~UsrDevice
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   UsrDevice::~UsrDevice( void )    // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: UsrDevice(%p)::~UsrDevice()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       UsrDevice::UsrDevice
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   UsrDevice::UsrDevice(            // Constructor
     Window*           window)      // Source Window
:  Device(window)
{
   #ifdef HCDM
     Logger::log("%4d: UsrDevice(%p)::UsrDevice(%p)\n",
                 __LINE__, this, window);
   #endif

   debugf("*** WARNING!!! *** OS/BSD/Device.cpp NOT FUNCTIONAL!!!\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       UsrDevice::setAttribute
//
// Purpose-
//       Change attribute.
//
//----------------------------------------------------------------------------
void
   UsrDevice::setAttribute(         // Set Attribute
     Object::Attribute attribute,   // Attribute identifier
     int               value)       // Attribute value
{
   #ifdef HCDM
     Logger::log("%4d: UsrDevice(%p)::setAttribute(%d,%d)\n", __LINE__, this,
                 attribute, value);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       UsrDevice::adjust
//
// Purpose-
//       Adjust the Device offset and length
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   UsrDevice::adjust(               // Adjust the offset and length
     const XYOffset&   offset,      // Offset
     const XYLength&   length)      // Length
{
   #ifdef HCDM
     Logger::log("%4d: UsrDevice(%p)::adjust({%d,%d},{%d,%d})\n",
                 __LINE__, this, offset.x, offset.y, length.x, length.y);
   #endif

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       UsrDevice::change
//
// Purpose-
//       Device change.
//
//----------------------------------------------------------------------------
void
   UsrDevice::change(               // Device changed
     const XYOffset&   offset,      // Offset
     const XYLength&   length)      // Length
{
   #ifdef HCDM
     Logger::log("%4d: UsrDevice(%p)::change({%d,%d},{%d,%d})\n",
                 __LINE__, this, offset.x, offset.y, length.x, length.y);
   #endif

   window->debug();
}

