//----------------------------------------------------------------------------
//
//       Copyright (c) 2021-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/Utf.h
//
// Purpose-
//       UTF utilities
//
// Last change date-
//       2022/04/08
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_UTF_H_INCLUDED
#define _LIBPUB_UTF_H_INCLUDED

#include <iterator>                 // For std::forward_iterator_tag

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Utf8;                         // UTF8  container
class Utf16;                        // UTF16 container
class Utf32;                        // UTF32 container

//----------------------------------------------------------------------------
//
// Class-
//       pub::Utf
//
// Purpose-
//       Unicode Transformation Format base class.
//
//----------------------------------------------------------------------------
class Utf {
public:
//----------------------------------------------------------------------------
// Utf::Enumerations and typedefs
//----------------------------------------------------------------------------
enum                                // Unicode characters
{  BYTE_ORDER_MARK= 0x00FEFF        // Byte Order Mark, a.k.a BOM
,  MARK_ORDER_BYTE= 0x00FFFE        // Little endian Byte Order Mark
,  UNI_REPLACEMENT= 0x00FFFD        // Unicode error replacement character
}; // enum Unicode characters

typedef uint8_t        utf8_t;      // The UTF8  character (octet) type
typedef uint16_t       utf16_t;     // The UTF16 character type
typedef uint32_t       utf32_t;     // The UTF32 code point type

//----------------------------------------------------------------------------
// Utf::Internal structures
//----------------------------------------------------------------------------
struct Init;                        // Forward reference

//----------------------------------------------------------------------------
// Utf::const_iterator (Data modification not allowed.)
//----------------------------------------------------------------------------
struct const_iterator {
typedef std::ptrdiff_t              difference_type;
typedef const utf32_t*              pointer;
typedef const utf32_t&              reference;
typedef utf32_t                     value_type;
typedef std::forward_iterator_tag   iterator_category;

const void*            origin= nullptr; // The iterator data
size_t                 offset= 0;   // The iterator offset
utf32_t                value= 0;    // The current value
uint32_t               itopts= 0;   // Iterator options

   ~const_iterator() = default;     // (Destructor)
   const_iterator() = default;      // (Default constructor)
   const_iterator(const const_iterator& src) = default; // (Copy constructor)
   const_iterator(const_iterator&& src) = default; // (Move constructor)

bool operator==(const const_iterator& that) const
{
   if( that.value == 0 ) {          // Quick compare: end() == *this
     if( value == 0 )
       return true;

     return false;
   }

   // Note: If origin and offset are equal, values must also be equal
   return (origin==that.origin && offset==that.offset && itopts==that.itopts);
}

bool operator!=(const const_iterator& that) const
{  return !operator==(that); }

value_type
   operator*()
{  return value; }
}; // class const_iterator

//----------------------------------------------------------------------------
// Utf::Utility methods
//----------------------------------------------------------------------------
static const const_iterator
                       the_end;     // The default end iterator
static const const_iterator&        // Note: All end iterators are equal
   end()                            // Get the built-in end iterator
{  return the_end; }                // (Conveniently inlinable)

static bool                         // TRUE iff code point is valid
   is_unicode(                      // Is code in allowed unicode range?
     utf32_t           code)        // The source code point
{
   if( code > 0x10FFFF              // If outside unicode range -or-
       || (code >= 0x00D800 && code <= 0x00DFFF) ) // If UTF16 surrogate pair
     return false;
   return true;
}
}; // class Utf

