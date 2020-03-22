//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Interface.h
//
// Purpose-
//       Define the Interface base class.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Interface
//
// Purpose-
//       Define the Interface base class.
//
// Implementation notes-
//       The intent is to define a set of classes which, except for their
//       constructors, contain only virtual or inline methods.
//
//----------------------------------------------------------------------------
class Interface {                   // Interface
//----------------------------------------------------------------------------
// Interface::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Interface( void );              // Destructor
}; // class Interface

#endif // INTERFACE_H_INCLUDED
