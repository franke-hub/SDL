//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Url.cpp
//
// Purpose-
//       Url implementation methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "Common.h"
#include "TextBuffer.h"

#include "Url.h"

using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Enum-
//       Item
//
// Purpose-
//       URL item enumerator.
//
//----------------------------------------------------------------------------
enum Item                           // URL Item
{  ITEM_URL                         // The entire URL
,  ITEM_PROTO                       // Protocol
,  ITEM_AUTH                        // Authority
,  ITEM_AUTHUSER                    // Authority user info
,  ITEM_AUTHHOST                    // Authority host
,  ITEM_AUTHPORT                    // Authority port
,  ITEM_PATH                        // Path
,  ITEM_QUERY                       // Query
,  ITEM_FRAGMENT                    // Fragment
}; // enum Item

//----------------------------------------------------------------------------
//
// Subroutine-
//       checker
//
// Purpose-
//       URI checker.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   checker(                         // Check URI
     const string&     uri)         // The URI string
{
   int                 C;           // Current character
   TextBuffer          part;        // The URI part
   char*               ptrC;        // Working char*
   char*               ptrT;        // Working char*
   const int           size= uri.size(); // URI size
   int                 x= 0;        // URI index

   C= 0;
   part.reset();
   while( x < size )
   {
     C= uri[x++];
     if( C == ':' )
       break;

     part.put(C);
   }

   if( C != ':' )
     return Url::RC_NOPROTO;

   ptrC= part.toChar();
   if( *ptrC == '\0' )
     return Url::RC_NOPROTO;

   while( *ptrC != '\0' )
   {
     if( !isalpha(*ptrC) )
       return Url::RC_SYNTAX;

     ptrC++;
   }

   if( x >= size || uri[x++] != '/' )
     return Url::RC_SYNTAX;
   if( x >= size || uri[x++] != '/' )
     return Url::RC_SYNTAX;

   part.reset();
   while( x < size )
   {
     C= uri[x++];
     if( C == '/' || C == '?' || C == '#' )
       break;

     part.put(C);
   }

   ptrC= part.toChar();
   if( *ptrC == '\0' )
     return Url::RC_NOHOST;

   ptrT= strchr(ptrC, '@');
   if( ptrT != NULL )
   {
     ptrC= ptrT + 1;
     if( strchr(ptrC, '@') != NULL )
       return Url::RC_SYNTAX;
   }

   ptrT= strchr(ptrC, ':');
   if( ptrT != NULL )
   {
     ptrC= ptrT + 1;
     if( *ptrC == '\0' )
       return Url::RC_SYNTAX;

     while( *ptrC != '\0' )
     {
       if( !isdigit(*ptrC) )
         return Url::RC_SYNTAX;

       ptrC++;
     }
   }

   return Url::RC_OK;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parser
//
// Purpose-
//       URL parser.
//
//----------------------------------------------------------------------------
static string                       // Resultant
   parser(                          // Parse URL
     const string&     uri,         // URI string
     Item              item)        // URI item
{
   TextBuffer          part;        // The URL part
   int                 C;           // Current character
   char*               ptrC;        // Working char*
   char*               ptrT;        // Working char*
   const int           size= uri.size(); // URI size
   int                 x= 0;        // URI index

   if( size == 0 )
     return "";

   C= 0;
   part.reset();
   while( x < size )
   {
     C= uri[x++];
     if( C == ':' )
       break;

     part.put(tolower(C));
   }
   if( item == ITEM_PROTO )
     return part.toString();

   x += 2;
   part.reset();
   while( x < size )
   {
     C= uri[x++];
     if( C == '/' || C == '?' || C == '#' )
       break;

     part.put(C);
   }
   if( item == ITEM_AUTH )
     return part.toString();
   else if( item == ITEM_AUTHHOST )
   {
     ptrC= part.toChar();
     ptrT= strchr(ptrC, '@');
     if( ptrT != NULL )
       ptrC= ptrT + 1;

     ptrT= strchr(ptrC, ':');
     if( ptrT != NULL )
       *ptrT= '\0';

     return ptrC;
   }
   else if( item == ITEM_AUTHUSER )
   {
     ptrC= part.toChar();
     ptrT= strchr(ptrC, '@');
     if( ptrT == NULL )
       return "";

     *ptrT= '\0';
     return ptrC;
   }
   else if( item == ITEM_AUTHPORT )
   {
     ptrC= part.toChar();
     ptrT= strchr(ptrC, ':');
     if( ptrT == NULL )
       return "";

     return ptrT+1;
   }

   part.reset();
   if( C != '?' && C != '#' && x < size )
   {
     while( x < size )
     {
       C= uri[x++];
       if( C == '?' || C == '#' )
         break;

       part.put(C);
     }
   }
   if( item == ITEM_PATH )
     return part.toString();
   if( x >= size )
     return "";

   if( C == '?' )
   {
     part.reset();
     while( x < size )
     {
       C= uri[x++];
       if( C == '#' )
         break;

       part.put(C);
     }

     if( item == ITEM_QUERY )
       return part.toString();
   }
   if( item == ITEM_QUERY )
     return "";

   if( C != '#' || item != ITEM_FRAGMENT )
   {
     fprintf(stderr, "%4d %s SNO(%c,%d)\n", __LINE__, __FILE__, C, item);
     return "";
   }

   part.reset();
   while( x < size )
   {
     C= uri[x++];
     part.put(C);
   }

   return part.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       Url::~Url
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Url::~Url( void )                // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Url::Url
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Url::Url( void )                 // Default constructor
:  uri()
{
}

   Url::Url(                        // Constructor
     const string&     uri)         // Using this URI
:  uri()
{
   setURI(uri);
}

   Url::Url(                        // Copy constructor
     const Url&        source)      // Source Url
:  uri(source.uri)
{
}

Url&                                // Resultant (*this)
   Url::operator=(                  // Assignment operator
     const Url&        source)      // Source Url
{
   uri= source.uri;
   return *this;
}

Url&                                // Resultant (*this)
   Url::operator=(                  // Assignment operator
     const string&     uri)         // Source URI
{
   setURI(uri);
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Url::Accessors
//
// Purpose-
//       Accessor functions.
//
//----------------------------------------------------------------------------
string                              // Resultant
   Url::getAuthority( void ) const  // Get Authority
{
   return parser(uri, ITEM_AUTH);
}

int                                 // Resultant
   Url::getDefaultPort( void ) const// Get default port
{
   string              prot= parser(uri, ITEM_PROTO);

   if( prot == "ftp" )
     return 21;

   if( prot == "http" )
     return 80;

   return (-1);
}

string                              // Resultant
   Url::getFragment( void ) const   // Get Fragment
{
   return parser(uri, ITEM_FRAGMENT);
}

string                              // Resultant
   Url::getHost( void ) const       // Get Host
{
   return parser(uri, ITEM_AUTHHOST);
}

string                              // Resultant
   Url::getPath( void ) const       // Get Path
{
   return parser(uri, ITEM_PATH);
}

int                                 // Resultant
   Url::getPort( void ) const       // Get Port
{
   string              text= parser(uri, ITEM_AUTHPORT);
   int                 port= (-1);  // Resultant
   int                 i;

   if( text.size() > 0 )
   {
     port= 0;

     for(i= 0; i<text.size(); i++)
     {
       port *= 10;
       port += (text[i] - '0');
     }
   }

   return port;
}

string                              // Resultant
   Url::getProtocol( void ) const   // Get protocol
{
   return parser(uri, ITEM_PROTO);
}

string                              // Resultant
   Url::getQuery( void ) const      // Get query
{
   return parser(uri, ITEM_QUERY);
}

string                              // Resultant
   Url::getURI( void ) const        // Get URI
{
   return uri;
}

string                              // Resultant
   Url::getUserInfo( void ) const   // Get user info
{
   return parser(uri, ITEM_AUTHUSER);
}

//----------------------------------------------------------------------------
//
// Method-
//       Url::set
//
// Purpose-
//       Set the URI
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Url::setURI(                     // Set the URI
     const string&     uri)         // Using this URI string
{
   this->uri= "";
   int result= checker(uri);
   if( result == 0 )
     this->uri= uri;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Url::reset
//
// Purpose-
//       Reset the URI
//
//----------------------------------------------------------------------------
void
   Url::reset( void )               // Reset the URI
{
   uri= "";
}

