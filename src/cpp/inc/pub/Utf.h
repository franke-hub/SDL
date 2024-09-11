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
//       2024/09/12
//
// Usage notes-
//       To expose Utf class types, include "pub/Utf.i"
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_UTF_H_INCLUDED
#define _LIBPUB_UTF_H_INCLUDED

#include <stdexcept>                // For std::utf_invalid_argument, ...
#include <cstdint>                  // For uint8_t, ...
#include <stdio.h>                  // For EOF

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
// Bytes Bits     First      Last  Byte[0]  Byte[1]  Byte[2]  Byte[3]
//     1    7 U+00'0000 U+00'007F 0-----7-      N/A      N/A      N/A ( 7 bits)
//     2   11 U+00'0080 U+00'07FF 110---5- 10----6-      N/A      N/A (11 bits)
//     3   16 U+00'0800 U+00'FFFF 1110--4- 10----6- 10----6-      N/A (16 bits)
//     4   21 U+01'0000 U+10'FFFF 11110-3- 10----6- 10----6- 10----6- (21 bits)
//
//     While it's possible to encode values between U+00'D800 and U+00'DBFF
//     (inclusive,) or greater than U+10'FFFF, such encodings are invalid.
//
// UTF-16 Encoding-
// Bytes Bits     First      Last  Byte[0]  Byte[1]  Byte[3]  Byte[4]
//     2   16 U+00'0000 U+00'FFFF ------8- ------8-      N/A      N/A
//     4   32 U+01'0000 U+10'FFFF 110110-- ------8- 110111-- ------8-
//
//     Encoding values U+D800-U+DFFF may only appear in surrogate pairs.
//     Surrogate pairs encode the UTF number range 0x01'0000-0x10'FFFFF.
//     The first (or leading) surrogate is in the range 0xD800-DBFF.
//     The second (or trailing) surrogate is in the range 0xDC00-DFFF.
//     Each surrogate contributes 10 bits to the 20 bit encoding range.
//     When decoding, 0x01'0000 is added to the pair value, making the
//     effective surrogate pair range 0x01'0000-10'FFFF.
//
// UTF-32 Encoding-
// Bytes Bits     First      Last  Byte[0]  Byte[1]  Byte[3]  Byte[4]
//     4   31 U+00'0000 U+10'FFFF ------8- ------8- ------8- ------8-
//
// Notes-
//     While it's possible to encode values between U+00'D800 and U+00'DBFF
//     (inclusive,) or greater than U+10'FFFF, such encodings are invalid.
//
// Invalid encodings-
//     When decoding or encoding, invalid encodings are silently replaced by
//     UNI_REPLACEMENT, the Unicode error replacement character.
//
//----------------------------------------------------------------------------
class Utf {
public:
//----------------------------------------------------------------------------
// Utf::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef uint8_t        utf8_t;      // The UTF-8    native unit type
typedef uint16_t       utf16_t;     // The UTF-16   native unit type
typedef uint16_t       utf16BE_t;   // The UTF-16BE native unit type
typedef uint16_t       utf16LE_t;   // The UTF-16LE native unit type
typedef uint32_t       utf32_t;     // The UTF-32   native unit type
typedef uint32_t       utf32BE_t;   // The UTF-32BE native unit type
typedef uint32_t       utf32LE_t;   // The UTF-32LE native unit type

typedef size_t         Count;       // A Symbol or column count
typedef size_t         Index;       // A Symbol or column index
typedef size_t         Length;      // Length in native units
typedef size_t         Offset;      // Offset/index in native units
typedef utf32_t        Symbol;      // A UTF symbol (code point)

enum                                // Unicode characters
{  BYTE_ORDER_MARK=    0x0000'FEFF  // Big endian Byte Order Mark, a.k.a BOM
,  MARK_ORDER_BYTE=    0x0000'FFFE  // Little endian Byte Order Mark

,  BYTE_ORDER_MARK32=  0x0000'FEFF  // 32-bit big endian Byte Order Mark
,  MARK_ORDER_BYTE32=  0xFFFE'0000  // 32-bit little endian Byte Order Mark

,  UNI_REPLACEMENT=    0x0000'FFFD  // Unicode error replacement character
}; // enum Unicode characters

enum MODE                           // Decoding/encoding mode
{  MODE_RESET= 0                    // Mode undefined, big endian assumed
,  MODE_BE                          // Big endian mode
,  MODE_LE                          // Little endian mode
}; // enum MODE

// Method decode: No characters remain
constexpr static const Symbol       // (UTF_EOF is an invalid UTF Symbol)
                       UTF_EOF= EOF; // Decode: No characters remain

//----------------------------------------------------------------------------
// Utf::Static utility methods
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff Symbol is a combining character
   is_combining(                    // Is Symbol combining?
     Symbol            code) noexcept; // (The Symbol)

static inline bool                  // TRUE iff Symbol is within unicode range
   is_unicode(                      // Is Symbol within unicode range?
     Symbol            code) noexcept; // (The Symbol)

static inline Length                // Length (in native units)
   utflen(                          // Get length (in native units)
     const utf8_t*    addr) noexcept; // Of this U8-string

static inline Length                // Length (in native units)
   utflen(                          // Get length (in native units)
     const utf16_t*    addr) noexcept; // Of this U16-string

static inline Length                // Length (in native units)
   utflen(                          // Get length (in native units)
     const utf32_t*    addr) noexcept; // Of this U32-string
}; // class Utf

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct utf8_decoder;
struct utf16_decoder;
struct utf32_decoder;
struct utf8_encoder;
struct utf16_encoder;
struct utf32_encoder;

