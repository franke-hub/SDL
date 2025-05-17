//----------------------------------------------------------------------------
//
//       Copyright (c) 2024-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Loader.h
//
// Purpose-
//       Include built-in objects
//
// Last change date-
//       2025/01/20
//
//----------------------------------------------------------------------------
#ifndef LOADER_H_INCLUDED
#define LOADER_H_INCLUDED

#include "shared_ptr-debug.h"       // For shared_ptr debugging control

//----------------------------------------------------------------------------
//
// Class-
//       Loader
//
// Purpose-
//       Create references to built-in Commands and Services
//
//----------------------------------------------------------------------------
class Loader {                      // Include built-ins
//----------------------------------------------------------------------------
// Loader::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Loader( void );                   // Constructor (Includes built-ins)

// Destructor declaration not required
}; // class Loader
#endif // LOADER_H_INCLUDED
