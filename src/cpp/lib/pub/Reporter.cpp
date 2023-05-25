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
//       Reporter.cpp
//
// Purpose-
//       Implement Reporter.h
//
// Last change date-
//       2022/05/25
//
//----------------------------------------------------------------------------
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception

#include "pub/Reporter.h"           // For pub::Reporter, implemented

using namespace _LIBPUB_NAMESPACE;
using namespace _LIBPUB_NAMESPACE::debugging;
using std::string;

namespace _LIBPUB_NAMESPACE {       // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Reporter*              Reporter::common= nullptr;
Reporter::mutex_t      Reporter::mutex;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Reporter*       internal= nullptr; // The auto-allocated Reporter

static struct GlobalDestructor {
inline
   ~GlobalDestructor( void )
{  Reporter::set(nullptr); }        // Cleans up Reporter::common & internal
}  globalDestructor;

//----------------------------------------------------------------------------
//
// Method-
//       Reporter::Reporter
//       Reporter::~Reporter
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Reporter::Reporter( void )
{  if( HCDM && VERBOSE > 0 ) debugf("Reporter(%p)::Reporter\n", this); }

   Reporter::~Reporter( void )
{  if( HCDM && VERBOSE > 0 ) debugf("Reporter(%p)::~Reporter\n", this);

   for(;;) {
     RecordItem* item= list.remq();
     if( item == nullptr )
       break;

     delete item;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Reporter::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Reporter::debug(const char* info) const  // Debugging display
{  debugf("Reporter(%p)::debug(%s)\n", this, info);

   size_t index= 0;
   for(RecordItem* item= list.get_head(); item; item= item->get_next()) {
     debugf("[%3zd] %s\n", index, item->record->name.c_str());
     ++index;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Reporter::get
//
// Function-
//       Extract the current default Reporter.
//
//----------------------------------------------------------------------------
Reporter*                           // -> Current default Reporter
   Reporter::get( void )            // Get the current default Reporter
{
   Reporter* result= Reporter::common;
   if( result == nullptr ) {
     std::lock_guard<decltype(mutex)> lock(mutex);

     result= Reporter::common;
     if( result == nullptr ) {
       result= internal= Reporter::common= new Reporter();
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reporter::set
//
// Function-
//       Update the default Reporter
//
//----------------------------------------------------------------------------
Reporter*                           // The removed Reporter object
   Reporter::set(                   // Set
     Reporter*         insert)      // This new default Reporter
{
   std::lock_guard<decltype(mutex)> lock(mutex);

   Reporter* removed = Reporter::common;
   if( removed == internal ) {
     delete internal;
     removed= internal= nullptr;
   }

   Reporter::common= insert;
   return removed;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reporter::insert
//
// Purpose-
//       Insert record
//
//----------------------------------------------------------------------------
void
   Reporter::insert(                // Insert
     Record*           record)      // This record
{  if( HCDM && VERBOSE > 0 )
     debugf("Reporter(%p)::insert(%p) %s\n", this
           , record, record->name.c_str());

   // NOTE: DUPLICATE CHECK NOT IMPLEMENTED
   std::lock_guard<decltype(mutex)> lock(mutex);
   RecordItem* item= new RecordItem(record);
   list.fifo(item);
}

//----------------------------------------------------------------------------
//
// Method-
//       Reporter::remove
//
// Purpose-
//       Remove record
//
//----------------------------------------------------------------------------
void
   Reporter::remove(                // Remove
     Record*           record)      // This record
{  if( HCDM && VERBOSE > 0 )
     debugf("Reporter(%p)::remove(%p) %s\n", this
           , record, record->name.c_str());

   std::lock_guard<decltype(mutex)> lock(mutex);
   for(RecordItem* item= list.get_head(); item; item= item->get_next()) {
     if( item->record == record ) {
       list.remove(item);
       delete item;
       break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Reporter::report
//
// Purpose-
//       Generate report
//
//----------------------------------------------------------------------------
void
   Reporter::report(                // Generate report
     f_reporter        reporter)    // Using this reporter
{  if( HCDM && VERBOSE > 0 ) debugf("Reporter(%p)::report\n", this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   for(RecordItem* item= list.get_head(); item; item= item->get_next()) {
     reporter(*(item->record));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Reporter::reset
//
// Purpose-
//       Reset all Records
//
//----------------------------------------------------------------------------
void
   Reporter::reset( void )          // Reset all Records
{  if( HCDM && VERBOSE > 0 ) debugf("Reporter(%p)::reset\n", this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   for(RecordItem* item= list.get_head(); item; item= item->get_next()) {
     item->record->h_reset();
   }
}
}  // namespace _LIBPUB_NAMESPACE