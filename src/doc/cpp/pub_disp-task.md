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
//       ~/src/doc/cpp/pub_disp-task.md
//
// Purpose-
//       Dispatch.h reference manual: Task
//
// Last change date-
//       2023/07/28
//
-------------------------------------------------------------------------- -->
## (pub::dispatch::Task::)Task, enqueue, work(void), work(Item*)

###### Defined in header <pub/Dispatch.h>

#### class pub::dispatch::Task : public pub::Worker

Task is derived from Worker.
Task::work(void) is scheduled using WorkerPool::work() when a work Item is
enqueued to the Task when Task's AI_list was empty.

#### class pub::dispatch::LambdaTask : public pub::dispatch::Task

The LambdaTask is derived from Task.
LambdaTask contains:

```
typedef std::function<void(Item*)> function_t;
function_t             callback;
```

and Task's work(Item*) method is overridden by:

```
virtual void LambdaTask::work(Item* item) {callback(item);}
```

<!-- ===================================================================== -->
---
#### void pub::dispatch::Task::Task(void);

The default constructor.

---
#### void pub::dispatch::Task::~Task(void);

The destructor.

The destructor checks the itemList and if it's not empty, enqueues an FC_CHASE
work Item and waits for its completion.

__Note__ Care must be taken not to delete a Task while running work(Item*),
since the destructor's FC_CHASE operation will block waiting for the
running work method to complete. This can occur, for example, if the work
Item contains a shared_ptr to the Task or an object containing the Task.
(Always use `pub::dispatch::Task::post` to post such work Items.)

---
#### void pub::dispatch::Task::enqueue(Item* item);

Atomically inserts the work Item onto the Task's work queue, and schedules
the Task if necessary.
Any number of threads may simultaneously enqueue work Items.

---
#### virtual void pub::dispatch::Task::work(void);

This is the Task's (internal) Worker method.
It processes all enqueued work Items, invoking work(Item*) for each item.

---
#### void pub::dispatch::Task::work(Item* item);

This method should be overriden, or LambdaTask used instead of Task.

The application processess the work Item.
When processing is complete, the application either enqueues the work Item
onto another Task or, if Item processing is complete, invokes item->post().

The work method is single-threaded without thread affinity.
That is, each time work is invoked it runs under control of a thread
and will not be invoked again until it returns.
However, the next time work is invoked it does not necessarily run under
control of the *same* thread.

<!-- ===================================================================== -->
---
#### void pub::dispatch::LambdaTask::LambdaTask(function_t *f*);

Constructs the LambdaTask using function *f* as the work(Item*) handler.

---
#### void pub::dispatch::LambdaTask::~LambdaTask(void);

The destructor, called before pub::dispatch::Task::~Tas upon LambdaTask
deletion.
(See the important pub::dispatch::Task::~Task destructor note.)

---
#### void pub::dispatch::LambdaTask::on_work(function_t f);

Replaces the Work(Item*) handler with the specified function.

Sample usage example:
```
#include <cstdio>
#include <pub/Dispatch.h>
int main( void )
{
    using namespace pub::dispatch;

    LambdaTask lambda_task([](Item* item)
    {
      printf("Initial Item handler\n");
      item->post();
    });
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

Normally you'd specify the work Item handler once using either the constructor
or the on_work method, not both (as is in the example.)
If replacing a work Item handler, it's the handler that's active
*when the work Item is processed* that's used.
