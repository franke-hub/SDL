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
//       ~/src/doc/cpp/pub_list-ai.md
//
// Purpose-
//       List.h reference manual: AI_list<T>
//
// Last change date-
//       2023/06/14
//
-------------------------------------------------------------------------- -->
## `AI_list&lt;T&gt;::` begin, end, fifo, get_tail, is_coherent, is_empty, is_on_list, reset(void), reset(void*)

###### Defined in header &lt;pub/List.h&gt

See also: [`AI_list&lt;T&gt;::iterator`](./pub_list-ai_iter.md)

Types:
- `AI_list&lt;T&gt;::struct Link`: The Link type

`AI_list&lt;T&gt;` Links are implemented using
`class T : public pub::AI_list&lt;T&gt::Link {...};`

---
#### iterator begin() noexcept;
Create/construct a begin() iterator.

Implementation note: The iterator *removes* all elements from the AI_list,
replacing the List with a dummy item.
It also reverses the AI_list's native LIFO ordering so that the iterator
returns FIFO ordered Links.

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

```
#include <pub/Debug.h>              // For namespace PUB::debugging
#include <pub/Dispatch.h>           // For namespace PUB::dispatch
#include <pub/List.h>               // For PUB::AI_list, ...
#include <pub/Thread.h>             // For PUB::Thread
#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;
using namespace PUB::dispatch;

int main() {
   debugf("NOT CODED YET\n");
}
```

#### See also:

- [Dispatch](Dispatch.md) Multi-threading dispatcher
  - pub::dispatch::Item, derived from AI_list&lt;pub::dispatch::Item&gt;::Link.
