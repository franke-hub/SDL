#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For pub::dispatch objects
#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;

int main() {
   debug_set_head(PUB::Debug::HEAD_THREAD | PUB::Debug::HEAD_TIME);
   debugh("main() invoked\n");

   using namespace PUB::dispatch;
   LambdaTask task([](Item* item) {
     debugh("LambdaTask invoked\n");
     item->post();
     debugh("LambdaTask complete\n");
   });

   Wait wait;
   Item item(&wait);
   task.enqueue(&item);
   wait.wait();

   debugh("main() complete\n");
}
