//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Ioda.cpp
//
// Purpose-
//       Implement http/Ioda.h
//
// Last change date-
//       2022/10/27
//
//----------------------------------------------------------------------------
// #define NDEBUG                   // TODO: USE (to disable asserts)
#include <new>                      // For std::bad_alloc
#include <cassert>                  // For assert
#include <cctype>                   // For isblank
#include <cstdio>                   // For fprintf, EOF
#include <cstdint>                  // For integer types
#include <cstdlib>                  // For malloc, size_t
#include <cstring>                  // For memcpy, memset, ...
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/List.h>               // For pub::List
#include <pub/utility.h>            // For pub::to_string

#include "pub/http/Ioda.h"          // For pub::http::Ioda, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::visify;

using std::bad_alloc;
using std::runtime_error;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define USE_TEMP_MESG false         // Preferred true, but doesn't compile

enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  LOG2_SIZE= 12                    // Log2(PAGE_SIZE)
,  PAGE_SIZE= 4096                  // The Iota::Page data size

,  USE_VERIFY= true                 // Use internal consistency checking?
}; // enum

//----------------------------------------------------------------------------
// Typedefs, enumerations, and constants
//----------------------------------------------------------------------------
typedef Ioda::Mesg     Mesg;
typedef Ioda::Page     Page;

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_page
//
// Purpose-
//       Allocate an Ioda::Page
//
// Implementation notes-
//       TODO: Optimize using an Ioda::Page pool.
//
//----------------------------------------------------------------------------
static inline Page*
   get_page( void )
{
   Page* page= (Page*)malloc(sizeof(Page));
   if( page == nullptr )
     throw std::bad_alloc();
   memset(page, 0, sizeof(Page));

   char* data= (char*)malloc(PAGE_SIZE);
   if( data == nullptr ) {
     free(page);
     throw std::bad_alloc();
   }

   page->data= data;
// page->used= 0;

   if( HCDM )
     debugf("%p.(%p)= get_page()\n", page, page->data);
   return page;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       put_page
//
// Purpose-
//       Deallocate an Ioda::Page
//
//----------------------------------------------------------------------------
static inline void
   put_page(Page* page)
{  if( HCDM )
     debugf("put_page(%p.(%p))\n", page, page->data);

   free(page->data);
   free(page);
}

//---------------------------------------------------------------------------
//
// Subroutine-
//       should_not_occur
//
// Purpose-
//       A should not occur condition occurred.
//
//----------------------------------------------------------------------------
static void should_not_occur(int line)
{
   debugf("%d %s Should not occur\n", line, __FILE__);
   throw runtime_error("Should not occur");
}

//============================================================================
//
// Method-
//       Ioda::Mesg::Mesg
//       Ioda::Mesg::~Mesg
//
// Purpose-
//       Constructors
//       Destructors
//
//----------------------------------------------------------------------------
   Ioda::Mesg::Mesg( void )
{  if( HCDM )
     debugh("Ioda(%p)::Mesg!\n", this);

   memset((struct msghdr*)this, 0, sizeof(struct msghdr));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Ioda::Mesg::Mesg(Mesg&& move)
{  if( HCDM )
     debugh("Ioda(%p)::Mesg(Mesg&& %p)\n", this, &move);

   memset((struct msghdr*)this, 0, sizeof(struct msghdr));
   msg_iov= move.msg_iov;
   msg_iovlen= move.msg_iovlen;

   move.msg_iov= nullptr;
   move.msg_iovlen= 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Ioda::Mesg::~Mesg( void )
{  if( HCDM )
     debugh("Ioda(%p)::Mesg~ {%p,%'zd)\n", this, msg_iov, size_t(msg_iovlen));

   free(msg_iov);
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::Mesg::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Ioda::Mesg::debug(const char* info) const // Debugging display
{
   debugf("Ioda::Mesg(%p)::debug(%s) {%p.%zd}\n", this, info
         , msg_iov, size_t(msg_iovlen));

   struct iovec* iov= msg_iov;
   for(size_t ix= 0; ix < size_t(msg_iovlen); ++ix) {
     intptr_t data= intptr_t(iov->iov_base);
     debugf("[%2zd] {%.10zx.%.4zx}\n", ix, data, iov->iov_len);
     ++iov;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::Mesg::size
//
// Purpose-
//       Get total data length
//
//----------------------------------------------------------------------------
size_t                              // The total data length
   Ioda::Mesg::size( void ) const   // Get total data length
{
   size_t total= 0;
   struct iovec* iov= msg_iov;
   for(size_t ix= 0; ix < size_t(msg_iovlen); ++ix) {
     total += iov->iov_len;
     ++iov;
   }

   return total;
}

//============================================================================
//
// Method-
//       Ioda::Page::Page
//       Ioda::Page::~Page
//
// Purpose-
//       Constructors
//       Destructors
//
//----------------------------------------------------------------------------
#if 0 // (Not constructed. Built by get_page.)
   Ioda::Page::Page( void )
:  List<Page>::Link()
{  if( HCDM )
     debugh("Ioda(%p)::Page\n", this);
}


   Ioda::Page::~Page( void )
{  if( HCDM )
     debugh("Ioda(%p)::~Page\n", this);
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::Page::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Ioda::Page::debug(const char* info) const // Debugging display
{
   intptr_t data= intptr_t(this->data);
   debugf("Ioda::Page(%p)::debug(%s) {%.10zx.%.4zx}\n", this, info, data, used);
}

//============================================================================
//
// Method-
//       Ioda::Ioda
//       Ioda::~Ioda
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Ioda::Ioda( void )             // Default constructor
:  list()
{  if( HCDM )
     debugh("Ioda(%p)::Ioda\n", this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Ioda::Ioda(Ioda&& move)        // Move constructor
:  list()
{  if( HCDM )
     debugh("Ioda(%p)::Ioda(Ioda&& %p)\n", this, &move);

   Page* head= move.list.get_head();
   Page* tail= move.list.get_tail();
   if( head ) {
     move.list.reset();
     list.insert(nullptr, head, tail);
   }
   size= move.size;
   used= move.used;
   move.size= 0;
   move.used= 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Ioda::Ioda(size_t s)               // Constructor
:  list()
{  if( HCDM )
     debugh("Ioda(%p)::Ioda(%'zd)\n", this, s);

   reset(s);
}

   Ioda::~Ioda( void )              // Destructor
{  if( HCDM )
     debugh("Ioda(%p)::~Ioda\n", this);

   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::operator=
//
// Purpose-
//       Assignment operator
//
//----------------------------------------------------------------------------
Ioda& Ioda::operator=(Ioda&& move)  // (Move) assignment operator
{  if( HCDM )
     debugh("Ioda(%p)::operator=(Ioda&&(%p)\n", this, &move);

   reset();
   size= move.size;
   used= move.used;
   Page* head= move.list.get_head();
   if( head ) {
     Page* tail= move.list.get_tail();
     move.list.reset();
     list.insert(nullptr, head, tail);
   }
   move.size= 0;
   move.used= 0;

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::operator+=
//
// Purpose-
//       Append method
//
//----------------------------------------------------------------------------
Ioda& Ioda::operator+=(Ioda&& move) // Move append
{  if( HCDM )
     debugh("Ioda(%p)::operator+=(Ioda&&(%p)\n", this, &move);

   if( size || move.size )          // Cannot append into/from read mode
     throw runtime_error("Ioda::operator+=, size != 0");
   if( this == &move )              // Cannot append move from ourself
     throw runtime_error("Ioda::operator+=(Ioda&& *this) disallowed");

   used += move.used;
   Page* head= move.list.get_head();
   if( head ) {
     Page* tail= move.list.get_tail();
     move.list.reset();
     list.insert(list.get_tail(), head, tail);
   }
   move.size= 0;
   move.used= 0;

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::operator std::string
//
// Purpose-
//       Cast to std::string
//
//----------------------------------------------------------------------------
   Ioda::operator std::string( void ) const // Cast to std::string
{  if( HCDM )
     debugh("Ioda(%p)::operator std::string\n", this);

   if( size )
     return "";                     // (No string for input Ioda)

   string S;                        // The resultant string
   for(Page* page= list.get_head(); page; page= page->get_next()) {
     string T(page->data, page->used);
     S += T;
   }

   return S;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Ioda::debug(const char* info) const // Debugging display
{  debugf("Ioda(%p)::debug(%s) size(%'zd)\n", this, info, size);
   size_t index= 0;
   size_t total= 0;
   for(Page* page= list.get_head(); page; page= page->get_next()) {
     if( page->used > 16 ) {
       string S(page->data, 16);
       S= visify(S);
       debugf("..[%2zd] %p {%p,%4zd} '%s'...\n", index++, page
              , page->data, page->used, S.c_str());
     } else {
       string S(page->data, page->used);
       S= visify(S);
       debugf("..[%2zd] %p {%p,%4zd} '%s'\n", index++, page
              , page->data, page->used, S.c_str());
     }

     total += page->used;
   }
   debugf("..[%2zd] %'8zd Total\n", index, total);
   if( total != used )
     debugf("..Total(%'zd) != used(%'zd) **** WARNING ****\n", total, used);
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::get_rd_mesg
//
// Purpose-
//       Initialize a read (scatter) Mesg
//
//----------------------------------------------------------------------------
void
   Ioda::get_rd_mesg(               // Get read (scatter) Mesg
     Mesg&             msg,         // (The resultant)
     size_t            size)        // Of this (maximum) length
{  if( HCDM )
     debugh("Ioda(%p)::get_rd_mesg\n", this);

   assert( size > 0 );              // (Some length would be useful)
   reset(size);
   if( msg.msg_iov ) {              // Delete any current content
     free(msg.msg_iov);
     msg.msg_iov= nullptr;
   }

   size_t count= 0;
   for(Page* page= list.get_head(); page; page= page->get_next())
     ++count;

   msg.msg_iovlen= count;
   if( count ) {
     struct iovec* iov= (struct iovec*)malloc(count*sizeof(struct iovec));
     if( iov == nullptr )
       throw bad_alloc();
     msg.msg_iov= iov;

     size_t recv= 0;
     for(Page* page= list.get_head(); page; page= page->get_next()) {
       assert( count-- > 0 );
       iov->iov_base= page->data;
       if( (recv + PAGE_SIZE) > size ) {
         iov->iov_len= size - recv;
         break;
       }

       iov->iov_len=  PAGE_SIZE;
       ++iov;
       recv += PAGE_SIZE;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::get_wr_mesg
//
// Purpose-
//       Initialize a write (gather) Mesg
//
//----------------------------------------------------------------------------
void
   Ioda::get_wr_mesg(               // Get gather vector from offset
     Mesg&             msg,         // (The resultant)
     size_t            size,        // Of this (maximum) length
     size_t            skip) const  // Starting from this offset
{  if( HCDM )
     debugh("Ioda(%p)::get_wr_mesg(%'zd,%'zd)\n", this, size, skip);

   if( size == 0 )                  // If size omitted
     size= used;                    // (Use entire buffer)
   assert( used > skip );           // Must have more than skip size left
   assert( size > 0 );              // Zero length maximum does not compute
   if( msg.msg_iov ) {              // Delete any current content
     free(msg.msg_iov);
     msg.msg_iov= nullptr;
   }

   Page* head= nullptr;             // The first data page
   for(Page* page= list.get_head(); page; page= page->get_next()) {
     if( skip < page->used ) {
       head= page;
       break;
     }

     assert( page->used > 0 );
     skip -= page->used;
   }
   assert( head != nullptr );

   size_t count= 1;                 // Count the used pages
   size_t sent= head->used - skip;
   for(Page* page= head->get_next(); page; page= page->get_next()) {
     if( sent > size )
       break;

     assert( page->used > 0 );
     sent += page->used;
     ++count;
   }

   // Create the iovec
   msg.msg_iovlen= count;
   struct iovec* iov= (struct iovec*)malloc(count*sizeof(struct iovec));
   if( iov == nullptr )
     throw bad_alloc();
   msg.msg_iov= iov;

   // Handle the first page
   sent= head->used - skip;
   iov->iov_base= head->data + skip;
   iov->iov_len=  sent;

   // Handle the remaining pages
   if( sent > size ) {              // If the unlikely single page case
     msg.msg_iovlen= 1;
     iov->iov_len= size;
   } else {
     ++iov;
     assert( count-- > 0 );
     for(Page* page= head->get_next(); page; page= page->get_next()) {
       iov->iov_base= page->data;
       iov->iov_len=  page->used;
       if( sent + page->used >= size ) { // If buffer size limit reached
         iov->iov_len= size - sent;
         break;
       }

       sent += page->used;
       ++iov;
       assert( count-- > 0 );
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::set_used
//
// Purpose-
//       Set used length, converting a read Ioda into a write Ioda.
//
//----------------------------------------------------------------------------
void
   Ioda::set_used(size_t size)      // Set used length
{  if( HCDM )
     debugh("Ioda(%p)::set_used(%'zd)\n", this, size);

   if( size > this->size )
     throw runtime_error("Ioda::set_used only truncates reads");
   if( size == 0 )
     throw runtime_error("Ioda::set_used zero length");

   this->size= 0;
   used= size;                      // Set the used size
   for(Page* page= list.get_head(); page; page= page->get_next()) {
     if( size < PAGE_SIZE ) {       // If last used page (or unused page)
       if( size ) {                 // If used page
         page->used= size;
         page= page->get_next();
         if( page == nullptr )
           return;
       }
       list.remove(page, list.get_tail());
       while( page ) {
         Page* next= page->get_next();
         put_page(page);
         page= next;
       }
       return;
     }

     size -= PAGE_SIZE;
     page->used= PAGE_SIZE;
   }

   // (We get here in the unusal case where the Ioda is completely full)
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::copy
//
// Purpose-
//       Copy (replace content with) Ioda
//
//----------------------------------------------------------------------------
void
   Ioda::copy(const Ioda& copy)     // Copy source Ioda
{  if( HCDM )
     debugh("Ioda(%p)::copy(%p)\n", this, &copy);


   if( copy.used == 0 ) {           // If degnerate case, nothing to copy
     reset(copy.size);
     return;
   }

   // Copy page by page
   reset();                         // (Replace)
   for(Page* page= copy.list.get_head(); page; page= page->get_next())
     write(page->data, page->used);
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::put
//
// Purpose-
//       Write character to Ioda
//
//----------------------------------------------------------------------------
void
   Ioda::put(int copy)              // Write character
{  if( HCDM && VERBOSE > 2 )
     debugh("Ioda(%p)::put('%c')\n", this, copy);

   Page* page= list.get_tail();
   if( page == nullptr || page->used >= PAGE_SIZE ) {
     page= get_page();
     list.fifo(page);
   }

   page->data[page->used++]= copy;
   ++used;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::reset
//
// Purpose-
//       Reset (empty) the Ioda
//       Reset the Ioda size, making it an input buffer
//
//----------------------------------------------------------------------------
void
   Ioda::reset( void )              // Reset (empty) the Ioda
{
   for(;;) {
     Page* page= list.remq();
     if( page == nullptr )
       break;

     put_page(page);
   }

   size= used= 0;
}

void
   Ioda::reset(size_t s)            // Reset the Ioda size
{  if( HCDM )
     debugh("Ioda(%p)::reset(%'zd)\n", this, s);

   reset();
   size= s;
   size_t count= (size + PAGE_SIZE - 1) >> LOG2_SIZE;
   while( count > 0 ) {
     Page* page= get_page();
     list.lifo(page);
     --count;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::split
//
// Purpose-
//       Split leading data
//
//----------------------------------------------------------------------------
void
   Ioda::split(                     // Split leading data
     Ioda&             ioda,        // (Leading data resultant)
     size_t            slen)        // Split at length
{  if( HCDM )
     debugh("Ioda(%p)::split(%p,%'zd(\n", this, &ioda, slen);

   ioda.reset();                    // Default, empty resultant
   if( slen == 0 )                  // If empty split
     return;
   if( slen >= used ) {             // If split at or after end
     ioda= std::move(*this);
     return;
   }

   size_t lead= 0;                  // Current leading length
   Page* head= list.get_head();     // Our head link
   for(Page* page= head; page; page= page->get_next()) {
     if( (lead + page->used) >= slen ) { // If split point found
       if( (lead + page->used) == slen ) { // If split at page boundary
         list.remove(head, page);
         ioda.list.insert(nullptr, head, page);

//       ioda.used= slen;           // (Common path)
//       used -= slen;              // (Common path)
       } else {
         int page_used= int(slen - lead); // The used byte count
         int page_left= page->used - page_used; // The remaining byte count

         list.remove(head, page);     // Remove the pages
         ioda.list.insert(nullptr, head, page); // Give them to the resultant
         page->used= page_used;       // Trimming the last page

         Page* copy= get_page();      // Duplicate common page
         memcpy(copy->data, page->data+page_used, page_left); // Copy remainder
         copy->used= page_left;       // Setting its length
         list.lifo(copy);             // Add it to the head of the list

//       ioda.used= slen;             // (Common path)
//       used -= slen;                // (Common path)
       }

       ioda.used= slen;               // (Common path)
       used -= slen;                  // (Common path)
       return;
     }

     lead += page->used;
   }

debugf("lead(%'zd) slen(%'zd) size(%'zd) used(%'zd)\n", lead, slen, size, used);
debug("should not occur");
   should_not_occur(__LINE__);        // Inconsistent with slen >= used
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::write
//
// Purpose-
//       Write buffer to Ioda
//
//----------------------------------------------------------------------------
void
   Ioda::write(                     // Write buffer
     const void*       copy,        // Buffer address
     size_t            size)        // Buffer length
{  if( HCDM && VERBOSE > 2 )
     debugh("Ioda(%p)::write(%p,%'zd)\n", this, copy, size);

   if( this->size != 0 )
     throw runtime_error("Ioda::write to input buffer");

   Page* page= list.get_tail();
   if( page == nullptr || page->used >= PAGE_SIZE ) {
     page= get_page();
     list.fifo(page);
   }

   char* addr= (char*)copy;         // (For address arithmetic)
   while( size ) {                  // Write buffer
     size_t left= size_t(PAGE_SIZE - page->used);
     if( size < left ) {
       memcpy(page->data + page->used, addr, size);
       page->used += size;
       used += size;
       break;
     }

     memcpy(page->data + page->used, addr, left);
     page->used= PAGE_SIZE;
     used += left;
     addr += left;
     size -= left;
     page= get_page();
     list.fifo(page);
   }
}

//============================================================================
//
// Method-
//       IodaReader::IodaReader
//       IodaReader::~IodaReader
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   IodaReader::IodaReader(const Ioda& I)
:  ioda(I)
{  if( HCDM ) debugh("IodaReader(%p)::IodaReader(%p)\n", this, &ioda); }

   IodaReader::~IodaReader( void )
{  if( HCDM ) debugh("IodaReader(%p)::~IodaReader\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       IodaReader::index
//
// Purpose-
//       Get character at offset
//
// Implementation notes-
//       Internal logic errors result in runtime_error("Should not occur")
//       These logic errors can also occur through usage errors, e.g.
//       if const Ioda& ioda doesn't remain constant.
//
//----------------------------------------------------------------------------
int
   IodaReader::index(size_t index) const // Index operation
{  if( HCDM && VERBOSE > 1 )
     debugh("IodaReader(%p)::operator[](%'zd)\n", this, index);

   if( index >= ioda.used )
     return EOF;

   if( ix_page == nullptr ) {       // If no cache exists
     ix_off0= 0;
     ix_page= ioda.list.get_head(); // Start at zero
     if( ix_page == nullptr )       // (Must have some used if index < used)
       should_not_occur(__LINE__);
   }

   while( index < ix_off0 ) {       // Reverse search
     ix_page= ix_page->get_prev();
     if( ix_page == nullptr )       // (Implies index < get_head()->used)
       should_not_occur(__LINE__);
     ix_off0 -= ix_page->used;
   }

   while( index >= ix_off0 + ix_page->used ) { // Forward search
     ix_off0 += ix_page->used;
     ix_page= ix_page->get_next();
     if( ix_page == nullptr )       // (Implies ioda.used < Sigma(page->used))
       should_not_occur(__LINE__);
   }

   return ix_page->data[index - ix_off0];
}

//----------------------------------------------------------------------------
//
// Method-
//       IodaReader::bksp
//       IodaReader::get
//       IodaReader::peek
//
// Purpose-
//       Get previous character
//       Get next character
//       Examine next character
//
//----------------------------------------------------------------------------
int IodaReader::bksp( void )
{
   if( offset == 0 )
     return EOF;

   --offset;
   return index(offset);
}

int IodaReader::get ( void )
{
   if( offset >= ioda.get_used() )
     return EOF;

   return index(offset++);
}

int IodaReader::peek( void ) const
{
   if( offset >= ioda.get_used() )
     return EOF;

   return index(offset);
}

//----------------------------------------------------------------------------
//
// Method-
//       IodaReader::get_line
//
// Purpose-
//       Get next line
//
//----------------------------------------------------------------------------
string IodaReader::get_line( void )
{
   if( offset >= ioda.get_used() )
     return "";

   string L;                        // The line
   for(;;) {
     char buffer[128];
     for(int length= 0; size_t(length) < sizeof(buffer); ++length) {
       int C= get();
       if( C == '\r' || C == '\n' || C == EOF ) {
         if( C == '\r' ) {
           C= peek();
           if( C == '\n' )
             get();
         }

         string T(buffer, length);
         L += T;
         return L;
       }
       buffer[length]= C;
     }
     string T(buffer, sizeof(buffer));
     L += T;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       IodaReader::get_token
//
// Purpose-
//       Get next token
//
// Implementation notes-
//       Leading and trailing blanks are *NOT* ignored.
//       Single and double quotes are treated as ordinary characters.
//
//----------------------------------------------------------------------------
string IodaReader::get_token(string delim)
{
   if( offset >= ioda.get_used() )
     return "";

   string S;                        // The token
   for(;;) {
     char buffer[128];
     for(int length= 0; size_t(length) < sizeof(buffer); ++length) {
       int C= get();
       const char* D= strchr(delim.c_str(), C);
       if( D || C == '\r' || C == '\n' || C == EOF ) {
         if( C == '\r' ) {
           C= peek();
           if( C == '\n' )
             get();
         }

         string T(buffer, length);
         S += T;
         return S;
       }
       buffer[length]= C;
     }
     string T(buffer, sizeof(buffer));
     S += T;
   }
}
}  // namespace _LIBPUB_NAMESPACE::http
