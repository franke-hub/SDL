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
//       Recorder.cpp
//
// Purpose-
//       Implement http/Recorder.h
//
// Last change date-
//       2022/09/15
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For fprintf()
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception

#include "pub/http/Recorder.h"      // For pub::Recorder, implemented

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
Recorder*              Recorder::common= nullptr;
Recorder::mutex_t      Recorder::mutex;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Recorder*       internal= nullptr; // The auto-allocated Recorder

static struct GlobalDestructor {    // On unload, remove Recorder::global
inline
   ~GlobalDestructor( void )
{  Recorder::set(nullptr); }        // Cleans up Recorder::common & internal
}  globalDestructor;

//----------------------------------------------------------------------------
//
// Method-
//       Recorder::Recorder
//       Recorder::~Recorder
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Recorder::Recorder( void )
{  if( HCDM && VERBOSE > 0 ) debugf("Recorder(%p)::Recorder\n", this); }

   Recorder::~Recorder( void )
{  if( HCDM && VERBOSE > 0 ) debugf("Recorder(%p)::~Recorder\n", this);

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
//       Recorder::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Recorder::debug(const char* info) const  // Debugging display
{  debugf("Recorder(%p)::debug(%s)\n", this, info);

   size_t index= 0;
   for(RecordItem* item= list.get_head(); item; item= item->get_next()) {
     debugf("[%3zd] %s\n", index, item->record->name.c_str());
     ++index;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Recorder::get
//
// Function-
//       Extract the current default Recorder.
//
//----------------------------------------------------------------------------
Recorder*                           // -> Current default Recorder
   Recorder::get( void )            // Get the current default Recorder
{
   Recorder* result= Recorder::common;
   if( result == nullptr ) {
     std::lock_guard<decltype(mutex)> lock(mutex);

     result= Recorder::common;
     if( result == nullptr ) {
       result= internal= Recorder::common= new Recorder();
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Recorder::set
//
// Function-
//       Update the default Recorder
//
//----------------------------------------------------------------------------
Recorder*                           // The removed Recorder object
   Recorder::set(                   // Set
     Recorder*         insert)      // This new default Recorder
{
   std::lock_guard<decltype(mutex)> lock(mutex);

   Recorder* removed = Recorder::common;
   if( removed == internal ) {
     delete internal;
     removed= internal= nullptr;
   }

   Recorder::common= insert;
   return removed;
}

//----------------------------------------------------------------------------
//
// Method-
//       Recorder::insert
//
// Purpose-
//       Insert record
//
//----------------------------------------------------------------------------
void
   Recorder::insert(                // Insert
     Record*           record)      // This record
{  if( HCDM && VERBOSE > 0 )
     debugf("Recorder(%p)::insert(%p) %s\n", this
           , record, record->name.c_str());

   // NO DUPLICATE CHECK
   std::lock_guard<decltype(mutex)> lock(mutex);
   RecordItem* item= new RecordItem(record);
   list.fifo(item);
}

//----------------------------------------------------------------------------
//
// Method-
//       Recorder::remove
//
// Purpose-
//       Remove record
//
//----------------------------------------------------------------------------
void
   Recorder::remove(                // Remove
     Record*           record)      // This record
{  if( HCDM && VERBOSE > 0 )
     debugf("Recorder(%p)::remove(%p) %s\n", this
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
//       Recorder::report
//
// Purpose-
//       Generate report
//
//----------------------------------------------------------------------------
void
   Recorder::report(                // Generate report
     f_reporter        reporter)    // Using this reporter
{  if( HCDM && VERBOSE > 0 ) debugf("Recorder(%p)::report\n", this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   for(RecordItem* item= list.get_head(); item; item= item->get_next()) {
     reporter(*(item->record));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Recorder::reset
//
// Purpose-
//       Reset all Records
//
//----------------------------------------------------------------------------
void
   Recorder::reset( void )          // Reset all Records
{  if( HCDM && VERBOSE > 0 ) debugf("Recorder(%p)::reset\n", this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   for(RecordItem* item= list.get_head(); item; item= item->get_next()) {
     item->record->h_reset();
   }
}
}  // namespace _LIBPUB_NAMESPACE
