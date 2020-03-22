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
//       detail/Allocator.h
//
// Purpose-
//       Allocator.h detail, included from ../Allocator.h                                   ts.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
namespace _OBJ_NAMESPACE {
namespace Allocator_detail {
// ITEM_CACHE: The number of atomically allocated and released base Items
enum {ITEM_CACHE= 8};

// PAGE_CACHE: The number extended pages examined for Item allocation
enum {PAGE_CACHE= 2};

struct Item {                       // Item
   Item*               next;        // Next Item in list
}; // struct Item

struct Page {                       // Item extention page descriptor
   Page*               next;        // Next Page in list
   Page*               prev;        // Prior Page in list
   Item*               head;        // First available Item in list
   uint32_t            offset;      // Offset of first available Item
   uint32_t            avails;      // Number of available Items
}; // struct Page
} // namespace Allocator_detail
} // namespace _OBJ_NAMESPACE
