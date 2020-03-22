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
//       SharedMem.cpp
//
// Purpose-
//       SharedMem object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#if   defined(_OS_WIN) || defined(_OS_CYGWIN) // shmget not supported!
#include "OS/WIN/SharedMem.cpp"

#elif defined(_OS_BSD)
#include "OS/BSD/SharedMem.cpp"

#else
#error "Invalid OS"
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::~SharedMem
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   SharedMem::~SharedMem( void )    // Destructor
{
   #ifdef HCDM
     debugf("%8s= SharedMem(%p)::~SharedMem()\n", "", this);
   #endif

   if( address != NULL )
   {
     detach(address);
     address= NULL;
   }

   if( segment != InvalidSegment && (control&Keep) == 0 )
   {
     remove(segment);
     segment= InvalidSegment;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::SharedMem
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   SharedMem::SharedMem(            // Constructor
     Size_t            length,      // Length of storage region
     Token             token,       // Associated token
     int               control)     // Control flags
:  address(NULL)
,  segment(InvalidSegment)
,  length(length)
,  token(token)
,  control(control)
{
   #ifdef HCDM
     debugf("%8s= SharedMem(%p)::SharedMem(%lu, %.8lx, %.8lx)\n", "", this,
            (long)length, (long)token, (long)control);
   #endif

   segment= access(length, token, control); // Access the segment
   if( segment == InvalidSegment )
     throw "ConstructException";

   address= attach(segment);        // Attach the segment
   if( address == NULL )
   {
     remove(segment);
     segment= InvalidSegment;
     throw "ConstructException";
   }
}

