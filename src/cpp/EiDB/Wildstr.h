//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Wildstr.h
//
// Purpose-
//       String functions with wildcard characters.
//
// Last change date-
//       2003/03/16
//
//----------------------------------------------------------------------------
#ifndef WILDSTR_H_INCLUDED
#define WILDSTR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       getWild                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       Get the wildcard list for a character.                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
const char*                         /* Wildcard list                        */
   getWild(                         /* Get wildcard list                    */
     int             wildchar);     /* The wildcard character               */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       setWild                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       Set the wildcard list for character.                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
const char*                         /* Prior value for wildcard             */
   setWild(                         /* Set wildcard character               */
     int             wildchar,      /* The wildcard character               */
     const char*     wildlist);     /* The wildcard value list              */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       wildcmp                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       memcmp with wildcards.                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int                                 /* Resultant (zero or non-zero)         */
   wildcmp(                         /* String compare using wildcards       */
     const char*     string,        /* Source string                        */
     const char*     comparand,     /* Compare string                       */
     unsigned        length);       /* Compare length                       */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       wildseg                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       Substring compare with wildcards.                                  */
/*                                                                          */
/*--------------------------------------------------------------------------*/
int                                 /* Resultant                            */
   wildseg(                         /* Substring compare with wildcards     */
     const char*     source,        /* Source string (comparand)            */
     const char*     target);       /* Target string (comparand)            */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       wildstr                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       strstr with wildcards.                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
char*                               /* Resultant                            */
   wildstr(                         /* Search for substring using wildcards */
     const char*     string,        /* Source string                        */
     const char*     substr);       /* Substring                            */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WILDSTR_H_INCLUDED */
