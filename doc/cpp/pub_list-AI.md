<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2025 Frank Eskesen.
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
//       ~/doc/cpp/pub_list-AI.md
//
// Purpose-
//       List.h reference manual: AI_list<T>
//
// Last change date-
//       2025/10/05
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/List.h>
## `pub::AI_list<T>`

See also: [`AI_list<T>::iterator`](./pub_list-AI_iter.md)

Types:
- `AI_list<T>::struct Link`: The Link type

`AI_list<T>` Links are implemented using
`class T : public pub::AI_list<T>::Link {...};`

---
#### <a id="begin">iterator begin() noexcept</a>
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
#### <a id="end">iterator end() noexcept</a>
Create/construct the (default) end() iterator.

---
#### <a id="fifo">pointer fifo(pointer link)</a>
Insert *link* onto the list with FIFO (First In, First Out) ordering.

---
#### <a id="get_tail">pointer get_tail(void)</a>
Obtain the tail link

---
#### <a id="is_coherent">bool is_coherent(void)</a>
(Debugging) Consistency check

---
#### <a id="is_empty">bool is_empty(void)</a>
Check whether the List is empty

---
#### <a id="is_on_list">bool is_on_list</a>
Check whether the List contains a Link

---
#### <a id="reset0">pointer reset(void)</a>
Reset (empty) the List

---
#### <a id="reset1">pointer reset(void*)</a>
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
