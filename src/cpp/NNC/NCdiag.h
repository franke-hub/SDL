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
//       NCdiag.h
//
// Purpose-
//       (NC) Neural Net Compiler: Diagnostics
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NCDIAG_H_INCLUDED
#define NCDIAG_H_INCLUDED

//----------------------------------------------------------------------------
// BRINGF() - printf if __BRINGUP__ is true
//----------------------------------------------------------------------------
#if __BRINGUP__                     // If bringup mode
#define BRINGUP(x) x

#else
#define BRINGUP(x)
#endif

#define BRINGF(x) BRINGUP(printf x) // Bringf becomes printf

#endif // NCDIAG_H_INCLUDED
