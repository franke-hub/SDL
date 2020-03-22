//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Middle.java
//
// Purpose-
//       Man-in-the-middle server. Reads from one socket, writes to another.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.util.*;

import user.util.logging.*;

public class Middle extends Thread
{
//----------------------------------------------------------------------------
// Middle.Attributes
//----------------------------------------------------------------------------
static int             BUFF_SIZE= 0x00100000; // Buffer size
StreamLogger           logger;      // Logger
String                 prefix;      // Message prefix

Socket                 inp;         // Input Socket
Socket                 out;         // Output Socket

//----------------------------------------------------------------------------
//
// Method-
//       Middle.Middle
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Middle(                          // Constructor
     StreamLogger      logger,      // The logger
     String            prefix,      // Message prefix
     Socket            inp,         // The input Socket
     Socket            out)         // The output Socket
   throws Exception
{
   this.logger= logger;
   this.prefix= prefix;

   this.inp= inp;
   this.out= out;
}

//----------------------------------------------------------------------------
//
// Method-
//       Middle.debugf
//
// Purpose-
//       Debugging printf.
//
//----------------------------------------------------------------------------
public void
   debugf(
     String            string)
{
   logger.log(prefix + string);
   System.out.println(prefix + string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Middle.outline
//
// Purpose-
//       Write output line.
//
//----------------------------------------------------------------------------
public void
   outline(
     StringBuffer      disp)
{
   for(int i= 0; i<disp.length(); i++)
   {
     int C= (int)disp.charAt(i) & 0x00ff;
     if( C < 0x20 || C > 0x7f )
     {
       byte head[]= new byte[4];
       for(int x= 0; x<4; x++)
       {
         head[x]= 0;
         if( x < disp.length() )
           head[x]= (byte)disp.charAt(i);
       }

       disp= new StringBuffer(
                 String.format("L(%6d) ... [%6d] 0x%4x",
                               disp.length(), i, (byte)C));
       break;
     }
   }

   debugf(disp.toString());
}

//----------------------------------------------------------------------------
//
// Method-
//       Middle.run
//
// Purpose-
//       Run the Middle thread
//
//----------------------------------------------------------------------------
public void
   run( )
{
   byte                buff[];      // Character buffer
   InputStream         reader= null;// Socket input stream
   OutputStream        writer= null;// Socket output stream

   try {
     buff= new byte[BUFF_SIZE] ;    // Transfer buffer
     reader= inp.getInputStream();
     writer= out.getOutputStream();
     debugf("Running: "
          + "I(" + inp.getLocalAddress() + ":" + inp.getLocalPort() +  ") "
          + "O(" + out.getInetAddress()  + ":" + out.getLocalPort() + ")");

     for(;;)
     {
       int size= reader.read(buff);
       if( size < 0 )
         break;

       if( size == 0 )
         debugf("size == 0");

       if( size > 0 )
       {
         writer.write(buff, 0, size);
         writer.flush();
         StringBuffer disp= new StringBuffer();
         for(int offset= 0; offset<size; offset++)
         {
           char C= (char)(buff[offset]&0x00ff);
           if( C == '\n' )
           {
             disp.append("\\n");
             outline(disp);
             disp= new StringBuffer();
           }
           else if( C == '\r' )
             disp.append("\\r");
           else if( C == '\t' )
             disp.append("\\t");
           else
             disp.append(C);
         }

         if( disp.length() > 0 )
           outline(disp);
       }
     }
   } catch(Exception e) {
     debugf("Exception: " + e);
   }

   try {
     reader.close();
   } catch(Exception e) {
   }

   try {
     writer.close();
   } catch(Exception e) {
   }
}
} // Class Middle

