<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2025 Frank Eskesen.
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
//       ~/doc/cpp/pub_list-DHSL.md
//
// Purpose-
//       List.h reference manual: DHSL_list<T>
//
// Last change date-
//       2025/10/05
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/List.h>
## `pub::DHSL_list<T>`

See also: [`DHSL_list<T>::iterator`](./pub_list-DHSL_iter.md)

#### Definition: "link set"
A link set is a set of links bounded by a head and a tail link.
While a DHSL_list<T> contains an entire link set, portions of the list
comprise a link set as long as the head link precedes the tail link in the
List.

A link set may also be comprised of links entirely outside the content of
any List.

A `{head,tail}` pair comprises a valid link set if and only if:
- `head->get_next()` {-> `get_next()` ... ->} `tail` and
- `tail->get_prev()` {-> `get_prev()` ... ->} `head`.
- The special case `{head,tail}` where `head == tail` is valid.

We do not rely on `head->get_prev()==nullptr` or `tail->get_next()==nullptr`
except when removing links from a list.

#### Types:
- `DHSL_list<T>::struct Link`: The Link type

`DHSL_list<T>` Links are implemented using
`class T : public pub::DHSL_list<T>::Link {...};`

---
#### <a id="begin">iterator begin() noexcept;</a>
Create/construct a begin() iterator.

---
#### <a id="end">iterator end() noexcept;</a>
Create/construct the (default) end() iterator.

---
#### <a id="debug">iterator debug(const char* info) const;</a>
Debugging display. Displays the List elements.

---
#### <a id="fifo">pointer fifo(pointer link);</a>
Insert `link` onto the list with FIFO (First In, First Out) ordering.
The `link` becomes the new tail link.

---
#### <a id="get_head">pointer get_head(void) const</a>
Returns the head link.

---
#### <a id="get_tail">pointer get_tail(void) const</a>
Returns the tail link.

---
#### <a id="insert3">void insert(pointer after, pointer head, pointer tail)</a>
Inserts the links head..tail onto the List following the `after` link.

Implementation note: this method *DOES NOT VERIFY* parameters.
- It doesn't verify that `if_on_list(after)` is true.
- It doesn't verify that `if_on_list(head)` is false.
- It doesn't verify that `if_on_list(tail)` is false.
- It doesn't verify that head..link is a valid link set. It doesn't
verify that `head->get_next()-> ... -> tail` is a valid sequence and doesn't
verify that `tail->get_prev()-> ... -> head` is a valid sequence.

It's not possible to verify whether or not links in the `{head,tail}` link set
are part of another List, and we do not for erroneous usage occurring when
either `if_on_list(head)==true` or `if_on_list(tail)==true`.

---
#### <a id="insert2">void insert(pointer after, pointer link)</a>
The same as insert(after, link, link). This inserts the single Link `link`
onto the List following `after` link.

Implementation note: this method *DOES NOT VERIFY* parameters.
- It doesn't verify that `if_on_list(after)` is true.
- It doesn't verify that `if_on_list(link)` is false.
</br>
Since this is implemented using `insert(after, link, link)`, the link set
is valid.

---
#### <a id="is_coherent">bool is_coherent(void)</a>
(Debugging) Consistency check. (Returns `true` if the List is coherent.)

---
#### <a id="is_on_list">bool is_on_list(pointer link) const</a>
Check whether the List contains a Link

---
#### <a id="lifo">pointer lifo(pointer link);</a>
Insert `link` onto the list with LIFO (Last In, First Out) ordering.
The `link` becomes the new head link.

---
#### <a id="remove">void remove(pointer head, pointer tail)</a>
Removes the link set {head .. tail}.

---
#### <a id="remq">pointer remq(void)</a>
Removes and returns the head link.

---
#### <a id="reset">pointer reset(void)</a>
Reset (empty) the List, returning the old head Link.

---
### <a id="example">Example:</a> Compare with [DHDL_list example](./pub_list-DHDL.md#example)
```
#include <assert.h>                 // For assert
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For size_t

#include <pub/List.h>               // For PUB::DHSL_list, ...
#define PUB _LIBPUB_NAMESPACE

struct Item : public pub::DHSL_list<Item>::Link {
size_t                 value;

   Item(size_t v) : value(v) {}
}; // class Item

int main() {
   pub::DHSL_list<Item> list;

   Item five(5);
   Item one(1);
   Item meaning(42);
   Item more(732);
   Item two(2);

   list.fifo(five&);               // Items in alphabetical order by name
   list.fifo(one&);
   list.fifo(meaning&);
   list.fifo(more&);
   list.fifo(two&);

   auto ix= list.begin();
   assert( ix->value == 5 );
   assert( (++ix)->value= 1 );
   assert( (++ix)->value= 42 );
   assert( (++ix)->value= 732 );
   assert( (++ix)->value= 2 );
   assert( (++ix) == list.end());
   assert( (++ix) == list.end());
   // Attempting to access ix->value would now throw an end_dereferenced error

   printf("NO errors\n");           // (Since no assert was triggered)
   return 0;
}
```
