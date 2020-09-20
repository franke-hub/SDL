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
//       2020/09/20
//
// Usage notes-
//       This implementation is NOT thread-safe.
//
//       There is only one buffer and one active offset pointer. You should
//       generally use different objects for encoding and decoding within
//       different threads.
//
//       This encoder/decoder implements RFC 3629, UTF-8 translation format.
//       Protected methods are called after error checking.
//
//----------------------------------------------------------------------------
#ifndef _PUB_UTF8_H_INCLUDED
#define _PUB_UTF8_H_INCLUDED

#include <stdio.h>                  // For fprintf, ...
#include <string.h>                 // For memcpy

#include "config.h"                 // For _PUB_NAMESPACE
#include "utility.h"                // For utility::to_string(), prereqs above

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
// UTF8::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum { REPLACE_CHAR= 0x0000FFFD };  // Error replacement character

//----------------------------------------------------------------------------
// UTF8::Attributes
//----------------------------------------------------------------------------
protected:
unsigned char*         utf8= nullptr; // The UTF-8 encoded string
mutable size_t         used= 0;     // The current size of the string
size_t                 size= 0;     // The maximum size of the string

//----------------------------------------------------------------------------
// UTF8::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ~UTF8( void ) {}                 // Destructor

   UTF8( void )                     // Default constructor
:  utf8(nullptr), used(0), size(0) {}

   UTF8(                            // String constructor
     char*             buff,        // The encode/decode character string
     size_t            size= 0)     // The encode/decode string maximum size
{  reset(buff, size); }

   UTF8(                            // String constructor
     unsigned char*    buff,        // The encode/decode character string
     size_t            size= 0)     // The encode/decode string maximum size
{  reset((char*)buff, size); }

void
   reset(                           // Reset the encode/decode buffer
     char*             buff,        // The encode/decode buffer
     size_t            size= 0)     // The encode/decode buffer length
{
   if( size == 0 && buff )          // Allow reset(nullptr{, 0})
     size= strlen(buff);

   this->utf8= (unsigned char*)buff;
   this->size= size;
   this->used= 0;
}

//----------------------------------------------------------------------------
// UTF8::Protected methods
//----------------------------------------------------------------------------
protected:
inline int                          // Return code, 0 OK
   chkX(                            // Verify extended characters
     unsigned          L)           // For this length
{
   if( (used + L) > size ) {        // If length outsize of buffer
     fprintf(stderr, "UTF8.decode [0x%.4zx]+%d >= size(%.4zx)\n"
                   , used - 1, L, size);
     used= size;
     return REPLACE_CHAR;
   }

   for(unsigned i= 0; i<L; i++) {
     int C= utf8[used+i];
     if( C < 0x0080 || C > 0x00BF ) {
       fprintf(stderr, "UTF8.decode [0x%.4zx]+%d Data char(0x%.2x)\n"
                     , used - 1, i + 1, C);
       used += i;
       return REPLACE_CHAR;
     }
   }

   return 0;
}

inline unsigned                     // The extended value
   getX(                            // Get extended value (UNCHECKED)
     unsigned          L)           // For this length
{
   unsigned result= 0;
   for(unsigned i= 0; i<L; i++) {
     result <<= 6;
     unsigned C= utf8[used++];
     result += (C & 0x003F);
   }

   return result;
}

inline void
   setX(                            // Insert extended value (UNCHECKED)
     unsigned          code,        // The extended value
     unsigned          L)           // The extended length
{
   if( L )                          // If additional characters
     setX(code >> 6, L - 1);        // Insert partial
   utf8[used++]= (code & 0x0000003F) | 0x00000080;
}

static inline bool                  // TRUE iff code is a valid start code
   is_start_encoding(               // Is
     unsigned          code)        // This a valid UTF-8 start encoding
{
     if( code < 0x00000080 || (code >= 0x000000C0 && code < 0x000000F7) )
       return true;

     return false;
}

// Decode error handlers
inline int                          // Always REPLACE_CHAR
   bad_decode(                      // ERROR: Attempt to decode
     unsigned          code,        // This invalid Unicode code point and
     unsigned          L) const     // This encoding length
{
   fprintf(stderr, "UTF8.decode [0x%.4zx] Overlong(0x%.6x)\n"
                 , used - L, code);
   return REPLACE_CHAR;
}

inline int                          // Always REPLACE_CHAR
   bad_start(                       // ERROR: Invalid UTF-8 start char
     unsigned          code)        // (The invalid char)
{
   fprintf(stderr, "UTF8.decode [0x%.4zx] Start char(0x%.2x)\n"
                 , used - 1, code);
   return REPLACE_CHAR;
}

inline int                          // Always REPLACE_CHAR
   bad_unicode(                     // ERROR: Attempt to decode
     unsigned          code,        // This invalid Unicode code point and
     unsigned          L) const     // This encoding length
{
   fprintf(stderr, "UTF8.decode [0x%.4zx] Not unicode(0x%.6x)\n"
                 , used - L, code);
   return REPLACE_CHAR;
}

