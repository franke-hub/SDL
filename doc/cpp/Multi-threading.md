<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
// SPDX-License-Identifier: CC-BY-SA-4.0
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/Multi-threading.md
//
// Purpose-
//       PUB Library, multi-threading management
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
# Multi-threading Management

The PUB library provides these multi-threading components:

| Component | Header | Description |
|-----------|--------|-------------|
| [Dispatcher](#dispatcher) | [Dispatch.h](./Dispatch.md) | Lock-free work queues: Tasks, Items, and completion handlers |
| [Thread](./Thread.md) | Thread.h | A separate unit of execution |
| [Worker](./Worker.md) | Worker.h | An interface class scheduled by the WorkerPool thread pool |
| [Latches](./Latch.md) | Latch.h | Spin latches, for short critical sections |
| [Event](./Event.md) | Event.h | Wait/post events |
| [Semaphore](./Semaphore.md) | Semaphore.h | Counting semaphore |
| [AI_list](./pub_list-AI.md) | List.h | Atomic Insert list: any number of threads may insert without locking |
| [mutex](#mutex) | mutex.h | A mutex that may be unlocked by a different thread |

<!-- ===================================================================== -->
## <a id="dispatcher">The Dispatcher</a>

The Dispatcher is a work queue system for multi-threaded programs.
It has four major components, all in namespace `pub::dispatch`:
- [Task](./pub_disp-task.md)
  - Processes Items sequentially, one at a time.
  - Runs single threaded, but the actual thread can change.
- [Item](./pub_disp-item.md)
  - Describes the operation to process and its associated data.
  - May be modified by the Task.
- [Done](./pub_disp-done.md#Done)
  - An Item's `done` field points to this.
  - Describes what happens when Item processing completes.
  - If an Item's `done` field is `nullptr`, the Item is deleted when
processing completes.
- [Wait](./pub_disp-done.md#Wait) (a subclass of Done)
  - Provides a wait method, which returns when Item processing completes.

```
*----------------*    The Task contains persistent data associated with a
|                |    work-driven function, and the function itself.
|      TASK      |
|                |--> (Atomic) list of to-do Items.
*----------------*

*----------------*    The Item contains information related to each function
|                |    invocation and completion.
|      ITEM      |
|                |--> Done/Wait/nullptr
*----------------*

*----------------*    The Done object contains the function that's driven when
|                |    Item processing completes.
|      DONE      |
|                |
*----------------*

*----------------*    The Wait object is a specialized Done object that allows
|                |    an application to wait for Item completion.
|      WAIT      |
|                |
*----------------*
```

The Dispatcher also provides a set of static
[utility functions](./pub_disp-disp.md), including timer (delay) functions.

See [Dispatch.h](./Dispatch.md) for the class and method reference.

### Application design overview

#### Items are atomically enqueued onto Tasks

- Items are added to a Task's work queue without locking.
Any number of threads may enqueue Items simultaneously.
- Enqueuing an Item onto an idle Task schedules the Task.
- Items are processed in FIFO order.
Items enqueued while the Task is running are included in that processing.
- Each Item points to the Done object that's driven when processing
completes.
If that pointer is `nullptr`, the Item is deleted when processing
completes, implementing the "fire and forget" idiom.

#### Tasks process Items sequentially

- A Task processes Items serially, one at a time.
- Applications implement `virtual void work(Item*)` either in a Task
subclass or by using a [LambdaTask](./pub_disp-lambda.md#LambdaTask).
- When a Task completes Item processing, it invokes `item->post()`.
A Task need not complete an Item, though.
It may instead enqueue the Item onto another (or even the same) Task.
- A Task runs single threaded, but the thread may change from one
invocation of `work(Item*)` to the next.
Tasks therefore cannot use thread-local storage.

#### Tasks can wait

- A running Task can do anything a running Thread can do, including wait.
- Use this with care: while a Task waits, it does not process its other
work Items.

### (Simple) sample application block diagram

```
#include <string>                   // For std::string
#include <cstdio>                   // For printf

#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Ioda.h>               // For pub::Ioda

using std::string;
typedef pub::Ioda           Ioda;
typedef pub::dispatch::Item Item;
typedef pub::dispatch::Task Task;
typedef pub::dispatch::Wait Wait;

enum
{  CC_NORMAL=   Item::CC_NORMAL
,  CC_ERROR=    Item::CC_ERROR
,  CC_ERROR_IT= Item::CC_ERROR_IT
};

*----------------*    Contains an IODA (I/O Data Area) containing the input
|      ITEM      |    data
|                |
|           IODA |    [I/O Buffer]
|           DONE |--> [WAIT]
*----------------*

struct Dev_item : public Item {
Wait           wait;
Ioda           ioda;

   Dev_item() : Item(&wait), wait(), ioda() {}
};

*----------------*    The device driver, reads from the device.
|      TASK      |
|                |
|           ITEM |-->ITEM->...->ITEM
*----------------*

struct Device : public Task {
int number= 0;

int read(Dev_item* item) // (Scaffolded)
{  item->ioda.put("Input line " + std::to_string(++number));
   if( number < 5 )
     return 0;
   return CC_ERROR;
}

virtual void
   work(Item* item)
{  Dev_item* dev_item= dynamic_cast<Dev_item*>(item);
   if( dev_item == nullptr ) {
     item->post(CC_ERROR_IT);
     return;
   }

   int cc= read(dev_item);     // Read the next input item
   dev_item->post(cc);         // Operation complete
}
};

*----------------*    Application that reads data from some device
|      APPL      |
*----------------*

int main() {
   Device device;
   for(;;) {
     Dev_item item;
     device.enqueue(&item);
     item.wait.wait();
     if( item.cc != CC_NORMAL )
       break;
     printf("%s\n", ((string)item.ioda).c_str());
   }

   return 0;
}
```

<!-- ===================================================================== -->
## <a id="mutex">pub::mutex</a>

Defined in header <pub/mutex.h>, `pub::mutex` is a replacement for
`std::mutex`. It provides the usual `lock`, `try_lock`, and `unlock`
methods, and works with `std::lock_guard`.

Unlike `std::mutex`, a `pub::mutex` is *guaranteed* to allow locking in
one thread and unlocking in another.

> **Cygwin note:** Under Cygwin, `std::mutex` has a bug that causes the
> Windows handle count to grow until no more handles can be allocated,
> at which point an exception is thrown.
> (The problem has been reported.)
> `pub::mutex` does not have this problem.

---
[Back](./REFERENCE.md)
