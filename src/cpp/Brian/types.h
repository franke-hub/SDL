//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       types.h
//
// Purpose-
//       Brian type definitions (and common includes)
//
// Last change date-
//       2024/10/10
//
//----------------------------------------------------------------------------
#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <cstdint>                  // For integer types
#include <memory>                   // For std::shared_ptr
#include <new>                      // For std::bad_alloc
#include <string>                   // For std::string
#include <sys/types.h>              // For standard types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Object.h>             // For pub::Object
#include <pub/utility.h>            // For pub::utility subroutines

#endif // TYPES_H_INCLUDED
