//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       config/config.h
//
// Purpose-
//       Standard obj library configuration controls.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJ_CONFIG_CONFIG_H_INCLUDED
#define OBJ_CONFIG_CONFIG_H_INCLUDED

#include <obj/define.h>             // For _OBJ_NAMESPACE

//----------------------------------------------------------------------------
//
// Namespace-
//       obj::config
//
// Purpose-
//       Define configuration controls.
//       (Currently just a place holder.)
//
//----------------------------------------------------------------------------
namespace _OBJ_NAMESPACE::config {
//----------------------------------------------------------------------------
// Generic controls
//----------------------------------------------------------------------------
enum                                // BUILD_MODE
{  MODE_BRINGUP= 0                  // Bringup test
,  MODE_ALPHA                       // Alpha test
,  MODE_BETA                        // Beta test
,  MODE_PRODUCTION                  // Production mode

,  MODE_DEFAULT= MODE_BETA          // Default mode
}; // enum

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Enable statistical counters?
{  USE_STATISTICS= true             // Enable statistical counters?
}; // enum
}  // namespace _OBJ_NAMESPACE::config

#endif // OBJ_CONFIG_CONFIG_H_INCLUDED
