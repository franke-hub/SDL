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
//       HttpSource.h
//
// Purpose-
//       Define a Http data source.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPSOURCE_H_INCLUDED
#define HTTPSOURCE_H_INCLUDED

#include <string>
#include <com/DataSource.h>
#include <com/Socket.h>

#ifndef PROPERTIES_H_INCLUDED
#include "Properties.h"
#endif

#ifndef URLCONNECTION_H_INCLUDED
#include "UrlConnection.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       HttpSource
//
// Purpose-
//       Define a Http data source.
//
//----------------------------------------------------------------------------
class HttpSource : public DataSource { // Http data source
//----------------------------------------------------------------------------
// HttpSource::Attributes
//----------------------------------------------------------------------------
protected:
class MetaVisitor;                  // Internal class HttpSource::MetaVisitor

UrlConnection          connect;     // The associated UrlConnection
Properties             reqProps;    // Request properties
Properties             rspProps;    // Response properties

//----------------------------------------------------------------------------
// HttpSource::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpSource( void );             // Destructor
   HttpSource( void );              // Default constructor

   HttpSource(                      // Constructor
     const char*       uri);        // The URL name

//----------------------------------------------------------------------------
// HttpSource::Methods
//----------------------------------------------------------------------------
public:
Properties&                        // The meta Properties
   setMetaProperties( void )       // Set meta Properties
{
   return reqProps;
}

const Properties&                   // The request Properties
   getRequestProperties( void ) const // Get request Properties
{
   return reqProps;
}

const Properties&                   // The response Properties
   getResponseProperties( void ) const // Get response Properties
{
   return rspProps;
}

const UrlConnection&                  // The UrlConnection
   getUrlConnection( void ) const     // Get UrlConnection
{
   return connect;
}

void
   setRequestProperty(              // Set request Property
     const std::string&name,        // Property name
     const std::string&value)       // Property value
{
   reqProps.setProperty(name, value);
}

virtual DataSource*                 // -> DataSource
   clone(                           // Clone this DataSource
     const char*       name) const; // With this (relative) name

virtual void
   close( void );                   // Close the URL

virtual int                         // Return code (0 OK)
   open(                            // Load a URL
     const char*       uri);        // The URL name

virtual int                         // Return code (0 OK)
   verify(                          // Verify a URI
     const char*       uri);        // The URL name

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::loadMetaProperties
//
// Purpose-
//       Load the <meta http_equiv=name content=value> properties
//
//----------------------------------------------------------------------------
public:
Properties&                        // The meta Properties
   loadMetaProperties(             // Load the meta Properties
     Properties&       properties);// Resultant Properties
}; // class HttpSource

#endif // HTTPSOURCE_H_INCLUDED
