<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
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
//       ~/doc/cpp/Allocator.md
//
// Purpose-
//       Allocator.h reference manual
//
// Last change date-
//       2023/11/26
//
-------------------------------------------------------------------------- -->
## pub::Allocator
\#include <pub/Allocator.h>

Class: Allocator (Base class)

#### Member functions

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [constructor](./pub_allocator.md#a_construct) | (Does nothing) |
| [destructor](./pub_allocator.md#a_construct) | (Does nothing) |
| [check](./pub_allocator.md#a_check) | Debugging: consistency check |
| [debug](./pub_allocator.md#a_debug) | Debugging display |
| [get](./pub_allocator.md#a_get) | Allocate storage |
| [put](./pub_allocator.md#a_put) | Release storage |

----

## [pub::BlockAllocator](./pub_allocator.md#blockallocator)

Class: BlockAllocator (Allocate/release a fixed size block.

#### Member functions

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [constructor](./pub_allocator.md#b_construct) | Initialize the BlockAllocator |
| [destructor](./pub_allocator.md#b_destruct) | Release all allocated storage |
| [get](./pub_allocator.md#b_get) | Allocate a fixed-size storage block |
| [put](./pub_allocator.md#b_put) | Release a fixed-size storage block |
