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
//       2024/10/07
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard
#include <string>                   // For std::string

#include <curl/curl.h>              // For CURL subroutines

#include <pub/Clock.h>              // For pub::Clock
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/String.h>             // For pub::String
#include <pub/Thread.h>             // For pub::Thread::sleep
#include <pub/utility.h>            // For pub::utility::to_string

#include "Command.h"                // For class Command
#include "Counter.h"                // For DEBUGGING object Counter
#include "Service.h"                // For class Service
#include "Curl.h"                   // For Curl_service, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub objects
using namespace PUB::debugging;     // For debugging subroutines
using PUB::Clock;
using PUB::Thread;
using PUB::utility::to_string;

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

#ifndef INCLUDE_SERVICE_TEST_COMMANDS // Build service test commands?
#undef  INCLUDE_SERVICE_TEST_COMMANDS
#define INCLUDE_SERVICE_TEST_COMMANDS
#endif

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
     debugh("curl_debug(%p,%d:%s,%p,'%zu',%p)\n", handle, type, type_name
           , text, size, unused);

   if( VERBOSE > 1 ) {
     if( type != CURLINFO_SSL_DATA_IN && type != CURLINFO_SSL_DATA_OUT ) {
       while( size > 0 && (text[size-1] == '\n' || text[size-1] == '\r') )
         --size;

       std::string mess(text, size);
       debugh("%s: '%s'\n", type_name, mess.c_str());
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
     debugh("curl_response(%p,'%zu','%zu',%p)\n", text, chunk, size, unused);

   if( size ) {                     // If data received
     std::string part(text, size);
     response += part;
   }

   return size;
}
} // extern "C"

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
//----------------------------------------------------------------------------
// Attributes
protected:
CURL*                  handle= nullptr; // The CURL handle
char                   error_buffer[CURL_ERROR_SIZE+8]; // Error message buffer

