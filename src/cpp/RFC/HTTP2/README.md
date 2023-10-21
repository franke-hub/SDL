<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/src/cpp/RFC/README.md
//
// Purpose-
//       RFC subdirectory implementation notes
//
// Last change date-
//       2023/10/19
//
-------------------------------------------------------------------------- -->

# ~/src/RFC/README.md

Copyright (C) 2023 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

----

This README file contains implementation notes and a historical implementation
record.

---

### 10/19/2023 Status: Added missing resize decoding/encoding operations.
Resize now actually updates the entry_array rather than always using all or
part of the default size.

- Phase-0 implementation complete
- Added IodaReader::get_writer which allows duplicate IodaReader creation.
(The duplicate IodaReader is used by resize operation validation logic.)
- Examples are now self-checking. All examples check transmission data and
final state.

---

### 10/13/2023 Status: RFC7541 HPACK Phase-0 encoder/decoder written.
- Files:
  - Main.cpp: Test case driver
  - Main7541.hpp: Appendix C test case driver
  - RFC7541.h: Class definitions
  - RFC7541.cpp: Class implementations.
  - RFC7541.hpp: (Static) tables and utility subroutines
- Class Huff: (Huffman encoding)
  - Operational
  - NO timing test
  - NOT production tested.
- Class Pack:
  - Mostly operational: all appendix C examples work properly
  - Missing: Resize transfer encoding/decoding (simple)
- Some error conditions may be untested.

- The phase-0 HPACK header table compressor implementation now runs all of
the examples as well as extended unit tests.
This implementation doesn't worry (much) about optimization, except for
the implementation of a "spiffy" static entry table, described later.
  - These specification issues are clarified:
    - The name and value data lengths DO NOT include any associated length
encoding.
    - Section 4.1 talks about counting the number of references to the name and
value in the entry.
Example C.3.1 shows that the Entry DOES NOT contain references.
If an Entry::name index is used, the indexed name is copied into the Entry and
that copy counts towards the used data space.
This is important for the production implementation because it allows a
round-robin approach to be used for name and value data storage.

- Renamed variable names "index_" to "entry_", both in the code and this
documentation file. Added a new typedef "Index_ix," a Value_t used to directly
access an entry_array element, i.e. when used as entry_array[index_ix].

##### Table entry_array layout, with example
This table aided in the implementation of Pack methods entry_ix2entry and
entry2entry_ix.
Earlier diagrams do not convey enough usable information.


```
 entry_size |              | Array_ix   entry_size |              | Array_ix
            +--------------+                       +--------------+
          7 |              | 1                   7 |      63      | 1
            +--------------+                       +--------------+
          6 |              | 2                   6 |      62      | 2
            +--------------+                       +--------------+
          5 |      64      | 3 _old              5 |              | 3 _ins
            +--------------+                       +--------------+
          4 |      63      | 4                   4 |              | 4
            +--------------+                       +--------------+
          3 |      62      | 5                   3 |              | 5
            +--------------+                       +--------------+
          2 |              + 6 _ins              2 |      66      + 6 _old
            +--------------|                       +--------------|
          1 |              + 7                   1 |      65      + 7
            +--------------+                       +--------------+
 Index_ix 0 |              | 8         Index_ix  0 |      64      | 8
            +--------------+                       +--------------+

 For _old > _ins, the upper and lower ranges both need to be tested.
```

---

### User interface

See RFC7541.h for the complete interface detail.

Encoding and decoding use Properties, a Property vector,
with some convenience methods added.

Each Property contains a name string, a value string, an encoding type code,
and the name and value each have Huffman encoding boolean controls.
I will probably stick with std::string for the Property's name and value type.
Some sort of copying that requires storage allocation seems inevitable.

Only the "==" and "!=" comparison operators are provided.
These operators only compare the name and value strings.

The header list is encoded from and decoded into a Property vector.
The vector<Property> is extended into a Properties class that adds some
convenience functions.

