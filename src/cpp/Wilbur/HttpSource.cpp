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
//       HttpSource.cpp
//
// Purpose-
//       HttpSource implementation methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/istring.h>
#include <com/Debug.h>
#include <com/Interval.h>
#include <com/Parser.h>
#include <com/Socket.h>
#include <com/Software.h>
#include <com/Thread.h>
#include <com/Unconditional.h>

#include "Common.h"
#include "Diagnostic.h"
#include "Global.h"
#include "HtmlNode.h"
#include "HtmlParser.h"
#include "HtmlNodeVisitor.h"
#include "TextBuffer.h"
#include "Url.h"

#include "HttpSource.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, I/O Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

enum                                // Generic enum
{  CHUNK_SIZE= 16384                // Chunk size
,  TRACE_SIZE= 16                   // IODM trace size
}; // enum

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Class-
//       HttpSource::MetaVisitor
//
// Purpose-
//       Visit the HTML nodes, extracting META entries.
//
//----------------------------------------------------------------------------
class HttpSource::MetaVisitor : public HtmlNodeVisitor {
//----------------------------------------------------------------------------
// HttpSource::MetaVisitor::Attributes
//----------------------------------------------------------------------------
public:
Properties&            properties;  // The updated Properties

//----------------------------------------------------------------------------
// HttpSource::MetaVisitor::Constructors
//----------------------------------------------------------------------------
public:
   MetaVisitor(
     Properties&       properties)
:  HtmlNodeVisitor()
,  properties(properties)
{}

//----------------------------------------------------------------------------
// HttpSource::MetaVisitor::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   visit(                           // Visit text HtmlNodes
     HtmlNode*         node)        // -> HtmlNode
{
   #if FALSE
     debugf("MetaVisitor::visit() ");
     debugf("type(%d) ", node->getType());
     debugf("name(%s) ", node->getName().c_str());
     debugf("data(%s) ", node->getData().c_str());
     debugf("\n");
   #endif

   // Look for META element nodes
   if( node->getType() == HtmlNode::TYPE_ELEM // If ELEMENT Node
       && stricmp("meta", node->getName().c_str()) == 0 ) // With META tag
   {
     ElemHtmlNode* elem= dynamic_cast<ElemHtmlNode*>(node);
     HtmlNode* child= elem->getChild();

     for(;;)                        // Get http-equiv/content value pairs
     {
       while( child != NULL && child->getType() != HtmlNode::TYPE_ATTR )
         child= child->getPeer();

       if( child == NULL )
         break;

       if( stricmp("http-equiv", child->getName().c_str()) != 0 )
       {
         debugf("META: expected(http-equiv), got(%s), \n",
                child->getName().c_str());
         break;
       }

       AttrHtmlNode* attr= dynamic_cast<AttrHtmlNode*>(child);
       std::string name= attr->getData(); // The http-equiv commmand

       child= child->getPeer();
       while( child != NULL && child->getType() != HtmlNode::TYPE_ATTR )
         child= child->getPeer();

       if( child == NULL )
       {
         debugf("META: missing(content) attribute\n");
         break;
       }

       if( stricmp("content", child->getName().c_str()) != 0 )
       {
         debugf("META: expected(content), got(%s), \n",
                child->getName().c_str());
         break;
       }

       attr= dynamic_cast<AttrHtmlNode*>(child);
       std::string value= attr->getData(); // The content value
       #if defined(HCDM) && FALSE
         debugf("Found (%s:%s)\n", name.c_str(), value.c_str());
       #endif
       properties.setProperty(name, value);

       child= child->getPeer();
     }
   }

   return 0;
}
}; // class HttpSource::MetaVisitor

