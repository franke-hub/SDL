// For ~/doc/cpp/Multi-threading.md
#include <string>                   // For std::string
#include <cstdio>                   // For printf

#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Ioda.h>               // For pub::Ioda

using std::string;
typedef pub::Ioda           Ioda;
typedef pub::dispatch::Item Item;
typedef pub::dispatch::Task Task;
typedef pub::dispatch::Wait Wait;

enum
{  CC_NORMAL=   Item::CC_NORMAL
,  CC_ERROR=    Item::CC_ERROR
,  CC_ERROR_IT= Item::CC_ERROR_IT
};

struct Dev_item : public Item {
Wait           wait;
Ioda           ioda;

   Dev_item() : Item(&wait), wait(), ioda() {}
};

struct Device : public Task {
int number= 0;

int read(Dev_item* item) // (Scaffolded)
{  item->ioda.put("Input line " + std::to_string(++number));
   if( number < 5 )
     return 0;
   return CC_ERROR;
}

virtual void
   work(Item* item)
{  Dev_item* dev_item= dynamic_cast<Dev_item*>(item);
   if( dev_item == nullptr ) item->post(CC_ERROR_IT);

   int cc= read(dev_item);     // Read the next input item
   dev_item->post(cc);         // Operation complete
}
};

int main() {
   Device device;
   for(;;) {
     Dev_item item;
     device.enqueue(&item);
     item.wait.wait();
     if( item.cc != CC_NORMAL )
       break;
     printf("%s\n", ((string)item.ioda).c_str());
   }

   return 0;
}
