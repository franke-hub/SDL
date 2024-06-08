//----------------------------------------------------------------------------
//
//       Copyright (c) 2021-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Utf.h
//
// Purpose-
//       UTF utilities
//
// Last change date-
//       2024/06/07
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_UTF_H_INCLUDED
#define _LIBPUB_UTF_H_INCLUDED

#include <cstdint>                  // For uint8_t, ...
#include <iterator>                 // For std::forward_iterator_tag
#include <string>                   // For std::string

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Utf8;                         // UTF8  container
class Utf16;                        // UTF16 container
class Utf32;                        // UTF32 container

// Version[1]: Current (working) version; Version[2]: utf8_decode/encode
#define USE_UTF_VERSION 1

//----------------------------------------------------------------------------
//
// Class-
//       Utf
//
// Purpose-
//       Unicode Transformation Format base class.
//
// Definitions-
//       C-string:   A '\0' terminated char[] sequence.
//       U-string:   A U8-string, U16-string, or U32-string.
//       U8-string:  A utf8_t(0)  terminated utf8_t[] sequence.
//       U16-string: A utf16_t(0) terminated utf16_t[] sequence.
//       U32-string: A utf32_t(0) terminated utf32_t[] sequence.
//
//       native units: Encoding units.
//           For a  U8-string, an encoding unit is a utf8_t
//           For a U16-string, an encoding unit is a utf16_t
//           For a U32-string, an encoding unit is a utf32_t
//
// UTF-8 Encoding-
// Bytes Bits    First     Last  Byte[0]  Byte[1]  Byte[2]  Byte[3]
//     1    7 U+000000 U+00007F 0-----7-      N/A      N/A      N/A ( 7 bits)
//     2   11 U+000080 U+0007FF 110---5- 10----6-      N/A      N/A (11 bits)
//     3   16 U+000800 U+00D7FF 1110--4- 10----6- 10----6-      N/A (16 bits)
//     3   16 U+00D800 U+00DFFF Disallowed: UTF16 surrogate pairs
//     3   16 U+00E000 U+00FFFF 1110--4- 10----6- 10----6-      N/A (16 bits)
//     4   21 U+010000 U+10FFFF 11110-3- 10----6- 10----6- 10----6- (21 bits)
//
// UTF-16 Encoding-
// Bytes Bits    First     Last  Byte[0]  Byte[1]  Byte[3]  Byte[4]
//     2   16 U+000000 U+00FFFF ------8- ------8-      N/A      N/A
//     4   32 U+010000 U+10FFFF 110110-- ------8- 110111-- ------8-
//
//     Encoding values U+D800-U+DFFF may only appear in surrogate pairs.
//     These pairs encode the number range 0x00000-0xFFFFF.
//     The first (or leading) surrogate is in the range 0xDB00-DBFF.
//     The second (or trailing) surrogate is in the range 0xDC00-DFFF.
//     Each surrogate contributes 10 bits to the 20 bit encoding range.
//     When decoding, 0x010000 is added to the pair value, making the
//     effective surrogate pair range 0x010000-10FFFF.
//
// UTF-32 Encoding-
// Bytes Bits    First     Last  Byte[0]  Byte[1]  Byte[3]  Byte[4]
//     4   31 U+000000 U+10FFFF ------8- ------8- ------8- ------8-
//
// Invalid  encoding information-
//     Invalid encodings are replaced by UNI_REPLACEMENT,
//     the Unicode error replacement character.
//
//----------------------------------------------------------------------------
class Utf {
public:
//----------------------------------------------------------------------------
// Utf::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef uint8_t        utf8_t;      // The UTF-8  encoding type
typedef uint16_t       utf16_t;     // The UTF-16 encoding type
typedef uint16_t       utf16BE_t;   // The UTF-16BE encoding type
typedef uint16_t       utf16LE_t;   // The UTF-16LE encoding type
typedef uint32_t       utf32_t;     // The UTF-32 encoding type
typedef uint32_t       utf32BE_t;   // The UTF-32BE encoding type
typedef uint32_t       utf32LE_t;   // The UTF-32LE encoding type

typedef size_t         Column;      // Offset/index in codepoints
typedef size_t         Points;      // Length in codepoints
typedef size_t         Offset;      // Offset/index in native units
typedef size_t         Length;      // Length in native units

enum                                // Unicode characters
{  BYTE_ORDER_MARK=    0x0000'FEFF  // Byte Order Mark, a.k.a BOM
,  MARK_ORDER_BYTE=    0x0000'FFFE  // Little endian Byte Order Mark
,  UNI_REPLACEMENT=    0x0000'FFFD  // Unicode error replacement character
}; // enum Unicode characters

enum MODE                           // Decoding/encoding mode
{  MODE_BE= 0                       // Big endian mode
,  MODE_LE                          // Little endian mode
}; // enum MODE

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
// Utf::Static iterators
//----------------------------------------------------------------------------
static const const_iterator
                       the_end;     // The default end iterator
static const const_iterator&        // Note: All end iterators are equal
   end()                            // Get the built-in end iterator
{  return the_end; }                // (Conveniently inlinable)

//----------------------------------------------------------------------------
// Utf::Static utility methods
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff codepoint is combining
   is_combining(                    // Is code a combining character?
     utf32_t           code);       // (The codepoint)

static inline bool                  // TRUE iff codepoint is valid unicode
   is_unicode(                      // Is codepoint valid unicode?
     utf32_t           code);       // (The codepoint)

static inline Length                // Length (in native units)
   strlen(                          // Get length (in bytes)
     const utf16_t*    addr);       // Of this U16-string

static inline Length                // Length (in native units)
   strlen(                          // Get length (in bytes)
     const utf32_t*    addr);       // Of this U32-string
}; // class Utf

