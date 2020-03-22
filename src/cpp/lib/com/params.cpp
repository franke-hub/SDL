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
//       params.cpp
//
// Purpose-
//       Parameter controls.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>

#include <com/syslib.h>

#include "com/params.h"

extern "C" {

/****************************************************************************/
/*                                                                          */
/* Subroutine-                                                              */
/*       swskip                                                             */
/*                                                                          */
/* Function-                                                                */
/*       Skip over the parameter name.                                      */
/*                                                                          */
/****************************************************************************/
static const char*                  /* -> Next character after name         */
   swskip(                          /* Skip over the parameter name         */
     const char*       name,        /* The name of the parameter            */
     const char*       parm)        /* The argument string                  */
{
   while (*name == *parm)           /* Scan the characters                  */
   {
     if (*name == '\0')             /* If end of string delimiter           */
       return(parm);                /* Return null string pointer           */
     if (*name == ':')              /* If end of field delimiter            */
       return(parm+1);              /* Return parameter pointer             */

     name++;                        /* Address the next characters          */
     parm++;
   }

   return(parm);                    /* Return the parameter pointer         */
}

/****************************************************************************/
/*                                                                          */
/* Subroutine-                                                              */
/*       swname                                                             */
/*                                                                          */
/* Function-                                                                */
/*       Validate switch name.                                              */
/*                                                                          */
/****************************************************************************/
extern int                          /* TRUE if name match                   */
   swname(                          /* Validate switch name                 */
     const char*     name,          /* The name of the parameter            */
     const char*     parm)          /* The argument string                  */
{
   while (*name == *parm)           /* Scan the characters                  */
   {
     if (*name == '\0')             /* If end of string delimiter           */
       return(TRUE);                /* Exact match, return                  */
     if (*name == ':')              /* If end of field delimiter            */
       return(TRUE);                /* Parameters match, return             */

     name++;                        /* Address the next characters          */
     parm++;
   }

   if (*name != '\0')               /* If not end of name string            */
     return(FALSE);                 /* Parmeters do not match               */

   if (*parm == '+'                 /* If explicit set                      */
       ||*parm == '-')              /* or explicit reset                    */
     return(TRUE);                  /* Parameters match, return             */

   return(FALSE);                   /* Switch is not decodable              */
}

/****************************************************************************/
/*                                                                          */
/* Subroutine-                                                              */
/*       swatob                                                             */
/*                                                                          */
/* Function-                                                                */
/*       Convert parameter string to boolean                                */
/*                                                                          */
/****************************************************************************/
extern int                          /* Parameter value                      */
   swatob(                          /* Convert parameter to boolean         */
     const char*       name,        /* The name of the parameter            */
     const char*       parm)        /* The argument string                  */
{
   parm= swskip(name, parm);        /* Skip over the parameter name         */

   if (*parm == '\0')               /* If null string                       */
     return(TRUE);                  /* Return, TRUE is default              */

   if (*parm == '+')                /* If form "/switch+"                   */
     return(TRUE);                  /* Return, explicit TRUE                */

   if (*parm == '-')                /* If form "/switch-"                   */
     return(FALSE);                 /* Return, explicit FALSE               */

   return(TRUE);                    /* All other forms are INVALID          */
}

/****************************************************************************/
/*                                                                          */
/* Subroutine-                                                              */
/*       swatod                                                             */
/*                                                                          */
/* Function-                                                                */
/*       Convert parameter string to floating point                         */
/*                                                                          */
/****************************************************************************/
extern double                       /* Parameter value                      */
   swatod(                          /* Convert parameter to double          */
     const char*       name,        /* The name of the parameter            */
     const char*       parm)        /* The argument string                  */
{
   parm= swskip(name, parm);        /* Skip over the parameter name         */

   return(atof(parm));              /* Return parameter value               */
}

/****************************************************************************/
/*                                                                          */
/* Subroutine-                                                              */
/*       swatol                                                             */
/*                                                                          */
/* Function-                                                                */
/*       Convert parameter string to integer                                */
/*                                                                          */
/****************************************************************************/
extern long                         /* Parameter value                      */
   swatol(                          /* Convert parameter to long            */
     const char*       name,        /* The name of the parameter            */
     const char*       parm)        /* The argument string                  */
{
   parm= swskip(name, parm);        /* Skip over the parameter name         */

   return(atol(parm));              /* Return integer value                 */
}

/****************************************************************************/
/*                                                                          */
/* Subroutine-                                                              */
/*       swatox                                                             */
/*                                                                          */
/* Function-                                                                */
/*       Convert parameter string to hexadecimal                            */
/*                                                                          */
/****************************************************************************/
extern long                         /* Parameter value                      */
   swatox(                          /* Convert parameter to hex             */
     const char*       name,        /* The name of the parameter            */
     const char*       parm)        /* The argument string                  */
{
   parm= swskip(name, parm);        /* Skip over the parameter name         */

   return(atox(parm));              /* Return integer value                 */
}

} // extern "C"
