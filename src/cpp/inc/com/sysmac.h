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
//       sysmac.h
//
// Purpose-
//       Builtin macro functions.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       Edit mode[UNIX] required for backslash cross-system compiles.
//
//----------------------------------------------------------------------------
#ifndef SYSMAC_H_INCLUDED
#define SYSMAC_H_INCLUDED

#ifndef DEFINE_H_INCLUDED
#include "define.h"                 // This include is guaranteed
#endif

#ifndef DEBUG_H_INCLUDED
#include <com/Debug.h>              // (For debugf)
#endif

//----------------------------------------------------------------------------
// NEEDS_WORK("string")                Used when code is incomplete
//----------------------------------------------------------------------------
#define NEEDS_WORK(string) \
    errorf("%s %4d: NEEDS_WORK(%s)\n", __FILE__, __LINE__, string)

//----------------------------------------------------------------------------
// Conversions
//----------------------------------------------------------------------------
#define C2L(c) *((long*)(c))        // *(char*) to long
#define C2S(c) *((short*)(c))       // *(char*) to short
#define L2P(l) ((void*)(l))
#define P2L(p) ((unsigned long)(p))

//----------------------------------------------------------------------------
// LOG2
//----------------------------------------------------------------------------
#undef log2
#define log2(x) \
   ( (x <          1) ? -1 \
   : (x <          2) ?  0 \
   : (x <          4) ?  1 \
   : (x <          8) ?  2 \
   : (x <         16) ?  3 \
   : (x <         32) ?  4 \
   : (x <         64) ?  5 \
   : (x <        128) ?  6 \
   : (x <        256) ?  7 \
   : (x <        512) ?  8 \
   : (x <       1024) ?  9 \
   : (x <       2048) ? 10 \
   : (x <       4096) ? 11 \
   : (x <       8192) ? 12 \
   : (x <      16384) ? 13 \
   : (x <      32768) ? 14 \
   : (x <      65536) ? 15 \
   : (x <     131072) ? 16 \
   : (x <     262144) ? 17 \
   : (x <     524288) ? 18 \
   : (x <    1048576) ? 19 \
   : (x <    2097152) ? 20 \
   : (x <    4194304) ? 21 \
   : (x <    8388608) ? 22 \
   : (x <   16777216) ? 23 \
   : (x <   33554432) ? 24 \
   : (x <   67108864) ? 25 \
   : (x <  134217728) ? 26 \
   : (x <  268435456) ? 27 \
   : (x <  536870912) ? 28 \
   : (x < 1073741824) ? 29 \
   : (x < 2147483648) ? 30 \
   :                    31 \
   )

//----------------------------------------------------------------------------
// MAX() and MIN()
//----------------------------------------------------------------------------
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b)) // Maximum (2 parameters)
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b)) // Minimum (2 parameters)
#endif

//----------------------------------------------------------------------------
// OFFSETOF()
//----------------------------------------------------------------------------
// #undef  offsetof                    // System version is BROKEN!
// #ifndef offsetof                    // If not already defined
// extern const char*     NULL_POINTER;// A NULL pointer
// #define offsetof(s_name, s_elem) ((size_t)&(((s_name*)NULL_POINTER)->s_elem))
// #endif

//----------------------------------------------------------------------------
// ROUND()  - Round x up to next m boundary
// TRUNC()  - Truncate x at m boundary
//----------------------------------------------------------------------------
#define trunc(x,m)  (((x) / (m)) * (m))// Truncate x at m
#define round(x,m)  trunc(((x)+(m)-1), (m))// Round x to next m

//----------------------------------------------------------------------------
// ROUND2()  - Round x up to next m boundary, m is a power of 2
// TRUNC2()  - Truncate x at m boundary, m is a power of 2
//----------------------------------------------------------------------------
#define trunc2(x,m)  ((x) & ~((m) - 1)) // Truncate x at m
#define round2(x,m)  trunc2(((x)+(m)-1), (m)) // Round x to next m

//----------------------------------------------------------------------------
// URHERE()  - Debugging trace aid
//----------------------------------------------------------------------------
#ifndef URHERE
#define URHERE() debugf("%s %4d: URHERE()\n", __FILE__, __LINE__);
#endif

#endif                              // SYSMAC_H_INCLUDED
