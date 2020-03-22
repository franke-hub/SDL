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
//       Service.i
//
// Purpose-
//       Service inlines.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SERVICE_I_INCLUDED
#define SERVICE_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Service::Global::getLength
//
// Purpose-
//       Return length of area.
//
//----------------------------------------------------------------------------
unsigned                            // Length of this area
   Service::Global::getLength( void ) const // Return length of area
{
   return traceOffset + traceLength;
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::isActive
//
// Purpose-
//       Determine whether Service is active.
//
//----------------------------------------------------------------------------
int                                 // TRUE if active
   Service::isActive( void )        // Is Service active?
{
   if( global != NULL
       && global->vword == Global::VALIDATOR
       && global->latch == 0 )
     return 1;

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::getLength
//
// Purpose-
//       Return length of Global area.
//
//----------------------------------------------------------------------------
unsigned                            // Length of Global area
   Service::getLength( void )       // Return length of area
{
   if( isActive() )
     return global->getLength();

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::word
//
// Purpose-
//       Convert string to word.
//
//----------------------------------------------------------------------------
uint32_t                            // Resultant word
   Service::word(                   // Convert string to word
     const char*       string)      // String (length >=4)
{
   return (*(uint32_t*)string);
}

#endif // SERVICE_I_INCLUDED
