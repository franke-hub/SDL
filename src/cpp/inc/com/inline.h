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
//       inline.h
//
// Purpose-
//       Inlining global controls.
//
// Last change date-
//       2007/01/01
//
// Usage notes-
//       This include file provides global control over inline methods.
//       A complete recompile is required if this header is changed since
//       includes which include this file may be <angle> includes.
//
//       Normal user includes:
//         #include <something.h>   // Prototypes
//
//       In "something.h"   (prototypes)
//         #ifndef  SOMETHING_H_INCLUDED
//         #define  SOMETHING_H_INCLUDED
//         #include "inline.h"      // Defines INLINE and INLINING
//         :
//         INLINE ReturnType method(Parameter p, ...);
//         :
//         #if INLINING
//         #include "something.i"   // Methods
//         #endif
//
//         #endif// !defined(SOMETHING_H_INCLUDED)
//
//       In "something.i"   (methods)
//         #ifndef  SOMETHING_I_INCLUDED
//         #define  SOMETHING_I_INCLUDED
//         :
//         ReturnType method(Parameter p, ...) // Note absence of INLINE
//         {// Method implementation
//         }
//         :
//         #endif// !defined(SOMETHING_I_INCLUDED)
//
//       In something.cpp (the "home" for methods in "something.i")
//         #include "something.h"   // Prototypes
//         #if !(INLINING)
//         #include "something.i"   // Methods
//         #endif
//
// Macro variables-
//       INLINING
//         Always defined, boolean.  (Default defined below.)
//         If 0, inline methods default to external calls.
//         If 1, inline methods default to inline calls.
//
//       INLINE
//         Always defined, string.   (Dependent upon INLINING)
//         May be either "" or "inline".
//
//----------------------------------------------------------------------------
#ifndef INLINE_H_INCLUDED
#define INLINE_H_INCLUDED
#endif

//----------------------------------------------------------------------------
// Constants for parameterization -- Defaults
//----------------------------------------------------------------------------
#ifndef INLINING                    // Default action for inline compilation
#define INLINING                  1 // If 1, inlines expand inline
#endif

#if     INLINING                    // If inlines expand inline
//----------------------------------------------------------------------------
// Compile inlines as inline calls
//----------------------------------------------------------------------------
#ifndef INLINE
#define INLINE inline
#endif

#else                               // If inlines expand to external calls
//----------------------------------------------------------------------------
// Compile inlines as external calls
//----------------------------------------------------------------------------
#ifndef INLINE
#define INLINE
#endif

#endif  // INLINING
