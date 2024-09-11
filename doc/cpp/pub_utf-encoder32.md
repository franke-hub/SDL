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
//       ~/doc/cpp/pub_utf-encoder32.md
//
// Purpose-
//       Utf.h utf32_encoder reference manual
//
// Last change date-
//       2024/09/12
//
-------------------------------------------------------------------------- -->
## <a id="header">pub::utf32_encoder</a>

### Defined in header <pub/Utf.h>

<!-- ===================================================================== -->
---
### <a id="attrib">Attributes</a>

```
protected:
const utf32_t*         buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in bytes
Index                  column= -1;  // Current buffer Column index
Offset                 offset= 0;   // Current buffer unit index
MODE                   mode=MODE_RESET; // Decoding mode, default: big-endian
```

---
### <a id="constr">Constructors</a>

#### utf32_encoder( void ) // Default constructor
Encoder fields can always be initialized using the reset method.

#### utf32_encoder(const utf32_encoder&) = delete // Copy constructor
The copy constructor is **NOT** provided, but copy assignment is.

#### utf32_encoder(const utf32_t*, Length, mode=MODE_RESET)
The buffer is set from the buffer address parameter,
the length is set from the buffer Length parameter,
the mode is set from the MODE parameter,
and the encoder is reset (column= -1; offset= 0;).

#### utf32_encoder(const char* address, Length length)
This alias constructor is provided for the utf32_encoder

---
### <a id="assign">Assignment operators</a>

#### utf32_encoder& operator=(const utf8_decoder&)
#### utf32_encoder& operator=(const utf16_decoder&)
#### utf32_encoder& operator=(const utf32_decoder&)
#### utf32_encoder& operator=(const utf8_encoder&)
#### utf32_encoder& operator=(const utf16_encoder&)
#### utf32_encoder& operator=(const utf32_encoder&)
The source encoder or decoder is copied into a decoder of the same type, then
that decoder is decoded symbol by symbol into the encoder.
If, during that copy, there is not enough room in the encoder's buffer
to encode that symbol a pub::utf_overflow exception is thrown.

---
### <a id="methods">Methods</a>

#### <a id="debugd">void debug(const char* info) const</a>
Write a debugging message to the trace file.
This message includes the info parameter and the content of all fields.

#### <a id="get-ci">Index get_column_index(void)</a>
Returns the current column.

#### <a id="get-md">MODE get_mode(void)</a>
Returns the current encoding MODE.

#### <a id="get-of">Offset get_offset(void)</a>
Returns the current offset.

#### <a id="set-md">void set_mode(MODE)</a>
Sets the current encoding MODE.

#### <a id="encode">unsigned encode(Symbol)</a>
Encodes the specified symbol, updating column and offset.

Invalid unicode Symbols are not stored as-is, they are silently replaced by
the UNI_REPLACEMENT Symbol.

Returns the encoding Length, in native units. If there is not enough room in
the encoder to encode the symbol, zero is returned.

#### <a id="resets">void reset(utf32_t*, Length, MODE) noexcept</a>
Replaces the encoder's buffer, length, and mode with the specified parameters,
then sets the encoder offset= 0 and column= -1.

#### void reset(void) noexcept
Sets the encoder offset= 0 and column= -1.
