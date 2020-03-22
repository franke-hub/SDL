//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the "un-license,"
//       explicitly released into the Public Domain.
//       (See accompanying file LICENSE.UNLICENSE or the original
//       contained within http://unlicense.org)
//
//----------------------------------------------------------------------------
//
// Title-
//       CRC32.h
//
// Purpose-
//       This is the official CRC-32 vchecksum polynomial, as defined for use
//       in PKZip, WinZip and Ethernet.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       Derived from public domain software.
//
//----------------------------------------------------------------------------
#ifndef CRC32_H_INCLUDED
#define CRC32_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       CRC32
//
// Purpose-
//       This is the official CRC-32 checksum polynomial, as defined for use
//       in PKZip, WinZip and Ethernet.
//
//----------------------------------------------------------------------------
class CRC32 {
//----------------------------------------------------------------------------
// Attributes
//----------------------------------------------------------------------------
protected:
uint32_t               value;       // Current value

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------
public:
   ~CRC32( void );                  // Destructor
   CRC32( void );                   // Constructor

//----------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------
public:
inline void
   accumulate(                      // Accumulate
     const void*       addr,        // Buffer address
     unsigned long     length)      // Buffer length
{
   value= sum(addr, length, value); // Update the checksum
}

inline uint32_t                     // The current checksum
   getValue( void ) const           // Get current checksum
{
   return value ^ 0xffffffff;
}

inline void
   reset( void )                    // Reset the CRC
{
   value= 0xffffffff;
}

static uint32_t                     // Accumulated checksum
   sum(                             // Accumulate
     const void*       addr,        // Buffer address
     unsigned long     size,        // Buffer length
     uint32_t          csum = 0xffffffff); // Current checksum
}; // class CRC32

#endif // CRC32_H_INCLUDED
