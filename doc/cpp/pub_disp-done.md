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
//       ~/doc/cpp/pub_disp-done.md
//
// Purpose-
//       Dispatch.h reference manual: Done, Wait
//
// Last change date-
//       2026/04/24
//
-------------------------------------------------------------------------- -->
## (pub\::dispatch\::)Done
## (pub\::dispatch\::)Wait
###### Defined in header <pub/Dispatch.h>

<!-- ===================================================================== -->
## <a id="class-done">class pub\::dispatch\::Done</a>

### *Attributes*

(None defined.)

### *Methods*

#### <a id="construct-done">Done();</a>

The (default) Done constructor.

#### virtual void done(Item* item);</a>

Handles the completion of the associated work Item. **Override this method**

<!-- ===================================================================== -->
---
### <a id="class-wait">class pub\::dispatch\::Wait : public Done</a>

### *Attributes*

`private: Event event;              // The wait/post Event

### *Methods*

#### <a id="construct-wait">Wait();</a>

The (default) Wait constructor.

#### private: virtual void done(Item* item) final;

This overrides Done\::done in order to implement Wait\::wait.

#### <a id="reset">void reset();</a>

Reset the Wait object (for reuse.)
Once posted, Wait does not wait until it's reset.

#### <a id="wait">int32_t void wait();</a>

Return value: The Item's completion code, set when Item.post is invoked
(normally by a Task.)

Wait for work Item completion.