//----------------------------------------------------------------------------
//
// Class-
//       pub::Utf8
//
// Purpose-
//       UTF8 encoder/decoder.
//
// UTF8 Encoding-
// Bytes Bits    First     Last  Byte[0]  Byte[1]  Byte[2]  Byte[3]
//     1    7 U+000000 U+00007F 0-----7-      N/A      N/A      N/A ( 7 bits)
//     2   11 U+000080 U+0007FF 110---5- 10----6-      N/A      N/A (11 bits)
//     3   16 U+000800 U+00D7FF 1110--4- 10----6- 10----6-      N/A (16 bits)
//     3   16 U+00D800 U+00DFFF Disallowed: UTF16 surrogate pairs
//     3   16 U+00E000 U+00FFFF 1110--4- 10----6- 10----6-      N/A (16 bits)
//     4   21 U+010000 U+10FFFF 11110-3- 10----6- 10----6- 10----6- (21 bits)
//     4   21 U+110000 U+1FFFFF Disallowed: Outside Unicode range
//
// Usage notes-
//       This class implements RFC 3629, UTF8 translation format.
//
//----------------------------------------------------------------------------
class Utf8 : public Utf {
//----------------------------------------------------------------------------
// Attributes
//----------------------------------------------------------------------------
protected:
utf8_t*                data= nullptr; // The utf8_t data
size_t                 size= 0;     // The allocated length (in bytes)

size_t                 codes= 0;    // The utf8_t data length (in code points)
size_t                 units= 0;    // The utf8_t data length (in bytes)

//----------------------------------------------------------------------------
// Utf8::const_iterator (Data modification not allowed.)
//----------------------------------------------------------------------------
public:
struct const_iterator : public Utf::const_iterator {
   ~const_iterator() = default;     // (Destructor)

   /// Construct iterator from const utf8_t*
   ///
   /// The utf8_t string must not be modified while any iterator uses it.
   /// The Utf8 object must not be modified while begin() iterators exist.
   const_iterator(
     const utf8_t*     origin);     // The string origin

   /// Standard constructors
   const_iterator() = default;      // (Default constructor)
   const_iterator(const const_iterator& src) = default; // (Copy constructor)
   const_iterator(const_iterator&& src) = default; // (Move constructor)

const_iterator&
   operator=(const const_iterator& src) = default; // (Copy assignment)
const_iterator&
   operator=(const_iterator&& src) = default; // (Move assignment)

const_iterator
   operator++(int)                  // (Postfix operator ++, returning copy)
{  const_iterator R(*this); operator++(); return R; }
const_iterator&
   operator++();                    // (Prefix ++ operator, returning *this)
}; // struct const_iterator

const const_iterator
   begin() const                    // The begin() iterator
{  const_iterator R(data); return R; }

//----------------------------------------------------------------------------
// Utf8::Destructor/Constructors
//----------------------------------------------------------------------------
   ~Utf8( void );                   // Destructor

   Utf8( void );                    // Default constructor
   Utf8(const Utf8&    src);        // Copy constructor
   Utf8(      Utf8&&   src);        // Move constructor

   Utf8(const utf8_t*  src);        // Construct from utf8_t string
   Utf8(const utf16_t* src);        // Construct from utf16_t string
   Utf8(const utf32_t* src);        // Construct from utf32_t string
   Utf8(const char*    src)         // Construct from C string
:  Utf8((const utf8_t*) src) {}     // (Always utf8_t string)

