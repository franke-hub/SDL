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
//       Utf.i
//
// Purpose-
//       Import UTF types into default namespace.
//
// Last change date-
//       2024/09/12
//
// Implementation notes-
//       Include this from a .cpp file to import "Utf.h" types.
//       (Don't include it from a .h file)
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_UTF_I_INCLUDED
#define _LIBPUB_UTF_I_INCLUDED

#include "Utf.h"                    // The import source

//----------------------------------------------------------------------------
// Exported Utf types
typedef pub::Utf::utf8_t            utf8_t;
typedef pub::Utf::utf16_t           utf16_t;
typedef pub::Utf::utf32_t           utf32_t;

typedef pub::Utf::Count             Count;
typedef pub::Utf::Index             Index;
typedef pub::Utf::Length            Length;
typedef pub::Utf::Offset            Offset;
typedef pub::Utf::Symbol            Symbol;

typedef pub::utf8_decoder           utf8_decoder;
typedef pub::utf16_decoder          utf16_decoder;
typedef pub::utf32_decoder          utf32_decoder;
typedef pub::utf8_encoder           utf8_encoder;
typedef pub::utf16_encoder          utf16_encoder;
typedef pub::utf32_encoder          utf32_encoder;

//----------------------------------------------------------------------------
// Exported Utf enumerations
enum                             // pub::Utf Unicode characters
{  BYTE_ORDER_MARK=                 pub::Utf::BYTE_ORDER_MARK
,  MARK_ORDER_BYTE=                 pub::Utf::MARK_ORDER_BYTE

,  BYTE_ORDER_MARK32=               pub::Utf::BYTE_ORDER_MARK32
,  MARK_ORDER_BYTE32=               pub::Utf::MARK_ORDER_BYTE32

,  UNI_REPLACEMENT=                 pub::Utf::UNI_REPLACEMENT
}; // pub::Utf Unicode characters

// MODE enumeration
typedef pub::Utf::MODE              MODE;
constexpr static const MODE
                       MODE_RESET= pub::Utf::MODE_RESET;
constexpr static const MODE
                       MODE_BE= pub::Utf::MODE_BE;
constexpr static const MODE
                       MODE_LE= pub::Utf::MODE_LE;

// Method decode: No characters remain
constexpr static const Symbol       // (UTF_EOF is an invalid UTF code point)
                       UTF_EOF= EOF; // Decode: No characters remain

//----------------------------------------------------------------------------
// Exported Utf subroutines
static inline bool                  // TRUE iff Symbol is combining character
   is_combining(                    // Is Symbol a combining character?
     Symbol            code)        // (The Symbol)
{  return pub::Utf::is_combining(code); }

static inline bool                  // TRUE iff Symbol is within unicode range
   is_unicode(                      // Is Symbol within unicode range?
     Symbol            code)        // (The Symbol)
{  return pub::Utf::is_unicode(code); }

static inline Length                // Length (in native units)
   utflen(                          // Get length (in native units)
     const utf8_t*     addr)        // Of this U8-string
{  return ::strlen((const char*)addr); }

static inline Length                // Length (in native units)
   utflen(                          // Get length (in native units)
     const utf16_t*    addr)        // Of this U16-string
{  return pub::Utf::utflen(addr); }

static inline Length                // Length (in native units)
   utflen(                          // Get length (in native units)
     const utf32_t*    addr)        // Of this U32-string
{  return pub::Utf::utflen(addr); }
#endif // _LIBPUB_UTF_I_INCLUDED
