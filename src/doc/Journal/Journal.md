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
//       Journal.md
//
// Purpose-
//       Development journal
//
// Last change date-
//       2023/03/05
//
-------------------------------------------------------------------------- -->

# ~/src/doc/Journal/Journal.md

Copyright (C) 2022-2023 Frank Eskesen.

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

I added a connect(std\::string) method to Socket.h to make it easier to connect
using the LAN, which also makes it easier to run clients and servers on
different machines. I did run into one nasty error which deserves special
mention. It's the first one documented in './Nasties.md'.

### 2022/05/18

Completed implementation of ~/src/cpp/lib/pub/TestSock.cpp, a datagram and
HTTP protocol stress tester.

Implemented pub\::SocketSelect class in Socket.h/Socket.cpp, used by
TestSock.cpp --datagram stress test.

### 2022/05/19

Note that SocketSelect is marked "NOT THREAD SAFE" in the implementation
notes. It actually *is* thread-safe since all operations are mutex locked.
It's not *usable* by multiple threads because a select operation blocks any
update to SocketSelect while it's in progress. Select operations can have an
arbitrarily long timeout.

I looks like pub\::SocketSelect can be greatly improved, working more like
Linux epoll in non-Linux environments. The Socket lookup can be made easier
and quicker by having the list of Sockets indexed by the file descriptor. We
can add an additional Socket, owned by SocketSelect, that can be part of every
poll request. With this, when a change to the polling list needs to be made,
it can be instantiated quickly. Also, we can separately maintain a (locked)
updated polling list which would be instantiated at the next
SocketSelect\::select operation. When the updated polling list is created, we
signal the change by writing a simple message on the owned socket, completing
any active poll operation.

We can also cross-correlate the SocketSelect and the Socket. A Socket can
contain a pointer to the SocketSelector so that it can automatically be
removed from the Socket list on deletion. We may need some sort of sequence
validation to be sure that a Socket on the file descriptor indexed list
hasn't been replaced during a poll event.

We can also add some sort of Socket callback function instead of the polling
select. This would would be a std\::function residing in the Socket, and we
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
- In general, using \::poll rather than \::select got better throughput. Select
operations are not used in Socket.cpp. SocketSelect\::select uses \::poll, which
can handle more sockets anyway. (Maybe SocketSelect\::select should be renamed
to SocketSelect\::poll.)
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
considers invariant, whether or not it actually is.

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
Socket\::selector field, which points to the SocketSelect object. Because of
locking considerations, this removal isn't as benign as it might first appear
and now results in a user error message that refers to the source code. A
long and complex comment was added to the source code explaining the rationale
behind the message. A short "how to fix your code" comment was also added.
See ~/src/cpp/lib/pub/Socket.cpp, method SocketSelect\::~SocketSelect.

### 2022/09/02 Maint commit

Made pub library source and include files more consistent.
The dev library files were also updated, but they are also being updated
for other reasons and are not ready to be released.

### 2022/09/12

Finally added the trivial List\::insert(after,link) and List\::remove(link)
method (rather than requiring head == tail) use a single link.

Normal profiling doesn't account for task switching.
The dispatcher timing test in ~/src/cpp/lib/pub/Test/TestDisp.cpp now takes
about 22 seconds to run on a machine where it used to take about 5 seconds.
Windows shows no such regression.
Normal profiling doesn't show anything unexpected.
The latest Fedora update brings the elapsed time down to 16 seconds.

In order to investigate (later) why this is happening, I added a new
dev/Recorder.h / Recorder.cpp function that provides data collection from
generic sensors to be added to a Recorder pool.

Working on the http library.
There are some things that need to be resolved.

I want to have multiple client application instances active at the same time.
This multiple simultaneous client-server connections, which means that the
Client map can't simply use the Server's sockaddr_u as an index by the
ClientAgent.
There may be a need for multiple connection indexes, with each index pointing
to something containing a list of possible Clients and the actual Client
resolution requiring the host and peer sockaddr_u objects.

