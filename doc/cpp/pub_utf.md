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
//       ~/doc/cpp/pub_utf.md
//
// Purpose-
//       Utf.h reference manual - methods
//
// Last change date-
//       2024/09/12
//
-------------------------------------------------------------------------- -->
## <a id="header">Utf.h method reference manual</a>

### Defined in header <pub/Utf.h>

------------------------------------------------------------------------------
### <a id="decoder08">[utf8_decoder ](./pub_utf-decoder08.md#header)</a>

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes      ](./pub_utf-decoder08.md#attrib) | Decoder attributes. |
| [constructor     ](./pub_utf-decoder08.md#constr) | Decoder constructors. |
| [assignment      ](./pub_utf-decoder08.md#assign) | Assignment operators. |
| [debug           ](./pub_utf-decoder08.md#debugd) | Debugging display. |
| [get_column_count](./pub_utf-decoder08.md#get-cc) | Get the total column count. |
| [get_column_index](./pub_utf-decoder08.md#get-ci) | Get the current column index. |
| [get_length      ](./pub_utf-decoder08.md#get-ln) | Get the decoder buffer length. |
| [get_offset      ](./pub_utf-decoder08.md#get-of) | Get the current offset. |
| [get_symbol_count](./pub_utf-decoder08.md#get-sc) | Get the total symbol count. |
| [is_combining    ](./pub_utf-decoder08.md#iscomb) | Test for combining Symbol. |
| [set_column_index](./pub_utf-decoder08.md#set-ci) | Set the current column index. |
| [set_symbol_index](./pub_utf-decoder08.md#set-si) | Set the current Symbol index. |
| [copy_column     ](./pub_utf-decoder08.md#copycc) | Copy the current column. |
| [current         ](./pub_utf-decoder08.md#curent) | Get the current Symbol. |
| [decode          ](./pub_utf-decoder08.md#decode) | Decode the current Symbol. |
| [reset           ](./pub_utf-decoder08.md#resets) | Reset the decoder. |

------------------------------------------------------------------------------
### <a id="decoder16">[utf16_decoder](./pub_utf-decoder16.md#header)</a>

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes      ](./pub_utf-decoder16.md#attrib) | Decoder attributes. |
| [constructor     ](./pub_utf-decoder16.md#constr) | Decoder constructors. |
| [assignment      ](./pub_utf-decoder16.md#assign) | Assignment operators. |
| [debug           ](./pub_utf-decoder16.md#debugd) | Debugging display. |
| [get_column_count](./pub_utf-decoder16.md#get-cc) | Get the total column count. |
| [get_column_index](./pub_utf-decoder16.md#get-ci) | Get the current column index. |
| [get_length      ](./pub_utf-decoder16.md#get-ln) | Get the decoder buffer length. |
| [get_mode        ](./pub_utf-decoder16.md#get-md) | Get the decoder MODE. |
| [get_offset      ](./pub_utf-decoder16.md#get-of) | Get the current offset. |
| [get_origin      ](./pub_utf-decoder16.md#get-or) | Get the first data Symbol offset. |
| [get_symbol_count](./pub_utf-decoder16.md#get-sc) | Get the total symbol count. |
| [is_combining    ](./pub_utf-decoder16.md#iscomb) | Test for combining Symbol. |
| [set_column_index](./pub_utf-decoder16.md#set-ci) | Set the current column index. |
| [set_mode        ](./pub_utf-decoder16.md#set-md) | Set the decoder MODE. |
| [set_symbol_index](./pub_utf-decoder16.md#set-si) | Set the current Symbol index. |
| [copy_column     ](./pub_utf-decoder16.md#copycc) | Copy the current column. |
| [current         ](./pub_utf-decoder16.md#curent) | Get the current Symbol. |
| [decode          ](./pub_utf-decoder16.md#decode) | Decode the current Symbol. |
| [reset           ](./pub_utf-decoder16.md#resets) | Reset the decoder. |

------------------------------------------------------------------------------
### <a id="decoder32">[utf32_decoder](./pub_utf-decoder32.md#header)</a>

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes      ](./pub_utf-decoder32.md#attrib) | Decoder attributes. |
| [constructor     ](./pub_utf-decoder32.md#constr) | Decoder constructors. |
| [assignment      ](./pub_utf-decoder32.md#assign) | Assignment operators. |
| [debug           ](./pub_utf-decoder32.md#debugd) | Debugging display. |
| [get_column_count](./pub_utf-decoder32.md#get-cc) | Get the total column count. |
| [get_column_index](./pub_utf-decoder32.md#get-ci) | Get the current column index. |
| [get_length      ](./pub_utf-decoder32.md#get-ln) | Get the decoder buffer length. |
| [get_mode        ](./pub_utf-decoder32.md#get-md) | Get the decoder MODE. |
| [get_offset      ](./pub_utf-decoder32.md#get-of) | Get the current offset. |
| [get_origin      ](./pub_utf-decoder32.md#get-or) | Get the first data Symbol offset. |
| [get_symbol_count](./pub_utf-decoder32.md#get-sc) | Get the total symbol count. |
| [is_combining    ](./pub_utf-decoder32.md#iscomb) | Test for combining Symbol. |
| [set_column_index](./pub_utf-decoder32.md#set-ci) | Set the current column index. |
| [set_mode        ](./pub_utf-decoder32.md#set-md) | Set the decoder MODE. |
| [set_symbol_index](./pub_utf-decoder32.md#set-si) | Set the current Symbol index. |
| [copy_column     ](./pub_utf-decoder32.md#copycc) | Copy the current column. |
| [current         ](./pub_utf-decoder32.md#curent) | Get the current Symbol. |
| [decode          ](./pub_utf-decoder32.md#decode) | Decode the current Symbol. |
| [reset           ](./pub_utf-decoder32.md#resets) | Reset the decoder. |

------------------------------------------------------------------------------
### <a id="encoder08">[utf8_encoder ](./pub_utf-encoder08.md#header)</a>

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes      ](./pub_utf-encoder08.md#attrib) | Encoder attributes. |
| [constructor     ](./pub_utf-encoder08.md#constr) | Encoder constructors. |
| [assignment      ](./pub_utf-encoder08.md#assign) | Assignment operators. |
| [debug           ](./pub_utf-encoder08.md#debugd) | Debugging display. |
| [get_column_index](./pub_utf-encoder08.md#get-ci) | Get the current column index. |
| [get_offset      ](./pub_utf-encoder08.md#get-of) | Get the current offset. |
| [encode          ](./pub_utf-encoder08.md#encode) | Encode a Symbol. |
| [reset           ](./pub_utf-encoder08.md#resets) | Reset the encoder. |

------------------------------------------------------------------------------
### <a id="encoder16">[utf16_encoder](./pub_utf-encoder16.md#header)</a>

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes      ](./pub_utf-encoder16.md#attrib) | Encoder attributes. |
| [constructor     ](./pub_utf-encoder16.md#constr) | Encoder constructors. |
| [assignment      ](./pub_utf-encoder16.md#assign) | Assignment operators. |
| [debug           ](./pub_utf-encoder16.md#debugd) | Debugging display. |
| [get_column_index](./pub_utf-encoder16.md#get-ci) | Get the current column index. |
| [get_mode        ](./pub_utf-encoder16.md#get-md) | Get the encoder MODE. |
| [get_offset      ](./pub_utf-encoder16.md#get-of) | Get the current offset. |
| [set_mode        ](./pub_utf-encoder16.md#set-md) | Set the encoder MODE. |
| [encode          ](./pub_utf-encoder16.md#encode) | Encode a Symbol. |
| [reset           ](./pub_utf-encoder16.md#resets) | Reset the encoder. |

------------------------------------------------------------------------------
### <a id="encoder32">[utf32_encoder](./pub_utf-encoder32.md#header)</a>

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes      ](./pub_utf-encoder32.md#attrib) | Encoder attributes. |
| [constructor     ](./pub_utf-encoder32.md#constr) | Encoder constructors. |
| [assignment      ](./pub_utf-encoder32.md#assign) | Assignment operators. |
| [debug           ](./pub_utf-encoder32.md#debugd) | Debugging display. |
| [get_column_index](./pub_utf-encoder32.md#get-ci) | Get the current column index. |
| [get_mode        ](./pub_utf-encoder32.md#get-md) | Get the encoder MODE. |
| [get_offset      ](./pub_utf-encoder32.md#get-of) | Get the current offset. |
| [set_mode        ](./pub_utf-encoder32.md#set-md) | Set the encoder MODE. |
| [encode          ](./pub_utf-encoder32.md#encode) | Encode a Symbol. |
| [reset           ](./pub_utf-encoder32.md#resets) | Reset the encoder. |