//----------------------------------------------------------------------------
// Exception: pub::utf_invalid_argument (a std::invalid_argument)
//
// Thrown by an decoder or encoder when a method passed an invalid argument.
//----------------------------------------------------------------------------
class utf_invalid_argument : public std::invalid_argument {
   using std::invalid_argument::invalid_argument;
}; // class utf_invalid_argument

//----------------------------------------------------------------------------
// Exception: pub::utf_overflow_error (a std::overflow_error)
//
// Thrown by an encoder when it's assigned from a source that would overflow
// its buffer. The assignment operation is partially complete.
//----------------------------------------------------------------------------
class utf_overflow_error : public std::overflow_error {
   using std::overflow_error::overflow_error;
}; // class utf_overflow_error

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
friend utf8_encoder;                // For operator=

//----------------------------------------------------------------------------
// utf8_decoder::Attributes
//----------------------------------------------------------------------------
protected:
const utf8_t*          buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in native units (bytes)
Index                  column= -1;  // Current buffer column index
Offset                 offset= 0;   // Current buffer unit index

//----------------------------------------------------------------------------
// utf8_decoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf8_decoder( void ) = default;  // Default constructor
   utf8_decoder(const utf8_decoder&) noexcept; // Copy constructor
   utf8_decoder(const utf8_encoder&) noexcept; // Copy constructor

   utf8_decoder(const utf8_t*, Length) noexcept; // Address/Length constructor
   utf8_decoder(const utf8_t*) noexcept; // U-string constructor

   ~utf8_decoder( void ) = default; // Destructor does nothing

// Alias constructors: cast (const char* => const utf8_t*)
   utf8_decoder(const char*, Length) noexcept; // Address/Length constructor
   utf8_decoder(const char*) noexcept; // C-string constructor

//----------------------------------------------------------------------------
// utf8_decoder::Assignment operators
//----------------------------------------------------------------------------
utf8_decoder&
    operator=(const utf8_decoder&) noexcept; // Copy utf8_decoder

utf8_decoder&
    operator=(const utf8_encoder&) noexcept; // Copy utf8_encoder

//----------------------------------------------------------------------------
// utf8_decoder::Accessor methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "") const noexcept; // Debugging message

Count                               // The total column Count
   get_column_count( void ) const;  // Get total column Count

Index                               // The current column Index
   get_column_index( void ) const;  // Get current column Index

Length
   get_length( void ) const         // Get length (in native units)
{  return length; }

Offset
   get_offset( void ) const         // Get current buffer offset
{  return offset; }

Count                               // The total symbol Count
   get_symbol_count( void ) const;  // Get total symbol Count

bool                                // TRUE iff Symbol is a combining character
   is_combining( void ) const       // Is the current Symbol combining?
{  return Utf::is_combining(current()); }

Count                               // Number of units past end of buffer
   set_column_index(Index);         // Set the column index

Count                               // Number of units past end of buffer
   set_symbol_index(Index);         // Set the Symbol index

//----------------------------------------------------------------------------
// utf8_decoder::Methods
//----------------------------------------------------------------------------
utf8_decoder                        // The current column substring
   copy_column( void ) const;       // Get current column substring

Symbol                              // The current Symbol
   current( void ) const;           // Retrieve the current Symbol

Symbol                              // The current Symbol
   decode( void );                  // Decode, updating column and offset

void
   reset(const utf8_t*, Length) noexcept; // Reset and re-initialize the decoder

