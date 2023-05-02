//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestSock.cpp
//
// Purpose-
//       Test Socket object.
//
// Last change date-
//       2020/10/03
//
// Usage-
//       TestSock receive
//       TestSock send hostname:hostport (on different command line)
// -or-  TestSock send_delay hostname:hostport (on different command line)
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/define.h>
#include <com/Debug.h>
#include <com/istring.h>
#include <com/Logger.h>
#include <com/Network.h>
#include <com/Software.h>
#include <com/Thread.h>
#include <com/Verify.h>

#include "com/Media.h"
#include "com/Socket.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define SIZEOF_BUFFER       (10000) // Maximum buffer size

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Socket::Port    hostPort= 7015; // Host Port (For linux firewall)
static Socket::Addr    peerAddr;    // Peer Addr
static Socket::Port    peerPort;    // Peer Port

static char*           buffer;      // Transmission buffer
static char*           checker;     // Transmission buffer checker
static int             delay;       // TRUE iff using send delay
static int             sender;      // POSITIVE iff sender, NEGATIVE iff receiver
static int             verbose;     // Verbosity, range 0..9

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Diagnostic exit.
//
//----------------------------------------------------------------------------
static void                         // (DOES NOT RETURN)
   info( void )                     // Informational exit
{
   fprintf(stderr, "Usage: %s {receive | send hostname:hostport}\n"
                   " [-V:verbosity]\n"
                 , __FILE__
          );
   fprintf(stderr, " -V (Diagnostic verbosity)\n");

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
     int               argc,
     char*             argv[])
{
   int                 error= FALSE;// Error encountered indicator
   int                 argi;        // Argument index
   char*               argp;        // Argument pointer

   char*               name;        // Host name
   char                string[128]; // Working string

   int                 i;

   //-------------------------------------------------------------------------
   // Parameter defaults
   //-------------------------------------------------------------------------
   delay= 0;
   sender= 0;
   verbose=      1;
   #ifdef HCDM
     verbose=    9;
   #endif

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   if( argc < 2 )
     info();

   for(argi=1; argi<argc; argi++)
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If switch parameter
     {
       argp++;                      // Skip the switch parameter
       if( *argp == 'V' )           // If verbosity
       {
         argp++;
         if( *argp != ':' )
         {
           fprintf(stderr, "Invalid parameter(%s)\n", argv[argi]);
           error= TRUE;
         }
         else
         {
           argp++;
           verbose= atoi(argp);
         }
       }
       else                         // Undefined parameter
       {
         fprintf(stderr, "Undefined parameter(%s)\n", argv[argi]);
         error= TRUE;
       }
     }
     else                           // If positional parameter
     {
       if( stricmp(argp, "receive") == 0 )
         sender= (-1);
       else if( stricmp(argp, "send") == 0 )
         sender= (+1);
       else if( stricmp(argp, "send_delay") == 0 )
       {
         delay= TRUE;
         sender= (+1);
       }
       else if( strlen(argp) >= sizeof(string) )
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter(%s) too long\n", argp);
       }
       else
       {
         strcpy(string, argp);
         for(i= 0; string[i] != ':'; i++)
         {
           if( string[i] == '\0' )
           {
             fprintf(stderr, "Invalid parameter(%s)\n", argp);
             error= TRUE;
             break;
           }
         }

         if( string[i] == ':' )
         {
           string[i]= '\0';
           peerAddr= Socket::nameToAddr(string);
           peerPort= atoi(&string[i+1]);
         }
       }
     }
   }

   //-------------------------------------------------------------------------
   // Consistency checks
   //-------------------------------------------------------------------------
   if( sender == 0 || (sender > 0 && (peerAddr == 0 || peerPort == 0)) )
     error= TRUE;

   if( error )
     info();

   buffer= (char*)malloc(SIZEOF_BUFFER+8);
   checker= (char*)malloc(SIZEOF_BUFFER+8);
   if( buffer == NULL || checker == NULL )
   {
     fprintf(stderr, "Storage shortage\n");
     exit(EXIT_FAILURE);
   }

   if( sender > 0 )
     Debug::set(new Debug("debug.s"));
   else
     Debug::set(new Debug("debug.r"));
