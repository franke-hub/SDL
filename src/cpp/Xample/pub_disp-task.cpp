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
