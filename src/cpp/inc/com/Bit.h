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
//       Bit.h
//
// Purpose-
//       Bit manipulatives.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef BIT_H_INCLUDED
#define BIT_H_INCLUDED

#ifndef  INLINE_H_INCLUDED
#include "inline.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Bit
//
// Purpose-
//       The Bit class contains static bit manipulators.
//
//----------------------------------------------------------------------------
class Bit {
//----------------------------------------------------------------------------
// Bit::Attributes
//----------------------------------------------------------------------------
private:
static const unsigned char bitClr[8]; // Bitfield zero mask table
static const unsigned char bitSet[8]; // Bitfield ones mask table

//----------------------------------------------------------------------------
// Bit::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Bit( void ) {}                  // Destructor

private:                            // Static class, no constructor
   Bit( void );                     // DISALLOWED Constructor

//----------------------------------------------------------------------------
// Bit::Methods
//----------------------------------------------------------------------------
public:
static INLINE int                   // The bit value (0 or 1)
   get(                             // Get bit
     const char*       string,      // -> Bit string (source)
     unsigned int      offset);     // The bit number

static INLINE void
   set(                             // Set bit
     char*             string,      // -> Bit string (modified)
     unsigned int      offset,      // The bit number
     int               value= 1);   // Value (0 or !0)

static INLINE void
   set0(                            // Set bit to 0
     char*             string,      // -> Bit string (modified)
     unsigned int      offset);     // The bit number

static INLINE void
   set1(                            // Set bit to 1
     char*             string,      // -> Bit string (modified)
     unsigned int      offset);     // The bit number
}; // class Bit

#if INLINING
#include "Bit.i"
#endif

#endif // BIT_H_INCLUDED
