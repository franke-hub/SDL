//----------------------------------------------------------------------------
//
//       Copyright (C) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the "un-license,"
//       explicitly released into the Public Domain.
//       (See accompanying file LICENSE.UNLICENSE or the original
//       contained within http://unlicense.org)
//
//----------------------------------------------------------------------------
//
// Title-
//       Socket.cpp
//
// Purpose-
//       Document standard posix socket usage.
//
// Last change date-
//       2019/03/28
//
// Usage notes-
//       NO ATTRIBUTION NEEDED OR WANTED.
//       This is public domain sample code.
//
//----------------------------------------------------------------------------
#include <errno.h>                  // For errno, ...
#include <stdio.h>                  // For printf, perror, ...
#include <stdlib.h>                 // For EXIT_FAILURE, ...
#include <string.h>                 // For memset, strerror, ...
#include <sys/unistd.h>             // For close, ...

#include "Socket.h"                 // (Separately licensed)

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       Print description
//
//----------------------------------------------------------------------------
void
   debug(                           // Print description of
     const char*       site,        // Site
     struct hostent*   hent)        // struct hostent*
{
   char                buffer[512]; // Working buffer

   for(int i= 0; hent->h_addr_list[i]; i++) {
     inet_ntop(hent->h_addrtype, hent->h_addr_list[i], buffer, sizeof(buffer));
     printf("[%2d] IPV4(%s): %s\n", i, site, buffer);
   }
}

int                                 // Associated length
   debug(                           // Print description of
     const char*       site,        // Site
     sockaddr*         sock)        // This socket address
{
   char                buffer[512]; // Working buffer
   int                 length= -1;  // Associated length

   if( sock->sa_family == AF_INET ) { // If IPV4
     sockaddr_in* in4= (sockaddr_in*)sock;
     length= sizeof(sockaddr_in);

     inet_ntop(AF_INET, &(in4->sin_addr), buffer, sizeof(buffer));
     printf("IPV4(%s): %s:%d\n", site, buffer, ntohs(in4->sin_port));
   } else if( sock->sa_family == AF_INET6 ) {
     sockaddr_in6* in6= (sockaddr_in6*)sock;
     length= sizeof(sockaddr_in6);

     inet_ntop(AF_INET6, &(in6->sin6_addr), buffer, sizeof(buffer));
     printf("IPV6(%s): [%s]:%d\n", site, buffer, ntohs(in6->sin6_port));
   } else {
     fprintf(stderr, "Invalid sa_family(%d)\n", sock->sa_family);
     exit(EXIT_FAILURE);
   }

   return length;
}

void
   debug(                           // Print description of
     const char*       site,        // Site
     struct addrinfo*  ai)          // This struct addrinfo
{
   char                buffer[512]; // Working buffer