A decoded Property's encoding type may differ from the Property's encoding
type passed to the encoder.
(The encoder automatically handles indexing requests when the Entry isn't
present in table storage.)
- ET_INDEX may be transmitted as ET_INSERT or ET_INSERT_NOINDEX.
- ET_NEVER may be transmitted as ET_NEVER_NOINDEX.
- ET_CONST may be transmitted as ET_CONST_NOINDEX.

The encoder *DOES NOT* "upgrade" an encoding type.
- ET_INSERT is never transmitted as ET_INDEX.
- ET_INSERT_NOINDEX is never transmitted as ET_INSERT or ET_INDEX.
- ET_NEVER is never transmitted as ET_INDEX.
- ET_NEVER_NOINDEX is never transmitted as ET_NEVER or ET_INDEX.
- ET_CONST is never transmitted as ET_INDEX.
- ET_CONST_NOINDEX is never transmitted as ET_CONST or ET_INDEX.

A decoded Property's Huffman encoding controls are always defaulted.
(The current default is encoded.)
I don't know why (except for testing) an application would actually want to
control whether or not encoding was used.

While the Property's equality operators "==" and "!=" are provided, only
the name and value are compared.
Some encoding information is gained or lost in transmission, as the
interface allows indexing to be used when available since an application
has no knowledge about whether an index can be used when it's actually needed.

Using the Ioda as a data buffer turned out to be a good idea. Some additional
debugging methods were added to Ioda so that encodings could be completely
displayed, and typedefs were added to clarify an Ioda's state.

#### Implementation notes

- We can (and will) use 32-bit offsets instead of 64 bit pointers.
The SETTINGS frame defines SETTINGS_HEADER_TABLE_SIZE, the maximum size of the
compression table.
Since all SETTINGS are 32-bit unsigned value, the maximum size of of a table
is limited to 4G.

We can use uint32_t offsets from a known (64-bit) storage origin instead to
save space.

Even if our implementation then ran into problems trying to support a 4G
table, these would theoretical problems rather than practical ones.
Large table sizes cannot be used by servers without (severly) limiting
the number of connections they could support.
They'd quickly run out of storage.

We'll limit the maximum supported table size to something less than 4G so
we won't have to spend any more time worrying about offset size problems that
would only arise when testing using impractical table sizes.

- Muddled thinking mia culpa: Way too much time was spend worrying about these
theoretical problems with a 4G table size.

- Thankfully, no name or data reference count is needed by Entries.
Reference counting implies that some really complex name/value data storage
management would be needed to handle this without storage allocation.
Since name and data storage is released in the order that it's allocated,
some sort of round-robin data storage allocation can be used.

The actual encoding table data storage will probably need to be somewhat
larger than the specified size to account for these issues:
- Temporary space needs to be available for collecting a name or value data
when decoding, even if that data isn't going to be indexed.
The SETTINGS frame defines SETTINGS_MAX_HEADER_LIST_SIZE as an *advisory*
value for the combined name, value, and Entry table index length.
  - Unresolved: What happens if an encoder fails to follow our advice?
Do we (or can we) abort the stream? Do we abort the connection?
Do we allocate temporary storage?
- The specification talks about moving the existing Entries in the Entry table
each time a new Entry is added. I didn't like that option from the get-go.
It's better to allocate from the top of the Entry table downward, avoiding
moving Entries in the table.
This *does* have an issue when the table wraps and new entries are allocated
from the top and removed from the bottom, but it's nothing that a little bit
of spiffy code can't handle while avoiding any sort of data movement.
(At this writing, that code hasn't been written yet, but it will be part of
the Phase-0 implementation.)
- Compression tables apply to a connection, not a stream.
We need to serialize encoding and decoding.
Continuation frames are part of this process.

We use a hash table lookup to find an Entry given a name or name/value pair.
This hash table is currently a fixed-length table of (pub library) Lists.
The pub library List implementation consists of a head and tail pointer,
along with mechanisms for accessing and controlling List elements.
List elements each have a forward and reverse link to the next or prior
link.

----

### Status: 09/22/2023

RFC7541 Implementation status:
- Files:
  - Main.cpp: Test case driver
  - RFC7541.h: Class definitions
  - RFC7541.cpp: Class implementations.
  - RFC7541.hpp: (Static) tables and utility subroutines
