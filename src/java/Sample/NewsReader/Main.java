//----------------------------------------------------------------------------
//
//       Copyright (C) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.java
//
// Purpose-
//       Java News Reader.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.*;

import user.util.Debug;

public class Main extends Debug {
//----------------------------------------------------------------------------
// Main.Configuration parameters
//----------------------------------------------------------------------------
static final String    fileName=      "article.out"; // Article file
static final boolean   bringup=       true;  // Bringup overrides?
static final int       DEFAULT_GROUP= 1057;  // Default NewsGroup, alt.0d
static final int       MAX_ARTICLE=   100;   // Maximum article count
static final boolean   verbose=       true;  // Talk verbose?

//----------------------------------------------------------------------------
// Main.Attributes
//----------------------------------------------------------------------------
protected String       hostName=    "news-server.hvc.rr.com"; // Target host
protected int          hostPort=    119;           // Target port number
protected int          hostGroup=   DEFAULT_GROUP; // Target NewsGroup
protected int          maxArticle=  MAX_ARTICLE;   // Maximum article count

protected Date         groupDate;   // Group date
protected Vector       groupVector; // Group Vector
protected NewsGroup    group;       // Selected group

protected Socket       talk;        // Talk Socket
protected BufferedReader
                       reader;      // Socket input stream
protected PrintWriter  writer;      // Socket output stream
protected PrintWriter  article;     // Article Writer

//----------------------------------------------------------------------------
//
// Method-
//       Main.Main
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Main(                            // Constructor
     String            args[])      // Argument list
   throws Exception
{
   if( args != null )
   {
     if( args.length > 0 )          // [0] groupName
       hostGroup= Integer.parseInt(args[0]);
   }

   // Test mode
   if( bringup )
   {
     hostName= "linux";             // Target host
     hostName= "192.168.1.254";     // Target host (linux)
     hostName= "127.0.0.1";         // Target host
     hostPort=  65025;              // Target port number

     HCDM= true;
     SCDM= true;
     INFO= true;
   }

   // Create the connection
   try {
     talk= new Socket(hostName, hostPort);
     reader= new BufferedReader(
             new InputStreamReader(talk.getInputStream())
                               );

     writer= new PrintWriter(
             new OutputStreamWriter(talk.getOutputStream())
                            );
   } catch(Exception e) {
     debugf("Cannot connect: " + hostName + ":" + hostPort + "\n");
     throw e;
   }
   throw new Exception("Bringup exception");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.send
//
// Purpose-
//       Send message on Socket.
//
//----------------------------------------------------------------------------
public void
   send(
     String            message)     // The message
   throws Exception
{
   writer.println(message);
   writer.flush();

   if( verbose )
     debugf("sent: '" + message + "'\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.receive
//
// Purpose-
//       Receive message answer on Socket.
//
//----------------------------------------------------------------------------
public String                       // The response message
   receive( )
   throws Exception
{
   String              result;      // Resultant

   result= reader.readLine();
   if( verbose )
     debugf("read: '" + result + "'\n");

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.drain
//
// Purpose-
//       Read, discarding any available data.
//
//----------------------------------------------------------------------------
public void
   drain(                           // Read all available data
     long              delay)       // Drain delay, milliseconds
{
   debugf("drain()..\n");

   try {
     for(;;)                        // Drain the buffer
     {
       if( !reader.ready() )
       {
         Thread.sleep(delay);
         if( !reader.ready() )
           break;
       }

       int C= reader.read();
       if( C < 0 )
         break;
     }
   } catch( Exception e ) {
     debugf("Main.drain\n");
     debugException(e);
   }

   debugf("..drain()\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.open
//
// Purpose-
//       Create the connection.
//
//----------------------------------------------------------------------------
public void
   open( )
   throws Exception
{
   // Create the FileWriter
   try {
     article= new PrintWriter(
              new BufferedWriter(
              new OutputStreamWriter(new FileOutputStream(fileName))
                             )  );
   } catch(Exception e) {
     debugf("Cannot open: " + fileName + "\n");
     throw e;
   }

   // Create the connection
   try {
     talk= new Socket(hostName, hostPort);
     reader= new BufferedReader(
             new InputStreamReader(talk.getInputStream())
                             );

     writer= new PrintWriter(
             new BufferedWriter(
             new OutputStreamWriter(talk.getOutputStream())
                          )  );
   } catch(Exception e) {
     debugf("Cannot connect: " + hostName + ":" + hostPort + "\n");
     throw e;
   }

   debugf(receive() + "\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.close
//
// Purpose-
//       Terminate the connection.
//
//----------------------------------------------------------------------------
public void
   close( )
   throws Exception
{
   debugf("Closing connections\n");

   // Terminate the connection
   if( talk != null )
   {
     drain(1500);
     send("QUIT");
     drain(1500);
     talk.close();
     talk= null;
   }

   // Close the FileWriter
   if( article != null )
   {
     article.close();
     article= null;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.getGroupVector
//
// Purpose-
//       Load the GROUP array.
//
//----------------------------------------------------------------------------
public void
   getGroupVector( )                // Load the GROUP array
   throws Exception
{
   NewsGroup         group;         // Working NewsGroup
   String            string;        // Working String

   try {
     groupVector= NewsGroup.read(); // Read the NewsGroup
     if( groupVector != null )      // If read
       return;
   } catch(Exception e ) {
   }

   groupVector= new Vector();       // Allocate the GROUP Vector

   send("LIST");                    // List command
   string= receive();
   if( !string.substring(0,3).equals("215")  )
     throw new Exception("LIST: bad Response: " + string);

   for(;;)                          // Load the newsgroup list
   {
     string= receive();
     if( string.equals(".") )
       break;

     group= new NewsGroup(string);
     groupVector.add(group);
   }

   try {
     NewsGroup.write(groupVector);
   } catch(Exception e ) {
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.retrieveArticle
//
// Purpose-
//       Select an Article.
//
//----------------------------------------------------------------------------
public void
   retrieveArticle(                 // Retrive an article
     String          ident)         // Article identifier
   throws Exception
{
   Vector            result;        // Resultant
   String            string;        // Working String

   string= "ARTICLE";
   if( ident != null )
     string= "ARTICLE " + ident;

   send(string);
   string= receive();
   if( !string.substring(0,3).equals("220")  )
   {
     debugf("ARTICLE: bad response: " + string + "\n");
     return;
   }

   for(;;)
   {
     string= receive();
     if( string.equals(".") )
       break;

     article.println(string);
   }
   article.write('\n');
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.nextArticle
//
// Purpose-
//       Position at next Article
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff next article exists
   nextArticle( )                   // Position at next Article
   throws Exception
{
   String            string;        // Working String

   if( maxArticle == 0 )
   {
     if( !verbose )
       debugf("\n");
     debugf("Max article count(" + MAX_ARTICLE + ") exceeded\n");
     return false;
   }
   maxArticle--;

   send("NEXT");
   string= receive();
   if( string.substring(0,3).equals("223")  )
   {
     if( !verbose )
       debugf(".");
     return true;
   }

   if( string.substring(0,3).equals("421")  )
     return false;

   debugf("NEXT: bad response: " + string + "\n");
   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.selectGroup
//
// Purpose-
//       Select a NewsGroup.
//
//----------------------------------------------------------------------------
public void
   selectGroup(                     // Select a group
     int             index)         // Group index
   throws Exception
{
   NewsGroup         group;         // Working NewsGroup
   String            string;        // Working String

   group= (NewsGroup)groupVector.elementAt(index-1);
   send("GROUP " + group.name);
   string= receive();

   if( !string.substring(0,3).equals("211")  )
     throw new Exception("GROUP: bad response: " + string);

   this.group= group;

   StringTokenizer st= new StringTokenizer(string);
   st.nextToken();                  // Skip response code (211)
   st.nextToken();                  // Skip message count
   group.first= Integer.parseInt(st.nextToken()); // First message number
   group.last=  Integer.parseInt(st.nextToken()); // Last message number

   if( HCDM )
     debugf("Selected: " + group.toString() + "\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.client
//
// Purpose-
//       Client.
//
//----------------------------------------------------------------------------
public void
   client( )
   throws Exception
{
   String              string;

   open();

   try {
     getGroupVector( );
     selectGroup(hostGroup);
     retrieveArticle("" + group.first);

     for(;;)
     {
       if( !nextArticle() )
         break;

       retrieveArticle(null);
     }


   } catch( Exception e ) {
     debugException(e);
   }

   debugf("\n");
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   Main                main= new Main(args); // Main Object
   String              string;      // Working String

   //-------------------------------------------------------------------------
   // Time of day message
   debugf("\n");
   debugf("       Host name: " + main.hostName + "\n");
   debugf("       Host port: " + main.hostPort + "\n");
   string= new SimpleDateFormat().format(new Date());
   debugf("      Start time: " + string + "\n");
   debugf("    Max articles: " + main.MAX_ARTICLE + "\n");

   //-------------------------------------------------------------------------
   // News reader
   main.client();
   debugf("Main complete\n");
   System.gc();
   System.runFinalization();
   System.gc();
}
} // Class Main

