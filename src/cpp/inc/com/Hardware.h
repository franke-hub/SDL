//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Hardware.h
//
// Purpose-
//       System hardware interfaces.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef HARDWARE_H_INCLUDED
#define HARDWARE_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       Hardware
//
// Purpose-
//       System hardware accessor class.
//
//----------------------------------------------------------------------------
class Hardware {                    // System hardware accessor class
//----------------------------------------------------------------------------
// Hardware::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Hardware( void );               // Destructor
   Hardware( void );                // Default Constructor

private:                            // Bitwise copy is prohibited
   Hardware(const Hardware&);       // Disallowed copy constructor
   Hardware& operator=(const Hardware&); // Disallowed assignment operator


//----------------------------------------------------------------------------
// Hardware::Static methods
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getLR
//
// Purpose-
//       Return the link register (return address).
//
//----------------------------------------------------------------------------
static void*                        // The caller's return address
   getLR( void );                   // Get link register

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getSP
//
// Purpose-
//       Return the stack pointer.
//
//----------------------------------------------------------------------------
static void*                        // The stack pointer
   getSP( void );                   // Get stack pointer

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getTSC
//
// Purpose-
//       Return the current timestamp counter
//
// Notes-
//       The timestamp counter is a high resolution elapsed time measurement
//       device. The lowest valid low order bit is updated each clock cycle.
//       Note: On some processors some of the low order bits may not change.
//
//----------------------------------------------------------------------------
static uint64_t                     // The timestamp counter
   getTSC( void );                  // Get timestamp counter
}; // class Hardware

#endif // HARDWARE_H_INCLUDED
