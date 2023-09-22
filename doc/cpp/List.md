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
//       ~/doc/cpp/List.md
//
// Purpose-
//       List.h reference manual
//
// Last change date-
//       2023/09/22
//
-------------------------------------------------------------------------- -->
## pub::List
\#include <pub/List.h>

Unlike std::List<T>, pub::List<T> elements are links, not copies.
The various List types do not *own* the Link objects; they only control their
position on a List.

Advantages:
- List handling is optimized, especially when copying is expensive.

Disdvantages:
- Since pub::List classes do not *own* List<T>::Link objects it becomes the
application's responsibility to create and delete them.
(While this is similar to that of a std::list<T*>, accessing a
pub::List<T>::Link requires one less load operation.)

Differences:
- ++pub::List<T>::end() == pub::List<T>::end()
- ++std::list<T>::end() == std::list<T>::begin()
- --pub::List<T>::end() == pub::List<T>::end()
- --std::list<T>::end() == std::list<T>::rbegin()
- --pub::List<T>::begin() == pub::List<T>::end()
- --std::list<T>::begin() == std::list<T>::rend()
- Methods *List<T>::end() and List<T>::end()-> throw an exception rather than
act in an undefined manner.
- std::list has methods not available in pub::List. It handle arrays extremely
well.
- pub::AI_list provides a lock-free atomic insertion capability not available
in std::list.

Notes:
- pub::List<T>::Link construction and destruction is *always* an application's
responsibility. For example, pub::~List<T> does nothing.
- For all List classes, the is_coherent and is_on_list methods run in linear
linear time, examining the entire list.
However, is_coherent reports FALSE should a List contains more than an
implementation defined (Currently 1G) Link count.
Other methods assume that the List is coherent and either ignore or do not
check for usage errors.


The List types:

- AI_list<T>: Atomic Insert singly linked list. (Thread safe)
- DHDL_list<T>: Doubly Headed Doubly Linked list.
- DHSL_list<T>: Doubly Headed Singly Linked list.
- SHSL_list<T>: Singly Headed Singly Linked list.
- List<T>: Alias for DHDL_list<T>.

The associated Link class is defined within the List class.
Link classes must be derived from that List::Link class.

Example List declaration and usage-
```
   class My_link : public List<My_link>::Link {
   public:
     My_link(...) : List<My_link>::Link(), ... { ... } // Constructor
     // Remainder of My_link definiion
   }; // class My_link, the elements to be put on a class List<My_link>

   List<My_link> my_list1;          // A List containing My_link elements
   List<My_link> my_list2;          // Another List contining My_link Links

   My_link* link1= new My_link();   // Create a new My_link
   my_list1.fifo(link1);            // Insert it (FIFO) onto my_list1
   My_link* link2= my_list1.remq(); // Then remove it, emptying my_list1
   assert( link1 == link2 );        // The link added is the link removed
   my_list2.lifo(link2);            // Insert it (LIFO) onto my_list2
   // my_list1 is empty, my_list2 only contains link2 (which == link1)
```

#### AI_list

The Atomic Insert list is a lock free list.
Any number of threads may insert Links, but only a single thread may check or
remove links.

| Method | Purpose |
|--------|---------|
| [begin](./pub_list-ai.md) | Create a begin() iterator |
| [end](./pub_list-ai.md) | Create an end() iterator |
| [fifo](./pub_list-ai.md) | Thread-safe FIFO order link insertion |
| [get_tail](./pub_list-ai.md) | Obtain the tail link |
| [is_coherent](./pub_list-ai.md) | (Debugging) Consistency check |
| [is_empty](./pub_list-ai.md) | Check whether the List is empty |
| [is_on_list](./pub_list-ai.md) | Check whether the List contains a Link |
| [reset(void)](./pub_list-ai.md) | Reset (empty) the List |
| [reset(void*)](./pub_list-ai.md) | Empty the List, replacing it with a dummy Link |

#### DHDL_list (__TODO__ Document)
#### DHSL_list (__TODO__ Document)
#### SHSL_list (__TODO__ Document)
