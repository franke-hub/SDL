//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Common.h [Name subject to change]
//
// Purpose-
//       Common area
//
// Last change date-
//       2024/11/27
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_COMMON_H_INCLUDED
#define _LIBPUB_COMMON_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <stdexcept>                // For std::runtime_error
#include <thread>                   // For std::thread::id
#include <stdint.h>                 // For uint32_t

#include "pub/Signal.h"             // For pub::Signal

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Namespace-
//       Common
//
// Purpose-
//       Common area for system/application controls
//
//----------------------------------------------------------------------------
namespace Common {                  // Common data area
//----------------------------------------------------------------------------
// Common::System error detection signal
//----------------------------------------------------------------------------
struct error_detection {            // An error detection event
const char*            file;        // File name
int                    line;        // Line number
};

typedef Signal<error_detection>     error_detection_signal;
error_detection_signal error_detected;
}; // struct Common

//----------------------------------------------------------------------------
//
// Struct-
//       Common
//
// Purpose-
//       Common data area
//
//----------------------------------------------------------------------------
struct Common {                     // Common data area
//----------------------------------------------------------------------------
// Common::System error detection signal
//----------------------------------------------------------------------------
struct error_detection {            // An error detection event
const char*            file;        // File name
int                    line;        // Line number
};

typedef Signal<error_detection>     error_detection_signal;
error_detection_signal error_detected;
}  // namespace Common
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_COMMON_H_INCLUDED
