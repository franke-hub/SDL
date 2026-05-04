<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
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
//       ~/src/cpp/Brian/Service.md
//
// Purpose-
//       Service descriptor.
//
// Last change date-
//       2024/11/02
//
-------------------------------------------------------------------------- -->
<!-- -------------------------------------------------------------------------
-------------------------------------------------------------------------- -->

Copyright (c) 2024 Frank Eskesen.

This file is free content, distributed under cc by-sa version 4.0
with attribution required.
(See accompanying file LICENSE.BY_SA-4.0 or the original contained
within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)

----

# ~/src/cpp/Brian/Service.md

#### What is a Service?

A Service can *be* just about anything, but it needs to have a unique name so
that it can be located by those that want to use the Service.

A Service defines all of its own interfaces, so users need to know about them.

All Services have a name that's used to find it.
A Service defines its own interfaces.
It does not need to implement or use any methods provided the Service class
or its optional base classes, but might want to.

Many Services are implemented in static classes so that they can automatically
be invoked during Brian's initialization and termination phases.
Such Services must have unique names, e.g.

```
static class MyService
:  public Service("My-unique-name") /* , ... */
{  /* ... */
}  my_unique_service; // (Instantiates class MyService)
```

Aside:
Since (the production version ) of Brian includes all modules into a single
load library, including a module insures that its static definition exists.
The Service constructor handles (its own) problems related to adding the
Service's name to a static Service map during static initialization.

Brian's initialization phase starts after static initialization completes and
its termination phase starts before static termination commences.
That is, they are run after Brian::main has been invoked and before it exits,
so order of initialization and termination is less of a problem.

##### Thread-like Services

Some Services either are or operate "Thread-like" objects.
(pub::Thread is, of course, a Thread-like object.) We'll just use Thread from
here on in, knowing that "Thread-like" is also implied.
These Services, and possibly some others, want to use the "Service Manager".

##### Optional base classes

Ok, what the hey are these? Simply put, optional base classes are a mechanism
used to determine whether or not a Service provides optional methods.

The "Service Manager" makes use of this mechanism. We'll talk more about the
"Service Manager" in (perhaps excruciating) detail shortly.

There are (currently) three defined optional base classes:
- has_start, which indicates that the Service provides the start method.
- has_stop,  which indicates that the Service provides the stop  method.
- has_wait,  which indicates that the Service provides the wait  method.

NEW: THESE SERVICES ARE PURE VIRTUAL.
There are just too many ways to screw up the method signature.
- You could use const Service* instead of Service*
- etc. Anyway I found a few of them, and making them pure virtual
(Even though they were implemented) found more that weren't expected.

Technically speaking, the word "provides" is not precise. Just specifying an
optional base class provides the associated method. The declaration of the
method overrides it. This might still be unclear, but it's not as necessary to
understand how the mechanism works as it is to understand how to use it.
Usage examples will be provided shortly. They should help.

The (user-provided) start, stop, and wait methods have similar signatures:
- `virtual start(const Service*)`
- `virtual stop(const Service*)`
- `virtual wait(const Service*)`

<!-- -------------------------------------------------------------------------
The "Service manager" optionally invokes methods when Brian starts (before it
is *fully* started) or terminates (again, before it *fully*) stops.
But, the Service interface belongs to its implementer.
The methods used by the "Service Manager" do not exist in Service.h

We use optional base classes to inform the "Service Manager" that these
methods exist.
(The default version of these base classes actually do something.
They currently only (optionally) write diagnostic messages that we
don't want to see if the method isn't there.)

There are (currently) three defined optional base classes:
- has_start, which indicates that the Service supports the start method.
- has_stop,  which indicates that the Service supports the stop  method.
- has_wait,  which indicates that the Service supports the wait  method.
-------------------------------------------------------------------------- -->

A Service defines which of these classes it uses by first defining has_start,
has_stop, and/or has_wait as base classes, then providing the associated
start, stop and/or wait method.

Here's a sample which (briefly) provides all three of these optional methods.
Since the optional methods use similar signatures, this is somewhat redundant.
```
class My_sample_service
:  public Service("sample-service-name")
,  public Service::has_start
,  public Service::has_stop
,  public Service::has_wait
{
// : (Stuff not related to optional methodImplementation unrelated to wait and has_wait
protected: // Optional methods with these signatures are usually protected.
virtual void start(Service* S) {
   // This demonstrates how to invoke start's base class:
   Service::has_start::start(S);    // Invoke has_wait's base class

// : (implementation-defined start.)
}

virtual void stop(Service* S) {
   // This demonstrates how to invoke stop's base class:
   Service::has_stop::stop(S);      // Invoke has_stop's base class

// : (implementation-defined stop.)
}

virtual void wait(Service* S) {
   // This demonstrates how to invoke wait's base class:
   Service::has_wait::wait(S);      // Invoke has_wait's base class

// : (implementation-defined wait.)
}
}; // class My_sample_service
static My_sample_service my_sample_service; // (Static instantiation)
```

   // The base class currently only writes a diagnostic message.
   // Implementers usually should invoke it while debugging and consider
   // invoking it in production mode.

##### The Service Manager

The Service Manager aids in Brian's initialization and termination.
It provides three (protected) methods:
- start_all, which invokes start on all Services derived from has_start.
- stop_all,  which invokes stop  on all Services derived from has_stop.
- wait_all,  which invokes wait  on all Services derived from has_wait.

