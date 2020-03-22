//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Allocator.cpp
//
// Purpose-
//       Implement Allocator.h methods.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <algorithm>                // For std::max, std::min
#include <atomic>                   // For std::atomic
#include <mutex>                    // For std::lock_guard
#include <new>                      // For placement operator new
#include <stdint.h>                 // For uint32_t
#include <string.h>                 // For memset (in PageAllocator)

#include <com/Debug.h>
#include <obj/built_in.h>           // For PageAllocator
#include "obj/List.h"
#include "obj/Object.h"
#include "obj/Statistic.h"

#include "obj/Allocator.h"          // Also includes Latch.h

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DISABLE_CHECKING true       // Disable check methods?
#define USE_LIMIT_DEBUG  true       // Use limited extended page debugging

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const size_t    IMPLEMENTATION_LIMIT= size_t(0x80000000);

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_addr
//
// Purpose-
//       Bringup: debug storage allocation and release
//
//----------------------------------------------------------------------------
static inline void                  // Normally unused
   debug_addr(                      // Debug storage allocation and release
     void*             addr)        // For this address
{
   debugSetIntensiveMode();
   debugf("debug_addr(%p)\n", addr);

   #if false // For use with suspected storage overlay
     char* data= (char*)addr;       // Use char* for arithmetic
     data -= 16;                    // Display prefix
     snap(data, 32);                // Display data
     dump(data, 32);                // Trace data
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       log2
//
// Purpose-
//       Get associated power of two.
//
// Implementation note-
//       log2(0) == log2(1) == 0
//
//----------------------------------------------------------------------------
static inline int                   // The associated power of two
   log2(                            // Get log2
     size_t            size)        // For this size
{
   int                 result;      // Resultant

   for(result= 0; size > 1; result++)
   {
     size >>= 1;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       round_up
//
// Purpose-
//       Round up size to next power of two.
//
//----------------------------------------------------------------------------
static inline size_t                // The next power of two
   round_up(                        // Get next power of two
     size_t            size)        // For this size
{
   if( size > IMPLEMENTATION_LIMIT ) // If too large
     throw NoStorageException("Implementation limit exceeded");

   size_t got_size= IMPLEMENTATION_LIMIT; // Implementation limit
   size_t try_size= got_size >> 1;
   while( got_size > 0 )
   {
     if( size > try_size )
       break;

     got_size >>= 1;
     try_size >>= 1;
   }

   return got_size;
}

static inline size_t                // The next aligned size
   round_up(                        // Get next aligned size
     size_t            size,        // For this size
     size_t            align)       // and this alignment
{
   align= round_up(align);          // Alignment must be power of two

   size += (align-1);               // Round up
   size &= ~(align-1);              // Truncate down
   return size;
}

//----------------------------------------------------------------------------
//
// Class-
//       PageAllocator
//
// Purpose-
//       Allocate aligned storage.
//
//----------------------------------------------------------------------------
#include "PageAllocator.hpp"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static union {
   std::size_t         alignment;
   char                buffer[sizeof(PageAllocator)];
} pageAllocator_buffer;             // The real PageAllocator

// Initialized by first construction
static Latch           latch_one;   // Initialization Latch
static Latch           latch_two;   // Initialization complete indicator

static PageAllocator&  pageAllocator= (*((PageAllocator*)&pageAllocator_buffer));

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::Allocator
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Allocator::Allocator( void )     // Default constructor
:  stat_gets(0), stat_puts(0), stat_find(0), stat_free(0)
,  itemLatch(), pageLatch(), itemHead(nullptr)
,  pageMask(0), pageSize(0), pageExtented(0), pageReserved(0)
,  itemOrigin(nullptr), itemEnding(nullptr), itemSize(0)
,  pageHead(nullptr), pageTail(nullptr), usedPages(0)
{  if( false ) debugf("Allocator(%p)::Allocator\n", this);

   // We need to jump through these hoops so that our static PageAllocator is
   // properly constructed before it is used.
   if( latch_two.latch == 0 )
   {
     std::lock_guard<decltype(latch_one)> lock(latch_one);
     if( latch_two.try_lock() )
     {
       ::new(&pageAllocator) PageAllocator();
     }
   }

   for(int i= 0; i<ITEM_CACHE; i++)
     itemCache[i].store(nullptr);
}

   Allocator::Allocator(            // Constructor
     size_t            size,        // The item size
     size_t            reserved,    // The (minimum) number of Items
     size_t            extended)    // The (minimum) number of Items per Page
{  ::new(this) Allocator();

   // Minimum page size calculation
   itemSize= std::max(size, sizeof(Item)); // Minimum allocation size
   itemSize= round_up(itemSize, 8); // Doubleword alignment
   extended= std::max(extended, size_t(32)); // Minimum number of Items per page
   size_t min_size= itemSize * extended; // Minimum page size
   min_size += 4095;                // Round to 4096 byte boundary
   min_size &= ~(4095);             // Truncate at 4096 byte boundary

   // Actual page size calculation
   pageSize= round_up(min_size);    // Round up to next power of two
   pageMask= ~(pageSize-1);         // Allocation size address mask
   size_t prefix= round_up(itemSize, sizeof(Page));
   size_t usable= pageSize - prefix;
   pageExtented= usable / itemSize;    // Extended page element count

   // Allocate the pre-allocated Items
   if( reserved )
   {
     reserved= std::max(reserved, size_t(32)); // Minimum reserved (if not zero)
     pageReserved= reserved;        // Number of reserved items
     reserved *= itemSize;          // Size of reserved area
     Item* item= (Item*)malloc(reserved * itemSize);
     if( item == nullptr ) throw bad_alloc; // Exception if failure
     itemOrigin= item;              // Set base origin address
     itemEnding= (Item*)((char*)item + reserved); // Set base ending address

     // Initialize the pre-allocated Items
     for(size_t i= 0; i<pageReserved; i++)
     {
       if( i < ITEM_CACHE )
         itemCache[i]= item;
       else
       {
         item->next= itemHead.load();
         itemHead.store(item);
       }

       item= (Item*)(intptr_t(item) + itemSize);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::~Allocator
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Allocator::~Allocator( void )    // Destructor
{  if( false ) debugf("Allocator(%p)::~Allocator\n", this);
   if( itemOrigin )
   {
     free(itemOrigin);
     itemOrigin= nullptr;
   }

   Page* page= pageHead;
   while( page )
   {
     Page* nextPage= page->next;
     rem_page(page);
     free_page(page);
     page= nextPage;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::check_item(Page*,Item*)
//       Allocator::check_item( void )
//       Allocator::check_page( void )
//       Allocator::check(Page*,Item*)
//       Allocator::check(Item*)
//       Allocator::check( void )
//
// Purpose-
//       Consistency checks (for debugging)
//
// Implementation note-
//       Return code only used with modified Exception::abort
//
//----------------------------------------------------------------------------
inline int                          // (Must hold pageLatch)
   Allocator::check_item(           // Validate current allocation of
     Page*             page,        // This (verified) extended Page
     Item*             item)        // For this Item
{  // if( DISABLE_CHECKING ) return false; // Only called from check(page,item)
   if( false ) debugf("Allocator(%p)::check_item(%p,%p)\n", this, page, item);

   // Verify that Item offset is < allocation
   if( (intptr_t(item) & (pageSize-1)) >= page->offset )
     return Exception::abort("item(%p) but offset(0x%.6x)", item, page->offset);

   // Verify that the Item is NOT in free list
   Item* nextItem= page->head;
   while( nextItem )
   {
     if( item == nextItem )
       return Exception::abort("item(%p) in free list", item);

     nextItem= nextItem->next;
   }

   return false;
}

inline int                          // (Must NOT hold itemLatch)
   Allocator::check_item( void )    // Validate ALL allocated reserved Items
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("Allocator(%p)::check_item\n", this);

   std::lock_guard<decltype(itemLatch)> lock(itemLatch);

   Item* item= itemHead.load();
   while( item )
   {
     // Verify that Item is NOT also in cache
     for(int i= 0; i<ITEM_CACHE; i++)
     {
       if( item == itemCache[i].load() )
         return Exception::abort("free Item(%p) == cache[%d]", item, i);
     }

     // Verify that Item is not duplicated in free list
     Item* next= item->next;
     while( next )
     {
       if( item == next )
         return Exception::abort("free Item(%p) duplicated", item);

       next= next->next;
     }

     item= item->next;
   }

   // Implementation note: Since we verified each element in the free list
   // is not also in cache, it must already be true that no cache element
   // is in the free list. We do not check cache element forPage consistency.
   return false;
}

inline int                          // (Must NOT hold pageLatch)
   Allocator::check_page( void )    // Validate ALL background Pages and Items
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("Allocator(%p)::check_page\n", this);

   std::lock_guard<decltype(pageLatch)> lock(pageLatch);

   Page* prevPage= nullptr;
   Page* page= pageHead;
   while( page )
   {
     if( (intptr_t(page) & (pageSize-1)) != 0 )
       return Exception::abort("page(%p) alignment error", page);

     if( page->prev != prevPage ) // (Consistency check)
       return Exception::abort("page(%p).prev(%p) != %p", page,
                               page->prev, prevPage);

     // Verify that all free elements are within page
     size_t count= 0;
     Item* prevItem= nullptr;
     Item* item= page->head;
     while( item )
     {
       if( ++count > pageExtented )
         return Exception::abort("Page(%p) count(%zd)", page, count);

       if( page != get_page4(item) )
         return Exception::abort("Page(%p) contains Item(%p)", page, item);

       prevItem= item;
       item= prevItem->next;
     }

     // Next page in list
     prevPage= page;
     page= prevPage->next;
   }

   if( pageTail != prevPage )
     return Exception::abort("tail(%p) not last(%p)", pageTail, prevPage);

   return false;
}

inline int                          // (Must hold pageLatch)
   Allocator::check(                // Validate current allocation of
     Page*             page,        // This (single) extended Page
     void*             item)        // And (optionally) this Item
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("Allocator(%p)::check(%p,%p)\n", this, page, item);

   Page* prevPage= nullptr;
   Page* nextPage= pageHead;
   unsigned count= 0;
   while( nextPage )
   {
     // With the prevPage/nextPage checks, it should not also be possible for
     // the free page list to loop back on itself. This does check usedPages.
     if( ++count > usedPages )      // If usedPages < actual count
       return Exception::abort("more free pages than usedPages(%zd)",
                               usedPages);

     if( (intptr_t(nextPage) & (pageSize-1)) != 0 )
       return Exception::abort("page(%p) alignment error", nextPage);

     if( nextPage->prev != prevPage ) // (Consistency check)
       return Exception::abort("page(%p).prev(%p) != %p", nextPage,
                               nextPage->prev, prevPage);

     if( page == nextPage )         // If page found
       return check_item(page, (Item*)item); // Also verify Item allocation

     prevPage= nextPage;
     nextPage= prevPage->next;
   }

   return Exception::abort("Page(%p) not in list, count(%d)",
                           page, count);
}

int                                 // (Must NOT hold itemLatch or pageLatch)
   Allocator::check(                // Validate current allocation of
     void*             item)        // This (single) Item
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("Allocator(%p)::check(%p)\n", this, item);

   if( (char*)item >= (char*)itemOrigin && (char*)item < (char*)itemEnding )
   { // Handle reserved item
     std::lock_guard<decltype(itemLatch)> fgLock(itemLatch);

     // Verify that Item is NOT in cache
     for(int i= 0; i<ITEM_CACHE; i++)
     {
       if( (Item*)item == itemCache[i].load() )
         return Exception::abort("Item(%p) == cache[%d]", item, i);
     }

     // Verify that Item is NOT in free list
     Item* next= itemHead.load();
     while( next )
     {
       if( (Item*)item == next )
         return Exception::abort("Item(%p) in free list", item);

       next= next->next;
     }
   }
   else
   {
     std::lock_guard<decltype(pageLatch)> bgLock(pageLatch);

     // Verify that the associated Page and Item are allocated
     Page* page= get_page4((Item*)item);
     return check(page, item);
   }

   return false;
}

int                                 // (Must NOT hold itemLatch or pageLatch)
   Allocator::check( void )
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("Allocator(%p)::check\n", this);

   if( check_item() ) return true;
   if( check_page() ) return true;
   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::debug_locked
//       Allocator::debug_static
//       Allocator::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Allocator::debug_locked( void )  // (Must hold pageLatch)
{  debugf("Allocator(%p)::debug\n"
          "..itemSize(0x%.4zx) itemOrigin(%p:%p) pageReserved(%zd)\n"
          "..pageMask(0x%zx) pageSize(0x0%zx,%zd) pageExtented(%zd)\n"
          , this, itemSize, itemOrigin, itemEnding, pageReserved
          , pageMask, pageSize, pageSize, pageExtented);

   debugf("..ITEM_CACHE(%d) PAGE_CACHE(%d)\n"
          "..DISABLE_CHECKING(%s)\n"
          , ITEM_CACHE, PAGE_CACHE
          , DISABLE_CHECKING ? "true" : "false"
          );

   debugf("..gets(%zd) puts(%zd) finds(%zd) frees(%zd)\n"
          , stat_gets.load(), stat_puts.load()
          , stat_find.load(), stat_free.load());

   debugf("..itemLatch.latch(%u) pageLatch.latch(%u)\n",
          itemLatch.latch, pageLatch.latch);

   {{{{
     size_t count= 0;
     Page* page= pageHead;
     while( page != nullptr )
     {
       count++;
       if( USE_LIMIT_DEBUG )        // (Limit extended page debugging)
       {
         if( count > PAGE_CACHE )
         {
           debugf("..** more **\n");
           break;
         }
       }

       debugf("..(%p)<-(%p)->(%p) avails(%u) head(%p) offset(0x%.6x)\n",
              page->prev, page, page->next,
              page->avails, page->head, page->offset);

       page= page->next;
     }

     count= 0;
     page= pageHead;
     while( page != nullptr )
     {
       count += page->avails;
       page= page->next;
     }

     debugf("..pageHead(%p) pageTail(%p) usedPages(%zd) freeItems(%zd)\n",
            pageHead, pageTail, usedPages, count);
   }}}}

   {{{{
     for(size_t count= 0; count<ITEM_CACHE; count++)
       debugf("..[%2zd] %p\n", count, itemCache[count].load());
   }}}}

   {{{{
     std::lock_guard<decltype(itemLatch)> lock(itemLatch);

     size_t count= 0;
     Item* item= itemHead.load();
     while( item )
     {
       count++;
       item= item->next;
     }
     debugf("..itemHead(%p) freeItems(%zd)\n", itemHead.load(), count);
   }}}}
}

void
   Allocator::debug_static( void )  // (Must NOT hold itemLatch or pageLatch)
{
   pageAllocator.debug();
}

void
   Allocator::debug( void )         // (Must NOT hold itemLatch or pageLatch)
{  std::lock_guard<decltype(pageLatch)> lock(pageLatch);

   debug_locked();
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::find_page
//
// Purpose-
//       Allocate a Page
//
//----------------------------------------------------------------------------
inline void*                        // The allocated Page
   Allocator::find_page( void )     // Allocate a Page
{
   void* page= pageAllocator.find_page(pageSize);
   statistic(stat_find);
   usedPages++;
   return page;
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::free_page
//
// Purpose-
//       Delete a Page
//
//----------------------------------------------------------------------------
inline void
   Allocator::free_page(            // Delete a Page
     void*             page)        // The Page to delete
{
   pageAllocator.free_page(page, pageSize);
   statistic(stat_free);
   usedPages--;
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::get_extended
//
// Purpose-
//       Find the first extended Page with available Items.
//
// Implementation note-
//       Caller must hold pageLatch
//
//----------------------------------------------------------------------------
inline Allocator::Page*             // The first extended Page with Items
   Allocator::get_extended( void )  // Get first extended Page with Items
{
   Page* next= pageHead;
   for(unsigned count= 0; count<PAGE_CACHE; count++)
   {
     if( next == nullptr || next->avails )
       return next;

     next= next->next;
   }

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::ins_page
//       Allocator::ins_tail
//       Allocator::rem_page
//
// Purpose-
//       Insert page onto the extended page list (head).
//       Insert page onto the extended page list (tail).
//       Remove page from the extended page list.
//
// Implementation note-
//       Caller must hold pageLatch
//
//----------------------------------------------------------------------------
inline void
   Allocator::ins_page(             // Insert onto the extended page list
     Page*             page)        // This page
{
   page->prev= nullptr;
   Page* head= pageHead;
   if( head )
   {
     head->prev= page;
     page->next= head;
   } else {
     pageTail= page;
     page->next= nullptr;
   }
   pageHead= page;
}

inline void
   Allocator::ins_tail(             // Insert onto the extended page list tail
     Page*             page)        // This page
{
   page->next= nullptr;
   Page* tail= pageTail;
   if( tail )
   {
     tail->next= page;
     page->prev= tail;
   } else {
     pageHead= page;
     page->prev= nullptr;
   }
   pageTail= page;
}

inline void
   Allocator::rem_page(             // Remove from the extended page list
     Page*             page)        // This page
{
   if( page->prev )
     page->prev->next= page->next;
   else
     pageHead= page->next;

   if( page->next )
     page->next->prev= page->prev;
   else
     pageTail= page->prev;
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::get_page
//
// Purpose-
//       Add an extention page to the Allocator
//
// Implementation note-
//       Caller must hold pageLatch
//
//----------------------------------------------------------------------------
inline Allocator::Item*             // The allocated Item
   Allocator::get_page( void )      // Allocate a new extention page
{
   // Allocate and initialize an extended page
   Page* page= (Page*)find_page();  // Allocate a new Page

   page->head= nullptr;
   page->offset= round_up(itemSize, sizeof(Page));
   page->avails= pageExtented;

   // Add the page to the extended list
   ins_page(page);

   // Allocate the first Item
   Item* item= (Item*)((char*)page + page->offset);
   page->offset += itemSize;
   page->avails--;

   return item;
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::put_page
//
// Purpose-
//       Remove full extention pages from the Allocator
//
// Implementation note-
//       Caller must hold pageLatch
//
//----------------------------------------------------------------------------
inline void
   Allocator::put_page(             // Release
     Page*            page)         // This full extended page
{
   rem_page(page);                  // Remove the page from the list

   // If we have any empty page in the extended allocation list,
   // add this page to the list to re-use it.
   unsigned count= 0;
   Page* next= pageHead;            // The first extended page
   if( next != nullptr && next->offset >= pageSize )
   while( next )
   {
     if( next->avails == 0 )        // If we need another extended page
     {
       rem_page(next);              // Move the empty page to the tail
       ins_tail(next);

       ins_page(page);              // Re-use this page
       return;
     }

     if( ++count > PAGE_CACHE )     // If extended pages list is full
        break;

     next= next->next;
   }

   // The current page is no longer needed. Free it.
   free_page(page);

   // Remove full pages from the used list
   page= pageTail;                  // Start from the end
   while( page )
   {
     if( page->avails < pageExtented ) // If the page is not full
       break;

     next= page->prev;

     rem_page(page);
     free_page(page);

     page= next;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::get
//
// Purpose-
//       Allocate a Item from the Allocator
//
// Implementation note-
//       Caller must NOT hold itemLatch or pageLatch
//
//----------------------------------------------------------------------------
inline void*                        // The allocated Item
   Allocator::get( void )           // Allocate a Item
{
   Item*               item;        // Resultant Item*

   statistic(stat_gets);

   // Attempt lock-free allocation from atomic list
   for(int i= 0; i<ITEM_CACHE; i++)
   {
     item= itemCache[i].load();
     if( item )
     {
       if( itemCache[i].compare_exchange_strong(item, nullptr) )
         return item;
     }
   }

   {{{{
     // Attempt allocation from free list
     std::lock_guard<decltype(itemLatch)> lock(itemLatch);

     item= itemHead.load();
     while( item )                  // Get must be single-threaded
     {
       Item* newItem= item->next;
       if( itemHead.compare_exchange_weak(item, newItem) )
         return item;
     }
   }}}}

   {{{{
     // Attempt allocation from extention pages
     std::lock_guard<decltype(pageLatch)> lock(pageLatch);

     Page* page= get_extended();    // First page with available Items
     if( page )
     {
       if( page->head )
       {
         item= page->head;
         page->head= item->next;
       } else {
         item= (Item*)((char*)page + page->offset);
         page->offset += itemSize;
       }

       page->avails--;
       return item;
     }

     // Out of reclaim options. Allocate a new extention page.
     return get_page();
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::put
//
// Purpose-
//       Release a Item back into the Allocator
//
// Implementation note-
//       Caller must NOT hold itemLatch or pageLatch
//
//----------------------------------------------------------------------------
inline void
   Allocator::put(                  // Deallocate
     void*             addr)        // This Item
{
   statistic(stat_puts);

   Item* item= static_cast<Item*>(addr);
   if( (char*)item >= (char*)itemOrigin && (char*)item < (char*)itemEnding )
   { // Handle reserved item
     // Atomically add to the atomic free list
     for(int i= 0; i<ITEM_CACHE; i++)
     {
       Item* oldItem= itemCache[i].load();
       if( oldItem == nullptr )
       {
         if( itemCache[i].compare_exchange_strong(oldItem, item) )
           return;
       }
     }

     {{{{
       // Atomically add to free list
       Item* oldItem= itemHead;
       item->next= oldItem;
       while( !itemHead.compare_exchange_weak(oldItem, item) )
         item->next= oldItem;
     }}}}
   } else {                         // Extended page Item
     Page* page= get_page4(item);
     {{{{
       std::lock_guard<decltype(pageLatch)> lock(pageLatch);

       item->next= page->head;
       page->head= item;
       page->avails++;

       if( page->avails >= pageExtented ) // If Page is full
         put_page(page);
       else if( page->avails == 1 ) // If Page was empty
       {
         if( get_extended() == nullptr )
         {
           // PAGE_CACHE empty. Move this Page to the head of the list.
           rem_page(page);
           ins_page(page);
         }
       }
     }}}}
   }
}
}  // namespace _OBJ_NAMESPACE

