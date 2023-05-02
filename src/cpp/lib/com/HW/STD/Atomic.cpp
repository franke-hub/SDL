//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HW/STD/Atomic.cpp
//
// Purpose-
//       Atomic library routines: default implementation.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <com/Mutex.h>

#include "com/Atomic.h"

//----------------------------------------------------------------------------
// Internal Mutex
//----------------------------------------------------------------------------
static Mutex*          mutex= NULL; // Our internal Mutex

//----------------------------------------------------------------------------
//
// Class-
//       _Atomic_h_Internal_Mutex_Control
//
// Purpose-
//       Allocate and delete the internal Mutex
//
// Implementation notes-
//       The atomic operations may be called during object construction.
//       Since this object is also constructed, a race condition exists.
//       The Mutex is allocated during static construction either by the
//       control object constructor or by the first call to one of the
//       atomic operations. Note that the Mutex object must not itself be
//       implemented using these atomic operators.
//
//----------------------------------------------------------------------------
static class _Atomic_h_Internal_Mutex_Control {
public:
inline _Atomic_h_Internal_Mutex_Control( void )
{
   getMutex();
}

inline ~_Atomic_h_Internal_Mutex_Control( void )
{
   delete mutex;
   mutex= NULL;
}

static inline Mutex&
   getMutex( void )
{
   if( mutex == NULL )
     mutex= new Mutex();

   return *mutex;
}
} control;                          // Internal Mutex control object

//----------------------------------------------------------------------------
//
// csb (Compare-and-swap byte)
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csb(                             // Compare-and-swap byte
     ATOMIC8*          swapAddr,    // -> Swap byte
     const int8_t      oldValue,    // Old value
     const int8_t      newValue)    // New value
{
   AutoMutex lock(control.getMutex());

   int cc= 1;
   if( (*swapAddr) == oldValue )
   {
     *swapAddr= newValue;
     cc= 0;
   }

   return cc;
}

//----------------------------------------------------------------------------
//
// csd (Compare-and-swap double)
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csd(                             // Compare-and-swap double
     ATOMIC64*         swapAddr,    // -> Swap doubleword
     const int64_t     oldValue,    // Old value
     const int64_t     newValue)    // New value
{
   AutoMutex lock(control.getMutex());

   int cc= 1;
   if( (*swapAddr) == oldValue )
   {
     *swapAddr= newValue;
     cc= 0;
   }

   return cc;
}

//----------------------------------------------------------------------------
//
// csh (Compare-and-swap halfword)
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csh(                             // Compare-and-swap halfword
     ATOMIC16*         swapAddr,    // -> Swap halfword
     const int16_t     oldValue,    // Old value
     const int16_t     newValue)    // New value
{
   AutoMutex lock(control.getMutex());

   int cc= 1;
   if( (*swapAddr) == oldValue )
   {
     *swapAddr= newValue;
     cc= 0;
   }

   return cc;
}

//----------------------------------------------------------------------------
//
// csw (Compare-and-swap word)
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csw(                             // Compare-and-swap word
     ATOMIC32*         swapAddr,    // -> Swap word
     const int32_t     oldValue,    // Old value
     const int32_t     newValue)    // New value
{
   AutoMutex lock(control.getMutex());

   int cc= 1;
   if( (*swapAddr) == oldValue )
   {
     *swapAddr= newValue;
     cc= 0;
   }

   return cc;
}

//----------------------------------------------------------------------------
//
// csp (Compare-and-swap Pointer
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csp(                             // Compare-and-swap pointer
     ATOMICP**         swapAddr,    // -> Swap pointer
     const ATOMICP*    oldValue,    // Old value
     const ATOMICP*    newValue)    // New value
{
   AutoMutex lock(control.getMutex());

   int cc= 1;
   if( (*swapAddr) == oldValue )
   {
     *swapAddr= (ATOMICP*)newValue;
     cc= 0;
   }

   return cc;
}

//----------------------------------------------------------------------------
//
// isync (Instruction synch)
//
// Does not return until all prior machine instructions have completed.
//
//----------------------------------------------------------------------------
extern "C" void
   isync( void )                    // Instruction synch
{
   AutoMutex lock(control.getMutex());
}

//----------------------------------------------------------------------------
//
// tsb (Test-and-set byte)
//
// cc= 1;
// atomic {
//   if (((*swapAddr) & 0x80) == 0)
//     cc= 0;
//   *swapAddr= 0xFF;
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   tsb(                             // Test-and-set byte
     ATOMIC8*          swapAddr)    // -> Test byte
{
   AutoMutex lock(control.getMutex());

   int cc= 1;
   if( ((*swapAddr) & 0x80) == 0 )
     cc= 0;

   *swapAddr= 0xff;

   return cc;
}

