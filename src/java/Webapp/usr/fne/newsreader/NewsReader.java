//----------------------------------------------------------------------------
//
//       Copyright (C) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NewsReader.java
//
// Purpose-
//       RFC 977 Network News Transfer Protocol (NNTP) reader.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
package usr.fne.newsreader;

import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

import usr.fne.common.*;

public class NewsReader implements LoggingService {
//----------------------------------------------------------------------------
// NewsReader.Attributes
//----------------------------------------------------------------------------
// Debugging controls
protected boolean      debug;       // Debugging control
protected LoggingService
                       logger;      // Our logger
protected int          verbose;     // Verbosity control

// News server controls
protected String       hostName;    // Target host
protected int          hostPort;    // Target port number

protected Socket       talk;        // Talk Socket
protected BufferedInputStream
                       reader;      // Socket input stream
protected BufferedOutputStream
                       writer;      // Socket output stream

// Properties
protected long         interval;    // Client delay interval, in seconds
protected String       dbHome;        // Database home directory
protected String       articleDbFile; // Article file name
protected String       allGroupsFile; // ALL groups filename
protected int          maxArticles;   // Maximum number of articles to read
protected String       subGroupsFile; // SUBscribed groups filename
protected boolean      subGroupsZero; // Reset subgroups?

// Database controls
protected BufferedOutputStream
                       articleWriter; // Article Writer
protected GroupList    allGroupsList; // ALL groups database
protected GroupList    subGroupsList; // SUBscribed groups database
protected HeaderList   headerList;    // List of articles

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.NewsReader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   NewsReader(                      // Constructor
     LoggingService    logger,      // Log control
     Properties        props)       // Properties
   throws Exception
{
   String              string;

   // Initialize for debugging
   this.logger=  logger;
   this.debug=   logger.getDebug();
   this.verbose= logger.getVerbose();

   if( debug ) log("NewsReader()..");

   // Initialize controls
   String dbPath= props.getProperty("database-path",".");
   hostName= props.getProperty("news-server-name","127.0.0.1");
   hostPort= Integer.parseInt(props.getProperty("news-server-port","119"));
   interval= Integer.parseInt(props.getProperty("refresh-interval","1800"));
   articleDbFile= dbPath + "/" +
                  props.getProperty("news-server-file","article.db");
   allGroupsFile= dbPath + "/" +
                  props.getProperty("news-groups-file","groups.all");
   maxArticles=   Integer.parseInt(props.getProperty("max-article-count","0"));
   subGroupsFile= dbPath + "/" +
                  props.getProperty("subs-groups-file","groups.sub");
   subGroupsZero= Boolean.parseBoolean(
                  props.getProperty("subs-groups-zero","false"));
   headerList= new HeaderList();

   // Debugging
   if( debug )
   {
     log("      Debug: " + debug);
     log("    Verbose: " + verbose);
     log("     Server: " + hostName + ":" + hostPort);
     log("    DB Path: " + dbPath);
     log(" Article DB: " + articleDbFile);
     log("AllGroupsDB: " + allGroupsFile);
     log("SubGroupsDB: " + subGroupsFile);
     log("       Date: " + new Date());

     if( subGroupsZero )
       log(" ResetGroup: " + subGroupsZero);
   }

   // Validate
   if( articleDbFile.equalsIgnoreCase(allGroupsFile)
       || articleDbFile.equalsIgnoreCase(subGroupsFile)
       || allGroupsFile.equalsIgnoreCase(subGroupsFile) )
     throw new Exception("Filenames are not unique");

   if( debug ) log("..NewsReader()");
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.LoggingService
//
// Purpose-
//       Implement LoggingService methods.
//
//----------------------------------------------------------------------------
public boolean                      // The DEBUG attribute
   getDebug( )                      // Get DEBUG attribute
{
   return debug;
}

public int                          // The VERBOSE attribute
   getVerbose( )                    // Get VERBOSE attribute
{
   return verbose;
}

public void
   log(                             // Write log message
     String            string)      // The log message
{
   logger.log("NewsReader: " + string);
}

public void
   log(                             // Write log message, Exception
     String            string,      // The log message
     Throwable         e)           // The Exception
{
   logger.log("NewsReader: " + string, e);
}

public void
   err(                             // Write error message
     String            string)      // The error messsage
{
   System.err.println("NewsReader: " + string);
   log(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.Accessors
//
// Purpose-
//       Attribute accessors.
//
//----------------------------------------------------------------------------
public long                         // The client delay interval, milliseconds
   getInterval( )                   // Get client delay interval
{
   return interval * 1000;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.init
//
// Purpose-
//       Initialize the NewsReader.
//
//----------------------------------------------------------------------------
public void
   init( )                          // Initialize the NewsReader
   throws Exception
{
   if( debug ) log("init()..");

   loadAllGroups(allGroupsFile);
   loadSubGroups(subGroupsFile);
   loadDatabase(articleDbFile);

   if( debug ) log("..init()");
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.term
//
// Purpose-
//       Terminate the NewsReader.
//
//----------------------------------------------------------------------------
public void
   term( )                          // Terminate the NewsReader
{
   if( debug ) log("term()..");

   // Update the group databases
   try {
     if( allGroupsList.getChanged() )
       allGroupsList.write(allGroupsFile);
   } catch( Exception e ) {
     e.printStackTrace();
   }

   try {
     if( subGroupsList.getChanged() )
       subGroupsList.write(subGroupsFile);
   } catch( Exception e ) {
     e.printStackTrace();
   }

   if( false )
     headerList.debug();

   if( debug ) log("..term()");
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.send
//
// Purpose-
//       Send message on Socket.
//
//----------------------------------------------------------------------------
protected void
   send(
     String            message)     // The message
   throws Exception
{
   int                 length= message.length();

   for(int i= 0; i<length; i++)
     writer.write((int)message.charAt(i));
   writer.write('\n');
   writer.flush();

   if( verbose > 5 ) log("send: '" + message + "'");
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.receive
//
// Purpose-
//       Receive message answer on Socket.
//
//----------------------------------------------------------------------------
protected String                    // The response message
   receive( )
   throws Exception
{
   StringBuffer        buffer= new StringBuffer();
   String              result= null;

   for(;;)
   {
     int C= reader.read();
     if( C < 0 )
     {
       if( buffer.length() > 0 )
         result= buffer.toString();

       break;
     }

     if( C == '\n' )
     {
       result= buffer.toString();
       break;
     }

     if( C != '\r' )
       buffer.append((char)C);
   }

   if( verbose > 5 ) log("recv: '" + result + "'");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.drain
//
// Purpose-
//       Read, discarding any available data.
//
//----------------------------------------------------------------------------
protected synchronized void
   drain(                           // Read all available data
     long              delay)       // Drain delay, milliseconds
{
   if( verbose > 1 ) log("drain()..");

   try {
     for(;;)                        // Drain the buffer
     {
       if( reader.available() == 0 )
       {
         Thread.sleep(delay);
         if( reader.available() == 0 )
           break;
       }

       int C= reader.read();
       if( C < 0 )
         break;
     }
   } catch( Exception e ) {
     log("drain", e);
   }

   if( verbose > 1 ) log("..drain()");
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.formatDate
//
// Purpose-
//       Convert a date into a NNTP date and time
//
//----------------------------------------------------------------------------
public static String                // Resultant
   formatDate(                      // Format Date
     Date              date)        // The Date
{
   SimpleDateFormat df= new SimpleDateFormat("yyMMdd HH:mm:ss z");
   df.setTimeZone(TimeZone.getTimeZone("GMT"));

   return df.format(date);
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.parseDate
//
// Purpose-
//       Parse a date from a NNTP date and time
//
//----------------------------------------------------------------------------
public static Date                  // Resultant
   parseDate(                       // Format Date
     String            date)        // The Date
   throws ParseException
{
   SimpleDateFormat df= new SimpleDateFormat("yyMMdd HH:mm:ss z");
   df.setTimeZone(TimeZone.getTimeZone("GMT"));

   return df.parse(date);
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.open
//
// Purpose-
//       Create the connection.
//
//----------------------------------------------------------------------------
protected synchronized void
   open( )
   throws Exception
{
   if( verbose > 1 ) log("open()..");

   // Create the FileWriter
   try {
     articleWriter= new BufferedOutputStream(
                    new FileOutputStream(articleDbFile,true));
   } catch(Exception x) {
     log("Cannot open: " + articleDbFile, x);
     throw x;
   }

   // Create the connection
   if( verbose > 5 ) log("open: creating socket");
   try {
     talk= new Socket(hostName, hostPort);
     reader= new BufferedInputStream(talk.getInputStream());
     writer= new BufferedOutputStream(talk.getOutputStream());

     if( verbose > 5 ) log("open: first receive");
     receive();                       // Ignore the connection reponse
     if( verbose > 5 ) log("(recv ignored)");
   } catch( Exception x ) {
     log("Unable to connect: " + hostName + ":" + hostPort, x);
     throw x;
   }

   if( verbose > 1 ) log("..open()");
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.close
//
// Purpose-
//       Terminate the connection.
//
//----------------------------------------------------------------------------
protected synchronized void
   close( )
   throws Exception
{
   if( verbose > 1 ) log("close()");

   // Terminate the connection
   if( talk != null )
   {
     drain(1500);
     send("QUIT");
     drain(1500);

     reader.close();
     writer.close();
     talk.close();
     reader= null;
     writer= null;
     talk= null;
   }

   // Close the FileWriter
   if( articleWriter != null )
   {
     articleWriter.close();
     articleWriter= null;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.loadAllGroups
//
// Purpose-
//       Load the GroupList.
//
//----------------------------------------------------------------------------
protected void
   loadAllGroups(                   // Load the GroupList
     String            fileName)    // GroupList database name
   throws Exception
{
   GroupList           list;        // Complete GroupList

   if( verbose > 1 ) log("loadAllGroups("+fileName+")");

   list= null;
   try {
     list= GroupList.read(fileName);// Load the GroupList
   } catch( FileNotFoundException e ) {
     err("loadAllGroups: FileNotFound: " + fileName);
   } catch( Exception e ) {
     log("loadAllGroups: Exception: " + fileName, e);
   }

   if( list == null )               // If not read
   {
     list= new GroupList();

     open();
     send("LIST");
     String string= receive();
     if( !string.substring(0,3).equals("215")  )
     {
       close();
       throw new Exception("LIST: bad Response: " + string);
     }

     for(;;)
     {
       string= receive();
       if( string.equals(".") )
         break;

       list.put(new NewsGroup(string));
     }
     close();

     try {
       list.write(fileName);
     } catch(Exception e ) {
       log("loadAllGroups: write", e);
     }
   }

   allGroupsList= list;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.loadSubGroups
//
// Purpose-
//       Load the subscribed GroupList.
//
//----------------------------------------------------------------------------
protected void
   loadSubGroups(                   // Load the subscription GroupList
     String            fileName)    // GroupList database name
   throws Exception
{
   GroupList           list;        // Complete GroupList

   if( verbose > 1 ) log("loadSubGroups("+fileName+")");

   list= null;
   try {
     list= GroupList.read(fileName);// Load the GroupList
   } catch( FileNotFoundException e ) {
     err("loadSubGroups: FileNotFound: " + fileName);
   } catch(Exception e ) {
     log("loadSubGroups: Exception: " + fileName, e);
   }

   if( list == null )               // If not read
   {
     log("loadSubGroups: Cannot load: " + fileName);
     list= new GroupList();         // Create empty list
   }
   subGroupsList= list;

   if( subGroupsZero )
   {
     Iterator itor= subGroupsList.iterator();
     while( itor.hasNext() )
     {
       NewsGroup group= (NewsGroup)itor.next();
       group.last= 0;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.loadDatabase
//
// Purpose-
//       Load the database.
//
//----------------------------------------------------------------------------
protected void
   loadDatabase(                    // Load the database
     String            fileName)    // Article database name
   throws Exception
{
   BufferedReader      reader;      // Input Reader
   String              string;      // Working string

   if( verbose > 1 ) log("loadDatabase()");

   try {
     reader= new BufferedReader(
             new FileReader(fileName)
             );
   } catch( FileNotFoundException e ) {
     err("loadDatabase: FileNotFound: " + fileName);
     return;
   } catch( Exception e ) {
     log("loadDatabase: Exception: " + fileName, e);
     return;
   }

   for(;;)
   {
     string= reader.readLine();
     if( string == null )
       break;

     NewsHeader header= NewsHeader.extract(string);
     if( header != null )
       headerList.put(header);
   }

   reader.close();
   if( false )
     headerList.debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.retrieveArticle
//
// Purpose-
//       Select an Article.
//
//----------------------------------------------------------------------------
protected void
   retrieveArticle( )               // Retrive an article
   throws Exception
{
   StringBuffer        buffer= new StringBuffer(); // Article buffer
   NewsHeader          header= null;// Article header
   String              string;      // Working String

   // Read the article
   send("ARTICLE");
   string= receive();
   if( string == null || !string.substring(0,3).equals("220")  )
   {
     if( string == null )
       string= "<null>";
     log("retrieveArticle: Response: '" + string + "'");
     return;
   }

   for(;;)
   {
     string= receive();
     if( string.length() == 1 && string.charAt(0) == '.' )
       break;

     if( header == null )
       header= NewsHeader.extract(string);

     buffer.append(string);
     buffer.append("\r\n");
   }

   // Accept/Reject the article
   if( header != null )
   {
     if( headerList.put(header) )
     {
       buffer.append("\r\n");
       buffer.append("\r\n");
       for(int i= 0; i<buffer.length(); i++)
         articleWriter.write((int)buffer.charAt(i));
     }
     else if( debug )
       log("retrieveArticle: Duplicate: " + header.getIdentifier());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.retrieveGroup
//
// Purpose-
//       Retrieve all new Articles in NewsGroup.
//
//----------------------------------------------------------------------------
protected void
   retrieveGroup(                   // Retrive a group
     NewsGroup         group)       // Group identifier
   throws Exception
{
   int                 last= group.getLast();
   String              string;
   StringTokenizer     st;

   send("GROUP " + group.getName());
   string= receive();
   if( string == null || !string.substring(0,3).equals("211")  )
   {
     if( string == null )
       string= "<null>";
     log("retrieveGroup: Response: '" + string + "'");
     return;
   }

   st= new StringTokenizer(string);
   st.nextToken();                // Skip response code (211)
   st.nextToken();                // Skip number of articles
   int lo= Integer.parseInt(st.nextToken()); // Get first article number
   int hi= Integer.parseInt(st.nextToken()); // Get last article number
   if( verbose > 1 )
     log(    "Group(" + group.getName()
         + ") Last("  + last
         + ") LO("    + lo
         + ") HI("    + hi
         + ")");

   if( hi > last )
   {
     if( maxArticles > 0 )
     {
       if( lo <= (hi - maxArticles) )
         lo= (hi - maxArticles) + 1;

       if( verbose > 1 )
         log("max-article-count(" + maxArticles + "), LO(" + lo + ")");
     }

     if( lo <= last )
       lo= last+1;

     // Access the first article
     while( lo <= hi )
     {
       send("STAT " + lo);
       string= receive();
       if( string == null || string.substring(0,3).equals("223") )
         break;

       lo++;
     }

     // Access the articles
     while( lo <= hi )
     {
       if( string == null || !string.substring(0,3).equals("223") )
         break;

       st= new StringTokenizer(string);
       st.nextToken();              // Skip response code (223)
       lo= Integer.parseInt(st.nextToken()); // Get article number
       string= st.nextToken();      // Get article identifier
       if( !headerList.contains(string) )
         retrieveArticle();
       else if( debug )
         log("retrieveGroup: Duplicate: " + string);

       group.setLast(lo);
       subGroupsList.setChanged(true);
       System.gc();
       send("NEXT");
       string= receive();
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsReader.client
//
// Purpose-
//       Client.
//
//----------------------------------------------------------------------------
public void
   client( )                        // Update all NewsGroups
   throws Exception
{
   Iterator            itor;        // Group Iterator
   NewsGroup           group;       // Current NewsGroup

   if( debug ) log("client()");

   // Update each NewsGroup
   group= null;
   itor= subGroupsList.iterator();
   while( itor.hasNext() )
   {
     if( talk == null )
       open();

     group= (NewsGroup)itor.next();
     try {
       retrieveGroup(group);
     } catch( Exception e ) {
       log("client: Group: " + group.getName(), e);
     }
   }
   close();

   // If nothing done, log that
   if( group == null )
   {
     log("client: No subscriptions");
     return;
   }
}
} // Class NewsReader

