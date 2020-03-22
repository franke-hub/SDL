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
//       Checksum.h
//
// Purpose-
//       Define the Checksum objects.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef CHECKSUM_H_INCLUDED
#define CHECKSUM_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       Checksum64
//
// Purpose-
//       Portable 64-bit Checksum.
//
//----------------------------------------------------------------------------
class Checksum64 {                  // Checksum64
//----------------------------------------------------------------------------
// Checksum64::Attributes
//----------------------------------------------------------------------------
private:
uint64_t               value;       // The check sum accumulator

//----------------------------------------------------------------------------
// Checksum64::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Checksum64( void );             // Destructor
inline
   Checksum64( void );              // Constructor

//----------------------------------------------------------------------------
// Checksum64::Operators
//----------------------------------------------------------------------------
public:
inline int                          // TRUE if equal
   operator==(                      // Are Checksums equal?
     Checksum64&       source) const; // (The other Checksum)

inline int                          // TRUE if unequal
   operator!=(                      // Are Checksums unequal?
     Checksum64&       source) const; // (The other Checksum)

//----------------------------------------------------------------------------
// Checksum64::Accessor methods
//----------------------------------------------------------------------------
public:
uint64_t                            // The current value
   getValue( void ) const;          // Get the Checksum

void
   setValue(                        // Set the Checksum
     uint64_t          value);      // (To this value)

//----------------------------------------------------------------------------
// Checksum64::Static methods
//----------------------------------------------------------------------------
public:
static uint64_t                     // Updated accumulator
   sum(                             // Accumulate
     const void*       buffer,      // Buffer address
     unsigned          length,      // Buffer length
     uint64_t          prior = 0);  // Prior accumulator

//----------------------------------------------------------------------------
// Checksum64::Methods
//----------------------------------------------------------------------------
public:
inline void
   reset( void );                   // Reset (zero) the accumulator

inline void
   accumulate(                      // Accumulate
     const void*       buffer,      // Buffer address
     unsigned          length);     // Buffer length
}; // class Checksum64

//----------------------------------------------------------------------------
//
// Class-
//       Checksum32
//
// Purpose-
//       Portable 32-bit Checksum
//
//----------------------------------------------------------------------------
class Checksum32 {                  // Checksum32
//----------------------------------------------------------------------------
// Checksum32::Attributes
//----------------------------------------------------------------------------
private:
uint32_t               value;       // The check sum accumulator

//----------------------------------------------------------------------------
// Checksum32::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Checksum32( void );             // Destructor
inline
   Checksum32( void );              // Constructor

//----------------------------------------------------------------------------
// Checksum32::Operators
//----------------------------------------------------------------------------
public:
inline int                          // TRUE if equal
   operator==(                      // Are Checksums equal?
     Checksum32&       source) const; // (The other Checksum)

inline int                          // TRUE if unequal
   operator!=(                      // Are Checksums unequal?
     Checksum32&       source) const; // (The other Checksum)

//----------------------------------------------------------------------------
// Checksum32::Accessor methods
//----------------------------------------------------------------------------
public:
uint32_t                            // The current value
   getValue( void ) const;          // Get the Checksum

void
   setValue(                        // Set the Checksum
     uint32_t          value);      // (To this value)

//----------------------------------------------------------------------------
// Checksum32::Static methods
//----------------------------------------------------------------------------
public:
static uint32_t                     // Updated accumulator
   sum(                             // Accumulate
     const void*       buffer,      // Buffer address
     unsigned          length,      // Buffer length
     uint32_t          prior = 0);  // Prior accumulator

//----------------------------------------------------------------------------
// Checksum32::Methods
//----------------------------------------------------------------------------
public:
inline void
   accumulate(                      // Accumulate
     const void*       buffer,      // Buffer address
     unsigned          length);     // Buffer length

inline void
   reset( void );                   // Reset (zero) the accumulator
}; // class Checksum32

//----------------------------------------------------------------------------
//
// Class-
//       Checksum16
//
// Purpose-
//       Portable 16-bit Checksum
//
//----------------------------------------------------------------------------
class Checksum16 {                  // Checksum16
//----------------------------------------------------------------------------
// Checksum16::Attributes
//----------------------------------------------------------------------------
private:
uint16_t               value;       // The check sum accumulator

//----------------------------------------------------------------------------
// Checksum16::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Checksum16( void );             // Destructor
inline
   Checksum16( void );              // Constructor

//----------------------------------------------------------------------------
// Checksum16::Operators
//----------------------------------------------------------------------------
public:
inline int                          // TRUE if equal
   operator==(                      // Are Checksums equal?
     Checksum16&       source) const; // (The other Checksum)

inline int                          // TRUE if unequal
   operator!=(                      // Are Checksums unequal?
     Checksum16&       source) const; // (The other Checksum)

//----------------------------------------------------------------------------
// Checksum16::Accessor methods
//----------------------------------------------------------------------------
public:
uint16_t                            // The current value
   getValue( void ) const;          // Get the Checksum

void
   setValue(                        // Set the Checksum
     uint16_t          value);      // (To this value)

//----------------------------------------------------------------------------
// Checksum16::Static methods
//----------------------------------------------------------------------------
public:
static uint16_t                     // Updated accumulator
   sum(                             // Accumulate
     const void*       buffer,      // Buffer address
     unsigned          length,      // Buffer length
     uint16_t          prior = 0);  // Prior accumulator

//----------------------------------------------------------------------------
// Checksum16::Methods
//----------------------------------------------------------------------------
public:
inline void
   accumulate(                      // Accumulate
     const void*       buffer,      // Buffer address
     unsigned          length);     // Buffer length

inline void
   reset( void );                   // Reset (zero) the accumulator
}; // class Checksum16

#include "Checksum.i"

#endif // CHECKSUM_H_INCLUDED