void
   reset(const char* addr, Length size) noexcept // (Reset alias)
{  reset((const utf8_t*)addr, size); }

void
   reset( void ) noexcept;          // Reset the decoder to it's initial state
}; // struct utf8_decoder

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
friend utf16_encoder;               // For operator=

//----------------------------------------------------------------------------
// utf16_decoder::Attributes
//----------------------------------------------------------------------------
protected:
const utf16_t*         buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in native units
Index                  column= -1;  // Current buffer column index
Offset                 offset= 0;   // Current buffer native unit index
MODE                   mode= MODE_RESET; // Default, big endian encoding mode

//----------------------------------------------------------------------------
// utf16_decoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf16_decoder( void ) = default; // Default constructor
   utf16_decoder(const utf16_decoder&) noexcept; // Copy constructor
   utf16_decoder(const utf16_encoder&) noexcept; // Copy constructor

   utf16_decoder(const utf16_t*, Length, MODE mode= MODE_RESET) noexcept;
   utf16_decoder(const utf16_t*, MODE mode= MODE_RESET) noexcept;

   ~utf16_decoder( void ) = default; // Destructor (does nothing)

//----------------------------------------------------------------------------
// utf16_decoder::Assignment operators
//----------------------------------------------------------------------------
utf16_decoder&
    operator=(const utf16_decoder&) noexcept; // Copy utf16_decoder

utf16_decoder&
    operator=(const utf16_encoder&) noexcept; // Copy utf16_encoder

//----------------------------------------------------------------------------
// utf16_decoder::Accessor methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "") const noexcept; // Debugging message

Count                               // The total column Count
   get_column_count( void ) const;  // Get total column Count

Index                               // The current column Index
   get_column_index( void ) const;  // Get current column Index

Length
   get_length( void ) const         // Get length (in native units)
{  return length; }

MODE
   get_mode( void ) const           // Get current MODE
{  return mode; }

Offset
   get_offset( void ) const         // Get current (unit) offset
{  return offset; }

Offset                              // Unit origin: 0 or 1
   get_origin( void ) const;        // Get unit origin, 0 or 1

Count                               // The total symbol Count
   get_symbol_count( void ) const;  // Get total symbol Count

bool                                // TRUE iff Symbol is a combining character
   is_combining( void ) const       // Is the current Symbol combining?
{  return Utf::is_combining(current()); }

Count                               // Number of units past end of buffer
   set_column_index(Index);         // Set the column index

void
   set_mode(MODE);                  // Set encoding mode

Count                               // Number of units past end of buffer
   set_symbol_index(Index);         // Set the Symbol index

//----------------------------------------------------------------------------
// utf16_decoder::Methods
//----------------------------------------------------------------------------
utf16_decoder                       // The current column substring
   copy_column( void ) const;       // Get current column substring

Symbol                              // The current Symbol
   current( void ) const;           // Retrieve the current Symbol

Symbol                              // The current Symbol
   decode( void );                  // Decode, updating column and offset

void                                // Reset the decoder
   reset(const utf16_t*, Length, MODE= MODE_RESET) noexcept;

void
   reset( void ) noexcept;          // Reset the decoder to it's initial state
}; // struct utf16_decoder

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
friend utf32_encoder;               // For operator=

//----------------------------------------------------------------------------
// utf32_decoder::Attributes
//----------------------------------------------------------------------------
protected:
const utf32_t*         buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in native units
Index                  column= -1;  // Current buffer column index
Offset                 offset= 0;   // Current buffer native unit index
MODE                   mode= MODE_RESET; // Default, big endian encoding mode

//----------------------------------------------------------------------------
// utf32_decoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf32_decoder( void ) = default; // Default constructor
   utf32_decoder(const utf32_decoder&) noexcept; // Copy constructor
   utf32_decoder(const utf32_encoder&) noexcept; // Copy constructor

   utf32_decoder(const utf32_t*, Length, MODE mode= MODE_RESET) noexcept;
   utf32_decoder(const utf32_t*, MODE mode= MODE_RESET) noexcept;

   ~utf32_decoder( void ) = default; // Destructor (does nothing)

//----------------------------------------------------------------------------
// utf32_decoder::Assignment operators
//----------------------------------------------------------------------------
utf32_decoder&
    operator=(const utf32_decoder&) noexcept; // Copy utf32_decoder

utf32_decoder&
    operator=(const utf32_encoder&) noexcept; // Copy utf32_encoder