//----------------------------------------------------------------------------
//
// Class-
//       Utf8
//
// Purpose-
//       UTF8 encoder/decoder.
//
// Implementation notes-
//       This class implements RFC 3629, UTF8 translation format.
//
//----------------------------------------------------------------------------
class Utf8 : public Utf {
//----------------------------------------------------------------------------
// Utf8::Attributes
//----------------------------------------------------------------------------
protected:
utf8_t*                data= nullptr; // The utf8_t data
size_t                 size= 0;     // The allocated length (in bytes)

size_t                 codes= 0;    // The utf8_t data length (in codepoints)
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
   get_codes( void ) const          // Get length (in codepoints)
{  return codes; }

static size_t                       // The length of the (utf8) string
   get_codes(                       // Get length (in codepoints)
     const std::string src);        // Of this string

//----------------------------------------------------------------------------
// Decode the next UTF32 encoding in the encoding buffer.
static utf32_t                      // The next UTF32 codepoint
   decode(                          // Decode next codepoint
     const utf8_t*     buff);       // Encoding buffer pointer

//----------------------------------------------------------------------------
// Encode a UTF32 codepoint, setting the encoding buffer.
static unsigned                     // The UTF8 encoding length
   encode(                          // Get UTF8 encoding
     utf32_t           code,        // For this codepoint
     utf8_t*           buff);       // (OUT) Encoding buffer

//----------------------------------------------------------------------------
// Get byte offset of code index
static size_t                       // The utf8_t* offset
   index(                           // Get utf8_t* offset for
     const utf8_t*     addr,        // This ('\0' terminated) utf8_t* string
     size_t            X);          // And this codepoint index

static size_t                       // The char* offset
   index(                           // Get char* offset for
     const char*       addr,        // This character string
     size_t            X)           // And this logical index
{  return index((const utf8_t*)addr, X); }

//----------------------------------------------------------------------------
// Get the number of bytes required to encode a UTF32 codepoint.
static unsigned                     // The UTF8 byte length of the codepoint
   length(                          // Get UTF8 byte length of a codepoint
     utf32_t           code)        // The codepoint (No error checking)
{
   if( !is_unicode(code) )          // If codepoint is invalid
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
//       Utf16
//
// Purpose-
//       UTF16 encoder/decoder.
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

size_t                 codes= 0;    // The utf16_t data length (in codepoints)
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
   get_codes( void ) const          // Get length (in codepoints)
{  return codes; }

//----------------------------------------------------------------------------
// Decode the next UTF32 codepoint in the encoding buffer.
static utf32_t                      // The next UTF32 codepoint
   decode(                          // Decode next codepoint
     const utf16_t*    buff);       // Encoding buffer pointer

//----------------------------------------------------------------------------
// Encode a UTF32 codepoint, setting the encoding buffer.
static unsigned                     // The UTF8 encoding length
   encode(                          // Get UTF8 encoding
     utf32_t           code,        // For this codepoint
     utf16_t*          buff);       // (OUT) Encoding buffer (big endian)

//----------------------------------------------------------------------------
// Get the number of units required to encode a UTF32 codepoint.
static unsigned                     // The Utf16 byte length of the codepoint
   length(                          // Get Utf16 byte length of a codepoint
     utf32_t           code)        // The codepoint (No error checking)
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
//       Utf32
//
// Purpose-
//       Utf32 encoder/decoder.
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

size_t                 codes= 0;    // The utf32_t data length (in codepoints)
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
   get_codes( void ) const          // Get length (in codepoints)
{  return codes; }

//----------------------------------------------------------------------------
// Decode the next UTF32 codepoint in the encoding buffer.
static utf32_t                      // The next UTF32 codepoint
   decode(                          // Decode next codepoint
     const utf32_t*    buff);       // Encoding buffer pointer

//----------------------------------------------------------------------------
// Encode a UTF32 codepoint, setting the encoding buffer.
static unsigned                     // The UTF8 encoding length
   encode(                          // Get UTF8 encoding
     utf32_t           code,        // For this codepoint
     utf32_t*          buff);       // (OUT) Encoding buffer

//----------------------------------------------------------------------------
// Utf32::Public methods
//----------------------------------------------------------------------------
void
   reset( void );                   // Reset (empty) the Utf32
}; // class Utf32

//----------------------------------------------------------------------------
//
// Struct-
//       utf_error
//
// Purpose-
//       The utf (runtime) error
//
//----------------------------------------------------------------------------
class utf_error : public std::runtime_error{
   using std::runtime_error::runtime_error;
}; // class utf_error

//----------------------------------------------------------------------------
//
// Struct-
//       utf8_decoder
//
// Purpose-
//       The UTF-8 decoder
//
//----------------------------------------------------------------------------
struct utf8_decoder : public Utf {
//----------------------------------------------------------------------------
// utf8_decoder::Attributes
//----------------------------------------------------------------------------
protected:
const utf8_t*          buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in bytes
Column                 column= 0;   // Current buffer codepoint index
Offset                 offset= 0;   // Current buffer byte index

//----------------------------------------------------------------------------
// utf8_decoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf8_decoder( void ) = default;  // Default constructor
   utf8_decoder(const utf8_decoder&); // Copy constructor

