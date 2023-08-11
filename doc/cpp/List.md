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
//       2023/07/28
//
-------------------------------------------------------------------------- -->
## pub::List
\#include <pub/List.h>

Unlike std::List<T>, pub::List elements are links.
The various List types do not *own* the Link objects.
They only control their position on a List.

The List types:

- AI_list<T>: Atomic Insert singly linked list. (Thread safe)
- DHDL_list<T>: Doubly Headed Doubly Linked list.
- DHSL_list<T>: Doubly Headed Singly Linked list.
- SHSL_list<T>: Singly Headed Singly Linked list.
- List<T>: Alias for DHDL_list<T>.

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

#### DHDL_list __TODO__ Document
#### DHSL_list __TODO__ Document
#### SHSL_list __TODO__ Document
#### List __TODO__ Document (alias of DHDL_list)
