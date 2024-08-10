//----------------------------------------------------------------------------
//
//       Copyright (C) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdType.h
//
// Purpose-
//       Editor: Editor common types
//
// Last change date-
//       2024/07/27
//
//----------------------------------------------------------------------------
#ifndef EDTYPE_H_INCLUDED
#define EDTYPE_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For system types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Utf.h>                // For pub::Utf types

//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef uint32_t                    GC_t;    // Graphic Context type

typedef std::string                 string;

typedef pub::Utf::Column            Column;
typedef pub::Utf::Length            Length;
typedef pub::Utf::Offset            Offset;
typedef pub::Utf::Points            Points;

typedef pub::Utf::utf8_t            utf8_t;
typedef pub::Utf::utf16_t           utf16_t;
typedef pub::Utf::utf16_t           utf16BE_t;
typedef pub::Utf::utf32_t           utf32_t;

#if 1
typedef pub::Utf::MODE              MODE;
constexpr static const MODE
                       MODE_BE= pub::Utf::MODE_BE;
#else
enum MODE                           // Import MODE
{  MODE_RESET= pub::Utf::MODE_RESET
,  MODE_BE= pub::Utf::MODE_BE
,  MODE_LE= pub::Utf::MODE_LE
};
#endif

constexpr static const uint32_t
                       UTF_EOF= pub::Utf::UTF_EOF;

typedef pub::utf8_decoder           utf8_decoder;
typedef pub::utf8_encoder           utf8_encoder;
typedef pub::utf16_encoder          utf16_encoder;

//----------------------------------------------------------------------------
//
// Struct-
//       geometry_t
//
// Purpose-
//       Geometry descriptor
//
//----------------------------------------------------------------------------
typedef struct geometry_t {         // The geometry descriptor type
   int32_t             x;           // (X position, 0 offset from left)
   int32_t             y;           // (Y position, 0 offset from top)
   uint32_t            width;       // The (X) width  (In columns or Pixels)
   uint32_t            height;      // The (Y) height (In rows or Pixels)
}  geometry_t;
#endif // EDTYPE_H_INCLUDED
