# ~/src/doc/Journal/Socket.md

Copyright (C) 2022 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

Last change date: 2022/05/15

----

This journal describes socket programming techniques that avoid problems found
in practice.

----

### 2022/05/15 Re-use of a socket connections's port pairs.

Problem: A client repeatedly makes short connections to the same server.
After about 30K connections, the client is unable to make further connections
getting the error "Address already in use."

If an client application connects to the same server application many times
in a short interval, it's possible to run into a situation where the client
fails to connect because the address port pair is already in use.

This occurs because the kernel does not immediately release the connection
information when a socket is closed. Instead, the socket goes into the
TIME_WAIT state. This logic is usually useful, since it prevents packets from
old connections interfering with new connections. However, if a server can
accept a large number of separate connections from the same client in a short
interval, it may want reset a connection rather than just closing it when
processing a client close. This is accomplished using SO_LINGER with linger
l_onoff= 1 and l_linger=0 before the close is invoked.

----

### 2022/05/15 Server listen/accept handling

Problem: It's possible for an accept operation to block. There are multiple
possible causes for this. (One of them is as a result of a client being unable
to connect because of the "Address already in use" problem described above.)
This blocked accept condition leaves a server in an unusable state.

A server can and should prevent this condition from occurring. After a listen
operation completes, a server should either use polling code such as
```
     pollfd pfd= {};
     pfd.fd= handle;                // ('handle' is the listen handle)
     pfd.events= POLLIN;
     int rc= poll(&pfd, 1, 1000);
     if( rc <= 0 ) {                // If polling error or timeout
       // Handle error condition (ignoring the listen)
       ...
     }
     client= ::accept(handle, ...); // (The accept won't block)
```
to insure that the accept won't block, or non-blocking listen/accept logic. (Using select rather than poll also works, but measured overhead was higher.)

----