   Utf8(const Utf16&   src);        // Constructor
   Utf8(const Utf32&   src);        // Constructor

static Init                         // Initializer
   get_init(const utf8_t*);
Init                                // Initializer
   get_init( void ) const;
void
   init(                            // Initialize
     const Init&       init);       // Using this initializer

//----------------------------------------------------------------------------
// Utf8::Operators
//----------------------------------------------------------------------------
Utf8& operator= (const Utf8&   src); // Assign copy Utf8 object
Utf8& operator= (      Utf8&&  src); // Assign move Utf8 object

Utf8& operator= (const Utf16&  src); // Assign copy Utf16 object
Utf8& operator= (const Utf32&  src); // Assign copy Utf32 object

Utf8& operator= (const utf8_t* src); // Assign copy utf_8 string
Utf8& operator= (const char*   src)  // Assign copy C string
{  return operator=((utf8_t*)src); }

//----------------------------------------------------------------------------
// Utf8::Public accessor methods
//----------------------------------------------------------------------------
// (DO NOT modify the returned utf8_t buffer)
// The buffer remains valid until any non-const method is invoked)
const utf8_t*                       // The utf8_t buffer, null terminated
   get_data( void ) const           // Get utf8_t buffer
{  return data; }

size_t                              // The length of the utf8_t buffer
   get_units( void ) const          // Get length (in bytes)
{  return units; }

size_t                              // The length of the utf8_t buffer
   get_codes( void ) const          // Get length (in code points)
{  return codes; }

//----------------------------------------------------------------------------
// Decode the next UTF32 encoding in the encoding buffer.
static utf32_t                      // The next UTF32 code point
   decode(                          // Decode next code point
     const utf8_t*     buff);       // Encoding buffer pointer

//----------------------------------------------------------------------------
// Encode a UTF32 code point, setting the encoding buffer.
static unsigned                     // The UTF8 encoding length
   encode(                          // Get UTF8 encoding
     utf32_t           code,        // For this code point
     utf8_t*           buff);       // (OUT) Encoding buffer

//----------------------------------------------------------------------------
// Get byte offset of code index
static size_t                       // The utf8_t* offset
   index(                           // Get utf8_t* offset for
     const utf8_t*     addr,        // This ('\0' terminated) utf8_t* string
     size_t            X);          // And this code point index

static size_t                       // The char* offset
   index(                           // Get char* offset for
     const char*       addr,        // This character string
     size_t            X)           // And this logical index
{  return index((const utf8_t*)addr, X); }

//----------------------------------------------------------------------------
// Get the number of bytes required to encode a UTF32 code point.
static unsigned                     // The UTF8 byte length of the code point
   length(                          // Get UTF8 byte length of a code point
     utf32_t           code)        // The code point (No error checking)
{
   if( !is_unicode(code) )          // If code point is invalid
     code= UNI_REPLACEMENT;         // (Use replacement code length)

   if( code < 0x000080 )
     return 1;
   if( code < 0x000800 )
     return 2;
   if( code < 0x010000 )
     return 3;
   return 4;
}

static unsigned                     // The next encoding length
   length(                          // Get UTF8 byte length
     const utf8_t*     buff);       // For this encoding

static unsigned                     // The next encoding length
   length(                          // Get UTF8 byte length
     const char*       buff)        // For this encoding
{  return length((const utf8_t*)buff); }

//----------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------
void
   reset( void );                   // Reset (empty) the Utf8
}; // class Utf8

//----------------------------------------------------------------------------
//
// Class-
//       pub::Utf16
//
// Purpose-
//       UTF16 encoder/decoder.
//
// UTF16 Encoding-
// Bytes Bits    First     Last  Byte[0]  Byte[1]  Byte[3]  Byte[4]
//     2   16 U+000000 U+00FFFF ------8- ------8-      N/A      N/A
//     4   32 U+010000 U+10FFFF 110110-- ------8- 110111-- ------8-
//
//     2   16 U+00D800 U+00DFFF Disallowed: UTF16 surrogate pairs
//     4   32 U+110000 U+1FFFFF Outside Unicode range; Cannot be encoded.
//
// Usage notes-
//       This class partially implements RFC 2781, UTF16 translation format.
//       Little endian data support is limited to input:
//         * Input: byte order marks may specify little endian data format.
//           If present, the byte order mark is translated into big endian.
//         * This class stores and presents UTF-16 data in big endian format
//           without regard to the endian-ness of the machine.
//
//----------------------------------------------------------------------------
class Utf16 : public Utf {
//----------------------------------------------------------------------------
// Utf16::Attributes
//----------------------------------------------------------------------------
protected:
utf16_t*               data= nullptr; // The utf16_t data
size_t                 size= 0;     // The allocated length (in units)

size_t                 codes= 0;    // The utf16_t data length (in code points)
size_t                 units= 0;    // The utf16_t data length (in units)

//----------------------------------------------------------------------------
// Utf16::const_iterator (Data modification not allowed.)
//----------------------------------------------------------------------------
public:
struct const_iterator : public Utf::const_iterator {
   ~const_iterator() = default;     // (Destructor)

   /// Construct iterator from const utf16_t*
   ///
   /// The utf16_t string must not be modified while any iterator uses it.
   /// The Utf16 object must not be modified while begin() iterators exist.
   const_iterator(
     const utf16_t*    origin);     // The string origin

   /// Standard constructors
   const_iterator() = default;      // (Default constructor)
   const_iterator(const const_iterator& src) = default; // (Copy constructor)
   const_iterator(const_iterator&& src) = default; // (Move constructor)

const_iterator&
   operator=(const const_iterator& src) = default; // (Copy assignment)
const_iterator&
   operator=(const_iterator&& src) = default; // (Move assignment)

const_iterator
   operator++(int)                  // (Postfix operator ++, returning copy)
{  const_iterator R(*this); operator++(); return R; }
const_iterator&
   operator++();                    // (Prefix ++ operator, returning *this)
}; // struct const_iterator

const const_iterator
   begin() const                    // The begin() iterator
{  const_iterator R(data); return R; }

//----------------------------------------------------------------------------
// Utf16::Destructor/Constructors/Initializers
//----------------------------------------------------------------------------
public:
   ~Utf16( void );                  // Destructor