debugSetIntensiveMode();

   //-------------------------------------------------------------------------
   // Information display
   //-------------------------------------------------------------------------
   debugf("%10s = %s %s %s\n", "Version", __DATE__, __TIME__, __FILE__);
   debugf("%10d = -V Verbosity\n", verbose);
   for(i= 0; ; i++)
   {
     name= (char*)Socket::getName(i);
     if( name == NULL )
       break;

     debugf("      Host = [%2d] %s %s\n", i, name,
            Socket::addrToChar(Socket::getAddr(i)));
   }

   if( sender > 0 )
     debugf("      Peer = %s:%d\n", Socket::addrToChar(peerAddr), peerPort);

   debugf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dgramRecv
//
// Purpose-
//       Test Datagram Socket recv.
//
//----------------------------------------------------------------------------
static void
   dgramRecv( void )                // Test datagram recv
{
   Socket              base(Socket::ST_DGRAM);
   SockSelect          select;
   Socket*             talk= &base;

   int                 L;

   int                 i;

   debugf("%4d Datagram port(%d)\n", __LINE__, hostPort);
   if( !verify(talk->setHost(0, hostPort) == 0) )
   {
     debugf("%4d setHostPort failure(%s)\n", __LINE__, talk->getSocketEI());
     return;
   }

   select.insert(talk);

   for(i= 0; i<256; i++)
     checker[i]= i;

   for(i= 0; i<256; i++)
   {
     talk= select.selectInp(5000);
     if( talk == NULL )
       break;

     memset(buffer, 0, 256);
     L= talk->recv(buffer, 256);
     if( !verify(L == 256) )
     {
       debugf("Receive length error\n");
       break;
     }
     if( !verify(memcmp(buffer, checker, 256) == 0) )
     {
       debugf("Receive data error\n");
       break;
     }
   }

   debugf("%4d Received(%d) of (%d)\n", __LINE__, i, 256);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       streamRecv
//
// Purpose-
//       Test Stream Socket recv.
//
//----------------------------------------------------------------------------
static void
   streamRecv( void )               // Test stream recv
{
   Socket              listen(Socket::ST_STREAM);
   Socket*             talk;

   int                 L;
   int                 O;

   int                 i;

   debugf("%4d Listening port(%d)\n", __LINE__, hostPort);
   talk= listen.listen(hostPort);
   debugf("%4d %s:%d stream ", __LINE__, talk->getHostName(), (int)talk->getHostPort());
   debugf("connected to %s:%d\n", talk->getPeerName(), (int)talk->getPeerPort());

   for(i= 0; i<256; i++)
     checker[i]= i;

   for(i= 0; i<256; i++)
   {
     O= 0;
     memset(buffer, 0, 256);
     while( O < 256 )
     {
       L= talk->recv(buffer+O, 256-O);
       if( L < 0 )
       {
         verify("Receive error");
         debugf("Receive error(%d) (%s)\n",
                talk->getSocketEC(), talk->getSocketEI());
         break;
       }

       O += L;
     }

     if( !verify(memcmp(buffer, checker, 256) == 0) )
     {
       debugf("Receive data error\n");
       break;
     }
   }

   delete talk;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       mediaRecv
//
// Purpose-
//       Test Media Socket read.
//
//----------------------------------------------------------------------------
static void
   mediaRecv( void )                // Test media recv
{
   Socket              listen(Socket::ST_STREAM);
   Socket*             talk;
   SockMedia           media;

   int                 L;
   int                 O;

   int                 i;

   debugf("%4d Listening port(%d)\n", __LINE__, hostPort);
   talk= listen.listen(hostPort);
   media.setSocket(talk);
   media.open(NULL, NULL);
   debugf("%4d %s:%d *media ", __LINE__, talk->getHostName(), (int)talk->getHostPort());
   debugf("connected to %s:%d\n", talk->getPeerName(), (int)talk->getPeerPort());

   for(i= 0; i<256; i++)
     checker[i]= i;

   for(i= 0; i<256; i++)
   {
     O= 0;
     memset(buffer, 0, 256);
     while( O < 256 )
     {
       L= media.read(buffer+O, 256-O);
       if( L < 0 )
       {
         verify("Receive error");
         debugf("Receive error(%d) (%s)\n",
                talk->getSocketEC(), talk->getSocketEI());
         break;
       }

       O += L;
     }

     if( !verify(memcmp(buffer, checker, 256) == 0) )
     {
       debugf("Receive data error\n");
       break;
     }
   }

   media.close();
   delete talk;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dgramSend
//
// Purpose-
//       Test Datagram Socket send.
//
//----------------------------------------------------------------------------
static void
   dgramSend( void )                // Test send
{
   Socket*             talk;
   int                 L;

   int                 i;

   debugf("%4d TCP/IP   Datagram(%s:%d) %s\n", __LINE__,
          Socket::addrToName(peerAddr), peerPort,
          Socket::addrToChar(peerAddr));

   talk= new Socket(Socket::ST_DGRAM);
   if( !verify(talk->setHost() == 0) )
   {
     debugf("%4d setHost failure(%s)\n", __LINE__, talk->getSocketEI());
     delete talk;
     return;
   }
   talk->setPeer(peerAddr, peerPort);

   for(i= 0; i<256; i++)
     buffer[i]= i;

   for(i= 0; i<256; i++)
   {
     L= talk->send(buffer, 256);
     verify( L == 256 );
   }

   Thread::sleep(1);
   delete talk;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       streamSend
//
// Purpose-
//       Test Stream Socket send.
//
//----------------------------------------------------------------------------
static void
   streamSend( void )               // Test send
{
   Socket*             talk;
   int                 L;

   int                 i;

   debugf("%4d stream Connecting(%s:%d) %s\n", __LINE__,
          Socket::addrToName(peerAddr), peerPort,
          Socket::addrToChar(peerAddr));

   talk= new Socket(Socket::ST_STREAM);
   if( !verify(talk->connect(peerAddr, peerPort) == 0) )
   {
     debugf("%4d Connect failure(%s)\n", __LINE__, talk->getSocketEI());
     delete talk;
     return;
   }

   for(i= 0; i<256; i++)
     buffer[i]= i;

   for(i= 0; i<256; i++)
   {
     if( i == 101 && delay )
     {
       debugf("%4d delay...\n", __LINE__);
       Thread::sleep(60.0);
       debugf("%4d ...delay\n", __LINE__);
     }

     L= talk->send(buffer, 256);
     verify( L == 256 );
   }

   Thread::sleep(1);
   delete talk;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       mediaSend
//
// Purpose-
//       Test Media Socket send.
//
//----------------------------------------------------------------------------
static void
   mediaSend( void )                // Test send
{
   SockMedia           media;
   Socket*             talk;
   int                 L;

   int                 i;

   debugf("%4d *media Connecting(%s:%d) %s\n", __LINE__,
          Socket::addrToName(peerAddr), peerPort,
          Socket::addrToChar(peerAddr));

   talk= new Socket(Socket::ST_STREAM);
   if( !verify(talk->connect(peerAddr, peerPort) == 0) )
   {
     debugf("%4d Connect failure(%s)\n", __LINE__, talk->getSocketEI());
     delete talk;
     return;
   }
   media.setSocket(talk);
   media.open(NULL, NULL);

   for(i= 0; i<256; i++)
     buffer[i]= i;

   for(i= 0; i<256; i++)
   {
     L= talk->send(buffer, 256);
     verify( L == 256 );
   }

   Thread::sleep(1);
   media.close();
   delete talk;
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
int
   main(
     int               argc,
     char*             argv[])
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis

   //-------------------------------------------------------------------------
   // Run the test
   //-------------------------------------------------------------------------
   if( sender > 0 )
   {
     streamSend();
     Thread::sleep(1);
     dgramSend();
     Thread::sleep(5);
     mediaSend();
   }
   else
   {
     streamRecv();
     Thread::sleep(1);
     dgramRecv();
     Thread::sleep(1);
     mediaRecv();
   }

   verify_exit();
}

