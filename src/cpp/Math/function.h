//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       function.h
//
// Purpose-
//       Define the function to be evaluated
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#define SIGMOID

#if   defined(KSINRX)
   #include "KsinRx.h"

#elif defined(MULTIF)
   #include "multifunc.h"

#elif defined(PARALLEL)
   #include "parallax.h"

#elif defined(SIGMOID)
   #include "sigmoid.h"

#else
   #error "function.h invalid selector"

#endif