   utf8_decoder(const utf8_t*, Length); // Constructor (size known)
   utf8_decoder(const utf8_t*);     // U-string constructor
   utf8_decoder(const char* addr)   // C-string constructor
{  utf8_decoder((const utf8_t*) addr); } // (Use U-string constructor)

   ~utf8_decoder( void ) = default; // Destructor does nothing

//----------------------------------------------------------------------------
// utf8_decoder::Accessor methods
//----------------------------------------------------------------------------
utf8_decoder                        // The current column substring
   copy_column( void ) const;       // Get current column substring

Points
   get_points( void );              // Get the total number of codepoints

Column                              // The current column index
   get_column( void ) const         // Get current column index
{  return column; }

Offset
   get_offset( void ) const         // Get current offset
{  return offset; }

/**
   @brief Set as specified.
   @param IX The specified column number.
   @return The number of characters past the end of the decoder range.
       If `IX` is within the decoder range, column is set to IX, offset is set
       appropriately, and zero is returned.
       If `IX` is past the end of the decode buffer, the column is set to
       position is set to the end of the buffer and the method returns
       index - column.
**/
Length                              // Number of characters past end of buffer
   index(Column        IX);         // Index to this column

static bool                         // TRUE iff codepoint is combining
   is_combining(                    // Is code a combining character?
     utf32_t           code)        // (The codepoint)
{  return Utf::is_combining(code); }

bool
   is_combining( void ) const       // Is the current character combining?
{  return is_combining(current()); }

//----------------------------------------------------------------------------
// utf8_decoder::Methods
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   current( void ) const;           // Decode the current codepoint

utf32_t                             // The current codepoint
   decode( void );                  // Decode, updating the current codepoint

void
   reset(const utf8_t* addr=nullptr, Length= 0); // Reset the decoder
}; // struct utf8_decoder

//----------------------------------------------------------------------------
//
// Struct-
//       utf8_encoder
//
// Purpose-
//       UTF-8 encoder
//
//----------------------------------------------------------------------------
struct utf8_encoder : public Utf {
//----------------------------------------------------------------------------
// utf8_encoder::Attributes
//----------------------------------------------------------------------------
protected:
utf8_t*                buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in bytes
Column                 column= 0;   // Current buffer codepoint index
Offset                 offset= 0;   // Current buffer byte index

//----------------------------------------------------------------------------
// utf8_encoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf8_encoder( void ) = default;  // Default constructor
   utf8_encoder(const utf8_encoder&) = delete; // NO copy constructor

