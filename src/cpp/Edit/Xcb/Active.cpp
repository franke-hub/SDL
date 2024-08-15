//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Active.cpp
//
// Purpose-
//       Implement Active.h
//
// Last change date-
//       2024/08/14
//
//----------------------------------------------------------------------------
#include <string.h>                 // For memcpy, memmove, strlen

#include <pub/Debug.h>              // For pub::Debug object
#include <pub/Must.h>               // For pub::must methods
#include <pub/Utf.h>                // For pub::Utf methods and objects
#include <pub/utility.h>            // For pub::utility::dump

#include "Active.h"                 // Implementation class
#include "Config.h"                 // For namespace config
#include "EdOpts.h"                 // For Editor options
#include "EdType.h"                 // For Editor types

#define PUB _LIBPUB_NAMESPACE
using namespace config;             // For config::opt_hcdm
using namespace PUB::debugging;     // For debugging
using namespace PUB;                // For namespace must::
using PUB::utility::dump;           // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum { BUFFER_SIZE= 2048 };         // Default buffer/expansion size (2**N)

//----------------------------------------------------------------------------
// Enumerations, typedefs and imports
//----------------------------------------------------------------------------
using pub::Utf;                     // For Utf types and methods

//----------------------------------------------------------------------------
//
// Method-
//       Active::~Active
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Active::~Active( void )          // Destructor
{
   if( opt_hcdm )
     traceh("Active(%p)::~Active\n", this);

   must::free(buffer);
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::Active
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Active::Active( void )           // Constructor
:  source(""), buffer_size(BUFFER_SIZE), buffer_used(0)
,  decoder(), encoder()
{
   if( opt_hcdm )
     traceh("Active(%p)::Active\n", this);

   buffer= (char*)must::malloc(buffer_size);
   fsm= FSM_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Active::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   if( info ) traceh("Active(%p)::debug(%s) FSM(%d)\n", this, info, fsm);
   traceh("..source(%p) buffer(%p) buffer_used(%zd) buffer_size(%zd)\n"
         , source, buffer, buffer_used, buffer_size);
   traceh("..source(%3zd.%s)\n", strlen(source), source);
   if( fsm != FSM_RESET ) {
     buffer[buffer_used]= '\0';     // (Buffer is mutable)
     traceh("..buffer(%3zd.%s)\n", buffer_used, buffer);
   }
   decoder.debug("Active::debug");  // (Last temporary) decoder
   encoder.debug("Active::debug");  // (Last temporary) encoder
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::get_buffer
//       Active::get_changed
//
// Purpose-
//       (Unconditionally) access the buffer, leaving trailing blanks.
//       Access the buffer if changed. If unchanged, return nullptr.
//
//----------------------------------------------------------------------------
const char*                         // The current buffer
   Active::get_buffer(              // Get '\0' delimited buffer
     Column            column)      // Starting at this Column
{  return get_column(column); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char*                         // The changed text, nullptr if unchanged
   Active::get_changed( void )      // Get changed text string
{
   if( fsm != FSM_CHANGED )         // If unchanged
     return nullptr;

   return truncate();               // Return truncated buffer
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::get_column
//       Active::get_offset
//
// Purpose-
//       (Unconditionally) access the buffer, leaving trailing blanks.
//       (Unconditionally) access the buffer, leaving trailing blanks.
//
//----------------------------------------------------------------------------
const char*                         // The buffer, beginning at column
   Active::get_column(              // Get '\0' delimited buffer
     Column            column)      // Starting at this Column
{
   fetch(column);                   // Load the buffer (May expand buffer)

   decoder.reset((const utf8_t*)buffer, buffer_used);
   Length length= decoder.set_column(column);
   if( length ) {
     fetch(buffer_used + length + 1); // Reload the buffer (Expands buffer)
     decoder.reset((const utf8_t*)buffer, buffer_used); // (Use expanded buffer)
     decoder.set_column(column);
   }

   return buffer + decoder.get_offset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char*                         // The buffer, beginning at offset
   Active::get_offset(              // Get '\0' delimited buffer
     Offset            offset)      // Starting at this Offset
{
   fetch(offset + 1);
   return buffer + offset;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::get_used
//
// Purpose-
//       Return the buffer length (including trailing blanks, if present)
//
//----------------------------------------------------------------------------
Active::Length                      // The current buffer used length
   Active::get_used( void )         // Get current buffer used length
{  fetch(); return buffer_used; }

//----------------------------------------------------------------------------
//
// Method-
//       Active::append_text
//
// Purpose-
//       Concatenate text substring.
//
//----------------------------------------------------------------------------
void
   Active::append_text(             // Concatenate text
     const char*       join,        // The join substring
     Length            size)        // The substring Length
{
   if( size == 0 )                  // If nothing to insert
     return;                        // (Line unchanged)

   fetch();                         // (Initialize buffer_used)
   expand(buffer_used + size + 1);  // Insure room for concatenation
   memcpy(buffer + buffer_used, join, size); // Concatenate
   buffer_used += size;
   buffer[buffer_used]= '\0';
   fsm= FSM_CHANGED;
}

void
   Active::append_text(             // Concatenate text
     const char*       join)        // The join string
{  append_text(join, strlen(join)); }

//----------------------------------------------------------------------------
//
// Method-
//       Active::expand
//
// Purpose-
//       Expand the buffer to the appropriate character size. (Does not fetch)
//
//----------------------------------------------------------------------------
void
   Active::expand(                  // Expand the buffer
     Length            length)      // To this length (+1)
{  if( opt_hcdm )
     traceh("Active(%p)::expand(%zd) [%zd,%zd]\n", this, length, buffer_used
           , buffer_size);

   if( length >= buffer_size ) {    // If expansion required
     size_t replace_size= length + BUFFER_SIZE;
     replace_size &= ~(BUFFER_SIZE - 1);
     char* replace= (char*)must::malloc(replace_size);
     if( fsm != FSM_RESET )
       memcpy(replace, buffer, buffer_used+1);
     must::free(buffer);
     buffer= replace;
     buffer_size= replace_size;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::fetch
//
// Purpose-
//       Fetch the line and/or expand the buffer to the appropriate size.
//
// Implementation note-
//       Use index() method to fetch and fill to column.
//
//----------------------------------------------------------------------------
void
   Active::fetch(                   // Fetch the text
     Length            length)      // With blank fill to this length
{
   if( fsm == FSM_RESET )
     buffer_used= strlen(source);

   size_t buffer_need= buffer_used + 1;
   if( length >= buffer_need )
     buffer_need= length + 1;
   if( buffer_need >= buffer_size ) // If expansion required
     expand(buffer_need);

   if( fsm == FSM_RESET ) {         // If not fetched yet
     fsm= FSM_FETCHED;              // Fetch it now
     memcpy(buffer, source, buffer_used);
     buffer[buffer_used]= '\0';
   }

   if( buffer_used < length ) {     // If expansion required
//   fsm= FSM_CHANGED;              // (Blank fill DOES NOT imply change)
     memset(buffer + buffer_used, ' ', length + 1 - buffer_used);
     buffer_used= length + 1;
     buffer[buffer_used]= '\0';
   }

   if( opt_hcdm )
     traceh("Active(%p)::fetch(%zd) [%zd/%zd]\n", this, length
           , buffer_used, buffer_size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::index
//
// Purpose-
//       Get offset of character at column index, with fetch and fill.
//
//----------------------------------------------------------------------------
Active::Offset                      // The character Offset
   Active::index(                   // Get character Offset of
     Column            column)      // This Column
{
   fetch(column);                   // Load the buffer (May expand buffer)

   decoder.reset((const utf8_t*)buffer, buffer_used);
   Length length= decoder.set_column(column);
   if( length ) {
     fetch(buffer_used + length + 1); // Reload the buffer (Expands buffer)
     decoder.reset((const utf8_t*)buffer, buffer_used); // (Use expanded buffer)
     decoder.set_column(column);
   }

   return decoder.get_offset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::insert_char
//
// Purpose-
//       Insert a (UTF-32) character using UTF-88 encoding.
//
//----------------------------------------------------------------------------
void
   Active::insert_char(             // Insert character
     Column            column,      // The current column
     int               code)        // The insert character
{
   if( code == 0 )                  // Don't insert null characters
     return;
   if( !pub::Utf::is_unicode(code) ) // Subtitute UNI_REPLACEMENT if invalid
     code= pub::Utf::UNI_REPLACEMENT;

   char insert_buff[8];             // The insert character encoder buffer
   encoder.reset((utf8_t*)insert_buff, sizeof(insert_buff));
   encoder.encode(code);
   replace_text(column, 0, insert_buff, encoder.get_offset());
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::insert_text
//
// Purpose-
//       Insert a text string.
//
//----------------------------------------------------------------------------
void
   Active::insert_text(             // Insert text
     Column            column,      // The insert column
     const char*       text)        // The insert text
{  replace_text(column, 0, text); }

//----------------------------------------------------------------------------
//
// Method-
//       Active::remove_char
//
// Purpose-
//       Remove a character.
//
//----------------------------------------------------------------------------
void
   Active::remove_char(             // Remove the character
     Column            column)      // At this column
{  replace_text(column, 1, ""); }   // (Replace it with nothing)

//----------------------------------------------------------------------------
//
// Method-
//       Active::replace_char
//
// Purpose-
//       Replace a character.
//
//----------------------------------------------------------------------------
void
   Active::replace_char(            // Replace the character
     Column            column,      // At this column
     int               code)        // With this character
{
   if( code == 0 )                  // Don't insert null characters
     return;
   if( !pub::Utf::is_unicode(code) ) // Subtitute UNI_REPLACEMENT if invalid
     code= pub::Utf::UNI_REPLACEMENT;

   char insert_buff[8];             // The insert character encoder buffer
   encoder.reset((utf8_t*)insert_buff, sizeof(insert_buff));
   encoder.encode(code);
   replace_text(column, 1, insert_buff, encoder.get_offset());
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::replace_text
//
// Purpose-
//       Replace (or insert) a text string.
//
//----------------------------------------------------------------------------
void
   Active::replace_text(            // Replace (or insert) text
     Column            column,      // The replacement Column
     Points            points,      // The replacement (delete) Column count
     const char*       text,        // The replacement (insert) text
     Length            insert)      // The replacement (insert) text Length
{
   Offset origin= index(column);    // The origin offset (+ blank fill)
   Length remove= 0;                // Removal length, in bytes
   if( points )
     remove= index(column + points) - origin;
   Length remain= buffer_used - (origin + remove); // Trailing text length
   fetch(origin + insert + remain); // If necessary, expand the buffer
   if( insert || remove ) {         // If inserting or removing text
     if( remain )                   // If text remains after removal
       memmove(buffer + origin + insert, buffer + origin + remove, remain);
     if( insert )                   // If inserting text
       memmove(buffer + origin, text, insert);

     fsm= FSM_CHANGED;
   }

   buffer_used= origin + insert + remain;
   buffer[buffer_used]= '\0';
}

void
   Active::replace_text(            // Replace (or insert) text
     Column            column,      // The replacement Column
     Points            points,      // The number of deleted columns
     const char*       text)        // The replacement (insert) text
{  replace_text(column, points, text, strlen(text)); }

//----------------------------------------------------------------------------
//
// Method-
//       Active::reset
//
// Purpose-
//       Reset the Active source string.
//
//----------------------------------------------------------------------------
void
   Active::reset(                   // Set source
     const char*       text)        // To this (immutable) text
{
   if( text == nullptr )
     text= "";

   source= text;
   fsm= FSM_RESET;
}

void
   Active::reset(                   // Set source
     const char*       text,        // From this (immutable) text
     size_t            size)        // Of this length
{
   source= nullptr;                 // NO ASSOCIATED SOURCE TEXT
   fsm= FSM_CHANGED;

   resize(size);
   memcpy(buffer, text, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::resize
//
// Purpose-
//       Resize the buffer
//
//----------------------------------------------------------------------------
const char*                         // The resized buffer
   Active::resize(                  // Resize the buffer
     size_t            size)        // To (exactly) this size
{
   fetch(size+1);
   buffer_used= size;
   buffer[buffer_used]= '\0';
   return buffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::truncate
//
// Purpose-
//       Remove trailing blanks
//
//----------------------------------------------------------------------------
const char*                         // The truncated buffer
   Active::truncate( void )         // Remove trailing blanks
{
   fetch();                         // (Initialize buffer_used)
   while( buffer_used > 0 &&  buffer[buffer_used - 1] == ' ' )
     buffer_used--;
   buffer[buffer_used]= '\0';       // Set string delimiter

   return buffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::undo
//
// Purpose-
//       Undo any changes.
//
//----------------------------------------------------------------------------
bool                                // Return code: TRUE if state changed
   Active::undo( void )             // Undo any changes
{
   if( fsm == FSM_CHANGED ) {       // If something to undo
     fsm= FSM_RESET;
     return true;
   }

   return false;
}
