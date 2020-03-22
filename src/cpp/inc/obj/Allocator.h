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
//       Allocator.h
//
// Purpose-
//       Storage allocator description.                                                                 ts.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJ_ALLOCATOR_H_INCLUDED
#define OBJ_ALLOCATOR_H_INCLUDED

#include <atomic>                   // For std::atomic

#include "obj/timing/Latch.h"       // (Timing test version)

#include "define.h"                 // For _OBJ_NAMESPACE, ...
#include "Latch.h"
#include "Statistic.h"

#include "detail/Allocator.h"

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Allocator
//
// Purpose-
//       Allocator descriptor.
//
// Implementation notes-
//       This storage allocator allocates smaller storage Items from a set of
//       aligned Pages. Aligned pages are obtained from an AlignedAllocator.
//
//----------------------------------------------------------------------------
class Allocator {                   // Allocator descriptor
//----------------------------------------------------------------------------
// Allocator::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
typedef Allocator_detail::Page Page;
typedef Allocator_detail::Item Item;

enum {PAGE_CACHE= Allocator_detail::PAGE_CACHE };
enum {ITEM_CACHE= Allocator_detail::ITEM_CACHE };

//----------------------------------------------------------------------------
// Allocator::Attributes
//----------------------------------------------------------------------------
protected:
STATISTIC              stat_gets;   // Number of get calls
STATISTIC              stat_puts;   // Number of put calls
STATISTIC              stat_find;   // Number of find_page calls
STATISTIC              stat_free;   // Number of free_page calls

protected:
Latch                  itemLatch;   // Reserved object latch
Latch                  pageLatch;   // Extended object latch

// (Protected by itemLatch and/or atomic operation)
std::atomic<Item*>     itemCache[ITEM_CACHE]; // (Base) Atomic free cache
std::atomic<Item*>     itemHead;    // First available pre-allocated Item

// (Initialized in constructor)
size_t                 pageMask;    // Page alignment mask
size_t                 pageSize;    // The size of each allocated page
size_t                 pageExtented; // Number of Items per extented page
size_t                 pageReserved; // Number of Items in reserved page

Item*                  itemOrigin;  // Pre-allocated element origin
Item*                  itemEnding;  // Pre-allocated element end address (+1)
size_t                 itemSize;    // The size of each allocation Item

// (Protected by pageLatch)
Page*                  pageHead;    // First extended page
Page*                  pageTail;    // Final extended page
size_t                 usedPages;   // Number of allocated extention pages

//----------------------------------------------------------------------------
// Allocator::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
protected:
   Allocator( void );               // Default Constructor

public:
   Allocator(                       // Constructor
     size_t            size,        // The size of each Item
     size_t            reserved,    // The (minimum) reserved Item count
     size_t            extended);   // The (minimum) Items/extended Page count

virtual
   ~Allocator( void );              // Destructor

// Disallowed: Copy constructor, assignment operator
   Allocator(const Allocator&) = delete;
Allocator& operator=(const Allocator&) = delete;

//----------------------------------------------------------------------------
// Allocator::Internal methods
//----------------------------------------------------------------------------
protected:                          // Internal self-checks, normally unused
public:                             // (Allow external checking)
int check_item(Page*, Item*);       // (Note: Must hold pageLatch)
int check_item( void );             // (Note: Must NOT hold itemLatch)
int check_page( void );             // (Note: Must NOT hold pageLatch)
int check(Page*, void*);            // (Note: Must hold pageLatch)
int check(void*);                   // (Note: Must NOT hold pageLatch/itemLatch)
int check( void );                  // (Note: Must NOT hold pageLatch/itemLatch)

protected:
void                                // (Note: Must hold pageLatch
   debug_locked( void );            // (Internal) debugging display

public:                             // (Used by bringup debugging)
static void
   debug_static( void );            // Debug Page alllocator

void                                // (Note: Must NOT hold pageLatch/itemLatch)
   debug( void );                   // (Internal) debugging display

protected:                          // Internal methods
inline void*                        // The allocated Page
   find_page( void );               // Allocate Page

inline void
   free_page(                       // Delete Page
     void*             page);       // The page to delete

inline Page*                        // The first extended Page with Items
   get_extended( void );            // Get first extended Page with Items

inline Page*                        // The associated Page*
   get_page4(                       // The associated Page*
     void*             item) const  // For this Item*
{
   intptr_t addr= (intptr_t)item;
   addr &= pageMask;
   return (Page*)addr;
}

inline void
   ins_page(                        // Insert Page onto extended list (head)
     Page*             page);       // The page to insert

inline void
   ins_tail(                        // Insert Page onto extended list (tail)
     Page*             page);       // The page to insert

inline void
   rem_page(                        // Remove Page from extended list
     Page*             page);       // The page to remove

inline Item*                        // Item (allocated from extention Page)
   get_page( void );                // Allocate extention Page

inline void
   put_page(                        // Deallocate
     Page*             page);       // This extention page

//----------------------------------------------------------------------------
// Allocator::Accessors
//----------------------------------------------------------------------------
public:
size_t                              // The number of used pages
   get_used_pages( void ) const     // Get number of used pages
{  return usedPages; }

//----------------------------------------------------------------------------
// Allocator::Methods
//----------------------------------------------------------------------------
public:
virtual void*                       // The allocated Item
   get( void );                     // Allocate a Item

virtual void
   put(                             // Deallocate
     void*             addr);       // This Item
}; // class Allocator
}  // namespace _OBJ_NAMESPACE
#endif // OBJ_ALLOCATOR_H_INCLUDED
