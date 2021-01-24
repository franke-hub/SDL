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
//       Pixel.i
//
// Purpose-
//       Graphical User Interface: Pixel: Inline methods
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_PIXEL_I_INCLUDED
#define GUI_PIXEL_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Pixel::~Pixel
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Pixel::~Pixel( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixel::Pixel
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   Pixel::Pixel( void )             // Default constructor
:  color(0)
{
}

   Pixel::Pixel(                    // Constructor
     Color_t           color)       // Color descriptor
:  color(color)
{
}

   Pixel::Pixel(                    // Constructor
     int               r,           // Red component
     int               g,           // Green component
     int               b)           // Blue component
:  color(getColor(r,g,b))
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixel::getColor
//       Pixel::getRed
//       Pixel::getGreen
//       Pixel::getBlue
//
// Purpose-
//       Static Color manipulators.
//
//----------------------------------------------------------------------------
Color_t                             // Resultant
   Pixel::getColor(                 // Create Color from RGB components
     int               r,           // Red component
     int               g,           // Green component
     int               b)           // Blue component
{
   r &= 0x00ff;                     // Prevent overflow
   g &= 0x00ff;
   b &= 0x00ff;

   return Color_t((r << 16) | (g <<8) | b);
}

int                                 // Resultant
   Pixel::getRed(                   // Get RED Color component
     Color_t           color)       // From Color
{
   return (color >> 16) & 0x00ff;
}

int                                 // Resultant
   Pixel::getGreen(                 // Get GREEN Color component
     Color_t           color)       // From Color
{
   return (color >> 8) & 0x00ff;
}

int                                 // Resultant
   Pixel::getBlue(                  // Get BLUE Color component
     Color_t           color)       // From Color
{
   return (color     ) & 0x00ff;
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixel::getColor
//       Pixel::getRed
//       Pixel::getGreen
//       Pixel::getBlue
//       Pixel::setColor
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
Color_t                             // Resultant
   Pixel::getColor( void ) const    // Get Color
{
   return color;
}

int                                 // Resultant
   Pixel::getRed( void ) const      // Get RED color component
{
   return Pixel::getRed(color);
}

int                                 // Resultant
   Pixel::getGreen( void ) const    // Get GREEN color component
{
   return Pixel::getGreen(color);
}

int                                 // Resultant
   Pixel::getBlue( void ) const     // Get BLUE color component
{
   return Pixel::getBlue(color);
}

void
   Pixel::setColor(                 // Set Color
     Color_t           color)       // From Color
{
   this->color= color;
}

void
   Pixel::setColor(                 // Set Color
     int               r,           // Red component
     int               g,           // Green component
     int               b)           // Blue component
{
   this->color= getColor(r, g, b);
}

#endif // GUI_PIXEL_I_INCLUDED
