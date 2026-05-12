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
