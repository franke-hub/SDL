//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbBase.i
//
// Purpose-
//       Define (or redefine) the DbBase macros.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#undef  DBCHECKSTOP                 // Allow multiple includes
#undef  DBCHECK
#undef  DBDEBUG

#define DBCHECKSTOP(message) checkstop(__FILE__, __LINE__, message);
#define DBCHECK(cc, message) dbCheck(__FILE__, __LINE__, int(cc), message);

#ifdef HCDM
#define DBDEBUG(rc, message) dbDebug(__FILE__, __LINE__, int(rc), message)
#else
#define DBDEBUG(rc, message) ((void)0)
#endif