- Class Huff:
  - Implemented.
  - Interface looks OK, but hasn't been production verified.
  - Unit tested. NO timing test or production test.
  - NO timing test
  - NOT production tested.
- Class Pack:
  - Scaffolded

----

### TODO: RFC7541 HPACK name/value compressor (DONE: 10/13/2023)

- Go through the reference examples, possibly implementing enough to handle
them one by one as described. This might clarify the spec somewhat.
- This prototype will be called the Phase-0 implementation, implementing the
interface but almost completely ignoring optimization.

----

### Muddled thinking: RFC7541 HPACK name/value compressor

This section includes thinking about the production implementation before
completely understanding the specification.
From the get-go, I wanted to optimize the Entry table.

Moved header notes:

```
//
// Implementation notes iv_table-
//       The Entries index value begins at STATIC_INDEX_DIM and ends at entry_size+1
//       The value table is allocated from the end of the combined table,
//       downward toward the end of the entry table.
//
//       When entering a new table, the entry_table is moved to make space
//       for the new entry and the value is allocated from end of the table
//       minus the value size. If there isn't enough room to insert the new
//       entry, we lop off the oldest entry table and its data. This involves
//       moving the remainder of the table to take up the space used by the
//       removed data and updating all the associated data pointers.
//
//       From what I can glean from an actual implementation, name and value
//       data lengths DO NOT include any associated length encoding. The 32
//       byte overhead applies without regard to the actual space required for
//       an Entry or name/value space management.
//
//       At this point we are uncertain about the handling of data references.
//       If data reference counts are used, we may have to continue lopping off
//       Entry table entries until enough space is made available by the
//       that process alone or free a data area that's not at the end of the
//       value_table area. In that case only *some* of the entry_table pointers
//       are updated. It gets a bit complex.
//
//       Since a Property dictionary applies to a connection (not a Stream,)
//       serialization will be required.
//
//       For compression to operate properly, the header frames must be
//       processed by the receiver in the same order that they are processed
//       by the sender. When headers are split into continuation frames, these
//       frames must immediately and sequentially follow the initial header
//       frame. When decoding, the HPACK decoder needs the entire header frame.
//
//       HPACK compression does not define an implementation, it only defines
//       compression and decompression operation. It should be noted that the
//       defined storage size used by HPACK is limited to UINT32_MAX since all
//       settings values are defined by a 32 bit field and the dynamic table
//       size must be less than or equal to that connection settings size.
```


### RFC7541 HPACK, Huffman compression

Implementation of Huffman compression was relatively straightforward once I
realized that the compression table was static. Whew.

----

### RFC7541 HPACK, Header compression
The header storage compressor isn't so simple.

#### Specification clarification issues

The specification describes a reference count in the Entry table, but doesn't
say how it might be used.
In particular, when an indexing reference is made, it's not clear whether or
not the storage allocation for the name is counted.

A simple way is to duplicate the Entry name and count its size.
This way Entry data storage can both be put into ring buffers, and the 32-byte
Entry descriptor and its associated data area are always discarded at the same
time.

There is some complexity there when the table is nearly full.
The Entry and data tables may need to be moved and compressed.
When data is moved, the Entry table pointers to that data needs to be moved
as well.
Most of this complexity can be avoided with name and value size limits and
over-allocating the combined Entry and data storage area.

If the indexed name isn't stored multiple times, then it's storage doesn't
necessarily go away when an Entry is discarded.
This leads to complex data storage management with fragmentation gaps.

This adds to the complexity and frequency of Entry and data table storage
compressions.

Note that the maximum combined Entry and name/value data areas is limited
to a 32-bit unsigned value, UINT32_MAX.
This means that one data area can be allocated and 32 bit offsets can
be used instead of 64 bit pointers.
Even if the maximum storage area was 32 bits and over allocation is used,
separate (64-bit) Entry and header data area origins can be used.

64 bit values should be used when calculating total used storage to avoid
arithmetic overflow considerations.

#### Finishing the design before starting it
I started off trying to figure out how to optimize the compressor storage
which consists of an indexed table and strings referenced by that table.
For part of that, I'm thinking that a downwardly allocating Entry table is less
overhead than a table where you insert entries at the bottom.
This table needs to be adjusted after the table is full.

