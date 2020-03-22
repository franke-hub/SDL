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
//       Checksum.i
//
// Purpose-
//       Checksum inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef CHECKSUM_I_INCLUDED
#define CHECKSUM_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Checksum64::~Checksum64
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   Checksum64::~Checksum64( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum64::Checksum64
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   Checksum64::Checksum64( void )   // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum64::operator==
//
// Function-
//       Are Checksums equal?
//
//----------------------------------------------------------------------------
int                                 // TRUE if Checksums are equal
   Checksum64::operator==(          // Are Checksums equal?
     Checksum64&       source) const  // (The other Checksum)
{
   return value == source.value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum64::operator!=
//
// Function-
//       Are Checksums unequal?
//
//----------------------------------------------------------------------------
int                                 // TRUE if Checksums are unequal
   Checksum64::operator!=(          // Are Checksums unequal?
     Checksum64&       source) const  // (The other Checksum)
{
   return value != source.value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum64::accumulate
//
// Function-
//       Accumulate the Checksum
//
//----------------------------------------------------------------------------
void
   Checksum64::accumulate(          // Accumulate the Checksum
     const void*       buffer,      // Buffer address
     unsigned          length)      // Buffer length
{
   setValue(Checksum64::sum(buffer, length, getValue()));
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum64::reset
//
// Function-
//       Reset (zero) the Checksum.
//
//----------------------------------------------------------------------------
void
   Checksum64::reset( void )        // Reset (zero) the Checksum
{
   value= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum32::~Checksum32
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   Checksum32::~Checksum32( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum32::Checksum32
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   Checksum32::Checksum32( void )   // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum32::operator==
//
// Function-
//       Are Checksums equal?
//
//----------------------------------------------------------------------------
int                                 // TRUE if Checksums are equal
   Checksum32::operator==(          // Are Checksums equal?
     Checksum32&       source) const// (The other Checksum)
{
   return value == source.value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum32::operator!=
//
// Function-
//       Are Checksums unequal?
//
//----------------------------------------------------------------------------
int                                 // TRUE if Checksums are unequal
   Checksum32::operator!=(          // Are Checksums unequal?
     Checksum32&       source) const// (The other Checksum)
{
   return value != source.value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum32::accumulate
//
// Function-
//       Accumulate the Checksum
//
//----------------------------------------------------------------------------
void
   Checksum32::accumulate(          // Accumulate the Checksum
     const void*       buffer,      // Buffer address
     unsigned          length)      // Buffer length
{
   setValue(Checksum32::sum(buffer, length, getValue()));
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum32::reset
//
// Function-
//       Reset (zero) the Checksum.
//
//----------------------------------------------------------------------------
void
   Checksum32::reset( void )        // Reset (zero) the checksum
{
   value= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum16::~Checksum16
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   Checksum16::~Checksum16( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum16::Checksum16
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   Checksum16::Checksum16( void )   // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum16::operator==
//
// Function-
//       Are Checksums equal?
//
//----------------------------------------------------------------------------
int                                 // TRUE if Checksums are equal
   Checksum16::operator==(          // Are Checksums equal?
     Checksum16&       source) const// (The other Checksum)
{
   return value == source.value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum16::operator!=
//
// Function-
//       Are Checksums unequal?
//
//----------------------------------------------------------------------------
int                                 // TRUE if Checksums are unequal
   Checksum16::operator!=(          // Are Checksums unequal?
     Checksum16&       source) const// (The other Checksum)
{
   return value != source.value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum16::accumulate
//
// Function-
//       Accumulate the Checksum
//
//----------------------------------------------------------------------------
void
   Checksum16::accumulate(          // Accumulate the Checksum
     const void*       buffer,      // Buffer address
     unsigned          length)      // Buffer length
{
   setValue(Checksum16::sum(buffer, length, getValue()));
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum16::reset
//
// Function-
//       Reset (zero) the Checksum.
//
//----------------------------------------------------------------------------
void
   Checksum16::reset( void )        // Reset (zero) the Checksum
{
   value= 0;
}

#endif // CHECKSUM_I_INCLUDED
