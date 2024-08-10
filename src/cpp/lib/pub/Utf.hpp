//----------------------------------------------------------------------------
//
//       Copyright (C) 2021-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Utf.hpp
//
// Purpose-
//       Implement Utf.h objects: utfN_decoder, utfN_encoder .
//
// Last change date-
//       2024/07/27
//
// Implementation notes-
//       Included from and part of Utf.cpp
//
//----------------------------------------------------------------------------

namespace _LIBPUB_NAMESPACE {
//============================================================================
//
// Method-
//       utf8_decoder::utf8_decoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf8_decoder::utf8_decoder(      // Copy constructor
     const utf8_decoder& from) noexcept // Source utf8_decoder
:  buffer(from.buffer), length(from.length)
{  }

   utf8_decoder::utf8_decoder(      // Copy constructor
     const utf8_encoder& from) noexcept // Source utf8_encoder
:  buffer(from.buffer), length(from.offset)
{  }

   utf8_decoder::utf8_decoder(      // Constructor
     const utf8_t*     addr,        // Decode buffer address
     Length            size) noexcept // Decode buffer length
:  buffer(addr), length(size)
{  }

   utf8_decoder::utf8_decoder(      // Constructor
     const utf8_t*     addr) noexcept // Decode buffer address
:  buffer(addr), length(::strlen((char*)addr) + 1)
{  }

   utf8_decoder::utf8_decoder(      // Constructor
     const char*       addr,        // Decode buffer address
     Length            size) noexcept // Decode buffer length
#if 0   // This method works
:  utf8_decoder((utf8_t*)addr, size) // Invoke the utf8_t*/Length constructor
{  }                                // But don't do anything else
#elif 1 // This method also works
:  buffer((utf8_t*)addr), length(size) // Set buffer/length directly
{  }                                // But don't do anything else
#else //// WRONG
///// This creates a utf8_decoder in stack storage but
///// that just gets discarded when the constructor exits.
///// THIS utf8_decoder is left empty, as if utf8_decoder(void) was used.
{  utf8_decoder((utf8_t*)addr), length(size); }
#endif

   utf8_decoder::utf8_decoder(      // Constructor
     const char*       addr) noexcept // Decode buffer address
:  buffer((utf8_t*)addr), length(::strlen(addr) + 1)
{  }

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
utf8_decoder&                       // (Always *this)
   utf8_decoder::operator=(         // Replace value from
     const utf8_decoder&
                       from) noexcept // This decoder
{
   if( this == &from )              // Disallow self-copy
     return *this;                  // (Violates const from)

   buffer= from.buffer;
   length= from.length;
   reset();
   return *this;
}

