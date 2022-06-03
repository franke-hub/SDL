<!-- -------------------------------------------------------------------------
//
// Title-
//       Socket.md
//
// Purpose-
//       Document socket programming and usage techniques
//
// Last change date-
//       2022/06/02
//
-------------------------------------------------------------------------- -->

# ~/src/doc/Journal/Socket.md

Copyright (C) 2022 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

----

This journal describes socket programming and usage techniques used to avoid
practical problems.

----

### If your code works, but not across machines

- First, check your firewalls.
Most likely it's the server's firewall disallowing an inbound socket, but
it's also possible that outbound (client) restrictions exist.
- Most routers handle UDP (packets) without any required setup. Check whether
yours is an exception, especially if using multi-cast.

----

### 2022/05/15 Re-use of a socket connection's port pairs.

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
l_onoff= 1 and l_linger=0 before the close is invoked. This immediately
terminates the connection, avoiding the TIME_WAIT before the socket address
pair can be reused.

Sample code:
```
    struct linger option;
    option.l_onoff= 1;
    option.l_linger= 0;
    int rc= setsockopt(handle, SOL_SOCKET, SO_LINGER, &option, sizeof(option));
    if( rc != 0 ) { /* Replace comment with your error recovery procedure */ }
```

----

### 2022/05/15 Server accept handling

Problem: It's possible for an accept operation to block. There are multiple
possible causes for this. (One of them is as a result of a client being unable
to connect because of the "Address already in use" problem described above.)
This blocked accept condition leaves a server in an unusable and unstoppable
state *when a client and server reside on the same machine*. The server is 
unable to immediately complete a connect operation to complete its blocked
accept.

A server can prevent the blocked condition from occurring. Before the accept
operation, a server should either use polling code such as
```
     pollfd pfd= {};
     pfd.fd= handle;
     pfd.events= POLLIN;
     int rc= poll(&pfd, 1, 3000);   // Replace this ms timeout as needed
     if( rc <= 0 ) {                // If polling error or timeout
       // Handle timeout or error condition ...
     }
     client= ::accept(handle, ...); // (The accept won't block)
```
to insure that the accept won't block or non-blocking listener sockets.

----

