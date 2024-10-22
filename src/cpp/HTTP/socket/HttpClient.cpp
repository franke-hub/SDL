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
//       HttpClient.cpp
//
// Purpose-
//       Curl-based HTTP client
//
// Last change date-
//       2024/10/08
//
// Implementation notes-
//       Derived from ~/src/cpp/Brian/Curl.cpp 2024/10/07
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string

#include <curl/curl.h>              // For CURL subroutines

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Socket.h>             // For pub::Socket::gethostname
#include <pub/utility.h>            // For pub::utility::to_string

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub objects
using namespace PUB::debugging;     // For debugging subroutines
using PUB::utility::to_string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

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
//       Curler
//
// Purpose-
//       CURL interface wrapper.
//
//----------------------------------------------------------------------------
class Curler {
//----------------------------------------------------------------------------
// Attributes
protected:
CURL*                  handle= nullptr; // The CURL handle
char                   error_buffer[CURL_ERROR_SIZE+8]; // Error message buffer

//-------------------------------------------------------------------------
public:
   Curler( void )                   // Constructor
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

   ~Curler( void )                  // Destructor
{
   if( handle ) {
     curl_easy_cleanup(handle);
     handle= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Curler::curl
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
   if( HCDM && false ) debugh("Curler::curl(%s)\n", url);

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
}; // class Curler

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainine code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Verify parameters
   std::string url= to_string("%s:8080", Socket::gethostname().c_str());

   if( argc == 2 )                  // If URL specified
     url= argv[1];

   //-------------------------------------------------------------------------
   // Read and display the web page
   Curler curler;                   // The CURL interface wrapper
   std::string output= curler.curl(url.c_str());
   debugf("%s\n", output.c_str());

   return 0;
}
