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
//       Callback.h
//
// Purpose-
//       Abstract Base Class for Callback object.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       A Callback object is typically used to return multiple items
//       to a function caller.
//
//       The parameters to to the Callback routine are specified by the
//       specific Callback class.
//
//----------------------------------------------------------------------------
#ifndef CALLBACK_H_INCLUDED
#define CALLBACK_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Callback
//
// Purpose-
//       The Callback object.
//
//----------------------------------------------------------------------------
class Callback {                    // Callback descriptor
//----------------------------------------------------------------------------
// Callback::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Callback( void ) {}             // Destructor
   Callback( void ) {}              // Constructor

//----------------------------------------------------------------------------
// Callback::Attributes
//----------------------------------------------------------------------------
private:
   // None defined
}; // class Callback

#endif // CALLBACK_H_INCLUDED
