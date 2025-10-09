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
//       ~/doc/cpp/List.md
//
// Purpose-
//       List.h reference manual
//
// Last change date-
//       2025/10/05
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/List.h>
## `pub::List`

| Class             | Description
| :---------------- | :--------------------------------------------------- |
| [AI_list<T>](#AI_list)     | Atomic Insert singly linked list |
| [DHDL_list<T>](#DHDL_list) | Doubly Headed Doubly Linked list |
| [DHDL_sort<T>](#DHDL_sort) | Doubly Headed Doubly Linked sortable list |
| [DHSL_list<T>](#DHSL_list) | Doubly Headed Singly Linked list |
| [SHSL_list<T>](#SHSL_list) | Singly Headed Singly Linked list |
| [List<T>](#DHDL_list)      | Alias for DHDL_list |
| [Sort<T>](#DHDL_sort)      | Alias for DHDL_sort |

** <a id="Link_Classes">Table 1: Link Classes</a> **

##### <a id="pub_v_std_list">Differences between pub::List and std::List </a>





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
</br>
- --pub::List<T>::end() == pub::List<T>::end()
- --std::list<T>::end() == std::list<T>::rbegin()
</br>
- --pub::List<T>::begin() == pub::List<T>::end()
- --std::list<T>::begin() == std::list<T>::rend()
- Methods *List<T>::end() and List<T>::end()-> throw an exception rather than
act in an undefined manner.
</br>
- pub::AI_list provides a lock-free atomic insertion capability not available
in std::list.
</br>
- std::list has methods not available in pub::List. It handle arrays extremely
well.

Notes:
- pub::List<T>::Link construction and destruction is *always* an application's
responsibility. For example, pub::~List<T> does nothing.
- For all List classes, the is_coherent and is_on_list methods run in linear
linear time, examining the entire list.
However, is_coherent reports FALSE should a List contains more than an
implementation defined (Currently 1G) Links.
Other methods assume that the List is coherent and either ignore or do not
check for usage errors.
- Error checking is minimal or non-existent. See method documentation for
details.

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

#### <a id="AI_list">AI_list</a>
The Atomic Insert list is a lock free thread-safe list.
Any number of threads may insert Links, but only a single thread may check or
remove links.

| Method | Purpose |
|--------|---------|
| [begin](./pub_list-AI.md#begin) | Create a begin() iterator |
| [end](./pub_list-AI.md#end) | Create an end() iterator |
| [fifo](./pub_list-AI.md#fifo) | Thread-safe FIFO order link insertion |
| [get_tail](./pub_list-AI.md#get_tail) | Obtain the tail link |
| [is_coherent](./pub_list-AI.md#is_coherent) | (Debugging) Consistency check |
| [is_empty](./pub_list-AI.md#is_empty) | Check whether the List is empty |
| [is_on_list](./pub_list-AI.md#is_on_list) | Check whether the List contains a Link |
| [reset(void)](./pub_list-AI.md#reset0) | Reset (empty) the List |
| [reset(void*)](./pub_list-AI.md#reset1) | Empty the List, replacing it with a dummy Link |

#### <a id="DHDL_list">DHDL_list (Alised as List)</a>
The DHDL_list (aliased as List) is a Doubly-Headed Doubly-Linked list.
It is not thread safe. Only one Thread may operate on a DHDL_list at a time.
Most DHDL_list operations complete in linear time.
Exceptions: is_coherent, is_on_list, and size.

| Method | Purpose |
|--------|---------|
| [begin](./pub_list-DHDL.md#begom) | Create a begin() iterator |
| [end](./pub_list-DHDL.md#end) | Create an end() iterator |
| [fifo](./pub_list-DHDL.md#fifo) | FIFO order link insertion |
| [get_head](./pub_list-DHDL.md#get_head) | Obtain the head link |
| [get_tail](./pub_list-DHDL.md#get_tail) | Obtain the tail link |
| [insert](./pub_list-DHDL.md#insert) | Positionally insert a set of links |
| [is_coherent](./pub_list-DHDL.md#is_coherent) | (Debugging) Consistency check |
| [is_on_list](./pub_list-DHDL.md#is_on_list) | Check whether the List contains a Link |
| [lifo](./pub_list-DHDL.md#lifo) | LIFO order link insertion |
| [reset(void)](./pub_list-DHDL.md#reset) | Empty the List, returning the head link. |
| [size(void)](./pub_list-DHDL.md#size) | Count the number of elements in the List. |

#### <a id="DHDL_sort">DHDL_sort (Aliased as Sort)</a>
The DHDL_sort (aliased as Sort) is a sortable Doubly-Headed Doubly-Linked list
It is not thread safe. Only one Thread may operate on a DHDL_sort at a time.
Most DHDL_sort operations complete in linear time.
Exceptions: is_coherent, is_on_list, size, and sort.

The DHDL_sort duplicates all functions of DHDL_list, adding the sort method.

| Method | Purpose |
|--------|---------|
| [begin](./pub_list-SORT.md#begin) | Create a begin() iterator |
| [end](./pub_list-SORT.md#end) | Create an end() iterator |
| [fifo](./pub_list-SORT.md#fifo) | FIFO order link insertion |
| [get_head](./pub_list-SORT.md#get_head) | Obtain the head link |
| [get_tail](./pub_list-SORT.md#get_tail) | Obtain the tail link |
| [insert](./pub_list-SORT.md#insert) | Positionally insert a set of links |
| [is_coherent](./pub_list-SORT.md#is_coherent) | (Debugging) Consistency check |
| [is_on_list](./pub_list-SORT.md#is_on_list) | Check whether the List contains a Link |
| [lifo](./pub_list-SORT.md#lifo) | LIFO order link insertion |
| [reset(void)](./pub_list-SORT.md#reset) | Empty the List, returning the head link. |
| [size(void)](./pub_list-SORT.md#size) | Count the number of elements in the List. |
| [sort(void)](./pub_list-SORT.md#sort) | Sort the List. |

#### <a id="DHSL_list">DHSL_list</a>

| Method | Purpose |
|--------|---------|
| [begin](./pub_list-DHSL.md#begin) | Create a begin() iterator |
| [end](./pub_list-DHSL.md#end) | Create an end() iterator |
| [fifo](./pub_list-DHSL.md#fifo) | FIFO order link insertion |
| [get_head](./pub_list-DHSL.md#get_head) | Obtain the head link |
| [get_tail](./pub_list-DHSL.md#get_tail) | Obtain the tail link |
| [insert](./pub_list-DHSL.md#insert3) | Positionally insert a set of links |
| [insert](./pub_list-DHSL.md#insert2) | Positionally insert a link |
| [is_coherent](./pub_list-DHSL.md#is_coherent) | (Debugging) Consistency check |
| [is_on_list](./pub_list-DHSL.md#is_on_list) | Check whether the List contains a Link |
| [lifo](./pub_list-DHSL.md#lifo) | LIFO order link insertion |
| [remove](./pub_list-DHSL.md#remove) | Remove an element from the List. |
| [remq](./pub_list-DHSL.md#remq) | Remove HEAD element from the List. |
| [reset](./pub_list-DHSL.md#reset) | Empty the List, returning the head link. |

#### <a id="SHSL_list">SHSL_list</a>

| Method | Purpose |
|--------|---------|
| [begin](./pub_list-SHSL.md#begin) | Create a begin() iterator |
| [end](./pub_list-SHSL.md#end) | Create an end() iterator |
| [get_tail](./pub_list-SHSL.md#get_tail) | Obtain the tail link |
| [insert](./pub_list-SHSL.md#insert3) | Positionally insert a set of links |
| [insert](./pub_list-SHSL.md#insert2) | Positionally insert a link |
| [is_coherent](./pub_list-SHSL.md#is_coherent) | (Debugging) Consistency check |
| [is_on_list](./pub_list-SHSL.md#is_on_list) | Check whether the List contains a Link |
| [lifo](./pub_list-SHSL.md#lifo) | LIFO order link insertion |
| [remq](./pub_list-SHSL.md#remq) | Remove TAIL element from the List. |
| [reset](./pub_list-SHSL.md#reset) | Empty the List, returning the head link. |