//----------------------------------------------------------------------------
//
// Subroutine-
//       getSocket
//
// Purpose-
//       Get socket from UrlConnection, reconnecting if required.
//
//----------------------------------------------------------------------------
static Socket*                      // -> Socket, NULL if cannot connect
   getSocket(                       // Get Socket from UrlConnection
     UrlConnection&    connect)     // The UrlConnection
{
   Socket*             socket= NULL;// Resultant

   if( connect.isConnected() )
     return &connect.getSocket();

   int rc= connect.connect();
   IFIODM( logf("%4d %s %d= connect\n", __LINE__, __FILE__, rc); )
   if( rc != 0 )
   {
     debugf("%4d %s connect failure(%d) %s\n", __LINE__, __FILE__,
            rc, connect.getSocket().getSocketEI());
   }
   else
   {
     socket= &connect.getSocket();
     rc= socket->setSocketSO(Socket::SO_RCVTIMEO, 3000);
     IFIODM( logf("%4d %s %d= socket.setSocketSO(SO_RCVTIMEO,3000)\n",
                  __LINE__, __FILE__, rc); )
     if( rc != 0 )
     {
       debugf("%4d %s socket.setSocketSO() Error(%s)\n",
              __LINE__, __FILE__, socket->getSocketEI());
     }
   }

   logf("%p= getConnection(%s)\n", socket, connect.getUrl().getURI().c_str());

   return socket;
}

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
// Subroutine-
//       strip
//
// Purpose-
//       Remove leading and trailing blanks from a string.
//
//----------------------------------------------------------------------------
static char*                        // -> Stripped string
   strip(                           // Strip a string
     char*             text)        // -> string (!MODIFIED!)
{
   int                 i;

   while( *text == ' ' )            // Remove leading blanks
     text++;

   i= strlen(text);
   for(i= strlen(text); i>0 && text[i-1] == ' '; i--)
     ;

   text[i]= '\0';
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::~HttpSource
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpSource::~HttpSource( void )  // Destructor
{
   IFSCDM( logf("%4d HttpSource::~HttpSource()\n", __LINE__); )

   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::HttpSource
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   HttpSource::HttpSource( void )   // Default constructor
:  DataSource()
,  connect(), reqProps(),  rspProps()
{
   IFSCDM( logf("%4d HttpSource::HttpSource()\n", __LINE__); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::HttpSource
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HttpSource::HttpSource(          // Constructor
     const char*       uri)         // For this URI
:  DataSource()
,  connect(), reqProps(),  rspProps()
{
   IFSCDM( logf("%4d HttpSource::HttpSource(%s)\n", __LINE__, uri); )

   open(uri);
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::clone
//
// Purpose-
//       Clone this DataSource.
//
//----------------------------------------------------------------------------
DataSource*                         // Resultant DataSource
   HttpSource::clone(               // Clone this DataSource
     const char*       name) const  // With this (relative) name
{
   IFSCDM( logf("%4d HttpSource::clone(%s)\n", __LINE__, name); )
   ELSCDM( (void)name; )            // Parameter unused without SCDM

   return NULL; // TODO: NOT CODED YET
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::close
//
// Purpose-
//       Close the URL.
//
//----------------------------------------------------------------------------
void
   HttpSource::close( void )        // Close the URL
{
   IFSCDM( logf("%4d HttpSource::close()\n", __LINE__); )

   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::open
//
// Purpose-
//       Load given URL.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   HttpSource::open(                // Load an HTTP URI
     const char*       uri)         // Using this URI
{
   int                 respCode= 404; // Response code

   const char*         C;           // Working char*
   Common&             common= *Common::get(); // Get Common
   char                chunk[CHUNK_SIZE]; // Data chunk
   Properties::Iterator
                       ci;          // Properties const_iterator
   int                 L;           // The current data length
   int                 size;        // Chunk size
   Socket*             socket;      // Working Socket*
   TextBuffer          text;        // A text string

   int                 i;
   int                 rc;

   IFSCDM( logf("%4d HttpSource::open(%s)\n", __LINE__, uri); )
   close();                         // Delete any existing data

   //-------------------------------------------------------------------------
   // Verify the URI
   //-------------------------------------------------------------------------
   rc= verify(uri);
   if( rc != 0 )
     return rc;

   Url url(uri);                    // Set the associated URL
   connect.setUrl(uri);             // Reset the name
   this->name= uri;

   //-------------------------------------------------------------------------
   // Send the URL request
   //-------------------------------------------------------------------------
   socket= getSocket(connect);      // Make the connection
   if( socket == NULL )
     return (-1);

   // Send "GET" request
   text.reset();
   text.put("GET /");
   text.put(url.getPath());
   text.put(" HTTP/1.1\r\n");

   text.put("Host: ");
   text.put(url.getAuthority());
   text.put("\r\n");

   if( reqProps.getProperty("User-Agent") == NULL && TRUE )
   {
     text.put("User-Agent: ");
     text.put(common.wilbur);
     text.put("/");
     text.put(common.global->VERSION_ID);
     text.put("/BRINGUP");
     text.put(" {frank@eskesystems.com, machine learning experiment}\r\n");
   }

   for(ci= reqProps.begin(); ci != reqProps.end(); ci++)
   {
     if( ci->first != "Host" )
     {
       text.put(ci->first.c_str(), ci->first.size());
       text.put(": ");
       text.put(ci->second);
       text.put("\r\n");
     }
   }

   text.put("\r\n");
   rc= Diagnostic::send(*socket, text.toChar(), text.size());
   if( rc != text.size() )
   {
     socket= getSocket(connect);    // Attempt reconnection
     if( socket == NULL )
       return (-1);

     rc= Diagnostic::send(*socket, text.toChar(), text.size());
     if( rc != text.size() )
       return (-1);
   }

   // Collect response code
   rc= Diagnostic::recvLine(*socket, chunk, sizeof(chunk), 3000);
   if( rc != 0 )
   {
     logf("%4d HttpSource reconnect\n", __LINE__);
     if( !connect.isConnected() )
     {
       socket= getSocket(connect);  // Attempt reconnection
       if( socket == NULL )
         return (-1);

       // RESEND the request
       rc= Diagnostic::send(*socket, text.toChar(), text.size());
       if( rc != text.size() )
         return (-1);
     }

     // RETRY the response code receive
     rc= Diagnostic::recvLine(*socket, chunk, sizeof(chunk), 3000);
     if( rc != 0 )
       return (-1);
   }

   #if defined(HCDM) && FALSE
     debugf("%4d %s Response(%s)\n", __LINE__, __FILE__, chunk);
   #endif
   if( memcmp(chunk, "HTTP/1.1 ", 9) != 0 )
     return 505;                    // Server error

   C= chunk+9;
   respCode= parseDec(C);
// if( respCode < 200 || respCode >= 400 )
//   return respCode;

   //-------------------------------------------------------------------------
   // Collect response properties
   rspProps.reset();
   for(;;)
   {
     rc= Diagnostic::recvLine(*socket, chunk, sizeof(chunk));
     if( rc != 0 )
     {
       socket= getSocket(connect);  // Attempt reconnection
       if( socket == NULL )
         return (-1);

       rc= Diagnostic::recvLine(*socket, chunk, sizeof(chunk));
       if( rc != 0 )
         return (-1);
     }

     char* ptrN= chunk;
     if( *ptrN == '\0' )
       break;

     while( isspace(*ptrN) )
       ptrN++;

     char* ptrT= ptrN + 1;
     while( *ptrT != ':' && *ptrT != '\0' )
       ptrT++;

     if( *ptrT != '\0' )
     {
       *ptrT= '\0';
       ptrT= strip(ptrT+1);
     }

     rspProps.setProperty(ptrN, ptrT);
   }

   //-------------------------------------------------------------------------
   // Collect the text
   unsigned contentLength= 0xffffffff;
   C= rspProps.getProperty("Content-Length");
   if( C != NULL )
     contentLength= parseDec(C);

   text.reset();
   C= rspProps.getProperty("Transfer-Encoding");
   if( C == NULL )
     C= "";
   if( stricmp(C, "chunked") == 0 )
   {
//   rspProps.delProperty("Transfer-Encoding");
     for(;;)
     {
       // Read chunk size
       for(;;)
       {
         L= Diagnostic::recv(*socket, chunk, 1);
         if( L != 1 || isspace(chunk[0]) )
           break;
       }

       if( L != 1 )
         return (-1);

       if( Diagnostic::recvLine(*socket, chunk+1, sizeof(chunk)-1) != 0 )
         return (-1);

       size= 0;
       for(i= 0; chunk[i] != '\0'; i++)
       {
         if( chunk[i] >= '0' && chunk[i] <= '9' )
         {
           size <<= 4;
           size += (chunk[i] - '0');
         }
         else if( chunk[i] >= 'a' && chunk[i] <= 'f' )
         {
           size <<= 4;
           size += (10 + chunk[i] - 'a');
         }
         else if( chunk[i] >= 'A' && chunk[i] <= 'F' )
         {
           size <<= 4;
           size += (10 + chunk[i] - 'A');
         }
         else
         {
           errorf("%4d %s invalid chunk size(%s)\n",
                  __LINE__, __FILE__, chunk);

           return (-1);
         }
       }

       if( size == 0 )
         break;

       while( size > 0 )
       {
         int M= sizeof(chunk);
         if( M > size )
           M= size;

         L= Diagnostic::recv(*socket, chunk, M);
         if( L == 0 )
           return (-1);
         if( L > M )
         {
           errorf("%4d %s SHOULD NOT OCCUR\n", __LINE__, __FILE__);
           L= M;
         }

         text.put(chunk, L);
         size -= L;
       }
     }

     // Footer
     for(;;)
     {
       rc= Diagnostic::recvLine(*socket, chunk, sizeof(chunk));
       if( rc != 0 )
         return (-1);

       if( chunk[0] == '\0' )
         break;

       IFHCDM( debugf("%4d %s Footer(%s)\n", __LINE__, __FILE__, chunk); )
     }
   }
   else
   {
     Interval interval;
     for(;;)
     {
////// logf("%4d HCDM\n", __LINE__);
       L= Diagnostic::recv(*socket, chunk, sizeof(chunk), Socket::MO_NONBLOCK);
       if( L <= 0 )
       {
         Socket::SocketEC ec= socket->getSocketEC();
         if( ec == Software::EC_WOULDBLOCK || ec == Software::EC_AGAIN )
         {
           if( interval.stop() > 0.1 )
             break;
           Thread::sleep(0.1);
           continue;
         }
         break;
       }

       interval.start();
       if( size_t(L) > sizeof(chunk) )
       {
         errorf("%4d %s SHOULD NOT OCCUR\n", __LINE__, __FILE__);
         L= sizeof(chunk);
       }

       text.put(chunk, L);
       if( (unsigned)text.size() >= contentLength )
         break;
     }
   }

   //-------------------------------------------------------------------------
   // Copy the text into the HttpSource object
   //-------------------------------------------------------------------------
   length= text.size();
   if( length > 0 && (length+4) > length )
   {
     origin= (unsigned char*)Unconditional::malloc(length+4);
     memcpy(origin, text.toChar(), length);
     memset(origin+length, 0, 4);

     setWidth();
   }

   return respCode;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::verify
//
// Purpose-
//       Verify a URI..
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   HttpSource::verify(              // Verify an HTTP URI
     const char*       uri)         // Using this URI
{
   IFSCDM( logf("%4d HttpSource::verify(%s)\n", __LINE__, uri); )

   Url url(uri);                    // Set the associated URL
   if( url.getProtocol() != "http" )// Protocol "http" required
   {
     IFHCDM( debugf("%4d %s Protocol(%s), Protocol(http) required\n",
                    __LINE__, __FILE__, url.getProtocol().c_str()); )
     return (-2);
   }

   if( url.getUserInfo() != "" )    // UserInfo is forbidden
   {
     IFHCDM( debugf("%4d %s UserInfo(%s) forbidden\n", __LINE__, __FILE__,
                    url.getUserInfo().c_str()); )
     return (-3);
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSource::loadMetaProperties
//
// Purpose-
//       Load the <meta http_equiv=name content=value> properties
//
//----------------------------------------------------------------------------
Properties&                        // The meta Properties
   HttpSource::loadMetaProperties( // Load the meta Properties
     Properties&       properties) // Resultant Properties
{
   MetaVisitor         visitor(properties); // Our internal visitor
   DataSource          source(*this); // Duplicate this DataSource
   HtmlParser          parser;

   int                 rc;

   rc= parser.parse(source);
   #if FALSE
     debugf("%d= HTMLparser.parse(%s)\n", rc, fileName);
     parser.debug();
   #endif

   if( rc == 0 )
     parser.getRoot()->visit(visitor);

   return properties;
}

