//----------------------------------------------------------------------------
//
//       Copyright (C) 2010-2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbServer.java
//
// Purpose-
//       Golfer Database Server local interface.
//
// Last change date-
//       2013/01/01
//
// Usage-
//       dbServer= new DbServer(properties)
//
//       dbServer.start()
//       dbServer.reset()
//
//----------------------------------------------------------------------------
package usr.fne.golfer;

import java.io.*;
import java.net.*;
import java.lang.*;
import java.util.*;

import usr.fne.common.QuotedTokenizer;

class DbServer {
//----------------------------------------------------------------------------
// DbServer.Attributes
//----------------------------------------------------------------------------
static final boolean   BRINGUP= false; // Bringup?
static final boolean   DEBUG= false;// Debugging?

static DbServer        server;      // The singleton DbServer object
Listener               listen;      // Listener thread
int                    port;        // Listener port
Vector<DbRemote>       remote= new Vector<DbRemote>(); // Remote servers

String                 dataFile;    // The database file name
TreeMap<String,String> treemap;     // The database map

static final String    COMMENT= "##"; // Comment type
static final String    SEP="@";     // Separates type from item

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.Utilities
//
// Purpose-
//       Utility methods
//
//----------------------------------------------------------------------------
public void debug(String string) {
   if( DEBUG )
     System.out.println("DbServer: " + string);
}

public void error(String string) {
   System.err.println("DbServer: " + string);
}

public void print(String string) {
   System.out.println(string);
}

//----------------------------------------------------------------------------
//
// Class-
//       DbServer.Listener
//
// Purpose-
//       Create a background task to listen for remote connections.
//
//----------------------------------------------------------------------------
class Listener extends Thread
{
//----------------------------------------------------------------------------
// DbServer.Listener.Attributes
//----------------------------------------------------------------------------
int                    fsm= DbCommon.FSM_RESET;

ServerSocket           socket;      // The Listener Socket

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.Listener.Listener
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Listener( )                      // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.Listener.reset
//
// Purpose-
//       Reset the Listener
//
//----------------------------------------------------------------------------
public synchronized void
   reset()
{
   fsm= DbCommon.FSM_RESET;
   try {
     socket.close();
   } catch(Exception e) {
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.Listener.run
//
// Purpose-
//       Listen for remote connections.
//
//----------------------------------------------------------------------------
public void
   run()
{
   debug("Listener.run()");

   try {
     socket= new ServerSocket(port);
   } catch( IOException e ) {
     error("Listener: Error creating socket: " + e);
     fsm= DbCommon.FSM_ERROR;
     return;
   }

   fsm= DbCommon.FSM_READY;
   debug("Listener: ready: port: " + port);
   while( fsm == DbCommon.FSM_READY )
   {
     Socket talk= null;

     // Wait for client connection
     try {
       debug("Listener: Waiting: " + fsm);
       talk= socket.accept();
     } catch( IOException e ) {
       error("Listener: Accept failed: " + e);
       e.printStackTrace();
       continue;
     } catch( Exception e ) {
       if( fsm != DbCommon.FSM_RESET )
       {
         error("Listener: Exception: " + e);
         fsm= DbCommon.FSM_ERROR;
       }
       break;
     }

     // Create a Thread to handle this connection.
     try {
       DbRemote client= new DbRemote(talk, server);
       client.send(DbCommon.VERSION);
       addRemote(client);
       client.start();
     } catch (Exception e) {
       error("Listener: Task creation failure: " + e);
       e.printStackTrace();
     }

     // Check for abandoned connections.
     synchronized (server)
     {
       boolean found= true;
       while( found )
       {
         found= false;
         for(int i= 0; i<remote.size(); i++)
         {
           DbRemote client= remote.elementAt(i);
           if( client.fsm != DbCommon.FSM_READY )
           {
             found= true;
             remRemote(client);
             break;
           }
         }
       }
     }
   }
}
} // class DbServer.Listener

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.DbServer
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
protected
   DbServer(                        // Initialize the DbServer
     Properties        props)       // Using these properties
{
   treemap= new TreeMap<String,String>();

   port= Integer.parseInt(props.getProperty("database-port", ""+DbCommon.PORT));
   String path= props.getProperty("database-path", ".");
   String name= props.getProperty("database-name", "GOLFER.DB");
   dataFile= path + "/" + name;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.addRemote
//
// Purpose-
//       Add a DbRemote to the Vector.
//
//----------------------------------------------------------------------------
public synchronized void
   addRemote(                       // Add a remote client
     DbRemote          client)      // The remote client
{
   debug("addRemote(" + client + ")");

   remote.add(client);
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.remRemote
//
// Purpose-
//       Remove a DbRemote from the Vector.
//
//----------------------------------------------------------------------------
public synchronized void
   remRemote(                       // Remove a remote client
     DbRemote          client)      // The remote client
{
   debug("remRemote(" + client + ")");

   remote.remove(client);
   try {
     client.join(500);
   } catch( Exception e ) {
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.singleton
//
// Purpose-
//       Access the singleton DbServer object
//
//----------------------------------------------------------------------------
public static synchronized DbServer
   singleton(                       // Initialize the DbServer
     Properties        props)       // Using these properties
{
   if( server == null )
   {
     server= new DbServer(props);
     server.start();
   }

   return server;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.start
//
// Purpose-
//       Start the listener thread.
//
//----------------------------------------------------------------------------
public synchronized void
   start()
{
   debug("start()");

   reset();

   // Load the database
   treemap= new TreeMap<String,String>();
   int errors= 0;
   try {
     BufferedReader reader= new BufferedReader(
                            new FileReader(dataFile));
     for(;;)
     {
       String line= reader.readLine();
       if( line == null )
         break;

       if( line.length() == 0 )
         continue;

       if( line.charAt(0) == '#' )
         continue;

       QuotedTokenizer tokenizer= new QuotedTokenizer(line);
       if( !tokenizer.hasMoreTokens() )
         continue;

       String cmd= tokenizer.nextToken();
       if( cmd.charAt(0) == '#' )
         continue;

       if( !tokenizer.hasMoreTokens() )
       {
         debug("No item: '" + line + "'");
         continue;
       }

       String item= tokenizer.nextToken();
       String key= cmd + SEP + item;
       if( !tokenizer.hasMoreTokens() )
       {
         String prior= treemap.remove(key.toUpperCase());
         if( prior == null )
         {
           errors++;
           debug("Remove: key(" + key + ") No prior data");
         }
       }
       else
       {
         String data= tokenizer.remainder();
         String prior= treemap.put(key.toUpperCase(), data);
         if( prior != null )
         {
           errors++;
           debug("Duplicate: key(" + key + ") old(" + prior + ") new(" + data + ")");
         }
       }
     }
   } catch( Exception e ) {
     errors++;
     error("Database load failure: " + e);
     e.printStackTrace();
   }

   if( errors == 0 )
     debug("Database loaded, NO errors");
   else if( errors == 1 )
     debug("Database loaded, 1 error");
   else
     debug("Database loaded, " + errors + " errors");

   // Start the listener
   listen= new Listener();
   listen.start();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.reset
//
// Purpose-
//       Terminate the listener thread.
//
//----------------------------------------------------------------------------
public synchronized void
   reset()
{
   debug("reset()");

   if( listen != null )
   {
     listen.reset();
     try {
       listen.join(500);
     } catch( Exception e ) {
     }

     listen= null;
   }

   while( remote.size() > 0 )
   {
     DbRemote client= remote.elementAt(0);
     client.reset();
     remRemote(client);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.get
//
// Purpose-
//       Extract data from database.
//
//----------------------------------------------------------------------------
public synchronized String          // The database item
   get(                             // Get database item
     String            type,        // The item type
     String            item)        // The item name
{
   String inpKey= null;
   if( item == null )
     inpKey= type;
   else
     inpKey= type + SEP + item;
   String result= treemap.get(inpKey.toUpperCase());
   debug("'"+result+"'= get("+type+","+item+")");

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.next
//
// Purpose-
//       Extract next item data from database.
//
//----------------------------------------------------------------------------
public synchronized String          // The database item
   next(                            // Get database item
     String            type,        // The item type
     String            item)        // The item name
{
   String result= null;
   String inpKey= null;
   if( item == null )
     inpKey= type;
   else
     inpKey= type + SEP + item;
   Map.Entry<String,String> find= treemap.higherEntry(inpKey.toUpperCase());
   if( find != null )
   {
     String outKey= find.getKey();
     String outVal= find.getValue();
     int index= outKey.indexOf(SEP);
     if( index >= 0 )
       result= outKey.substring(0, index) + " "
             + outKey.substring(index+1) + " "
             + outVal;
   }

   debug("'"+result+"'= DbClient.next("+type+","+item+")");

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.put
//
// Purpose-
//       Insert data into database.
//
//----------------------------------------------------------------------------
public synchronized String          // The prior data
   put(                             // Put database item
     String            type,        // The item type
     String            item,        // The item name
     String            data)        // The associated data record
   throws IOException
{
   // Append to the dataFile
   PrintWriter writer= new PrintWriter(
                       new FileWriter(dataFile, true)); // Open for APPEND

   writer.println(type + " " + item + " " + data);
   writer.close();

   // Handle comment once it's in the database
   if( type == COMMENT )
     return null;

   // Append to the map
   String key= type + SEP + item;
   String result= treemap.put(key.toUpperCase(), data);
   if( result != null )
     debug("put: Duplicate: key(" + key + ") old(" + result + ") new(" + data + ")");

   debug("'"+result+"'= put("+type+","+item+","+data+")");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.remove
//
// Purpose-
//       Remove data from database.
//
//----------------------------------------------------------------------------
public synchronized String          // The prior data
   remove(                          // Get database item
     String            type,        // The item type
     String            item)        // The item name
   throws IOException
{
   // Remove from the dataFile
   PrintWriter writer= new PrintWriter(
                       new FileWriter(dataFile, true)); // Open for APPEND

   writer.println(type + " " + item);
   writer.close();

   // Remove from the map
   String inpKey= null;
   if( item == null )
     inpKey= type;
   else
     inpKey= type + SEP + item;
   String result= treemap.remove(inpKey.toUpperCase());
   debug("'"+result+"'= remove("+type+","+item+")");

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.bringup
//
// Purpose-
//       Run bringup tests.
//
//----------------------------------------------------------------------------
public static void
   bringup(                         // Bringup tests
     DbServer          server)      // DbServer
{
   // Debug NEXT
   if( false )
   {
     String type= " ";
     String item= " ";
     for(;;)
     {
       String next= server.next(type, item);
       if( next == null )
         break;

       QuotedTokenizer t= new QuotedTokenizer(next);
       type= t.nextToken();
       item= t.nextToken();

       System.out.println("type(" + type + ") item(" + item + ") data(" + t.remainder() + ")");
     }
   }

   // Debug PUT
   if( false )
   {
     try {
       String prior= server.put("DbServer.java", "TEST_INSERT", "Test data " + new Date());
       System.out.println("Prior: '" + prior + "'");
     } catch(IOException e ) {
       System.out.println("EXCEPTION: " + e);
       e.printStackTrace();
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbServer.main
//
// Purpose-
//       Utility to run stand-alone DbServer.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   // Get properties
   Properties props= new Properties();
   try {
     props.load(new FileInputStream("data/golfer.pro"));
   } catch( Exception e ) {
     System.err.println("Unable to load properties: " + e);
   }

   // Initialize the server
   DbServer server= DbServer.singleton(props);
   if( BRINGUP )
     bringup(server);

   // On keypress, terminate
   System.out.println("Server started, press ENTER to terminate");
   try {
     for(;;)
     {
       int in= System.in.read();
       if( in < 0 )
         break;

       System.out.write(in);
       if( in == '\n' )
         break;
     }
   } catch(Exception e) {
     System.err.println("Exception: " + e);
     e.printStackTrace();
   }

   server.reset();
}
} // class DbServer
