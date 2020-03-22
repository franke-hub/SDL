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
//       HW/STD/Hardware.cpp
//
// Purpose-
//       Hardware object default implementation
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static uint64_t      timeStamp= 0;  // Pseudo-timestamp

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getTSC
//
// Purpose-
//       Get timestamp counter.
//
//----------------------------------------------------------------------------
uint64_t                            // The timestamp counter
   Hardware::getTSC( void )         // Get timestamp counter
{
   return timeStamp++;
}

