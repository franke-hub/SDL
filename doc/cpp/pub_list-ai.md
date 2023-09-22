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
//       ~/doc/cpp/pub_list-ai.md
//
// Purpose-
//       List.h reference manual: AI_list<T>
//
// Last change date-
//       2023/09/22
//
-------------------------------------------------------------------------- -->
## `AI_list<T>::` begin, end, fifo, get_tail, is_coherent, is_empty, is_on_list, reset(void), reset(void*)

###### Defined in header <pub/List.h>

See also: [`AI_list<T>::iterator`](./pub_list-ai_iter.md)

Types:
- `AI_list<T>::struct Link`: The Link type

`AI_list<T>` Links are implemented using
`class T : public pub::AI_list<T>::Link {...};`

---
#### iterator begin() noexcept;
Create/construct a begin() iterator.

*Implementation notes*:
- The begin iterator *removes* all elements from the AI_list,
replacing the List with a dummy item.
It also reverses the AI_list's native LIFO ordering so that the iterator
returns FIFO ordered Links.
- Applications *MUST NOT* exit an AI_list begin/end loop early, before the
begin() iterator equals end().
Doing so can result in more problems than you might imagine.

---
#### iterator end() noexcept;
Create/construct the (default) end() iterator.

---
#### pointer fifo(pointer link);
Insert *link* onto the list with FIFO (First In, First Out) ordering.

---
#### pointer get_tail(void)
Obtain the tail link

---
#### bool is_coherent(void)
(Debugging) Consistency check

---
#### bool is_empty(void)
Check whether the List is empty

---
#### bool is_on_list
Check whether the List contains a Link

---
#### pointer reset(void)
Reset (empty) the List

---
#### pointer reset(void*)
Empty the List, replacing it with a dummy Link

---

### Example:
Note that the Item added onto the list while iterating is handled by the
same iteration.


```
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
```

#### See also:

- [Dispatch](Dispatch.md) Multi-threading dispatcher
  - pub::dispatch::Item, derived from AI_list<pub::dispatch::Item>::Link.
