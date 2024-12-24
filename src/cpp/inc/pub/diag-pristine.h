//----------------------------------------------------------------------------
//
//       Copyright (c) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       diag-pristine.h
//
// Purpose-
//       Debugging diagnostic: Catch "wild stores" clobbering objects.
//
// Last change date-
//       2024/11/21
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DIAG_PRISTINE_H_INCLUDED
#define _LIBPUB_DIAG_PRISTINE_H_INCLUDED

#include <cstdint>                  // For uint64_t

#include "pub/bits/pubconfig.h"     // For _LIBPUB macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace diag {
//----------------------------------------------------------------------------
//
// Class-
//       diag::Pristine
//
// Purpose-
//       Check for wild stores.
//
// Usage notes-
//       For an object declared as `X object` that you suspect is getting
//       clobbered by wild stores, use:
//         Pristine before;
//         X object;
//         Pristine after;
//
//       REMOVE Pristine declarations in production code.
//       The Pristine destructor invokes check("Destructor"). You can also
//       invoke the check method at any time.
//
//----------------------------------------------------------------------------
class Pristine {                    // Pristine base class
//----------------------------------------------------------------------------
// diag::Pristine::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef uint64_t       Word;        // Our checkword type

enum                                // (For 4K range
{  DIM= 512                         // Number of uint64_t's in page
,  MID= 257                         // Midpoint prime
};

//----------------------------------------------------------------------------
// diag::Pristine::Attributes
//----------------------------------------------------------------------------
public:
// opt_hcdm: If true and an error occurs, the array is dumped
static int             opt_hcdm;    // Hard Core Debug Mode (default false)

protected:
Word                   array[DIM]= {}; // The check array

//----------------------------------------------------------------------------
// diag::Pristine::Constructors, destructor
//----------------------------------------------------------------------------
public:
   Pristine(uint64_t word= uint64_t(0x7654321089ABCDEF)); // Default constructor

   ~Pristine( void );               // Destructor [invokes check("Destructor")]

//----------------------------------------------------------------------------
// diag::Pristine::check, check for wild store
//----------------------------------------------------------------------------
int                                 // Return code, 0 if no error found
   check(                           // Debugging check
     const char*       info="") const; // Caller information
}; // class Pristine
}  // namespace diag
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DIAG_PRISTINE_H_INCLUDED
