//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
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
//       2023/08/04
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
#include <pub/Statistic.h>          // For pub::Active_record
#include <pub/utility.h>            // For pub::to_string

#include "pub/Ioda.h"               // For pub::Ioda, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::visify;

using std::bad_alloc;
using std::runtime_error;
using std::string;

#undef PAGE_SIZE                    // (Defined by some libraries)

namespace _LIBPUB_NAMESPACE {       // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define USE_TEMP_MESG false         // Preferred true, but doesn't compile

enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  LOG2_SIZE= 12                    // Log2(PAGE_SIZE)
,  PAGE_SIZE= 4096                  // The (constant) data size

,  USE_REPORT= true                 // Use event Reporter?
,  USE_VERIFY= true                 // Use internal consistency checking?
}; // enum

//----------------------------------------------------------------------------
// Event reporting
//----------------------------------------------------------------------------
static Active_record   data_count("IODA Data"); // Data counter
static Active_record   ioda_count("IODA"); // IODA counter
static Active_record   page_count("IODA Page"); // Page counter
static Active_record   ivec_count("IODA IOvec"); // IOvec counter

namespace {
static struct StaticGlobal {
   StaticGlobal(void)               // Constructor
{
   if( USE_REPORT ) {
     ioda_count.insert();
     data_count.insert();
     page_count.insert();
     ivec_count.insert();
   }
}

   ~StaticGlobal(void)              // Destructor
{
   if( USE_REPORT ) {
     data_count.remove();
     ioda_count.remove();
     page_count.remove();
     ivec_count.remove();
   }
}
}  staticGlobal;
}  // Anonymous namespace

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
   if( USE_REPORT )
     page_count.inc();

   char* data= (char*)malloc(PAGE_SIZE);
   if( data == nullptr ) {
     free(page);
     throw std::bad_alloc();
   }
   page->data= data;
   page->used= 0;
   if( USE_REPORT )
     data_count.inc();

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

   if( USE_REPORT ) {
     data_count.dec();
     page_count.dec();
   }
}

