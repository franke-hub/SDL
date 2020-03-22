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
//       Ident.h
//
// Purpose-
//       Identifier.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef IDENT_H_INCLUDED
#define IDENT_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Ident
//
// Purpose-
//       Identifier.
//
//----------------------------------------------------------------------------
class Ident {                       // Identifier
//----------------------------------------------------------------------------
// Ident::Attributes
//----------------------------------------------------------------------------
private:
int                    ident;       // Identifier

//----------------------------------------------------------------------------
// Ident::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Ident( void );                  // Destructor

inline
   Ident( void );                   // Constructor

//----------------------------------------------------------------------------
// Ident::Accessor Methods
//----------------------------------------------------------------------------
inline int                          // The identifier
   getIdent( void ) const;          // Get identifier

inline void
   setIdent(                        // Set identifier
     int               ident);      // The identifier
}; // class Ident

#include "Ident.i"

#endif // IDENT_H_INCLUDED
