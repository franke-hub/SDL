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
//       ~/doc/cpp/Dispatch.md
//
// Purpose-
//       Dispatch.h reference manual (overview)
//
// Last change date-
//       2026/04/25
//
-------------------------------------------------------------------------- -->
## pub\::dispatch\::Disp
## pub\::dispatch\::Done
## pub\::dispatch\::Item
## pub\::dispatch\::Task
## pub\::dispatch\::Wait
## pub\::dispatch\::LambdaDone
## pub\::dispatch\::LambdaTask
###### Defined in header <pub/Dispatch.h>

#### Conceptual overview

The Dispatcher is a work queue system for multithreaded programs consisting
of four major components:
- (pub\::dispatch\::)Task
  - Sequentially processes Items
  - Single thread logic (but the actual thread can change.)
- (pub\::dispatch\::)Item
  - Describes the operation to process and its associated data
  - Modifiable by the Task
- (pub\::dispatch\::)Done
  - Item.done points to this
  - Discribes what happens when Item processing completes
  - Default processing (Item.done == nullptr) is to delete the Item.
- (pub\::dispatch\::)Wait (a subclass of Done)
  - Provides a wait method, which returns when Item processing completes

The Dispatcher also contains a set of static [utility functions](#Disp).

###### Implementation notes
- All dispatch objects are defined in namespace pub\::dispatch.

<!-- ===================================================================== -->
#### <a id="Disp">Disp</a>
Dispatcher utility functions. All methods are static.

##### Member functions

| Utility Methods | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-disp.md#construct) | There is no constructor. All methods are static |
| [enqueue](./pub_disp-disp.md#enqueue) | Enqueue a work Item |
| [post](./pub_disp-disp.md#post) | Post work Item |

| Timer Control Methods | Purpose |
|--------|---------|
| [delay](./pub_disp-disp.md#delay) | Schedule a delay notification |
| [cancel](./pub_disp-disp.md#cancel) | Cancel a delay notification |
| [shutdown](./pub_disp-disp.md#shutdown) | Shutdown Dispatch timer functions |

<!-- ===================================================================== -->
#### <a id="Task">Task</a>
A Dispatch work Item handler.
Handles work Items serially, in the order they were enqueued to the Task.

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-task.md#construct) | Construct a Task object |
| [enqueue](./pub_disp-task.md#enqueue) | Enqueue a work Item for this Task |
| [work(Item*)](./pub_disp-task.md#work) | Process a work Item |

<!-- ===================================================================== -->
#### <a id="Item">Item</a>
A Dispatch work Item.

##### Member attributes

| Name   | Type | Purpose |
|--------|------|---------|
| fc   | int | Function code. Negative values are reserved for internal use. |
| cc   | int | Completion code. Negative values are pre-defined. |
| done | Done* | Completion callback. If nullptr, delete the work Item. |

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-item.md#construct) | Construct a work Item object |
| [post](./pub_disp-item.md#post) | Post work Item completion |

<!-- ===================================================================== -->
#### <a id="Done">Done</a>
A Dispatch Item completion handler.
(If none, the Dispatch Item is deleted when complete.)

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-done.md#construct) | Construct a Done object |
| [done](./pub_disp-done.md#done) | Handle work Item completion |

<!-- ===================================================================== -->
#### <a id="Wait">Wait (Extends Done.)</a>

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-done.md#construct-wait) | Construct a Wait object |
| [reset](./pub_disp-done.md#reset) | Reset for reuse. |
| [wait](./pub_disp-done.m#wait) | Wait for work Item completion. |

<!-- ===================================================================== -->
#### <a id="LambdaDone">LambdaDone (Extends Done.)</a>
The done(Item*) method is implemented as a lambda function.

Types:
- typedef std\::function<void(Item*)>  Done_if; // Lambda function done interface

Fields:<br>
- protected Done_if callback;

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-lambda.md#construct-ldd) | Construct a LambdaDone object |
| [on_done](./pub_disp-lambda.md#on_done) | Replaces callback, the lambda function. |

<!-- ===================================================================== -->
#### <a id="LambdaTask">LambdaTask</a>
Extends Task, implementing work(Item*) as a lambda function.

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-lambda.md#construct-ltd) | Construct the LambdaTask object |
| [on_work](./pub_disp-lambda.md#on_work) | Specify the work Item handler. |

<!-- ===================================================================== -->
#### Example 1 (Task\::work Override)

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

<!-- ===================================================================== -->
#### Example 2 (LambdaTask and LambdaDone)

```cpp
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For pub::dispatch objects
#include <pub/Event.h>              // For pub::Event

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;
using namespace PUB::dispatch;
typedef PUB::Event Event;

int main() {
   debug_set_head(PUB::Debug::HEAD_THREAD | PUB::Debug::HEAD_TIME);
   debugh("main() invoked\n");

   Event wait;
   LambdaDone lambda_done([&wait](Item*)
   {
     debugh("LambdaDone invoked\n");
     wait.post();
     debugh("LambdaDone complete\n");
   });

   LambdaTask lambda_task([](Item* item)
   {
     debugh("LambdaTask invoked\n");
     item->post();
     debugh("LambdaTask complete\n");
   });

   Item item(&lambda_done);
   lambda_task.enqueue(&item);
   wait.wait();

   lambda_task.on_work([](Item* item)
   {
     debugh("Replacement LambdaTask work handler\n");
     item->post();
     debugh("Replacement LambdaTask work handler complete\n");
   });

   lambda_done.on_done([&wait](Item*)
   {
     debugh("Replacement LambdaDone done handler\n");
     wait.post();
     debugh("Replacement LambdaDone done handler complete\n");
   });

   debugf("\nLambda functions replaced\n");
   wait.reset();
   lambda_task.enqueue(&item);
   wait.wait();

   debugh("main() complete\n");
}
```
