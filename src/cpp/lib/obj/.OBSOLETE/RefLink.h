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
//       RefLink.h
//
// Purpose-
//       Reference Link.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       USE_ATOMIC_GET:   OK, Marginal improvement with Latch mutex
//       USE_ATOMIC_PUT:   OK, Marginal improvement with Latch mutex
//       USE_LATCH:        OK, More efficient than std::mutex
//       if USE_LATCH == false && USE_ATOMIC_PUT == false,
//           significant performance degradation occurs.
//           This has not been investigated.
//
//----------------------------------------------------------------------------
#ifndef OBJ_REFLINK_H_INCLUDED
#define OBJ_REFLINK_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <mutex>                    // For std::mutex
#include <stdint.h>                 // For uint32_t

#include "obj/Object.h"
#include "obj/Latch.h"
#include "obj/Statistic.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DISABLE_CHECKING true       // Disable check methods?
#define USE_ATOMIC_GET   true       // Use atomic get?
#define USE_ATOMIC_PUT   true       // Use atomic put?
#define USE_LATCH        true       // Use Latch (rather than std::mutex)
#define USE_LIMIT_DEBUG  true       // Use limited extended page debugging

namespace _OBJ_NAMESPACE {
static STATISTIC       stat_gets(0); // Number of get calls
static STATISTIC       stat_puts(0); // Number of put calls

//----------------------------------------------------------------------------
//
// Subroutine-
//       no_storage
//
// Purpose-
//       Handle storage allocation failure.
//
//----------------------------------------------------------------------------
static inline void
   no_storage( void )               // Throw std::bad_alloc
{
   std::bad_alloc NoStorageException;
   throw NoStorageException;
}

//----------------------------------------------------------------------------
//
// Struct-
//       RefLink
//
// Purpose-
//       Reference link descriptor.
//
//----------------------------------------------------------------------------
struct RefLink {                    // Reference link descriptor
enum {  POOL_SIZE= 4096 };          // Number of pre-allocated elements
   RefLink*            refLink;     // Next RefLink in list
   Object*             object;      // Associated Object*
}; // struct RefLink

//----------------------------------------------------------------------------
//
// Struct-
//       RefPage
//
// Purpose-
//       Ref link extention page descriptor.
//
//----------------------------------------------------------------------------
struct RefPage {                    // Ref link extention page descriptor
   RefPage*            next;        // Next RefPage in list
   RefPage*            prev;        // Prior RefPage in list
   RefLink*            refLink;     // First available RefLink in list
   unsigned            offset;      // Offset of first available RefLink
   unsigned            avails;      // Number of available RefLinks
}; // struct RefPage

//----------------------------------------------------------------------------
//
// Struct-
//       RefLinkManager
//
// Purpose-
//       Ref link allocation manager.
//
// Implementation note-
//       RefLinkManager.size is used both for the size of the base allocation
//       RefLink list and for the size of each RefPage.
//
//----------------------------------------------------------------------------
struct RefLinkManager {             // Ref allocator
enum
{  ITEM_CACHE= 8                    // Number of cache elements
,  PAGE_CACHE= 2                    // Extended page allocation count
}; // enum

#if( USE_LATCH )
   Latch               fgMutex;     // Base allocation/Release latch
   Latch               bgMutex;     // Extended allocation/Release latch
#else
   std::mutex          fgMutex;     // Base allocation/Release lock
   std::mutex          bgMutex;     // Extended allocation/Release lock
#endif

   std::atomic<RefLink*>
                       linkCache[ITEM_CACHE]; // The base atomic free cache
   std::atomic<RefLink*>
                       linkHead;    // The base allocation block free list

   // Note: pageHead/pageTail protected by bgMutex
   RefPage*            pageHead;    // First extention RefPage
   RefPage*            pageTail;    // Final extention RefPage

   size_t              usedPages= 0; // Number of RefPages allocated
   RefPage*            linkOrigin;  // The base allocation block origin address
   size_t              pageSize;    // The base allocation block length

