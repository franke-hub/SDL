//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/UTF8.h
//
// Purpose-
//       UTF-8 Encoder/decoder
//
// Last change date-
//       2020/01/24
//
// Usage notes-
//       This implementation is NOT thread-safe.
//
//       There is only one buffer and one active offset pointer. You should
//       generally use different objects for encoding and decoding within
//       different threads.
//
//       This encoder/decoder implements 3629, UTF-8 translation format.
//       The encode(code, unsigned char*) internal method is non-conformant
//       because it does not provide required error checking. It is in a
//       protected method not intended for conformant use.
//
//----------------------------------------------------------------------------
#ifndef _PUB_UTF8_H_INCLUDED
#define _PUB_UTF8_H_INCLUDED

#include <stdio.h>                  // For sprintf
#include <string.h>                 // For memcpy

#include "config.h"                 // For _PUB_NAMESPACE
#include "utility.h"                // For utility::to_string(), prereqs above
#include "Exception.h"              // For Exception

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       UTF8
//
// Purpose-
//       UTF-8 encoding and decoding.
//
// UTF-8 Encoding-
// Bytes Bits    First     Last  Byte[0]  Byte[1]  Byte[2]  Byte[3]
//     1    7 U+000000 U+00007F 0-----7-      N/A      N/A      N/A ( 7 bits)
//     2   11 U+000080 U+0007FF 110---5- 10----6-      N/A      N/A (11 bits)
//     3   16 U+000800 U+00D7FF 1110--4- 10----6- 10----6-      N/A (16 bits)
//     3   16 U+00D800 U+00DFFF Disallowed: UTF16 surrogate pairs
//     3   16 U+00E000 U+00FFFF 1110--4- 10----6- 10----6-      N/A (16 bits)
//     4   21 U+010000 U+10FFFF 11110-3- 10----6- 10----6- 10----6- (21 bits)
//     4   21 U+110000 U+1FFFFF Disallowed: Outside Unicode range
//
//----------------------------------------------------------------------------
class UTF8 {                        // UTF-8 encoding/decoding
//----------------------------------------------------------------------------
// UTF8::Attributes
//----------------------------------------------------------------------------
protected:
unsigned char*         utf8;        // The UTF-8 encoded string
mutable size_t         used;        // The current size of the string
size_t                 size;        // The maximum size of the string

//----------------------------------------------------------------------------
// UTF8::Exceptions
//----------------------------------------------------------------------------
public:
class UTF8Error : public Exception { using Exception::Exception;
}; // class UTF8Error

class BufferEmpty : public UTF8Error {
using UTF8Error::UTF8Error;
}; // class BufferEmpty

class BufferFull : public UTF8Error {
using UTF8Error::UTF8Error;
}; // class BufferFull

//----------------------------------------------------------------------------
// UTF8::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ~UTF8( void ) {}                 // Destructor

   UTF8( void ) = delete;           // NO default constructor

   UTF8(                            // String constructor
     char*             outs,        // The output character string
     size_t            size)        // The output string maximum size
:  utf8((unsigned char*)outs), used(0), size(size) {}

