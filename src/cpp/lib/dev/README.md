<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
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
//       2022/10/25
//
-------------------------------------------------------------------------- -->

# ~/src/lib/dev/README.md

Copyright (C) 2022 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

----

This README describes the DEV (development) library.
This library is a development test library, intended as a placeholder for
partially developed code intended for eventual inclusion in the PUB library.

This library is currently built and tested from ~/obj/cpp/lib/dev/Test. It is
not updated using make from  ~/obj/cpp/lib/.

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
The ClientAgent and ListenAgent (both in Agent.cpp) contain polling loops.
The ClientAgent drives all connected Clients.
The ListenAgent drives both the Listen and Server.

This is the first operational distribution release with reasonable throughput.
While the HTTP/1 protocol is operational, it's still fragile. Known bugs exist.
See ~/src/cpp/lib/dev/.README for detailed status information.

While there are implementation hooks that provide client and server functions,
they currently have close to zero overhead. That is, they don't do much.
That said, the throughput performance is pretty good.
On a Fedora Linux machine running both client and server code, we're able to
process well over 50,000 I/O operations per second.
While these are HTTP/1 (unencrypted) request/response operations, connections
are only made once.
We currently only measure requests and response operations, not connects.

----
