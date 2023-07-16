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
//       ~/src/doc/cpp/pub_list-ai_iter.md
//
// Purpose-
//       List.h reference manual: AI_list iterator
//
// Last change date-
//       2023/06/14
//
-------------------------------------------------------------------------- -->
## `AI_list&lt;T&gt;::iterator`:: constructors, begin, end, fifo, get_tail, is_coherent, is_empty, is_on_list, reset(void), reset(void*)

###### Defined in header &lt;pub/bits/List.h&gt

While an AI_list::iterator references the AI_list, the AI_list *does not*
reference the iterator.

Fields:
- pointer _left: The remaining Links
- pointer _link: The current Link
- AI_list* _list: The associated AI_list&lt;T&gt;*

---
#### AI_list::iterator::iterator(void) noexcept;
Construct an end() iterator.
(All end() iterators compare equal.)

---
#### AI_list::iterator::iterator(AI_list*) noexcept;
Construct a begin() iterator.
The begin() iterator *removes* all elements from the AI_list, replacing the
List with a dummy Link.

Implementation notes:
The set of AI_list Links on the List are actually added in LIFO order.
The begin() iterator reverses that order so that the iterator returns Links
in FIFO order.
Additionally, if Links are added to the list while the iterator is active,
when the set of iterator Links has been completely processed the newly added
Links begin a new set of iterator Links using the *same* iterator.

---
#### AI_list::iterator::iterator(AI_list::iterator&) noexcept;
(The copy constructor). There is no move constructor.

---
#### pointer get(const) noexcept;
Returns the current Link, which may be a nullptr.

---
#### operator bool();
Returns true iff the current Link* != nullptr.

---
#### operator->() const;
Returns the current Link.
If the current Link == nullptr, an exception is thrown.
(This occurs when the end() iterator is derefereced.)

---
#### iterator& operator++() noexcept;
Updates the current Link to point to the next (FIFO ordered) Link.

Note: end()++ == end().
No exception occurs unless end().operator->() is used.

Implementation note: This is where Links added to the List while the iterator
is active are processed.

---
#### iterator& operator++(int) noexcept;
Updates the current Link to point to the next (FIFO ordered) Link,
returning the iterator state before the update.

---
#### friend bool operator==(const iterator&, const iterator) noexcept;
#### friend bool operator!=(const iterator&, const iterator) noexcept;
Compares iterator's current Link* for (in)equality
without regard to the associated List*.
Note: All end() iterators List* and Link* are nullptr.
