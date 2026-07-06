//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       RdPatch.cpp
//
// Purpose-
//       Common routines used by RdClient and RdServer.
//
// Last change date-
//       2026/05/30
//
// Implementation notes-
//       PATCH: Use /etc/hosts name if available
//
//----------------------------------------------------------------------------
#include <cerrno>                   // For errno, ...
#include <cstring>                  // For memcpy
#include <string>                   // For std::string
#include <netdb.h>                  // For gai_strerror
#include <unistd.h>                 // For gethostname
#include <arpa/inet.h>              // For internet address types
#include <sys/socket.h>             // For sockaddr, socklen_t

#include <com/Debug.h>              // For debugf
#include <pub/Data.h>               // For pub::Data, ...
#include <pub/Tokenizer.h>          // For pub::Tokenizer

#include "RdPatch.h"                // For get_sockaddr, implemented

using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false
,  VERBOSE= 0
}; // (generic) enum

//----------------------------------------------------------------------------
//
// Subroutine-
//       etc_addr
//
// Purpose-
//       Get socket information for name from "/etc/hosts" file
//
// Implementation notes-
//       Backport patch to use /etc/hosts name. (Requires pub library)
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   etc_addr(                        // Convert "host:port" to sockaddr
     const std::string&host,        // The host name string
     sockaddr*         sock,        // OUT: The sockaddr
     socklen_t*        size)        // INP/OUT: The sockaddr length
{
   // Search "/etc/hosts" file for host name
   using namespace pub::data;
   Data file("/etc", "hosts");
   pub::DHDL_list<Line>& list= file.line();

   for(Line* line= list.get_head(); line; line= line->get_next() ) {
     pub::Tokenizer tokenizer(line->text);
     pub::Tokenizer::Iterator it= tokenizer.begin();
     if( it != tokenizer.end() ) {
       std::string addr= it();
       if( addr[0] != '#' ) {
         for(it= ++it; it != tokenizer.end(); ++it) {
           if( it() == host ) {
             struct in_addr v4_addr;
             int
             rc= inet_pton(AF_INET, addr.c_str(), &v4_addr);
             if( rc == 1 ) {
               if( size_t(*size) < sizeof(sockaddr_in) ) {
                 errno= ENOMEM;
                 return -1;
               }

               memset(sock, 0, sizeof(sockaddr_in));
               ((sockaddr_in*)sock)->sin_family= AF_INET;
               ((sockaddr_in*)sock)->sin_addr= v4_addr;
               *size= sizeof(sockaddr_in);
               return 0;
             }

#if 0 // inc/com/Socket.h can only support AF_INET
             struct in6_addr v6_addr;
             rc= inet_pton(AF_INET6, addr.c_str(), &v6_addr);
             if( rc == 1 ) {
               if( size_t(*size) < sizeof(sockaddr_in6) ) {
                 errno= ENOMEM;
                 return -1;
               }

               memset(sock, 0, sizeof(sockaddr_in6));
               ((sockaddr_in6*)sock)->sin6_family= AF_INET6;
               ((sockaddr_in6*)sock)->sin6_addr= v6_addr;
               *size= sizeof(sockaddr_in6);
               return 0;
             }
#endif

             debugf("/etc/hosts invalid line: %s\n", line->text);
             return -1;
           }
         }
       }
     }
   }

   return -1;                       // Not found
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_sockaddr
//
// Purpose-
//       Get socket information for name
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   get_sockaddr(                    // Convert "host:port" to sockaddr
     const std::string&nps,         // The "host:port" name string
     void*             _addr,       // OUT: The sockaddr
     int*              _size)       // INP/OUT: The sockaddr length
{  if(HCDM) debugf("get_sockaddr(%s,%p,%d)\n", nps.c_str(), _addr, *_size);

   struct sockaddr* addr= (struct sockaddr*)_addr;
   socklen_t*       size= (socklen_t*)_size;

   // Validate parameters
   if( size_t(*size) < sizeof(sockaddr_in) ) {
     debugf("%4d RdPatch length(%d) < minimum(%zd)\n", __LINE__, *size
           , sizeof(sockaddr_in));
     errno= EINVAL;
     return EINVAL;
   }

   // Separate host name and port number from string
   size_t x= nps.size();
   while( --x ) {
     char C= nps[x];
     if( C < '0' || C > '9' ) {
       if( C == ':' )
         break;

       debugf("get_sock_addr(%s) invalid port number)\n", nps.c_str());
       errno= EINVAL;
       return EINVAL;
     }
   }
   if( x == 0 && nps[0] != ':' ) {
     debugf("%4d RdPatch name(%s) missing ':' delimiter\n", __LINE__
           , nps.c_str());
     errno= EINVAL;
     return EINVAL;
   }

   std::string name;
   if( x )
     name= nps.substr(0, x);
   else {
     char buffer[256];
     buffer[0]= '\0';
     ::gethostname(buffer, sizeof(buffer));
     name= buffer;
   }

   std::string port= nps.substr(x+1);
   if( port == "" )
     port= "0";

   // If name is specified in /etc/hosts, use the associated address
   if( etc_addr(name, addr, size) == 0 ) {
     ((sockaddr_in*)addr)->sin_port= htons((short)std::stoi(port));
     return 0;
   }

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Select the first usable entry returned by getaddrinfo
   addrinfo* info= nullptr;         // Resultant info
   int rc= getaddrinfo(name.c_str(), port.c_str(), nullptr, &info);
   if( rc ) {                       // If unable to get addrinfo
     debugf("%4d '%s' %s\n", __LINE__, nps.c_str(), gai_strerror(rc));
     *size= 0;
     errno= EINVAL;
     rc= -1;
   } else {
     addrinfo* used= info;
     while( used ) {
       if( used->ai_family == AF_INET && used->ai_socktype == SOCK_STREAM )
         break;

       used= used->ai_next;
     }
     if( used ) {
       memset(addr, 0, sizeof(sockaddr_in));
       ((sockaddr_in*)addr)->sin_family= AF_INET;
       ((sockaddr_in*)addr)->sin_addr= ((sockaddr_in*)used->ai_addr)->sin_addr;
       ((sockaddr_in*)addr)->sin_port= htons((short)std::stoi(port));
       *size= sizeof(sockaddr_in);
     } else {
       rc= -1;
       errno= EINVAL;
     }

     freeaddrinfo(info);
   }
   return rc;
}
