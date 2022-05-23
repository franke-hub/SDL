# ~/src/doc/Journal/Nasties.md

Copyright (C) 2022 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

Last change date: 2022/05/15

----

This journal records problems that were particularly difficult to debug.

----

### 2022/05/15

At this writing, ~/src/cpp/lib/pub/Test/TestSock.cpp's HTTP stress test ran a
server under a separate thread, and a client that sent a request, read the
response, then closed the connection. On Linux, this stress test would run
for several seconds and then the client connection would fail with the error
"Cannot assign requested address." Furthermore, the server would not complete
normally. It had to be interrupted. This problem did not occur on Windows.[1]

I approached the problem by going after the server problem first. Server's that
don't *always* shut down properly need fixing. I added a bunch of debugging
traceh() statements, trying to figure out exactly where the program was stuck.
I narrowed it down to ~/src/cpp/lib/pub/Socket.cpp, in Socket::listen. The
::accept following the ::listen wasn't completing. Furthermore, the logic in
TestSock.cpp to stop the StreamServer thread didn't work. Closing both the
listener socket and the client socket still left the ::accept in limbo.

More and more diagnostic tracing eventually found that there was some sort of
problem with port re-use. The accept would fail after about 30K operations,
and the last working connection port would be about the same as the first.

Looking for solutions I found:
- https://stackoverflow.com/questions/12565112/how-to-break-c-accept-function,
suggesting using select before accept to insure that the accept will be
accepted. This prevents a server hang, but not the client connection failures.
- https://stackoverflow.com/questions/3757289/when-is-tcp-option-so-linger-0-required,
describing when it's appropriate to use socket option SO_LINGER. There is some
discussion and debate among the responses about the question, leaning towards
avoid using it if possible.

However, a server can detect when a client closes a socket normally or there
is some sort of non-recoverable error. Using SO_LINGER in this instance allows
connection re-use in this normal case, so the client won't get a connection
failure the next time the same address/port pair is re-used. After a normal
close, the client doesn't get any indication when the connection is reset so
it should be safe to use then. After a transmission error, in-flight messages
are in limbo anyway so the SO_LINGER reset doesn't add any extra client
recovery complexity.

Note that only a linger with l_onoff= 1 and l_linger=0 prevents the socket from
going into TIME_WAIT state.

The code used to diagnose the problem is included below. (This is a cleaned up
version. There were a lot more debugging statements used while the actual
problem and its solution were less clear.) The code will probably be removed
from the source shortly since it clutters the logic.

