<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2026 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
// SPDX-License-Identifier: CC-BY-4.0
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/pub_disp-item.md
//
// Purpose-
//       Dispatch.h reference manual: Item
//
// Last change date-
//       2026/04/18
//
-------------------------------------------------------------------------- -->
## (pub::dispatch::)Item
###### Defined in header <pub/Dispatch.h>

## <a id="struct-item">class pub\::dispatch\::Item : public AI_list<pub::dispatch::Item>::Link</a>

### *Attributes*

Constants:

Completion codes: (enum CC)
- CC_NORMAL= 0 // Normal (OK)
- CC_PURGE= -1 // Function purged
- CC_ERROR= -2 // Generic error
- CC_ERROR_FC= // Invalid function code

Function codes: (enum FC)
- FC_VALID= 0  // (All other user function codes are positive)
- FC_CHASE= -1 // Chase (Handled by Dispatcher)
- FC_UNDEF= -2 // Undefined/invalid function code

Fields:
- int fc: Function Code, default FC_VALID (0). Negative values are reserved
for internal use.
- int cc: Completion Code, default CC_NORMAL (0). Negative values are
reserved for pre-defined conditions.
- Done* done: Completion handler, default nullptr. The nullptr default causes
the Item to be deleted when posted.

### *Methods*

#### <a id="construct">pub::dispatch::Item::Item(void);</a>

The (default) constructor.

#### <a id="construct-d">pub::dispatch::Item::Item(Done* _done);</a>

Initialization constructor. Initializes: done(_done)

#### <a id="construct-fd">pub::dispatch::Item::Item(int _fc, Done* _done= nullptr);</a>

Initialization constructor. Initializes: fc(_fc), done(_done)

#### <a id="post">void pub::dispatch::Item::post(int _cc= 0);</a>

Posts the work Item completion, setting cc= _cc;

If done is specified, invokes done->done().
Otherwise, the Item is deleted.