// Encode error handlers
inline ssize_t                      // Always -1
   not_unicode(                     // ERROR: Attempt to encode
     unsigned          code) const  // This invalid Unicode code point
{
   fprintf(stderr, "UTF8.encode(0x%.4x) Not unicode\n", code);
   return -1;
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
ssize_t                             // Offset after encoding, -1 if error
   encode(                          // Encode
     unsigned          code)        // This Unicode code point
{
   unsigned L= 1;
   if( code >= 0x00000080 ) {
     L= 2;
     if( code >= 0x00000800 ) {
       L= 3;
       if( code >= 0x00010000 ) {
         L= 4;
         if( code >= 0x00110000 )
           return not_unicode(code);
       } else if( code >= 0x0000D800 && code <= 0x0000DFFF )
         return not_unicode(code);
     }
   }

   // If the buffer does not have enough space to encode the code point, an
   // error message is displayed and the encoder state remains unchanged.
   if( (used + L) > size ) {
     fprintf(stderr, "UTF8.encode(0x%.6x) Used(0x%zx)+%d > size(0x%zx)\n"
                   , code, used, L, size);
     return -1;
   }

   // Disallowed value ranges have been detected
   switch(L) {
     case 1:                        // Range 0x000000..0x00007F
       utf8[used++]= code;
       break;

     case 2:                        // Range 0x000080..0x0007FF
       utf8[used++]= (code >>  6) | 0x000000C0;
       setX(code, 0);
       break;

     case 3:                        // Range 0x000800..0x00FFFF
       utf8[used++]= (code >> 12) | 0x000000E0;
       setX(code, 1);
       break;

     case 4:                        // Range 0x010000..0x1FFFFF
       utf8[used++]= (code >> 18) | 0x000000F0;
       setX(code, 2);
       break;

     default:
       fprintf(stderr, "%4d UTF8.encode(%.6x) Internal error\n", __LINE__
                     , code);
       throw "Should Not Occur";
       break;
   }

   return used;
}

int                                 // The next Unicode code point, -1 if EOF
   decode( void )                   // Get next Unicode code point
{
   if( used >= size ) return -1;    // If EOF, return -1

   unsigned            code= utf8[used++]; // First input character

   unsigned L= 0;
   if( code < 0x000000C0 ) {        // Range 0x000000..0x00007F
     if( code >= 0x00000080 )       // Range 0x000080..0x0000BF (invalid start)
       return bad_start(code);
   } else if( code < 0x000000E0 ) { // Range 0x000080..0x0007FF
     L= 1;
   } else if( code < 0x000000F0 ) { // Range 0x000800..0x00FFFF (w/disallowed)
     L= 2;
   } else if( code < 0x000000F8 ) { // Range 0x010000..0x1FFFFF (w/disallowed)
     L= 3;
   } else {
     return bad_start(code);
   }

   if( chkX(L) )                    // If invalid continuation character/length
     return REPLACE_CHAR;

   switch( L ) {
     case 0:                        // Range 0x000000..0x00007F
       break;                       // (No translation)

     case 1:                        // Range 0x000080..0x0007FF
       code  &= 0x0000001F;
       code <<=  6;
       code  |= getX(1);
       if( code < 0x00000080 )
         return bad_decode(code, 2);
       break;

     case 2:                        // Range 0x000800..0x00FFFF (w/disallowed)
       code  &= 0x0000000F;
       code <<= 12;
       code  |= getX(2);
       if( code < 0x00000800 )
         return bad_decode(code, 3);
       if( code > 0x0000D7FF && code < 0x0000E000 )
         return bad_unicode(code, 3);
       break;

     case 3:                        // Range 0x010000..0x1FFFFF (w/disallowed)
       code  &= 0x00000007;
       code <<= 18;
       code  |= getX(3);
       if( code < 0x010000 )
         return bad_decode(code, 4);
       if( code > 0x0010FFFF )
         return bad_unicode(code, 4);
       break;

     default:
       fprintf(stderr, "%4d UTF8.decode(%d) Internal error\n", __LINE__, L);
       throw "Should Not Occur";
       break;
   }

   return code;
}

void
   bs( void )                       // Decode: Back Space
{
   while( used > 0 ) {              // While backward space exists
     unsigned code= utf8[used];
     if( is_start_encoding(code) )
       return;

     used--;
   }
}

void
   fs( void )                       // Decode: Forward Space
{
   while( used < size ) {           // While forward space exists
     unsigned code= utf8[++used];
     if( is_start_encoding(code) )
       return;
   }
}
}; // class UTF8
}  // namespace _PUB_NAMESPACE
#endif // _PUB_UTF8_H_INCLUDED
