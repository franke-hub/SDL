//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HTTP.cpp
//
// Purpose-
//       Implement http/HTTP.h
//
// Last change date-
//       2023/06/24
//
//----------------------------------------------------------------------------
// #include <string>                   // For std::string

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/http/HTTP.h"          // For pub::http::HTTP, implemented

#define _PUB _LIBPUB_NAMESPACE
using namespace _PUB;
using namespace _PUB::debugging;
// using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Status code to (minimal) text conversion table
//----------------------------------------------------------------------------
struct code_to_text {
int                    code;        // The numeric code
const char*            text;        // The associated text
};

static code_to_text    code_text[]=
{  {  0, "UNKNOWN CODE"}
,  {100, "CONTINUE"}
,  {101, "SWITCHING PROTOCOLS"}
,  {102, "PROCESSING"}              // (WebDAV)
,  {103, "EARLY HINTS"}
,  {200, "OK"}
,  {201, "CREATED"}
,  {202, "ACCEPTED"}
,  {203, "NON-AUTHORITIVE INFORMATION"}
,  {204, "NO CONTENT"}
,  {205, "RESET CONTENT"}
,  {206, "PARTIAL CONTENT"}
,  {207, "MULTI-STATUS"}            // (WebDAV)
,  {208, "ALREADY REPORTED"}        // (WebDAV)
,  {226, "IM USED"}                 // IM: Instance Manipulation
,  {300, "MULTIPLE CHOICE"}
,  {301, "MOVED PERMANENTLY"}
,  {302, "FOUND"}                   // (Moved temporarily)
,  {303, "SEE OTHER"}
,  {304, "NOT MODIFIED"}
,  {307, "TEMPORARY REDIRECT"}
,  {308, "PERMANENT REDIRECT"}
,  {400, "BAD REQUEST"}
,  {401, "NOT AUTHORIZED"}
,  {402, "PAYMENT REQUIRED"}
,  {403, "FORBIDDEN"}
,  {404, "NOT FOUND"}
,  {405, "METHOD NOT ALLOWED"}
,  {406, "NOT ACCEPTABLE"}
,  {407, "PROXY AUTHENTICATION REQUIRED"}
,  {408, "REQUEST TIMEOUT"}
,  {409, "CONFLICT"}
,  {410, "GONE"}
,  {411, "LENGTH REQUIRED"}
,  {412, "PRECONDITION FAILED"}
,  {413, "PAYLOAD TOO LARGE"}
,  {414, "URI TOO LARGE"}
,  {415, "UNSUPPORTED MEDIA TYPE"}
,  {416, "RANGE NOT SATISFIABLE"}
,  {417, "EXPECTATION FAILED"}
,  {418, "I'M A TEAPOT"}
,  {421, "MISDIRECTED REQUEST"}
,  {422, "UNPROCESSABLE CONTENT"}   // (WebDAV)
,  {423, "LOCKED"}                  // (WebDAV)
,  {424, "FAILED DEPENDENCY"}       // (WebDAV)
,  {425, "TOO EARLY"}
,  {426, "UPGRADE REQUIRED"}
,  {428, "PRECONDITION REQUIRED"}
,  {429, "TOO MANY REQUESTS"}
,  {431, "REQUEST HEADER FIELDS TOO LARGE"}
,  {451, "UNAVAILABLE FOR LEGAL REASONS"}
,  {500, "INTERNAL SERVER ERROR"}
,  {501, "NOT IMPLEMENTED"}
,  {502, "BAD GATEWAY"}
,  {503, "SERVICE UNAVAILABLE"}
,  {504, "GATEWAY TIMEOUT"}
,  {505, "HTTP VERSION NOT SUPPORTED"}
,  {506, "VARIANT ALSO NEGOTIATES"}
,  {507, "INSUFFICIENT STORAGE"}    // (WebDAV)
,  {508, "LOOP DETECTED"}           // (WebDAV)
,  {510, "NOT EXTENDED"}
,  {511, "NETWORK AUTHENTICATION REQUIRED"}
,  {599, "CLIENT DISCONNECTED"}
,  {  0, nullptr}                   // Delimiter
};

//----------------------------------------------------------------------------
//
// Method-
//       HTTP::status_text
//
// Purpose-
//       Get text for status code
//
//----------------------------------------------------------------------------
const char*                         // The status text
   HTTP::status_text(               // Get status text for
     int               code)        // This numeric code
{
   for(int i= 1; code_text[i].text; ++i) { // (code_text[0] is internal error)
     if( code == code_text[i].code )
       return code_text[i].text;
   }

   debugh("%4d %s code(%d) undefined\n", __LINE__, __FILE__, code);
   return code_text[0].text;
}
}  // namespace _LIBPUB_NAMESPACE::http
