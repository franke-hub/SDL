<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/pub_utf-decoder08.md
//
// Purpose-
//       Utf.h utf8_decoder reference manual
//
// Last change date-
//       2024/09/12
//
-------------------------------------------------------------------------- -->
## <a id="header">pub::utf8_decoder</a>

### Defined in header <pub/Utf.h>

<!-- ===================================================================== -->
---
### <a id="attrib">Attributes</a>

```
protected:
const utfN_t*          buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in bytes
Index                  column= -1;  // Current buffer Column index
Offset                 offset= 0;   // Current buffer unit index
```

---
### <a id="constr">Constructors</a>

#### utf8_decoder( void ) // Default constructor
Decoder fields can always be initialized using the reset method.

#### utf8_decoder(const utf8_decoder&) // Copy (decoder) constructor
The buffer and length fields are copied from the source decoder.
The decoder begins at its origin, with offset= 0 and column= -1.

Note that the source decoder's buffer is shared and must remain constant.

#### utf8_decoder(const utf8_encoder&) // Copy (encoder) constructor
The buffer field is copied from the encoder
and the decoder's length is set from the encoder's offset.
The decoder begins at its origin, with offset= 0 and column= -1.

Note that the source encoder's buffer is now shared and must remain constant.

#### utf8_decoder(const utf8_t*, Length)
The buffer and length fields are set from the parameters and
the decoder begins at its origin, with offset= 0 and column= -1.

#### utf8_decoder(const char* addr, Length length)
This is an alias for utf8_encoder((utf8_t*)addr, length).

#### utf8_decoder(const utf8_t* addr)
This is an alias for utf8_encoder(addr, utflen(addr)).

#### utf8_decoder(const char* addr)
This is an alias for utf8_encoder((utf8_t*)addr, strlen(addr)).

---
### <a id="assign">Assignment operators</a>

#### utf8_decoder& operator=(const utf8_decoder&)
The buffer and length are copied from the source decoder.
The decoder is reset to begin at its origin, with offset= 0 and column= -1.

#### utf8_decoder& operator=(const utf8_encoder&)
The buffer (address) is copied from the encoder and
the decoder's length is set from the encoder's offset.
The decoder is reset to begin at its origin, with offset= 0 and column= -1.

---
### <a id="methods">Methods</a>

#### <a id="debugd">void debug(const char* info) const</a>
Write a debugging message to the trace file.
This message includes the info parameter and the content of all fields.

#### <a id="get-cc">Count get_column_count( void ) const</a>
Returns the total column count.

#### <a id="get-ci">Index get_column_index( void ) const</a>
Returns the current (adjusted) column index.

#### <a id="get-ln">Length get_length( void ) const</a>
Returns the current Length in native units.

#### <a id="get-of">Offset get_offset( void ) const</a>
Returns the current Offset in native units.

#### <a id="get-sc">Count get_symbol_count( void ) const</a>
Returns the total symbol count.

#### <a id="iscomb">bool is_combining(void)</a>
Returns Utf::is_combining(current()).

#### <a id="set-ci">Count set_column_index(Index)</a>
Positions the decoder at the specified column, returning 0 if the Index value
is within the decoder.

If the specified Index is not within the buffer,
the buffer is positioned at the UTF_EOF position and
the number of native units past the end of the buffer is returned.

#### <a id="set-si">Count set_symbol_index(Index)</a>
Positions the decoder at the specified symbol, returning 0 if the Index value
is within the decoder.

If the specified Index is not within the buffer,
the buffer is positioned at the UTF_EOF position and
the number of native units past the end of the buffer is returned.

#### <a id="copycc">utf8_decoder copy_column( void ) const</a>
Returns the current Column substring.

Since the current Column may contain combining characters,
the resultant decoder may contain multiple Symbols.

#### <a id="curent">Symbol current( void ) const</a>
Returns the current Symbol.

If the decoder is positioned such that decode would return UTF_EOF,
UTF_EOF is returned.

#### <a id="decode">Symbol decode( void )</a>
Returns the current Symbol, then positions the decoder at the next Symbol.

If there are no more Symbols in the buffer, UTF_EOF is returned.

#### <a id="resets">void reset(const utf8_t*, Length)</a>
Resets and re-initializes the decoder, setting offset= 0 and column= -1.

#### void reset(const char* addr, Length length)</a>
This is an alias for reset((const utf8_t*)addr, Length length).

#### void reset(void)
This resets the decoder to its initial state, setting offset= 0 and column= -1.
