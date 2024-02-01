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
//       ~/doc/cpp/diag-shared_ptr.md
//
// Purpose-
//       diag-shared_ptr.h reference manual
//
// Last change date-
//       2023/12/13
//
-------------------------------------------------------------------------- -->
## Defined in header <pub/diag-shared_ptr.h>

##### diag-shared_ptr.h
`diag-shared-ptr.h` is used to help debug objects containing shared_ptr's.
It operates optionally using a mechanism described later.
When inactive, there is *NO* CPU overhead.

For each object that contains a shared_ptr or a weak_ptr, add:
- `INS_DEBUG_OBJ(name)` in (each) constructor.
- `REM_DEBUG_OBJ(name)` in the destructor.

`name` is the C-string name of the object.

In cases where an object containing shared or weak_ptr's is used (rather than
constructed), add:
- `INS_DEBUG_MAP(name, that)` where an object reference is added.
- `REM_DEBUG_MAP(name, that)` where an object reference is removed.

`that` is the address of the referenced object.

The `INS/REM_DEBUG_MAP` mechanism is used where an object containing
shared or weak_ptr's is dynamically referenced.
This is useful, for example,
when an object containing shared or weak_ptr's is included in a `std::map`
(The map `name` parameter would be the map's logical name).

All source code that use diag-shared_ptr.h include (either directly or
indirectly) a control file as their *first* include before any controlled
include file but *after* any system or otherwise uncontrolled include file.

This control file, when active contains:
```
#undef  USE_DEBUG_PTR
#define USE_DEBUG_PTR
#include "pub/bits/diag-shared_ptr.i"
```

When inactive, the control file contains:
```
#define USE_DEBUG_PTR
#undef  USE_DEBUG_PTR
#include "pub/bits/diag-shared_ptr.i"
```

When active, certain std library functions are redefined:
- #define make_shared pub_diag::make_debug
- #define shared_ptr  pub_diag::debug_ptr
- #define weak_ptr    pub_diag::dweak_ptr

and the INS/REM_DEBUG_OBJ and INS/REM_DEBUG_MAP macros insert and remove object
addresses from the internal container map.

When inactive, the library functions remain unchanged and the
INS/REM_DEBUG_OBJ and INS/REM_DEBUG_MAP macros do nothing and none of the
template code in diag-shared_ptr.h is used.

#### How it works
The library contains two static maps:
- A container map, mapping each container name to its storage address.
- A reference map, mapping each shared or weak pointer to its storage address.

These maps are mutex protected. (All operations are thread-safe.)

The container map is updated by the INS/REM_DEBUG_OBJ and INS/REM_DEBUG_MAP
macros.
The reference map is updated when shared pointers are modified, either
adding or removing the reference.

Combining these two maps, we have a list of shared pointer containers and
a list of active shared pointers.
Method `std::pub_diag::Debug_ptr("caller info")` displays this combined list
sorted by address.
Each reference also includes its offset from the last container address.
(Addresses and offsets are hexidecimal.)

When its global destructor is invoked at program termination, the map display
occurs if any containers or references exist.
(No containers or references should exist at this point, so this display
indicates an error.)

#### Usage notes
Temporary shared_ptr's can also exist in automatic (stack) storage.
Unless INS/REM_DEBUG_MAP is used to define them, these temporaries have no
associated container.

You can either define a pseudo-container using something similar to:
```
void function_F1() {
   std::shared_ptr<Thing> T= new Thing(); // (or copy from another shared_ptr)
   INS_DEBUG_MAP("F1 AUTO Thing", &T); // (In scope)
   // :
   // (Immediately before function_F1 exits)
   REM_DEBUG_MAP("F1 AUTO Thing", &T); // (Out of scope)
}
```

All references are displayed with their offset from a mapped container.
If a  reference's actual container isn't mapped, its offset will be
greater than the length of the mapped container.
Offsets greater than (hexadecimal) ffff are displayed as ffff.

#### Usage notes (example)
The DEV library uses diag-shared_ptr.h logic.
- All DEV include files either contain `#include "dev/bits/devconfig.h`
as their only DEV include file or include another DEV include file.
This logic forces `~/src/cpp/inc/dev/bits/devconfig.h` to be included
by the compiler before any other DEV library include.
- For dependency control, all (source and include) DEV library files that
include DEV files use `#include "dev/include-file.h"` rather than
`#include <dev/include-file.h>`.