   utf8_encoder(utf8_t*, Length);   // Address/length constructor

   ~utf8_encoder( void ) = default; // Destructor does nothing

//----------------------------------------------------------------------------
// utf8_encoder::Accessor methods
//----------------------------------------------------------------------------
Column                              // The current column index
   get_column( void ) const         // Get current column index
{  return column; }

Offset
   get_offset( void ) const         // Get current offset
{  return offset; }

//----------------------------------------------------------------------------
// utf8_encoder::Methods
//----------------------------------------------------------------------------
/**
   @brief Encode the next codepoint, updating column and offset.
   @param codepoint The codepoint.
       Invalid codepoints are replaced UNI_REPLACEMENT, the error replacement
       codepoint.
   @return The encoding byte length.
       If the codepoint cannot be encoded within the data buffer, zero is
       returned and the utf8_encoder is unchanged.
**/
unsigned                            // Encoding byte Length
   encode(utf32_t);                 // Encode this value

void
   reset(utf8_t* addr=nullptr, Length= 0); // Reset the encoder
}; // struct utf8_encoder

//----------------------------------------------------------------------------
//
// Struct-
//       utf16_decoder
//
// Purpose-
//       The UTF-16 decoder
//
//----------------------------------------------------------------------------
struct utf16_decoder : public Utf {
//----------------------------------------------------------------------------
// utf16_decoder::Attributes
//----------------------------------------------------------------------------
protected:
const utf16_t*         buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in native units
Column                 column= 0;   // Current buffer codepoint index
Offset                 offset= 0;   // Current buffer native unit index
MODE                   mode= MODE_BE; // Default, big endian encoding mode

//----------------------------------------------------------------------------
// utf16_decoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf16_decoder( void ) = default; // Default constructor
   utf16_decoder(const utf16_decoder&); // Copy constructor

   utf16_decoder(const utf16_t*, Length); // Constructor (size known)
   utf16_decoder(const utf16_t*);   // U-string constructor

