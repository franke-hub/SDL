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
//       HW/X86/Hardware.cpp
//
// Purpose-
//       Hardware object implementation: X586 processor
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include "com/Hardware.h"

#if   defined(_MSC_VER)             // Trigger for Microsoft compiler
  #include "Hardware.WIN"
#elif defined(__GNUC__)             // Trigger for GNU compiler
  #include "Hardware.GNU"
#else
  #error "Compiler not supported"
#endif