   unsigned            pageCount;   // Number of RefLinks per RefPage
   intptr_t            pageMask;    // The size pageFor mask

//----------------------------------------------------------------------------
// Accessors
size_t                              // The number of used pages
   get_used_pages( void ) const     // Get number of used pages
{  return usedPages; }

//----------------------------------------------------------------------------
// Constructor
   RefLinkManager( void )
:  pageSize(RefLink::POOL_SIZE * sizeof(RefLink)), linkHead(nullptr)
,  pageHead(nullptr), pageTail(nullptr)
{
   RefLink* link= (RefLink*)aligned_alloc(pageSize, pageSize); // Allocate an aligned page
   if( !link ) no_storage();        // Handle failure
   linkOrigin= (RefPage*)link;      // Set base origin address
   pageMask= ~(intptr_t(pageSize-1));  // Size allocation mask

   size_t headSize= sizeof(RefPage) + sizeof(RefLink) - 1;
   headSize &= ~(sizeof(RefLink) - 1);
   pageCount= (pageSize - headSize) / sizeof(RefLink);

   // Initialize the pre-allocated RefLinks
   for(int i= 0; i<RefLink::POOL_SIZE; i++)
   {
     if( i < ITEM_CACHE )
       linkCache[i]= link;
     else
     {
       link->refLink= linkHead.load();
       linkHead.store(link);
     }

     link++;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RefLinkManager::check
//
// Purpose-
//       Consistency checks (for debugging)
//
// Implementation note-
//       Return code only used with modified Exception::abort
//
//----------------------------------------------------------------------------
inline int                          // (Must hold bgMutex)
   checkLink(                       // Validate current allocation of
     RefPage*          page,        // This (verified) extended page
     RefLink*          link)        // For this link
{  // if( DISABLE_CHECKING ) return false; // Only called from check(page,link)
   if( false ) debugf("RefLinkManager(%p)::checkLink(%p,%p)\n", this, page, link);

   if( link == nullptr )            // If not checking the link
     return false;                  // We're already done

   // Verify that link offset is < allocation
   if( (intptr_t(link) & (pageSize-1)) >= page->offset )
     return Exception::abort("link(%p) but offset(0x%.6x)", link, page->offset);

   // Verify that link is NOT in free list
   RefLink* nextLink= page->refLink;
   while( nextLink )
   {
     if( link == nextLink )
       return Exception::abort("link(%p) in free list", link);

     nextLink= nextLink->refLink;
   }

   return false;
}

inline int                          // (Must hold bgMutex)
   check(                           // Validate current allocation of
     RefPage*          page,        // This (single) extended page
     RefLink*          link)        // And (optionally) this link
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("RefLinkManager(%p)::check(%p,%p)\n", this, page, link);

   RefPage* prevPage= nullptr;
   RefPage* nextPage= pageHead;
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
       return checkLink(page, link); // Also verify link allocation

     prevPage= nextPage;
     nextPage= prevPage->next;
   }

   return Exception::abort("page(%p) for link(%p) not in list, count(%d)",
                           page, link, count);
}

inline int                          // (Must NOT hold fgMutex or bgMutex)
   check(                           // Validate current allocation of
     RefLink*          link)        // This (single) link
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("RefLinkManager(%p)::check(%p)\n", this, link);

   RefPage* page= pageFor(link);
   if( page == linkOrigin )         // If Foreground link
   {
     std::lock_guard<decltype(fgMutex)> fgLock(fgMutex);

     // Verify that link is NOT in cache
     for(int i= 0; i<ITEM_CACHE; i++)
     {
       if( link == linkCache[i].load() )
         return Exception::abort("link(%p) in cache[%d]", link, i);
     }

     // Verify that link is NOT in free list
     RefLink* next= linkHead.load();
     while( next )
     {
       if( link == next )
         return Exception::abort("link(%p) in free list", link);

       next= next->refLink;
     }
   }
   else
   {
     std::lock_guard<decltype(bgMutex)> bgLock(bgMutex);

     // Verify that the associated page and link are allocated
     return check(page, link);
   }

