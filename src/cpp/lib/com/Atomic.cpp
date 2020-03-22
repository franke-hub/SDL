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
//       Atomic.cpp
//
// Purpose-
//       Atomic functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#if   defined(_HW_X86)
  #include "HW/X86/Atomic.cpp"
#else
  #include "HW/STD/Atomic.cpp"
#endif

