#include <assert.h>                 // For assert
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For size_t

#include <pub/Dispatch.h>           // For namespace PUB::dispatch
#include <pub/List.h>               // For PUB::AI_list, ...
#define PUB _LIBPUB_NAMESPACE

struct Item : public pub::AI_list<Item>::Link {
size_t                 value;
   Item(size_t v) : value(v) {}
}; // class Item

int main() {
   pub::AI_list<Item> list;

   Item one(1);
   Item two(2);
   Item meaning(42);
   Item more(732);

   assert( list.fifo(&one) == nullptr); // Add onto empty list
   assert( list.fifo(&two) == &one);
   assert( list.fifo(&meaning) == &two);

   size_t index= 0;
   for(auto ix= list.begin(); ix != list.end(); ++ix) {
     switch(index++) {
       case 0:
         assert( ix->value == 1 );
         break;

       case 1:
         assert( ix->value == 2 );
         list.fifo(&more);
         break;

       case 2:
         assert( ix->value == 42 );
         break;

       case 3:
         assert( ix->value == 732 );
         break;

       default:
         printf("SHOULD NOT OCCUR\n");
     }
   }

   assert( index == 4 && list.get_tail() == nullptr );
   printf("NO errors\n");
   return 0;
}
