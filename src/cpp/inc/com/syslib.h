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
//       syslib.h
//
// Purpose-
//       Standard library functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SYSLIB_H_INCLUDED
#define SYSLIB_H_INCLUDED

#include <string.h>

#ifndef SYSMAC_H_INCLUDED
#include "sysmac.h"                 // This include is guaranteed
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// ATOX()
//----------------------------------------------------------------------------
extern long                         // Resultant value
   atox(                            // Convert ascii to hexidecimal
     const char*       ptrstr);     // Pointer to string

//----------------------------------------------------------------------------
// NOP()
//----------------------------------------------------------------------------
extern void
   nop(                             // No function
     const void*       ignored);    // Quasi-refer to this

//----------------------------------------------------------------------------
// ZERO()
//----------------------------------------------------------------------------
#define zero(a,l)      memset((a), 0, (l))

#ifdef __cplusplus
} // extern "C"
#endif

#endif                              // SYSLIB_H_INCLUDED
