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
//       Utf_type.i
//
// Purpose-
//       Allow application import of UTF types
//
// Last change date-
//       2024/08/14
//
// Implementation notes-
//       To avoid namespace collisions, include this from .cpp files.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_UTF_TYPE_I_INCLUDED
#define _LIBPUB_BITS_UTF_TYPE_I_INCLUDED

//----------------------------------------------------------------------------
// Exported Utf types
typedef pub::Utf::Column            Column;
typedef pub::Utf::Cpoint            Cpoint;
typedef pub::Utf::Points            Points;
typedef pub::Utf::Length            Length;
typedef pub::Utf::Lpoint            Lpoint;
typedef pub::Utf::Offset            Offset;

typedef pub::Utf::utf8_t            utf8_t;
typedef pub::Utf::utf16_t           utf16_t;
typedef pub::Utf::utf32_t           utf32_t;

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
constexpr static const utf32_t      // (UTF_EOF is an invalid UTF codepoint)
                       UTF_EOF= EOF; // Decode: No characters remain

//----------------------------------------------------------------------------
// Exported Utf subroutines (renaming pub::Utf::strlen)
static inline bool                  // TRUE iff codepoint is combining
   is_combining(                    // Is code a combining character?
     utf32_t           code)        // (The codepoint)
{  return pub::Utf::is_combining(code); }

static inline bool                  // TRUE iff codepoint is valid unicode
   is_unicode(                      // Is codepoint valid unicode?
     utf32_t           code)        // (The codepoint)
{  return pub::Utf::is_unicode(code); }

static inline Length                // Length (in native units)
   utf_strlen(                      // Get length (in bytes)
     const utf8_t*     addr)        // Of this U8-string
{  return ::strlen((const char*) addr); }

static inline Length                // Length (in native units)
   utf_strlen(                      // Get length (in bytes)
     const utf16_t*    addr)        // Of this U16-string
{  return pub::Utf::strlen(addr); }

static inline Length                // Length (in native units)
   utf_strlen(                      // Get length (in bytes)
     const utf32_t*    addr)        // Of this U32-string
{  return pub::Utf::strlen(addr); }
#endif // _LIBPUB_BITS_UTF_TYPE_I_INCLUDED
