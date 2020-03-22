//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_Mem.h
//
// Purpose-
//       Memory test control.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef TEST_MEM_H_INCLUDED
#define TEST_MEM_H_INCLUDED

//----------------------------------------------------------------------------
// External routines
//----------------------------------------------------------------------------
void
   memtest0(                        // Memory test routine
     unsigned*         addr,        // Region address
     size_t            size);       // Region length

#endif // TEST_MEM_H_INCLUDED
