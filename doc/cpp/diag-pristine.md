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
//       ~/doc/cpp/diag-pristine.md
//
// Purpose-
//       diag-pristine.h reference manual
//
// Last change date-
//       2023/12/14
//
-------------------------------------------------------------------------- -->
## Defined in header <pub/diag-pristine.h>

##### diag-pristine.h
`diag-pristine.h` is used to help debug "wild store" problems when the object
being compromised is known.

Usage notes-
   For an object declared as `X object` that you suspect is getting
clobbered by wild stores, use:
```
   pub::Pristine before;
   X object;
   pub::Pristine after;
```

   The Pristine destructor invokes check("Destructor").
You can also invoke the check method at any time.

   REMOVE Pristine declarations in production code.

#### Attributes
`public: static int opt_hcdm;` If non-zero, when check detects an error
in addition to the error message the object content is dumped.
(Its default value is 0.)

#### int check(const char* info=")
Checks the Pristine object for consistency.
The return code is zero if the the object is consistent.
If inconsistent, an error message is written and the return code is non-zero.
