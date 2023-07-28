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
//       ~/src/doc/cpp/pub_disp-item.md
//
// Purpose-
//       Dispatch.h reference manual: Item
//
// Last change date-
//       2023/07/28
//
-------------------------------------------------------------------------- -->
## (pub::dispatch::Item::)Item, post

###### Defined in header <pub/Dispatch.h>

#### struct pub::dispatch::Item : public AI_list<pub::dispatch::Item>::Link

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
- fc: (int) Function Code, default FC_VALID (0). Negative values handled internally.
- cc: (int) Completion Code, default CC_NORMAL (0). Negative values are pre-defined.
- done: pub::dispatch::Done*, default nullptr (Indicated delete when done)

<!-- ===================================================================== -->
---
#### pub::dispatch::Item::Item(void);

The (default) constructor.

---
#### pub::dispatch::Item::Item(Done* _done);

Initialization constructor. Initializes: done(_done)

---
#### pub::dispatch::Item::Item(int _fc, Done* _done= nullptr);

Initialization constructor. Initializes: fc(_fc), done(_done)

---
#### void pub::dispatch::Item::post(int _cc= 0);

Posts the work Item completion, setting cc= _cc;

If done is specified, invokes done->done().
Otherwise, the Item is deleted.
