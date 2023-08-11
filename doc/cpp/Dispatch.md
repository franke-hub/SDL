<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/Dispatch.md
//
// Purpose-
//       Dispatch.h reference manual (overview)
//
// Last change date-
//       2023/08/11
//
-------------------------------------------------------------------------- -->
## pub::Dispatch
\#include <pub/Dispatch.h>

### Dispatch objects
Dispatch objects are defined in namespace pub::dispatch.

<!-- ===================================================================== -->
---
#### Disp
Dispatcher utility functions. All methods are static.

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-disp.md) | There is no constructor. All methods are static |
| [cancel](./pub_disp-disp.md) | Cancel a delay notification |
| [delay](./pub_disp-disp.md) | Schedule a delay notification |
| [post](./pub_disp-disp.md) | Post work Item utility |
| [enqueue](./pub_disp-disp.md) | Enqueue a work Item |
| [shutdown](./pub_disp-disp.md) | Shutdown Dispatch timer functions |

<!-- ===================================================================== -->
---
#### Done
A Dispatch Item completion handler.
(If none, the Dispatch Item is deleted when complete.)

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-done.md) | Construct a Done object |
| [done](./pub_disp-done.md) | Handle work Item completion |

<!-- ===================================================================== -->
---
#### LambdaDone (Extends Done.)
The done() method is a lambda function.
<br>
__TODO__ Appears unused. If so, candidate for removal.

Types:
- typedef std::function<void(Item*)> function_t; // Lambda function work handler

Fields:<br>
- protected function_t callback;

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-done.md) | Construct a LambdaDone object |
| [done](./pub_disp-done.md) | (Invokes callback, the lambda function.) |
| [on_done](./pub_disp-done.md) | Replaces callback, the lambda function. |

<!-- ===================================================================== -->
---
#### Wait (Extends Done.)

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-done.md) | Construct a Wait object |
| [done](./pub_disp-done.md) | (Implementation defined.) |
| [reset](./pub_disp-done.md) | Reset for reuse. |
| [wait](./pub_disp-done.md) | Wait for work Item completion. |

<!-- ===================================================================== -->
---
#### Item
A Dispatch work Item.

##### Member attributes

| Name   | Type | Purpose |
|--------|------|---------|
| fc   | int | Function code. Negative values are handled internally. |
| cc   | int | Completion code. Negative values are pre-defined. |
| done | Done* | Completion callback. If nullptr, delete the work Item. |

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-item.md) | Construct a work Item object |
| [done](./pub_disp-item.md) | Handle work Item completion |
| [post](./pub_disp-item.md) | Post work Item completion |

<!-- ===================================================================== -->
---
#### Task
A Dispatch work Item handler.
Handles work Items serially, in the order they were enqueued to the Task.

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-task.md) | Construct a Task object |
| [enqueue](./pub_disp-task.md) | Enqueue a work Item for this Task |
| [work(void)](./pub_disp-task.md) | Implement Worker interface |
| [work(Item*)](./pub_disp-task.md) | Process a work Item |

<!-- ===================================================================== -->
---
#### LambdaTask
Extends Task, implementing work(Item*) as a lambda function.

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_disp-task.md) | Construct the LambdaTask object |
| [(destructor)](./pub_disp-task.md) | Delete the LambdaTask object |
| [on_work](./pub_disp-task.md) | Specify the work Item handler. |


#### Example
```
#include <cstdio>
#include <pub/Dispatch.h>
int main( void )
{
    using namespace pub::dispatch;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    LambdaTask lambda_task([](Item* item)
    {
      printf("Initial Item handler\n");
      item->post();
    });
#pragma GCC diagnostic pop
    Wait wait;
    Item item(&wait);
    lambda_task.enqueue(&item);
    wait.wait();

    lambda_task.on_work([](Item* item)
    {
      printf("Replacement Item handler\n");
      item->post();
    });
    wait.reset();
    lambda_task.enqueue(&item);
    wait.wait();
}
```

__TODO__ Figure out why GCC 13.1.1 gives a "maybe-uninitialized" error
and GCC 11.4.0 doesn't. (Both versions execute correctly.)

Normally you'd specify a work Item handler once using either the constructor
or the on_work method, not both (as is in the example.)
If replacing a work Item handler, it's the handler that's active
*when the work Item is processed* that's used.
