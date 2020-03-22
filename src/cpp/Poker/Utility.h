//----------------------------------------------------------------------------
//
//       Copyright (C) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Utility.h
//
// Purpose-
//       Inline utility functions.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include "Define.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       max
//
// Purpose-
//       Return the higher value.
//
//----------------------------------------------------------------------------
inline int                          // The maximum
   max(                             // Get maximum
     int               a,
     int               b)
{
   int                 result= a;

   if( b > result )
     result= b;

   return result;
}

inline double                       // The maximum
   max(                             // Get maximum
     double            a,
     double            b)
{
   double              result= a;

   if( b > result )
     result= b;

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       min
//
// Purpose-
//       Return the lower value.
//
//----------------------------------------------------------------------------
inline int                          // The minimum
   min(                             // Get minimum
     int               a,
     int               b)
{
   int                 result= a;

   if( b < result )
     result= b;

   return result;
}

inline double                       // The minimum
   min(                             // Get minimum
     double            a,
     double            b)
{
   double              result= a;

   if( b < result )
     result= b;

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       randf
//
// Purpose-
//       Random value, range 0..(.99999)
//
//----------------------------------------------------------------------------
inline double                       // Random value
   randf( void )                    // Get random double value
{
   return (double)(rand()&0x7fffffff) / (double)(0x80000000UL);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       randomly
//
// Purpose-
//       With probability P, return TRUE.
//
//----------------------------------------------------------------------------
inline int                          // TRUE or FALSE
   randomly(                        // Randomly return TRUE or FALSE
     double            p)           // With this probability
{
   unsigned long       P;           // Integer probability, 0 .. 0x80000000UL
   unsigned long       Q;           // Integer random number

   if( p <= 0.0 )
     return FALSE;
   if( p >= 1.0 )
     return TRUE;

   P= (unsigned long)((double)p * (double)(0x80000000UL));
   Q= rand()&0x7fffffff;
   return (P > Q);
}

#endif // UTILITY_H_INCLUDED
