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
//       ~/src/doc/cpp/pub_disp-done.md
//
// Purpose-
//       Dispatch.h reference manual: Done, Wait
//
// Last change date-
//       2023/07/16
//
-------------------------------------------------------------------------- -->
## (pub::dispatch::)Done::Done, Done::done; LambdaDone::LambdaDone, LambdaDone::done, LambdaDone::on_done; Wait::Wait, Wait::done, Wait::reset, Wait::wait

###### Defined in header &lt;pub/Dispatch.h&gt

<!-- ===================================================================== -->
---
### *pub::dispatch::Done methods*
#### pub::dispatch::Done::Done(void);

The (default) Done constructor.

---
#### virtual void done(Item* item);

**Override this method**
Handle the completion of the associated work Item.

<!-- ===================================================================== -->
---
### *pub::dispatch::LambdaDone attributes*

`typedef std::function&lt;void(Item*)&gt; function_t;` // The lambda function signature

`protected: function_t callback` // The lambda function instance

### *pub::dispatch::LambdaDone methods*

#### pub::dispatch::Wait::LambdaDone(void) : public Done;

The (default) LambdaDone constructor.
There is no default lambda function. It's left undefined.

---
#### pub::dispatch::Wait::LambdaDone(function_t _cb) : public Done;

An initializing LambdaDone constructor, defining the lambda function.

---
#### virtual void pub::dispatch::LambdaDone::done(Item* item);

For internal use only.
This overrides Done::done. It just invokes the callback lambda function.

You provide the done functionality either in the constructor or
by using the on_done method.

---
#### virtual void pub::dispatch::LambdaDone::on_done(function_t _cb);

Replaces the callback lambda function.

<!-- ===================================================================== -->
---
### *pub::dispatch::Wait methods*
#### pub::dispatch::Wait::Wait(void) : public Done;

The (default) Wait constructor.

---
#### virtual void pub::dispatch::Wait::done(Item* item);

(For internal use only.
This overrides Done::done and implements Wait::wait.)

---
#### void pub::dispatch::Wait::reset(void);

Reset the Wait object (for reuse.)
After wait completion, the Wait object cannot be reused unless it is reset.

---
#### void pub::dispatch::Wait::wait(void);

Wait for work Item completion.
