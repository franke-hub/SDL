//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
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
//       2021/01/24
//
//----------------------------------------------------------------------------
#include <Include.h>               // Test ONE include
static_assert(false, "Force compile failure");
