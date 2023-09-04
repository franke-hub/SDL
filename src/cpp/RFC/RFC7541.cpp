//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       RFC7541.cpp
//
// Purpose-
//       RFC7541, HTTP/2 HPACK compression
//
// Last change date-
//       2023/09/04
//
//----------------------------------------------------------------------------
#include <cassert>                  // For assert
#include <cstdint>                  // For uint32_t, uint16_t, ...
#include <cstring>                  // For memcpy, memcmp, ...
#include <cctype>                   // For isprint
#include <new>                      // For std::bad_alloc
#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string

#include <pub/utility.h>            // For pub::utility::dump
#include <pub/Debug.h>              // For pub::debugf, ...

#include "RFC7541.h"                // For class RFC7541, implemented

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub objects
using namespace PUB::debugging;     // For debugging subroutines

typedef RFC7541::octet octet;       // Import octet, for subroutines

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

#define USE_OUTLINE false           // Use out-line code? (not in-line)

enum                                // Octet constants
{  BITS_PER_OCTET= 8                // Number of bits in an octet
,  BITS_USED_MASK= 7                // Number of bits used in last byte
,  LOG2_PER_OCTET= 3                // Log2(BITS_PER_OCTET)
}; // Octet constants

//----------------------------------------------------------------------------
// Internal objects and tables
//----------------------------------------------------------------------------
#include "RFC7541.hpp"              // Internal objects and tables

