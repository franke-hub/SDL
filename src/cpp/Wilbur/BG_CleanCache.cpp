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
//       BG_CleanCache.cpp
//
// Purpose-
//       BG_CleanCache implementation methods.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#define __STDC_FORMAT_MACROS        // For linux inttypes.h
#include <assert.h>
#include <inttypes.h>               // For PRI*64
#include <stdio.h>

#include <com/Julian.h>
#include <com/Thread.h>

#include "Common.h"
#include "DbHttp.h"
#include "DbMeta.h"
#include "DbText.h"

#include "BG_CleanCache.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       BG_CleanCache::~BG_CleanCache
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   BG_CleanCache::~BG_CleanCache( void ) // Destructor
{
   IFHCDM( logf("BG_CleanCache(%p)::~BG_CleanCache()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       BG_CleanCache::BG_CleanCache
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   BG_CleanCache::BG_CleanCache( void ) // Constructor
:  DispatchTask()
{
   IFHCDM( logf("BG_CleanCache(%p)::BG_CleanCache()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       BG_CleanCache::work
//
// Purpose-
//       Process work Item
//
//----------------------------------------------------------------------------
void
   BG_CleanCache::work(             // Process Work
     DispatchItem*     item)        // The work Item
{
   IFHCDM( logf("BG_CleanCache(%p)::work(%p)...\n", this, item); )

   Julian              now;         // The current time

   uint64_t            httpIX;      // HTTP index
   uint64_t            timeEX;      // Expiration time

   //-------------------------------------------------------------------------
   // Examine DbHttp, looking for expired cache elements
   DbMeta* dbMeta= DbMeta::get();
   DbHttp* dbHttp= dbMeta->dbHttp;
   DbText* dbText= dbMeta->dbText;

   IFHCDM( logf("NOW: %.3f\n", now.getTime()); )
   for(;;)
   {
     httpIX= timeEX= 0;             // Begin at the beginning
     httpIX= dbHttp->nextTime(httpIX, timeEX); // Get the first entry
     if( httpIX == 0 )              // If nothing left
       break;

     char buffer[DbHttp::MAX_VALUE_LENGTH+1]; // Resultant buffer
     DbHttp::Value* value= dbHttp->getValue(buffer, httpIX);
     assert( value != NULL );

     timeEX= dbHttp->fetch64(&value->time); // Get next time
     if( double(timeEX) > now )     // If not expired
       break;

     uint64_t textIX= dbHttp->fetch64(&value->text);
     IFHCDM( logf("%4d BG_CleanCache text(%.16" PRIx64 ") time(%16" PRId64 ") "
                  "now(%.3f) http(http://%s)\n", __LINE__,
                  textIX, timeEX, now.getTime(), value->name);
     )

     // Delete both the HTTP and the TEXT entries
     DbTxn* dbTxn= dbHttp->getTxn(); // Create transaction

     dbText->remove(textIX, dbTxn); // Delete the associated TEXT entry
     dbHttp->remove(httpIX, dbTxn); // Delete the associated HTTP entry

     dbHttp->commit(dbTxn);         // Commit transaction

     Thread::sleep(0.125);          // Delay between entries
   }

   item->post(0);
   IFHCDM( logf("...BG_CleanCache(%p)::work(%p)\n", this, item); )
}

