//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
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
//       2021/08/24
//
//----------------------------------------------------------------------------
#include <string.h>                 // For memcpy, memmove, strlen

#include <pub/Debug.h>              // For pub::Debug object
#include <pub/Must.h>               // For pub::Must methods
#include <pub/Utf.h>                // For pub::Utf methods and objects

#include "Config.h"                 // For config::opt_hcdm
#include "Active.h"                 // Implementation class

using namespace config;             // For config::opt_hcdm
using namespace pub::debugging;     // For debugging
using namespace pub;                // For Must::

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum { BUFFER_SIZE= 2048 };         // Default buffer/expansion size (2**N)

//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
typedef pub::Utf::utf8_t utf8_t;    // Import pub::Utf::utf8_t

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
     debugh("Active(%p)::~Active\n", this);

   Must::free(buffer);
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
{
   if( opt_hcdm )
     debugh("Active(%p)::Active\n", this);

   buffer= (char*)Must::malloc(buffer_size);
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
   if( info ) debugf("Active(%p)::debug(%s) fsm(%d)\n", this, info, fsm);
   debugf("..source(%s).%zd\n", source, strlen(source));
   if( fsm != FSM_RESET ) {
     buffer[buffer_used]= '\0';     // (Buffer is mutable)
     debugf("..buffer(%s).%zd/%zd\n", buffer, buffer_used, buffer_size);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::get_buffer
//
// Purpose-
//       (Unconditionally) access the buffer, leaving trailing blanks.
//
//----------------------------------------------------------------------------
const char*                         // The current buffer
   Active::get_buffer(              // Get '\0' delimited buffer
     Column            column)      // Starting at this Column
{
   // Implementation note: It's possible for index() to modify buffer, so we
   // can't just return buffer + index(column).
   Offset offset= index(column);
   return buffer + offset;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::get_changed
//
// Purpose-
//       Access the buffer if changed. If unchanged, return nullptr.
//
//----------------------------------------------------------------------------
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
//       Active::get_cols
//
// Purpose-
//       Return the buffer Column count (trailing blanks removed)
//
//----------------------------------------------------------------------------
Active::Ccount                      // The current buffer Column count
   Active::get_cols( void )         // Get current buffer Column count
{
   truncate();                      // (Initialize, remove trailing blanks)
   Ccount ccount= 0;                // The column count
   for(auto it= pub::Utf8::const_iterator((const utf8_t*)buffer); *it; ++it)
     ccount++;

   return ccount;
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
{
   if( opt_hcdm )
     debugh("Active(%p)::expand(%zd) [%zd,%zd]\n", this, length, buffer_used, buffer_size);

   if( length >= buffer_size ) {    // If expansion required
     size_t replace_size= length + BUFFER_SIZE;
     replace_size &= ~(BUFFER_SIZE - 1);
     char* replace= (char*)Must::malloc(replace_size);
     if( fsm != FSM_RESET )
       memcpy(replace, buffer, buffer_used+1);
     Must::free(buffer);
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
     debugh("Active(%p)::fetch(%zd) [%zd/%zd]\n", this, length
           , buffer_used, buffer_size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::index
//
// Purpose-
//       Address character at column index, fetching and filling if required.
//
//----------------------------------------------------------------------------
Active::Offset                      // The character Offset
   Active::index(                   // Get character Offset for
     Column            column)      // This Column
{
   fetch(column);                   // Load the buffer
   Offset offset= 0;
   while( column > 0 ) {
     if( buffer_used <= offset ) {  // If blank fill required
       fetch(buffer_used + column);
       offset= buffer_used - 1;
       break;
     }
     offset += pub::Utf8::length(buffer + offset); // Next character length
     --column;
   }

   return offset;
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
   if( code == 0 )                  // Don't insert a null character
     return;
   if( !pub::Utf::is_unicode(code) ) // Subtitute UNI_REPLACEMENT if invalid
     code= pub::Utf::UNI_REPLACEMENT;

   char insert_buff[8];             // The insert character encoder buffer
   pub::Utf8::encode(code, (utf8_t*)insert_buff);
   replace_text(column, 0, insert_buff, pub::Utf8::length(code));
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
   if( code == 0 )                  // Don't insert a null character
     return;
   if( !pub::Utf::is_unicode(code) ) // Subtitute UNI_REPLACEMENT if invalid
     code= pub::Utf::UNI_REPLACEMENT;

   char insert_buff[8];             // The insert character encoder buffer
   pub::Utf8::encode(code, (utf8_t*)insert_buff);
   replace_text(column, 1, insert_buff, pub::Utf8::length(code));
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
     Ccount            ccount,      // The replacement (delete) Ccount
     const char*       text,        // The replacement (insert) text
     Length            insert)      // The replacement (insert) text Length
{
   Offset origin= index(column);    // The origin offset (+ blank fill)
   Length remove= 0;                // Removal length, in bytes
   if( ccount )
     remove= index(column + ccount) - origin;
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
     Ccount            ccount,      // The Column count of deleted columns
     const char*       text)        // The replacement (insert) text
{  replace_text(column, ccount, text, strlen(text)); }

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
