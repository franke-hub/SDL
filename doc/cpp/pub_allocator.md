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
//       ~/doc/cpp/pub_allocator.md
//
// Purpose-
//       Allocator.h reference manual
//
// Last change date-
//       2023/11/16
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Allocator.h>

The Allocator classes are experimental. Their use is discouraged.

The Allocator class was created in order to compare allocation of fixed-size
block allocation and the system allocators.
The fixed-size blocks were maintained as a block pool.

The Allocator class is not a std::allocator and therefore cannot be used
with the standard library.

The base class simply uses malloc/free.
The BlockAllocator performs well on Cygwin,
but Linux malloc/free (surprisingly) outperforms it.

---

## pub::Allocator:: check, debug, get, put

Allocator is a base class.

####
---
#### <a id="a_allocator">Allocator( void ) = default;</a>
#### virtual ~Allocator( void ) = default;
The (base class) Allocator constructor and destructor do nothing.

---
#### <a id="a_check">virtual int check( void );</a>
The check method is a debugging method intended to be used by subclasses
for internal consistency checking.

Return value: A non-zero value indicates failure.
(The base class simply returns zero.)

---
#### <a id="a_debug">virtual void debug(const char* info= nullptr);</a>
The debug method is a debugging method intended to be used by subclasses
for an informational debugging display.

The base class does nothing.

---

#### <a id="a_get">virtual void* get(size_t size=0);</a>
Allocate a storage block of the given size.
A BlockAllocator (which only allocates fixed-sizee blocks) MAY ignore the
size parameter.

Exceptions: bad_alloc, thrown on allocation failure.

Return value: The address of the allocated storage. (Never nullptr)

(The base class uses malloc.)

---

#### <a id="a_put">virtual void put(void* addr, size_t size=0);</a>
Release a storage block of the given size.
A BlockAllocator (which only allocates fixed-sizee blocks) MAY ignore the
size parameter.

(The base class uses free, ignoring the size parameter.)

---

## <a id="blockallocator">BlockAllocator:: get, put</a>

The BlockAllocator allocates and releases fixed-size blocks.

####
---
#### <a id="b_allocator">BlockAllocator(size_t size, size_t b_size);</a>
The constructor verifies the parameters but does not allocate any storage.
(A storage block is allocated by the first get method call.)

---
#### virtual ~BlockAllocator( void );
The destructor releases all BlockAllocator storage.

---
#### <a id="b_get">virtual void* get(size_t size=0);</a>
Allocate one (fixed-size) block.

If supplied, get verifies that the size parameter
matches the constructor's size parameter.

Exceptions: bad_alloc, thrown on allocation failure.

Return value: The address of the allocated storage. (Never nullptr)

---
#### <a id="b_put">virtual void put(void* addr, size_t size=0);</a>
Release one (fixed-size) block.

If supplied, put verifies that the size parameter
matches the constructor's size parameter.
