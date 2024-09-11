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
//       ~/doc/cpp/Utf.md
//
// Purpose-
//       Utf.h reference manual
//
// Last change date-
//       2024/09/12
//
-------------------------------------------------------------------------- -->
## pub::Utf
\#include <pub/Utf.h>

Terminology:
- buffer: The (application-provided) storage area used for decoding and
encoding.
- column: A column is comprised of an initial (non-combining) Symbol plus the
combining Symbols that follow it.
A column represents a glyph. Glyphs (for applications that support them)
require only single display column.
(As a special case, the first Symbol is always treated as non-combining.)
- decoder: The object used for accessing a const encoded buffer.
In addition to the decode method, methods are provided for positioning within
the buffer.
- encoder: The object used for encoding Symbols into a buffer.
- native unit: The machine storage unit used by a decoder or encoder.
- Symbol: A single UTF character (or "code point".)

Native units:
A native unit is the decoding or encoding data type.

The utf8_decoder and utf8_encoder use utf8_t native units.
Depending on a Symbol's value,
1, 2, 3, or 4 native units are needed to represent a Symbol.

The utf16_decoder and utf16_encoder use utf16_t native units.
Either 1 or 2 native units are needed to represent a Symbol.

The utf32_decoder and utf32_encoder use utf32_t native units.
Each native unit represents a Symbol.

### Overview

All objects are defined in namespace 'pub'.

Utf.h contains seven objects:
- class Utf: This is the base class for all UTF decoders and encoders.
It includes type definitions, enumerations, constants, and static methods.
- struct utf8_decoder:  The UTF-8  native unit decoder.
- struct utf16_decoder: The UTF-16 native unit decoder.
- struct utf32_decoder: The UTF-32 native unit decoder.
- struct utf8_encoder:  The UTF-8  native unit encoder.
- struct utf16_encoder: The UTF-16 native unit encoder.
- struct utf32_encoder: The UTF-32 native unit encoder.

Additionally, two std::exceptions are extended:
- class pub::utf_invalid_argument: Thrown when an argument is rejected.
(base class: std::invalid_argument)
- class pub::utf_overflow_error: Thrown during an assignment operation when a
(target) encoder buffer cannot completely contain the source buffer.
(base class: std::overflow_error)

Neither decoders nor encoders allocate storage.
All storage areas are provided by and managed by the application.
All decoder and encoder destructors are default destructors.
They don't do anything.

