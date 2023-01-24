//----------------------------------------------------------------------------
//
//       Copyright (C) 2008-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbClient.java
//
// Purpose-
//       Golfer Database Server remote client.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.io.*;
import java.net.*;
import java.lang.*;
import java.util.*;

public class DbClient extends DbStatic {
//----------------------------------------------------------------------------
// DbClient.Attributes
//----------------------------------------------------------------------------
static final boolean   DEBUG= true; // DEBUG control

TreeMap<String,String> cache;       // Local cache
public int             fsm;         // The current state
public int             port;        // The connection port
public BufferedReader  reader;      // Socket Reader
public Socket          socket;      // Connection Socket
public PrintWriter     writer;      // Socket Writer

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.DbClient
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbClient(                        // Constructor
     String            host)        // Server host name
{
   debug("DbClient(" + host + ")");

   fsm= FSM_RESET;
   cache= new TreeMap<String,String>();
   ready(host);
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.DebuggingInterface
//
// Purpose-
//       Extend DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Object debugging message
{
   print(".fsm: " + fsm);
   print(".port: " + port);
   print(".reader: " + (reader != null));
   print(".socket: " + (socket != null));
   print(".writer: " + (writer != null));
}

public boolean                      // TRUE iff debug should write
   isDebug( )                       // Is debugging active?
{
   return DEBUG;                    // DEBUGGING control
}

public void
   error(
     String            string)
{
   System.err.println(toString() + string);
}

public void
   print(
     String            string)
{
   System.out.println(toString() + string);
}

public String
   toString( )                      // Used only in DebuggingInterface methods
{
   return "DbClient[" + port + "]: ";
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.recv
//
// Purpose-
//       Receive message.
//
//----------------------------------------------------------------------------
public String recv() throws IOException {
// debug("recv()..");
   String str= reader.readLine();
// debug("..recv("+str+")");
   return str;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.send
//
// Purpose-
//       Send message.
//
//----------------------------------------------------------------------------
public void send(String str) throws IOException {
// debug("send("+str+")");
   writer.println(str);
   writer.flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.ready
//
// Purpose-
//       Ready the Client.
//
//----------------------------------------------------------------------------
boolean ready(String host) {
   boolean result= (fsm != FSM_READY);
   if( result && host != null )
   {
     try {
       socket= new Socket(host, PORT);
       port= socket.getLocalPort();
       reader= new BufferedReader(
               new InputStreamReader(socket.getInputStream()));
       writer= new PrintWriter(
               new OutputStreamWriter(socket.getOutputStream()));
       recv();
       fsm= FSM_READY;
     } catch(Exception e) {
       error("DbClient: " + e);
       reset();
       result= false;
     }
   }

// debug("" + result + "= ready('" + host + "')");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.reset
//
// Purpose-
//       Reset the Client.
//
//----------------------------------------------------------------------------
void reset() {
// debug("reset()");

   try {
     if( reader != null ) {
       reader.close();
       reader= null;
     }
   } catch( Exception e ) {}

   try {
     if( writer != null ) {
       writer.close();
       writer= null;
     }
   } catch( Exception e ) {}

   try {
     if( socket != null ) {
       socket.close();
       socket= null;
     }
   } catch( Exception e ) {}

   fsm= FSM_RESET;
   port= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.get
//
// Purpose-
//       Extract data from database.
//
//----------------------------------------------------------------------------
public String                       // The database item
   get(                             // Get database item
     String            type,        // The item type
     String            item)        // The item name
{
   boolean             cached= true;
   String              result= null;

   try {
     String key= type + SEP + item;
     key= key.toUpperCase();
     result= cache.get(key);
     if( result == null )
     {
       cached= false;
       send(CMD_GET + " " + type + " " + item);
       result= recv();
       if( result == null )
         result= CMD_NULL;
       cache.put(key, result);      // Yes, we cache null resultants
     }

     if( result.equals(CMD_NULL) )
       result= null;
   } catch(Exception e) {
     error("get: Exception: " + e);
     reset();
     result= null;
   }

   debug("'"+result+"'= get("+type+","+item+")" + (cached ? " CACHED" : ""));

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.next
//
// Purpose-
//       Extract next item data from database.
//
//----------------------------------------------------------------------------
public String                       // The database item
   next(                            // Get database item
     String            type,        // The item type
     String            item)        // The item name
{
   String              result= null;

   try {
     send(CMD_NEXT + " " + type + " " + item);
     result= recv();
     if( result != null && result.equals(CMD_NULL) )
       result= null;
   } catch(Exception e) {
     error("next: Exception: " + e);
     reset();
     result= null;
   }

   debug("'"+result+"'= next("+type+","+item+")");

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.put
//
// Purpose-
//       Insert data into database.
//
//----------------------------------------------------------------------------
public String                       // The prior data
   put(                             // Put database item
     String            type,        // The item type
     String            item,        // The item name
     String            data)        // The associated data record
   throws IOException
{
   String              result= "NG";

   try {
     send(CMD_PUT+" "+type+" "+item+" "+data);
     result= recv();
     if( result != null && result.equals(CMD_NULL) )
       result= null;
     if( data == null )
       data= CMD_NULL;
     String key= type + SEP + item;
     if( type != COMMENT )
       cache.put(key.toUpperCase(), data);
   } catch(IOException e) {
     error("put: IOException: " + e);
     reset();
     throw e;
   } catch(Exception e) {
     error("put: Exception: " + e);
     reset();
     throw new IOException("DbClient.put Exception", e);
   }

   debug("'"+result+"'= put("+type+","+item+","+data+")");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbClient.remove
//
// Purpose-
//       Remove data from database.
//
//----------------------------------------------------------------------------
public String                       // The prior data
   remove(                          // Remove database item
     String            type,        // The item type
     String            item)        // The item name
   throws IOException
{
   String              result= "NG";

   try {
     send(CMD_REMOVE+" "+type+" "+item);
     result= recv();
     if( result != null && result.equals(CMD_NULL) )
       result= null;
     String key= type + SEP + item;
     cache.remove(key.toUpperCase());
   } catch(IOException e) {
     error("remove: IOException: " + e);
     reset();
     throw e;
   } catch(Exception e) {
     error("remove: Exception: " + e);
     reset();
     throw new IOException("DbClient.remove Exception", e);
   }

   debug("'"+result+"'= remove("+type+","+item+")");
   return result;
}
} // class DbClient
