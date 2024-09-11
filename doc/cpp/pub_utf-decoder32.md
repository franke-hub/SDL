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
//       ~/doc/cpp/pub_utf-decoder32.md
//
// Purpose-
//       Utf.h decoder utf32_decoder reference manual
//
// Last change date-
//       2024/09/12
//
-------------------------------------------------------------------------- -->
## <a id="header">pub::utf32_decoder</a>

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
MODE                   mode=MODE_RESET; // Decoding mode, default: big-endian
```

---
### <a id="constr">Constructors</a>

#### utf32_decoder( void ) // Default constructor
Decoder fields can always be initialized using the reset method.

#### utf32_decoder(const utf32_decoder&) // Copy (decoder) constructor
The buffer, length, and mode fields are copied from the source decoder.
The decoder begins at its origin, with offset= 0 and column= -1.

Note that the source decoder's buffer is shared and must remain constant.

#### utf32_decoder(const utf32_encoder&) // Copy (encoder) constructor
The buffer and mode fields are copied from the source encoder
and the decoder's length is set from the encoder's offset.
The decoder begins at its origin, with offset= 0 and column= -1.

Note that the source encoder's buffer is now shared and must remain constant.

#### utf32_decoder(const utf32_t*, Length, MODE= MODE_RESET)
The buffer, length, and MODE fields are set from the parameters and
the decoder begins at its origin, with offset= 0 and column= -1.

#### utf32_decoder(const utf32_t* addr, MODE mode= MODE_RESET )
This is an alias for utf32_encoder(addr, utflen(addr), mode).

---
### <a id="assign">Assignment operators</a>

#### utf32_decoder& operator=(const utf32_decoder&)
The buffer, length, and mode are copied from the source decoder.
The decoder is reset to begin at its origin, with offset= 0 and column= -1.

#### utf32_decoder& operator=(const utf32_encoder&)
The buffer (address) and mode are copied from the source decoder, and
the decoder's length is set from the encoder's offset.
The decoder is reset to begin at its origin, with offset= 0 and column= -1.

---
#### <a id="debugd">void debug(const char* info) const</a>
Write a debugging message to the trace file.
This message includes the info parameter and the content of all fields.

#### <a id="get-cc">Count get_column_count( void ) const</a>
Returns the total column count.

#### <a id="get-ci">Index get_column_index( void ) const</a>
Returns the current (adjusted) column index.

#### <a id="get-ln">Length get_length( void ) const</a>
Returns the current Length in native units.

#### <a id="get-md">MODE get_mode( void ) const</a>
Returns the current MODE.

#### <a id="get-of">Offset get_offset( void ) const</a>
Returns the current Offset in native units.

#### <a id="get-or">Offset get_origin( void ) const</a>
Gets the current encoding origin.
The encoding origin will be one if the first Symbol is a Byte Order Mark,
zero otherwise.

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

#### <a id="set-md">void set_mode(MODE)</a>
Updates the current MODE.

#### <a id="set-si">Count set_symbol_index(Index)</a>
Positions the decoder at the specified symbol, returning 0 if the Index value
is within the decoder.

If the specified Index is not within the buffer,
the buffer is positioned at the UTF_EOF position and
the number of native units past the end of the buffer is returned.

#### <a id="copycc">utf32_decoder copy_column( void ) const</a>
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

#### <a id="resets">void reset(const utf32_t*, Length, MODE= MODE_RESET)</a>
Resets and re-initializes the decoder.

The buffer and length are set from the address and Length parameters.
The decoder is reset to begin at its origin, with offset= 0 and column= -1.

The mode field is initially set from the MODE parameter.
If mode == MODE_RESET, the first buffer Symbol is then checked:
- If it's a MARK_ORDER_BYTE32, the mode is set to MODE_LE (little endian.)
- Otherwise the mode is set to MODE_BE (big endian.)

#### void reset(void)
This resets the decoder to its initial state, setting offset= 0 and column= -1.
(The mode field remains unchanged.)
