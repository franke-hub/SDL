//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       diag-pristine.cpp
//
// Purpose-
//       Implement diag-pristine.h.
//
// Last change date-
//       2023/12/05
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace pub::debugging methods
#include "pub/diag-pristine.h"      // For pub::Pristine, implemented
#include <pub/utility.h>            // For pub:utility::dump

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using utility::dump;

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
int                    Pristine::opt_hcdm= false; // Hard Core Debug Mode?

//----------------------------------------------------------------------------
//
// Method-
//       Pristine::Pristine
//       Pristine::~Pristine
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Pristine::Pristine(Word word)    // Checkword constructor
{  if( HCDM && VERBOSE ) debugf("Pristine(%p)!\n", this);

   for(int i= 0; i<DIM; ++i)        // Initialize
     array[i]= word;
}

   Pristine::~Pristine( void )      // Destructor
{  if( HCDM && VERBOSE ) debugf("Pristine(%p)~\n", this);

   check("Destructor");
}

//----------------------------------------------------------------------------
//
// Method-
//       Pristine::check
//
// Purpose-
//       Check data array consistency, report error if invalid
//
//----------------------------------------------------------------------------
int
   Pristine::check(                 // Debugging check
     const char*       info) const  // Caller information
{
   Word check_word= array[MID];     // Any array word would work as well
   for(int i= 0; i<DIM; ++i) {      // Verify
     if( array[i] != check_word ) {
       // Add a breakpoint here if more information is desired
       errorf("\n\n>>>>>>>>>>>> Pristine(%p)::fault(%s) [%3d] <<<<<<<<<<<<\n"
             , this, info, i);
       if( opt_hcdm )
         dump(array, sizeof(array));
       errorf("\n");
       return 1;
     }
   }

   return 0;
}
}  // namespace _LIBPUB_NAMESPACE
