//----------------------------------------------------------------------------
//
//       Copyright (C) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Config.cpp
//
// Purpose-
//       Namespace config definitions.
//
// Last change date-
//       2021/01/24
//
//----------------------------------------------------------------------------
#include "Config.h"                 // Config container definitions

//----------------------------------------------------------------------------
//
// Namespace-
//       config
//
// Purpose-
//       Configuration controls
//
//----------------------------------------------------------------------------
namespace config {                  // Option container
int                    opt_hcdm= false; // Hard Core Debug Mode?
const char*            opt_test= nullptr; // Bringup test?
int                    opt_verbose= -1; // Debugging verbosity
}  // namespace config
