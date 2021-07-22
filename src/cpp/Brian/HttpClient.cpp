//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2021 Frank Eskesen.
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
//       Curl-based HTTP client.
//
// Last change date-
//       2021/07/15
//
// Prerequisites-
//       cURL: http://curl.haxx.se/ (Also google "cURL")
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // Use ... arguments
#include <ctype.h>                  // For isdigit
#include <netdb.h>                  // For gethostbyname
#include <stdint.h>                 // For int64_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>                   // For asctime
#include <unistd.h>                 // For sleep

#include <arpa/inet.h>              // For inet_addr
#include <curl/curl.h>
#include <sys/timeb.h>              // For struct timeb, ftime
#include <sys/types.h>              // For int64_t

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
#define DELAY_IP 1200               // Delay interval, probable internet down
#define DELAY_NG 3600               // Delay interval, unsuccessful access
#define DELAY_OK  600               // Delay interval, successful access

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>            // For IFHCDM()
#include <com/define.h>             // For _ATTRIBUTE_PRINTF

//----------------------------------------------------------------------------
// Printf functions, forward reference
//----------------------------------------------------------------------------
static void logger(const char*, ...) _ATTRIBUTE_PRINTF(1, 2);
static void shouldNotOccur(const char*, ...) _ATTRIBUTE_PRINTF(1, 2);

//----------------------------------------------------------------------------
// Configuration controls
//----------------------------------------------------------------------------
static char*           INP_FILE_NAME= NULL; // Initialized in InitTerm object
static char*           LOG_FILE_NAME= NULL; // (Updated by replace())

static char*           CTL_protocol= NULL;
static char*           CTL_use= NULL;
static char*           CTL_web= NULL;
static char*           CTL_server= NULL;
static char*           CTL_username= NULL;  // Initialized in readControl()
static char*           CTL_password= NULL;
static char*           CTL_target= NULL;

// tic const char*     USER_AGENT= "eskesystems.com UpdateDNS 1.0";

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
FILE*                  stdlog= NULL; // The log file
static char            error_buffer[CURL_ERROR_SIZE+8]; // Error message buffer

static char            last_iptext[32]; // Last known IP address string
static char            this_iptext[32]; // Current IP address string

static unsigned        respSize;    // Number of response bytes read
static char            response[65536]; // Response accumulator

static int             sw_output;   // TRUE iff -out parameter used
static int             sw_verify;   // TRUE iff -v parameter used

//----------------------------------------------------------------------------
// Internal constants
//----------------------------------------------------------------------------
struct NV                           // Name/Value pair for parameter updates
{  const char*         N;           // Parameter name
   char**              V;           // -> Parameter value
}; // struct NV

