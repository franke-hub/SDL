<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/config.md
//
// Purpose-
//       config.h reference manual
//
// Last change date-
//       2023/11/16
//
-------------------------------------------------------------------------- -->
\#include <pub/config.h>

#### Defines the following macros:

- The PUB library's namespace name: `#define _PUB_NAMESPACE pub`

- The no-return attribute: `#define ATTRIB_NORETURN __attribute__ ((noreturn))`

- The GNU printf method attribute:
`#define ATTRIB_PRINTF(fmt, arg) __attribute__ ((format (printf, fmt, arg)))`
  - Parameters-
    - `fmt` is the attribute number of the printf format argument, and
    - `arg` is the attribute number of the first variable argument.
For a va_list argument, `arg` is specified as 0 and must immediately follow
the `fmt` argument.

  - Attribute numbers begin at 1.
For non-static class methods, the attribute numbers begin at 2.
(The implied `this` object pointer is argument number 1.)