   ~utf16_decoder( void ) = default; // Destructor (does nothing)

//----------------------------------------------------------------------------
// utf16_decoder::Accessor methods
//----------------------------------------------------------------------------
#if 0
utf16_decoder                       // The current column substring
   copy_column( void ) const;       // Get current column substring

Points
   get_points( void );              // Get the total number of codepoints
#endif

Column                              // The current column index
   get_column( void ) const         // Get current column index
{  return column; }

Offset
   get_offset( void ) const         // Get current (byte) offset
{  return offset; }

static bool                         // TRUE iff codepoint is combining
   is_combining(                    // Is code a combining character?
     utf32_t           code)        // (The codepoint)
{  return Utf::is_combining(code); }

bool
   is_combining( void ) const       // Is the current character combining?
{  return is_combining(current()); }

/**
   @brief Set the decoding mode
   @param m The decoding mode
     The decoding mode may only be set before any decoding has occurred,
     and cannot override a Byte Order Mark in the decoding buffer.
     (A utf_error is thrown if this restriction is violated.)
**/
void
   set_mode(MODE);                  // Set encoding mode

//----------------------------------------------------------------------------
// utf16_decoder::Methods
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   current( void ) const;           // Retrieve the current codepoint

utf32_t                             // The current codepoint
   decode( void );                  // Decode, updating the current codepoint

/**
   @brief Set the specified column index.
   @param IX The specified column number.
   @return The number of characters (bytes) past the end of the decoder range.
       If `IX` is within the decoder range, column is set to IX, offset is set
       appropriately, and zero is returned.
       If `IX` is past the end of the decode buffer, the column is set to
       position is set to the end of the buffer and the method returns
       index - column.
**/
Length                              // Number of characters past end of buffer
   index(Column        IX);         // Index to this column

void
   reset(const utf16_t* addr=nullptr, Length= 0); // Reset the decoder
}; // struct utf16_decoder

//----------------------------------------------------------------------------
//
// Struct-
//       utf16_encoder
//
// Purpose-
//       The UTF-16 encoder
//
//----------------------------------------------------------------------------
struct utf16_encoder : public Utf {
//----------------------------------------------------------------------------
// utf16_encoder::Attributes
//----------------------------------------------------------------------------
protected:
utf16_t*               buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in bytes
Column                 column= 0;   // Current buffer codepoint index
Offset                 offset= 0;   // Current buffer byte index
MODE                   mode= MODE_BE; // Default, big endian encoding mode

//----------------------------------------------------------------------------
// utf16_encoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf16_encoder( void ) = default; // Default constructor
   utf16_encoder(const utf16_encoder&) = delete; // NO copy constructor

   utf16_encoder(utf16_t*, Length); // Constructor (size known)

   ~utf16_encoder( void ) = default; // Destructor (does nothing)

//----------------------------------------------------------------------------
// utf16_encoder::Accessor methods
//----------------------------------------------------------------------------
Column                              // The current column index
   get_column( void ) const         // Get current column index
{  return column; }

Offset
   get_offset( void ) const         // Get current (byte) offset
{  return offset; }

/**
   @brief Set the encoding mode
   @param m The encoding mode
     The encoding mode may only be set before any encoding has occurred.
     (A utf_error is thrown if this restriction is violated or the specified
     MODE is invalid.)
**/
void
   set_mode(MODE m);                // Set encoding mode

//----------------------------------------------------------------------------
// utf16_encoder::Methods
//----------------------------------------------------------------------------
unsigned                            // The encoding length, in units
   encode(utf32_t);                 // Encode, updating the current codepoint

void
   reset(utf16_t* addr=nullptr, Length= 0); // Reset the encoder
}; // struct utf16_encoder

//----------------------------------------------------------------------------
//
// Struct-
//       utf32_decoder
//
// Purpose-
//       The UTF-32 decoder
//
//----------------------------------------------------------------------------
struct utf32_decoder : public Utf {
//----------------------------------------------------------------------------
// utf32_decoder::Attributes
//----------------------------------------------------------------------------
protected:
const utf32_t*         buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in native units
Column                 column= 0;   // Current buffer codepoint index
Offset                 offset= 0;   // Current buffer native unit index
MODE                   mode= MODE_BE; // Default, big endian encoding mode

//----------------------------------------------------------------------------
// utf32_decoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf32_decoder( void ) = default; // Default constructor
   utf32_decoder(const utf32_decoder&); // Copy constructor

