<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2022-2026 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
// SPDX-License-Identifier: CC-BY-4.0
//----------------------------------------------------------------------------
//
// Title-
//       Debugging.md
//
// Purpose-
//       Document difficult to debug problems.
//
// Last change date-
//       2026/03/16
//
-------------------------------------------------------------------------- -->

Copyright &copy; 2022-2026 Frank Eskesen.

This file is free content, distributed under cc by-sa version 4.0 with
attribution required.
(See accompanying file LICENSE.BY_SA-4.0 or the original contained within
https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)

----

This journal records problems that were particularly difficult to debug.

- [\[Thread start syncronization timing bug"\]](#start_drive_timing_bug)
- [\[SSH Fails "kex_exchange_identification"\]](#kex_exchange_id)
- [\[Too Many Open Sockets\]](#too-many-sockets) <br/>

----

# <a id="start_drive_timing_bug">Thread completes before it finishes starting</a>

## **The problem**

The (~/src/cpp/lib/pub/Test/)TimeDisp.cpp associated library source files were
modified to allow the WorkerPool size to be modified by applications.
After doing this and running with a worker pool size of two, TimeDisp would
sometimes abort or not run to completion.
The reason for this was not obvious, except that it was likely to be some sort
of timing bug.

Multithreading timing bugs are notoriously difficult to debug.
This one was no exception to that rule.

## **Adding  trace file logging obscured the problem**

Using gdb debugging allowed us to see the state after a failure, but not its
cause.
We activated debugging tracef file event logging, but then the problem didn't
recur.
This is not unusual for multithreading problems since file writing changes
event sequencing.

Event tracing in memory is a low overhead alternative.
When backed by a memory mapped file, this transient information isn't lost.
Both Linux and Cygwin implentation are extremely well implemented.
Even when a process aborts, the file matches the current storage state.

We already had memory tracing functions implemented in Trace.cpp, but needed
more trace points.
The TimeDisp test application was modified to (optionally) create the memory
mapped trace table, and event tracing was added.
Additional event tracing was added to library code, notably Trace.cpp and
Worder.cpp.
The problem was became reproducable, but we didn't get enough information.
Thus began a sequence of adding more internal trace points and correcting
possible problems until, finally, the actual problem was discovered.
Adding a trace of Thread::start()'s exit gave us the final clue needed.

## **An initial red herring**

Thinking Thread.cpp was solid, We examined Latch.cpp, looking for errors.
We didn't find any, but we did clean up the code so that all lock methods
use the same try_lock retry mechanism.

We also had some minor improvements to some of the try_lock mechanisms.

## **Implementation Background**

In Thread.cpp, method start() creates a pthread, invoking method drive().
We already knew that start needed to wait before it exited.
Method drive initializes a Thread Local Storage Struct (TLSS) used to keep
some of the Thread state.
We already traced the Thread's constructor and destructor as well as the
destructor's exit, (after which the Thread object cannot be referenced.)

## **Internal trace analysis**

A gdb backtrace after an abort sometimes found a WorkerThread method after
an abort, implicating Worker.cpp, Thread.cpp, and Dispatch.cpp.
With optimized parameters, the TimeDisp test application drives the
Dispatch::Task's work() method at a rate of over 30 million operations per
second.

### What trace analysis showed for a failure event:

We were actually kind of lucky because one failure resulted in a duplicate
free() function call.
This was lucky because it caused an an abort, immediately terminating tracing.

We have an Event object implements the wait/post interface.
It contains an int32_t post code, a std::condition_variable, and a
std::mutex protecting the condition variable.
The wait method waits for the post method to be invoked from a different
thread.
The Event object resides in the TLSS.

The Thread under consideration is a WorkerThread, used to drive a work method.

WorkerThread is defined and implemented in Worker.cpp.
The Worker interface contains nothing but the (virtual) work method.

It contains a pointer to the Worker.

The trace sequence:
- WorkerPool::work invocation (Using a new or reused WorkerThread)
  - The WorkerPool is empty, so a new WorkerThread is created
  - The WorkerThread constructor invokesThread::start
- Thread::start invocation: (Starts the Thread, invoking the run method)
  - start allocates the TLSS, putting its address in the Thread object
  - start uses pthread_create to create a pthread.
    - The pthread handle is saved in the TLSS
    - Thread::drive is the pthread's initial method (It's static)
    - The Thread object is the initial method's parameter
  - start waits for a drive_initialized Event, to be posted by drive<br>
We can't know exactly when start's wait begins.
Thread::drive runs asynchonously.
- Thread::drive invocation:
  - The Thread and TLSS addresses are copied to stack storage
  - A thread-local pointer, tl_tlss is initialized<br>
Note that start can't initialize this. It's not in the same pthread.
  - The Thread's state (in the TLSS) is set to FSM_DRIVE
  - The drive_initialized Event is posted
  - (We update some statistics)
  - Thread::run is invoked, completing Thread::start's stated mission
    - (This eventually results in a Worker::work invocation)
- WorkerThread::run invocation:
  - Worker::work is invoked
- Lots of other unrelated events occur.
- === Under WorkerThread run control
  - WorkerThread Worker::work completes
    - WorkerThread::done is invoked (the WorkerPool is full)
      - WorkerThread::stop is invoked, marking the thread non-operational
  - WorkerThread::run continues and deletes itself
  - The Thread destructor is invoked
  - The destructor issues a Thread::detach, which issues pthread_detach.
  - The Thread destructor exits. (The Thread object storage is deleted.)
  - The run method exits, Thread::drive continues
  - Thread.cpp does not reference Thread object storage (This condition has been
considered.)<br>
It does, however, reference the thread local storage (which is where the
detached state of the Thread was recorded.)
  - Thread.cpp DELETES THE TLSS
  - Thread.cpp continues, exiting the drive method and completing the pthread
  - Uh-oh! We saw the TLSS delete but NOT Thread::start()'s exit, and its
wait object is in the thread local storage we just deleted.

## **The fix**

Once the problem was known, the fix was relatively easy.

- An additional Event, named start_completed, was added to the TLSS.
- This event is posted by Thread::start as it's about to complete.
Once posted, Thread::start doesn't reference the Thread or TLSS again.
- Thread::drive waits for this event before driving the run method<br>
Once Thread::drive's wait completes, the TLSS Events are never referenced.

## **Problem "autopsy" report**

### Why did this problem occur?

When the WorkerThread was created, the WorkerPool must have been empty
(or a WorkerPool Thread would have been reused.)

When the WorkerThread was done, the WorkerPool must have been filled to
capacity (or it would have been added to the WorkerPool list.)

But, the same situation could have occured for *any* self-deleting thread.
And, we'd like to (and, with this fix, now do) support a WorkerPool of size 0.

### Why didn't we find this problem earlier?

We don't have many instances of Threads that self-delete and, before TimeDisp,
didn't have any that were capable of stress testing.

### Additional testing with a WorkerPool size of zero
We started a TimeDisp test to run for an hour with pool size==0.
This completed without error on Fedora Linux.

On CYGWIN, however, we got errors after about 16 minutes:
- terminate called recursively
- terminate called recursively after throwing an instance of std::system_error.

Analyzing this problem, we noticed that Window's Task Manager reported a
continuously growing handle count.
We deduced that this occurred because Threads were being created faster than
they were completing.
Thread::start created new Threads that were starting but not completing before
Thread::start exited.

To fix this, we added a Semaphore that limits the number of concurrent Threads.
While this did not control handle growth, TimeDisp with a runtime of four
hours ran without error.

**That did not fix the problem**

TimeDisp now ran for about seven and a half hours before failing.

I created a new test `~/src/cpp/Test/Test_pthread.cpp` to duplicate Thread.cpp
operation while minimizing pub library usage.
This used the same sequencing logic between starting a thread and running it,
duplicating and using the pub Event logic.
It had the same Cygwin handle growth problem.

On a hunch (and since I don't understand the underlying mechanisms used by
std::condition_variable and std::mutex,) I created a new Event type,
Event_yield, which uses an atomic variable and uses std::this_thread::yield()
as its wait mechanism.
I replaced the (mutex/condition_variable) Event with the Event_yield, and the
handle growth problem disappeared in the test.

It did not disappear when using the pub library code.
The pub library code used another (mutex/condition_variable) Event in
Worker.cpp's WorkerThread, where it waits for a reuse/termination signal.

Changing Worker.cpp to use a std::mutex rather than an Event did not solve the
problem either.

Further modifications to ~/src/cpp/Test/Test_pthread.cpp to isolate the
problem showed that the problem occurs for any allocated object that contains
a std::mutex, as is still the case in Worker.cpp's WorkerThread (and Event.h)

This is a Cygwin implementation bug. A bug report was sent to the Cygwin
mailing list.

To work around this problem, set the worker pool size large enough to avoid
WorkerThread allocation.
Note that pub::System::debug() provide relevants information.
TimeDisp.cpp invokes pub::System::debug when option --verbose=2 (or higher) is
specified.

----

# <a id="kex_exchange_id">SSH fails reporting "kex_exchange_identification" error.</a>
## **Initial (obscure) fix**
**in which we describe a "trip down the rabbit hole," fixing a problem without
really understanding its cause.**

See [**Key Exchange Fix**](#key_exchange_fix) for a rabbit hole free problem
and solution description.

......

I have a LAN containing multiple physical and virtual machines.

After switching routers on my LAN, I ran into a strange issue:
**SSH from Windows to Fedora stopped working**, even though:

- All machines could ping each other
- Fedora → Windows SSH worked fine
- Windows → Windows SSH also worked
- Fedora could SSH everywhere

But when I tried to SSH from Windows (OpenSSH) to Fedora, I consistently got
this error:

kex_exchange_identification: read: Software caused connection abort banner
exchange: Connection to 192.168.50.xxx port 22: Software caused connection
abort

Meanwhile, Fedora logged this:

Connection reset by 192.168.50.xxx port [some port]

### The Fix

**I had to manually change Window's IP subnet mask from `255.255.255.0` to
`255.255.255.255`.**

Yes, really.

After that change, SSH to Fedora worked immediately.

### How I Found It

I have two virtual Ubuntu machines
- Ubuntu[0] could SSH everywhere
- Ubuntu[1] failed in the same way the Windows machines failed.

I compared the network settings on the two Ubuntu machines.
The only significant network difference was the subnet mask.

Setting the mask to `255.255.255.255` (which normally means no subnet )
_somehow_ fixed the handshake failure, even on the Windows machines.

### Why This Works

Maybe it was because my network configuration changed recently:

I did recently change my physical router.
The old one had a network address of '192.168.0.1', and
the new one uses the network address '192.168.50.1'.

I also have a virtual box virtual adapter on my windows machine
(which runs all my VirtualBox virtual machines)
has a network adapter of '192.168.56.1'.

My fedora machine also has a virtual box virtual adapter (virbr0) which has a
network adapter of 192.168.122.1.
It hasn't had any virtual machines configured for a long time.

#### But the truth is, I don't know what causes this problem or why the fix works.

But if you're seeing this error *and everything else seems fine*, try this.

### How to fix it.

#### On the router itself

[(Reference)](https://www.digitalcitizen.life/change-subnet-mask-windows-10/)
See: "How to change the Subnet Mask from the router's interface"

I didn't actually see this until writing this note and can't vouch for it,
but it looks like maybe I should've tried this first.

#### Windows 10 or 11 with manual IP

1. Go to `Control Panel > Network and Internet > Network Connections`
2. Right-click your active adapter → Properties
3. Select **Internet Protocol Version 4 (TCP/IPv4)** → Properties
4. Set:
   - IP address: your manual IP
   - Subnet mask: `255.255.255.255`
   - Gateway and DNS as needed

Apply, save, and try SSH again.

#### Windows using DHCP

[(Reference)](https://www.digitalcitizen.life/change-subnet-mask-windows-10/)
See: How to change the Subnet Mask in Windows using PowerShell

- Open an administrator Windows PowerShell prompt
- Use "Get-NetAdapter -physical" to list your network adapters.
Each adapter is associated with an ifIndex (Interface Index.)
- Use "Set-NetIPAddress -InterfaceIndex {number} -PrefixLength 32", which sets
your subnet mask to 255.255.255.255.

(Maybe there's more to it than this because the change didn't persist after a
reboot.)
Because the change didn't persist, I changed my DHCP Windows machine to static
IP and used the manual method instead.

#### Ubuntu with manual IP

- Use the "Edit Connections" menu from network conections menu on the top of
the display screen.
- Select the network to change
- Change the netmask to 32 (indicating 32 bits, or 255,255,255,255)

#### Fedora with manual IP

- nmcli connection show (Displays the device names)
- sudo ifconfig {device} netmask 255.255.255.255
- ifconfig ## Verify the change

#### Reboot test

After making network changes on any machine, reboot to make sure they still
work. If there are problems, now's the time to fix them.

------------------------------------------------------------------------------

## <a id="key_exchange_fix">Key Exchange Fix

### The actual problem
During the network update, the Fedora machine subnet mask was (somehow)
changed from 255.255.255.0 (a 24-bit mask) into 255.255.255.255 (a 32-bit
mask.)

The fix was to change the mask back to 255.255.255.0, its original value.
This would have been simpler if the actual problem was determined before other
subnet masks were also changed.

So the net is: **Make sure that *all* devices communicating using SSH use the
same subnet mask.**

----

# <a id="too-many-sockets">Too Many Open Sockets</a>

At this writing, ~/src/cpp/lib/pub/Test/TestSock.cpp's HTTP stress test ran a
server under a separate thread, and a client that sent a request, read the
response, then closed the connection. On Linux, this stress test would run
for several seconds and then the client connection would fail with the error
"Cannot assign requested address." Furthermore, the server would not complete
normally. It had to be interrupted.

I approached the problem by going after the server problem first. Server's that
don't *always* shut down properly need fixing. I added a bunch of debugging
traceh() statements, trying to figure out exactly where the program was stuck.
I narrowed it down to ~/src/cpp/lib/pub/Socket.cpp, in Socket::accept. The
::accept wasn't completing. Furthermore, the logic in
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

Use this sample code before closing your socket:

```
    struct linger option;
    option.l_onoff= 1;
    option.l_linger= 0;
    int rc= setsockopt(handle, SOL_SOCKET, SO_LINGER, &option, sizeof(option));
    if( rc != 0 ) { /* Replace this comment with your error recovery procedure */ }
```

Error recovery code is optional.
(Aside from writing an error message, there's really not a lot you can do.)

----
