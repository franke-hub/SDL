<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/src/lib/dev/README.md
//
// Purpose-
//       DEV library description
//
// Last change date-
//       2023/04/24
//
-------------------------------------------------------------------------- -->

# ~/src/lib/dev/README.md

Copyright (C) 2022-2023 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

----

This README describes the DEV (development) library.
This library is a development test library, intended as a placeholder for
partially developed code intended for eventual inclusion in the PUB library.

This library is currently built in ~/obj/cpp/lib/dev and tested from
~/obj/cpp/lib/dev/Test/.
The dev library may also updated by using make from ~/obj/cpp/lib/.

----

This library currently contains HTTP client and server code in the style
used by NODE.js modules.
The eventual goal is to provide both an HTTP/1 client/server as well as
an HTTP/2 client/server.

This code makes heavy use of the pub library, including Debug.h, Dispatch.h,
Select.h, Socket.h, Thread.h, and Trace.h functions.

Select.h contains a polling control mechanism created primarily to support
this library.

----

#### Status

The Client (in Client.cpp), Listen (in Listen.cpp), and Server (in Server.cpp)
all operate asynchronously, driven from polling loops.
The polling loops reside in Agent.cpp (objects ClientAgent and ListenAgent.)

The ClientAgent polling loop drives the Client objects and
the ListenAgent polling loop drives both the Listen and Server objects.

This is the first operational distribution release with reasonable throughput.
While the HTTP/1 protocol is operational, it's still somewhat fragile.
Known bugs exist.
(See ~/src/cpp/lib/dev/.README for detailed status information.)

On a Fedora Linux machine running both client and server code, we're able to
complete well over 50,000 request/response operations per second.

However, we only count HTTP/1 (unencrypted) request/response operations.
Each Client creates a connection at the beginning of the test sequence and
reuses that connection thereafter.

Fixes were added to correct request/response opertions when each
sequence requireds a connect operation.
(This is done using T_Stream's --major=1 option.)

While these fixes allow simple unit testing and short stress testing
(with improved performance;) however, it's unstable when stress tested.

----