   Utf16( void );                   // Default constructor
   Utf16(const Utf16&  src);        // Copy constructor
   Utf16(      Utf16&& src);        // Move constructor

   Utf16(const utf8_t* src);        // Construct from utf8_t string
   Utf16(const utf16_t*src);        // Construct from utf16_t string
   Utf16(const utf32_t*src);        // Construct from utf32_t string
   Utf16(const char*   src)         // Construct from C string
:  Utf16((const utf8_t*) src) {}    // (Always utf8_t string)

   Utf16(const Utf8&   src);        // Constructor
   Utf16(const Utf32&  src);        // Constructor

static Init                         // Initializer
   get_init(const utf16_t*);
Init                                // Initializer
   get_init( void ) const;
void
   init(                            // Initialize
     const Init&       init);       // Using this initializer

//----------------------------------------------------------------------------
// Utf16::Operators
//----------------------------------------------------------------------------
Utf16& operator= (const Utf16&  src); // Assign copy Utf16 object
Utf16& operator= (      Utf16&& src); // Assign move Utf16 object

Utf16& operator= (const Utf8&   src); // Assign copy Utf8  object
Utf16& operator= (const Utf32&  src); // Assign copy Utf32 object

Utf16& operator= (const utf8_t* src); // Assign copy utf_8 string
Utf16& operator= (const char*   src)  // Assign copy C string
{  return operator=((utf8_t*) src); }

//----------------------------------------------------------------------------
// Utf16::Public accessor methods
//----------------------------------------------------------------------------
// (DO NOT modify the returned utf16_t buffer)
// The buffer remains valid until any non-const method is invoked)
const utf16_t*                      // The utf16_t buffer, null terminated
   get_data( void ) const           // Get utf16_t buffer
{  return data; }

size_t                              // The length of the utf16_t buffer
   get_units( void ) const          // Get length (in units)
{  return units; }

size_t                              // The length of the utf16_t buffer
   get_codes( void ) const          // Get length (in code points)
{  return codes; }

//----------------------------------------------------------------------------
// Decode the next UTF32 code point in the encoding buffer.
static utf32_t                      // The next UTF32 code point
   decode(                          // Decode next code point
     const utf16_t*    buff);       // Encoding buffer pointer

//----------------------------------------------------------------------------
// Encode a UTF32 code point, setting the encoding buffer.
static unsigned                     // The UTF8 encoding length
   encode(                          // Get UTF8 encoding
     utf32_t           code,        // For this code point
     utf16_t*          buff);       // (OUT) Encoding buffer (big endian)

//----------------------------------------------------------------------------
// Get the number of units required to encode a UTF32 code point.
static unsigned                     // The Utf16 byte length of the code point
   length(                          // Get Utf16 byte length of a code point
     utf32_t           code)        // The code point (No error checking)
{
   if( code < 0x010000 )
     return 1;

   return 2;
}

//----------------------------------------------------------------------------
// Utf16::Public methods
//----------------------------------------------------------------------------
void
   reset( void );                   // Reset (empty) the Utf16
}; // class Utf16

//----------------------------------------------------------------------------
//
// Class-
//       pub::Utf32
//
// Purpose-
//       Utf32 encoder/decoder.
//
// UTF32 Encoding-
// Bytes Bits    First     Last  Byte[0]  Byte[1]  Byte[3]  Byte[4]
//     4   31 U+000000 U+10FFFF ------8- ------8- ------8- ------8-
//
//     4   16 U+00D800 U+00DFFF Disallowed: UTF16 surrogate pairs
//     4   32 U+110000 && above Outside Unicode range.
//
// Usage notes-
//       This class stores and presents data in native endian format.
//       (Use Utf8 or Utf16 for external data transport.)
//
//----------------------------------------------------------------------------
class Utf32 : public Utf {
//----------------------------------------------------------------------------
// Attributes
//----------------------------------------------------------------------------
protected:
utf32_t*               data= nullptr;  // The utf32_t data
size_t                 size= 0;     // The allocated length (in units)

size_t                 codes= 0;    // The utf32_t data length (in code points)
//                     units= codes; // (By definition)

//----------------------------------------------------------------------------
// Utf32::const_iterator (Data modification not allowed.)
//----------------------------------------------------------------------------
public:
struct const_iterator : public Utf::const_iterator {
   ~const_iterator() = default;     // (Destructor)

