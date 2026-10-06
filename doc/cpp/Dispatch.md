<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2026 Frank Eskesen.
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
//       ~/doc/cpp/Dispatch.md
//
// Purpose-
//       Dispatch.h reference manual (overview)
//
// Last change date-
//       2026/09/30
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Dispatch.h>
## pub::dispatch

The Dispatcher is a lock-free work queue system for multi-threaded programs.
See [Multi-threading Management](./Multi-threading.md#dispatcher) for a
conceptual overview and application design notes.

Usage notes:
- All Dispatch objects are defined in namespace `pub::dispatch`.
- A Task processes its work Items one at a time, in the order they were
enqueued. Any number of threads may enqueue Items without locking.
- A work Item completes when its `post` method is invoked. If the Item has a
Done object, that object's `done` method is then invoked. Otherwise, the Item
is deleted.

** <a id="Dispatch_classes">Dispatch Classes</a> **
| Class             | Description
| :---------------- | :--------------------------------------------------- |
| [Disp](#Disp) | Static utility and timer functions |
| [Task](#Task) | Work Item handler |
| [Item](#Item) | Work Item |
| [Done](#Done) | Work Item completion handler |
| [Wait](#Wait) | Done object that waits for work Item completion |
| [LambdaTask](#LambdaTask) | Task with a lambda function work Item handler |
| [LambdaDone](#LambdaDone) | Done with a lambda function completion handler |

<!-- ===================================================================== -->
#### <a id="Disp">[Disp](./pub_disp-disp.md)</a>
All Disp methods are static. There is no constructor.

| Method | Purpose |
|--------|---------|
| [cancel](./pub_disp-disp.md#cancel) | Cancel a delay. |
| [delay](./pub_disp-disp.md#delay) | Complete a work Item after a delay. |
| [enqueue](./pub_disp-disp.md#enqueue) | Enqueue a work Item onto a Task. |
| [post](./pub_disp-disp.md#post) | Complete a work Item using an internal Task. |
| [shutdown](./pub_disp-disp.md#shutdown) | Terminate delay processing. |

<!-- ===================================================================== -->
#### <a id="Task">[Task](./pub_disp-task.md)</a>
| Method | Purpose |
|--------|---------|
| [destructor](./pub_disp-task.md#destructor) | Delete the Task, after all enqueued Items complete. |
| [enqueue](./pub_disp-task.md#enqueue) | Enqueue a work Item onto this Task. |
| [is_running](./pub_disp-task.md#is_running) | Test: is the Task (instantaneously) running? |
| [work](./pub_disp-task.md#work) | Process a work Item. **Override this method.** |

<!-- ===================================================================== -->
#### <a id="Item">[Item](./pub_disp-item.md)</a>
| Attribute | Type | Purpose |
|-----------|------|---------|
| [fc](./pub_disp-item.md#fc) | int | Function code. Negative values are reserved. |
| [cc](./pub_disp-item.md#cc) | int | Completion code. Negative values are pre-defined. |
| [done](./pub_disp-item.md#done) | Done* | Completion handler. If nullptr, the Item is deleted when posted. |

| Method | Purpose |
|--------|---------|
| [constructor](./pub_disp-item.md#constructor) | Create a work Item. |
| [post](./pub_disp-item.md#post) | Complete the work Item. |

<!-- ===================================================================== -->
#### <a id="Done">[Done](./pub_disp-done.md#Done)</a>
| Method | Purpose |
|--------|---------|
| [done](./pub_disp-done.md#done) | Handle work Item completion. **Override this method.** |

<!-- ===================================================================== -->
#### <a id="Wait">[Wait](./pub_disp-done.md#Wait)</a>
Wait extends Done.

| Method | Purpose |
|--------|---------|
| [reset](./pub_disp-done.md#reset) | Reset the Wait object for reuse. |
| [wait](./pub_disp-done.md#wait) | Wait for work Item completion. |

<!-- ===================================================================== -->
#### <a id="LambdaTask">[LambdaTask](./pub_disp-lambda.md#LambdaTask)</a>
LambdaTask extends Task, implementing `work(Item*)` as a lambda function.

| Method | Purpose |
|--------|---------|
| [constructor](./pub_disp-lambda.md#LambdaTask-constructor) | Create the LambdaTask, optionally specifying its work handler. |
| [on_work](./pub_disp-lambda.md#on_work) | Replace the work handler. |

<!-- ===================================================================== -->
#### <a id="LambdaDone">[LambdaDone](./pub_disp-lambda.md#LambdaDone)</a>
LambdaDone extends Done, implementing `done(Item*)` as a lambda function.

| Method | Purpose |
|--------|---------|
| [constructor](./pub_disp-lambda.md#LambdaDone-constructor) | Create the LambdaDone, optionally specifying its completion handler. |
| [on_done](./pub_disp-lambda.md#on_done) | Replace the completion handler. |

<!-- ===================================================================== -->
#### Examples

Task subclass, overriding `work(Item*)`:

```cpp
#include <cstdio>
#include <pub/Dispatch.h>

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::dispatch;

class MyTask : public PUB::dispatch::Task {
public:
   MyTask( void ) = default;

virtual void
   work(Item* item) override
{
   printf("MyTask item handler\n");
   item->post();
}
}; // class MyTask

int main() {
   printf("main() invoked\n");

   MyTask task;
   Wait wait;
   Item item(&wait);
   task.enqueue(&item);
   wait.wait();

   printf("main() complete\n");
}
```

For a LambdaTask and LambdaDone example, see
[LambdaTask and LambdaDone](./pub_disp-lambda.md#example).

For a larger example, see the
[sample application](./Multi-threading.md#dispatcher) in Multi-threading
Management.
