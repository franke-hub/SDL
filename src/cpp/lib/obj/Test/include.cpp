//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       include.cpp
//
// Purpose-
//       Test include files.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "com/Debug.h"

#include "obj/Allocator.h"
#include "obj/Array.h"
#include "obj/Event.h"
#include "obj/Exception.h"
#include "obj/Latch.h"
#include "obj/List.h"
#include "obj/Object.h"
#include "obj/Ref.h"
#include "obj/Semaphore.h"
#include "obj/Statistic.h"
#include "obj/String.h"
#include "obj/Thread.h"

#include "obj/built_in.h"
#include "obj/define.h"
#include "obj/ifmacro.h"

using namespace _OBJ_NAMESPACE;

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count
//   char*           argv[])        // Argument array
{
   printf("Compile-only test\n");
   return 0;
}

