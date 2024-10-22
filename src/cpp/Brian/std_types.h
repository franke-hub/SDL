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
//       std_types.h
//
// Purpose-
//       STL type definitions and exports
//
// Last change date-
//       2024/10/10
//
//----------------------------------------------------------------------------
#ifndef STD_TYPES_H_INCLUDED
#define STD_TYPES_H_INCLUDED

#include <cstdint>                  // For integer types
#include <memory>                   // For std::make_shared, std::shared_ptr
#include <new>                      // For std::bad_alloc
#include <string>                   // For std::string
#include <sys/types.h>              // For standard types

using std::make_shared;
using std::shared_ptr;
using std::bad_alloc;
using std::string;

#endif // STD_TYPES_H_INCLUDED
