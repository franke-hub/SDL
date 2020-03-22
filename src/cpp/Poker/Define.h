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
//       Define.h
//
// Purpose-
//       Poker definitions.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef DEFINE_H_INCLUDED
#define DEFINE_H_INCLUDED

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef __CHECK__
#define __CHECK__                   // If defined, CHECK code included
#endif

#ifndef __DEBUG__
#define __DEBUG__                   // If defined, DEBUG code included
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifdef __CHECK__
  #define check_cast(into, from) dynamic_cast<into>(from)
#else
  #define check_cast(into, from)  static_cast<into>(from)
#endif

#endif // DEFINE_H_INCLUDED