//---------------------------------------------------------------------------
//
// Subroutine-
//       checkstop
//
// Purpose-
//       A should not occur condition occurred.
//
//----------------------------------------------------------------------------
static void checkstop(int line)
{  utility::checkstop(line, __FILE__, "Should not occur"); }

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
   Ioda::Mesg::Mesg(Mesg&& from)
{  if( HCDM )
     debugh("Ioda(%p)::Mesg(Mesg&& %p)\n", this, &from);

   memset((struct msghdr*)this, 0, sizeof(struct msghdr));
   msg_iov= from.msg_iov;
   msg_iovlen= from.msg_iovlen;

   from.msg_iov= nullptr;
   from.msg_iovlen= 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Ioda::Mesg::~Mesg( void )
{  if( HCDM )
     debugh("Ioda(%p)::Mesg~ {%p,%'zd)\n", this, msg_iov, size_t(msg_iovlen));

   if( msg_iov ) {
     free(msg_iov);

     if( USE_REPORT )
       ivec_count.dec();
   }
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

   if( USE_REPORT )
     ioda_count.inc();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Ioda::Ioda(Ioda&& from)        // Move constructor
:  list()
{  if( HCDM )
     debugh("Ioda(%p)::Ioda(Ioda&&(%p))\n", this, &from);

   move(std::move(from));

   if( USE_REPORT )
     ioda_count.inc();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Ioda::Ioda(size_t s)               // Constructor
:  list()
{  if( HCDM )
     debugh("Ioda(%p)::Ioda(%'zd)\n", this, s);

   reset(s);

   if( USE_REPORT )
     ioda_count.inc();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Ioda::~Ioda( void )              // Destructor
{  if( HCDM )
     debugh("Ioda(%p)::~Ioda\n", this);

   reset();

   if( USE_REPORT )
     ioda_count.dec();
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
Ioda&
   Ioda::operator=(Ioda&& from)     // Assignment move operator
{  if( HCDM )
     debugh("Ioda(%p)::operator=(Ioda&&(%p))\n", this, &from);

   move(std::move(from));
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::operator-=
//
// Purpose-
//       Prepend operator
//
//----------------------------------------------------------------------------
// (Reserved for expansion)

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::operator+=
//
// Purpose-
//       (Move) append operator
//
//----------------------------------------------------------------------------
Ioda&
   Ioda::operator+=(Ioda&& from)    // Move append
{  if( HCDM )
     debugh("Ioda(%p)::operator+=(Ioda&&(%p))\n", this, &from);

   if( this == &from )              // Cannot append from self
     throw runtime_error("Ioda::operator+=(*this)");
   if( size || from.size )          // Cannot append into/from a read Ioda
     throw runtime_error("Ioda::operator+=, read Ioda");

   used += from.used;
   Page* head= from.list.get_head();
   if( head ) {
     Page* tail= from.list.get_tail();
     list.insert(list.get_tail(), head, tail);
     from.list.reset();
   }
   from.size= 0;
   from.used= 0;

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::operator std::string
//
// Purpose-
//       (std::string) cast operator
//
//----------------------------------------------------------------------------
   Ioda::operator std::string( void ) const // (std::string) cast operator
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
{  debugf("Ioda(%p)::debug(%s) used(%'zd) size(%'zd)\n", this, info
         , used, size);
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
//       Ioda::set_rd_mesg
//
// Purpose-
//       Initialize a read (scatter) Mesg
//
//----------------------------------------------------------------------------
void
   Ioda::set_rd_mesg(               // Set read (scatter) Mesg
     Mesg&             msg,         // (The resultant)
     size_t            size)        // Of this (maximum) length
{  if( HCDM )
     debugh("Ioda(%p)::get_rd_mesg\n", this);

   assert( size > 0 );              // (Some length would be useful)
   reset(size);
   if( msg.msg_iov ) {              // Delete any current content
     free(msg.msg_iov);
     msg.msg_iov= nullptr;

     if( USE_REPORT )
       ivec_count.dec();
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
     if( USE_REPORT )
       ivec_count.inc();

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
//       Ioda::set_wr_mesg
//
// Purpose-
//       Initialize a write (gather) Mesg
//
//----------------------------------------------------------------------------
void
   Ioda::set_wr_mesg(               // Set write (scatter) Mesg
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
     if( USE_REPORT )
       ivec_count.dec();
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
   if( USE_REPORT )
     ivec_count.inc();

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
   Ioda::copy(const Ioda& from)     // Copy source Ioda
{  if( HCDM )
     debugh("Ioda(%p)::copy(%p)\n", this, &from);

   if( this == &from )              // Don't copy from self
     return;

   if( from.used == 0 ) {           // If copy from read Ioda, nothing to copy
     reset(from.size);
     return;
   }

   // Copy page by page
   reset();                         // Discard current content, if any
   for(Page* page= from.list.get_head(); page; page= page->get_next())
     write(page->data, page->used);
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::discard
//
// Purpose-
//       Discard leading data
//
//----------------------------------------------------------------------------
void
   Ioda::discard(                   // Discard leading data
     size_t            slen)        // For this length
{  if( HCDM )
     debugf("Ioda(%p)::discard(%'zd)\n", this, slen);

   // Handle special cases
   if( slen == 0 )                  // If discard none
     return;
   if( slen >= used ) {             // If discard all
     reset();
     return;
   }

   size_t lead= 0;                  // Current leading length
   Page* tail= nullptr;             // The last removed link
   Page* head= list.get_head();     // Our head link
   for(Page* page= head; page; page= page->get_next()) {
     if( (lead + page->used) >= slen ) { // If split point found
       if( (lead + page->used) == slen ) { // If split at page boundary
         list.remove(head, page);
         tail= page;
       } else {
         int page_used= int(slen - lead); // The used byte count
         int page_left= page->used - page_used; // The remaining byte count

         memcpy(page->data, page->data+page_used, page_left); // Move remainder
         page->used= page_left;
         if( head != page ) {       // If pages need to be removed
           page= page->get_prev();
           list.remove(head, page);
           tail= page;
         }
       }

       // Discard removed pages
       if( tail ) {                 // If any pages removed
         for(;;) {
           Page* temp= head;
           head= head->get_next();
           put_page(temp);
           if( temp == tail )
             break;
         }
       }

       used -= slen;                // (Common path)
       return;
     }

     lead += page->used;
   }

debugf("lead(%'zd) slen(%'zd) used(%'zd) size(%'zd)\n", lead, slen, used, size);
   checkstop(__LINE__);             // Inconsistent with !(slen >= used)
}

//----------------------------------------------------------------------------
//
// Method-
//       Ioda::move
//
// Purpose-
//       Move (replace content with) Ioda
//
//----------------------------------------------------------------------------
void
   Ioda::move(Ioda&& from)          // Move source Ioda
{  if( HCDM )
     debugh("Ioda(%p)::move(%p)\n", this, &from);

   if( this == &from )              // Don't move from self
     return;

   reset();                         // Discard current content (if any)
   size= from.size;
   used= from.used;
   Page* head= from.list.get_head();
   if( head ) {
     Page* tail= from.list.get_tail();
     list.insert(nullptr, head, tail);
     from.list.reset();             // (Sets: list._head= list._tail= nullptr)
   }
   from.size= from.used= 0;
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
   Ioda::put(int from)              // Write character
{  if( HCDM && VERBOSE > 2 )
     debugh("Ioda(%p)::put('%c')\n", this, from);

   Page* page= list.get_tail();
   if( page == nullptr || page->used >= PAGE_SIZE ) {
     page= get_page();
     list.fifo(page);
   }

   page->data[page->used++]= from;
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

   ioda.reset();                    // Empty the resultant
   if( slen == 0 )                  // If empty split
     return;
   if( slen >= used ) {             // If split at or after end
     ioda.move(std::move(*this));
     return;
   }

   size_t lead= 0;                  // Current leading length
   Page* head= list.get_head();     // Our head link
   for(Page* page= head; page; page= page->get_next()) {
     if( (lead + page->used) >= slen ) { // If split point found
       if( (lead + page->used) == slen ) { // If split at page boundary
         list.remove(head, page);
         ioda.list.insert(nullptr, head, page);
       } else {
         int page_used= int(slen - lead); // The used byte count
         int page_left= page->used - page_used; // The remaining byte count

         list.remove(head, page);   // Remove the pages
         ioda.list.insert(nullptr, head, page); // Give them to the resultant
         page->used= page_used;     // Trimming the last page

         Page* last= get_page();    // Duplicate (last) common page
         memcpy(last->data, page->data+page_used, page_left); // Copy remainder
         last->used= page_left;     // Setting its length
         list.lifo(last);           // Add it to the end of the list
       }

       ioda.used= slen;             // (Common path)
       used -= slen;                // (Common path)
       return;
     }

     lead += page->used;
   }

debugf("lead(%'zd) slen(%'zd) size(%'zd) used(%'zd)\n", lead, slen, size, used);
   checkstop(__LINE__);             // Inconsistent with !(slen >= used)
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
     const void*       from,        // Buffer address
     size_t            size)        // Buffer length
{  if( HCDM && VERBOSE > 2 )
     debugh("Ioda(%p)::write(%p,%'zd)\n", this, from, size);

   if( this->size != 0 )
     throw runtime_error("Ioda::write to input buffer");

   if( size == 0 )                  // If (silly) zero length write
     return;                        // (Could avoid allocating an unused page)

   Page* page= list.get_tail();
   if( page == nullptr || page->used >= PAGE_SIZE ) {
     page= get_page();
     list.fifo(page);
   }

   // While copying data from the buffer into an Ioda::Page,
   // - Page* page is the valid tail page of the page list.
   // - Page* page->used is less than PAGE_SIZE. (It's zero if newly allocated)
   const char* addr= (char*)from;   // (For address arithmetic)
   while( size ) {                  // Write buffer
     size_t left= size_t(PAGE_SIZE - page->used);
     if( size <= left ) {           // If this page completes the write
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
       checkstop(__LINE__);
   }

   while( index < ix_off0 ) {       // Reverse search
     ix_page= ix_page->get_prev();
     if( ix_page == nullptr )       // (Implies index < get_head()->used)
       checkstop(__LINE__);
     ix_off0 -= ix_page->used;
   }

   while( index >= ix_off0 + ix_page->used ) { // Forward search
     ix_off0 += ix_page->used;
     ix_page= ix_page->get_next();
     if( ix_page == nullptr )       // (Implies ioda.used < Sigma(page->used))
       checkstop(__LINE__);
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
int
   IodaReader::bksp( void )
{
   if( offset == 0 )
     return EOF;

   --offset;
   return index(offset);
}

int
   IodaReader::get( void )
{
   if( offset >= ioda.get_used() )
     return EOF;

   return index(offset++);
}

int
   IodaReader::peek( void ) const
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
string
   IodaReader::get_line( void )
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
string
   IodaReader::get_token(string delim)
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
}  // namespace _LIBPUB_NAMESPACE
