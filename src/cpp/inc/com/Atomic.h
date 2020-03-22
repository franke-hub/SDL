/*----------------------------------------------------------------------------
**
**       Copyright (c) 2007-2014 Frank Eskesen.
**
**       This file is free content, distributed under the Lesser GNU
**       General Public License, version 3.0.
**       (See accompanying file LICENSE.LGPL-3.0 or the original
**       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
**
**--------------------------------------------------------------------------*/
/*                                                                          */
/* Title-                                                                   */
/*       Atomic.h                                                           */
/*                                                                          */
/* Purpose-                                                                 */
/*       Atomic functions.                                                  */
/*                                                                          */
/* Last change date-                                                        */
/*       2014/01/01                                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef ATOMIC_H_INCLUDED
#define ATOMIC_H_INCLUDED

#include <stdint.h>

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef volatile int64_t ATOMIC64;  // ATOMIC 64-bit value
typedef volatile int32_t ATOMIC32;  // ATOMIC 32-bit value
typedef volatile int16_t ATOMIC16;  // ATOMIC 16-bit value
typedef volatile int8_t  ATOMIC8;   // ATOMIC  8-bit value
typedef volatile void    ATOMICP;   // ATOMIC pointer

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* csb (Compare-and-swap byte)                                              */
/*                                                                          */
/* cc= 1;                                                                   */
/* atomic {                                                                 */
/*   if ((*swapAddr) == oldValue) {                                         */
/*     *swapAddr= newValue;                                                 */
/*     cc= 0;                                                               */
/*     }                                                                    */
/*   }                                                                      */
/* return cc;                                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
extern "C" int                      /* Condition code                       */
   csb(                             /* Compare-and-swap byte                */
     ATOMIC8*          swapAddr,    /* -> Swapbyte                 BDY(BYTE)*/
     const int8_t      oldValue,    /* Old value                            */
     const int8_t      newValue);   /* New value                            */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* csh (Compare-and-swap halfword)                                          */
/*                                                                          */
/* cc= 1;                                                                   */
/* atomic {                                                                 */
/*   if ((*swapAddr) == oldValue) {                                         */
/*     *swapAddr= newValue;                                                 */
/*     cc= 0;                                                               */
/*     }                                                                    */
/*   }                                                                      */
/* return cc;                                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
extern "C" int                      /* Condition code                       */
   csh(                             /* Compare-and-swap halfword            */
     ATOMIC16*         swapAddr,    /* -> Swapbyte                 BDY(HALF)*/
     const int16_t     oldValue,    /* Old value                            */
     const int16_t     newValue);   /* New value                            */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* csd (Compare-and-swap double)                                            */
/*                                                                          */
/* cc= 1;                                                                   */
/* atomic {                                                                 */
/*   if ((*swapAddr) == oldValue) {                                         */
/*     *swapAddr= newValue;                                                 */
/*     cc= 0;                                                               */
/*     }                                                                    */
/*   }                                                                      */
/* return cc;                                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
extern "C" int                      /* Condition code                       */
   csd(                             /* Compare-and-swap double              */
     ATOMIC64*         swapAddr,    /* -> Swapword                BDY(DWORD)*/
     const int64_t     oldValue,    /* Old value                            */
     const int64_t     newValue);   /* New value                            */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* csp (Compare-and-swap Pointer                                            */
/*                                                                          */
/* cc= 1;                                                                   */
/* atomic {                                                                 */
/*   if ((*swapAddr) == oldValue) {                                         */
/*     *swapAddr= newValue;                                                 */
/*     cc= 0;                                                               */
/*     }                                                                    */
/*   }                                                                      */
/* return cc;                                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
extern "C" int                      /* Condition code                       */
   csp(                             /* Compare-and-swap pointer             */
     ATOMICP**       swapAddr,      /* -> Swapword                  BDY(PTR)*/
     const ATOMICP*  oldValue,      /* Old value                            */
     const ATOMICP*  newValue);     /* New value                            */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* csw (Compare-and-swap word)                                              */
/*                                                                          */
/* cc= 1;                                                                   */
/* atomic {                                                                 */
/*   if ((*swapAddr) == oldValue) {                                         */
/*     *swapAddr= newValue;                                                 */
/*     cc= 0;                                                               */
/*     }                                                                    */
/*   }                                                                      */
/* return cc;                                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
extern "C" int                      /* Condition code                       */
   csw(                             /* Compare-and-swap word                */
     ATOMIC32*         swapAddr,    /* -> Swapword                 BDY(WORD)*/
     int32_t           oldValue,    /* Old value                            */
     int32_t           newValue);   /* New value                            */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* isync (Instruction synch)                                                */
/*                                                                          */
/* Does not return until all prior machine instructions have completed.     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
extern "C" void
   isync( void );                   /* Instruction synch                    */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* tsb (Test-and-set byte)                                                  */
/*                                                                          */
/* cc= 1;                                                                   */
/* atomic {                                                                 */
/*   if (((*swapAddr) & 0x80) == 0)                                         */
/*     cc= 0;                                                               */
/*   *swapAddr= 0xFF;                                                       */
/*   }                                                                      */
/* return cc;                                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
extern "C" int                      /* Condition code                       */
   tsb(                             /* Compare-and-swap byte                */
     ATOMIC8*          swapAddr);   /* -> Testbyte                 BDY(BYTE)*/

#endif /* ATOMIC_H_INCLUDED */
