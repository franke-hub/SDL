//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Include.cpp
//
// Purpose-
//       Test include prerequisites. Include one and only one file.
//
// Last change date-
//       2020/09/06
//
//----------------------------------------------------------------------------
#include <Xcb/Include.h>            // Test ONE include
static_assert(false, "Force compile failure");
