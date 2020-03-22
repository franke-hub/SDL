/*----------------------------------------------------------------------------
**
**       Copyright (C) 2007 Frank Eskesen.
**
**       This file is free content, distributed under the Lesser GNU
**       General Public License, version 3.0.
**       (See accompanying file LICENSE.LGPL-3.0 or the original
**       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
**
**--------------------------------------------------------------------------*/
/*                                                                          */
/* Title-                                                                   */
/*       hcdm.h                                                             */
/*                                                                          */
/* Purpose-                                                                 */
/*       Debug controls.                                                    */
/*                                                                          */
/* Last change date-                                                        */
/*       2007/01/01                                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef HCDM_H_INCLUDED
#define HCDM_H_INCLUDED

/*--------------------------------------------------------------------------*/
/* Constants for parameterization                                           */
/*--------------------------------------------------------------------------*/
#ifndef HCDM
#undef  HCDM                        /* If defined, Hard Core Debug Mode     */
#endif

#ifndef IODM
#undef  IODM                        /* If defined, I/O Debug Mode           */
#endif

#ifndef SCDM
#undef  SCDM                        /* If defined, Soft Core Debug Mode     */
#endif

#define IFHCDM() if( hcdm )
#define IFIODM() if( iodm )
#define IFSCDM() if( scdm )

/*--------------------------------------------------------------------------*/
/* Hard-Core Debug Mode control                                             */
/*--------------------------------------------------------------------------*/
extern int             hcdm;        /* Hard Core Debug Mode control         */
extern int             iodm;        /* I/O Debug Mode control               */
extern int             scdm;        /* Soft Core Debug Mode control         */

#endif /* HCDM_H_INCLUDED */
