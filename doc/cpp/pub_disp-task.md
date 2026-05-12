<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2026 Frank Eskesen.
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
//       ~/doc/cpp/pub_disp-task.md
//
// Purpose-
//       Dispatch.h reference manual: Task
//
// Last change date-
//       2026/04/18
//
-------------------------------------------------------------------------- -->
## (pub::dispatch::)Task
###### Defined in header <pub/Dispatch.h>

<!-- ===================================================================== -->
## <a id="class-task">class pub\::dispatch\::Task : public pub\::Worker</a>

A Task serially processes Items in its work queue in single-threaded fashion,
although the actual Thread may vary between Items.
Any number of Threads can simultaneously add work Items to the work queue
without locking. Items are processed in the order they are enqueued.

### *Attributes*

protected: AI_list<Item> itemList; // The work Item list.

### *Methods*

#### <a id="construct-done">void Task();</a>

The (default) constructor.

#### <a id="destruct">void ~Task();</a>

The destructor.

Before exiting the destructor examines the itemList and, if it's not empty,
enqueues an FC_CHASE work Item and waits for its completion.
This sequence prevents the destruction of a Task with outstanding work Items.

__Note__ Care must be taken not to delete a Task while running work(Item*),
since the destructor's FC_CHASE operation will block waiting for the
running work method to complete. This can occur, for example, if the work
Item contains a shared_ptr to the Task or to an object containing the Task.
(Use `Disp\::post` rather than `Item\::post` to complete such work Items.)

#### <a id="enqueue">void enqueue(Item* item);</a>

Atomically inserts the work Item onto the Task's work queue and schedules
the Task if necessary.
Any number of threads may simultaneously enqueue work Items.

#### private: virtual void work() final;

This overrides Worker::work(). It's used by Dispatch to process Items,
invoking work(Item*) for each Item.

#### <a id="work">protected: virtual void work(Item* item);</a>

See also: The [LambdaTask](pub_disp-lambda.md#class-task) provides an
alternative coding mechanism for implementing the work function.

This method should be overridden.

This method is invoked by the dispatcher to process a work Item.
When processing is complete, the application either enqueues the work Item
onto another Task or, if Item processing is complete, invokes item->post().

The work method is single-threaded but runs without thread affinity.
That is, each time work is invoked it runs under control of a thread
and will not be invoked again until it returns.
However, each time work is invoked it the underlying thread may differ.