   utf32_decoder(const utf32_t*, Length); // Constructor (size known)
   utf32_decoder(const utf32_t*);   // U-string constructor

   ~utf32_decoder( void ) = default; // Destructor (does nothing)

//----------------------------------------------------------------------------
// utf32_decoder::Accessor methods
//----------------------------------------------------------------------------
#if 0
utf32_decoder                       // The current column substring
   copy_column( void ) const;       // Get current column substring

Points
   get_points( void );              // Get the total number of codepoints
#endif

Column                              // The current column index
   get_column( void ) const         // Get current column index
{  return column; }

Offset
   get_offset( void ) const         // Get current (byte) offset
{  return offset; }

static bool                         // TRUE iff codepoint is combining
   is_combining(                    // Is code a combining character?
     utf32_t           code)        // (The codepoint)
{  return Utf::is_combining(code); }

bool
   is_combining( void ) const       // Is the current character combining?
{  return is_combining(current()); }

/**
   @brief Set the decoding mode
   @param m The decoding mode
     The decoding mode may only be set before any decoding has occurred,
     and cannot override a Byte Order Mark in the decoding buffer.
     (A utf_error is thrown if this restriction is violated.)
**/
void
   set_mode(MODE);                  // Set encoding mode

//----------------------------------------------------------------------------
// utf32_decoder::Methods
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   current( void ) const;           // Retrieve the current codepoint

utf32_t                             // The current codepoint
   decode( void );                  // Decode, updating the current codepoint

/**
   @brief Set the specified column index.
   @param IX The specified column number.
   @return The number of characters (bytes) past the end of the decoder range.
       If `IX` is within the decoder range, column is set to IX, offset is set
       appropriately, and zero is returned.
       If `IX` is past the end of the decode buffer, the column is set to
       position is set to the end of the buffer and the method returns
       index - column.
**/
Length                              // Number of characters past end of buffer
   index(Column        IX);         // Index to this column

void
   reset(const utf32_t* addr=nullptr, Length= 0); // Reset the decoder
}; // struct utf32_decoder

//----------------------------------------------------------------------------
//
// Struct-
//       utf32_encoder
//
// Purpose-
//       The UTF-32 encoder
//
//----------------------------------------------------------------------------
struct utf32_encoder : public Utf {
//----------------------------------------------------------------------------
// utf32_encoder::Attributes
//----------------------------------------------------------------------------
protected:
utf32_t*               buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in bytes
Column                 column= 0;   // Current buffer codepoint index
Offset                 offset= 0;   // Current buffer byte index
MODE                   mode= MODE_BE; // Default, big endian encoding mode

//----------------------------------------------------------------------------
// utf32_encoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf32_encoder( void ) = default; // Default constructor
   utf32_encoder(const utf32_encoder&) = delete; // NO copy constructor

   utf32_encoder(utf32_t*, Length); // Constructor (size known)

   ~utf32_encoder( void ) = default; // Destructor (does nothing)

//----------------------------------------------------------------------------
// utf32_encoder::Accessor methods
//----------------------------------------------------------------------------
Column                              // The current column index
   get_column( void ) const         // Get current column index
{  return column; }

Offset
   get_offset( void ) const         // Get current (byte) offset
{  return offset; }

/**
   @brief Set the encoding mode
   @param m The encoding mode
     The encoding mode may only be set before any encoding has occurred.
     (A utf_error is thrown if this restriction is violated or the specified
     MODE is invalid.)
**/
void
   set_mode(MODE m);                // Set encoding mode

//----------------------------------------------------------------------------
// utf32_encoder::Methods
//----------------------------------------------------------------------------
unsigned                            // The encoding length, in units
   encode(utf32_t);                 // Encode, updating the current codepoint

void
   reset(utf32_t* addr=nullptr, Length= 0); // Reset the encoder
}; // struct utf32_encoder
_LIBPUB_END_NAMESPACE

#include "bits/Utf.i"               // Inline implementations
#endif // _LIBPUB_UTF_H_INCLUDED
