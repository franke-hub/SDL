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
//       SharedMem.i
//
// Purpose-
//       SharedMem inlines.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SHAREDMEM_I_INCLUDED
#define SHAREDMEM_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       SharedMem::getAddress
//
// Purpose-
//       Extract the associated Address.
//
//----------------------------------------------------------------------------
SharedMem::Address*                 // Associated Address
   SharedMem::getAddress( void ) const // Get associated Address
{
   return address;
}

//----------------------------------------------------------------------------
//
// Method-
//       SharedMem::getSegment
//
// Purpose-
//       Extract the associated Segment.
//
//----------------------------------------------------------------------------
SharedMem::Segment                  // The Segment
   SharedMem::getSegment( void ) const // Extract associated Segment
{
   return segment;
}

//----------------------------------------------------------------------------
//
// Method-
//       SharedMem::getLength
//
// Purpose-
//       Extract the associated Length.
//
//----------------------------------------------------------------------------
SharedMem::Size_t                   // The Length
   SharedMem::getLength( void ) const // Extract associated Length
{
   return length;
}

//----------------------------------------------------------------------------
//
// Method-
//       SharedMem::getToken( void )
//
// Purpose-
//       Extract the associated Token.
//
//----------------------------------------------------------------------------
SharedMem::Token                    // The Token
   SharedMem::getToken( void ) const // Extract associated Token
{
   return token;
}

#endif // SHAREDMEM_I_INCLUDED
