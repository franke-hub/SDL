<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
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
//       ~/doc/cpp/Multi-threading.md
//
// Purpose-
//       PUB Library, multi-threading management
//
// Last change date-
//       2026/04/30
//
-------------------------------------------------------------------------- -->
# Multi-threading Management: Dispatcher

### Conceptual overview

See also: [Implementation details](./Dispatch.md)

The Dispatcher is a work queue system for multithreaded programs consisting
of four major components:
- (pub\::dispatch\::)[Task](#task)
  - Sequentially processes Items
  - Single thread logic (but the actual thread can change.)
- (pub\::dispatch\::)[Item](#item)
  - Describes the operation to process and its associated data
  - Modifiable by the Task
- (pub\::dispatch\::)[Done](#done)
  - Item.done points to this
  - Discribes what happens when Item processing completes
  - Default processing (Item.done == nullptr) is to delete the Item.
- (pub\::dispatch\::)[Wait](#wait) (a subclass of Done)
  - Provides a wait method, which returns when Item processing completes

```
*----------------*    The Task contains persistent data associated with a
|                |    work-driven function, and the function itself.
|      TASK      |
|                |--> (Atomic) list of to-do Items.
*----------------*

*----------------*    The Item contains information related to each function
|                |    invocation and completion.
|      ITEM      |
|                |--> Done/Wait/NULL
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

The Dispatcher also contains a set of static [utility functions](#Disp).

<!-- ===================================================================== -->
### Application design overview

#### Items are atomically enqueued onto Tasks

- Lock-free (Compare_exchange) additions to the Task's Item List
  - Items can be simultaneously added by **any number** of actors
  - Adding an Item to an empty list used to schedule processing
  - Items are added in reverse order
- Begin inverts the order; Items are processed in FIFO order
- Items added to the list during begin/end processing included in iteration
- Items contain a pointer to the Done object which implements the function
driven when processing completes.
  - If the pointer is null, the Item is deleted on completion. (Implementing
the "Fire and Forget" idiom.)

Each begin/end iteration runs under the same Thread, but the Thread may differ
between iterations.

#### Tasks process Items sequentially in FIFO order

Each begin/end iteration runs under the same Thread, but the Thread may differ
between iterations. (Tasks cannot use thread-local storage.)

- Tasks process Items serially, one at a time
  - Tasks operation in single Thread mode but do not use a fixed Thread.
  - Applications implement `virtual void work(Item*)` using either a subclass
or a `LambdaTask.`
  - When a Task completes Item processing, it invokes `Item->done().`
  - Tasks do not necessarily complete Item processing. They may enqueue the
Item on another (or even the same) Task.

#### Tasks can wait

- A running Task can do anything a running Thread can do
- Use with care: while waiting, Tasks DO NOT process other work Items.

<!-- ===================================================================== -->
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
   if( dev_item == nullptr ) item->post(CC_ERROR_IT);

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
## <a id="Disp">Disp: static utility functions</a>

### Timer functions

The dispatcher provides a set of timing functions:

#### void* delay(double seconds, Item item)

- Return value: A token that can be used to cancel the delay
- seconds: The number of seconds to delay
- item: The Item posted when the delay completes (or is cancelled)

#### bool cancel(void* token)

- Return value: `true` if and only if the token was still active
- token: The return value from delay

Applications can determine whether or not an Item was successfully cancelled
using:
- The return code from cancel
- The Item's completion code
  - CC_NORMAL: The delay expired
  - CC_PURGE: The delay was cancelled

#### void shutdown()

Cancels all pending delay operations and terminates the (internal) Timer
Thread.

While it is not *necessary* for an application to shutdown the Timer Thread,
no Dispatcher Timer cleanup occurs when a process completes.
Process cleanup does delete the Timer Thread and any storage associated with
unexpired delay requests.

### Other functions

#### void enqueue(Task* task, Item* item)

Applications should generally use `task->enqueue(item)` instead. This method
uses internal tracing not normally active in production distributions.

#### void post(Item* item, cc= 0)

The Item is posted under control of a (hidden) Dispatch Task.

This feature is useful under certain circumstances.

Consider an Item containing a shared_ptr reference to the running Task.
When the Task destructor discovers that the Task is running, it issues a
CHASE operation and waits for it to complete, insuring that all queued Items
have completed.
If posting the Item removes the last shared_ptr (deleting the running Task,)
a livelock occurs because the CHASE is blocked by the running Task.

Moving the post operation to a different task prevents this livelock.