static NV              CTL_nv[]=    // Name/Value pair table
{   {"protocol", &CTL_protocol}
,   {"use",      &CTL_use}
,   {"web",      &CTL_web}
,   {"server",   &CTL_server}
,   {"login",    &CTL_username}
,   {"password", &CTL_password}
,   {NULL,       NULL}
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       vlogger
//
// Purpose-
//       Write a message onto stdlog
//
//----------------------------------------------------------------------------
static void
   vlogger(                         // Write logger message
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // The remaining arguments
{
   if( stdlog != NULL )
   {
     double            tod;        // Resultant time (double)
     timeval tv;
     gettimeofday(&tv, nullptr);
     tod= (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

     fprintf(stdlog, "%14.3f ", tod); // Write time of day in log
     vfprintf(stdlog, fmt, argptr);
     fflush(stdlog);
   } else {
     vfprintf(stderr, fmt, argptr); // (stderr omits time of day)
     fflush(stderr);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       logger
//
// Purpose-
//       Write a message onto stdlog
//
//----------------------------------------------------------------------------
static void
   logger(                          // Write logger message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogger(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       accr
//
// Purpose-
//       Response accumulator
//
//----------------------------------------------------------------------------
extern "C" {
static int                          // Number of bytes written
   accr(                            // CURL response accumulator
     char*             addr,        // Data address
     size_t            length,      // Data length
     size_t            chunk,       // Number of bytes in each element
     void*             handle)      // User data (UNUSED)
{
   (void)handle;                    // (Indicate unused)

   IFHCDM( logger("accr(%p,%u,%u,%p) %u\n", addr, length, chunk, handle, respSize); )
   if( length >= sizeof(response) || chunk >= sizeof(response) )
     return 0;                      // ERROR: RESPONSE TOO BIG

   length *= chunk;                 // Number of bytes received
   if( (respSize + length) >= sizeof(response) )
     return 0;                      // ERROR: RESPONSE TOO BIG

   memcpy(response+respSize, addr, length); // Copy in the new chunk
   respSize += length;              // Update the length
   response[respSize]= '\0';        // Set trailing delimiter

   IFHCDM( logger("%d response(%s)\n", respSize, response); )

   return length;
}
} // extern "C"

//----------------------------------------------------------------------------
//
// Subroutine-
//       replace
//
// Purpose-
//       Replace an existing (allocated) string, either by duplicating its
//       replacement or, if replace == NULL, by freeing it.
//
//----------------------------------------------------------------------------
static char*                        // -> (Duplicated) replacement string
   replace(                         // Replace an allocated string
     char*             current,     // -> Current string
     const char*       replace)     // -> Replacement string
{
   if( current != NULL )
   {
     free(current);
     current= NULL;
   }

   if( replace != NULL )
   {
     current= strdup(replace);
     if( current == NULL )
       shouldNotOccur("Storage shortage");
   }

   return current;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       shouldNotOccur
//
// Purpose-
//       Write a message onto stdlog and stderr, then terminate
//
//----------------------------------------------------------------------------
static void
   shouldNotOccur(                  // Write logger message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogger(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   if( stdlog != NULL )             // If message written to log
   {
     fclose(stdlog);                // Duplicate message on stderr
     stdlog= NULL;

     va_start(argptr, fmt);         // Initialize va_ functions
     vlogger(fmt, argptr);
     va_end(argptr);                // Close va_ functions
   }

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Class-
//       AutoCURL
//
// Purpose-
//       Automatic CURL handle cleanup.
//
// Implementation notes-
//       Common CURL handle initialization is performed here.
//
//----------------------------------------------------------------------------
class AutoCURL {                    // Automatic CURL handle cleanup
protected:
CURL*                  handle;      // CURL handle

public:
inline
   ~AutoCURL( void )                // Destructor
{
   IFHCDM( logger("AutoCURL(%p)::~AutoCURL(%p)\n", this, handle); )

   if( handle != NULL )             // (Always true, but good practice)
   {
     curl_easy_cleanup(handle);
     handle= NULL;
   }
}

inline
   AutoCURL(                        // Constructor
     CURL*             handle)      // CURL handle
:  handle(handle)
{
   IFHCDM( logger("AutoCURL(%p)::AutoCURL(%p)\n", this, handle); )

   // Common initialization functions
   IFHCDM( curl_easy_setopt(handle, CURLOPT_VERBOSE, long(TRUE)); )
   curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer); // Error message buffer
   curl_easy_setopt(handle, CURLOPT_NOPROGRESS, long(TRUE)); // No progress meter
// curl_easy_setopt(handle, CURLOPT_USERAGENT, USER_AGENT); // Set user agent
   curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, accr); // Response accumulator
   curl_easy_setopt(handle, CURLOPT_WRITEDATA, long(0)); // UNUSED
}
}; // class AutoCURL

//----------------------------------------------------------------------------
//
// Class-
//       InitTerm
//
// Purpose-
//       Static initialization/termination.
//
//----------------------------------------------------------------------------
class InitTerm {                    // Automatic cleanup
protected:
   CURLcode            initCURL;    // CURL initialization code

public:
inline
   ~InitTerm( void )                // Destructor
{
   IFHCDM( logger("InitTerm(%p)::~InitTerm()\n", this); )

   // CURL global termination
   if( initCURL == 0 )
     curl_global_cleanup();

   // Release allocated strings
   INP_FILE_NAME= replace(INP_FILE_NAME, NULL);
   LOG_FILE_NAME= replace(LOG_FILE_NAME, NULL);

   CTL_protocol=  replace(CTL_protocol,  NULL);
   CTL_use=       replace(CTL_use,       NULL);
   CTL_web=       replace(CTL_web,       NULL);
   CTL_server=    replace(CTL_server,    NULL);
   CTL_username=  replace(CTL_username,  NULL);
   CTL_password=  replace(CTL_password,  NULL);
   CTL_target=    replace(CTL_target,    NULL);
}

inline
   InitTerm( void )                 // Constructor
:  initCURL(CURLcode(-1))
{
   IFHCDM( logger("InitTerm(%p)::InitTerm()\n", this); )

   // CURL global initialization
   initCURL= curl_global_init(CURL_GLOBAL_ALL);
   if( initCURL != 0 )
     shouldNotOccur("Internal error in curl_global_init\n");

   // Static char* initialization
   INP_FILE_NAME= strdup("./UpdateDNS.inp");
   LOG_FILE_NAME= strdup("./UpdateDNS.log");

   CTL_protocol=  strdup("dyndns2");
   CTL_use=       strdup("none");
   CTL_web=       strdup("http://myip.dnsdynamic.org");
   CTL_server=    strdup("https://www.dnsdynamic.org");
   if( INP_FILE_NAME == NULL
       || LOG_FILE_NAME == NULL
       || CTL_protocol  == NULL
       || CTL_use       == NULL
       || CTL_web       == NULL
       || CTL_server    == NULL )
     shouldNotOccur("Storage shortage");
}
}; // class InitTerm
static InitTerm        initTerm;    // Static initialization/termination

//----------------------------------------------------------------------------
//
// Subroutine-
//       fetchTOD
//
// Purpose-
//       Fetch date and time
//
//----------------------------------------------------------------------------
static const char*                  // Resultant
   fetchTOD(                        // Fetch the current date and time
     char*             result)      // Resultant
{
   time_t now= time(NULL);
   struct tm* tblock= localtime(&now) ;
   strcpy(result, asctime(tblock));
   int L= strlen(result) - 1;       // Length is never zero
   if( result[L] == '\n' )          // Remove trailing '\n' if present
     result[L]= '\0';

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fetchURL
//
// Purpose-
//       Fetch a URL
//
//----------------------------------------------------------------------------
static CURLcode                     // The response code
   fetchURL(                        // Fetch the associated URL
     CURL*             handle,      // For this handle
     const char*       url,         // And this URL
     const char*       dateTime)    // Date and time (for error message)
{
   IFHCDM( logger("fetchURL(%s)\n", url); )

   // Initialize the response
   respSize= 0;
   response[0]= '\0';

   // Fetch the URL
   curl_easy_setopt(handle, CURLOPT_URL, url);
   CURLcode cc= curl_easy_perform(handle);

   // Remove trailing CR/LF from response (if present)
   int L= strlen(response);
   L--;
   while( L >= 0 && (response[L] == '\r' || response[L] == '\n') )
   {
     response[L]= '\0';
     L--;
   }

   // If error, write message
   if( cc != 0 )
     logger("%s ERROR: %d= fetchURL(%s) %s\n",
            dateTime, cc, url, error_buffer);

   // Return the response code
   return cc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       use_nslookup
//
// Purpose-
//       Obtain the host IP address.
//
//----------------------------------------------------------------------------
static int64_t                      // The host IP address, 0 if unavailable
   use_nslookup( void )             // Get host IP address via nslookup
{
   // Get the current ip address via name resolution, i.e. gethostbyname
   int64_t look_ipaddr= 0;
   hostent* hostEntry= ::gethostbyname(CTL_target);
   if( hostEntry != NULL )
     look_ipaddr= (unsigned long)ntohl(*(long*)hostEntry->h_addr);

   return look_ipaddr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       use_web
//
// Purpose-
//       Obtain the host IP address, setting this_iptext
//
//----------------------------------------------------------------------------
static int64_t                      // The host IP address, 0 if unavailable
   use_web(                         // Get host IP address via web
     CURL*             handle,      // CURL handle
     const char*       dateTime)    // Current date and time
{
   // Mode: use=web, url in CTL_web
   CURLcode cc= fetchURL(handle, CTL_web, dateTime); // (Sets response)
   if( cc != 0 )
     return 0;                      // FAILED, probable internet outage

   // Validate the response (numeric values unchecked)
   int dots= 0;                     // Number of '.' characters
   int numb= 1;                     // Number required
   size_t x= 0;                     // Reponse index
   while( response[x] != '\0' )
   {
     if( isdigit(response[x]) )
       numb= 0;
     else
     {
       if( numb || response[x] != '.' )
       {
         dots= 0;
         break;
       }

       dots++;
       numb= 1;
     }

     x++;
   }

   if( numb || dots != 3 || x >= sizeof(this_iptext) )
   {
     // If invalid IPV4 address or response too long
     logger("%s ERROR: Invalid response(%s) from(%s)\n",
            dateTime, response, CTL_web);
     return 0;
   }

   strcpy(this_iptext, response);   // Response is actual IP address
   IFHCDM( logger("this_iptext(%s)\n", this_iptext); )

   return ntohl(inet_addr(this_iptext));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       protocol_duckdns
//
// Purpose-
//       If required, update our IP address in the database.
//
//----------------------------------------------------------------------------
static int                          // Number of seconds to delay
   protocol_duckdns( void )         // Update the database
{
   char                dateTime[32];// Date/Time holding area
   int64_t             look_ipaddr; // IP address from use_nslookup
   int64_t             myip_ipaddr; // IP address from use_web
   char                work[4096];  // Work area, mostly to build URLs

   CURLcode            cc;

   // Get time of day for log
   fetchTOD(dateTime);

   // Get the current ip address via name resolution, i.e. gethostbyname
   look_ipaddr= use_nslookup();

   // Initialize CURL, set up handle cleanup code
   CURL* handle= curl_easy_init();
   IFHCDM( logger("%p= curl_easy_init()\n", handle); )
   if( handle == NULL )
   {
     logger("%s ERROR: %4d curl_easy_init() failure\n", dateTime, __LINE__);
     return DELAY_NG;
   }

   AutoCURL autoCURL(handle);       // Common initialization, guaranteed cleanup

   if( strcmp(CTL_use, "web") == 0 )
   {
     // Mode: use=web, url in CTL_web
     myip_ipaddr= use_web(handle, dateTime); // (Setting this_iptext)
     if( myip_ipaddr == 0 )         // If failure
       return DELAY_IP;

     IFHCDM( logger("%4d look(%llx) myip(%llx)\n", __LINE__, look_ipaddr, myip_ipaddr); )
     if( look_ipaddr == myip_ipaddr )
     {
       strcpy(last_iptext, this_iptext);
       logger("%s No update required, no update performed\n", dateTime);
       return DELAY_OK;
     }

     // If the last IP address we got was the same, skip update
     // (A problem probably exists, but we can't correct it automatically.)
     IFHCDM( logger("last_iptext(%s)\n", last_iptext); )
     if( strcmp(last_iptext, this_iptext) == 0 )
     {
       logger("%s last_iptext == this_iptext(%s), no update performed\n",
              dateTime, this_iptext);
       return DELAY_IP;
     }
   }

   //-------------------------------------------------------------------------
   // Update required
   strcpy(work, CTL_server);        // e.g. "https://www.duckdns.org"
   int L= strlen(work) - 1;         // Add trailing '/' if not present
   if( work[L] != '/' )             // (Length verified non-zero)
     strcat(work, "/");

   strcat(work, "update/?domains="); // Set hostname
   char* C= work + strlen(work);    // The CTL_target origin
   strcat(work, CTL_target);
   C= strchr(C, '.');               // Strip ".duckdns.org" from domains
   if( C != NULL )
     *C= '\0';

   strcat(work, "&token=");         // Set token
   strcat(work, CTL_password);

   if( strcmp(CTL_use, "web") == 0 )
   {
     strcat(work, "&ip=");            // Set IP address
     strcat(work, this_iptext);
   }

   cc= fetchURL(handle, work, dateTime);
   if( cc != 0 )
     return DELAY_NG;

   // Verify resultant
   strcpy(last_iptext, this_iptext); // Prevent pointless retry
   if( memcmp(response, "OK", 2) != 0 )
   {
     logger("%s ERROR: Unexpected update response(%s)\n", dateTime, response);
     if( memcmp(response, "nochg", 5) == 0 )
       logger("WARNING: nochg requests are considered abusive\n");
     else
       logger("WARNING: UpdateDNS INTERNAL PROGRAM ERROR LIKELY\n");

     return DELAY_NG;               // Extra delay for these conditions
   }

   // Successful update
   if( strcmp(CTL_use, "web") == 0 )
     logger("%s IP address updated to '%s'\n", dateTime, last_iptext);
   else
     logger("%s IP address verified\n", dateTime);
   return DELAY_OK;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       protocol_dyndns2
//
// Purpose-
//       If required, update our IP address in the database.
//
//----------------------------------------------------------------------------
static int                          // Number of seconds to delay
   protocol_dyndns2( void )         // Update the database
{
   char                dateTime[32];// Date/Time holding area
   int64_t             look_ipaddr; // IP address from use_nslookup
   int64_t             myip_ipaddr; // IP address from use_web
   char                work[4096];  // Work area, mostly to build URLs

   CURLcode            cc;

   // Get time of day for log
   fetchTOD(dateTime);

   // Get the current ip address via name resolution, i.e. gethostbyname
   look_ipaddr= use_nslookup();

   // Initialize CURL, set up handle cleanup code
   CURL* handle= curl_easy_init();
   IFHCDM( logger("%p= curl_easy_init()\n", handle); )
   if( handle == NULL )
   {
     logger("%s ERROR: %4d curl_easy_init() failure\n", dateTime, __LINE__);
     return DELAY_NG;
   }

   AutoCURL autoCURL(handle);       // Common initialization, guaranteed cleanup

   // Mode: use=web, url in CTL_web
   myip_ipaddr= use_web(handle, dateTime); // (Setting this_iptext)
   if( myip_ipaddr == 0 )           // If failure
     return DELAY_NG;

   IFHCDM( logger("%4d look(%llx) myip(%llx)\n", __LINE__, look_ipaddr, myip_ipaddr); )
   if( look_ipaddr == myip_ipaddr )
   {
     strcpy(last_iptext, this_iptext);
     logger("%s No update required, no update performed\n", dateTime);
     return DELAY_OK;
   }

   // If the last IP address we got was the same, skip update
   // (A problem probably exists, but we can't correct it automatically.)
   IFHCDM( logger("last_iptext(%s)\n", last_iptext); )
   if( strcmp(last_iptext, this_iptext) == 0 )
   {
     struct in_addr ipaddr= {htonl(look_ipaddr)};
     logger("%s look_iptext(%s) last_iptext == this_iptext(%s), no update performed\n",
            dateTime, inet_ntoa(ipaddr), this_iptext);
     logger("(ERROR: Manual IP address validation required)\n");
     return DELAY_NG;
   }

   //-------------------------------------------------------------------------
   // Update required
   strcpy(work, CTL_server);        // e.g. "https://www.dnsdynamic.org"
   int L= strlen(work) - 1;         // Add trailing '/' if not present
   if( work[L] != '/' )             // (Length verified non-zero)
     strcat(work, "/");

   strcat(work, "api/?hostname=");  // Set hostname
   strcat(work, CTL_target);

   strcat(work, "&myip=");          // Set IP address
   strcat(work, this_iptext);

   curl_easy_setopt(handle, CURLOPT_HTTPAUTH, long(CURLAUTH_ANY));
   curl_easy_setopt(handle, CURLOPT_NETRC, long(CURL_NETRC_IGNORED));
   curl_easy_setopt(handle, CURLOPT_USERNAME, CTL_username);
   curl_easy_setopt(handle, CURLOPT_PASSWORD, CTL_password);
   cc= fetchURL(handle, work, dateTime);
   if( cc != 0 )
     return DELAY_NG;

   // Verify resultant
   strcpy(last_iptext, this_iptext); // Prevent pointless retry
   if( memcmp(response, "good ", 5) != 0 )
   {
     logger("%s ERROR: Unexpected update response(%s)\n", dateTime, response);
     if( memcmp(response, "nochg", 5) == 0 )
       logger("WARNING: nochg requests are considered abusive\n");
     else
       logger("WARNING: UpdateDNS INTERNAL PROGRAM ERROR LIKELY\n");

     return DELAY_NG;               // Extra delay for these conditions
   }

   if( strcmp(response+5, last_iptext) != 0 )
   {
     logger("%s ERROR: Unexpected update response(%s)\n", dateTime, response);
     return DELAY_NG;
   }

   // Successful update
   logger("%s IP address updated to '%s'\n", dateTime, last_iptext);
   return DELAY_OK;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       readControlLine
//
// Purpose-
//       Load a control file line, handle trim.
//
//----------------------------------------------------------------------------
static int                          // The line terminator
   readControlLine(                 // Read a control file line
     FILE*             inp)         // The control file handle
{
   int                 C;           // Current character

   respSize= 0;                     // No response yet
   for(;;)
   {
     C= fgetc(inp);                 // Next character
     if( C == '\n' || C == '\0' || C == EOF ) // If terminator
       break;

     if( C == '#' )                 // If begin comment
     {
       while( C != '\n' && C != '\0' && C != EOF ) // Skip to end of line
         C= fgetc(inp);

       break;
     }

     if( C == '\r' )                // Ignore CR
       continue;

     if( C == ' ' )                 // Ignore blank
       continue;

     if( C == '\'' || C == '\"' )   // If begin quote
     {
       int quote= C;                // Process quote
       for(;;)                      // Within quote
       {
         C= fgetc(inp);             // Next character
         if( C == quote )           // If quote terminator
           break;

         if( C == '\r' || C == '\n' || C == '\0' || C == EOF )
           shouldNotOccur("Missing end quote in(%s)\n", response);

         if( respSize >= (sizeof(response)-1) )
           shouldNotOccur("Control line too long(%s)\n", response);

         response[respSize++]= C;
       }

       continue;                    // After quote
     }

     // Add character to control line
     if( respSize >= (sizeof(response)-1) )
       shouldNotOccur("Control line too long(%s)\n", response);

     response[respSize++]= C;
   }

   response[respSize]= '\0';        // Terminate line
   return C;                        // Return terminator character
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       readControl
//
// Purpose-
//       Load the control file
//
//----------------------------------------------------------------------------
static void
   readControl( void )              // Load the control file
{
   int                 i;

   FILE* inp= fopen(INP_FILE_NAME, "rb"); // Open the input file
   if( inp == NULL )
     shouldNotOccur("Open failed for control file(%s)\n", INP_FILE_NAME);

   // Read the control file
   for(;;)
   {
     int cc= readControlLine(inp);  // Read next line
     if( response[0] != '\0' )      // If the line is not empty
     {
       char* V= strchr(response, '='); // Look for '=' delimiter
       if( V == NULL )              // If not found, target line
       {
         if( CTL_target != NULL )
           shouldNotOccur("%s, target(%s), but target(%s) already set\n",
                          INP_FILE_NAME, response, CTL_target);

         CTL_target= replace(CTL_target, response);
       }
       else
       {
         *V= '\0';                  // Set name delimiter
         V++;                       // Address value
         for(i= 0; CTL_nv[i].N != NULL; i++)
         {
           if( strcmp(CTL_nv[i].N, response) == 0 )
             break;
         }

         if( CTL_nv[i].N == NULL )
           shouldNotOccur("%s, unknown control(%s)\n", INP_FILE_NAME, response);

         *CTL_nv[i].V= replace(*CTL_nv[i].V, V);
       }
     }

     if( cc == EOF )
       break;
   }

   // Verify required values
   if( strcmp(CTL_protocol, "dyndns2") != 0
       && strcmp(CTL_protocol, "duckdns") != 0 )
     shouldNotOccur("protocol(%s) not supported\n", CTL_protocol);
   if( strcmp(CTL_use, "web") != 0 && strcmp(CTL_use, "none") != 0 )
     shouldNotOccur("use(%s) not supported\n", CTL_use);
   if( CTL_username == NULL ) shouldNotOccur("Missing login=\n");
   if( CTL_password == NULL ) shouldNotOccur("Missing password=\n");
   if( CTL_target   == NULL ) shouldNotOccur("Missing update target\n");
   if( memcmp(CTL_web, "http://", 7) != 0 )
     shouldNotOccur("web(%s) invalid, not http://\n", CTL_web);
   if( memcmp(CTL_server, "http://", 7) != 0
       && memcmp(CTL_server, "https://", 8) != 0 ) // (Also verifies length)
     shouldNotOccur("server(%s) invalid, neither http:// nor https://\n",
                    CTL_server);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       updater
//
// Purpose-
//       If required, update our IP address in the database.
//
//----------------------------------------------------------------------------
static int                          // Number of seconds to delay
   updater( void )                  // Update the database
{
   if( strcmp(CTL_protocol, "dyndns2") == 0 )
     return protocol_dyndns2();

   if( strcmp(CTL_protocol, "duckdns") == 0 )
     return protocol_duckdns();

   shouldNotOccur("protocol(%s) not supported\n", CTL_protocol);
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter informational display.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter analysis
{
   fprintf(stderr,
           "UpdateDNS: Update dynamic DNS server\n"
           "\n"
           "Options:\n"
           "  -inp:name-of-control-file (Default: ./UpdateDNS.inp)\n"
           "  -out:name-of-control-file (Default: NONE) (Overwrite)\n"
           "  -log:name-of-logging-file (Default: ./UpdateDNS.log) (Append)\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 ERROR;       // Error encountered indicator
   int                 HELPI;       // Help encountered indicator

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   ERROR= FALSE;                    // Set defaults
   HELPI= FALSE;
   sw_output= FALSE;
   sw_verify= FALSE;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       if( strcmp("-help", argp) == 0 // If help request
           || strcmp(argp, "--help") == 0 )
         HELPI= TRUE;

       else if( memcmp(argp, "-inp:", 5) == 0 )
         INP_FILE_NAME= replace(INP_FILE_NAME, argp+5);

       else if( memcmp(argp, "-log:", 5) == 0 )
         LOG_FILE_NAME= replace(LOG_FILE_NAME, argp+5);

       else if( memcmp(argp, "-out:", 5) == 0 ) {
         sw_output = TRUE;
         LOG_FILE_NAME= replace(LOG_FILE_NAME, argp+5);
       }

       else if( strcmp(argp, "-v") == 0 // Not a published parameter
           || strcmp(argp, "-verify") == 0 )
         sw_verify= TRUE;

       else                         // If invalid switch
       {
         ERROR= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       ERROR= TRUE;
       fprintf(stderr, "Unexpected parameter '%s'\n", argv[argi]);
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( HELPI || ERROR )
   {
     if( ERROR )
       fprintf(stderr, "\n");

     info();
   }

   if( sw_verify )
   {
     fprintf(stderr, "-inp: '%s'\n", INP_FILE_NAME);
     if( sw_output )
       fprintf(stderr, "-out: '%s'\n", LOG_FILE_NAME);
     else
       fprintf(stderr, "-log: '%s'\n", LOG_FILE_NAME);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis

   // Create the log file
   if( sw_output )
     stdlog= fopen(LOG_FILE_NAME, "wb"); // Open the log file (replace)
   else
     stdlog= fopen(LOG_FILE_NAME, "ab"); // Open the log file
   if( stdlog == NULL )
     shouldNotOccur("Cannot open log file(%s)\n", LOG_FILE_NAME);

   // Read the control file
   readControl();
   logger("UpdateDNS started for %s\n", CTL_target);

   // If sw_verify, display all values
   if( sw_verify )
   {
     logger("protocol: '%s'\n", CTL_protocol);
     logger("use:      '%s'\n", CTL_use);
     logger("web:      '%s'\n", CTL_web);
     logger("server:   '%s'\n", CTL_server);
     logger("login:    '%s'\n", CTL_username);
     logger("password: '%s'\n", CTL_password);
     logger("target:   '%s'\n", CTL_target);
   }

   // Update loop (never terminates)
   strcpy(last_iptext, "0.0.0.0");
   for(;;)
   {
     int delay= updater();
     IFHCDM( logger("%d= updater()\n", delay); )
     if( delay == 0 )
       break;

     sleep(delay);
   }
   shouldNotOccur("Internal error, program terminated\n");

   return EXIT_FAILURE;
}

