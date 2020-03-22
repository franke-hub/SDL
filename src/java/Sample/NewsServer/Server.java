//----------------------------------------------------------------------------
//
//       Copyright (C) 2008 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Server.java
//
// Purpose-
//       Sample RFC 977 Network News Transfer Protocol (NNTP) server.
//       Includes RFC 2980 extensions.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.util.*;

public class Server extends Thread
{
//----------------------------------------------------------------------------
// Server.Constants for parameterization
//----------------------------------------------------------------------------
static final int       FSM_RESET= 0; // Reset
static final int       FSM_READY= 1; // Ready to start
static final int       FSM_SERVE= 2; // Running server
static final int       FSM_DONE=  3; // Complete

final String           groupArray[]=
{  "G1         1  20 n"             // Name, lo, hi, posting?
,  "G2       101 120 n"
,  "alt.G2   101 120 y"
,  "alt.0d   201 220 y"
,  "alt.test 301 340 y"
};

//----------------------------------------------------------------------------
// Server.Attributes
//----------------------------------------------------------------------------
Socket                 talk;        // Talk Socket
BufferedReader         reader;      // Socket input stream
PrintWriter            writer;      // Socket output stream

int                    index;       // The Server index
int                    fsm;         // Finite State Machine
int                    group;       // The current group
int                    article;     // The current article

//----------------------------------------------------------------------------
//
// Method-
//       Server.Server
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Server(                          // Constructor
     int               index,       // The Server index
     Socket            talk)        // The connection Socket
   throws Exception
{
   this.index= index;
   this.fsm= FSM_READY;
   this.talk= talk;
   reader= new BufferedReader(
           new InputStreamReader(talk.getInputStream())
                             );

   writer= new PrintWriter(
           new BufferedWriter(
           new OutputStreamWriter(talk.getOutputStream())
                          )  );

   group= (-1);
   article= (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.debugf
//
// Purpose-
//       Debugging printf.
//
//----------------------------------------------------------------------------
public void
   debugf(
     String            string)
{
   System.out.print("[" + index + "] " + string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.send
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
   debugf("send: " + message + "\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.receive
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
   debugf("recv: " + result + "\n");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.articleID
//
// Purpose-
//       Generate article identifier.
//
//----------------------------------------------------------------------------
public String                       // Resultant article identifier
   articleID(                       // Generate article identifier
     int               ident)       // Article number
{
   return "<" + ident + "@127.0.0.1>";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.getARTICLE
//
// Purpose-
//       Get article number from ARTICLE, HEAD, BODY or STAT command
//
//----------------------------------------------------------------------------
public int                          // The article number, (-1) if invalid
   getARTICLE(                      // Get article number
     String            string)      // Command
   throws Exception
{
   int                 article= this.article;

   if( group < 0 || group >= groupArray.length )
   {
     send("412 no newsgroup has been selected");
     return (-1);
   }

   StringTokenizer st= new StringTokenizer(groupArray[group]);
   String name= st.nextToken();
   int      lo= Integer.parseInt(st.nextToken());
   int      hi= Integer.parseInt(st.nextToken());

   try {
     if( string.equals("") )
     {
       if( article < 0 )
       {
         send("420 no current article has been selected");
         return (-1);
       }

       if( article < lo || article > hi )
       {
         send("423 no such article number in the newsgroup");
         return (-1);
       }
     }
     else if( string.charAt(0) == '<' && string.charAt(string.length()-1) == '>' )
     {
       try {
         int atIndex= string.indexOf('@', 1);
         if( atIndex < 0 )
           throw new Exception("non-existent article");
         article= Integer.parseInt(string.substring(1, atIndex));
         if( article < lo || article > hi )
           throw new Exception("non-existent article");
         if( !string.substring(atIndex).equals("@localhost>") )
           throw new Exception("non-existent article");
       } catch( Exception e ) {
         send("430 non-existent article");
         return (-1);
       }
     }

     else if( string.charAt(0) >= '0' && string.charAt(0) <= '9' )
     {
       article= Integer.parseInt(string);
       if( article < lo || article > hi )
       {
         send("430 non-existent article");
         return (-1);
       }

       this.article= article;
     }
     else
       throw new Exception("command syntax error");
   } catch( Exception e ) {
     send("501 command syntax error");
     return (-1);
   }

   return article;
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.putBODY
//
// Purpose-
//       BODY response data.
//
//----------------------------------------------------------------------------
public void
   putBODY(                         // Send BODY response data
     String            group,       // Group name
     int               article)     // Article number
   throws Exception
{
   if( group.equals("alt.G2") )
     group= "G2";

   for(int i= 0; i<20; i++)
     send("[" + group + ":" + article + "] Text line " + i);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.getHEAD
//
// Purpose-
//       HEAD response data.
//
//----------------------------------------------------------------------------
public void
   putHEAD(                         // Send HEAD response data
     String            group,       // Group name
     int               article)     // Article number
   throws Exception
{
   send("From: \"Rob Server\" <bart@localhost>");

   String refers= group;
   if( refers.equals("G2") )
     refers= "G2,alt.G2";
   else if( refers.equals("alt.G2") )
     refers= "alt.G2,G2";
   send("Newsgroups: " + refers);

   send("Subject: Article " + article + " information");
   send("Date: Thu, 15 Mar 2007 12:13:14 -0800");
   send("Message-ID: " + articleID(article));
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdARTICLE
//
// Purpose-
//       Handle ARTICLE command.
//
//----------------------------------------------------------------------------
public String                       // Response (if any)
   cmdARTICLE(                      // Process ARTICLE command
     String            string)      // Parameter string
   throws Exception
{
   int                 i;

   int article= getARTICLE(string);
   if( article < 0 )
     return null;

   StringTokenizer st= new StringTokenizer(groupArray[group]);
   String name= st.nextToken();
   int      lo= Integer.parseInt(st.nextToken());
   int      hi= Integer.parseInt(st.nextToken());

   // Send acknowledgement
   send("220 "+article+" "+articleID(article)+" article retrieved - head and body follow");

   // Send HEAD
   putHEAD(name, article);

   // Send delimiter
   send("");

   // Send BODY
   putBODY(name, article);

   return ".";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdBODY
//
// Purpose-
//       Handle BODY command.
//
//----------------------------------------------------------------------------
public String                       // Response (if any)
   cmdBODY(                         // Process BODY command
     String            string)      // Parameter string
   throws Exception
{
   int article= getARTICLE(string);
   if( article < 0 )
     return null;

   StringTokenizer st= new StringTokenizer(groupArray[group]);
   String name= st.nextToken();
   int      lo= Integer.parseInt(st.nextToken());
   int      hi= Integer.parseInt(st.nextToken());

   // Send acknowledgement
   send("222 "+article+" "+articleID(article)+" article retrieved - body follows");

   // Send BODY
   putBODY(name, article);

   return ".";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdGROUP
//
// Purpose-
//       Handle GROUP command.
//
//----------------------------------------------------------------------------
public String
   cmdGROUP(
     String            string)
   throws Exception
{
   String              result;      // Resultant

   group= (-1);
   result= "411 non-existent";
   for(int i= 0; i<groupArray.length; i++)
   {
     StringTokenizer st= new StringTokenizer(groupArray[i]);
     String name= st.nextToken();
     if( string.equals(name) )
     {
       group= i;
       article= (-1);
       int lo= Integer.parseInt(st.nextToken());
       int hi= Integer.parseInt(st.nextToken());
       result= "211 "+(hi-lo)+" "+lo+" "+hi+" "+name+" group selected";
       break;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdHEAD
//
// Purpose-
//       Handle HEAD command.
//
//----------------------------------------------------------------------------
public String                       // Response (if any)
   cmdHEAD(                         // Process HEAD command
     String            string)      // Parameter string
   throws Exception
{
   int article= getARTICLE(string);
   if( article < 0 )
     return null;

   StringTokenizer st= new StringTokenizer(groupArray[group]);
   String name= st.nextToken();
   int      lo= Integer.parseInt(st.nextToken());
   int      hi= Integer.parseInt(st.nextToken());

   // Send acknowledgement
   send("221 "+article+" "+articleID(article)+" article retrieved - head follows");

   // Send HEAD
   putHEAD(name, article);

   return ".";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdLIST
//
// Purpose-
//       Handle LIST command.
//
//----------------------------------------------------------------------------
public String
   cmdLIST(
     String            string)
   throws Exception
{
   int                 i;

   send("215 list of newsgroups follows");
   for(i= 0; i<groupArray.length; i++)
     send(groupArray[i]);

   return ".";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdMODE
//
// Purpose-
//       Handle MODE command.
//
//----------------------------------------------------------------------------
public String
   cmdMODE(
     String            string)
   throws Exception
{
   return "500 Not supported";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdNEXT
//
// Purpose-
//       Handle NEXT command.
//
//----------------------------------------------------------------------------
public String
   cmdNEXT(
     String            string)
   throws Exception
{
   if( !string.equals("") )
     return "501 NEXT does not expect data";

   if( group < 0 || group >= groupArray.length )
     return "412 No newsgroup selected";

   if( article < 0 )
     return "420 no current article has been selected";

   StringTokenizer st= new StringTokenizer(groupArray[group]);
   String name= st.nextToken();
   int      lo= Integer.parseInt(st.nextToken());
   int      hi= Integer.parseInt(st.nextToken());

   if( article >= hi )
     return "421 no next article in this group";

   article++;
   return "223 "+article+" "+articleID(article)+" article retrieved - request text separately";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdQUIT
//
// Purpose-
//       Handle QUIT command.
//
//----------------------------------------------------------------------------
public String
   cmdQUIT(
     String            string)
   throws Exception
{
   return "205 Goodbye";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.cmdSTAT
//
// Purpose-
//       Handle STAT command.
//
//----------------------------------------------------------------------------
public String                       // Response (if any)
   cmdSTAT(                         // Process STAT command
     String            string)      // Parameter string
   throws Exception
{
   StringBuffer        refers;
   int                 i;

   int article= getARTICLE(string);
   if( article < 0 )
     return null;

   StringTokenizer st= new StringTokenizer(groupArray[group]);
   String name= st.nextToken();
   int      lo= Integer.parseInt(st.nextToken());
   int      hi= Integer.parseInt(st.nextToken());

   return "223 "+article+" "+articleID(article)+" article retrieved - request text separately";
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.run
//
// Purpose-
//       Run the Server thread
//
//----------------------------------------------------------------------------
public void
   run( )
{
   Date                timeStart;   // Starting time
   Date                timeFinal;   // Ending time
   double              elapsed;     // Elapsed time

   fsm= FSM_SERVE;
   debugf("Ready\n");

   timeStart= new Date();
   try {
     server();
   } catch(Exception e) {
     debugf("Exception: " + e + "\n");
   }
   timeFinal= new Date();

   try {
     Thread.sleep(1500);
   } catch(Exception e) {
     debugf("Exception: " + e + "\n");
   }

   try {
     writer.close();
   } catch(Exception e) {
     debugf("Exception: " + e + "\n");
   }

   try {
     reader.close();
   } catch(Exception e) {
     debugf("Exception: " + e + "\n");
   }

   try {
     talk.close();
   } catch(Exception e) {
     debugf("Exception: " + e + "\n");
   }

   elapsed= timeFinal.getTime() - timeStart.getTime();
   elapsed /= 1000.0;
   debugf("Elapsed: " + elapsed + " seconds\n");
   fsm= FSM_DONE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Server.server
//
// Purpose-
//       Server.
//
//----------------------------------------------------------------------------
public void
   server( )
   throws Exception
{
   String              string;      // Input/response String
   String              command;     // Input command
   int                 x;           // String index

   int                 i;

   send("201 NewsServer ready, no posting");
   for(;;)
   {
     string= receive();
     if( string == null )
     {
       debugf("End of file\n");
       break;
     }

     command= string;
     x= string.indexOf(' ');
     if( x >= 0 )
     {
       command= string.substring(0, x);
       string= string.substring(x+1);
     }
     else
       string= "";

     if( command.equalsIgnoreCase("ARTICLE") )
       string= cmdARTICLE(string);

     else if( command.equalsIgnoreCase("BODY") )
       string= cmdBODY(string);

     else if( command.equalsIgnoreCase("GROUP") )
       string= cmdGROUP(string);

     else if( command.equalsIgnoreCase("HEAD") )
       string= cmdHEAD(string);

     else if( command.equalsIgnoreCase("LIST") )
       string= cmdLIST(string);

     else if( command.equalsIgnoreCase("MODE") )
       string= cmdMODE(string);

     else if( command.equalsIgnoreCase("NEXT") )
       string= cmdNEXT(string);

     else if( command.equalsIgnoreCase("QUIT") )
     {
       string= cmdQUIT(string);
       send(string);
       break;
     }

     else if( command.equalsIgnoreCase("STAT") )
       string= cmdSTAT(string);

     else
       string= "500 Invalid command";

     if( string != null )
       send(string);
   }
}
} // Class Server

