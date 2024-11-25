<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/src/cpp/Brian/Brian.md
//
// Purpose-
//       Document Brian's interfaces and logic.
//
// Last change date-
//       2024/11/25
//
-------------------------------------------------------------------------- -->

# ~/src/cpp/Brian/Brian.md

Copyright (C) 2024 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

#### What is Brian?

Brian, firstly, is a dyslexic spelling of Brain.
That it's intent, at to be brainy. It's not there yet.

Whats inside Brian?
- A set of commands, each of which are derived from Command.h.
(Command.h documentation is currently only in Command.h.
Command.md is needed, but not started yet.)

- A set of services, each of which are derived from
[Service.h](./Service.md).
  - This documentation has been started but is a messy, bringup state.

#### What else?
There's more to say, but Brian hasn't defined itself enough yet to make
anything we might say about it useful.

Its interfaces are subject to change on a day-to-day basis,
so don't start writing code that your unwilling to rewrite tomorrow when an
interface changes.

Right now I'm documenting the Service interface and liking it more and more as
I go along. (Mostly because it's so flexible)
But the code's also changing as the doc changes.
I'm not happy that Service.md is actually required for a novice user to
write a Service, but it's either that or fill Service.h with comments that
make it appear more complex and scary than it actually is.

Right now, I'm thinking that the Command interface needs to be changed. (Maybe
renaming work to something else, like main(), maybe something more different.)
This is an inclination that's percolating; it's not ready to drink yet.)

#### 2024/11/25 (Temporary; Debugging status)
Brian has some sort of problem in termination (after the quit command) where
sometimes it completes and sometimes it doesn't.
This has been a while and debugging has not been easy.
It's still not clear where the problem is or why it occurs.
I'm going to put work on Brian aside for a while, so it's going to be
committed "as-is" with debugging statements both in Brian code and pub library
code.
Two of the run logs are included with the Brian source. They have been
partially modified to show (known) differences in the termination sequence.
Hopefully, that's where the nasty problem lies.