The start_all, stop_all, and wait_all are only invoked by Common.cpp.
Common.cpp invokes:
- start_all from Common::Common, Common's constructor.
- stop_all  from Common::shutdown, Brian's normal termination mechanism.
- wait_all  also from Common::shutdown, but after stop_all's invocation.

Service implementer's should note that start_all, stop_all and wait_all
do not return until *all* Services are examined.
Service::start, stop, and wait should not take much time to complete.
(We'll see how that goes in practice.)

Now that you know what (and where) the Service Manager does, it's time to
explain why it does what it does.

Let's do this tomorrow when there's more time left in the day.

The Service manager gets involved during Brian's initialization and
termination sequences.
(These run *after* static initialization and *before* static termination.)

During initialization the Service manager invokes Service::start.
A Service can start it's Thread at that time.

During termination (after the shutdown command has been invoked),
the Service manager invokes Service::stop.
This gives the Service a chance to stop its Thread.

Note that Thread does not provide a stop method.
When needed, we generally add an `operational` boolean to our Threads that the
`run` method checks each time it handles something new.

We also add a `stop` method to our Threads.
It sets `operational` false and gives the `run` method something to do.

Example:
```
class StoppableThread : public pub::Thread /* , ... */
bool operational= false; // Set true when ready to run
// : (More attributes, possibly private methods, etc
{
public:
// : (Skipping constructor, destructor, other method)
virtual void run( void )
{
   while( operational ) {
     // Wait for something to do
     // if( !operational /* (maybe with) && !dummy_work */ ) break;
     // :
     // Handle whatever happened
   }
}

void stop( void ) // Externally invokable
{
   operational= false;
   // Give run some (dummy) work to do
}

void wait( void ) // Sometimes added as an alias
{  join(); }
```

Maybe you'll even see some sort of StoppableThread added to
"~/src/cpp/inc/pub" someday. Who knows?

After all "stoppable" Services have been stopped, the Service manager invokes
wait for all "waitable" Services.

Implementation note: The Service manager starts, stops, and waits for all
Services by iterating its static Service map.
We copying the map to a list before iterating the list so, if you must,
you can remove your Service from the map pretty much at any time.

Since Services are invoked sequentially, start, stop, and wait should
not be long-running methods.
If this can't be avoided, at least write an informational message.

#### The Service interface

Services *do not need* to define start, stop, and wait methods and can even
define methods with this name that do different things.
Their






Each Service defines its own
interfaces, and has the following (optional) control methods:
  start(Service*) Invoked when the Common object is created.
  stop(Service*)  Invoked when Common::shutdown is invoked.
     It terminates the Service (so that wait will complete.)
  wait(Service*)  Invoked after all Services have been stopped.
     It waits for the stop to completely take effect.
These methods are useful when controlling Threads or similar objects.

During initialization, each Service's start method is invoked (if
  if it's a startable Service, i.e. one that provides the method.)
Termination is a two stage sequence:
  First, the stop method is invoked for each stoppable Service,
    notifying it that it should terminate.
  Next, the wait method is invoked for each waitable Service, which
    does not return until it *has* terminated.

Since Services are invoked sequentially, start, stop, and wait should
not be long-running methods.

A Service defines which of these methods are supported by specifying
has_start, has_stop, and/or has_wait as a base class.
Example: MyService supports start and stop, but not wait:
  class MyService
  :  public Service
  ,  public Service::has_start, public Service::has_stop
  {
  :
  virtual void start(const Service* S) { /* ... */ }
  virtual void stop(const Service* S) { /* ... */ }
  :
  }; // class MyService

Services are not required to support the any of the optional methods
if they don't need them, or to use the `const Service*` parameter.
The parameter is needed if you want to invoke the base class.
A stop method invokes the base class using:
  Service::has_stop::stop(S); // Where S is the `const Service*`

<!-- -------------------------------------------------------------------------
// COMMENTED OUT: Was in Service.h, is now here.
// IN PROGRESS AND MAY ALREADY BE DOCUMENTED ABOVE.
//       A Service can be just about anything. Each Service defines its own
//       interfaces, and has the following (optional) control methods:
//         start(Service*) Invoked when the Common object is created.
//         stop(Service*)  Invoked when Common::shutdown is invoked.
//            It terminates the Service (so that wait will complete.)
//         wait(Service*)  Invoked after all Services have been stopped.
//            It waits for the stop to completely take effect.
//       These methods are useful when controlling Threads or similar objects.
//
//       During initialization, each Service's start method is invoked (if
//         if it's a startable Service, i.e. one that provides the method.)
//       Termination is a two stage sequence:
//         First, the stop method is invoked for each stoppable Service,
//           notifying it that it should terminate.
//         Next, the wait method is invoked for each waitable Service, which
//           does not return until it *has* terminated.
//
//       Since Services are invoked sequentially, start, stop, and wait should
//       not be long-running methods.
//
//       A Service defines which of these methods are supported by specifying
//       has_start, has_stop, and/or has_wait as a base class.
//       Example: MyService supports start and stop, but not wait:
//         class MyService
//         :  public Service
//         ,  public Service::has_start, public Service::has_stop
//         {
//         :
//         virtual void start(const Service* S) { /* ... */ }
//         virtual void stop(const Service* S) { /* ... */ }
//         :
//         }; // class MyService
//
//       Services are not required to support the any of the optional methods
//       if they don't need them, or to use the `const Service*` parameter.
//       The parameter is needed if you want to invoke the base class.
//       A stop method invokes the base class using:
//         Service::has_stop::stop(S); // Where S is the `const Service*`
-------------------------------------------------------------------------- -->
