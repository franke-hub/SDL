//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       BG_CleanCache.h
//
// Purpose-
//       BG Service: Clean Cache.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef BG_CLEANCACHE_H_INCLUDED
#define BG_CLEANCACHE_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class DispatchItem;                 // Dispatch: work Item

//----------------------------------------------------------------------------
//
// Class-
//       BG_CleanCache
//
// Purpose-
//       Drive background services and insure they terminate properly.
//
//----------------------------------------------------------------------------
class BG_CleanCache : public DispatchTask {
//----------------------------------------------------------------------------
// BG_CleanCache::Constructors
//----------------------------------------------------------------------------
public:
   ~BG_CleanCache( void );          // Destructor
   BG_CleanCache( void );           // Constructor

//----------------------------------------------------------------------------
// BG_CleanCache::Methods
//----------------------------------------------------------------------------
public:
virtual void
   work(
     DispatchItem*     item);
}; // class BG_CleanCache

#endif // BG_CLEANCACHE_H_INCLUDED