//----------------------------------------------------------------------------
// utf32_decoder::Accessor methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "") const noexcept; // Debugging message

Count                               // The total column Count
   get_column_count( void ) const;  // Get total column Count

Index                               // The current column index
   get_column_index( void ) const;  // Get current column index

Length
   get_length( void ) const         // Get length (in native units)
{  return length; }

MODE
   get_mode( void ) const           // Get current MODE
{  return mode; }

Offset
   get_offset( void ) const         // Get current (unit) offset
{  return offset; }

Offset                              // Unit origin: 0 or 1
   get_origin( void ) const;        // Get unit origin, 0 or 1

Count                               // The total symbol Count
   get_symbol_count( void ) const;  // Get total symbol Count

bool                                // TRUE iff Symbol is a combining character
   is_combining( void ) const       // Is the current Symbol combining?
{  return Utf::is_combining(current()); }

Count                               // Number of units past end of buffer
   set_column_index(Index);         // Set the column index

void
   set_mode(MODE);                  // Set encoding mode

Count                               // Number of units past end of buffer
   set_symbol_index(Index);         // Set the Symbol index

//----------------------------------------------------------------------------
// utf32_decoder::Methods
//----------------------------------------------------------------------------
utf32_decoder                       // The current column substring
   copy_column( void ) const;       // Get current column substring

Symbol                              // The current Symbol
   current( void ) const;           // Retrieve the current Symbol

Symbol                              // The current Symbol
   decode( void );                  // Decode, updating column and offset

void                                // Reset the decoder
   reset(const utf32_t*, Length, MODE= MODE_RESET) noexcept;

void
   reset( void ) noexcept;          // Reset the decoder to it's initial state
}; // struct utf32_decoder

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
friend utf8_decoder;                // For copy constructor, operator=

//----------------------------------------------------------------------------
// utf8_encoder::Attributes
//----------------------------------------------------------------------------
protected:
utf8_t*                buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in native units (bytes)
Index                  column= -1;  // Current buffer column index
Offset                 offset= 0;   // Current buffer unit index

//----------------------------------------------------------------------------
// utf8_encoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf8_encoder( void ) = default;  // Default constructor
   utf8_encoder(const utf8_encoder&) = delete; // NO copy constructor

   utf8_encoder(utf8_t*, Length) noexcept; // Address/Length constructor

   ~utf8_encoder( void ) = default; // Destructor does nothing

// Alias constructor: static cast addr: char* => utf8_t*
   utf8_encoder(char* addr, Length size) noexcept; // (Alias constructor)

//----------------------------------------------------------------------------
// utf8_encoder::Assignment operators
//----------------------------------------------------------------------------
utf8_encoder&
    operator=(const utf8_decoder&) noexcept(false); // Copy utf8_decoder

inline utf8_encoder&
    operator=(const utf8_encoder&) noexcept(false); // Copy utf8_encoder

utf8_encoder&
    operator=(const utf16_decoder&) noexcept(false); // Copy utf16_decoder

inline utf8_encoder&
    operator=(const utf16_encoder&) noexcept(false); // Copy utf16_encoder

utf8_encoder&
    operator=(const utf32_decoder&) noexcept(false); // Copy utf32_decoder

inline utf8_encoder&
    operator=(const utf32_encoder&) noexcept(false); // Copy utf32_encoder

//----------------------------------------------------------------------------
// utf8_encoder::Accessor methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "") const noexcept; // Debugging message

Index                               // The current column index
   get_column_index( void ) const   // Get current column index
{  return column; }

Offset
   get_offset( void ) const         // Get current offset
{  return offset; }

//----------------------------------------------------------------------------
// utf8_encoder::Methods
//----------------------------------------------------------------------------
unsigned                            // Encoding unit Length
   encode(Symbol);                  // Encode, updating column and offset

void
   reset(utf8_t*, Length) noexcept; // Reset the encoder

void
   reset(char* addr, Length size) noexcept // (Reset alias)
{  reset((utf8_t*)addr, size); }

void
   reset( void ) noexcept;          // Reset the encoder to it's initial state
}; // struct utf8_encoder

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
friend utf16_decoder;               // For copy constructor, operator=

//----------------------------------------------------------------------------
// utf16_encoder::Attributes
//----------------------------------------------------------------------------
protected:
utf16_t*               buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in native units
Index                  column= -1;  // Current buffer column index
Offset                 offset= 0;   // Current buffer native unit index
MODE                   mode= MODE_RESET; // Default, big endian encoding mode