   UTF8(                            // String constructor
     unsigned char*    outs,        // The output character string
     size_t            size)        // The output string maximum size
:  utf8(outs), used(0), size(size) {}

//----------------------------------------------------------------------------
// UTF8::Protected methods
//----------------------------------------------------------------------------
protected:
inline unsigned                     // The extended value
   get2(                            // Get extended value
     unsigned          X)           // At this offset
{
   if( used >= size )
     throw UTF8Error(utility::to_string("decode[%zd] 0x%.2x[%d] past end",
                                        used-1, utf8[used-X-1], X));

   unsigned bits= utf8[used++];
   if( bits < 0x00000080 || bits >= 0x000000C0 )
     throw UTF8Error(utility::to_string("decode[%zd] 0x%.2x[%d] 0x%.2x invalid",
                                        used-1, utf8[used-X-1], X, bits));

   return (bits & 0x0000003F);
}

static inline unsigned              // The value to insert
   set2(                            // Insert extended value
     unsigned          bits)        // The extended value
{
   bits &= 0x0000003F;
   return (bits | 0x00000080);
}

void
   encode(                          // Encode (NO ERROR CHECKING)
     unsigned          code,        // This Unicode code point
     unsigned char*    outs)        // Into this output string
{
   if( code < 0x00000080 )          // If encoding size == 1
   {
     outs[0]= code;
   } else if( code < 0x00000800 )   // If encoding size == 2
   {
     outs[0]= (code >> 6) | 0x000000C0;
     outs[1]= set2(code);
   }
   else if( code < 0x00010000 )     // If encoding size == 3
   {
     outs[0]= (code >> 12) | 0x000000E0;
     outs[1]= set2(code >> 6);
     outs[2]= set2(code);
   }
   else if( code < 0x00200000 )     // If encoding size == 4
   {
     outs[0]= (code >> 18) | 0x000000F0;
     outs[1]= set2(code >> 12);
     outs[2]= set2(code >>  6);
     outs[3]= set2(code);
   }
}

static inline bool                  // TRUE iff code is a valid start code
   is_start_encoding(               // Is
     unsigned          code)        // This a valid UTF-8 start encoding
{
     if( code < 0x00000080 )
       return true;
     if( code >= 0x000000C2 && code < 0x000000F5 )
       return true;

     return false;
}

// Exception generators
inline void
   encoding(                        // ERROR: Invalid encoding
     unsigned          code) const  // For this Unicode code point
{
   throw UTF8Error(utility::to_string("decode[%zd] 0x%.6x encoding",
                                      used, code));
}

static inline void
   not_decode(                      // ERROR: Attempt to decode
     unsigned          code)        // This invalid Unicode code point
{
   throw UTF8Error(utility::to_string("decode(0x%.6x) not Unicode", code));
}

static inline void
   not_encode(                      // ERROR: Attempt to encode
     unsigned          code)        // This invalid Unicode code point
{
   throw UTF8Error(utility::to_string("encode(0x%.6x) not Unicode", code));
}

inline void
   not_start( void ) const          // ERROR: Invalid UTF-8 start code
{
   throw UTF8Error(utility::to_string("decode[%zd] 0x%.2x[0] invalid",
                                      used-1, utf8[used-1]));
}

//----------------------------------------------------------------------------
// UTF8::Accessors
//----------------------------------------------------------------------------
public:
size_t get_size(void) const { return size; } // Get buffer size
size_t get_used(void) const { return used; } // Get number of bytes used
void set_used(size_t used_) { this->used= used_; } // Set used count

//----------------------------------------------------------------------------
// UTF8::Methods
//----------------------------------------------------------------------------
size_t                              // Offset after encoding [==  get_used()]
   encode(                          // Encode
     unsigned          code)        // This Unicode code point
{
   unsigned L= 1;
   if( code >= 0x00000080 )
   {
     L= 2;
     if( code >= 0x00000800 )
     {
       L= 3;
       if( code >= 0x00010000 )
       {
         L= 4;
         if( code >= 0x00110000 )
           not_encode(code);
       } else if( code >= 0x0000D800 && code <= 0x0000DFFF )
           not_encode(code);
     }
   }

   // If the buffer does not have enough space to encode the code point, the
   // BufferFull exception is thrown. The encoder state remains unchanged.
   if( (size - used) <= L )
     throw BufferFull();

   encode(code, utf8+used);
   used += L;
   return used;
}

unsigned                            // The next Unicode code point
   decode( void )                   // Get next Unicode code point
{
   if( used >= size )
     throw BufferEmpty();

   unsigned            code= utf8[used++]; // First input character

   if( code < 0x000000C0 )
   {
     if( code >= 0x00000080 )
       not_start();
   }
   else if( code < 0x000000E0 )
   {
     code &= 0x0000003F;
     code <<= 6;
     code  += get2(1);
     if( code < 0x00000080 )
       encoding(code);
   }
   else if( code < 0x000000F0 )
   {
     code &= 0x0000001F;
     code <<= 6;
     code  += get2(1);
     code <<= 6;
     code  += get2(2);
     if( code < 0x00000800 )
       encoding(code);
     else if( code >= 0x0000D800 && code <= 0x0000DFFF )
       not_decode(code);
   }
   else if( code < 0x000000F5 )
   {
     code &= 0x0000000F;
     code <<= 6;
     code  += get2(1);
     code <<= 6;
     code  += get2(2);
     code <<= 6;
     code  += get2(3);
     if( code < 0x00010000 )
       encoding(code);
     else if( code >= 0x00110000 )
       not_decode(code);
   }
   else
     not_start();

   return code;
}

void
   bs( void )                       // Back space
{
   while( used > 0 )                // While backward space exists
   {
     unsigned code= utf8[used];
     if( is_start_encoding(code) )
       return;

     used--;
   }
}

void
   fs( void )                       // Forward space
{
   if( used < size )                // If there is any forward space
   {
     unsigned code= utf8[used];
     unsigned L= 1;
     if( code < 0x000000C0 )
     {
       if( code >= 0x00000080 )
         not_start();
     }
     else if( code < 0x000000E0 )
       L= 2;
     else if( code < 0x000000F0 )
       L= 3;
     else if( code < 0x000000F5 )
       L= 4;
     else
       throw UTF8Error(utility::to_string("fs[%zd] 0x%.2x[0] invalid",
                                          used, utf8[used]));

     if( (size - used) > L )
       throw UTF8Error(utility::to_string("fs[%zd] 0x%.2x[0] past end",
                                          used, utf8[used]));
   }

   throw BufferEmpty("fs");
}

// After an error, find the next valid start character
void
   synch( void )                    // Skip to next valid start character
{
   while( used < size )
   {
     unsigned code= utf8[used];
     if( is_start_encoding(code) )
       return;

     used++;
   }

   throw BufferEmpty("synch failure");
}
}; // class UTF8
}  // namespace _PUB_NAMESPACE
#endif // _PUB_UTF8_H_INCLUDED
