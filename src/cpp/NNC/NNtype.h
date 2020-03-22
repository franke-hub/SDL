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
//       NNtype.h
//
// Purpose-
//       (NN) Neural Net: Typedefs
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NNTYPE_H_INCLUDED
#define NNTYPE_H_INCLUDED

#include <ctype.h>
#include <stdint.h>

#ifndef DEFINE_H_INCLUDED
#include <com/define.h>
#endif

#ifndef PGS_H_INCLUDED
#include <PGS.h>
#endif

#ifndef NN_H_INCLUDED
#include "NN.h"
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       makeUPPER
//
// Purpose-
//       Convert a string to upper case.
//
//----------------------------------------------------------------------------
static inline void
   makeUPPER(                       // Convert
     char*             str)         // This string to UPPER CASE
{
   while( *str != '\0' )
   {
     *str= toupper(*str);
     str++;
   }
}

#endif // NNTYPE_H_INCLUDED
