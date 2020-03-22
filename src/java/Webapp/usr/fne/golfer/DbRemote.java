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
//       DbRemote.java
//
// Purpose-
//       Golfer Database Server remote interface.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
package usr.fne.golfer;

import java.io.*;
import java.net.*;
import java.lang.*;
import java.util.*;

import usr.fne.common.QuotedTokenizer;

class DbRemote extends Thread {
//----------------------------------------------------------------------------
// DbRemote.Attributes
//----------------------------------------------------------------------------
static final boolean   DEBUG= false;// Debugging?

public int             fsm;         // The current state
public int             port;        // The connection port
public BufferedReader  reader;      // Socket Reader
public DbServer        server;      // Our database server
public Socket          socket;      // Connection Socket
public PrintWriter     writer;      // Socket Writer

//----------------------------------------------------------------------------
//
// Method-
//       DbRemote.Utilities
//
// Purpose-
//       Utility methods
//
//----------------------------------------------------------------------------
public void debug(String string) {
   if( DEBUG )
       System.out.println(toString() + ": " + string);
}

public void error(String string) {
   System.err.println(toString() + ": " + string);
}

public String toString() {
   return "DbRemote[" + port + "]";
}

//----------------------------------------------------------------------------
//
// Method-
//       DbRemote.DbRemote
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbRemote(                        // Constructor
     Socket            socket,      // Talk Socket
     DbServer          server)      // Associated Server
     throws IOException
{
   super("DbRemote");

   fsm= DbCommon.FSM_RESET;

   port= socket.getPort();
   reader= new BufferedReader(
           new InputStreamReader(socket.getInputStream()));
   writer= new PrintWriter(
           new OutputStreamWriter(socket.getOutputStream()));

   this.socket= socket;
   this.server= server;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbRemote.recv
//
// Purpose-
//       Receive message.
//
//----------------------------------------------------------------------------
public String recv() throws Exception {
   debug("recv()..");
   String str = reader.readLine();
   debug("..recv("+str+")");
   return str;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbRemote.send
//
// Purpose-
//       Send message.
//
//----------------------------------------------------------------------------
public void send(String str) throws Exception {
   if( str == null )
     str= DbCommon.CMD_NULL;

   debug("send("+str+")");
   writer.println(str);
   writer.flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbRemote.run
//
// Purpose-
//       Operate the thread.
//
//----------------------------------------------------------------------------
public void run() {
   debug("run()..");

   fsm= DbCommon.FSM_READY;
   while( fsm == DbCommon.FSM_READY )
   {
       try {
           String string= recv();
           if( string == null )
           {
             fsm= DbCommon.FSM_FINAL;
             break;
           }

           QuotedTokenizer tokenizer= new QuotedTokenizer(string);
           String code= null;
           String type= null;
           String item= null;
           String data= null;

           if( tokenizer.hasMoreTokens() )
             code= tokenizer.nextToken();

           if( tokenizer.hasMoreTokens() )
             type= tokenizer.nextToken();

           if( tokenizer.hasMoreTokens() )
             item= tokenizer.nextToken();

           if( tokenizer.hasMoreTokens() )
             data= tokenizer.remainder();

           if( code.equals(DbCommon.CMD_GET) )
             send(server.get(type, item));

           else if( code.equals(DbCommon.CMD_NEXT) )
             send(server.next(type, item));

           else if( code.equals(DbCommon.CMD_PUT) )
             send(server.put(type, item, data));

           else if( code.equals(DbCommon.CMD_REMOVE) )
             send(server.remove(type, item));

           else
           {
             error("Malformed request: '" + string + "'");
             send("!ERROR: Malformed");
           }
       } catch (EOFException e) { // No more data on this socket
           reset();
           fsm= DbCommon.FSM_FINAL;
           break;
       } catch (IOException e) { // Probable PUT failure
           reset();
           fsm= DbCommon.FSM_FINAL;
           break;
       } catch (Exception e) { // Unknown exception. Complain and quit.
           error("run(): Exception: " + e);
           e.printStackTrace();
           reset();
           break;
       }
   }
}

protected void finalize() {
    reset();
}

public synchronized void reset() {
    debug("reset()..");

    fsm= DbCommon.FSM_RESET;
    interrupt();
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

    debug("..reset()");
}
}
