//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Nice.i
//
// Purpose-
//       Implements Nice.h.
//
// Last change date-
//       2007/01/01
//
// Usage-
//       Included once in a package.
//
//----------------------------------------------------------------------------
#ifndef NICE_I_INCLUDED
#define NICE_I_INCLUDED

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
int                    Nice::generator= 0; // The serial number generator

//----------------------------------------------------------------------------
//
// Method-
//       Nice::Nice
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Nice::Nice( void )               // Default constructor
:  serialNum(++generator)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Nice::Nice(const Nice&)
//
// Purpose-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   Nice::Nice(                      // Copy constructor
     const Nice&       source)      // Source Nice&
{
   serialNum= source.serialNum;
}

//----------------------------------------------------------------------------
//
// Method-
//       Nice::operator=(const Nice&)
//
// Purpose-
//       Assignment operator.
//
//----------------------------------------------------------------------------
Nice&                               // Resultant
   Nice::operator=(                 // Assignment operator
     const Nice&       source)      // Source Nice&
{
   serialNum= source.serialNum;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Nice::operator<(const Nice&) const
//
// Purpose-
//       Less-than comparison operator.
//
//----------------------------------------------------------------------------
int                                 // Resultant (TRUE || FALSE)
   Nice::operator<(                 // Less than operator
     const Nice&       source) const// Source Nice&
{
   return (serialNum < source.serialNum);
}

//----------------------------------------------------------------------------
//
// Method-
//       Nice::operator==(const Nice&) const
//
// Purpose-
//       Equality comparison
//
//----------------------------------------------------------------------------
int                                 // Resultant (TRUE || FALSE)
   Nice::operator==(                // Equality operator
     const Nice&       source) const // Source Nice&
{
   return (serialNum == source.serialNum);
}

//----------------------------------------------------------------------------
//
// Method-
//       Nice::operator!=(const Nice&) const
//
// Purpose-
//       Inequality comparison
//
//----------------------------------------------------------------------------
int                                 // Resultant (TRUE || FALSE)
   Nice::operator!=(                // Inequality operator
     const Nice&       source) const // Source Nice&
{
   return (serialNum != source.serialNum);
}

//----------------------------------------------------------------------------
//
// Method-
//       Nice::s( void ) const
//
// Purpose-
//       An equality preserving function.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Nice::s( void ) const            // An equality preserving function
{
   return serialNum;
}

//----------------------------------------------------------------------------
//
// Method-
//       Nice::i( void )
//
// Purpose-
//       A function which does NOT preserve equality.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Nice::i( void )                  // This class is no longer extra-nice
{
   return ++serialNum;
}

#endif // NICE_I_INCLUDED
