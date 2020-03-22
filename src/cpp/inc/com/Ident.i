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
//       Ident.i
//
// Purpose-
//       Ident inline functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef IDENT_I_INCLUDED
#define IDENT_I_INCLUDED

//----------------------------------------------------------------------------
//
// Subroutine-
//       Ident::Ident
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Ident::Ident( void )             // Constructor
:  ident(0)
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Ident::getIdent
//
// Purpose-
//       Extract the indentifier.
//
//----------------------------------------------------------------------------
int                                 // The identifier
   Ident::getIdent( void ) const    // Get identifier
{
   return ident;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Ident::setIdent
//
// Purpose-
//       Set the Ident identifier.
//
//----------------------------------------------------------------------------
void
   Ident::setIdent(                 // Set identifier
     int             ident)         // The identifier
{
   this->ident= ident;              // Set the identifier
}

#endif // IDENT_I_INCLUDED