   /// Construct iterator from const utf32_t*
   ///
   /// The utf32_t string must not be modified while any iterator uses it.
   /// The Utf32 object must not be modified while begin() iterators exist.
   const_iterator(
     const utf32_t*    origin);     // The string origin

   /// Standard constructors
   const_iterator() = default;      // (Default constructor)
   const_iterator(const const_iterator& src) = default; // (Copy constructor)
   const_iterator(const_iterator&& src) = default; // (Move constructor)

const_iterator&
   operator=(const const_iterator& src) = default; // (Copy assignment)
const_iterator&
   operator=(const_iterator&& src) = default; // (Move assignment)

const_iterator
   operator++(int)                  // (Postfix operator ++, returning copy)
{  const_iterator R(*this); operator++(); return R; }
const_iterator&
   operator++();                    // (Prefix ++ operator, returning *this)
}; // struct const_iterator

const const_iterator
   begin() const                    // The begin() iterator
{  const_iterator R(data); return R; }

//----------------------------------------------------------------------------
// Utf32::Destructor/Constructors
//----------------------------------------------------------------------------
public:
   ~Utf32( void );                  // Destructor

   Utf32( void );                   // Default constructor
   Utf32(const Utf32&  src);        // Copy constructor
   Utf32(      Utf32&& src);        // Move constructor

   Utf32(const utf8_t* src);        // Construct from utf8_t string
   Utf32(const utf16_t*src);        // Construct from utf16_t string
   Utf32(const utf32_t*src);        // Construct from utf32_t string
   Utf32(const char*   src)         // Construct from C string
:  Utf32((const utf8_t*) src) {}    // (Always utf8_t string)

   Utf32(const Utf8&   src);        // Constructor
   Utf32(const Utf16&  src);        // Constructor

static Init                         // Initializer
   get_init(const utf32_t*);
Init                                // Initializer
   get_init( void ) const;
void
   init(                            // Initialize
     const Init&       init);       // Using this initializer

//----------------------------------------------------------------------------
// Operators
//----------------------------------------------------------------------------
Utf32& operator= (const Utf32&  src); // Assign copy Utf32 object
Utf32& operator= (      Utf32&& src); // Assign move Utf32 object

Utf32& operator= (const Utf8&   src); // Assign copy Utf8  object
Utf32& operator= (const Utf16&  src); // Assign copy Utf16 object

Utf32& operator= (const utf8_t* src); // Assign copy utf_8 string
Utf32& operator= (const char*   src)  // Assign copy C string
{  return operator=((utf8_t*) src); }

//----------------------------------------------------------------------------
// Utf32::Public accessor methods
//----------------------------------------------------------------------------
// (DO NOT modify the returned utf32_t buffer)
// The buffer remains valid until any non-const method is invoked)
const utf32_t*                      // The utf32_t buffer, null terminated
   get_data( void ) const           // Get utf32_t buffer
{  return data; }

size_t                              // The length of the utf32_t buffer
   get_units( void ) const          // Get length (in units)
{  return codes; }

size_t                              // The length of the utf32_t buffer
   get_codes( void ) const          // Get length (in code points)
{  return codes; }

//----------------------------------------------------------------------------
// Decode the next UTF32 code point in the encoding buffer.
static utf32_t                      // The next UTF32 code point
   decode(                          // Decode next code point
     const utf32_t*    buff);       // Encoding buffer pointer

//----------------------------------------------------------------------------
// Encode a UTF32 code point, setting the encoding buffer.
static unsigned                     // The UTF8 encoding length
   encode(                          // Get UTF8 encoding
     utf32_t           code,        // For this code point
     utf32_t*          buff);       // (OUT) Encoding buffer

//----------------------------------------------------------------------------
// Utf32::Public methods
//----------------------------------------------------------------------------
void
   reset( void );                   // Reset (empty) the Utf32
}; // class Utf32
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_UTF_H_INCLUDED