utf8_decoder&                       // (Always *this)
   utf8_decoder::operator=(         // Replace value from
     const utf8_encoder&
                       from) noexcept // This encoder
{
   buffer= from.buffer;
   length= from.offset;
   reset();
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   utf8_decoder::debug(             // Debugging display
     const char*       info) const noexcept // Informational message
{
   traceh("utf8_decoder(%p) debug(%s)\n" // (TODO: RESTORE DEBUGF, not traceh)
          "..buffer(%p) column(%zd) offset(%zd) length(%zd)\n", this, info
         , buffer, column, offset, length);

dump(Debug::get()->get_FILE(), buffer, length); // TODO: REMOVE
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::get_column
//
// Purpose-
//       Get the current column index
//
//----------------------------------------------------------------------------
Utf::Column                         // The current Column index
   utf8_decoder::get_column( void ) const // Get current Column index
{
   if( column == OFFSET_COLUMN )
     return OFFSET_COLUMN;

   if( offset < length ) {          // If there is a current character
     if( column == Column(-1) || !is_combining(current()) )
       return column + 1;
   }

   return column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::get_points
//
// Purpose-
//       Get the total codepoint count
//
//----------------------------------------------------------------------------
Utf::Points                         // The total codepoint count
   utf8_decoder::get_points( void ) const // Get total codepoint count
{
   utf8_decoder copy(*this);
   while( copy.decode() != UTF_EOF )
     ;

   return copy.column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::set_column
//
// Purpose-
//       Set the column
//
//----------------------------------------------------------------------------
Utf::Length                         // Number of units past end of buffer
   utf8_decoder::set_column(        // Set the column
     Column            IX)          // To this column index
{
   if( IX <= column ) {
     column= -1;
     offset= 0;

     if( IX == 0 )
       return 0;
   }

   while( (column + 1) < IX ) {
     utf32_t code= decode();
     if( code == UTF_EOF ) {
       if( column == Column(-1) )   // If empty decoder
         return IX;
       return IX - column;
     }
   }

   if( offset > 0 ) {
     while( is_combining(current()) )
       decode();
   }

   if( offset < length )
     return 0;

   return IX - column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::set_offset
//
// Purpose-
//       Set the offset
//
// Implementation notes-
//       Methods decode() and set_offset() do not work well together.
//       If set_offset sets an offset the middle of a character encoding, the
//       next decode would return UNI_REPLACEMENT.
//
//----------------------------------------------------------------------------
Utf::Length                         // Number of units past end of buffer
   utf8_decoder::set_offset(        // Set the offset
     Offset            IX)          // To this offset
{
   column= OFFSET_COLUMN;           // (Invalid column)

   if( IX <= length ) {
     offset= IX;
     return 0;
   }

   offset= length;
   return IX - length;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::copy_column
//
// Purpose-
//       Copy the current column, including continuation characters
//
// Implementation note-
//       The resultant's column number is zero.
//
//----------------------------------------------------------------------------
utf8_decoder                        // The current column substring
   utf8_decoder::copy_column( void ) const // Copy the current column
{
   utf8_decoder copy;

   copy.buffer= buffer + offset;
   copy.length= length - offset;

   copy.decode();                   // (Include the current column codepoint)
   while( copy.is_combining() )     // Include combining codepoints
     copy.decode();

   copy.length= copy.offset;
   copy.column= copy.offset= 0;

   return copy;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::current
//
// Purpose-
//       Decode the current codepoint
//
//----------------------------------------------------------------------------
utf32_t                             // The current encoding
   utf8_decoder::current( void )  const // Get current encoding
{
   if( offset >= length )
     return UTF_EOF;

   utf32_t code= buffer[offset];
   if( code < 0x80 )                // If ASCII encoding
     return code;                   // (Done)

   if( code < 0xC0 || code > 0xF7 ) // If invalid start code
     return UNI_REPLACEMENT;        // Use UNI_REPLACEMENT

   // Multiple character encodings
   unsigned size= 2;                // Number of encoding characters
   if( code < 0xE0 ) {              // (0XC0 .. 0xDF)
//// size= 2;                       // Two character encoding
     code &= 0x1F;
   } else if( code < 0xF0 ) {       // (0XE0 .. 0xEF)
     size= 3;                       // Three character encoding
     code &= 0x0F;
   } else {                         // (0XF0 .. 0xF7)
     size= 4;                       // Four character encoding
     code &= 0x07;
   }

   // Decode continuation characters, rejecting invalid encodings
   if( size > (length - offset ) )
     return UNI_REPLACEMENT;

   for(unsigned i= 1; i<size; ++i) {
     int C= buffer[offset + i];
     if( C < 0x80 || C > 0xBF )
       return UNI_REPLACEMENT;

     code <<= 6;
     code  |= (C & 0x3F);
   }

   // Check for overlong encoding (code < 0x80 already handled)
   if( size == 2 ) {
     if( code < 0x0000'0080 )
       return UNI_REPLACEMENT;
   } else if( size == 3 ) {
     if( code < 0x0000'0800 )
       return UNI_REPLACEMENT;
   } else /* (size == 4) */ {
     if( code < 0x0001'0000 )
       return UNI_REPLACEMENT;
   }

   if( !is_unicode(code) )
     code= UNI_REPLACEMENT;

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::decode
//
// Purpose-
//       Decode the next codepoint, updating column and offset
//
//----------------------------------------------------------------------------
Utf::utf32_t                        // The current codepoint
   utf8_decoder::decode( void )     // Decode next codepoint
{
   if( offset >= length )
     return UTF_EOF;

   utf32_t code= buffer[offset];
   if( code < 0x80 ) {              // If ASCII encoding
     if( column != OFFSET_COLUMN )
       ++column;
     ++offset;
     return code;
   }

   if( code < 0xC0 || code > 0xF7 ) { // If invalid start code
     if( column != OFFSET_COLUMN )
       ++column;
     ++offset;
     return UNI_REPLACEMENT;        // Use UNI_REPLACEMENT
   }

   // Multiple character encodings
   unsigned size= 2;                // Number of encoding characters
   if( code < 0xE0 ) {              // (0XC0 .. 0xDF)
//// size= 2;                       // Two character encoding
     code &= 0x1F;
   } else if( code < 0xF0 ) {       // (0XE0 .. 0xEF)
     size= 3;                       // Three character encoding
     code &= 0x0F;
   } else {                         // (0XF0 .. 0xF7)
     size= 4;                       // Four character encoding
     code &= 0x07;
   }

   // Decode continuation characters, rejecting invalid encodings
   if( size > (length - offset ) ) {
     offset= length;
     return UNI_REPLACEMENT;
   }

   ++offset;                        // (Account for the lead character)
   for(unsigned i= 1; i<size; ++i) {
     int C= buffer[offset++];
     if( C < 0x80 || C > 0xBF ) {
       if( column != OFFSET_COLUMN )
         ++column;
       return UNI_REPLACEMENT;
     }

     code <<= 6;
     code  |= (C & 0x3F);
   }

   // Check for overlong encoding (size == 1 cannot be overlong)
   if( size == 2 ) {
     if( code < 0x0000'0080 )
       code= UNI_REPLACEMENT;
   } else if( size == 3 ) {
     if( code < 0x0000'0800 )
       code= UNI_REPLACEMENT;
   } else /* (size == 4) */ {
     if( code < 0x0001'0000 )
       code= UNI_REPLACEMENT;
   }

   if( !is_unicode(code) )
     code= UNI_REPLACEMENT;
   if( !is_combining(code) || column == Column(-1) ) {
     if( column != OFFSET_COLUMN )
       ++column;
   }

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::reset
//
// Purpose-
//       Reset the decoder
//
//----------------------------------------------------------------------------
void
   utf8_decoder::reset(             // Reset the decoder
     const utf8_t*     addr,        // Encoding buffer pointer
     Length            size) noexcept // Encoding buffer pointer (unit) Length
{
   if( addr == nullptr )
     size= 0;
   else if( size == 0 )
     addr= nullptr;

   buffer= addr;
   length= size;
   column= -1;
   offset= 0;
}

void
   utf8_decoder::reset( void ) noexcept // Reset the decoder to initial state
{  column= -1; offset= 0; }         // (Leaving length unchanged)

//============================================================================
//
// Method-
//       utf16_decoder::utf16_decoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf16_decoder::utf16_decoder(    // Copy constructor
     const utf16_decoder& from) noexcept // Source utf16_decoder
:  buffer(from.buffer), length(from.length), mode(from.mode)
{  reset(buffer, length, mode); }

   utf16_decoder::utf16_decoder(    // Copy constructor
     const utf16_encoder& from) noexcept // Source utf16_encoder
:  buffer(from.buffer), length(from.length), mode(from.mode)
{  reset(buffer, length, mode); }

   utf16_decoder::utf16_decoder(    // Buffer constructor
     const utf16_t*    addr,        // Buffer address
     Length            size,        // Buffer length (native units)
     MODE              mode) noexcept // Encoding mode
:  buffer(addr), length(size), mode(mode)
{  reset(buffer, length, mode); }

   utf16_decoder::utf16_decoder(    // Buffer constructor
     const utf16_t*    addr,        // Buffer address
     MODE              mode) noexcept // Encoding mode
:  buffer(addr), length(strlen(addr) + 1), mode(mode)
{  reset(buffer, length, mode); }

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
utf16_decoder&                      // (Always *this)
   utf16_decoder::operator=(        // Replace value copying (in-place)
     const utf16_decoder&
                       from) noexcept // This decoder
{
   if( this == &from )              // Disallow self-copy
     return *this;                  // (Violates const from)

   buffer= from.buffer;
   length= from.length;
   mode= from.mode;
   reset();

   return *this;
}

utf16_decoder&                      // (Always *this)
   utf16_decoder::operator=(        // Replace value copying (in-place)
     const utf16_encoder&
                       from) noexcept // This encoder
{
   buffer= from.buffer;
   length= from.offset;
   mode= from.mode;
   reset();

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   utf16_decoder::debug(            // Debugging display
     const char*       info) const noexcept // Informational message
{
   debugf("utf16_decoder(%p) debug(%s)\n"
          "..buffer(%p) column(%zd) offset(%zd) length(%zd) mode(%s)\n", this
         , info, buffer, column, offset, length, get_mode_name(mode));
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::get_column
//
// Purpose-
//       Get the current column index
//
//----------------------------------------------------------------------------
Utf::Column                         // The current Column index
   utf16_decoder::get_column( void ) const // Get current Column index
{
   if( column == OFFSET_COLUMN )
     return OFFSET_COLUMN;

   if( offset < length ) {          // If there is a current character
     if( column == Column(-1) || !is_combining(current()) )
       return column + 1;
   }

   return column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::get_origin
//
// Purpose-
//       Get the origin, accounting for BYTE_ORDER_MARK
//
//----------------------------------------------------------------------------
Utf::Offset                         // The origin, either 0 or 1
   utf16_decoder::get_origin( void ) const // Get the origin
{
   if( length == 0 )                // If no room for a BYTE_ORDER_MARK
     return 0;                      // Use default origin

   if( mode == MODE_RESET ) {       // If the mode isn't set
     if( be16toh(buffer[0]) == BYTE_ORDER_MARK
         || be16toh(buffer[0]) == MARK_ORDER_BYTE )
       return 1;

     return 0;                      // No BYTE_ORDER_MARK found
   }

   if( fetch16(buffer[0], mode) == BYTE_ORDER_MARK )
     return 1;

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::get_points
//
// Purpose-
//       Get the total codepoint count
//
//----------------------------------------------------------------------------
Utf::Points                         // The total codepoint count
   utf16_decoder::get_points( void ) const // Get total codepoint count
{
   utf16_decoder copy(*this);
   while( copy.decode() != UTF_EOF )
     ;

   return copy.column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::set_column
//
// Purpose-
//       Set the specified Column
//
//----------------------------------------------------------------------------
Utf::Length                         // The number of units past end of buffer
   utf16_decoder::set_column(       // Set column to
     Column            IX)          // This column index
{
   if( IX <= column ) {
     column= -1;
     offset= get_origin();

     if( IX == 0 )
       return 0;
   }

   while( (column + 1) < IX ) {
     utf32_t code= decode();
     if( code == UTF_EOF ) {
       if( column == Column(-1) )   // If empty decoder
         return IX;
       return IX - column;
     }
   }

   if( offset > get_origin() ) {
     while( is_combining(current()) )
       decode();
   }

   if( offset < length )
     return 0;

   return IX - column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::set_mode
//
// Purpose-
//       Set/initialize the decoder MODE
//
//----------------------------------------------------------------------------
void
   utf16_decoder::set_mode(         // Set decoding mode
     MODE              M)           // The decoding mode
{
   if( offset || M > MODE_LE ) {    // If decoding started or invalid MODE
     debugf("decoder16::set_mode(%d) offset(%zd)\n", M, offset);
     throw utf_error("set_mode usage error");
   }

   if( mode != MODE_RESET && M != mode ) { // If mode switch
     debugf("decoder16::set_mode(%d) mode(%d)\n", M, mode);
     throw utf_error("set_mode usage error");
   }

   mode= M;
}

int                                 // Return code, 0 OK
   utf16_decoder::set_mode( void )  // Initialize decoding mode
{
   if( mode != MODE_RESET )         // If "already initialized" error
     return -1;                     // (MODE remains unchanged)

   if( offset > 1 )                 // If "decoding started" error
     return -2;                     // (MODE remains MODE_RESET, big endian)

   // If a little endian BYTE_ORDER_MARK is found, set little endian mode
   if( length > 0 && be16toh(buffer[0]) == MARK_ORDER_BYTE )
     mode= MODE_LE;               // Set little endian mode
   else                           // (May have a big endian BYTE_ORDER_MARK)
     mode= MODE_BE;               // Use the default mode

   return 0;                      // Mode initialized
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::set_offset
//
// Purpose-
//       Set the specified Column
//
// Implementation notes-
//       Methods decode() and set_offset() do not work well together.
//       If set_offset sets an offset in the middle of a surrogate pair, a
//       following decode would return UNI_REPLACEMENT.
//
//----------------------------------------------------------------------------
Utf::Length                         // The number of units past end of buffer
   utf16_decoder::set_offset(       // Set offset to
     Offset            IX)          // This offset
{
   column= OFFSET_COLUMN;           // (Invalid column)

   Offset origin= get_origin();
   if( IX < origin )
     IX= origin;

   if( IX <= length ) {
     offset= IX;
     return 0;
   }

   offset= length;
   return IX - length;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::copy_column
//
// Purpose-
//       Copy the current column, including continuation characters
//
// Implementation note-
//       The resultant's column number is zero.
//
//----------------------------------------------------------------------------
utf16_decoder                        // The current column substring
   utf16_decoder::copy_column( void ) const // Copy the current column
{
   utf16_decoder copy;

   copy.buffer= buffer + offset;
   copy.length= length - offset;

   copy.decode();                   // (Include the current column codepoint)
   while( copy.is_combining() )     // Include combining codepoints
     copy.decode();

   copy.length= copy.offset;
   copy.column= copy.offset= 0;

   return copy;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::current
//
// Purpose-
//       Get the current codepoint
//
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   utf16_decoder::current( void ) const // Get current codepoint
{
   if( offset >= length )
     return UTF_EOF;

   utf32_t code= fetch16(buffer[offset], mode);
   if( code < 0x00'D800 || code >= 0x00'E000 ) // If standard encoding
     return code;

   // Surrogate pair encoding
   if( code >= 0x00'DC00 )          // Second half of encoding first: ERROR
     return UNI_REPLACEMENT;

   if( 2 > (length - offset ) )     // If second half missing (not in buffer)
     return UNI_REPLACEMENT;

   utf32_t half= fetch16(buffer[offset+1], mode); // Get second half of pair
   if( half < 0x00'DC00 || half >= 0x00'E000 ) // If second half invalid
     return UNI_REPLACEMENT;

   // Resultant is always in unicode range but never in surrogate pair range.
   code= 0x01'0000 + ((code & 0x00'03FF) << 10 | (half & 0x00'03FF));

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::decode
//
// Purpose-
//       Decode the current codepoint, updating column and offset
//
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   utf16_decoder::decode( void )    // Decode the current codepoint
{
   if( offset >= length )
     return UTF_EOF;

   utf32_t code= fetch16(buffer[offset], mode);
   if( code < 0x00'D800 || code >= 0x00'E000 ) { // If standard encoding
     ++offset;
     if( !is_combining(code) || column == Column(-1) ) {
       if( column != OFFSET_COLUMN )
         ++column;
     }
     return code;
   }

   // Surrogate pair encoding
   if( code >= 0x00'DC00 ) {        // Second half of encoding first: ERROR
     ++offset;
     if( column != OFFSET_COLUMN )
       ++column;
     return UNI_REPLACEMENT;
   }

   if( 2 > (length - offset ) ) {   // If second half missing (not in buffer)
     ++offset;
     if( column != OFFSET_COLUMN )
       ++column;
     return UNI_REPLACEMENT;
   }

   utf32_t half= fetch16(buffer[offset+1], mode); // Get second half of pair
   if( half < 0x00'DC00 || half >= 0x00'E000 ) { // If second half invalid
     ++offset;
     if( column != OFFSET_COLUMN )
       ++column;
     return UNI_REPLACEMENT;
   }

   // Resultant is always in unicode range but never in surrogate pair range.
   code= 0x01'0000 + ((code & 0x00'03FF) << 10 | (half & 0x00'03FF));
   if( !is_combining(code) || column == Column(-1) ) {
     if( column != OFFSET_COLUMN )
       ++column;
   }
   offset += 2;

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::reset
//
// Purpose-
//       Reset the decoder
//
//----------------------------------------------------------------------------
void
   utf16_decoder::reset(            // Reset the decoder
     const utf16_t*    addr,        // Buffer address
     Length            size,        // Buffer length (in units)
     MODE              mode) noexcept // The decoding mode
{
   if( addr == nullptr )
     size= 0;
   else if( size == 0 )
     addr= nullptr;

   buffer= addr;
   length= size;
   this->mode= mode;
   column= -1;
   offset= get_origin();
}

void
   utf16_decoder::reset( void ) noexcept // Reset the decoder to initial state
{  column= -1; offset= get_origin(); } // (Leaving length and mode unchanged)

//============================================================================
//
// Method-
//       utf32_decoder::utf32_decoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf32_decoder::utf32_decoder(    // Copy constructor
     const utf32_decoder& from) noexcept // Source utf32_decoder
:  buffer(from.buffer), length(from.length), mode(from.mode)
{  reset(buffer, length, mode); }

   utf32_decoder::utf32_decoder(    // Copy constructor
     const utf32_encoder& from) noexcept // Source utf32_encoder
:  buffer(from.buffer), length(from.length), mode(from.mode)
{  reset(buffer, length, mode); }

   utf32_decoder::utf32_decoder(    // Buffer constructor
     const utf32_t*    addr,        // Buffer address
     Length            size,        // Buffer length (native units)
     MODE              mode) noexcept // Encoding mode
:  buffer(addr), length(size), mode(mode)
{  reset(buffer, length, mode); }

   utf32_decoder::utf32_decoder(    // Buffer constructor
     const utf32_t*    addr,        // Buffer address
     MODE              mode) noexcept // Encoding mode
:  buffer(addr), length(strlen(addr) + 1), mode(mode)
{  reset(buffer, length, mode); }

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
utf32_decoder&                      // (Always *this)
   utf32_decoder::operator=(        // Replace value copying (in-place)
     const utf32_decoder&
                       from) noexcept // This decoder
{
   if( this == &from )              // Disallow self-copy
     return *this;                  // (Violates const from)

   buffer= from.buffer;
   length= from.length;
   mode= from.mode;
   reset();

   return *this;
}

utf32_decoder&                      // (Always *this)
   utf32_decoder::operator=(        // Replace value copying (in-place)
     const utf32_encoder&
                       from) noexcept // This encoder
{
   buffer= from.buffer;
   length= from.offset;
   mode= from.mode;
   reset();

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   utf32_decoder::debug(            // Debugging display
     const char*       info) const noexcept // Informational message
{
   debugf("utf32_decoder(%p) debug(%s)\n"
          "..buffer(%p) column(%zd) offset(%zd) length(%zd) mode(%s)\n", this
         , info, buffer, column, offset, length, get_mode_name(mode));
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::get_column
//
// Purpose-
//       Get the current column index
//
//----------------------------------------------------------------------------
Utf::Column                         // The current Column index
   utf32_decoder::get_column( void ) const // Get current Column index
{
   if( column == OFFSET_COLUMN )
     return OFFSET_COLUMN;

   if( offset < length ) {          // If there is a current character
     if( column == Column(-1) || !is_combining(current()) )
       return column + 1;
   }

   return column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::get_origin
//
// Purpose-
//       Get the origin, accounting for BYTE_ORDER_MARK
//
//----------------------------------------------------------------------------
Utf::Offset                         // The origin, either 0 or 1
   utf32_decoder::get_origin( void ) const // Get the origin
{
   if( length == 0 )                // If no room for a BYTE_ORDER_MARK
     return 0;                      // Use default origin

   if( mode == MODE_RESET ) {       // If the mode isn't set
     if( be32toh(buffer[0]) == BYTE_ORDER_MARK32
         || be32toh(buffer[0]) == MARK_ORDER_BYTE32 )
       return 1;

     return 0;                      // No BYTE_ORDER_MARK found
   }

   if( fetch32(buffer[0], mode) == BYTE_ORDER_MARK32 )
     return 1;

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::get_points
//
// Purpose-
//       Get the total codepoint count
//
//----------------------------------------------------------------------------
Utf::Points                         // The total codepoint count
   utf32_decoder::get_points( void ) const // Get total codepoint count
{
   utf32_decoder copy(*this);
   while( copy.decode() != UTF_EOF )
     ;

   return copy.column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::set_column
//
// Purpose-
//       Set the specified Column
//
//----------------------------------------------------------------------------
Utf::Length                         // The number of (units) past end
   utf32_decoder::set_column(       // Set column to
     Column            IX)          // This column index
{
   if( IX <= column ) {
     column= -1;
     offset= get_origin();

     if( IX == 0 )
       return 0;
   }

   while( (column + 1) < IX ) {
     utf32_t code= decode();
     if( code == UTF_EOF ) {
       if( column == Column(-1) )   // If empty decoder
         return IX;
       return IX - column;
     }
   }

   if( offset > get_origin() ) {
     while( is_combining(current()) )
       decode();
   }

   if( offset < length )
     return 0;

   return IX - column;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::set_mode
//
// Purpose-
//       Set the decoder MODE
//
//----------------------------------------------------------------------------
void
   utf32_decoder::set_mode(         // Set decoding mode
     MODE              M)           // The decoding mode
{
   if( offset || M > MODE_LE ) {    // If decoding started or invalid MODE
     debugf("decoder32::set_mode(%d) offset(%zd)\n", M, offset);
     throw utf_error("set_mode usage error");
   }

   if( mode != MODE_RESET && M != mode ) { // If mode switch
     debugf("decoder32::set_mode(%d) mode(%d)\n", M, mode);
     throw utf_error("set_mode usage error");
   }

   mode= M;
}

int                                 // Return code, 0 OK
   utf32_decoder::set_mode( void )  // Initialize decoding mode
{
   if( mode != MODE_RESET )         // If "already initialized" error
     return -1;                     // (MODE remains unchanged)

   if( offset > 1 )                 // If "decoding started" error
     return -2;                     // (MODE remains MODE_RESET, big endian)

   // If a little endian BYTE_ORDER_MARK is found, set little endian mode
   if( length > 0 && be32toh(buffer[0]) == MARK_ORDER_BYTE32 )
     mode= MODE_LE;               // Set little endian mode
   else                           // (May have a big endian BYTE_ORDER_MARK)
     mode= MODE_BE;               // Use the default mode

   return 0;                      // Mode initialized
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::set_offset
//
// Purpose-
//       Set the specified Column
//
//----------------------------------------------------------------------------
Utf::Length                         // The number of (units) past end
   utf32_decoder::set_offset(       // Set offset to
     Offset            IX)          // This offset
{
   column= OFFSET_COLUMN;           // (Invalid column)

   Offset origin= get_origin();
   if( IX < origin )
     IX= origin;

   if( IX <= length ) {
     offset= IX;
     return 0;
   }

   offset= length;
   return IX - length;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::copy_column
//
// Purpose-
//       Copy the current column, including continuation characters
//
// Implementation note-
//       The resultant's column number is zero.
//
//----------------------------------------------------------------------------
utf32_decoder                        // The current column substring
   utf32_decoder::copy_column( void ) const // Copy the current column
{
   utf32_decoder copy;

   copy.buffer= buffer + offset;
   copy.length= length - offset;

   copy.decode();                   // (Include the current column codepoint)
   while( copy.is_combining() )     // Include combining codepoints
     copy.decode();

   copy.length= copy.offset;
   copy.column= copy.offset= 0;

   return copy;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::current
//
// Purpose-
//       Get the current codepoint
//
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   utf32_decoder::current( void ) const // Get current codepoint
{
   if( offset >= length )
     return UTF_EOF;

   utf32_t code= fetch32(buffer[offset], mode);
   if( !is_unicode(code) )
     code= UNI_REPLACEMENT;

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::decode
//
// Purpose-
//       Decode the current codepoint, updating column and offset
//
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   utf32_decoder::decode( void )    // Decode the current codepoint
{
   if( offset >= length )           // If EOF position
     return UTF_EOF;

   utf32_t code= fetch32(buffer[offset++], mode);
   if( !is_unicode(code) )
     code= UNI_REPLACEMENT;

   if( !is_combining(code) || column == Column(-1) ) {
     if( column != OFFSET_COLUMN )
       ++column;
   }

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::reset
//
// Purpose-
//       Reset the decoder
//
//----------------------------------------------------------------------------
void
   utf32_decoder::reset(            // Reset the decoder
     const utf32_t*    addr,        // Buffer address
     Length            size,        // Buffer length (in units)
     MODE              mode) noexcept // The decoding mode
{
   if( addr == nullptr )
     size= 0;
   else if( size == 0 )
     addr= nullptr;

   buffer= addr;
   length= size;
   this->mode= mode;
   column= -1;
   offset= get_origin();
}

void
   utf32_decoder::reset( void ) noexcept // Reset the decoder to initial state
{  column= -1; offset= get_origin(); } // (Leaving length and mode unchanged)

//============================================================================
//
// Method-
//       utf8_encoder::utf8_encoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf8_encoder::utf8_encoder(      // Address/length encoder
     utf8_t*           addr,        // Address
     Length            size) noexcept // Length
:  buffer(addr), length(size)
{  debugf("utf8_encoder(%p).!(%p, %zd)\n", this, addr, size); }

//----------------------------------------------------------------------------
//
// Method-
//       utf8_encoder::operator=
//
// Purpose-
//       Assignment operators
//
// Implementation notes-
//       We *DO NOT COPY* utf16_t or utf32_t initial BYTE_ORDER_MARKs.
//
//----------------------------------------------------------------------------
utf8_encoder&                       // (Always *this)
   utf8_encoder::operator=(         // Replace value copying
     const utf8_decoder&
                       from) noexcept(false) // This decoder
{
   if( buffer == from.buffer ) {    // If we're using the same buffer
     if( from.get_length() > length ) // If buffer length mismatch
       throw utf_error("operator=() incomplete"); // (offset > length) error

     column= from.get_points();
     offset= from.get_length();
     return *this;
   }

   utf8_decoder copy(from);         // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();
   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

utf8_encoder&                       // (Always *this)
   utf8_encoder::operator=(         // Replace value copying
     const utf16_decoder&
                       from) noexcept(false) // This decoder
{
   utf16_decoder copy(from);        // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();
   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

utf8_encoder&                       // (Always *this)
   utf8_encoder::operator=(         // Replace value copying
     const utf32_decoder&
                       from) noexcept(false) // This decoder
{
   utf32_decoder copy(from);        // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();
   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_encoder::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   utf8_encoder::debug(             // Debugging display
     const char*       info) const noexcept // Informational message
{
   debugf("utf8_encoder(%p) debug(%s)\n"
          "..buffer(%p) column(%zd) offset(%zd) length(%zd)\n", this, info
         , buffer, column, offset, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_encoder::utf8_encode
//
// Purpose-
//       Encode one codepoint
//
// Implementation notes-
//       Currently no combining codepoints have 4 character encodings, but
//       who knows what tomorrow brings?
//       We check whether 4 character encodings are combining anyway.
//
//----------------------------------------------------------------------------
unsigned                            // The encoding length
   utf8_encoder::encode(            // Encode
     utf32_t           code)        // This codepoint
{
   if( offset >= length )           // If buffer full
     return 0;

   if( code < 0x0000'0080 ) {       // Single byte encoding
     buffer[offset++]= (utf8_t)code;
     ++column;
     return 1;
   }

   Length left= length - offset;    // The available buffer length
   if( code < 0x0000'0800 ) {       // Two byte encoding
     if( left < 2 )
       return 0;

     buffer[offset + 1]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buffer[offset + 0]= (utf8_t)(code | 0xC0);
     if( offset == 0 || !is_combining(code) )
       ++column;
     offset += 2;
     return 2;
   }

   if( !is_unicode(code) )          // If invalid codepoint
     code= UNI_REPLACEMENT;         // Use replacement codepoint
   if( code < 0x0001'0000 ) {       // Three byte encoding
     if( left < 3 )
       return 0;

     buffer[offset + 2]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buffer[offset + 1]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buffer[offset + 0]= (utf8_t)(code | 0xE0);
     if( offset == 0 || !is_combining(code) )
       ++column;
     offset += 3;
     return 3;
   }

   // Four byte encoding
   if( left < 4 )
     return 0;

   buffer[offset + 3]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buffer[offset + 2]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buffer[offset + 1]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buffer[offset + 0]= (utf8_t)(code | 0xF0);
   if( !is_combining(code) || column == 0 )
     ++column;
   offset += 4;
   return 4;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_encoder::reset
//
// Purpose-
//       Reset the encoder
//
//----------------------------------------------------------------------------
void
   utf8_encoder::reset(             // Reset the encoder
     utf8_t*           addr,        // Encoding buffer pointer
     Length            size) noexcept // Encoding buffer pointer (unit) Length
{
   if( addr == nullptr )
     size= 0;
   else if( size == 0 )
     addr= nullptr;

   buffer= addr;
   length= size;
   column= -1;
   offset= 0;
}

void
   utf8_encoder::reset( void ) noexcept // Reset the encoder to initial state
{  column= -1; offset= 0; }         // (Leaving length unchanged)

//============================================================================
//
// Method-
//       utf16_encoder::utf16_encoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf16_encoder::utf16_encoder(    // Buffer constructor
     utf16_t*          addr,        // Buffer address
     Length            size,        // Buffer length (native units)
     MODE              mode) noexcept // Encoding mode
:  buffer(addr), length(size), mode(mode)
{  reset(buffer, length, mode); }

//----------------------------------------------------------------------------
//
// Method-
//       utf16_encoder::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
utf16_encoder&                      // (Always *this)
   utf16_encoder::operator=(        // Replace value copying
     const utf8_decoder&
                       from) noexcept(false) // This decoder
{
   utf8_decoder copy(from);         // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();
   mode= MODE_BE;
   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

utf16_encoder&                      // (Always *this)
   utf16_encoder::operator=(        // Replace value copying
     const utf16_decoder&
                       from) noexcept(false) // This decoder
{
   mode= from.get_mode();

   if( buffer == from.buffer ) {    // If we're using the same buffer
     if( from.get_length() > length ) // If buffer length mismatch
       throw utf_error("operator=() incomplete"); // (offset > length) error

     column= from.get_points();
     offset= from.get_length();
     return *this;
   }

   utf16_decoder copy(from);        // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();                         // {column= -1; offset= 0;}
   if( from.get_origin() != 0 )     // If BYTE_ORDER_MARK present
     encode(BYTE_ORDER_MARK);

   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

utf16_encoder&                      // (Always *this)
   utf16_encoder::operator=(        // Replace value copying
     const utf32_decoder&
                       from) noexcept(false) // This decoder
{
   mode= from.get_mode();

   utf32_decoder copy(from);        // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();                         // {column= -1; offset= 0;}
   if( from.get_origin() != 0 )     // If BYTE_ORDER_MARK present
     encode(BYTE_ORDER_MARK);

   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_encoder::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   utf16_encoder::debug(            // Debugging display
     const char*       info) const noexcept // Informational message
{
   debugf("utf16_encoder(%p) debug(%s)\n"
          "..buffer(%p) column(%zd) offset(%zd) length(%zd) mode(%s)\n", this
         , info, buffer, column, offset, length, get_mode_name(mode));
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_encoder::set_mode
//
// Purpose-
//       Set the decoder MODE
//
//----------------------------------------------------------------------------
void
   utf16_encoder::set_mode(         // Set encoding mode
     MODE              M)           // The encoding mode
{
   if( offset || M > MODE_LE ) {    // If encoding started or invalid MODE
     debugf("encoder16::set_mode(%d) offset(%zd)\n", M, offset);
     throw utf_error("set_mode usage error");
   }

   mode= M;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_encoder::encode
//
// Purpose-
//       Encode a codepoint
//
//----------------------------------------------------------------------------
unsigned                            // The encoding length, in units
   utf16_encoder::encode(           // Encode
     utf32_t           code)        // This codepoint
{
   if( !is_unicode(code) )          // If codepoint is invalid
     code= UNI_REPLACEMENT;         // Encode replacement character instead

   if( column == Column(-1) ) {
     if( offset == 0 && code == BYTE_ORDER_MARK ) {
       buffer[offset++]= store16(code, mode);
       return 1;
     }
   }

   if( code < 0x01'0000 ) {
     if( offset >= length )
       return 0;

     if( !is_combining(code) || column == Column(-1) )
       ++column;

     buffer[offset++]= store16((utf16_t)code, mode);

     return 1;
   }

   if( (length - offset) < 2 )
     return 0;

   if( !is_combining(code) || column == Column(-1) )
     ++column;

   code -= 0x01'0000;
   buffer[offset + 1]= store16((code & 0x00'03ff) | 0x00'DC00, mode);
   code >>= 10;
   buffer[offset + 0]= store16((code & 0x00'03ff) | 0x00'D800, mode);
   offset += 2;

   return 2;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_encoder::reset
//
// Purpose-
//       Reset the encoder
//
//----------------------------------------------------------------------------
void
   utf16_encoder::reset(            // Reset the encoder
     utf16_t*          addr,        // Buffer address
     Length            size,        // Buffer length (in units)
     MODE              mode) noexcept // The decoding mode
{
   if( addr == nullptr )
     size= 0;
   else if( size == 0 )
     addr= nullptr;

   buffer= addr;
   length= size;
   column= -1;
   offset= 0;
   this->mode= mode;
}

void
   utf16_encoder::reset( void ) noexcept // Reset the encoder to initial state
{  column= -1; offset= 0; }         // (Leaving length and mode unchanged)

//============================================================================
//
// Method-
//       utf32_encoder::utf32_encoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf32_encoder::utf32_encoder(    // Buffer constructor
     utf32_t*          addr,        // Buffer address
     Length            size,        // Buffer length (native units)
     MODE              mode) noexcept // Encoding mode
:  buffer(addr), length(size), mode(mode)
{  reset(buffer, length, mode); }

//----------------------------------------------------------------------------
//
// Method-
//       utf32_encoder::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
utf32_encoder&                      // (Always *this)
   utf32_encoder::operator=(        // Replace value copying
     const utf8_decoder&
                       from) noexcept(false) // This decoder
{
   utf8_decoder copy(from);         // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();
   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

utf32_encoder&                      // (Always *this)
   utf32_encoder::operator=(        // Replace value copying
     const utf16_decoder&
                       from) noexcept(false) // This decoder
{
   mode= from.get_mode();

   utf16_decoder copy(from);        // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();                         // {column= -1; offset= 0;}
   if( from.get_origin() != 0 )     // If BYTE_ORDER_MARK present
     encode(BYTE_ORDER_MARK);
   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

utf32_encoder&                      // (Always *this)
   utf32_encoder::operator=(        // Replace value copying
     const utf32_decoder&
                       from) noexcept(false) // This decoder
{
   mode= from.get_mode();

   if( buffer == from.buffer ) {    // If we're using the same buffer
     if( from.get_length() > length ) // If buffer length mismatch
       throw utf_error("operator=() incomplete"); // (offset > length) error

     column= from.get_points();
     offset= from.get_length();
     return *this;
   }

   utf32_decoder copy(from);        // Copy the decoder (so from's constant)

   // Copy the copied decoder, column by column
   reset();                         // {column= -1; offset= 0;}
   if( from.get_origin() != 0 )     // If BYTE_ORDER_MARK present
     encode(BYTE_ORDER_MARK);
   for(;;) {
     utf32_t code= copy.decode();   // Decode the next codepoint
     if( code == UTF_EOF )          // If EOF
       break;

     if( encode(code) == 0 )        // If not enough room
       throw utf_error("operator=() incomplete");
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_encoder::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   utf32_encoder::debug(            // Debugging display
     const char*       info) const noexcept // Informational message
{
   debugf("utf32_encoder(%p) debug(%s)\n"
          "..buffer(%p) column(%zd) offset(%zd) length(%zd) mode(%s)\n", this
         , info, buffer, column, offset, length, get_mode_name(mode));
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_encoder::set_mode
//
// Purpose-
//       Set the decoder MODE
//
//----------------------------------------------------------------------------
void
   utf32_encoder::set_mode(         // Set encoding mode
     MODE              M)           // The encoding mode
{
   if( offset || M > MODE_LE ) {    // If encoding started or invalid MODE
     debugf("encoder32::set_mode(%d) offset(%zd)\n", M, offset);
     throw utf_error("set_mode usage error");
   }

   mode= M;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_encoder::encode
//
// Purpose-
//       Encode a codepoint
//
//----------------------------------------------------------------------------
unsigned                            // The encoding length, in units
   utf32_encoder::encode(           // Encode
     utf32_t           code)        // This codepoint
{
   if( offset >= length )           // If buffer full
     return 0;

   if( !is_unicode(code) )          // If codepoint is invalid
     code= UNI_REPLACEMENT;         // Encode replacement character instead

   if( column == Column(-1) ) {
     if( offset == 0 && code == BYTE_ORDER_MARK32 ) {
       buffer[offset++]= store32(code, mode);
       return 1;
     }

     column= 0;                     // First column, never combining
   } else if( !is_combining(code) ) { // Not first column, check combining
     ++column;
   }

   buffer[offset++]= store32(code, mode);
   return 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_encoder::reset
//
// Purpose-
//       Reset the encoder
//
//----------------------------------------------------------------------------
void
   utf32_encoder::reset(            // Reset the encoder
     utf32_t*          addr,        // Buffer address
     Length            size,        // Buffer length (in units)
     MODE              mode) noexcept // The encoding mode
{
   if( addr == nullptr )
     size= 0;
   else if( size == 0 )
     addr= nullptr;

   buffer= addr;
   length= size;
   column= -1;
   offset= 0;
   this->mode= mode;
}

void
   utf32_encoder::reset( void ) noexcept // Reset the encoder to initial state
{  column= -1; offset= 0; }         // (Leaving length and mode unchanged)
}  // namespace _LIBPUB_NAMESPACE
