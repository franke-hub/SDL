<!-- -------------------------------------------------------------------------
//
// Title-
//       Journal.md
//
// Purpose-
//       Development journal
//
// Last change date-
//       2022/06/05
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

### 2022/06/03 Don't make this stupid mistake

Because if you do, your VFT (Virtual Function Table) becomes inconsistent.
The virtual function table is an array of virtual functions that the compiler
considers invarient, whether or not it actually is.

Socket.h
```
#ifdef _GNU_SOURCE
virtual int                         // Return code (0 OK)
   ppoll(                           // Poll this Socket
     struct pollfd*    pfd,         // The (system) pollfd
     const struct timespec*
                       timeout,     // Timeout
     const sigset_t*   sigmask)     // Signal set mask
{  /* implementation */ }
#endif
```

Move your implementation to the .cpp file. A virtual function should never
be conditionally defined. You'll wind up pointing at the wrong function.
Sometimes. Sometimes not.

Yup, I actually did this. DOH.

The same sort of problem can occur if you modify the order of virtual function
declarations. A library and all users need to recompile their code to stay in
synch

### 2022/06/05

While debugging and inserting a simple statement to the latched code in
Worker.cpp, I noticed that it adversely affected the timing of TestDisp.cpp.
I moved the statistic updating outside of the latched code, using atomic
operations instead. Even though there's slightly more code there, TestDisp.cpp
now has better throughput. I also added statistics to Thread.cpp.

The ~/src/cpp/HTTP samples have been updated and are now more consistent with
TestSock.cpp. This process also served as a TestSock.cpp code inspection.
There's more to be done to clean up the samples, but they're OK for now. It's
time for a synchronization commit and getting back to the stream server,
which is still not ready for distribution. Before the trunk commit, some maint
commits will be needed for distribution verification. In particular, object
subdirectories might have excessive .gitignore'd files.

### 2022/06/05 Interesting bug

~/src/cpp/HTTP/Makefile.BSD (temporarily) contained
```
test: ;
	@echo -e "a\nb\nc\n"
```
which echoed `-e a\n\b\nc` (with the newline escapes properly handled but on
my distribution test machine wrongly including the "-e".) To remove the "-e"
this had to be changed to
```
test: ;
	@printf "a\nb\nc\n"
```
Automatic certificate generation was added along the way for no extra charge.

### 2022/06/08 Socket updated

- Removed attempts at error recovery. It's now entirely the user's
responsibility. Since it now closely matches the socket interface, it's no
longer considered experimental.
- Might want to implement some SSL_ interfaces, like SSL_set_mode. It's used
in SSL_accept but maybe should be set by the user instead. Don't know enough
yet to know what else might be needed.
- No longer track setting send/recv timeout option. The retry logic that tried
to use it was marginal at best.
- Renamed SSL_Socket to SSL_socket.
- Can now handle sockaddr* longer than host/peer_addr. (Extended size has been
coded but not tested.)
- As usual, when finding things that should have been be done better they've
been implemented.

### 2022/06/11 Conditional idiom

- `if( condition || true )` (true)
- `if( condition && true )` (condition)
<br><br>
- `if( condition || false )` (condition)
- `if( condition && false )` (false)

This idiom allows a conditional statement to be temporarily modified when
testing. Because the same number of characters are used, modifying is slightly
easier than:

- `if( condition || true )` (true)
- `if( condition || false )` (condition)
<br><br>
- `if( condition && true )` (condition)
- `if( condition && false )` (false)

When compiled using optimization, there's no runtime overhead with either
method. These coding idioms are used to quickly compare options and sometimes
left in distributed code.

### 2022/06/12 Maint commit

SocketSelect tables now start off relatively small, supporting file descriptor
numbers less than 32 can grow in stages up to the maximum allowed value. The
tables do not shrink since the storage used is reasonably small (20 bytes per
file descriptor number) and shrinkage is likely to be temporary anyway.

TestSock was switched to use SocketSelect polling. Errors found during testing
were fixed. One error was notable. The SocketSelect destructor removes the
Socket::selector field, which points to the SocketSelect object. Because of
locking considerations, this removal isn't as benign as it might first appear
and now results in a user error message that refers to the source code. A
long and complex comment was added to the source code explaining the rationale
behind the message. A short "how to fix your code" comment was also added.
See ~/src/cpp/lib/pub/Socket.cpp, method SocketSelect::~SocketSelect.

----