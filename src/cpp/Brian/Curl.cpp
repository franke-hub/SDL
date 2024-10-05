//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Curl.cpp
//
// Purpose-
//       Curl Commands and Services
//
// Last change date-
//       2024/10/05
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard
#include <string>                   // For std::string

#include <curl/curl.h>              // For CURL subroutines

#include "pub/Clock.h"              // For pub::Clock
#include "pub/Debug.h"              // For namespace pub::debugging
#include "pub/String.h"             // For pub::String
#include "pub/Thread.h"             // For pub::Thread::sleep

#include "Command.h"                // For class Command
#include "Counter.h"                // For DEBUGGING object Counter
#include "Service.h"                // For class Service
#include "Curl.h"                   // For Service_fetchURL, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub objects
using namespace PUB::debugging;     // For debugging subroutines
using PUB::Clock;
using PUB::Thread;

typedef PUB::dispatch::Done         Done; // Import pub::dispatch Objects
typedef PUB::dispatch::Item         Item;
typedef PUB::dispatch::Wait         Wait;
typedef PUB::dispatch::Task         Task;

typedef PUB::dispatch::LambdaDone   LambdaDone;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

static constexpr double FETCH_INTERVAL= 30.0; // Minimum request interval

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::string     response;    // The response accumulator string

//----------------------------------------------------------------------------
//
// Subroutine-
//       curl_debug
//
// Purpose-
//       Handle CURL debugging information (by ignoring it)
//
//----------------------------------------------------------------------------
extern "C" {
static int                          // Return code, 0 expected
   curl_debug(                      // Handle CURL debugging information
     CURL*             handle,      // The CURL handle
     curl_infotype     type,        // Debugging message type
     char*             text,        // Message address
     size_t            size,        // Message length
     void*             unused)      // User data (UNUSED)
{
static const char* type_name_list[8]=
{  {"TEXT"}
,  {"HEADER_IN"}
,  {"HEADER_OUT"}
,  {"DATA_IN"}
,  {"DATA_OUT"}
,  {"SSL_DATA_IN"}
,  {"SSL_DATA_OUT"}
,  {"END"}
};

   const char* type_name= "INVALID TYPE";
   if( type < 8 )
     type_name= type_name_list[type];

   if( HCDM )
     debugf("curl_debug(%p,%d:%s,%p,'%zu',%p)\n", handle, type, type_name
           , text, size, unused);

   if( VERBOSE > 1 ) {
     if( type != CURLINFO_SSL_DATA_IN && type != CURLINFO_SSL_DATA_OUT ) {
       while( size > 0 && (text[size-1] == '\n' || text[size-1] == '\r') )
         --size;

       std::string mess(text, size);
       debugf("%s: '%s'\n", type_name, mess.c_str());
     }
   }

   return 0;
}
} // extern "C"

//----------------------------------------------------------------------------
//
// Subroutine-
//       curl_response
//
// Purpose-
//       Response accumulator
//
//----------------------------------------------------------------------------
extern "C" {
static size_t                       // Number of bytes accepted
   curl_response(                   // CURL response accumulator
     char*             text,        // Text address
     size_t            chunk,       // Chunk count (always 1)
     size_t            size,        // Text length
     void*             unused)      // User data (UNUSED)
{  if( HCDM )
     debugf("curl_response(%p,'%zu','%zu',%p)\n", text, chunk, size, unused);

   if( size ) {                     // If data received
     std::string part(text, size);
     response += part;
   }

   return size;
}
} // extern "C"

//----------------------------------------------------------------------------
//
// Class-
//       FetchURL_task
//
// Purpose-
//       Rate-limited web page fetch
//
//----------------------------------------------------------------------------
class FetchURL_task : public Task { // Our work handler
//----------------------------------------------------------------------------
// class FetchURL_task::Request
public:
class Request : public Item {       // Our work Item
public:
std::string            response;    // The web page content
std::string            url;         // The web page to fetch

pub::dispatch::Wait    _wait;       // Our Done object

   Request(                         // Constructor
     std::string       url)         // The web page URL
:  Item(&_wait), url(url) {}

void
   wait( void )                     // Wait for completion
{  _wait.wait(); }
}; // class FetchURL_task::Request

//----------------------------------------------------------------------------
// Attributes
protected:
CURL*                  handle= nullptr; // The CURL handle
char                   error_buffer[CURL_ERROR_SIZE+8]; // Error message buffer

double                 last= 0.0;   // Last request time

public:
//----------------------------------------------------------------------------
// FetchURL_task::Constructor/destructor
   FetchURL_task( void )            // Constructor
:  Task()
{  if( HCDM ) debugf("FetchURL_task!\n");

   handle= curl_easy_init();
   if( handle == nullptr )
     return;

   if( HCDM || true ) {
     curl_easy_setopt(handle, CURLOPT_VERBOSE, long(true));
     curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION, curl_debug);
   }

   // Initialize CURL error message buffer
   curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer);

   // Use the default user agent