   if( ai->ai_family == AF_INET ) {
     sockaddr_in* in4= (sockaddr_in*)ai->ai_addr;
     inet_ntop(AF_INET, &(in4->sin_addr), buffer, sizeof(buffer));
     printf("IPV4(%s): %s ftp[%x,%d,%d]\n", site,
            buffer, ai->ai_flags, ai->ai_socktype, ai->ai_protocol);
   } else if( ai->ai_family == AF_INET6 ) {
     sockaddr_in6* in6= (sockaddr_in6*)ai->ai_addr;
     inet_ntop(AF_INET6, &(in6->sin6_addr), buffer, sizeof(buffer));
     printf("IPV6(%s): %s ftp[%x,%d,%d]\n", site,
            buffer, ai->ai_flags, ai->ai_socktype, ai->ai_protocol);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       visify
//
// Purpose-
//       Print string, expanding '\r' and '\n' to "\\r" and "\\n" respectively
//
//----------------------------------------------------------------------------
void
   visify(                          // Visify
     const char*       inp,         // This string
     int               L)           // For this length
{
   for(int i= 0; i<L; i++) {
     int C= inp[i];
     if( C == '\r' ) {
       printf("\\");
       C= 'r';
     } else if( C == '\n' ) {
       printf("\\");
       C= 'n';
     }
     printf("%c", C);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       client
//
// Purpose-
//       Client send/recv operation.
//
//----------------------------------------------------------------------------
int                                 // Return code
   client(                          // Client send/receive
     const char*       site,        // Client host name
     sockaddr*         sock)        // Socket address
{
   char   buffer[4096];             // Working buffer
   int    fd= Socket::CLOSED;       // File descriptor (used directly)
   int    ERRNO;                    // Saved errno
   int    length;                   // Socket address length
   int    rc;                       // A return code

   if( sock == nullptr ) {
     printf("Cannot locate '%s' %d,%s\n", site, errno, strerror(errno));
     return EXIT_FAILURE;
   }

   length= debug(site, sock);       // Display socket, return length
   fd= ::socket(sock->sa_family, SOCK_STREAM, 0);

   // Connect
   rc= ::connect(fd, sock, length);
   ERRNO= errno;
   if( rc != 0 ) {
     sprintf(buffer, "Cannot connect '%s'", site);
     perror(buffer);
     ::close(fd);
     return EXIT_FAILURE;
   }

   // Set timeouts
   struct timeval tv= { 3, 0 };     // 3.0 second timeout
   rc= ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
   if( rc != 0 )
     perror("setsockopt(SO_SNDTIMEO)");
   rc= ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
   if( rc != 0 )
     perror("setsockopt(SO_RCVTIMEO)");

   // Send request
static const char*     request=
   "GET / HTTP/1.1\r\n"
   "Host: %s\r\n"
   "Accept: */*\r\n"
   "User-Agent: Example\r\n"
   "\r\n"
   ;

   sprintf(buffer, request, site);
   int L= ::send(fd, buffer, strlen(buffer), 0);
   ERRNO= errno;
   if( L > 0 ) {
     printf("%d= send(", L); visify(buffer, strlen(buffer)); printf(")\n");

     // Send OK, receive responses until timeout
     char* response= buffer;
     for(;;) {
       int L= ::recv(fd, response, sizeof(buffer)-1, 0);
       ERRNO= errno;
       if( L <= 0 ) {
         fprintf(stderr, "%d= recv(%zd) %d,%s\n", L, sizeof(buffer)-1,
                 ERRNO, strerror(ERRNO));
         break;
       }

       printf("%d= recv(", L); visify(response, L); printf(")\n");
     }
   } else {
     fprintf(stderr, "%d= send(%zd) %d,%s\n", L, strlen(request),
             ERRNO, strerror(ERRNO));
   }

   // Close the Socket
   ::close(fd);
   return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       use_getaddrinfo
//
// Purpose-
//       Use getaddrinfo to generate sockaddr*
//
//----------------------------------------------------------------------------
sockaddr*                           // Resultant
   use_getaddrinfo(                 // Generate sockaddr* using getaddrinfo
     const char*       site)        // And this site
{
static sockaddr_storage  inS;       // sockaddr (maximum size)
   struct addrinfo     hints;
   struct addrinfo*    ai_list= nullptr;

   printf("use_getaddrinfo\n");
   memset(&inS,   0, sizeof(inS));
   memset(&hints, 0, sizeof(hints));
   hints.ai_family=   AF_UNSPEC;
   hints.ai_socktype= SOCK_STREAM;
   hints.ai_protocol= 0;
   hints.ai_addr=     (sockaddr*)&inS;
// int rc= getaddrinfo(site, "HTTP", &hints, &ai_list);
   int rc= getaddrinfo(site, nullptr, nullptr, &ai_list);
   int ERRNO= errno;
   if( rc != 0 ) {
     printf("%d= getaddrinfo(%s) %p\n", rc, site, ai_list);
     errno= ERRNO;
     perror("getaddrinfo");
     return nullptr;
   }

   // DIAGNOSTICS
   struct addrinfo* valid= nullptr;
   if( ai_list ) {
     int count= 0;
     struct addrinfo* ai= ai_list;
     while( ai ) {
       if( !valid ) {
         if( ai->ai_socktype == 0 || ai->ai_socktype == SOCK_STREAM ) {
           if( ai->ai_protocol == 0 || ai->ai_protocol == IPPROTO_TCP ) {
             valid= ai;
             printf("[%2d]*", count++);
           }
         }

         if( !valid )
           printf("[%2d] ", count++);
       } else
         printf("[%2d] ", count++);

       debug(site, ai);
       ai= ai->ai_next;
     }
   } else {
     return nullptr;
   }

   // Copy the first valid addrinfo.ai_addr
   if( !valid ) {                   // If no valid
     freeaddrinfo(ai_list);
     return nullptr;
   }

   memcpy(&inS, valid->ai_addr, valid->ai_addrlen);
   freeaddrinfo(ai_list);

   // Set port numer
   if( inS.ss_family == AF_INET )
     ((sockaddr_in*)&inS)->sin_port= htons(80);
   else
     ((sockaddr_in6*)&inS)->sin6_port= htons(80);

   return (sockaddr*)&inS;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       use_gethostbyname
//
// Purpose-
//       Use gethostbyname to generate sockaddr*
//
//----------------------------------------------------------------------------
sockaddr*                           // Resultant
   use_gethostbyname(               // Generate sockaddr* using gethostbyname
     const char*       site)        // And this site
{
static sockaddr_in     in4;         // IPV4 sockaddr
static sockaddr_in6    in6;         // IPV6 sockaddr
   struct sockaddr* addr= nullptr;  // Resultant sockaddr

   printf("use_gethostbyname\n");

   memset(&in4, 0, sizeof(in4));
   memset(&in6, 0, sizeof(in6));
   struct hostent* hent= gethostbyname(site);
   if( hent != nullptr ) {
     debug(site, hent);

     if( hent->h_addrtype == AF_INET ) {
       in4.sin_family= AF_INET;
       in4.sin_addr= *(struct in_addr*)hent->h_addr;
       in4.sin_port= htons(80);       // Set port number
       addr= (sockaddr*)&in4;
     } else if( hent->h_addrtype == AF_INET6 ) {
       in6.sin6_family= AF_INET6;
       in6.sin6_addr= *(struct in6_addr*)hent->h_addr;
       in6.sin6_port= htons(80);       // Set port number
       addr= (sockaddr*)&in6;
     } else {
       fprintf(stderr, "Invalid hostent.addrtype(%d)\n", hent->h_addrtype);
     }
   }

   return addr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   struct sockaddr*    addr= nullptr; // Our sockaddr

   // Verify sizes (only)
   if( false ) {
     printf("%4zd= sizeof(sockaddr_in)\n",      sizeof(sockaddr_in));
     printf("%4zd= sizeof(sockaddr_in6)\n",     sizeof(sockaddr_in6));
     printf("%4zd= sizeof(sockaddr_storage)\n", sizeof(sockaddr_storage));
     return EXIT_FAILURE;
   }

   // Get site address
   const char* site= "example.com";
   if( argc > 1 )
     site= argv[1];

   // Reverse these to change which one is used
   addr= use_gethostbyname(site);
   addr= use_getaddrinfo(site);

   // Process client request
   if( true  ) {
     return client(site, addr);
   }

   return EXIT_FAILURE;
}
