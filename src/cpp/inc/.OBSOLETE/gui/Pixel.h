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
//       Pixel.h
//
// Purpose-
//       Graphical User Interface: Picture element
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_PIXEL_H_INCLUDED
#define GUI_PIXEL_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"                  // This include is guaranteed
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
//
// Struct-
//       Pixel
//
// Purpose-
//       Describe a picture element.
//
// Usage-
//       A Pixel is used where the address of a Color is required
//       or when a Color needs to be manipulated.
//
//----------------------------------------------------------------------------
struct Pixel {                      // Picture element
//----------------------------------------------------------------------------
// Pixel::Attributes
//----------------------------------------------------------------------------
Color_t                color;       // Associated Color

//----------------------------------------------------------------------------
// Pixel::Constructors
//----------------------------------------------------------------------------
inline
   ~Pixel( void );                  // Destructor
inline
   Pixel( void );                   // Default constructor (BLACK)

inline
   Pixel(                           // Constructor
     Color_t           c);          // Color descriptor

inline
   Pixel(                           // Constructor
     int               r,           // Red component
     int               g,           // Green component
     int               b);          // Blue component

//----------------------------------------------------------------------------
// Pixel::Object methods
//----------------------------------------------------------------------------
inline Color_t                      // Resultant
   getColor( void ) const;          // Get Color

inline int                          // Resultant
   getRed( void ) const;            // Get red component

inline int                          // Resultant
   getGreen( void ) const;          // Get green component

inline int                          // Resultant
   getBlue( void ) const;           // Get blue component

inline void
   setColor(                        // Set value
     Color_t           color);      // From Color

inline void
   setColor(                        // Set value
     int               r,           // Red component
     int               g,           // Green component
     int               b);          // Blue component

//----------------------------------------------------------------------------
// Pixel::Static methods
//----------------------------------------------------------------------------
static inline Color_t               // Resultant
   getColor(                        // Convert to Color
     int               r,           // Red component
     int               g,           // Green component
     int               b);          // Blue component

static inline int                   // Resultant
   getRed(                          // Get red component
     Color_t           color);      // From Color

static inline int                   // Resultant
   getGreen(                        // Get green component
     Color_t           color);      // From Color

static inline int                   // Resultant
   getBlue(                         // Get blue component
     Color_t           color);      // From Color
}; // struct Pixel
#include "Pixel.i"
#include "namespace.end"

#endif // GUI_PIXEL_H_INCLUDED
