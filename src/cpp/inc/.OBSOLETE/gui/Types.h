//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Types.h
//
// Purpose-
//       Graphical User Interface: Standard types
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_TYPES_H_INCLUDED
#define GUI_TYPES_H_INCLUDED

#include <stdint.h>                 // This include is guaranteed
#include <string>                   // This include is guaranteed
#include <com/define.h>             // This include is guaranteed
#include <com/Logger.h>

#ifndef GUI_CONSTANT_H_INCLUDED
#include "Constant.h"
#endif

#ifndef GUI
#define GUI _GUI
////ine GUI GUI_a8y7jkxf94r6yu8m
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef uint8_t        UTF8_t;      // UTF-8 Unicode encoding word
typedef uint16_t       UTF16_t;     // UTF-16 Unicode encoding word
typedef uint32_t       Color_t;     // Pixel representation, 0x00rrggbb

typedef unsigned       Length_t;    // A length
typedef Length_t       XLength_t;   // X (horizontal) length
typedef Length_t       YLength_t;   // Y (vertical) length

typedef unsigned       Offset_t;    // An offset
typedef Offset_t       XOffset_t;   // X (horizontal) offset
typedef Offset_t       YOffset_t;   // Y (vertical) offset

//----------------------------------------------------------------------------
//
// Struct-
//       RGB
//
// Purpose-
//       Standard RGB color set.
//
//----------------------------------------------------------------------------
struct RGB {                        // Container for RGB color enum
enum                                // Red, Green, Blue color set
{  Black=              0x00000000   // Black
,  Blue=               0x000000FF   // Blue
,  Green=              0x0000FF00   // Green
,  Cyan=               0x0000FFFF   // Cyan
,  Red=                0x00FF0000   // Red
,  Magenta=            0x00FF00FF   // Magenta
,  Brown=              0x00404000   // Brown
,  LightGrey=          0x00C0C0C0   // Light Grey
,  Grey=               0x00808080   // Grey
,  DarkGrey=           0x00404040   // Dark  Grey
,  LightBlue=          0x008080FF   // Light Blue
,  LightGreen=         0x0080FF80   // Light Green
,  LightCyan=          0x00C0FFFF   // Light Cyan
,  LightRed=           0x00FF8080   // Light Red
,  Pink=               0x00FF8080   // Pink
,  LightMagenta=       0x00FFC0FF   // Light Magenta
,  Yellow=             0x00FFFF00   // Yellow
,  White=              0x00FFFFFF   // White
,  MAXRGB=             0x00FFFFFF   // Highest valid RGB color value
,  INVALID=            0xFFFFFFFF   // Invalid color value
}; // enum
}; // struct RGB

//----------------------------------------------------------------------------
//
// Struct-
//       XYLength
//
// Purpose-
//       Describe a length in terms of an X (column) and Y (row) value.
//
//----------------------------------------------------------------------------
struct XYLength {                   // Length descriptor
   XLength_t           x;           // X (column) length
   YLength_t           y;           // Y (row) length
}; // struct XYLength

//----------------------------------------------------------------------------
//
// Struct-
//       XYOffset
//
// Purpose-
//       Describe a position in terms of an X (column) and Y (row) value.
//
// Notes-
//       The (top,left) corner is offset(0,0).
//       Negative offsets are not allowed.
//
//----------------------------------------------------------------------------
struct XYOffset {                   // Position descriptor
   XOffset_t           x;           // X (column) offset
   YOffset_t           y;           // Y (row) offset
}; // struct XYOffset

//----------------------------------------------------------------------------
//
// Struct-
//       XYValues
//
// Purpose-
//       Describe signed X, Y values
//
//----------------------------------------------------------------------------
struct XYValues {                   // Value descriptor
   signed              x;           // X value
   signed              y;           // Y value
}; // struct XYValues
#include "namespace.end"

//----------------------------------------------------------------------------
// Included files
//----------------------------------------------------------------------------
#include "Attributes.h"
#include "Pixel.h"

#endif // GUI_TYPES_H_INCLUDED