//----------------------------------------------------------------------------
// utf16_encoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf16_encoder( void ) = default; // Default constructor
   utf16_encoder(const utf16_encoder&) = delete; // NO copy constructor
   utf16_encoder(utf16_t*, Length, MODE mode= MODE_RESET) noexcept;

   ~utf16_encoder( void ) = default; // Destructor (does nothing)

//----------------------------------------------------------------------------
// utf16_encoder::Assignment operators
//----------------------------------------------------------------------------
utf16_encoder&
    operator=(const utf8_decoder&) noexcept(false); // Copy utf8_decoder

inline utf16_encoder&
    operator=(const utf8_encoder&) noexcept(false); // Copy utf8_encoder

utf16_encoder&
    operator=(const utf16_decoder&) noexcept(false); // Copy utf16_decoder

inline utf16_encoder&
    operator=(const utf16_encoder&) noexcept(false); // Copy utf16_encoder

utf16_encoder&
    operator=(const utf32_decoder&) noexcept(false); // Copy utf32_decoder

inline utf16_encoder&
    operator=(const utf32_encoder&) noexcept(false); // Copy utf32_encoder

//----------------------------------------------------------------------------
// utf16_encoder::Accessor methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "") const noexcept; // Debugging message

Index                               // The current column index
   get_column_index( void ) const   // Get current column index
{  return column; }

MODE
   get_mode( void ) const           // Get current MODE
{  return mode; }

Offset
   get_offset( void ) const         // Get current (byte) offset
{  return offset; }

void
   set_mode(MODE);                  // Set encoding mode

//----------------------------------------------------------------------------
// utf16_encoder::Methods
//----------------------------------------------------------------------------
unsigned                            // The encoding length, in units
   encode(Symbol);                  // Encode, updating column and offset

void                                // Reset the encoder
   reset(utf16_t*, Length, MODE mode= MODE_RESET) noexcept;

void
   reset( void ) noexcept;          // Reset the encoder to it's initial state
}; // struct utf16_encoder

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
friend utf32_decoder;               // For copy constructor, operator=

//----------------------------------------------------------------------------
// utf32_encoder::Attributes
//----------------------------------------------------------------------------
protected:
utf32_t*               buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in native units
Index                  column= -1;  // Current buffer column index
Offset                 offset= 0;   // Current buffer native unit index
MODE                   mode= MODE_RESET; // Default, big endian encoding mode

//----------------------------------------------------------------------------
// utf32_encoder::Constructors/destructor
//----------------------------------------------------------------------------
public:
   utf32_encoder( void ) = default; // Default constructor
   utf32_encoder(const utf32_encoder&) = delete; // NO copy constructor

   utf32_encoder(utf32_t*, Length, MODE mode= MODE_RESET) noexcept;

   ~utf32_encoder( void ) = default; // Destructor (does nothing)

//----------------------------------------------------------------------------
// utf32_encoder::Assignment operators
//----------------------------------------------------------------------------
utf32_encoder&
    operator=(const utf8_decoder&) noexcept(false); // Copy utf8_decoder

inline utf32_encoder&
    operator=(const utf8_encoder&) noexcept(false); // Copy utf8_encoder

utf32_encoder&
    operator=(const utf16_decoder&) noexcept(false); // Copy utf16_decoder

inline utf32_encoder&
    operator=(const utf16_encoder&) noexcept(false); // Copy utf16_encoder

utf32_encoder&
    operator=(const utf32_decoder&) noexcept(false); // Copy utf32_decoder

inline utf32_encoder&
    operator=(const utf32_encoder&) noexcept(false); // Copy utf32_encoder

//----------------------------------------------------------------------------
// utf32_encoder::Accessor methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "") const noexcept; // Debugging message

Index                               // The current column index
   get_column_index( void ) const   // Get current column index
{  return column; }

Offset
   get_offset( void ) const         // Get current (byte) offset
{  return offset; }

void
   set_mode(MODE);                  // Set encoding mode

//----------------------------------------------------------------------------
// utf32_encoder::Methods
//----------------------------------------------------------------------------
unsigned                            // The encoding length, in units
   encode(Symbol);                  // Encode, updating column and offset

void                                // Reset the encoder
   reset(utf32_t*, Length, MODE mode= MODE_RESET) noexcept;

void
   reset( void ) noexcept;          // Reset the encoder to it's initial state
}; // struct utf32_encoder
_LIBPUB_END_NAMESPACE

#include "bits/Utf.i"               // Inline implementations
#endif // _LIBPUB_UTF_H_INCLUDED
