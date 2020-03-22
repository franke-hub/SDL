//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Obsolete.h
//
// Purpose-
//       Obsolete mechanisms.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef OBSOLETE_H_INCLUDED
#define OBSOLETE_H_INCLUDED
#error "Obsolete.h included"

#include "Network.h"

namespace NETWORK_H_NAMESPACE {
//----------------------------------------------------------------------------
// Tried all these before thinking about just using a different pointer to
// get the is_set() access semantics.
//
// struct NN::atomic_flag -or- NN::atomic_flag_ptr
//     All of these implementations work; none of them are needed.
//     Need to run timing tests before selecting production implementation.
//
// TODO: Implement using compare_exchange_weak/strong
//----------------------------------------------------------------------------
#if false
// If the is_set method isn't used there is no performance penalty.
struct atomic_flag : public std::atomic_flag { // Add is_set function
// This implementation requires gnu "atomic_base.h" base implementation
bool is_set( void ) const noexcept { // Surprisingly inefficient
   return _M_i;
}
}; // struct atomic_flag :: public std::atomic_flag


#elif false
// Compatible replacement, used the default restrictive memory order.
// Implementation requires std::atomic_char{}.is_lock_free() == true
struct atomic_flag : public std::atomic_char {
   using std::atomic_char::atomic_char;
   ~atomic_flag() noexcept = default;

// We additionally provide casting operators (bool) and (char)
// For additional casting operators, access flag directly.
inline
   operator bool() noexcept {return operator char() != '\0';}

// Methods
inline void
   clear()
{  exchange('\0'); }

inline bool
   is_set()
{  return operator bool(); }

inline bool
   test_and_set()
{  return exchange(0x01); }
}; // struct atomic_flag : public std::atomic_char


#elif false
// Different symantics: atomic_flag_ptr::method(volatile char* P)
struct atomic_flag_ptr { // Use a regular character as a std::atomic_flag
// This, unfortunately, needs to be a runtime check
// #if sizeof(char) != sizeof(std::atomic_flag)
//  #error "Cannot build atomic_char"
// #endif

static inline void
   clear(                           // Same as with std::atomic_flag* P
     volatile char*    P)           // (*P).clear()
{  std::atomic_flag* Q= (std::atomic_flag*)P;
   (*Q).clear();
}

static inline bool
   is_set(                          // Simple character test
     volatile const char*
                       P)           // (*P) != '\0' (you can do this yourself)
{  return (*P) != '\0'; }

static inline bool
   test_and_set(                    // Same as with std::atomic_flag* P
     volatile char*    P)           // (*P).test_and_set()
{  std::atomic_flag* Q= (std::atomic_flag*)P;
   return (*Q).test_and_set();
}
}; // struct atomic_flag_ptr
#endif
}  // namespace NETWORK_H_NAMESPACE

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_atomic
//
// Purpose-
//       Obsolete parts of Dirty.cpp.test_atomic()
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   test_atomic( void )              // Test std::atomic_flag
{  debugf("test_atomic: Test atomic operations.\n");

   int error_count = 0;
   int DIM= 1024;

   #if false // Now requires Obsolete.h
     NN::atomic_flag    array[DIM];
     debugf("NN::atomic_flag(%p).%ld\n", array, sizeof(array));
     for(int i= 0; i<DIM; i++)
       array[i].clear();
     assert( !array[3].test_and_set() );
     assert( !array[7].test_and_set() );
     assert(  array[3].test_and_set() );
     assert(  array[3].is_set() );
     assert(  array[7].is_set() );
     assert(  array[7].is_set() );
     assert( !array[8].is_set() );
     assert( !array[8].is_set() );
     snap(array, sizeof(array));
   #elif false // Now requires Obsolete.h
     char                array[DIM];
     debugf("char(%p).%ld\n", array, sizeof(array));
     for(int i= 0; i<DIM; i++)
       NN::atomic_char::clear(array+i);
     assert( !NN::atomic_char::test_and_set(array+3) );
     assert( !NN::atomic_char::test_and_set(array+7) );
     assert(  NN::atomic_char::is_set(array+3) );
     assert(  NN::atomic_char::is_set(array+7) );
     assert(  NN::atomic_char::is_set(array+7) );
     assert( !NN::atomic_char::is_set(array+8) );
     assert( !NN::atomic_char::is_set(array+8) );
     snap(array, sizeof(array));
   #endif

   debugf("ec(%d) test_atomic\n\n", error_count);
   return error_count;
}

