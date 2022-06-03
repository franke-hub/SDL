<!-- -------------------------------------------------------------------------
//
// Title-
//       Journal.md
//
// Purpose-
//       Development journal
//
// Last change date-
//       2022/06/02
//
-------------------------------------------------------------------------- -->

# ~/src/doc/Journal/Journal.md

Copyright (C) 2022 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

----

This journal is a record of "the distribution": a set of libraries, programs,
and associated documentation.

----

### 2022/05/15

The current high level goal is to develop a C++ HTTP1/2 stream client/server
package loosely based on Nodejs's javascript package.

This is under development in ~/src/cpp/Dirty/http, and is not a visible part
of the distributed code. It's in a messy state because SSL socket usage is not
well understood. ~/src/cpp/HTTP contains sample SSL socket programs using
openssl/bio.h and openssl/ssl.h. These currently don't run well, but I thought
fixing them would improve my understanding of socket and SSL. This got too
confusing with the openssl interfaces.

There wasn't a current basic socket test in the pub library, so the next step
was to build one. The resulting program is
~/src/cpp/lib/pub/Test/TestSock.cpp, which is a simple HTTP client/server
stressor and datagram socket test. The com library supports datagram sockets,
so the pub library should too.

I added a connect(std::string) method to Socket.h to make it easier to connect
using the LAN, which also makes it easier to run clients and servers on
different machines. I did run into one nasty error which deserves special
mention. It's the first one documented in './Nasties.md'.

### 2022/05/18

Completed implementation of ~/src/cpp/lib/pub/TestSock.cpp, a datagram and
HTTP protocol stress tester.

Implemented pub::SocketSelect class in Socket.h/Socket.cpp, used by
TestSock.cpp --datagram stress test.

### 2022/05/19

Note that SocketSelect is marked "NOT THREAD SAFE" in the implementation
notes. It actually *is* thread-safe since all operations are mutex locked.
It's not *usable* by multiple threads because a select operation blocks any
update to SocketSelect while it's in progress. Select operations can have an
arbitrarily long timeout.

I looks like pub::SocketSelect can be greatly improved, working more like
Linux epoll in non-Linux environments. The Socket lookup can be made easier
and quicker by having the list of Sockets indexed by the file descriptor. We
can add an additional Socket, owned by SocketSelect, that can be part of every
poll request. With this, when a change to the polling list needs to be made,
it can be instantiated quickly. Also, we can separately maintain a (locked)
updated polling list which would be instantiated at the next
SocketSelect::select operation. When the updated polling list is created, we
signal the change by writing a simple message on the owned socket, completing
any active poll operation.

We can also cross-correlate the SocketSelect and the Socket. A Socket can
contain a pointer to the SocketSelector so that it can automatically be
removed from the Socket list on deletion. We may need some sort of sequence
validation to be sure that a Socket on the file descriptor indexed list
hasn't been replaced during a poll event.

We can also add some sort of Socket callback function instead of the polling
select. This would would be a std::function residing in the Socket, and we
could use Worker (pool) threads to drive them. This could be implemented as a
derived class of SocketSelect (say SocketDriver or SocketThread) that would do
all the polling.

### 2022/05/23

Updated SocketSelect lookup logic. It uses the indexing described above which
actually needed a file descriptor to pollfd index also. The separate socket
owned by SocketSelect for poll interruption isn't implemented yet. It uses
getrlimit to determine the maximum number of open files and allocates both
the file descriptor indexed Socket* array and file descriptor to pollfd entry
array using that maximum size. No reallocation is needed; tables can hold the
the maximum number of file descriptors (and therefore the maximum possible
file descriptor index.)

~/src/cpp/HTTP/SampleBIO.cpp and ~/src/cpp/HTTP/SampleSSL.cpp are operational,
but not fully tested. They haven't been tested using multiple browsers,
leaving the bug_1000 logic untested.

Both SampleBIO and SampleSSL were failing because they didn't use the
SO_LINGER reset logic needed for address/port pair re-use. SampleSSL was
originally coded to use host address 127.0.0.1 rather than the host name. The
Socket.cpp library routing changed so that the host's network address is used
by default, so that had to be accounted for also.

Synchronization commit: As usual miscellaneous changes are also included.

Not included: TODO: ~/src/cpp/HTTP/Sample* and ~/src/cpp/lib/pub/TestSock.cpp
have lots of common features. They should be synchronized so that each one
has the structure of the others.

### 2022/06/02

Socket.cpp/TestSock.cpp experimental results running short stress tests:
- A 5 second test was more than enough. Operations/second throughput remained
essentially unchanged when a 24 hour test was used.
- In general, using ::poll rather than ::select got better throughput. Select
operations are not used in Socket.cpp. SocketSelect::select uses ::poll, which
can handle more sockets anyway. (Maybe SocketSelect::select should be renamed
to SocketSelect::poll.)
- The number of stress test client threads has interesting effects,
particularly in the PacketClient stress test.
  - For PacketClient, when the client and server are on the same machine an
extra thread hinders rather than help. The increase in lost packets is larger
than the increase in sent packets. When run on different machines the number
of successful round-trip completions more than doubles. Presumably this is
because one client can be sending while the other is receiving and the server
isn't overloaded. With three client threads throughput decreases.
  - For StreamClient more clients resulted in more throughput up to a point.
After that point there was no appreciable difference for a large range of 
client threads, then throughput slowly decreased.

With a large number of clients, the server thread count reaches the WorkerPool
maximum. This might be why StreamClient throughput decreased at all.

----