When we reach the bottom of the table, we could either move the entries so that
table_old == table_top *or* just allocate from the top so that we handle a
split table when table_new > table_old.
The newest (lower) part of the table is from table_new to table_top and the
oldest (higher) part of the table is from table_bot to table_old.
That sounds more better than moving entries around.
Probably want to use table_ins (the insert point) one below where table_new
appears in this diagram.

Design 1
```
             +----------------+
table_top--> | (top of table) | (May equal table_old, table_new)
             +----------------+
             :                : <- Entries from table_old to table_top have
             :                :    been released.
             +----------------+
table_old--> | Oldest insert  | (Indirect index to oldest insert)
             +----------------+
             :                : <- Entries from table_new to table_old are
             :                :    active. Entry table_new is index[62].
             +----------------+
table_new--> | Newest insert  | (Indirect index to newest insert)
             +----------------+
             :                : <- Entries are allocated from table_new
             :                :    downward to table_bot.
             :                :
             :                :
             +----------------+
table_bot--> |                | (Lowest available insert point)
             +----------------+
```

Design 2, moved from source code.
```
//
// Implementation notes (entry_array)-
//       Values entry_old and entry_ins are positive downward indexes from
//       entry_size.
//
//       When entry_ins == entry_old, the table is empty.
//           (entry_ins and entry_old should be 0)
//
//       When entry_ins > entry_old,
//           entry_used= entry_ins - entry_old.
//
//                         :<<<< offset >>>>:
//              entry_size | 0              |
//                     --- +----------------+
//                         | 1              |
//                         +----------------+
//                         :                :
//                         +----------------+ ---
//                         | entry_old      |  A
//                         +----------------+  |
//                         :                :  Allocated
//                         +----------------+  |
//                         |                |  V
//                         +----------------+ ---
//                         | entry_ins      |
//                         +----------------+
//                         :                :
//                         +----------------+
//                         | entry_size     |
//                     --- +----------------+
//                         | entry_size + 1 |
//
//       When entry_ins < entry_old,
//           entry_used= entry_ins + entry_size + 1 - entry_old
//
//                         :<<<< offset >>>>:
//              entry_size | 0              |
//                    ---  +----------------+ ---
//                         | 1              |  A
//                         +----------------+  |
//                         :                :  Allocated new
//                         +----------------+  |
//                         |                |  V
//                         +----------------+ ---
//                         | entry_ins      |
//                         +----------------+
//                         :                :  (Available)
//                         +----------------+ ---
//                         | entry_old      |  A
//                         +----------------+  |
//                         :                :  Allocated older entries
//                         +----------------+  |
//                         | entry_size     |  V
//                     --- +----------------+ ---
//                         | entry_size + 1 |
```

#### Phase-0 prototype
Now I'm thinking about building a phase-0 prototype which doesn't worry about
allocation so much as keeping track of the virtual allocations.
The virtual allocation allows 32 bytes per Entry plus the size of the
(uncompressed) data strings.
No overhead is allowed when counting the data string lengths.

This prototype will show how strings are discarded, and anything that might
be needed to aid encoding and decoding.
Optimization isn't needed, but data tracking is.
It seems like something might be missing from the specification regarding
when some of these data areas are removed. We'll see.

So, the phase-0 prototype:
- Can include that spiffy ring buffer Entry allocator.
It can be big enough to handle any phase-0 test.
- Can use std::string data elements.
All we need to do is keep track of the total string length that's in use.
- Doesn't need to worry so much about where it gets external data.
Just fake it: use internal methods returning static data.

The production version will need to operate without storage management calls.
Final data storage management might be tricky.
If the Entry table and string allocation areas are in the same area, the
boundary between these areas might have to dynamically change.
That can be pretty complex depending on where the Entry pointers are.
Some Entries might need to be moved.

Random phase-0 thoughts:
- I'm thinking about using Ioda objects as output/input data areas in the
production version. Maybe just simple std::strings in phase zero.
- Need compare in Properties for test verification.
- Shut up and start coding.

----
