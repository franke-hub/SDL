/*----------------------------------------------------------------------------
**
**       Copyright (C) 2007 Frank Eskesen.
**
**       This file is free content, distributed under the Lesser GNU
**       General Public License, version 3.0.
**       (See accompanying file LICENSE.LGPL-3.0 or the original
**       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
**
*****************************************************************************/
/*                                                                          */
/* Title-                                                                   */
/*       params.h                                                           */
/*                                                                          */
/* Purpose-                                                                 */
/*       Parameter controls.                                                */
/*                                                                          */
/* Last change date-                                                        */
/*       2007/01/01                                                         */
/*                                                                          */
/****************************************************************************/
#ifndef PARAMS_H_INCLUDED
#define PARAMS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

extern int                          /* TRUE if name matches                 */
   swname(                          /* Validate parameter name              */
     const char*       name,        /* The name of the parameter            */
     const char*       parm);       /* The argument string                  */

extern int                          /* Parameter value                      */
   swatob(                          /* Convert parameter to boolean         */
     const char*       name,        /* The name of the parameter            */
     const char*       parm);       /* The argument string                  */

extern double                       /* Parameter value                      */
   swatod(                          /* Convert parameter to double          */
     const char*       name,        /* The name of the parameter            */
     const char*       parm);       /* The argument string                  */

extern long                         /* Parameter value                      */
   swatol(                          /* Convert parameter to long            */
     const char*       name,        /* The name of the parameter            */
     const char*       parm);       /* The argument string                  */

extern long                         /* Parameter value                      */
   swatox(                          /* Convert parameter to hex             */
     const char*       name,        /* The name of the parameter            */
     const char*       parm);       /* The argument string                  */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* PARAMS_H_INCLUDED */
