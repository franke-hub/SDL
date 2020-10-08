//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Diagnostic.cpp
//
// Purpose-
//       Diagnostic implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <com/Socket.h>
#include <com/Software.h>

#include "Common.h"
#include "Diagnostic.h"

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

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       Diagnostic::httpTrace
//
// Purpose-
//       Trace HTTP request/response.
//
//----------------------------------------------------------------------------
void
   Diagnostic::httpTrace(           // Trace HTTP request/response
     const char*       prefix,      // Request/Response prefix
     const char*       buffer,      // Buffer address
     unsigned          length)      // Buffer length
{
   unsigned            offset;      // Working buffer offset
   int                 C;           // Current character

   for(offset= 0; offset<length; offset++)
   {
     C= buffer[offset] & 0x000000ff;
     if( C == '\t' || C == '\r' || C == '\n' )
       continue;

     if( C < 0x20 || C > 0x7f )
     {
       logf("%s (Not ASCII)\n", prefix);
       dumpv(buffer, length, 0);
       return;
     }
   }

   C= 0;
   char temp[256];
   int X= 0;
   for(offset= 0; offset < length; offset++)
   {
     if( size_t(X) >= (sizeof(temp)-2) )
     {
       temp[sizeof(temp)-1]= '\0';
       logf("%s %s...[%u]\n", prefix, temp, length);
       while( offset < length && buffer[offset] != '\n' )
         offset++;

       X= 0;
       continue;
     }

     C= buffer[offset] & 0x000000ff;
     switch(C)
     {
       case '\t':
         temp[X++]= '\\';
         temp[X++]= 't';
         break;

       case '\r':
         temp[X++]= '\\';
         temp[X++]= 'r';
         break;

       case '\n':
         temp[X++]= '\\';
         temp[X++]= 'n';
         temp[X]= '\0';
         logf("%s %s\n", prefix, temp);
         X= 0;
         break;

       default:
         temp[X++]= C;
         break;
     }
   }

   if( X != 0 || C == 0 )
   {
     temp[X]= '\0';
     logf("%s %s\n", prefix, temp);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Diagnostic::recv
//
// Purpose-
//       Receive Socket message.
//
//----------------------------------------------------------------------------
int                                 // Socket.recv() return code
   Diagnostic::recv(                // Receive Socket message
     Socket&           socket,      // The socket
     char*             addr,        // The message address
     int               size,        // The message length
     Socket::SocketMO  opts)        // Options
{
   int                 rc;          // Resultant

   IFIODM( socket.setSocketEC(0); )

   rc= socket.recv(addr, size, opts);

   IFIODM(
     if( socket.getSocketEC() != 0 )
       logf("Diagnostic.recv Error(%d) %s\n",
            (int)socket.getSocketEC(), socket.getSocketEI());
     else
       httpTrace("<<<INP<<<", addr, rc < 0 ? 0 : rc);
   )

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       recvLine
//
// Purpose-
//       Receive Socket message line.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Diagnostic::recvLine(            // Receive Socket message
     Socket&           socket,      // The socket
     char*             buffer,      // Result buffer
     unsigned          size,        // Result buffer size
     unsigned          timeout)     // Timeout (milliseconds)
{
   int                 result= 0;   // Resultant
   int                 L;           // Receive length
   unsigned            x;           // Buffer index

   x= 0;
   for(;;)
   {
     IFIODM( socket.setSocketEC(0); )
     L= socket.recv(buffer + x, 1, Socket::MO_NONBLOCK);
     if( L != 1 )
     {
       Socket::SocketEC ec= socket.getSocketEC();
       if( ec == Software::EC_WOULDBLOCK || ec == Software::EC_AGAIN )
       {
         IFIODM( logf("Diagnostic.recvLine %u\n", timeout); )

         SockSelect select;
         select.insert(&socket);
         if( select.selectInp(timeout) != NULL )
           continue;
       }

       result= (-1);
       break;
     }

     if( buffer[x] == '\n' || buffer[x] == '\0' )
       break;

     if( buffer[x] != '\r' )
     {
       if( x >= (size-1) )
         result= (-1);
       else
         x++;
     }
   }
   buffer[x]= '\0';

   IFIODM(
     if( socket.getSocketEC() != 0 )
       logf("Diagnostic.recvLine Error(%d) %s\n",
            (int)socket.getSocketEC(), socket.getSocketEI());
     else
       httpTrace("<<<INP<<<", buffer, strlen(buffer));
   )

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Diagnostic::send
//
// Purpose-
//       Send Socket message.
//
//----------------------------------------------------------------------------
int                                 // Socket.send() return code
   Diagnostic::send(                // Send Socket message
     Socket&           socket,      // The socket
     const char*       addr,        // The message address
     int               size,        // The message length
     Socket::SocketMO  opts)        // Options
{
   int                 rc;          // Resultant

   IFIODM( socket.setSocketEC(0); )

   rc= socket.send(addr, size, opts);
   IFIODM(
     httpTrace(">>>OUT>>>", addr, size);
     if( socket.getSocketEC() != 0 )
       logf("Diagnostic.send Error(%d) %s\n",
            (int)socket.getSocketEC(), socket.getSocketEI());

   )

   return rc;
}

