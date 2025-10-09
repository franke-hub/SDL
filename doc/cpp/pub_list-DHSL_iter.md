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
//       ~/doc/cpp/pub_list-DHSL_iter.md
//
// Purpose-
//       List.h reference manual: DHSL_list iterator
//
// Last change date-
//       2025/10/05
//
-------------------------------------------------------------------------- -->
###### Defined by header <pub/List.h>
## `pub::DHSL_list<T>::iterator`

The DHSL_list<T>::iterator *does not* reference the DHSL_list,
and the DHSL_list *does not* reference the iterator.
The iterator only contains a pointer to the current DHSL_list<T>::Link.

Fields:
- pointer _link: The current Link

Where:<br/>
For a DHSL_list<T> const iterator:<br/>
- typedef const T* pointer;

For a DHSL_list<T> iterator:<br/>
- typedef T* pointer;

Since Iterators only access the current Link, the List must remain
coherent while an Iterator is actively used.
In particular, removing the Iterator link makes the Iterator unusable
whether or not the remove operation is done via the Iterator.

---
#### DHSL_list::iterator::iterator(void) noexcept;
Construct an end() iterator.
(All end() iterators compare equal.)

---
#### DHSL_list::iterator::iterator(DHSL_list*) noexcept;
Construct a begin() iterator.
The begin() iterator sets the iterator to the current head element.

---
#### DHSL_list::iterator::iterator(DHSL_list::iterator&) noexcept;
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

---
#### iterator& operator++(int) noexcept;
Updates the iterator to point to the next (FIFO ordered) Link,
returning the iterator state before the update.

---
#### friend bool operator==(const iterator&, const iterator) noexcept;
#### friend bool operator!=(const iterator&, const iterator) noexcept;
Compares iterator's current Link* for (in)equality
without regard to the associated List*.
Note: All end() iterator's _link fields are nullptr.
