//----------------------------------------------------------------------------
//
//       Copyright (c) 2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       MAC_64.h
//
// Purpose-
//       Document usage for 32/64 bit cross-compile macros.
//
// Last change date-
//       2016/01/01
//
// Implementation notes-
//       No source module includes this header. It is only for documentation.
//
//----------------------------------------------------------------------------
#include <inttypes.h>               // For PRI*64
#include <stdio.h>                  // For printf

   #ifdef SHOULD_NOT OCCUR
     int64_t           V = 12345678901234567890;

     printf("(int64_t %" PRId64 ")\n", this, V);
     printf("(int64_t 0x%" PRIx64 ")\n", this, V);

     #undef _ADDR64
     #if defined(_WIN64) || defined(__x86_64__)
       #define _ADDR64
     #elif !defined(_CC_MSC) && !defined(_CC_GCC)
       #error "_ADDR64 indeterminate"
     #endif
   #endif