Each application instance requires that its requests are processed in sequence,
but applications will want to queue multiple requests.
An application can insure that requests are transmitted in sequence by
binding to a particular Client/Server connection.
How does the ClientAgent manage these multiple ClientServer connections?
Do we share connections, allowing multiple applications to access each one?
Maybe not, at least to start.

Connections can take a long time to complete, requiring multiple steps and
possibly upgrading from HTTP/1 to HTTP/2 along the way.
The ClientAgent has to provide this functionality, and probably requires
a dispatch\::Task and Worker threads for this purpose.

When terminating a connection, we might want to keep the structure in place
for a few seconds to allow it to be reused without requiring the complete
connection process.

Servers should probably listen for both ipv6 and ipv4 connections.
Clients should probably try both ipv6 and ipv4 addresses before rejecting
a connection.

The ClientAgent and ServerAgent need to manage the Select polling.
They need to be able to use the sockaddr_u pair to locate the associated
Client or Server given the Socket that can process data.
They need to adjust the polling event mask depending upon their current
state, always using non-blocking I/O requests.

When receiving data, Clients and Servers cannot quickly tell whether or not
more data is required to for response and request completion.
As a first implementation pass, all data received will be enqueued
to the client or server application.
Read polling will always be active, but write polling will only be used when
a transmission blocks.

### 2022/10/16

Commit: Client uses asynchronous polling. Also includes Diagnostic.h and
Recorder.h commits.

We now use Ioda (Input/Output Data Area) rather than Data for buffering.
This features minimal data copying when moving data between components,
and when discarding leading data.

While the Windows throughput is essentially unchanged, the Linux throughput
has dramatically regressed. This needs to be fixed but I also wanted to
synchronize the maint and trunk branches.

#### inc/dev/Recorder.h
Used for generic statistic recording, reporting, and resetting.
Recorders are inserted into or removed from a global table. The recorded
information is displayed on demand.

#### inc/pub/Diagnostic.h
Contains debugging diagnostic objects.
- Pristine, used to check for "wild stores" clobbering objects. A Pristine
object is (temporarily) placed before and after an object suspected of
being somehow clobbered by wild stores.
- namespace std::pub_diag, used for shared_ptr tracking. Useful for finding
shared_ptr instances. This uses conditional macros to redefine make_shared,
shared_ptr, and weak_ptr so most code is unchanged. It also defines and
conditionally activates INS_DEBUG_OBJ(x) and REM_DEBUG_OBJ(x) macros to
be placed in objects containing shared pointers of interest.

Unless activated, namespace std::pub_diag has no overhead. When activated,
objects and their shared_ptrs can be displayed at any time using the
static method std::Debug_ptr::debug. This turned out to be useful and was
probably quicker to implement than tracking down one instance where a
shared_ptr<Stream> was not being cleared. (This also happened to be a
memory leak of an object not managed by a shared_ptr.)

While (C++11) template<T,U> dynamic_pointer_cast(shared_ptr<U>) is implemented,
the other associated pointer_casts are not used in the dev library and
are not implemented both because they weren't needed and couldn't be properly
tested.

### 2022/10/24 Trunk commit ~/src/cpp/lib/dev HTTP/1 operational

Commit: Both Client and Server use asynchronous polling.

The HTTP server remains in development. Known bugs exist, and only HTTP/1
without encryption is currently supported.

Cygwin throughput is best when only one Client/Server pair is used, i.e.
`T_Stream --server --stress=1` This needs investigation.

Linux throughput has dramatically improved. This version matches
../pub/Test/TestSock throughput when four client threads are used and
scales linearly at least up to ten clients. (Test termination problems
currently preclude using more clients than that.)

I originally went looking for a red herring when investigating this problem,
adding timing event recording to the Client/Server path. Doing this, I found
that the client and server thread clocks were not closely synchronized.
In fact, the Server clock (in both Cygwin and Linux) ran about .15 seconds
behind the Client clock. The Server would seem to receive a request nearly
.15 seconds before it was sent and the Client would receive its (asynchronous)
response more than .15 seconds after it was sent.

