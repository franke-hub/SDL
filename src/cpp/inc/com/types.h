/*----------------------------------------------------------------------------
**
**       Copyright (C) 2007-2012 Frank Eskesen.
**
**       This file is free content, distributed under the Lesser GNU
**       General Public License, version 3.0.
**       (See accompanying file LICENSE.LGPL-3.0 or the original
**       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
**
**--------------------------------------------------------------------------*/
/*                                                                          */
/* Title-                                                                   */
/*       types.h                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       Standard type definitions.                                         */
/*                                                                          */
/* Last change date-                                                        */
/*       2012/01/01                                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#include <sys/types.h>              /* This include is guaranteed           */

#ifdef _OS_WIN
  #ifndef _SIZE_T_DEFINED
    #define _SIZE_T_DEFINED
    #ifdef _WIN64
      typedef unsigned __int64 size_t;
    #else
      typedef unsigned int   size_t;
    #endif
  #endif
#endif
