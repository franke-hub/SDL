//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Checksum.cpp
//
// Purpose-
//       Checksum methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef _OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <com/define.h>
#include <com/Debug.h>

#include "com/Checksum.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If defined, hard-core debug mode
#undef  HCDM                        // If defined, hard-core debug mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       Checksum64::sum
//
// Purpose-
//       Accumulate the Checksum.
//
//----------------------------------------------------------------------------
uint64_t                            // Current accumulator
   Checksum64::sum(                 // Accumulate the Checksum
     const void*       buffer,      // Buffer address
     unsigned          length,      // Buffer length
     uint64_t          prior)       // Prior accumulator
{
   const uint8_t*      ptrU08;      // -> Current character
   const uint32_t*     ptrU32;      // -> Current fullword
   uint64_t            hi, lo, hicarry, locarry; // Accumulators
   int                 u32Count;    // Number of full fullword pairs
   int                 remain;      // Number of extra bytes at end
   int                 i;

   ptrU08= (const uint8_t*)buffer;  // Address the buffer
   ptrU32= (const uint32_t*)buffer; // Address the buffer
   u32Count= (length/8)*2;          // Number of full fullword pairs
   remain= length & 7;              // Number of extra bytes at end

   hi= (prior >> 32);               // Initialize the accumulators
   lo= (prior << 32) >> 32;

   for(i=0; i < u32Count; i+=2)     // Accumulate the words
   {
     hi += ntohl(ptrU32[i]);
     lo += ntohl(ptrU32[i+1]);
   }

   if( remain >= 1 )                // Account for extra bytes
     hi += ptrU08[4*u32Count+0] * 0x01000000;
   if( remain >= 2 )
     hi += ptrU08[4*u32Count+1] * 0x00010000;
   if( remain >= 3 )
     hi += ptrU08[4*u32Count+2] * 0x00000100;
   if( remain >= 4 )
     hi += ptrU08[4*u32Count+3];
   if( remain >= 5 )
     lo += ptrU08[4*u32Count+4] * 0x01000000;
   if( remain >= 6 )
     lo += ptrU08[4*u32Count+5] * 0x00010000;
   if( remain >= 7 )
     lo += ptrU08[4*u32Count+6] * 0x00000100;

   hicarry= hi >> 32;               // Fold in carry bits
   locarry= lo >> 32;
   while( hicarry || locarry )
   {
     hi = (hi & 0x00000000FFFFFFFFLL) + locarry;
     lo = (lo & 0x00000000FFFFFFFFLL) + hicarry;
     hicarry = hi >> 32;
     locarry = lo >> 32;
   }

   return (hi << 32) + lo;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum64::getValue
//
// Function-
//       Extract Checksum.value
//
//----------------------------------------------------------------------------
uint64_t                            // The value
   Checksum64::getValue( void ) const // Extract the value
{
   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum64::setValue
//
// Function-
//       Set Checksum.value
//
//----------------------------------------------------------------------------
void
   Checksum64::setValue(            // Set the value
     uint64_t        value)         // (To this value)
{
   this->value= value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Checksum32::sum
//
// Purpose-
//       Calculate 32-bit checksum
//
// Restrictions-
//       Length must be < 2**18, or carry can overflow
//
//----------------------------------------------------------------------------
uint32_t                            // Updated accumulator
   Checksum32::sum(                 // Accumulate the Checksum
     const void*       buffer,      // Buffer address
     unsigned          length,      // Buffer length
     uint32_t          prior)       // Prior accumulator
{
   const uint8_t*      ptrU08;      // -> Current character
   const uint16_t*     ptrU16;      // -> Current halfword
   uint32_t            hi, lo, hicarry, locarry; // Accumulators
   int                 u16Count;    // Number of full halfword pairs
   int                 remain;      // Number of extra bytes at end
   int                 i;

   ptrU08= (const uint8_t*)buffer;  // Address the buffer
   ptrU16= (const uint16_t*)buffer; // Address the buffer
   u16Count= (length/4)*2;          // Number of full halfword pairs
   remain= length & 3;              // Number of extra bytes at end

   hi= (prior >> 16);               // Initialize the accumulators
   lo= (prior << 16) >> 16;

   for(i=0; i < u16Count; i+=2)     // Accumulate the words
   {
     hi += ntohs(ptrU16[i]);
     lo += ntohs(ptrU16[i+1]);
   }

   if( remain >= 1 )                // Account for extra bytes
     hi += ptrU08[2*u16Count+0] * 0x0100;
   if( remain >= 2 )
     hi += ptrU08[2*u16Count+1];
   if( remain == 3 )
     lo += ptrU08[2*u16Count+2] * 0x0100;

   hicarry= hi >> 16;               // Fold in carry bits
   locarry= lo >> 16;
   while( hicarry || locarry )
   {
     hi = (hi & 0x0000FFFF) + locarry;
     lo = (lo & 0x0000FFFF) + hicarry;
     hicarry = hi >> 16;
     locarry = lo >> 16;
   }

   return (hi << 16) + lo;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum32::getValue
//
// Function-
//       Extract Checksum32.value
//
//----------------------------------------------------------------------------
uint32_t                            // The value
   Checksum32::getValue( void ) const // Extract the value
{
   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum32::setValue
//
// Function-
//       Set Checksum32.value
//
//----------------------------------------------------------------------------
void
   Checksum32::setValue(            // Set the value
     uint32_t        value)         // (To this value)
{
   this->value= value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Checksum16::sum
//
// Purpose-
//       Calculate 16-bit checksum
//
// Restrictions-
//       Length must be < 2**18, or carry can overflow
//
//----------------------------------------------------------------------------
uint16_t                            // Updated accumulator
   Checksum16::sum(                 // Accumulate the Checksum
     const void*     buffer,        // Buffer address
     unsigned        length,        // Buffer length
     uint16_t        prior)         // Prior accumulator
{
   const uint8_t*    ptrU08;        // -> Current byte
   uint32_t          accum;         // Accumulator

   ptrU08= (const uint8_t*)buffer;  // Address the buffer

   accum= prior;
   for(unsigned i=0; i < length; i++) // Accumulate the bytes
   {
     accum += ptrU08[i]/* & 0x00ff*/;
   }

   while( (accum >> 16) != 0 )      // Fold in carry bits
   {
     accum = (accum & 0x0000ffff) + (accum >> 16);
   }

   return accum;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum16::getValue
//
// Function-
//       Extract Checksum16.value
//
//----------------------------------------------------------------------------
uint16_t                            // The value
   Checksum16::getValue( void ) const // Extract the value
{
   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checksum16::setValue
//
// Function-
//       Set Checksum16.value
//
//----------------------------------------------------------------------------
void
   Checksum16::setValue(            // Set the value
     uint16_t        value)         // (To this value)
{
   this->value= value;
}