#if 0
//----------------------------------------------------------------------------
//
// Method-
//       NN::Hidden::fanout
//
// Purpose-
//       Keep the threading comments, fuzzy as they may be.
//
//----------------------------------------------------------------------------
RC                                  // Return code
   Hidden::fanout(                  // Process fanout
     Token             out_token,   // Storage locator
     Pulse             signal)      // Signal strength
{
   IFDEBUG( debugf("Hidden(%s).fanout(%.16" PRIx64 ",%d)\n",
                   _inner.name.c_str(), out_token, signal); )

   // TODO: Performance critical path, optimize
   Index_t index= get_index(out_token);

   v_char_flag* c_flag= _inner.isdone + index;
   if( *c_flag )                    // If already set, don't need to re-set
     return 0;                      // (Avoids cache line modification.)

   if( _inner.check_ticker() ) throw SynchException("clock");
   NN::atomic_flag* a_flag= (NN::atomic_flag*)c_flag;
   if( (*a_flag).test_and_set() )   // If we just lost a close race
     return 0;                      // Exit happy

   if( check_ticker() ) {           // Oh, my! We are now on a new thread
     // If the flag is not set now, the flag clearing process snuck in
     // during the last few statements. In that (unlikely) event, our
     // work on this thread is done.
     if( !(*c_flag) ) throw SynchException("clock");

     // This situation is indeterminate, and complicated.
     //   The inner clock cycle update occurs on some other thread.
     //     On that thread (and only on that thread), these are the stages:
     //       1) Preliminaries: Whatever occurs before step 2
     //       2) The clock cycle is updated
     //          (A small delay can be added here, but that guarantees nothing.)
     //       3) All the flags are cleared
     //       4) A new set of threads are started from the input layer
     //       5) These new threads will start setting flags.
     //   The current thread tested the clock cycle before setting the flag.
     //     That test was in sequence 1, or we wouldn't be here.
     //   We then set the flag. If it was already set we wouldn't be here
     //   We tested the clock cycle again, and found the clock cycle updated.
     //   We have to either figure out or guess where in the clock update
     //   sequence we set the flag:
     //     Was it before or after sequence 3?
     //     Did some thread set the same flag in sequence 5?
     //
     //   Complicated.
     //
     //   The flag is set, but by who? Did (or will) another thread sneak in?
     //   There are essentially three actions we can take:
     //     action 0: Do nothing: throw the SynchException
     //               We exit with the flag still set.
     //     action 1: Clear the flag, then throw the SynchException
     //               We back out the change to the flag.
     //     action 2: Change our thread clock to the inner clock and continue.
     //
     //
     //   case 1: Another thread has also set the flag. (Unlikely.)
     //     action 0: Just exit
     //         This is a correct action. The other
     //     action 1: Clear the flag and exit
     //         This is a correct action. ????????
     //     action 2: Change our clock to the new tick and continue.
     //         The fanout will be run twice. This causes the weights
     //         to be set twice (bad). This can propagate further down
     //         the list, maybe. Anyway, it's not good.
     //   case 2: No other thread already set the flag. (Probable.)
     //     case 2a: No other thread has or will check the flag before we:
     //         action 1: Clear the flag and exit
     //             This is a correct action.
     //         action 2: Change our clock to the new tick and continue.
     //             This is also a correct action.
     //     case 2b: Another thread skips processing the neuron before we can
     //              complete our action.
     //         action 1: Clear the flag and exit
     //             This is somewhat bad. The propagation won't occur unless
     //             the fanout is driven again. However, any fanin checker
     //             that runs after this will do the job. (TODO: DOUBLE CHECK.)
     //         action 2: Change our clock to the new tick and continue.
     //             This is a correct action.
     //-----------------------------------------------------------------------
     // We can choose our action randomly, or just go with one.
     // Somewhat boggled and unsure that something wasn't missed, here's:
     #if true    // Action 0 or 1
       #if true  // Action 1
         (*a_flag).clear();
       #endif
       throw SynchException("clock");
     #else       // Action 2
       thread->clock= _inner.clock;
     #endif
   }

   FanoutBundle& _fanout= _inner.fanout[index];
   for(int i= 0; i<FanoutBundle::DIM; i++) {
     Token inp_token= _fanout.index[i];
     if( inp_token == 0 ) {
       // TODO: Maybe we can grow a fanout here, or something.
       break;
     }

     inp_token += _ending;
     Pulse weight= _fanout.weight[i];
     NetNeuron& neuron= _inner.locate_Neuron(inp_token);
     rc += neuron.run(_thread, inp_token, weight);
   }

   return rc;
}

inline RC                           // Fanin count
   FanoutNeuron::fanout_bundle(     // Drive fanout bundle
//   Token             source,      // (ONLY FOR DEBUGGING)
     FanoutBundle&     bundle,      // The bundle
     Value_t           trigger)     // The trigger value
{
   RC rc= 0;
   for(int i= 0; i<FanoutBundle::DIM; i++) {
     Token index= bundle.index[i];

     // The trigger is modified by the fanout weight
     Pulse pulse= to_pulse(trigger, bundle.weight[i]);

#if false
     IFDEBUG( if( _testID == 202 ) {
         Buffer buffer;
         debugf("%s\n>> HCDM: fanout_bundle(%lx)[%lx]\n", to_buffer(buffer),
                source, index);
         debugf(">> HCDM: trigger(%d) bundle.weight(%d) pulse(%d)\n",
                trigger, bundle.weight[i], pulse);
     } ) // IFDEBUG( if ...
#endif

     if( pulse != 0 ) {
       Token token= _ending + index;
       rc += _nextN->fanin(token, pulse);
     }
   }

   return rc;
}
#endif

#endif // OBSOLETE_H_INCLUDED
