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