```
//----------------------------------------------------------------------------
//
// Method-
//       Socket::listen (in ~/src/cpp/lib/pub/Socket.cpp)
//
// Purpose-
//       Listen for and accept new connections
//
//----------------------------------------------------------------------------
Socket*                             // The new connection Socket
   Socket::listen( void )           // Get new connection Socket
{  if( HCDM )
     debugh("Socket(%p)::listen handle(%d)\n", this, handle);

   int rc= ::listen(handle, SOMAXCONN); // Wait for new connection
   if( rc != 0 ) {                  // If listen failure
     if( IODM ) {
       trace(__LINE__, "%d= listen()", rc);
       display_ERR();
     }
     return nullptr;
   }

   // Accept the next connection
   int client;
   for(;;) {
     /* IMPLEMENTATION NOTE: *************************************************
     A problem occurs in Test/TestSock --stress, where clients only use
     connections for one HTTP operation. After about 30K operations clients
     fail to connect and the server ::accept operation does not complete.

     Problem 1) The server accept operation blocks.
     It's only this part of the problem that we can address here. The options
     tried are coded below. Options 0 and 1 rely on the client to fix the
     problem and options 2 and 3 prevent the accept from blocking. We don't
     want to leave Socket operations in an unrecoverable blocked state if it's
     reasonably avoidable. Option 3 only has about a 0.5% overhead over the
     entire HTTP operation sequence, and is the implementation chosen.

     Problem 2) Clients fail to connect.
     This occurs because sockets are left in the TIME_WAIT state after close,
     and the rapid re-use of ports exhausts the port space. In a server, this
     can only be fixed using SO_LINGER with linger l_onoff=1 and l_linger=0
     to immediately close its half of the socket. It only needs to do this
     when it detects a client close or a transmission error, so there's no
     associated client recovery required.

     We implement this SO_LINGER logic in TestSock's StreamServer::serve
     method. With that logic and StreamServer::stop's normal recovery logic,
     TestSock --stress runs properly with any of the options below.

     Implementation options are coded below.
     ************************************************************************/

// ===========================================================================
#define ACCEPT_OPTION 3
#define LISTEN_HCDM false

#if false                           // (Used for option verification)
static int once= true;
     if( once ) {
       once= false;
       debugf("%4d HCDM ACCEPT_OPTION(%d)\n", __LINE__, ACCEPT_OPTION);
     }
#endif

#if ACCEPT_OPTION == 0
     // Do nothing...
     //
     // The client fails to connect after about 30K operations and the ::accept
     // operation hangs.
     //
     // 6025 ops/second Timing w/TestSock USE_LINGER == true

#elif ACCEPT_OPTION == 1
     // Add a short time delay
     //
     // The client fails to connect after about 30K operations and the ::accept
     // operation hangs.
     //
     // 3200 ops/second Timing w/TestSock setting SO_LINGER option

     usleep(125);

#elif ACCEPT_OPTION == 2
     // Use select to insure that the accept won't block.
     //
     // The client fails to connect after about 30K operations.
     // The server does not see the client's failing connection attempts.
     // With TestSock's StreamServer::stop method disabled, the select times
     // out, and the "::accept would block" path is driven.
     //
     // With the stop method enabled, the listener socket is closed well before
     // the select timeout. In this instance (for some unknown reason) select
     // returns 1, so the accept fails with "Bad file descriptor" because it's
     // using the CLOSED handle.
     //
     // 5825 ops/second Timing w/TestSock USE_LINGER == true

     struct timeval tv= {};
     tv.tv_usec= 1000000;

     fd_set rd_set;
     FD_ZERO(&rd_set);
     FD_SET(handle, &rd_set);
     int rc= select(handle+1, &rd_set, nullptr, nullptr, &tv);
     if( LISTEN_HCDM )
       traceh("%4d %d=select(%d) tv(%zd,%zd) %d:%s\n", __LINE__, rc, handle+1
             , tv.tv_sec, tv.tv_usec, errno, strerror(errno));
     if( rc == 0 ) {                // If timeout
       if( IODM )
         debugh("%4d %s ::accept would block\n", __LINE__, __FILE__);
       return nullptr;
     }

#elif ACCEPT_OPTION == 3
     // Use poll to insure that the accept won't block.
     //
     // The client fails to connect after about 30K operations.
     // The server does not see the client's failing connection attempts.
     // When the poll times out (or fails) the accept is simply skipped,
     // and nullptr returned (indicating a transient error.)
     //
     // 6000 ops/second Timing w/TestSock USE_LINGER == true

     pollfd pfd= {};
     pfd.fd= handle;
     pfd.events= POLLIN;
     int rc= poll(&pfd, 1, 1000);   // 1 second timeout (1000 ms)
     if( LISTEN_HCDM )
       traceh("%4d %d=poll() {%.4x,%.4x}\n", __LINE__, rc
             , pfd.events, pfd.revents);
     if( rc <= 0 ) {                // If polling error or timeout
       if( IODM ) {
         if( rc < 0 )
           trace(__LINE__, "%d= poll()", rc);
         else
           debugh("%4d %s ::accept would block\n", __LINE__, __FILE__);
       }
       return nullptr;
     }
#endif // ====================================================================

     if( LISTEN_HCDM && false  )
       traceh("%4d HCDM accept\n", __LINE__);
     peer_size= sizeof(peer_addr);
     client= ::accept(handle, (sockaddr*)&peer_addr, &peer_size);
     if( LISTEN_HCDM )
       traceh("%4d HCDM(%d) %d %d,%d accepted %d %d:%s\n", __LINE__, handle
             , ACCEPT_OPTION, get_host_port(), get_peer_port(), client
             , errno, strerror(errno));

     if( client >= 0 )              // If valid handle
       break;

     if( handle < 0 )               // If socket is currently closed
       return nullptr;              // (Expected)

     if( errno != EINTR ) {         // If not interrupted
       if( IODM )
         errorp("listen [accept]");

       return nullptr;
     }
   }

   // NOTE: Copy constructor only copies host_addr/size and peer_addr/size.
   Socket* result= new Socket(*this);
   result->handle= client;
   if( IODM )
     trace(__LINE__, "%p[%d]= listen", result, client);

   return result;
}
```

[1] The problem does not occur on Windows (Cygwin) probaby because it only
completes about 120 operations/second. By the time a port would be reused,
it's already exited TIME_WAIT state and available.

----
