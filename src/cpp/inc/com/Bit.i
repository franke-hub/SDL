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
//       Bit.i
//
// Purpose-
//       Bit manipulatives inline functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef BIT_I_INCLUDED
#define BIT_I_INCLUDED

//----------------------------------------------------------------------------
//
// Subroutine-
//      Bit::get
//
// Function-
//      Get bit in array
//
//----------------------------------------------------------------------------
int                                 // The bit value (0 or 1)
   Bit::get(                        // Get bit
     const char*       string,      // -> Bit string (source)
     unsigned int      offset)      // The bit number
{
   unsigned int        ci;          // Character index
   unsigned int        bi;          // Bit index

   ci= offset >> 3;                 // Get the character index
   bi= offset  & 7;                 // Get the bit index

   return ((*(string+ci) & bitSet[bi]) != 0); // Get the bit
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      Bit::set0
//
// Function-
//      Clear bit in array
//
//----------------------------------------------------------------------------
void
   Bit::set0(                       // Set bit to 0
     char*             string,      // -> Bit string (modified)
     unsigned int      offset)      // The bit number
{
   unsigned int        ci;          // Character index
   unsigned int        bi;          // Bit index

   ci= offset >> 3;                 // Get the character index
   bi= offset  & 7;                 // Get the bit index

   *(string+ci) &= bitClr[bi];      // Clear the bit
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      Bit::set1
//
// Function-
//      Set bit in array
//
//----------------------------------------------------------------------------
void
   Bit::set1(                       // Set bit to 1
     char*             string,      // -> Bit string (modified)
     unsigned int      offset)      // The bit number
{
   unsigned int        ci;          // Character index
   unsigned int        bi;          // Bit index

   ci= offset >> 3;                 // Get the character index
   bi= offset  & 7;                 // Get the bit index

   *(string+ci) |= bitSet[bi];      // Set the bit
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      Bit::set
//
// Function-
//      Set bit in array
//
//----------------------------------------------------------------------------
void
   Bit::set(                        // Set bit to value
     char*             string,      // -> Bit string (modified)
     unsigned int      offset,      // The bit number
     int               value)       // Value (0 or !0)
{
   if( value == 0 )
     set0(string, offset);
   else
     set1(string, offset);
}

#endif // BIT_I_INCLUDED