All decoders use similar interfaces.
Decoder buffer data never changes.
(Decoder methods never modify the buffer.
An decoder's buffer must never be modified by the application.)

Likewise, all encoders use similar interfaces.
The encoder's constructor is given a buffer address and length.
Once set, the buffer address and length are never modified.
Buffer content and indexing are updated by copy, encoding, or assignment.

Copy and assignment operations for decoders:
   Assignment or copy into a target decoder from a source decoder sets the
target decoder's address and length from the source decoder, and resets the
target decoder's position (to the beginning of the buffer.)
The source and target decoders are then indexed separately.
A source decoder may only be copied into a target decoder of the same type.

   Assignment or copy into a target decoder from a source encoder sets the
target decoder's address and length from the source encoder, and resets the
target decoder's position (to the beginning of the buffer.)
A source encoder may only be copied into a target decoder of the same UTF type.
Copy and assignment into a decoder does not involve data movement. The target
decoder always *shares* the source buffer.
Note that after an encoder to decoder assignment, the encoder must not then
modify the buffer area it shared with the decoder.

Copy and assignment operations for encoders:
   Assignment or copy into a target encoder from a source encoder or decoder
is always a copy operation. It proceeds as follows:
- The encoder is reset (its column and offset are set to zero)
- A decoder copy of the source is created. (The source encoder or decoder
is never modified)
- One by one, Symbols are decoded from the decoder copy and encoded into
the target encoder.
If the Symbol can't be encoded because there's no room in the buffer,
the pub::utf_overflow_error exception is thrown and encoding terminates.

A Byte-Order-Mark is never copied into a utf8_encoder.

------------------------------------------------------------------------------

### class UTF

Typedefs:
```
typedef uint8_t        utf8_t;      // The UTF-8  native unit type
typedef uint16_t       utf16_t;     // The UTF-16 native unit type
typedef uint32_t       utf32_t;     // The UTF-32 native unit type

typedef size_t         Count;       // A Column or Symbol count
typedef size_t         Index;       // A Column or Symbol index
typedef size_t         Length;      // Length in native units
typedef size_t         Offset;      // Offset/index in native units
typedef uint32_t       Symbol;      // A UTF Symbol (code point)
```

Enumerations:
```
enum
{  BYTE_ORDER_MARK=    0x0000'FEFF  // Big endian Byte Order Mark, a.k.a BOM
,  MARK_ORDER_BYTE=    0x0000'FFFE  // Little endian Byte Order Mark
,  MARK_ORDER_BYTE32=  0xFFFE'0000  // 32-bit little endian Byte Order Mark
,  UNI_REPLACEMENT=    0x0000'FFFD  // Unicode error replacement character
};

enum MODE                           // Decoding/encoding mode
{  MODE_RESET= 0                    // Mode undefined, big endian assumed
,  MODE_BE                          // Big endian mode
,  MODE_LE                          // Little endian mode
}; // enum MODE
```

Static constants:
```
constexpr static const utf32_t      // (UTF_EOF is an invalid UTF Symbol)
                       UTF_EOF= EOF; // Method decode: No characters remain
```

------------------------------------------------------------------------------

### [Decoders]
- [utf8_decoder ](./pub_utf.md#decoder08)
- [utf16_decoder](./pub_utf.md#decoder16)
- [utf32_decoder](./pub_utf.md#decoder32)

### [Encoders]
- [utf8_encoder ](./pub_utf.md#encoder08)
- [utf16_encoder](./pub_utf.md#encoder16)
- [utf32_encoder](./pub_utf.md#encoder32)

------------------------------------------------------------------------------
### Example:
This example converts UTF-8 strings into UTF-16 strings, reading each UTF-8
string from stdin and and writing the UTF-16 to stdout.

```
#include <cstdio>                   // For EOF
#include <exception>                // For std::runtime_error
#include "pub/Utf.i"                // Import Utf.h symbols into namespace

enum { DIM= 512 };                  // Input/output buffer size

static void output(                 // Convert and output UTF-8 string
   const char*         text,        // The UTF-8 string
   int                 size)        // Of this BYTE length
{
   utf16_t             out[DIM];    // UTF-16 output buffer
   utf16_encoder       encoder(out, DIM, MODE_BE);

   encoder= utf8_decoder(text, size);
   if( encoder.encode('\n') == 0 )
     throw pub::utf_overflow_error("Output buffer too small");

   int M= (int)(encoder.get_offset() * sizeof(utf16_t));
   for(int i= 0; i < M; ++i) {
     int C= putchar(((char*)out)[i]);
     if( C < 0 )                    // If error (not (unsigned char)text[i])
       throw std::runtime_error("putchar EOF");
   }
}

int main() {
   char                inp[DIM];    // UTF-8 input buffer

   putchar(0xFE);                   // Write big-endian Byte Order Mark
   putchar(0xFF);                   // "

   for(;;) {                        // Read and convert lines
     for(int inp_ix= 0; ;) {
       int C= getchar();
       if( C == EOF ) {
         if( inp_ix )
           output(inp, inp_ix);
         return 0;
       }

       if( C == '\r' )              // Ignore '\r'
         continue;
       if( C == '\n' ) {            // If end of line
         output(inp, inp_ix);
         break;
       }

       if( inp_ix >= DIM )
         throw pub::utf_overflow_error("Input buffer too small");
       inp[inp_ix++]= C;
     }
   }
}
```
