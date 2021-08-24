//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PostHttpServer.cpp
//
// Purpose-
//       PostHttpServer implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <com/Debug.h>
#include <com/istring.h>            // for memicmp
#include <com/Thread.h>

#include "Common.h"
#include "DateParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Properties.h"
#include "TextBuffer.h"

#include "HttpServer.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef BRINGUP
#undef  BRINGUP                     // If defined, BRINGUP Mode
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Class-
//       PostHttpServer
//
// Purpose-
//       Serve an HTTP POST request.
//
//----------------------------------------------------------------------------
class PostHttpServer : public HttpServer {  // Http POST request processor
//----------------------------------------------------------------------------
// PostHttpServer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PostHttpServer( void );         // Destructor
   PostHttpServer( void );          // Constructor

//----------------------------------------------------------------------------
// PostHttpServer::Methods
//----------------------------------------------------------------------------
public:
virtual void
   serve(                           // Handle HTTP request/response
     HttpRequest&      request,     // The HTTP request
     HttpResponse&     response);   // The HTTP response
}; // class PostHttpServer

//----------------------------------------------------------------------------
//
// Subroutine-
//       retrieveHexchar
//
// Purpose-
//       Retrieve a hexadecimal character from a string
//
//----------------------------------------------------------------------------
static int                          // The resultant value
   retrieveHexchar(                 // Extract keyword from input
     const char*&      C,           // The input text character
     int               result)      // The current resultant
{
   result <<= 4;
   if( *C >= '0' && *C <= '9' )
     result += (*C - '0');
   else if( *C >= 'a' && *C <= 'f' )
     result += 10 + (*C - 'a');
   else if( *C >= 'A' && *C <= 'F' )
     result += 10 + (*C - 'A');

   if( *C != '\0' )
     C++;

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       retrieveKeyword
//
// Purpose-
//       Retrieve the specified form name
//
//----------------------------------------------------------------------------
static void
   retrieveKeyword(                 // Extract keyword from input
     const char*       key,         // The keyword
     TextBuffer&       inp,         // The input data
     TextBuffer&       out)         // The keyword resultant
{
   out.reset();
   const char* C= inp.toChar();
   const int L= strlen(key);

   //-------------------------------------------------------------------------
   // Locate the keyword
   if( memcmp(C, key, L) != 0
       || *(C + L) != '=' )
   {
     for(;;)
     {
       C= strchr(C, '&');
       if( C == NULL )
         return;

       C++;
       if( memcmp(C, key, L) == 0
           && *(C + L) == '=' )
         break;
     }
   }

   int P= ' ';
   C += L + 1;
   while( *C != '&' && *C != '\0' )
   {
     if( *C == '+' )
     {
       if( P != ' ' )
         out.put(' ');
       P= ' ';

       while( *C == '+' )
         C++;
     }

     if( *C == '%' )
     {
       C++;
       int percent= 0;
       percent= retrieveHexchar(C, percent);
       percent= retrieveHexchar(C, percent);
       if( percent == ' ' || percent == '\t'
           || percent == '\r' || percent == '\n' )
       {
         if( P != ' ' )
           out.put(' ');
         P= ' ';
       }
       else
       {
         P= percent;
         if( P != 0 )
           out.put(P);
       }
       continue;
     }

     P= *C;
     out.put(P);
     C++;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PostHttpServer::~PostHttpServer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PostHttpServer::~PostHttpServer( void )
{
   IFHCDM( traceh("PostHttpServer(%p)::~PostHttpServer()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       PostHttpServer::PostHttpServer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PostHttpServer::PostHttpServer( void ) // Constructor
:  HttpServer()
{
   IFHCDM( traceh("PostHttpServer(%p)::PostHttpServer()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       PostHttpServer::serve
//
// Purpose-
//       Handle HTTP request/response
//
//----------------------------------------------------------------------------
void
   PostHttpServer::serve(           // Handle HTTP request/response
     HttpRequest&      Q,           // The HTTP request
     HttpResponse&     S)           // The HTTP response
{
   char                temp[4096];  // Working buffer

   traceh("PostHttpServer(%p)::serve(%p,%p)\n", this, &Q, &S);

   //-------------------------------------------------------------------------
   // BRINGUP: Display the properties
   #if defined(BRINGUP) && TRUE
     traceh("\n");
     traceh("Properties:\n");
     traceh("request.httpID: '%s'\n", Q.getHttpID().c_str());
     traceh("request.opCode: '%s'\n", Q.getOpCode().c_str());
     traceh("request.opPath: '%s'\n", Q.getOpPath().c_str());
     traceh("request.major: '%d'\n",  Q.getMajor());
     traceh("request.minor: '%d'\n",  Q.getMinor());

     Properties::Iterator it;
     for(it= Q.begin(); it != Q.end(); it++)
       traceh("\'%s\' = \'%s\'\n", it->first.c_str(), it->second.c_str());
     traceh("\n");
   #endif

   //-------------------------------------------------------------------------
   // Parse the request
   std::string httpID= Q.getHttpID();
   std::string opCode= Q.getOpCode();
   std::string opPath= Q.getOpPath();
   if( opCode == "POST" )
   {
     TextBuffer text;
     Q.getText(text);

     S.generateCode(S.http202);
     S.generateServer();
     S.putln("Content-Type: text/html");
     S.generateDate();
     S.put("\r\n");
     S.put("POST response in progress.");

     TextBuffer buff;
     retrieveKeyword("generic", text, buff);
     if( memicmp(buff.toChar(), "shutdown", 8) == 0 )
     {
       S.put("<br>Shutdown accepted");
       Common::get()->shutdown();
       return;
     }

     //-----------------------------------------------------------------------
     // BRINGUP - Respond with the properties
     #if defined(BRINGUP) && TRUE
       S.put("<br>httpID: '"); S.put(httpID); S.put("'");
       S.put("<br>opCode: '"); S.put(opCode); S.put("'");
       S.put("<br>opPath: '"); S.put(opPath); S.put("'");
       retrieveKeyword("userid", text, buff);
       S.put("<br>userid: '"); S.put(buff.toString()); S.put("'");
       retrieveKeyword("passwd", text, buff);
       S.put("<br>passwd: '"); S.put(buff.toString()); S.put("'");
       retrieveKeyword("generic", text, buff);
       S.put("<br>generic: '"); S.put(buff.toString()); S.put("'");
       retrieveKeyword("context", text, buff);
       S.put("<br>context: '"); S.put(buff.toString()); S.put("'");
     #endif

     for(int i= 0; i < 20; i++)
     {
       int delay= Random::standard.modulus(5000);
       Thread::sleep((double)delay/1000.0);
       sprintf(temp, "<br>SLOWDOWN [%3d] delay(%d)\r\n", i, delay);
       S.put(temp, strlen(temp));
       S.flush();
     }

     S.put("<br>POST complete\r\n\r\n");
   }
   else if( opCode == "GET" || opCode == "HEAD" )
     HttpServer::serve(Q, S);
   else
     S.generateError(S.http405);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DLL_make
//
// Purpose-
//       Allocate and initialize a PostHttpServer object
//
//----------------------------------------------------------------------------
extern "C" Interface* DLL_make( void ); // (Not very far) Forward reference
extern "C"
Interface*                          // Our Interface
   DLL_make( void )                 // Get Interface
{
   Interface* result= new PostHttpServer();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DLL_take
//
// Purpose-
//       Finalize and release storage for an Interface Object.
//
//----------------------------------------------------------------------------
extern "C" void DLL_take(Interface*); // (Not very far) Forward reference
extern "C"
void
   DLL_take(                        // Recycle
     Interface*        object)      // This Interface Object
{
   delete object;                   // Delete the Object
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       my_init
//
// Purpose-
//       _OS_BSD DLL initialization function.
//
//----------------------------------------------------------------------------
#if defined(_OS_BSD) && defined(BRINGUP) && defined(HCDM)
__attribute__((constructor))
static void my_init()
{
   IFHCDM( traceh("Inside my_init()\n"); )
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       my_fini
//
// Purpose-
//       _OS_BSD DLL termination function.
//
//----------------------------------------------------------------------------
#if defined(_OS_BSD) && defined(BRINGUP) && defined(HCDM)
__attribute__((destructor))
static void my_fini()
{
   IFHCDM( traceh("Inside my_fini()\n"); )
}
#endif

