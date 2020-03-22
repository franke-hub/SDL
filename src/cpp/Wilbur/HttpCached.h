//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpCached.h
//
// Purpose-
//       Define a Cached HTTP DataSource.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPCACHED_H_INCLUDED
#define HTTPCACHED_H_INCLUDED

#ifndef HTTPSOURCE_H_INCLUDED
#include "HttpSource.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       HttpCached
//
// Purpose-
//       Define a cached HTTP DataSource.
//
//----------------------------------------------------------------------------
class HttpCached : public HttpSource { // Http cached data source
//----------------------------------------------------------------------------
// HttpCached::Attributes
//----------------------------------------------------------------------------
protected:
unsigned               nullTimeout; // The null result timeout

//----------------------------------------------------------------------------
// HttpCached::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpCached( void );             // Destructor
   HttpCached( void );              // Default constructor

//----------------------------------------------------------------------------
// HttpCached::Methods
//----------------------------------------------------------------------------
public:
unsigned                            // The null resultant cache hold time
   getNullTimeout( void ) const     // Get the null resultant cache hold time
{
   return nullTimeout;
}

void
   setNullTimeout(                  // Set the null resultant cache hold time
     unsigned          seconds)     // To this many seconds
{
   nullTimeout= seconds;
}

virtual int                         // Return code (0 OK)
   open(                            // Load the HttpCached
     const char*       uri,         // For this URL string
     int               cached= FALSE); // TRUE to only load from cache
}; // class HttpCached

#endif // HTTPCACHED_H_INCLUDED
