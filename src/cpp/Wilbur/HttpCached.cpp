//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpCached.cpp
//
// Purpose-
//       HttpCached implementation methods.
//
// Last change date-
//       2023/08/04
//
//----------------------------------------------------------------------------
#define __STDC_FORMAT_MACROS        // For linux inttypes.h
#include <string>

#include <assert.h>
#include <inttypes.h>               // For PRIx64
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Julian.h>
#include <com/Parser.h>
#include <sys/types.h>

#include "DateParser.h"
#include "DbHttp.h"
#include "DbMeta.h"
#include "DbText.h"
#include "Url.h"

#include "HttpCached.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define IX_HTTP_DATA 3              // DbInfo HTTP index (Link)
#define IX_HTTP_TIME 2              // DbInfo HTTP index (Expiration time)
#define IX_TEXT_DATA 1              // DbInfo TEXT index (Link)
#define IX_TEXT_TIME 0              // DbInfo TEXT index (Expiration time)

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, I/O Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include <com/ifmacro.h>

typedef int64_t JulianSecond;       // A JulianSecond

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseDec
//
// Purpose-
//       Parse a string, extracting a decimal value.
//       Leading blanks are skipped.
//
// Returns-
//       Return (decimal value)
//       String (The value delimiter)
//
//----------------------------------------------------------------------------
static int                          // Return value
   parseDec(                        // Extract decimal value from string
     const char*&      C)           // -> String (updated)
{
   Parser parser(C);
   int result= parser.toDec();
   C= parser.getString();
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpCached::~HttpCached
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpCached::~HttpCached( void )  // Destructor
{
   IFSCDM( traceh("%4d HttpCached::~HttpCached()\n", __LINE__); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpCached::HttpCached
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   HttpCached::HttpCached( void )   // Default constructor
:  HttpSource()
,  nullTimeout(0)
{
   IFSCDM( traceh("%4d HttpCached::HttpCached()\n", __LINE__); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpCached::open
//
// Purpose-
//       Load the URI.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   HttpCached::open(                // Load the HttpSource
     const char*       uri,         // For this URL string
     int               cached)      // TRUE to only load from cache
{
   int                 result= (-1);// Resultant

   char                buffer[DbHttp::MAX_VALUE_LENGTH+1];
   DbHttp::Value*      value;

   uint64_t            httpIX;      // The DbHttp index
   uint64_t            textIX;      // The DbText index
   int64_t             timeEX;      // Expiration time (Julian second)

   Julian              now;         // The current time

   int                 rc;

   //-------------------------------------------------------------------------
   // Verify the URI
   //-------------------------------------------------------------------------
   IFSCDM( traceh("%4d HttpCached::open(%s)\n", __LINE__, uri); )
   reset();                         // Delete any existing data

   rc= verify(uri);                 // Verify the URI
   if( rc != 0 )
     return rc;

   connect.setUrl(uri);             // Reset the URL and name
   this->name= uri;

   //-------------------------------------------------------------------------
   // Load the URL from cache
   //-------------------------------------------------------------------------
   DbMeta* dbMeta= DbMeta::get();
   DbHttp* dbHttp= dbMeta->dbHttp;
   DbText* dbText= dbMeta->dbText;

   textIX= 0;                       // Default, no text index
   httpIX= dbHttp->locate(uri);     // Get the DbHttp index
   IFHCDM( traceh("%4d HttpCached httpIX(%" PRIx64 ")\n", __LINE__, httpIX); )
   if( httpIX != 0 )                // If name is cached
   {
     value= dbHttp->getValue(buffer, httpIX); // Get the HTTP value
     if( value != NULL )
     {
       IFHCDM( traceh("%4d HttpCached <found>\n", __LINE__); )

       // It's possible that we have valid data even though the cache
       // expiration time has been met.
       textIX= dbHttp->fetch64(&value->text);
       timeEX= dbHttp->fetch64(&value->time);
       if( JulianSecond(now.getTime()) >= timeEX )
         IFHCDM( traceh("%4d HttpCached <expired>\n", __LINE__); )
       else
       {
         if( textIX == 0 )
         {
           origin= (unsigned char*)malloc(1);
           if( origin != NULL )
             *origin= '\0';
         }
         else
           origin= (unsigned char*)dbText->getValue(textIX);

         if( origin != NULL )
         {
           //-----------------------------------------------------------------
           // Load from cache successful
           length= strlen((char*)origin);
           setWidth();

           IFHCDM( traceh("%4d HttpCached <cached>\n", __LINE__); )
           return 0;
         }
       }
     }
   }

   if( cached )                     // If only cache load allowed
     return (-1);                   // Failed

   //-------------------------------------------------------------------------
   // Load the URI from source
   //-------------------------------------------------------------------------
   result= HttpSource::open(uri);
   IFHCDM( traceh("%4d HttpCached %d= HttpSource())\n", __LINE__, result); )
   if( result != 0 && result != 200 ) // If failure (NULL result)
   {
     if( nullTimeout == 0 || result != 404 )
     {
       if( textIX != 0 )
         dbText->remove(textIX);

       IFHCDM( traceh("%4d HttpCached <failed> %d\n", __LINE__, result); )
       return result;
     }

     result= 0;
     if( origin == NULL )
       origin= (unsigned char*)malloc(1);
     origin[0]= '\0';
     length= 0;
     char work[32];
     sprintf(work, "%u", nullTimeout);
     rspProps.setProperty("max-age", work);
   }

   //-------------------------------------------------------------------------
   // Append the <meta http-equiv=name content=value> properties
   //-------------------------------------------------------------------------
   IFHCDM( traceh("%4d HttpCached <loaded> %d\n", __LINE__, result); )
   if( TRUE )                       // Select meta properties?
     loadMetaProperties(rspProps);  // Load the response properties

   //-------------------------------------------------------------------------
   // Determine the cache expiration time
   //-------------------------------------------------------------------------
   timeEX= uint64_t(now.getTime()) + 7*24*3600; // Default, one week in future

   const char* C= rspProps.getProperty("max-age");
   if( C != NULL )
     timeEX= uint64_t(now.getTime()) + parseDec(C);

   C= rspProps.getProperty("expires");
   if( C != NULL )
   {
     time_t nTime= DateParser::parse(C);
     timeEX= uint64_t(Julian::getUTC1970Time()) + nTime;
   }

   C= rspProps.getProperty("cache-control");
   if( C != NULL )
   {
     if( stricmp("no-cache", C) == 0 || stricmp("no-store", C) == 0 )
       return 0;                    // Return, do not cache

     if( memicmp("max-age=", C, 8) == 0 )
     {
       C += 8;
       timeEX= uint64_t(now.getTime()) + parseDec(C);
     }
   }

   IFHCDM( traceh("%4d HttpCached now(%f) timeEX(%f)\n", __LINE__, (double)now, (double)timeEX); )

   //-------------------------------------------------------------------------
   // Transactionally insert/revise the HTTP and TEXT data base entries
   //-------------------------------------------------------------------------
   int L= strlen(uri) + sizeof(uint64_t) + sizeof(uint64_t);
   if( L > DbHttp::MAX_VALUE_LENGTH ) // If name too long
     return result;                 // Do not cache

   DbTxn* dbTxn= dbHttp->getTxn();

   IFHCDM( traceh("%4d HttpCached textIX(%" PRIx64 ") insert\n", __LINE__, textIX); )
   if( textIX == 0 )
   {
     if( length > 0 )
     {
       textIX= dbText->insert((char*)origin, dbTxn);
       if( textIX == 0 )
       {
         IFHCDM( traceh("%4d HttpCached abort\n", __LINE__); )
         dbHttp->abort(dbTxn);      // Abort transaction
         return result;             // Return result (not cached)
       }
     }
   }
   else
   {
     if( length == 0 )
     {
       dbText->remove(textIX, dbTxn);
       textIX= 0;
     }
     else if( dbText->revise(textIX, (char*)origin, dbTxn) != 0 )
     {
       IFHCDM( traceh("%4d HttpCached abort\n", __LINE__); )
       dbHttp->abort(dbTxn);        // Abort transaction
       return result;               // Return result (not cached)
     }
   }

   value= dbHttp->setValue(buffer, textIX, timeEX, uri);
   if( httpIX == 0 )
   {
     httpIX= dbHttp->insert(value, dbTxn);
     IFHCDM( traceh("%4d HttpCached httpIX(%" PRIx64 ") insert\n", __LINE__, httpIX); )
   }
   else                             // Index (and buffer) are valid
   {
     if( dbHttp->revise(httpIX, value, dbTxn) != 0 )
       httpIX= 0;
     IFHCDM( traceh("%4d HttpCached httpIX(%" PRIx64 ") revise\n", __LINE__, httpIX); )
   }

   if( httpIX == 0 )
   {
     IFHCDM( traceh("%4d HttpCached abort\n", __LINE__); )
     dbHttp->abort(dbTxn);
   }
   else
   {
     IFHCDM( traceh("%4d HttpCached commit\n", __LINE__); )
     dbHttp->commit(dbTxn);
   }

   return 0;
}

int                                 // Return code (0 OK)
   HttpCached::open(                // Load the HttpSource
     const char*       uri)         // For this URL string
{  return open(uri, false); }