   return false;
}

inline int                          // (Must NOT hold fgMutex)
   checkLink( void )                // Validate ALL allocated foreground links
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("RefLinkManager(%p)::checkLink\n", this);

   std::lock_guard<decltype(fgMutex)> lock(fgMutex);

   RefLink* link= linkHead.load();
   while( link )
   {
     // Verify that link is NOT also in cache
     for(int i= 0; i<ITEM_CACHE; i++)
     {
       if( link == linkCache[i].load() )
         return Exception::abort("free link(%p) == cache[%d]", link, i);
     }

     // Verify that link is not duplicated in free list
     RefLink* next= link->refLink;
     while( next )
     {
       if( link == next )
         return Exception::abort("free link(%p) duplicated", link);

       next= next->refLink;
     }

     link= link->refLink;
   }

   // Implementation note: Since we verified each element in the free list
   // is not also in cache, it must already be true that no cache element
   // is in the free list. We do not check cache element forPage consistency.
   return false;
}

inline int                          // (Must NOT hold bgMutex)
   checkPage( void )                // Validate ALL background pages and links
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("RefLinkManager(%p)::checkPage\n", this);

   std::lock_guard<decltype(bgMutex)> lock(bgMutex);

   RefPage* prevPage= nullptr;
   RefPage* page= pageHead;
   while( page )
   {
     if( (intptr_t(page) & (pageSize-1)) != 0 )
       return Exception::abort("page(%p) alignment error", page);

     if( page->prev != prevPage ) // (Consistency check)
       return Exception::abort("page(%p).prev(%p) != %p", page,
                               page->prev, prevPage);

     // Verify that all free elements are within page
     unsigned count= 0;
     RefLink* prevLink= nullptr;
     RefLink* link= page->refLink;
     while( link )
     {
       if( ++count > pageCount )
         return Exception::abort("page(%p) count(%u)", page, count);

       if( page != pageFor(link) )
         return Exception::abort("page(%p) contains link(%p)", page, link);

       prevLink= link;
       link= prevLink->refLink;
     }

     // Next page in list
     prevPage= page;
     page= prevPage->next;
   }

   if( pageTail != prevPage )
     return Exception::abort("tail(%p) not last(%p)", pageTail, prevPage);

   return false;
}

