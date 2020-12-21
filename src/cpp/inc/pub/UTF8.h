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
//       UTF-8 utilities
//
// Last change date-
//       2020/12/20
//
// Usage notes-
//       The Encoder/Decoder implement RFC 3629, UTF-8 translation format.
//       Protected methods are only called after error checking.
//
//----------------------------------------------------------------------------
#ifndef _PUB_UTF8_H_INCLUDED
#define _PUB_UTF8_H_INCLUDED

#include <stdio.h>                  // For fprintf, ...
#include <string.h>                 // For memcpy
#include "Debug.h"                  // For pub::debugging::options

namespace pub::UTF8 {
#define pub_verb (pub::debugging::options::pub_hcdm || pub::debugging::options::pub_verbose > 0)

//----------------------------------------------------------------------------
//
// Namespace-
//       pub::UTF8
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
enum { REPLACE_CHAR= 0x0000FFFD };  // Error replacement character
typedef uint8_t        utf8_t;      // The UTF-8 character (octet) type
typedef int            utf32_t;     // The UTF-32 code point type

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::UTF8::dec
//
// Purpose-
//       Locate prior start code. (Maximum skip: 4 bytes)
//
//----------------------------------------------------------------------------
#ifdef __GNUC__
   #pragma GCC diagnostic push;
   #pragma GCC diagnostic ignored "-Warray-bounds"
#endif
static inline const utf8_t*         // The next start code
   dec(                             // Get next start code
     const utf8_t*     addr)        // The current start code, length >= 4
{
   if( addr[-1] < 0x80 || addr[-1] >= 0xC0 ) // Normal || error
     return addr - 1;

   if( addr[-2] < 0x80 || addr[-2] >= 0xC0 ) // Error || normal
     return addr - 2;

   if( addr[-3] < 0x80 || addr[-3] >= 0xC0 ) // Error || normal
     return addr - 3;

   return addr - 4;                 // Normal || error
}
static inline utf8_t* dec(utf8_t* addr)
{ return const_cast<utf8_t*>(dec(const_cast<const utf8_t*>(addr))); }

static inline const utf8_t*         // The prior start code
   dec(                             // Get prior start code
     const utf8_t*     addr,        // The current start code
     size_t            size)        // Buffer origin offset
{
   if( size == 0 ) return addr;     // Do not decrement beyond origin
   if( addr[-1] < 0x80 || addr[-1] >= 0xC0 ) // Normal || error
     return addr - 1;

   if( size == 1 ) return addr - 1; // Do not decrement beyond origin
   if( addr[-2] < 0x80 || addr[-2] >= 0xC0 ) // Error || normal
     return addr - 2;

   if( size == 2 ) return addr - 2; // Do not decrement beyond origin
   if( addr[-3] < 0x80 || addr[-3] >= 0xC0 ) // Error || normal
     return addr - 3;

   if( size == 3 ) return addr - 3; // Do not decrement beyond origin
   return addr - 4;                 // Normal || error
}
static inline utf8_t* dec(utf8_t* addr, size_t size)
{ return const_cast<utf8_t*>(dec(const_cast<const utf8_t*>(addr), size)); }
#ifdef __GNUC__
   #pragma GCC diagnostic pop;
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::UTF8::inc
//
// Purpose-
//       Locate next start code (Maximum skip: 4 bytes)
//
//----------------------------------------------------------------------------
static inline const utf8_t*         // The next valid start code
   inc(                             // Get next valid start code
     const utf8_t*     addr)        // The current start code
{
   if( *addr < 0xC0 )               // (Skips invalid starts 0x80..0xBF)
     return addr + 1;

   if( *addr < 0xE0 )
     return addr + 2;

   if( *addr < 0xF0 )
     return addr + 3;

   return addr + 4;
}
static inline utf8_t* inc(utf8_t* addr)
{ return const_cast<utf8_t*>(inc(const_cast<const utf8_t*>(addr))); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::UTF8::index
//
// Purpose-
//       Convert utf8_t* logical index to char* offset
//
//----------------------------------------------------------------------------
static inline size_t                // The char* offset
   index(                           // Get char* offset for
     const utf8_t*     addr,        // This ('\0' terminated) utf8_t* string
     size_t            X)           // And this logical index
{
   size_t O= 0;                     // Current offset
   while( X > 0 ) {
     uint8_t SC= addr[O];           // The current start character
     if( SC == '\0' ) break;
     if( SC < 0xC0 ) {              // If one character (or error) encoding
       ++O;
     } else if( SC < 0xE0 ) {       // If two character encoding
       if( addr[++O] == '\0' ) break;
       ++O;
     } else if( SC < 0xF0 ) {       // If three character encoding
       if( addr[++O] == '\0' ) break;
       if( addr[++O] == '\0' ) break;
       ++O;
     } else {                       // If four character encoding
       if( addr[++O] == '\0' ) break;
       if( addr[++O] == '\0' ) break;
       if( addr[++O] == '\0' ) break;
       ++O;
     }

     --X;
   }

   return O;
}

static inline size_t                // The char* offset
   index(                           // Get char* offset for
     const char*       addr,        // This ('\0' terminated) utf8_t* string
     size_t            X)           // And this logical index
{  return index((const utf8_t*)addr, X); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::UTF8::is_start_encoding
//
// Purpose-
//       Determine whether a UTF-8 character is a valid start code
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff code is a valid start code
   is_start_encoding(               // Is the UTF-8 character a valid start?
     int               code)        // The UTF-8 start character
{
   if( code < 0x00000080 || (code >= 0x000000C0 && code < 0x000000F7) )
     return true;

   return false;
}

class Decoder {                     // UTF-8 decoder
//----------------------------------------------------------------------------
//
// Class-
//       pub::UTF8::Decoder
//
// Purpose-
//       Convert (const) utf8_t* string into utf32_t words
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// pub::UTF8::Decoder::Attributes
//----------------------------------------------------------------------------
protected:
const utf8_t*          utf8= nullptr; // The UTF-8 encoded string
mutable size_t         used= 0;     // The current string index
size_t                 size= 0;     // The size of the string

//----------------------------------------------------------------------------
// pub::UTF8::Decoder::Destructor
//----------------------------------------------------------------------------
public:
   ~Decoder( void ) {}              // Destructor (does nothing)

//----------------------------------------------------------------------------
// pub::UTF8::Decoder::Constructors
//----------------------------------------------------------------------------
public:
   Decoder( void )                  // Default constructor
:  utf8(nullptr), used(0), size(0) {}

   Decoder(                         // String constructor
     const char*       _buff,       // The decode character string
     size_t            _size= 0)    // The decode string size
{  reset(_buff, _size); }

   Decoder(                         // String constructor
     const utf8_t*     _buff,       // The decode character string
     size_t            _size= 0)    // The decode string size
{  reset(_buff, _size); }

//----------------------------------------------------------------------------
//
// Class-
//       pub::UTF8::Decoder::reset
//
// Purpose-
//       Reset the decode buffer
//
//----------------------------------------------------------------------------
void
   reset(                           // Reset the encode buffer
     const utf8_t*     _buff,       // The encode buffer
     size_t            _size= 0)    // The encode buffer length
{
   if( _size == 0 && _buff )        // Allow reset(nullptr{, 0})
     _size= strlen((const char*)_buff);

   this->utf8= _buff;
   this->size= _size;
   this->used= 0;
}
void reset(const char* buff, size_t _size= 0)
{ reset((const utf8_t*)buff, _size); }

//----------------------------------------------------------------------------
// pub::UTF8::Decoder::Protected methods
//----------------------------------------------------------------------------
protected:
inline int                          // Return code, 0 OK
   chkX(                            // Verify extended characters
     unsigned          L)           // For this length
{
   if( (used + L) > size ) {        // If length outsize of buffer
     if( pub_verb )
       fprintf(stderr, "UTF8.decode [0x%.4zx]+%d >= size(%.4zx)\n"
                     , used - 1, L, size);
     used= size;
     return REPLACE_CHAR;
   }

   for(unsigned i= 0; i<L; i++) {
     int C= utf8[used+i];
     if( C < 0x0080 || C > 0x00BF ) {
       if( pub_verb )
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

// Decode error handlers
inline utf32_t                      // Always REPLACE_CHAR
   bad_decode(                      // ERROR: Attempt to decode
     unsigned          code,        // This invalid Unicode code point and
     unsigned          L) const     // This encoding length
{
   if( pub_verb )
     fprintf(stderr, "UTF8.decode [0x%.4zx] Overlong(0x%.6x)\n"
                   , used - L, code);
   return REPLACE_CHAR;
}

inline utf32_t                      // Always REPLACE_CHAR
   bad_start(                       // ERROR: Invalid UTF-8 start char
     unsigned          code)        // (The invalid char)
{
   if( pub_verb )
     fprintf(stderr, "UTF8.decode [0x%.4zx] Start char(0x%.2x)\n"
                   , used - 1, code);
   return REPLACE_CHAR;
}

inline utf32_t                      // Always REPLACE_CHAR
   bad_unicode(                     // ERROR: Attempt to decode
     unsigned          code,        // This invalid Unicode code point and
     unsigned          L) const     // This encoding length
{
   if( pub_verb )
     fprintf(stderr, "UTF8.decode [0x%.4zx] Not unicode(0x%.6x)\n"
                   , used - L, code);
   return REPLACE_CHAR;
}

//----------------------------------------------------------------------------
// pub::UTF8::Decoder::::Accessors
//----------------------------------------------------------------------------
public:
size_t get_size(void) const { return size; } // Get buffer size
size_t get_used(void) const { return used; } // Get number of bytes used

//----------------------------------------------------------------------------
// pub::UTF8::Decoder::::Methods
//----------------------------------------------------------------------------
utf32_t                             // The next Unicode code point, -1 if EOF
   decode( void )                   // Get next Unicode code point
{
   if( used >= size ) return -1;    // If EOF, return -1

   utf32_t code= utf8[used++];      // Working resultant; Start character
   unsigned L= 0;                   // Number of additional characters
   if( code < 0x000000C0 ) {        // Range 0x000000..0x0000BF
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

   switch( L ) {                    // Check for disallowed range
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
   dec( void )                      // Decode: Back Space
{
   while( used > 0 ) {              // While backward space exists
     unsigned code= utf8[used];
     if( is_start_encoding(code) )
       return;

     used--;
   }
}

void
   inc( void )                      // Decode: Forward Space
{
   while( used < size ) {           // While forward space exists
     unsigned code= utf8[++used];
     if( is_start_encoding(code) )
       return;
   }
}
}; // class Decoder

class Encoder {                     // UTF-8 encoder
//----------------------------------------------------------------------------
//
// Class-
//       pub::UTF8::Encoder
//
// Purpose-
//       Convert utf32_t words into utf8_t* string
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// pub::UTF8::Encoder::Attributes
//----------------------------------------------------------------------------
protected:
utf8_t*                utf8= nullptr; // The UTF-8 encoded string
mutable size_t         used= 0;     // The current size of the string
size_t                 size= 0;     // The maximum size of the string

//----------------------------------------------------------------------------
// pub::UTF8::Encoder::Destructor
//----------------------------------------------------------------------------
public:
   ~Encoder( void ) {}              // Destructor (does nothing)

//----------------------------------------------------------------------------
// pub::UTF8::Encoder::Constructors
//----------------------------------------------------------------------------
public:
   Encoder( void )                  // Default constructor
:  utf8(nullptr), used(0), size(0) {}

   Encoder(                         // String constructor
     char*             _buff,       // The encode/decode character string
     size_t            _size= 0)    // The encode/decode string maximum size
{  reset(_buff, _size); }

   Encoder(                         // String constructor
     utf8_t*           _buff,       // The encode/decode character string
     size_t            _size= 0)    // The encode/decode string maximum size
{  reset(_buff, _size); }

//----------------------------------------------------------------------------
//
// Class-
//       pub::UTF8::Encoder::reset
//
// Purpose-
//       Reset the decode buffer
//
//----------------------------------------------------------------------------
void
   reset(                           // Reset the encode buffer
     utf8_t*           _buff,       // The encode buffer
     size_t            _size= 0)    // The encode buffer length
{
   if( _size == 0 && _buff )        // Allow reset(nullptr{, 0})
     _size= strlen((char*)_buff);

   this->utf8= _buff;
   this->size= _size;
   this->used= 0;
}
void reset(char* buff, size_t _size= 0) { reset((utf8_t*)buff, _size); }

//----------------------------------------------------------------------------
// pub::UTF8::Encoder::Protected methods
//----------------------------------------------------------------------------
protected:
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
   utf8[used++]= utf8_t((code & 0x0000003F) | 0x00000080);
}

// Encode error handlers
inline ssize_t                      // Always -1
   not_unicode(                     // ERROR: Attempt to encode
     unsigned          code) const  // This invalid Unicode code point
{
   if( pub_verb )
     fprintf(stderr, "UTF8.encode(0x%.4x) Not unicode\n", code);
   return -1;
}

//----------------------------------------------------------------------------
// pub::UTF8::Encoder::Accessors
//----------------------------------------------------------------------------
public:
size_t get_size(void) const { return size; } // Get buffer size
size_t get_used(void) const { return used; } // Get number of bytes used

//----------------------------------------------------------------------------
// pub::UTF8::Encoder::Methods
//----------------------------------------------------------------------------
ssize_t                             // Offset after encoding, -1 if error
   encode(                          // Encode
     int               code)        // This Unicode code point
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
     if( pub_verb )
       fprintf(stderr, "UTF8.encode(0x%.6x) Used(0x%zx)+%d > size(0x%zx)\n"
                     , code, used, L, size);
     return -1;
   }

   // Disallowed value ranges have been detected
   switch(L) {
     case 1:                        // Range 0x000000..0x00007F
       utf8[used++]= utf8_t(code);
       break;

     case 2:                        // Range 0x000080..0x0007FF
       utf8[used++]= utf8_t((code >>  6) | 0x000000C0);
       setX(code, 0);
       break;

     case 3:                        // Range 0x000800..0x00FFFF
       utf8[used++]= utf8_t((code >> 12) | 0x000000E0);
       setX(code, 1);
       break;

     case 4:                        // Range 0x010000..0x1FFFFF
       utf8[used++]= utf8_t((code >> 18) | 0x000000F0);
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
}; // class Encoder
#undef pub_verb
}  // namespace _PUB_NAMESPACE
#endif // _PUB_UTF8_H_INCLUDED
