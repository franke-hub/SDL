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