// curl_easy_setopt(handle, CURLOPT_USERAGENT, USER_AGENT);

   // Use our response accumulator
   curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_response);

   // Set (UNUSED) curl_response parameter
   curl_easy_setopt(handle, CURLOPT_WRITEDATA, nullptr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~FetchURL_task()                 // Destructor
{  if( HCDM && false ) debugf("FetchURL_task~\n");

   if( handle ) {
     curl_easy_cleanup(handle);
     handle= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       FetchURL_task::fetch
//
// Purpose-
//       Fetch a URL
//
//----------------------------------------------------------------------------
std::string                         // The web page string
   fetch(                           // Fetch
     const char*       url)         // This URL
{
   if( handle == nullptr )          // If construction failed
     return "";                     // All fetches fail

   static std::mutex mutex;         // (Protects static CURL interface)
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( HCDM ) debugf("FetchURL_task::fetch(%s)\n", url);

   response= "";                    // (Re)initialize the accumulator string

   // Fetch the URL
   curl_easy_setopt(handle, CURLOPT_URL, url);
   CURLcode cc= curl_easy_perform(handle);

   // If error, write message
   if( cc )
     fprintf(stderr, "ERROR: %d= fetchURL(%s) %s\n", cc, url, error_buffer);

   return response;                 // Return (copying the response string)
}

//----------------------------------------------------------------------------
//
// Method-
//       FetchURL_task::work
//
// Purpose-
//       Request
//
//----------------------------------------------------------------------------
void
   work(                            // Handle
     Item*             item)        // This Sevice_fetchURL::Request
{
   Request* request= dynamic_cast<Request*>(item);
   if( request == nullptr ) {       // If invalid Item type
     item->post(Item::CC_ERROR_FC);
     return;
   }

   if( HCDM ) debugf("FetchURL_task::work(%s)\n", request->url.c_str());

   // Rate limiter
   double now= Clock::now();        // The current time
   if( now - last < FETCH_INTERVAL ) {  // If too soon
     Thread::sleep(FETCH_INTERVAL - (now - last)); // Delay
     now= Clock::now();
   }
   last= now;

   request->response= fetch(request->url.c_str());
   item->post(Item::CC_NORMAL);
}
}; // class FetchURL_task
static FetchURL_task fetchURL_task;

//============================================================================
//
// Method-
//       Service_fetchURL::Service_fetchURL
//       Service_fetchURL::~Service_fetchURL
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Service_fetchURL::Service_fetchURL() // Constructor
   : Service("fetchURL"), task(fetchURL_task)
{  if( HCDM ) debugf("Service_fetchURL!\n"); }

   Service_fetchURL::~Service_fetchURL() // Destructor
{  if( HCDM && false ) debugf("Service_fetchURL~\n"); }

//----------------------------------------------------------------------------
//
// Method-
//       Service_fetchURL::curl
//
// Purpose-
//       Asynchrounously display a web page
//
//----------------------------------------------------------------------------
void
   Service_fetchURL::curl(          // Display the web page
     const char*       url)         // At this URL
{  if( HCDM ) debugf("Service_fetchURL.curl(%s)\n", url);

   typedef FetchURL_task::Request   Request;

   class Curl_done : public Done {
   Counter             counter;     // DEBUGGING object counter

   public:
      Curl_done( void ) : Done() {}

   virtual void
      done(Item* item)
   {
     if( HCDM || VERBOSE > 0 )
       debugf("Curl_done(%p).item(%p)\n", this, item);
     Request* request= (Request*)item;
     debugf("URL(%s):\n%s\n", request->url.c_str(), request->response.c_str());

     delete item;
     delete this;
   }
   }; // class CurlDone

   Curl_done* done= new Curl_done();
   Request* item= new Request(url);
   if( HCDM || VERBOSE > 0 )
     debugf("CURL: done(%p) item(%p)\n", done, item);

   item->done= done;
   task.enqueue(item);
}

//----------------------------------------------------------------------------
//
// Method-
//       Service_fetchURL::fetch
//
// Purpose-
//       Fetch a web page
//
//----------------------------------------------------------------------------
std::string                         // The web page
   Service_fetchURL::fetch(         // Get web page
     const char*       url)         // At this URL
{  if( HCDM ) debugf("Service_fetchURL.fetch(%s)\n", url);

   FetchURL_task::Request item(url); // Our work item
   task.enqueue(&item);             // Enqueue request
   item.wait();                     // Wait for completion

   return item.response;            // Return response string
}
static Service_fetchURL fetchURL_service;

//============================================================================
//
// Class-
//       Command_curl
//
// Purpose-
//       Read a and display a web page
//
//----------------------------------------------------------------------------
class Command_curl : public Command {
//-------------------------------------------------------------------------
public:
   Command_curl() : Command("curl")
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Command_curl::fetchURL
//
// Purpose-
//       Fetch a URL
//
//----------------------------------------------------------------------------
void
   fetchURL(                        // Fetch the associated URL
     const char*       url)         // And this URL
{
   if( true ) {
     std::string output= fetchURL_service.fetch(url); // Synchronous display
     debugf("\ncurl: %s\n%s\n", url, output.c_str());
   } else {
     fetchURL_service.curl(url);    // Asynchonous display
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       work
//
// Purpose-
//       Run the Command
//
//----------------------------------------------------------------------------
virtual Command::resultant          // Resultant
   work(                            // Handle Command
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Verify parameters
   const char* web_page= "localhost:6419";

   if( argc < 2 )                   // If no URL specified
     fprintf(stderr, "URL parameter missing\n");
   else if( argc > 2 )              // If more than one URL specified
     fprintf(stderr, "Only one URL parameter can be specified\n");
   else
     web_page= argv[1];

   //-------------------------------------------------------------------------
   // Read and display the web page
   fetchURL(web_page);

   return nullptr;
}
}; // class Command_curl
static Command_curl command_curl;
