//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       diag-stack.h
//
// Purpose-
//       Debugging diagnostic: Test for stack errors.
//
// Last change date-
//       2024/11/22
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DIAG_STACK_H_INCLUDED
#define _LIBPUB_DIAG_STACK_H_INCLUDED

#include <cstdlib>                  // For size_t

#include "pub/bits/pubconfig.h"     // For _LIBPUB macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace diag {
//----------------------------------------------------------------------------
//
// Struct-
//       pub::diag::Stack
//
// Purpose-
//       Stack diagnostics
//
//----------------------------------------------------------------------------
struct Stack {                      // Stack diagostics
const char*            origin= nullptr; // The Stack origin address
const char*            active= nullptr; // The Stack active (current) address
const char*            ending= nullptr; // The Stack ending address
size_t                 length= 0;   // The Stack length
int                    stack_type= 0; // The stack type

enum                                // The stack type
{  ST_RESET= 0                      // RESET, indeterminate
,  ST_ORIGIN_DOWN                   // Stack starts at origin, decreases
,  ST_ORIGIN_UP                     // Stack starts at origin, increases
,  ST_ENDING_DOWN                   // Stack starts at ending, decreases
,  ST_ENDING_UP                     // Stack starts at ending, increases
};

//----------------------------------------------------------------------------
// pub::diag::Stack | Constructors/destructor
//----------------------------------------------------------------------------
   Stack( void )                    // (Default) constructor
{  update(); }                      // Updates the state

   ~Stack( void ) = default;        // Destructor

//----------------------------------------------------------------------------
// pub::diag::Stack | Diagnostic methods
//----------------------------------------------------------------------------
void                                // (ABORT if not in bounds)
   check( void ) const;             // Verify Stack in-bounds

void
   debug(const char*   info= "") const; // Write (debugf) debugging information

void
   trace(const char*   info= "") const; // Write (tracef) debugging information

const char*                         // The stack type (name)
   type( void ) const;              // Get stack type (name)

//----------------------------------------------------------------------------
// pub::diag::Stack | left | Get number of Stack bytes unused
//----------------------------------------------------------------------------
ssize_t                             // The number of Stack bytes remaining
   left( void ) const;              // Get number of Stack bytes remaining

//----------------------------------------------------------------------------
// pub::diag::Stack | used | Get number of Stack bytes used
//----------------------------------------------------------------------------
ssize_t                             // The number of Stack bytes used
   used( void ) const;              // Get number of Stack bytes used

//----------------------------------------------------------------------------
// pub::diag::Stack | Update | Initialize/update the Stack state
//----------------------------------------------------------------------------
void
   update( void );                  // Update the stack state
}; // struct Stack
}  // namespace diag
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DIAG_STACK_H_INCLUDED
