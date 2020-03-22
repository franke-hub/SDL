//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
// jconfig.h router
//----------------------------------------------------------------------------
#ifdef _WIN32
  #include "win/jconfig.h"
#else
  #include "bsd/jconfig.h"
#endif
