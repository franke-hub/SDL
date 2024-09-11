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
//       ~/doc/cpp/pub_utf-encoder08.md
//
// Purpose-
//       Utf.h utf8_encoder reference manual
//
// Last change date-
//       2024/09/12
//
-------------------------------------------------------------------------- -->
## <a id="header">pub::utf8_encoder</a>

### Defined in header <pub/Utf.h>

<!-- ===================================================================== -->
---
### <a id="attrib">Attributes</a>

```
protected:
const utf8_t*          buffer= nullptr; // Data buffer address
Length                 length= 0;   // Data length in bytes
Index                  column= -1;  // Current buffer Column index
Offset                 offset= 0;   // Current buffer unit index
```

---
### <a id="constr">Constructors</a>

#### utf8_encoder( void ) // Default constructor
Encoder fields can always be initialized using the reset method.

#### utf8_encoder(const utf8_encoder&) = delete // Copy constructor
The copy constructor is **NOT** provided, but copy assignment is.

#### utf8_encoder(const utf8_t*, Length)
The buffer is set from the buffer address parameter,
the length is set from the buffer Length parameter,
and the encoder is reset (column= -1; offset= 0;).

#### utf8_encoder(const char* address, Length length)
This alias constructor is provided for the utf8_encoder

---
### <a id="assign">Assignment operators</a>

#### utf8_encoder& operator=(const utf8_decoder&)
#### utf8_encoder& operator=(const utf16_decoder&)
#### utf8_encoder& operator=(const utf32_decoder&)
#### utf8_encoder& operator=(const utf8_encoder&)
#### utf8_encoder& operator=(const utf16_encoder&)
#### utf8_encoder& operator=(const utf32_encoder&)
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

#### <a id="get-of">Offset get_offset(void)</a>
Returns the current offset.

#### <a id="encode">unsigned encode(Symbol)</a>
Encodes the specified symbol, updating column and offset.

Invalid unicode Symbols are not stored as-is, they are silently replaced by
the UNI_REPLACEMENT Symbol.

Returns the encoding Length, in native units. If there is not enough room in
the encoder to encode the symbol, zero is returned.

#### <a id="resets">void reset(utf8_t*, Length) noexcept</a>
Replaces the encoder's buffer address and Length with the specified value,
then sets the encoder offset= 0 and column= -1.

#### void reset(void) noexcept
Sets the encoder offset= 0 and column= -1.
