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
//       Color.i
//
// Purpose-
//       Color inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef COLOR_I_INCLUDED
#define COLOR_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Color::Char::retAttribute
//
// Purpose-
//       Set the default attributes.
//
//----------------------------------------------------------------------------
int                                 // The resultant attribute character
   Color::Char::retAttribute(       // Return the attribute character
     VGA               fg,          // For this foreground color
     VGA               bg)          // and this background color
{
   return (bg*16) | fg;
}

//----------------------------------------------------------------------------
//
// Method-
//       Color::Char::setAttribute
//
// Purpose-
//       Set the default attributes.
//
//----------------------------------------------------------------------------
void
   Color::Char::setAttribute(       // Set the attribute character
     VGA               fg,          // Set this foreground color
     VGA               bg)          // Set this background color
{
   attr= (bg*16) | fg;
}

#endif // COLOR_I_INCLUDED
