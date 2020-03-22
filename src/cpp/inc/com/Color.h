//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Color.h
//
// Purpose-
//       Define the standard colors.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef COLOR_H_INCLUDED
#define COLOR_H_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       RGBColor
//
// Purpose-
//       Container for the RGB enumeration.
//
//----------------------------------------------------------------------------
struct RGBColor {                   // RGB Color enumeration
//----------------------------------------------------------------------------
// RGBColor::Enumerations and Typedefs
//----------------------------------------------------------------------------
enum                                // Red, Green, Blue array standard colors
{
   Black=               0x00000000, // Black
   Blue=                0x000000FF, // Blue
   Green=               0x0000FF00, // Green
   Cyan=                0x0000FFFF, // Cyan
   Red=                 0x00FF0000, // Red
   Magenta=             0x00FF00FF, // Magenta
   Brown=               0x00404000, // Brown
   LightGrey=           0x00C0C0C0, // Light Grey
   DarkGrey=            0x00A0A0A0, // Dark  Grey
   LightBlue=           0x008080FF, // Light Blue
   LightGreen=          0x0080FF80, // Light Green
   LightCyan=           0x00C0FFFF, // Light Cyan
   LightRed=            0x00FF8080, // Light Red
   Pink=                0x00FF8080, // Pink
   LightMagenta=        0x00FFC0FF, // Light Magenta
   Yellow=              0x00FFFF00, // Yellow
   White=               0x00FFFFFF, // White
   MAXRGB=              0x00FFFFFF  // Highest valid VGA color value
}; // enum
}; // struct RGBColor

//----------------------------------------------------------------------------
//
// Struct-
//       VGAColor
//
// Purpose-
//       Container for the VGA enumeration.
//
//----------------------------------------------------------------------------
struct VGAColor {                   // VGA Color enumeration
//----------------------------------------------------------------------------
// VGAColor::Enumerations and Typedefs
//----------------------------------------------------------------------------
enum                                // Video Graphics Array standard colors
{
   Black=                   0x0000, // Black
   Blue=                    0x0001, // Blue
   Green=                   0x0002, // Green
   Cyan=                    0x0003, // Cyan
   Red=                     0x0004, // Red
   Magenta=                 0x0005, // Magenta
   Brown=                   0x0006, // Brown
   Grey=                    0x0007, // Grey
   DarkGrey=                0x0008, // Dark Grey
   LightBlue=               0x0009, // Light Blue
   LightGreen=              0x000A, // Light Green
   LightCyan=               0x000B, // Light Cyan
   LightRed=                0x000C, // Light Red
   Pink=                    0x000C, // Pink
   LightMagenta=            0x000D, // Light Magenta
   Yellow=                  0x000E, // Yellow
   White=                   0x000F, // White
   MAXVGA=                  0x000F  // Highest valid VGA color value
}; // enum VGA
}; // struct VGAColor

//----------------------------------------------------------------------------
//
// Struct-
//       Color
//
// Purpose-
//       Container for Color types.
//
//----------------------------------------------------------------------------
struct Color {                      // Color types
typedef int            RGB;         // Color::RGB type
typedef int            VGA;         // Color::VGA type

//----------------------------------------------------------------------------
//
// Struct-
//       Color::Char
//
// Purpose-
//       Define a character with a color attribute.
//
//----------------------------------------------------------------------------
struct Char                         // Color::Char type
{
inline static int                   // The resultant attribute character
   retAttribute(                    // Return the attribute character
     VGA               fg,          // For this foreground color
     VGA               bg);         // and this background color

inline void
   setAttribute(                    // Set the attribute character
     VGA               fg,          // Set this foreground color
     VGA               bg);         // Set this background color

   short               data;        // Data
   short               attr;        // Attribute
}; // struct Color::Char
}; // struct Color

#include "Color.i"

#endif // COLOR_H_INCLUDED
