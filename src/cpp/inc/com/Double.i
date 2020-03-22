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
//       Double.i
//
// Purpose-
//       Double inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef DOUBLE_I_INCLUDED
#define DOUBLE_I_INCLUDED

//----------------------------------------------------------------------------
//
// Subroutine-
//      Double::~Double
//
// Function-
//      Destructor
//
//----------------------------------------------------------------------------
   Double::~Double( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      Double::Double( void )
//
// Function-
//      Default constructor
//
//----------------------------------------------------------------------------
   Double::Double( void )           // Default constructor
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      Double::Double(double)
//
// Function-
//      Value constructor
//
//----------------------------------------------------------------------------
   Double::Double(                  // Constructor
     double            source)      // Source value
{
   value= source;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      Double::operator=
//
// Function-
//      Assignment operator
//
//----------------------------------------------------------------------------
Double&                             // Return (reference)
   Double::operator=(               // Assignment operator
     double            source)      // Source value
{
   value= source;
   return *this;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      Double::operator double
//
// Function-
//      (double) cast operator
//
//----------------------------------------------------------------------------
   Double::operator double( void ) const // (double) cast operator
{
   return value;
}

#endif // DOUBLE_I_INCLUDED
