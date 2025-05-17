//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       diag-stack.cpp
//
// Purpose-
//       Implement diag-stack.h.
//
// Last change date-
//       2024/11/22
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For pthread_getattr_np
#endif

#include <mutex>                    // For std::lock_guard
#include <cerrno>                   // For errno
#include <cstring>                  // For strerror

#include <pthread.h>                // For pthread interface
#include <unistd.h>                 // For pause

#include <pub/Debug.h>              // For namespace pub::debugging methods
#include "pub/diag-stack.h"         // For namespace pub::Stack, implemented
#include "pub/utility.h"            // For utility::dump

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;

namespace _LIBPUB_NAMESPACE {
namespace diag {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static void report_error(int, const char*);

//----------------------------------------------------------------------------
//
// Suboutine-
//       direction
//
// Purpose-
//       Return the stack direction
//
// Implementation note-
//       This subroutine is USELESS and INCORRECT if inlined.
//
//----------------------------------------------------------------------------
static int __attribute__ ((noinline)) direction(const Stack*);
static int __attribute__ ((noinline))      // The stack direction: <0 || >0
   direction(                       // Get stack direction
     const Stack*      stack)       // The Caller's Stack
{
   char                callee[8];   // A callee attribute
// debugf("callee(%p) caller(%p)\n", callee, stack->active);
   return int(callee - stack->active); // The stack direction
}

//----------------------------------------------------------------------------
//
// Suboutine-
//       f2c
//
// Purpose-
//       Convert FSM to C-string
//
//----------------------------------------------------------------------------
static const char*                  // The FSM name
   f2c(                             // Get FSM name
     int               fsm)         // From this FSM
{
   if( fsm == Stack::ST_ENDING_UP )
     return "ENDING_DOWN";

   if( fsm == Stack::ST_ORIGIN_UP )
     return "ORIGIN_UP";

   if( fsm == Stack::ST_ENDING_DOWN )
     return "ENDING_DOWN";

   if( fsm == Stack::ST_ORIGIN_DOWN )
     return "ORIGIN_DOWN";

   return "Invalid";
}

//----------------------------------------------------------------------------
//
// Suboutine-
//       init_attr
//
// Purpose-
//       Initialize the current thread attributes (Do not delete)
//
//----------------------------------------------------------------------------
static int                          // Return code, (errno or zero)
   init_attr(                       // Initialize (get) current attributes
     pthread_attr_t* attr)          // The current attributes
{
   int rc= pthread_getattr_np(pthread_self(), attr);
   if( rc )
     report_error(rc, "pthread_getattr_np");

   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       report_error
//
// Purpose-
//       Report an error
//
//----------------------------------------------------------------------------
static void
   report_error(                    // Handle an error
     int               rc,          // ERRNO code
     const char*       op)          // The failing operation name
{  debugf("%s ERROR %d:%s\n", op, rc, strerror(rc)); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::diag::Stack::Stack
//
// Purpose-
//       Constructor (Currently inlined)
//
//----------------------------------------------------------------------------
//   Stack::Stack( void )             // Constructor
//{  update(); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::diag::Stack::check
//       pub::diag::Stack::debug
//       pub::diag::Stack::trace
//       pub::diag::Stack::type
//
// Purpose-
//       Stack bounds check
//       Diagnostic display
//       Diagnostic trace
//       Get stack type
//
//----------------------------------------------------------------------------
void
   Stack::check( void ) const       // Stack bounds check
{
   const char* error_type= nullptr;
   switch(stack_type) {
     case ST_ORIGIN_DOWN:           // ORIGIN => ENDING (Origin high)
       if( active > origin || active <= (origin+64) )
         error_type= "UNDERFLOW";
       break;

     case ST_ORIGIN_UP:             // ORIGIN => ENDING (Origin low)
       if( active < origin || active >= (ending-64) )
         error_type= "OVERFLOW";
       break;

     case ST_ENDING_DOWN:           // ENDING => ORIGIN (Ending high)
       if( active > ending || active >= (origin+64) )
         error_type= "UNDERFLOW";
       break;

     case ST_ENDING_UP:             // ENDING => ORIGIN (Ending low)
       if( active < ending || active >= (origin-64) )
         error_type= "OVERFLOW";
       break;

     default:
       debugf("%4d pub::diag::Stack stack_type(%d)\n", __LINE__, stack_type);
       break;
   }

   if( error_type ) {
     debugf("\n\n\nSTACK %s, Thread paused\n\n\n", error_type);
     pause();                       // It's debugging time!
   }
}

void
   Stack::debug(                    // Diagnostic display
     const char*       info) const  // Caller information
{
   std::lock_guard<Debug> lock(*Debug::get());

#if 1 // New version
   debugf("diag-stack::Stack(%p)::debug(%s)\n", this, info);

   debugf("..origin(0x%12.12zx) active(0x%12.12zx) ending(0x%12.12zx)\n"
          "..length(    0x%.8zx)   used(    0x%.8zx)   left(    0x%.8zx)\n"
         , intptr_t(origin), intptr_t(active), intptr_t(ending)
         , length , used(), left());
#elif 1 // This change stopped it from working, believe it or not
   debugf("diag-stack::Stack(%p)::debug(%s)\n", this, info);

   debugf("..TYPE(%s) used(%#.8zx) left(%#.8zx)\n"
          "..origin(%p) active(%p) ending(%p) length(%#zx)\n"
         , f2c(stack_type), used(), left(), origin, active, ending, length);
#elif 1 // ORIGINAL, but reverting the change doesn't make it work
      // Currently running with OPTIMIZE=-g; Don't know what it was when working
      // Also currently running with USE_STATIC==true. (Change from AOK?)
   debugf("..TYPE(%s)\n..origin(%p) active(%p) ending(%p) length(%#zx)\n"
         , f2c(stack_type), origin, active, ending, length);
   debugf("..used(%#.8zx) left(%#.8zx)\n", used(), left());
#endif
}

void
   Stack::trace(                    // Diagnostic trace
     const char*       info) const  // Caller information
{
   std::lock_guard<Debug> lock(*Debug::get());

#if 1 // New version
   tracef("diag-stack::Stack(%p)::trace(%s)\n", this, info);

   tracef("..origin(0x%12.12zx) active(0x%12.12zx) ending(0x%12.12zx)\n"
          "..length(    0x%.8zx)   used(    0x%.8zx)   left(    0x%.8zx)\n"
         , intptr_t(origin), intptr_t(active), intptr_t(ending)
         , length , used(), left());
#else // Old(er) version
   tracef("diag-stack::Stack(%p)::trace(%s)\n", this, info);

   tracef("..origin(%p) active(%p) ending(%p) length(%#zx)\n"
          "..used(%#.8zx) left(%#.8zx)\n"
         , origin, active, ending, length, used(), left());
#endif
}

const char*                         // The Stack type name
   Stack::type( void ) const        // Get Stack type name
{  return f2c(stack_type); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::diag::Stack::left
//
// Purpose-
//       Determine remaining stack size
//
//----------------------------------------------------------------------------
ssize_t                             // The remaining stack space length
   Stack::left( void ) const        // Get remaininig stack space length
{  return length - used(); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::diag::Stack::used
//
// Purpose-
//       Determine used stack size
//
//----------------------------------------------------------------------------
ssize_t                             // The used stack space length
   Stack::used( void ) const        // Get used stack space length
{
   switch(stack_type) {
     case ST_ORIGIN_DOWN:           // ORIGIN => ENDING (Origin high)
       return origin - active;
       break;

     case ST_ORIGIN_UP:             // ORIGIN => ENDING (Origin low)
       return active - origin;
       break;

     case ST_ENDING_DOWN:           // ENDING => ORIGIN (Ending high)
       return ending - active;
       break;

     case ST_ENDING_UP:             // ENDING => ORIGIN (Ending low)
       return active - ending;
       break;

     default:
       debugf("%4d pub::diag::Stack stack_type(%d)\n", __LINE__, stack_type);
       return length;
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::diag::Stack::update
//
// Purpose-
//       Update the stack state
//
//----------------------------------------------------------------------------
void
   Stack::update( void )            // Update the stack state
{
   pthread_attr_t      attr;        // Our attributes
   int rc= init_attr(&attr);        // Retrieve attributes
   if( rc )                         // If error
     return;                        // We can't determine bounds

   // Get stack origin and length (for static initialization thread)
   rc= pthread_attr_getstack(&attr, (void**)&origin, &length);
   if( rc ) {
     report_error(rc, "pthread_attr_getstack");
     return;
   }

   // Set stack type
   active= (const char*)&attr;
   int updown= direction(this);

   if( updown > 0 ) {               // If stack goes upward
     if( active > origin ) {
       stack_type= ST_ORIGIN_UP;
       ending= origin + length;
       // debugf("%4d ORG_UP origin(%p) active(%p) ending(%p)\n", __LINE__
       //       , origin, active, ending);
     } else {
       stack_type= ST_ENDING_UP;
       ending= origin - length;
       // debugf("%4d END_UP origin(%p) active(%p) ending(%p)\n", __LINE__
       //       , origin, active, ending);
     }
   } else {                         // If stack goes downward
     if( active > origin ) {
       stack_type= ST_ENDING_DOWN;
       ending= origin + length;
       // debugf("%4d END_DOWN origin(%p) active(%p) ending(%p)\n", __LINE__
       //       , origin, active, ending);
     } else {
       stack_type= ST_ORIGIN_DOWN;
       ending= origin - length;
       // debugf("%4d ORG_DOWN origin(%p) active(%p) ending(%p)\n", __LINE__
       //       , origin, active, ending);
     }
   }
}
}  // namespace diag- - - - - - - - - - -- - - - - - - - - - - - - - - - - - -
}  // namespace _LIBPUB_NAMESPACE