//-------------------------------------------------------------------------
public:
   Command_curl( void ) : Command("curl") // Constructor
{
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

   ~Command_curl( void )            // Destructor
{
   if( handle ) {
     curl_easy_cleanup(handle);
     handle= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_curl::curl
//
// Purpose-
//       Fetch a URL
//
// Implementation notes-
//       This method runs serially without any *other* delay.
//
//----------------------------------------------------------------------------
std::string                         // The web page
   curl(                            // Fetch the associated URL
     const char*       url)         // And this URL
{
   static std::mutex mutex;         // (Protects static CURL interface)
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( HCDM && false ) debugh("Command_curl::curl(%s)\n", url);

   response= "";                    // Reset the accumulator string
   if( handle ) {                   // If construction succeeded
     // Fetch the URL
     curl_easy_setopt(handle, CURLOPT_URL, url);
     CURLcode cc= curl_easy_perform(handle);

     // If error, return error string
     if( cc )
       response= to_string("ERROR: %d= curl(%s) %s\n", cc, url, error_buffer);
   }

   return response;                 // Return (copying the response string)
}

//----------------------------------------------------------------------------
//
// Method-
//       work
//
// Purpose-
//       Run the curl Command, invoking the curl method
//
//----------------------------------------------------------------------------
virtual Command::resultant          // Resultant
   work(                            // Handle Command
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Verify parameters
   const char* url= "localhost:6419";

   if( argc < 2 )                   // If no URL specified
     fprintf(stderr, "URL parameter missing\n");
   else if( argc > 2 )              // If more than one URL specified
     fprintf(stderr, "Only one URL parameter can be specified\n");
   else
     url= argv[1];

   //-------------------------------------------------------------------------
   // Read and display the web page
   std::string output= curl(url);
   debugf("curl '%s':\n%s\n", url, output.c_str());

   return nullptr;
}
}; // class Command_curl
static Command_curl command_curl;

//============================================================================
//
// Class-
//       Curl_task
//
// Purpose-
//       Rate-limited web page fetch
//
//----------------------------------------------------------------------------
class Curl_task : public Task {     // Our work handler
double                 last= 0.0;   // Last request time

public:
//----------------------------------------------------------------------------
// Curl_task::Constructor/destructor
   Curl_task( void )                // Constructor
:  Task()
{  if( HCDM ) debugh("Curl_task!\n"); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~Curl_task()                     // Destructor
{  if( HCDM && false ) debugh("Curl_task~\n"); }

//----------------------------------------------------------------------------
//
// Method-
//       Curl_task::work
//
// Purpose-
//       Handle work (Curl_item)
//
//----------------------------------------------------------------------------
void
   work(                            // Handle
     Item*             _item)       // This Curl_item
{
   typedef Curl_service::Item       Curl_item;
   Curl_item* item= dynamic_cast<Curl_item*>(_item);
   if( item == nullptr )    {       // If invalid Item type
     _item->post(Item::CC_ERROR_FC);
     return;
   }

   if( HCDM ) debugh("Curl_task::work(%s)\n", item->request.c_str());

   // Rate limiter
   double now= Clock::now();        // The current time
   if( now - last < FETCH_INTERVAL ) {  // If too soon
     Thread::sleep(FETCH_INTERVAL - (now - last)); // Delay
     now= Clock::now();
   }

   item->response= command_curl.curl(item->request.c_str());
   item->post(Item::CC_NORMAL);

   last= now;
}
}; // class Curl_task
static Curl_task curl_task;

//============================================================================
//
// Method-
//       Curl_service::Curl_service
//       Curl_service::~Curl_service
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Curl_service::Curl_service()     // Constructor
:  Service("curl"), task(curl_task)
{  if( HCDM ) debugh("Curl_service!\n"); }

   Curl_service::~Curl_service()    // Destructor
{  if( HCDM && false ) debugh("Curl_service~\n"); }

//----------------------------------------------------------------------------
//
// Method-
//       Curl_service::async
//
// Purpose-
//       Asynchrounously display a web page [rate limited]
//
//----------------------------------------------------------------------------
void
   Curl_service::async(             // Asynchronously display the web page
     const char*       url)         // At this URL
{  if( HCDM ) debugh("Curl_service.async(%s)\n", url);

   typedef Curl_service::Item       Curl_item;

   class Curl_done : public pub::dispatch::Done {
   Counter             counter;     // DEBUGGING object counter

   public:
      Curl_done( void ) : Done() {}

   virtual void
      done(pub::dispatch::Item* _item)
   {
     if( HCDM || VERBOSE > 0 )
       debugh("Curl_done(%p).item(%p)\n", this, _item);
     Curl_item* item= (Curl_item*)_item;
     debugh("Curl_service.async(%s):\n%s\n"
           , item->request.c_str(), item->response.c_str());

     delete item;
     delete this;
   }
   }; // class CurlDone

   Curl_done* done= new Curl_done();
   Item* item= new Item(url, done);
   if( HCDM || VERBOSE > 0 )
     debugh("CURL: done(%p) item(%p)\n", done, item);

   task.enqueue(item);
}

//----------------------------------------------------------------------------
//
// Method-
//       Curl_service::curl
//
// Purpose-
//       Fetch a web page [rate limited]
//
//----------------------------------------------------------------------------
std::string                         // The web page
   Curl_service::curl(              // Get web page
     const char*       url)         // At this URL
{  if( HCDM ) debugh("Curl_service.curl(%s)\n", url);

   Item item(url);                  // Our work item
   task.enqueue(&item);             // Enqueue request
   item.wait();                     // Wait for completion

   return item.response;            // Return response string
}
static Curl_service curl_service;

//----------------------------------------------------------------------------
//
// Commands-
//       Curl_service tests
//
// Purpose-
//       Test the Curl_service
//
//----------------------------------------------------------------------------
#ifdef INCLUDE_SERVICE_TEST_COMMANDS
static class Curl_service_async : public Command {
public:
   Curl_service_async( void ) : Command("curlserv-async") {}

virtual resultant
   work(int argc, char* argv[])
{
   const char* url= "localhost:6419";
   if( argc == 2 )                  // If URL specified
     url= argv[1];
   curl_service.async(url);

   return nullptr;
}
} curl_service_async;

static class Curl_service_curl : public Command {
public:
   Curl_service_curl( void ) : Command("curlserv-curl") {}

virtual resultant
   work(int argc, char* argv[])
{
   const char* url= "localhost:6419";
   if( argc == 2 )                  // If URL specified
     url= argv[1];
   std::string output= curl_service.curl(url);
   debugh("curlserv-curl(%s):\n%s\n", url, output.c_str());

   return nullptr;
}
} curl_service_curl;

static class Curl_service_url : public Command {
public:
   Curl_service_url( void ) : Command("curlserv-url") {}

virtual resultant
   work(int argc, char* argv[])
{
   const char* url= "localhost:6419";
   if( argc == 2 )                  // If URL specified
     url= argv[1];

   Service* service= Service::locate("curl");
   Curl_service* curl_service= dynamic_cast<Curl_service*>(service);
   if( curl_service == nullptr ) {
     debugf("ERROR: Didn't find \"curl\" service\n");
   } else {
     Curl_service::Item item(url);
     curl_service->task.enqueue(&item);
     item.wait();
     debugh("curlserv-url(%s):\n%s\n", url, item.response.c_str());
   }

   return nullptr;
}
} curl_service_url;
#endif