// Fill validation table
static const int       fill_table[8]= { 0, 1, 3, 7, 15, 31, 63, 127};

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::Huff
//       RFC7541::Huff::~Huff
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
#if USE_OUTLINE
   RFC7541::Huff::Huff( void )      // Constructor
{  if( HCDM )
     debugf("Huff(%p)::Huff()\n", this);
   // = default;
}

   RFC7541::Huff::Huff(const std::string& s) // Copy std::string constructor
{  if( HCDM )
     debugf("Huff(%p)::Huff(string(%s))\n", this, s.c_str());

   *this= RFC7541::encode(s);
}

   RFC7541::Huff::Huff(const Huff& h) // Copy constructor
{  if( HCDM )
     debugf("Huff(%p)::Huff(const Huff&(%p) {%p,%zd} addr(%p))\n", this
           , &h, h.addr, h.size, addr);

   _copy(h);
}


   RFC7541::Huff::Huff(Huff&& h)    // Move constructor
{  if( HCDM )
     debugf("Huff(%p)::Huff(Huff&&(%p) {%p,%zd})\n", this, &h, h.addr, h.size);

   addr= h.addr; size= h.size;
   h.addr= nullptr; h.size= 0;
}

   RFC7541::Huff::~Huff( void )     // Destructor
{  if( HCDM )
     debugf("Huff(%p)::~Huff() {%p,%zd}\n", this, addr, size);

   free(addr);
   addr= 0;
   size= 0;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   RFC7541::Huff::debug(const char* info) const // Debugging display
{
   debugf("Huff(%p).debug(%s) {%p,%zd}\n", this, info, addr, size);
   pub::utility::dump(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
#if USE_OUTLINE
RFC7541::Huff&
   RFC7541::Huff::operator=(        // Assignment
   const std::string&  s)           // From std::string
{  if( HCDM )
     debugf("Huff(%p)::op=(string(%s)) {%p,%zd}\n", this, s.c_str(), addr, size);

   *this= RFC7541::encode(s);
   return *this;
}

RFC7541::Huff&
   RFC7541::Huff::operator=(        // Copy assignment
     const Huff&       h)
{  if( HCDM )
     debugf("Huff(%p)::op=(Huff&(%p)) {%p,%zd}\n", this, &h, addr, size);

   _copy(h);
   return *this;
}

RFC7541::Huff&
   RFC7541::Huff::operator=(        // Move assignment
     Huff&&            h)
{  if( HCDM )
     debugf("Huff(%p)::op=(Huff&&(%p)) {%p,%zd}\n", this, &h, addr, size);

   free(addr);
   addr= h.addr; size= h.size;
   h.addr= nullptr; h.size= 0;
   return *this;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::operator==
//       RFC7541::Huff::operator!=
//
// Purpose-
//       Equality comparison operator
//       Inequality comparison operator
//
//----------------------------------------------------------------------------
bool
   RFC7541::Huff::operator==(const Huff& h) const // Equality comparison
{  if( HCDM )
     debugf("Huff(%p)::op==(const Huff&(%p))\n", this, &h);

   if( addr == h.addr && size == h.size ) // Self or empty comparison
     return true;

   if( size == h.size ) {           // If size is the same (won't be zero)
     if( memcmp(addr, h.addr, h.size) == 0 ) // If values are identical
       return true;
   }

   return false;
}

bool
   RFC7541::Huff::operator!=(const Huff& h) const // Inequality comparison
{  return !operator==(h); }

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::_copy
//
// Purpose-
//       Copy method
//
//----------------------------------------------------------------------------
void
   RFC7541::Huff::_copy(            // Copy from
      const Huff&      h)           // This RFC7541::Huff
{  if( HCDM )
     debugf("Huff(%p)::_copy(Huff(%p) {%p,%zd}) addr(%p)\n", this
           , &h, h.addr, h.size, addr);

   free(addr);
   if( h.size ) {
     addr= (octet*)malloc(h.size);
     if( addr == nullptr ) {
       size= 0;
       throw bad_alloc();
     }
     memcpy(addr, h.addr, h.size);
   }

   size= h.size;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::decode
//
// Purpose-
//       Accessor method
//
//----------------------------------------------------------------------------
#if USE_OUTLINE
std::string RFC7541::Huff::decode( void ) const // Decode
{  if( HCDM )
     debugf("Huff(%p)::decode() {%p,%zd}\n", this, addr, size);

   return RFC7541::decode(addr, size);
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::RFC7541
//       RFC7541::~RFC7541
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
#if USE_OUTLINE
   RFC7541::RFC7541( void )         // NOT CODED YET
{  }

   RFC7541::~RFC7541( void )        // NOT CODED YET
{  }
#endif

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   RFC7541::debug(const char* info) // Debugging display
{
   debugf("RFC7541::debug(%s)\n", info);

   // Debug decode_index
   debugf("\ndecode_index:\n");
   for(int i= 0; i<DECODE_INDEX_DIM; ++i) {
     const Bits7541& X= decode_index[i];
     debugf("[%3d]: {%3d, %2d, %.8x, %.8x}\n", i, X.min_index, X.bits
           , X.min_encode, X.max_encode);
   }

   // Debug decode_table
   debugf("\ndecode_table:\n");
   for(int i= 0; i<DECODE_TABLE_DIM; ++i) {
     const Huff7541& T= decode_table[i];
     debugf("[%3d]: {%3d, %2d, %#.8x} '%c'\n", i, T.decode, T.bits, T.encode
           , T.decode < 256 && isprint(T.decode) ? T.decode : '~');
   }

   // Debug encode_table
   debugf("\nencode_table:\n");
   for(int i= 0; i<ENCODE_TABLE_DIM; ++i) {
     const Huff7541& T= encode_table[i];
     debugf("[%3d]: {%3d, %2d, %#.8x} '%c'\n", i, T.decode, T.bits, T.encode
           , T.decode < 256 && isprint(T.decode) ? T.decode : '~');
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::decode
//
// Purpose-
//       Decode compressed string
//
//----------------------------------------------------------------------------
std::string                         // The decompressed string
   RFC7541::decode(                 // Decode buffer
      const octet*     addr,        // Buffer address
      size_t           size)        // Buffer length
{  if( HCDM ) debugf("RFC7541::decode(%p,%zd)\n", addr, size);

   enum {BUFF_DIM= 15};              // The output buffer size
   size_t              inp_index= 0; // The input buffer index
   std::string         out_string;  // The output string
   char                out_buffer[BUFF_DIM+1]; // The output buffer
   int                 out_index= 0; // The output buffer index

   // DECODE -----------------------------------------------------------------
   enum {ACC_WIDTH= 64};            // Accumulator width, in bits
   uint64_t accumulator= 0;         // Accumulator
   int      acc_index= 0;           // Accumulator bit index
   bool     end_of_string= false;   // End of string encountered

   while( inp_index <= size ) {
     // Test character in range
     for(int index_ix= 0; index_ix<DECODE_INDEX_DIM; ++index_ix) {
       const Bits7541& B= decode_index[index_ix];

       // Implementation note: If the number of bits between code points
       // increases more than BITS_PER_OCTET between values, use:
       // while( B.bits > acc_index ) instead of if( ... )
       if( B.bits > acc_index ) {   // If more accumulator bits are needed
         if( inp_index >= size ) {
           // Use when bits between code_points can be > BITS_PER_OCTET
           // if( acc_index > BITS_PER_OCTET )
           //   throw std::runtime_error("encoding error");

           end_of_string= true;
           break;
         }

         accumulator <<= BITS_PER_OCTET;
         accumulator  |= addr[inp_index++];
         acc_index += BITS_PER_OCTET;
       }

       // The accumulator has enough bytes to test
       uint64_t decumulator= (accumulator >> (acc_index - B.bits));
       decumulator <<= (ACC_WIDTH - B.bits); // Clear high order bits
       decumulator >>= (ACC_WIDTH - B.bits);

       if( decumulator>B.max_encode ) { // If more bits required
         if( index_ix == DECODE_INDEX_DIM ) // If at end of table
           throw std::runtime_error("encoding error: size");
         continue;
       } else {                     // We have enough bits
         int index= decumulator - B.min_encode;
         if( index < 0 )            // If invalid encoding
           throw std::runtime_error("encoding error: value");
         index += B.min_index;

         if( out_index >= BUFF_DIM ) {
           std::string addend(out_buffer, BUFF_DIM);
           out_string += addend;
           out_index= 0;
         }
         out_buffer[out_index++]= decode_table[index].decode;

         acc_index -= B.bits;
         break;
       }
     }
     if( end_of_string )
       break;
   }

   if( out_index ) {                // Add remainder
     std::string addend(out_buffer, out_index);
     out_string += addend;
   }

   if( acc_index ) {
     accumulator <<= (ACC_WIDTH - acc_index); // Zero any removed bits
     accumulator >>= (ACC_WIDTH - acc_index);
   } else {
     accumulator= 0;                // (accum <<= ACC_WIDTH) == accum
   }
   if( accumulator != (uint64_t)fill_table[acc_index] )
     throw std::runtime_error("encoding error: fill");

   return out_string;
}

#if USE_OUTLINE
std::string                         // The decompressed string
   RFC7541::decode(                 // Decode
      const Huff&      h)           // This Huff
{  if( HCDM ) debugf("RFC7541::decode Huff(%p) {%p,%zd}\n", &h, h.addr, h.size);

   return decode(h.addr, h.size);
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::encode
//
// Purpose-
//       Encode string
//
//----------------------------------------------------------------------------
RFC7541::Huff                       // The encoded string
   RFC7541::encode(                 // Encode
      const std::string& s)         // This string
{  if( HCDM ) debugf("RFC7541::encode(%s)\n", s.c_str());

   Huff h;
   size_t size= encoded_length(s);  // The encoded length
   if( size == 0 )                  // If empty string
     return h;                      // Use empty encoding

   h.addr= (octet*)malloc(size);    // Allocate octet buffer
   if( h.addr == nullptr )
     throw bad_alloc();
   h.size= size;

   // ENCODE -----------------------------------------------------------------
   enum {ACC_WIDTH= 64};            // Accumulator width, in bits
   uint64_t accumulator= 0;         // Accumulator
   int      acc_index= 0;           // Accumulator bit index
   size_t   out_index= 0;           // Resultant (h.data) byte index

   for(auto it= s.cbegin(); it != s.cend(); ++it) {
     const Huff7541& H= encode_table[(octet)*it];
     //const octet O= (octet)*it;     // The next octet
     //const int   bits= encode_table[O].bits; // The encoded octet size, in bits
     if( (acc_index + H.bits) > ACC_WIDTH ) { // If accumulator dump required
       while( acc_index > BITS_PER_OCTET ) { // While full bytes exist
         h.addr[out_index++]= (accumulator >> (acc_index - BITS_PER_OCTET));
         acc_index -= BITS_PER_OCTET;
       }
     }

     accumulator <<= H.bits;
     accumulator  |= H.encode;
     acc_index += H.bits;
   }

   // Flush accumulator bytes, if present
   while( acc_index >= BITS_PER_OCTET ) { // While full bytes exist
     h.addr[out_index++]= (accumulator >> (acc_index - BITS_PER_OCTET));
     acc_index -= BITS_PER_OCTET;
   }

   // Handle last character fill
   if( acc_index ) {
     accumulator <<= BITS_PER_OCTET;
     accumulator  |= 0x000000FF;
     accumulator >>= acc_index;
     h.addr[out_index++]= (octet)accumulator;
   }

   // DIAGNOSTICS
   if( out_index != size ) { // Correctness check
     debugf("%4d %s SHOULD NOT OCCUR\n", __LINE__, __FILE__);
     debugf("..out_index(%zd) ..size(%zd)\n", out_index, size);
     exit(2);
   }

   return h;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::encoded_length
//
// Purpose-
//       Get the encoded length of a std::string (in bytes)
//
//----------------------------------------------------------------------------
size_t                              // The encoded length
   RFC7541::encoded_length(         // Get encoded length
      const std::string& s)         // Of this string
{
   size_t size= 0;                  // The encoded length (in bits)

   for(auto it= s.cbegin(); it != s.cend(); ++it) {
     octet c= (octet)*it;
     size += encode_table[c].bits;
   }

   size  += BITS_USED_MASK;         // Account for fractional byte
   size >>= LOG2_PER_OCTET;         // Size in bytes

   return size;
}