Investigation showed that yes, different CPUs (on different threads) can be
out of synch. I didn't find anything indicating that the synchronization
error would be this large. While considering writing a clock synchronization
thread (which the kernel should be able to generally do without actually
needing a thread) the red herring began to stink.

Linux top and the gnome-system-monitor both showed that essentially *no*
multi-threading occurred, so I switched to implementing Server polling.
This requires multi-threading both when servicing requests and when receiving
reponses. With a few glitches here and there this, proved easy to do using
Client.cpp to model the changes needed in Server.cpp.
More server multi-threading fixed the throughput issue, and polling reduced
the overhead compared to ~/src/cpp/lib/pub/TestSock.cpp.

I'm temporarily leaving the timing code (in ~/src/cpp/lib/dev/Global.cpp and
../../inc/dev/Global.h) and spread throughout the HTTP/1 send/receive path.
The Global code will be moved to ~/src/cpp/lib/.OBSOLETE/dev rather than
simply discarded but the recording hooks will be removed.
If you're interested in these later, use gitk to look at today's version.

### 2022/10/26 Next steps

- Harden pub library code (static deconstruction.)
- Drive dev library code error paths
- Cleanup
  - Remove pub/Buffer (it's been replaced by dev/Ioda)
  - Remove dev/Global (moving code to .OBSOLETE subdirectory)
  - General dev library code inspection.
- Investigate Cygwin performance anomalies running ~/obj/dev/Test/T_Stream

### 2022/11/17 Maint commit

I wanted to update the dev library to connect for each operation similarly
to TestSock. This has proven to be more difficult than expected. The commit
has code to test this change, but it's not driven by default.

While some error paths have been tested, the dev library is still fragile.
While the standard stress test usually works, it's not at 100 percent.
The new connection per operation code is not just fragile, it's broken.
Static debugging doesn't find the stress test problems and gdb isn't always
helpful. More memory trace debugging is required.

### 2022/11/19 Trunk/maint commit (push)

The associated code was published yesterday, 11/18, largely fixing the worst
(but not all of) the dev library problems.

The major problem was in ~/src/lib/pub/Select::remove(). A socket remove would
be enqueued and almost immediately deleted. If a poll was also outstanding, the
Select's socket could point at freed storage.

### 2022/12/18

Library change information moved to ~/src/cpp/lib/pub/ and ~/src/cpp/lib/dev as
appropriate. We'll use this Journal for status of a more general nature.

### 2022/01/11

Paused working on HTTP client/server for a bit, instead updating editxcb so
that the status line wasn't hidden by the history line.
After completing that update, also updated ~/src/cpp/Util/worder.cpp splitting
off worder.hpp which improves and isolates the Dictionary function.
While it's true that the Dictionary was on the back burner, updating it added
significant testing time for the updated editxcb.

### 2022/02/02

The java-based golfer programs (in ~/src/java/Webapp/usr/fne/golfer/* and
~/src/java/Webapp/usr/fne/applet/*) were updated.
The installation procedure was improved but probably not to the point where
it could be used without additional documentation.

These programs needed to be updated because all graphic programs were derived
from JApplet, and we could no longer find a browser that supported applets.
They are now derived from JFrame.

This took longer than expected because these programs hadn't been updated in
quite a while and many of the language nuances needed to be re-learned and
some new ones were discovered.

The rest of February and the beginning of March have been mostly reserved for
non-programming activities.
Grandkids are turning 8 and 10, and the golfer application will get some use.

### 2023/04/05

March was mostly spent getting and recovering from a severe cold.
(Testing ruled out Covid, Flu-A, Flu-B or RSV.)

Testing ~/src/cpp/lib/pub/Test/TestDisp.cpp showed a significant but unexpected
Linux throughput performance improvement. (Windows performance didn't change.)
This was most likely due to changes in the Linux kernel and runtime library.

----
