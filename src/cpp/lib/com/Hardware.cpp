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
//       Hardware.cpp
//
// Purpose-
//       Hareware object implementation.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include "com/Debug.h"
#include "com/Hardware.h"
#include "com/Thread.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       getShiftTSC (REFERENCE ONLY)
//
// Purpose-
//       Determine the number of constant TSC bits.
//
//----------------------------------------------------------------------------
#if 0
inline int                          // The number of constant TSC bits
   getShiftTSC(                     // Get number of constant TSC bits
     int               count)       // Repeat counter
{
   int                 result;      // The number of constant TSC bits

   uint64_t            temp;        // Temporary (separate for debugging)
   uint64_t            ones;        // getTSC() one bits

   int                 i;

   if( count <= 0 )                 // If defaulted or invalid
     count= 16384;                  // Use the default count

   ones= 0;
   for(i= 0; i<count; i++)
   {
     temp= Hardware::getTSC();
     ones |= temp;
     if( (ones&1) != 0 )            // If low order bit set
       break;

     Thread::yield();
   }

   for(result= 0; (ones&1) == 0; result++)
     ones >>= 1;

   return result;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::~Hardware
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Hardware::~Hardware( void )      // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::Hardware
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Hardware::Hardware( void )       // Default constructor
{
}

#if defined(_HW_X86)
  #include "HW/X86/Hardware.cpp"
#else
  #include "HW/STD/Hardware.cpp"
#endif

