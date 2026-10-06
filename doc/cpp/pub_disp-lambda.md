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
//       ~/doc/cpp/pub_disp-lambda.md
//
// Purpose-
//       Dispatch.h reference manual: LambdaTask, LambdaDone
//
// Last change date-
//       2026/09/30
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Dispatch.h>
## pub::dispatch::LambdaTask
## pub::dispatch::LambdaDone

LambdaTask and LambdaDone create custom Task and Done objects without
requiring a derived class.
Any data the lambda function needs can be defined along with the LambdaTask
or LambdaDone object and captured by reference.
(LambdaDone uses this mechanism in the [example](#example).)

A LambdaTask or LambdaDone created without a lambda function has no handler.
If a work Item is processed or completed before a handler is specified, a
`std::bad_function_call` exception is thrown.

<!-- ===================================================================== -->
## <a id="LambdaTask">pub::dispatch::LambdaTask</a>

LambdaTask extends [Task](./pub_disp-task.md), implementing
`work(Item*)` as a lambda function.

Types:
- `typedef std::function<void(Item*)> Work_if;` The work handler interface.

---
#### <a id="LambdaTask-constructor">pub::dispatch::LambdaTask::LambdaTask();</a>
#### pub::dispatch::LambdaTask::LambdaTask(Work_if f);

Creates the LambdaTask, using function `f` (if specified) as the
`work(Item*)` handler.

---
#### <a id="on_work">void pub::dispatch::LambdaTask::on_work(Work_if f);</a>

Replaces the `work(Item*)` handler with function `f`.

<!-- ===================================================================== -->
## <a id="LambdaDone">pub::dispatch::LambdaDone</a>

LambdaDone extends [Done](./pub_disp-done.md#Done), implementing
`done(Item*)` as a lambda function.

Types:
- `typedef std::function<void(Item*)> Done_if;` The done handler interface.

---
#### <a id="LambdaDone-constructor">pub::dispatch::LambdaDone::LambdaDone();</a>
#### pub::dispatch::LambdaDone::LambdaDone(Done_if f);

Creates the LambdaDone, using function `f` (if specified) as the
`done(Item*)` handler.

---
#### <a id="on_done">void pub::dispatch::LambdaDone::on_done(Done_if f);</a>

Replaces the `done(Item*)` handler with function `f`.

<!-- ===================================================================== -->
### <a id="example">Example</a>

```cpp <!-- ~/src/cpp/Xample/pub_disp-lambda.cpp -->
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

Sample Output:

```
1777049709.805 <@000000000000> main() invoked
1777049709.806 <@000a00037070> LambdaTask invoked
1777049709.806 <@000a00037070> LambdaDone invoked
1777049709.806 <@000a00037070> LambdaDone complete
1777049709.806 <@000a00037070> LambdaTask complete

Lambda functions replaced
1777049709.806 <@000a00037070> Replacement LambdaTask work handler
1777049709.806 <@000a00037070> Replacement LambdaDone done handler
1777049709.806 <@000a00037070> Replacement LambdaDone done handler complete
1777049709.806 <@000a00037070> Replacement LambdaTask work handler complete
1777049709.806 <@000000000000> main() complete
```
