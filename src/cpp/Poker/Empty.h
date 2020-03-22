//----------------------------------------------------------------------------
//
//       Copyright (C) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Empty.h
//
// Purpose-
//       Empty description.
//
// Last change date-
//       2017/01/01
//
// Usage-
//       c /Empty/ClassName/ ; skip header; Remove this line
//       c /Empty/CLASSNAME/ ; Remove this line ; continue
//
//----------------------------------------------------------------------------
#ifndef EMPTY_H_INCLUDED
#define EMPTY_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Empty
//
// Purpose-
//       Empty object descriptor.
//
//----------------------------------------------------------------------------
class Empty                         // Empty
{
//----------------------------------------------------------------------------
// Empty::Typedefs and enumerations
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// Empty::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Empty( void );                  // Destructor
   Empty( void );                   // Default constructor

private:                            // Bitwise copy is prohibited
   Empty(const Empty&);             // Disallowed copy constructor
   Empty& operator=(const Empty&);  // Disallowed assignment operator

//----------------------------------------------------------------------------
// Empty::Accessor methods
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// Empty::Methods
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// Empty::Protected methods
//----------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------
// Empty::Static attributes
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// Empty::Attributes
//----------------------------------------------------------------------------
protected:
}; // class Empty

#include "Empty.i"

#endif // EMPTY_H_INCLUDED