inline int                          // (Must NOT hold fgMutex or bgMutex)
   check( void )
{  if( DISABLE_CHECKING ) return false;
   if( false ) debugf("RefLinkManager(%p)::check\n", this);

   if( checkLink() ) return true;
   if( checkPage() ) return true;
   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       RefLinkManager::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
inline void
   debug( void )                    // (Must NOT hold fgMutex or bgMutex)
{  debugf("RefLinkManager(%p)::debug base(%p) pageCount(%d)\n", this,
          linkOrigin, pageCount);

   debugf("..POOL_SIZE(%d) ITEM_CACHE(%d) PAGE_CACHE(%d)\n"
          "..USE_ATOMIC_GET(%s) USE_ATOMIC_PUT(%s)\n"
          "..USE_LATCH(%s) DISABLE_CHECKING(%s)\n"
          , RefLink::POOL_SIZE, ITEM_CACHE, PAGE_CACHE
          , USE_ATOMIC_GET ? "true" : "false"
          , USE_ATOMIC_PUT ? "true" : "false"
          , USE_LATCH ? "true" : "false"
          , DISABLE_CHECKING ? "true" : "false"
          );

   debugf("..gets(%zd) puts(%zd)\n", stat_gets.load(), stat_puts.load());

#if USE_LATCH
   debugf("..fgMutex.latch(%u) bgMutex.latch(%u)\n",
          fgMutex.latch, bgMutex.latch);
#endif

   {{{{
     std::lock_guard<decltype(bgMutex)> lock(bgMutex);

     size_t count= 0;
     RefPage* page= pageHead;
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

       debugf("..(%p)<-(%p)->(%p) avails(%u) free(%p) offset(0x%.6x)\n",
              page->prev, page, page->next,
              page->avails, page->refLink, page->offset);

       page= page->next;
     }

     debugf("..pageHead(%p) pageTail(%p) used(%zd)\n",
            pageHead, pageTail, usedPages);
   }}}}

#if USE_ATOMIC_GET
   for(size_t count= 0; count<ITEM_CACHE; count++)
     debugf("..[%2zd] %p\n", count, linkCache[count].load());
#endif // USE_ATOMIC_GET

   {{{{
     std::lock_guard<decltype(fgMutex)> lock(fgMutex);

     size_t count= 0;
     RefLink* link= linkHead.load();
     while( link )
     {
       link= link->refLink;
       count++;
     }
     debugf("..linkHead(%p) count(%zd)\n", linkHead.load(), count);
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       RefLinkManager::pageFor
//
// Purpose-
//       Get RefPage* associated with RefLink*
//
//----------------------------------------------------------------------------
inline RefPage*                     // The associated RefPage*
   pageFor(                         // Get associated RefPage*
     RefLink*          link) const  // For this RefLink*
{
   intptr_t addr= (intptr_t)link;   // The RefLink*
   addr &= pageMask;                // The RefPage*
   return (RefPage*)addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       RefLinkManager::insPage
//       RefLinkManager::remPage
//
// Purpose-
//       Insert page onto the extended page list.
//       Remove page from the extended page list.
//
// Implementation note-
//       Caller must hold bgMutex
//
//----------------------------------------------------------------------------
protected:
inline void
   insPage(                         // Insert onto the extended page list
     RefPage*          page)        // This page
{
   page->next= nullptr;
   page->prev= nullptr;
   RefPage* head= pageHead;
   if( head )
   {
     head->prev= page;
     page->next= head;
   } else {
     pageTail= page;
   }
   pageHead= page;
}

inline void
   remPage(                         // Remove from the extended page list
     RefPage*          page)        // This page
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
//       RefLinkManager::getPage
//
// Purpose-
//       Add an extention page to the RefLinkManager
//
// Implementation note-
//       Caller must hold bgMutex
//
//----------------------------------------------------------------------------
protected:
inline RefLink*                     // The allocated RefLink
   getPage( void )                  // Allocate a new extention page
{
   RefPage* page= (RefPage*)aligned_alloc(pageSize, pageSize); // Allocate an aligned page
   if( !page ) no_storage();        // Handle allocation failure
   usedPages++;

   // Initialize the extended page
   size_t headSize= sizeof(RefPage) + sizeof(RefLink) - 1;
   headSize &= ~(sizeof(RefLink) - 1);
   page->refLink= nullptr;
   page->offset= headSize;
   page->avails= pageCount;

   // Add the page to the extended list
   insPage(page);

   // Allocate the first link
   RefLink* link= (RefLink*)((char*)page + page->offset);
   page->offset += sizeof(RefLink);
   page->avails--;
   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       RefLinkManager::putPage
//
// Purpose-
//       Remove full extention pages from the RefLinkManager
//
// Implementation note-
//       Caller must hold bgMutex
//
//----------------------------------------------------------------------------
protected:
inline void
   putPage(                         // Release
     RefPage*         page)         // This full extended page
{
   remPage(page);                   // Remove the page from the list

   // If we have an empty page in the extended allocation list,
   // add this page to the list to re-use it.
   unsigned count= 0;
   RefPage* next= pageHead;         // The first extended page
   if( next != nullptr && next->offset >= pageSize )
   while( next )
   {
     if( next->avails == 0 )        // If we need another extended page
     {
       insPage(page);               // Re-use this page
       return;
     }

     if( ++count >= PAGE_CACHE )    // If extended pages list is full
        break;

     next= next->next;
   }

   // The current page is no longer needed. Free it.
   usedPages--;
   free(page);

   // Remove full pages from the used list
   page= pageTail;                  // Start from the end
   while( page )
   {
     if( page->avails < pageCount ) // If the page is not full
       break;

     next= page->prev;

     remPage(page);
     usedPages--;
     free(page);

     page= next;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RefLinkManager::get
//
// Purpose-
//       Allocate a RefLink from the RefLinkManager
//
// Implementation note-
//       Caller must NOT hold fgMutex or bgMutex
//
//----------------------------------------------------------------------------
public:
inline RefLink*                     // The allocated RefLink
   get( void )                      // Allocate a RefLink
{
   RefLink*            link;        // Resultant RefLink*

   statistic(stat_gets);

   // Attempt lock-free allocation from atomic list
#if USE_ATOMIC_GET
   for(int i= 0; i<ITEM_CACHE; i++)
   {
     link= linkCache[i].load();
     if( link )
     {
       if( linkCache[i].compare_exchange_strong(link, nullptr) )
         return link;
     }
   }
#endif // USE_ATOMIC_GET

   {{{{
     // Attempt allocation from free list
     std::lock_guard<decltype(fgMutex)> lock(fgMutex);

     link= linkHead.load();
#if USE_ATOMIC_PUT
     while( link )                  // Get must be single-threaded
     {
       RefLink* newLink= link->refLink;
       if( linkHead.compare_exchange_weak(link, newLink) )
         return link;
     }
#else
     if( link )
     {
       linkHead.store(link->refLink);
       return link;
     }
#endif // USE_ATOMIC_PUT
   }}}}

   {{{{
     // Attempt allocation from extention pages
     std::lock_guard<decltype(bgMutex)> lock(bgMutex);

     int count= 0;
     RefPage* page= pageHead;
     while( page )
     {
       link= page->refLink;
       if( link )
       {
         page->refLink= link->refLink;
         page->avails--;

         return link;
       }

       if( page->offset < pageSize )
       {
         link= (RefLink*)((char*)page + page->offset);
         page->offset += sizeof(RefLink);
         page->avails--;
         return link;
       }

       count++;
       if( count > PAGE_CACHE )
         break;
       page= page->next;
     }

     // Out of reclaim options. Allocate a new extention page.
     return getPage();
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       RefLinkManager::free
//
// Purpose-
//       Release a RefLink back into the RefLinkManager
//
// Implementation note-
//       Caller must NOT hold fgMutex or bgMutex
//
//----------------------------------------------------------------------------
public:
inline void
   put(                             // Deallocate
     RefLink*          link)        // This RefLink
{
   statistic(stat_puts);

   RefPage* page= pageFor(link);
   if( page == linkOrigin )
   {
#if USE_ATOMIC_GET
     // Atomically add to the atomic free list
     for(int i= 0; i<ITEM_CACHE; i++)
     {
       RefLink* oldLink= linkCache[i].load();
       if( oldLink == nullptr )
       {
         if( linkCache[i].compare_exchange_strong(oldLink, link) )
           return;
       }
     }
#endif // USE_ATOMIC_GET

     {{{{
#if USE_ATOMIC_PUT
       // Atomically add to RefLinkManager free list
       RefLink* oldLink= linkHead;
       link->refLink= oldLink;
       while( !linkHead.compare_exchange_weak(oldLink, link) )
         link->refLink= oldLink;
#else
       // Add to RefLinkManager free list
       std::lock_guard<decltype(fgMutex)> lock(fgMutex);

       link->refLink= linkHead.load();
       linkHead.store(link);
#endif // USE_ATOMIC_PUT
     }}}}
   } else {                         // Extended page RefLink
     {{{{
       std::lock_guard<decltype(bgMutex)> lock(bgMutex);

       link->refLink= page->refLink;
       page->refLink= link;
       page->avails++;

       if( page->avails >= pageCount )
         putPage(page);
     }}}}
   }
}
}; // struct RefLinkManager
}  // namespace _OBJ_NAMESPACE
#endif // OBJ_REFLINK_H_INCLUDED
