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
//       ~/doc/cpp/pub_disp-lambda.md
//
// Purpose-
//       Dispatch.h reference manual: LambdaTask, LambdaDone
//
// Last change date-
//       2026/04/25
//
-------------------------------------------------------------------------- -->
## (pub\::dispatch\::)LambdaTask
## (pub\::dispatch\::)LambdaDone
###### Defined in header <pub/Dispatch.h>

The LambdaTask and LambdaDone objects are useful for creating custom Task
and Done objects without requiring derived object.

Any required data areas can be defined along with the LambdaTask or LambdaDone
object and passed by reference to the Lambda function.
(LambdaDone uses this mechanism in the example.)

<!-- ===================================================================== -->
## <a id="class-task">class pub\::dispatch\::LambdaTask : public Task</a>

### *Attributes*

`public: typedef std\::function<void(Item*)> Work_if;` // The work method interface

`protected: Work_if do_work` // The work method instance

### *Methods*

#### <a id="construct-ltd">void LambdaTask();</a>

The default the LambdaTask constructor leaves do_work uninitialized.
Uninitialized function calls throw the 'bad_function_call' std::exception.

#### <a id="construct-ltf">void LambdaTask(Work_if f);</a>

Constructs the LambdaTask using function `f` as the work(Item*) handler.

#### <a id="on_work">void pub::dispatch::LambdaTask::on_work(Work_if f);</a>

Replaces do_work with the specified lambda function.

<!-- ===================================================================== -->
## <a id="class-done">class pub\::dispatch\::LambdaDone : public Done</a>

### *Attributes*

`public: typedef std\::function<void(Item*)> Done_if;` // The done method interface

`protected: Done_if do_done` // The done method instance

### *Methods*

#### <a id="construct-ldd">LambdaDone();</a>

The default the LambdaDone constructor leaves do_done uninitialized.
Uninitialized function calls throw the 'bad_function_call' std::exception.

#### <a id="construct-ldf">LambdaDone(Done_if f);</a>

Constructs the LambdaDone using function `f` as the done(Item*) handler.

#### <a id="on_done">virtual void on_done(Done_if f);</a>

Replaces do_done with the specified lambda function.

<!-- ===================================================================== -->
### Example

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
