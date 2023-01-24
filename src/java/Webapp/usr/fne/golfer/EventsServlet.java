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
//       EventsServlet.java
//
// Purpose-
//       Events HttpServlet.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
// package usr.fne.golfer;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;
import java.util.TreeMap;
import java.util.Vector;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.ServletConfig;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.ServletInputStream;
import javax.servlet.ServletRequest;

import usr.fne.common.LoggingService;
import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       EventsServlet
//
// Purpose-
//       Events HttpServlet.
//
//----------------------------------------------------------------------------
public class EventsServlet extends HttpServlet implements LoggingService
{
//----------------------------------------------------------------------------
// EventsServlet.Attributes
//----------------------------------------------------------------------------
boolean                debug;       // Debugging control
int                    verbose;     // Verbosity control

ServletConfig          config;      // ServletConfig
ServletContext         context;     // ServletContext
DbServer               dbServer;    // Our database local server

static final String    versionID= "VID 2009/05/14 14:00";

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
protected static int                // The concatenation index
   catcon(                          // Deconcatenate
     String            itemqual,    // The item.qualifier
     int               index)       // The qualifier index
{
   int offset= itemqual.indexOf('.');
   if( offset < 0 )
     return offset;

   for(--index; index > 0; --index)
   {
     offset++;
     int next= itemqual.substring(offset).indexOf('.');
     if( next < 0 )
       return next;

     offset += next;
   }

   return offset;
}

protected String                    // The database item
   dbGet(                           // Get database item
       String          type,        // The item type
       String          item)        // The item name
{
   return dbServer.get(type, item);
}

protected String                    // The database item
   dbNext(                          // Get next database item
       String          type,        // The item type
       String          item)        // The item name
{
   return dbServer.next(type, item);
}

protected String                    // The prior database item
   dbPut(                           // Get database item
       String          type,        // The item type
       String          item,        // The item name
       String          data)        // The item data
   throws IOException
{
   return dbServer.put(type, item, data);
}

protected String                    // The prior database item
   dbRemove(                        // Get database item
       String          type,        // The item type
       String          item)        // The item name
   throws IOException
{
   return dbServer.remove(type, item);
}

protected static String             // Resultant
   stripQuotes(                     // Strip quotes
     String            string)      // From this String
{
   if( string == null )
     return null;

   int length= string.length();
   if( length < 2 )
     return null;

   if( string.charAt(0) == '\"' && string.charAt(length-1) == '\"' )
     return string.substring(1, length-1);

   return string;
}


//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.log
//
// Purpose-
//       Write a message to the log.
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
   log(                             // Write a log message
     String            message)     // The log message
{
   log(message, null);
}

public void
   log(                             // Write log message
     String            message,     // The log message
     Throwable         e)           // (Optional) associated exception
{
   if( context == null || verbose > 1 )
   {
     System.out.println("EventsServlet: " + message);
     if( e != null )
       e.printStackTrace();
   }
   else
   {
     if( e != null )
       context.log("EventsServlet: " + message, e);
     else
       context.log("EventsServlet: " + message);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.init
//
// Purpose-
//       Called when the Servlet is first started for one-time initialization.
//
//----------------------------------------------------------------------------
public void
   init()
   throws ServletException
{
   boolean             error= false;// TRUE if initialization failure
   String              string;      // Working String

   // Get configuration parameters
   config  = getServletConfig();
   context = config.getServletContext();
   log("init()");

   String  propertyPath = config.getInitParameter("property-path");
   String  propertyFile = config.getInitParameter("property-file");
   String  propertyName = context.getRealPath(propertyPath+"/"+propertyFile);

   // Get properties
   Properties props= new Properties();
   try {
     props.load(new FileInputStream(propertyName));
   } catch( Exception e ) {
     log("init: Error loading: " + propertyName, e);
     throw new ServletException("Initialization failed");
   }

   // Set debug control
   verbose= Integer.parseInt(props.getProperty("verbose","0"));
   debug= (verbose != 0);

   // Initialize databases
   dbServer= DbServer.singleton(props);

   // Bringup debugging
   if( debug )
   {
     log("Parameters:");
     log(" property-path: " + propertyPath);
     log(" property-file: " + propertyFile);
     log(" property-name: " + propertyName);
     log("Controls:");
     log("         debug: " + debug);
     log("       verbose: " + verbose);
     log("Environment:");
     log("          root: " + context.getRealPath("."));
     log("   ServletInfo: " + getServletInfo());
     log("   ServletName: " + getServletName());

     try {
       string= context.getResource("/.").toString();
     } catch( Exception e ) {
       string= "Exception: " + e;
     }
     log("x.ResourceName: " + string);
     log("  x.ServerInfo: " + context.getServerInfo());
     log(" x.ContextName: " + context.getServletContextName());
     log("Applications:");
     log("         state: " + (error ? "Error" : "Ready"));
   }

   if( error )
     throw new ServletException("Initialization failed");
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.doGet
//
// Purpose-
//       Called for each HTTP GET request.
//
//----------------------------------------------------------------------------
public void
   doGet(                           // Handle HTTP "GET" request
     HttpServletRequest  req,       // Request information
     HttpServletResponse res)       // Response information
   throws ServletException, IOException
{
   String q= req.getQueryString();
   if( debug ) log("doGet("+q+")");

   res.setContentType("text/html");
   PrintWriter out = res.getWriter();

   String name = req.getParameter("name");
   if( name == null || name.equals("") )
     name=req.getRemoteHost();

   query(req, res);
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.doPost
//
// Purpose-
//       Called for each HTTP POST request.
//
//----------------------------------------------------------------------------
public void
   doPost(                          // Handle HTTP "POST" request
     HttpServletRequest  req,       // Request information
     HttpServletResponse res)       // Response information
   throws ServletException, IOException
{
   String q= req.getQueryString();
   if( debug ) log("doPost("+q+")");

   res.setContentType("text/html");
   PrintWriter out = res.getWriter();

   out.println("<HTML>");
   out.println("<HEAD><TITLE>EventsServlet POST</TITLE></HEAD>");
   out.println("<BODY>POST NOT EXPECTED</BODY>");
   out.println("</HTML>");
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.destroy
//
// Purpose-
//       Clean up when Servlet is stopped.
//
//----------------------------------------------------------------------------
public void
   destroy()                        // Stop this Servlet
{
   if( debug ) log("destroy()..");

   dbServer.reset();
   dbServer= null;

   if( debug ) log("..destroy()");
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.getParm
//
// Purpose-
//       Extract parameter from query.
//
//----------------------------------------------------------------------------
public static String                // Resultant parameter, null if not found
   getParm(                         // Extract parameter from query
     String            param,       // The parameter
     String            query)       // The query
{
   int                 C= 0;        // The current character
   int                 quote= 0;    // Quote state character

   int                 pL= param.length(); // Parameter length
   int                 pX= 0;       // Parameter index
   int                 qL= query.length(); // Query length
   int                 qX= 0;       // Query index

   while( qX < qL )
   {
     C= query.charAt(qX++);
     if( quote != 0 )
     {
       if( C == quote )
       {
         quote= 0;
         continue;
       }
     }

     if( C == '\'' || C == '\"' )
     {
       pX= (-1);
       quote= C;
       continue;
     }

     if( pX >= 0 )
     {
       if( pX == pL )
       {
         if( C == '=' )
           break;
       }
       else if( C == param.charAt(pX) )
         pX++;
       else
         pX= (-1);
     }

     if( pX < 0 )
     {
       if( C == ',' )
         pX= 0;
     }
   }

   if( C != '=' || pL != pX )
     return null;

   // The parameter has been found.
   if( qX >= qL )
     return "";

   StringBuffer sb= new StringBuffer();
   C= query.charAt(qX);
   if( C == '\'' || C == '\"' )
   {
     quote= C;
     qX++;
     while( qX < qL )
     {
       C= query.charAt(qX++);
       if( C == quote )
         break;

       sb.append((char)C);
     }
   }
   else
   {
     while( qX < qL )
     {
       C= query.charAt(qX++);
       if( C == ',' )
         break;

       sb.append((char)C);
     }
   }

   return sb.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.putError
//
// Purpose-
//       Generate error response.
//
//----------------------------------------------------------------------------
public void
   putError(                        // Generate error response
     PrintWriter       out,         // The response writer
     String            msg)         // The error message
{
   out.println("<HTML>");
   out.println("<HEAD><TITLE>" + msg + "</TITLE></HEAD>");
   out.println("<BODY>");
   out.println("<H1 align=\"center\">" + msg + "</H1>");
   out.println("</BODY>");
   out.println("</HTML>");
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.userPass
//
// Purpose-
//       Generate userid credentials.
//
//----------------------------------------------------------------------------
public String                       // Resultant credential String
   userPass(                        // Generate error response
     String            userName,    // The user name
     String            userPass)    // The user password
{
   StringBuffer        sb= new StringBuffer();

   sb.append("username=" + userName);
   if( userPass != null )
     sb.append(",password=" + userPass);

   return sb.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.dateOption
//
// Purpose-
//       Generate a date OPTION list
//
//----------------------------------------------------------------------------
protected void
   dateOption(                      // Generate a date OPTION list
     PrintWriter         out,       // The output PrintWriter
     String              eventID)   // The associated EVENTS_ID
{
   String CMD= "EVENTS.DATE";
   String ITEM= eventID;
   for(;;)
   {
     String text= dbNext(CMD, ITEM);
     if( text == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(text);
     String type= t.nextToken();

     if( !type.equals(CMD) )
       break;

     ITEM= t.nextToken();
     int index= catcon(ITEM, 1);
     if( index < 0 )
       continue;

     if( !eventID.equals(ITEM.substring(0,index)) )
       break;

     // Process the date
     String date= ITEM.substring(index+1); // Skip over year, too
     out.println("      <OPTION value='" + date + "'>" + date.substring(5) + "</OPTION>");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.findOption
//
// Purpose-
//       Generate an OPTION list
//
//----------------------------------------------------------------------------
protected void
   findOption(                      // Generate an OPTION list
     PrintWriter         out,       // The output PrintWriter
     String              BASE)      // The command string base
{
   TreeMap<String,String> treeMap= new TreeMap<String,String>();
   String CMD= BASE + ".FIND";
   String ITEM= "";
   for(;;)
   {
     String text= dbNext(CMD, ITEM);
     if( text == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(text);
     String type= t.nextToken();

     if( !type.equals(CMD) )
       break;

     ITEM= t.nextToken();
     String tempID= t.nextToken();
     if( !ITEM.equals(tempID) )
       continue;

     String showID= dbGet(BASE + ".SHOW", tempID);
     if( showID == null )
       continue;

     treeMap.put(stripQuotes(showID), tempID);
   }

   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     Map.Entry me= (Map.Entry)i.next();
     String showID= (String)me.getKey();
     String findID= (String)me.getValue();
     out.println("      <OPTION value='" + findID + "'>" + showID + "</OPTION>");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsServlet.query
//
// Purpose-
//       Handle a query.
//
//----------------------------------------------------------------------------
protected void
   query(                           // Handle a query
     HttpServletRequest  req,       // Request information
     HttpServletResponse res)       // Response information
   throws ServletException, IOException
{
   String q= req.getQueryString();
   if( debug ) log("query("+q+")");

   PrintWriter out = res.getWriter();
   String BOGUS= "<br> Malformed request: query: '" + q + "'";

   //-------------------------------------------------------------------------
   // Get eventID and eventName
   String eventID= getParm("event", q);
   String eventName= null;

   if( eventID == null || eventID.equalsIgnoreCase("DEFAULT") )
     eventID= dbGet("DEFAULT", "EVENTS_ID");

   if( eventID != null )
     eventName= stripQuotes(dbGet("EVENTS.SHOW", eventID));

   //=========================================================================
   // FIRSTPAGE request
   // Events?FIRSTPAGE
   if( q.equalsIgnoreCase("FIRSTPAGE") )
   {
     if( eventID != null )
       q= "event=" + eventID;
     else
     {
       //---------------------------------------------------------------------
       // Generate database
       //---------------------------------------------------------------------
       String credentials= dbGet("USERID.FIND", "MAINT");
       if( credentials == null )
         dbPut("USERID.FIND", "MAINT", "admin");

       q= "maint=PANEL,username=admin";
     }
   }

   //=========================================================================
   // EVENT request
   // Events?event=<id> || Events?event=DEFAULT
   if( q.startsWith("event=") )
   {
     if( eventID == null )
     {
       putError(out, "No DEFAULT Event");
       return;
     }

     if( eventName == null )
     {
       putError(out, "Invalid Event: " + eventID);
       return;
     }

     //-----------------------------------------------------------------------
     // HEADING
     //-----------------------------------------------------------------------
     out.println("<HTML>");
     out.println("<HEAD><TITLE>" + eventName + "</TITLE></HEAD>");
     out.println(versionID);
     out.println("<script language=\"javascript\" src=\"applet.js\">");
     out.println("</script>");
     out.println("<script language=\"javascript\">");
     out.println("function reset()");
     out.println("{");
     out.println("  document.form1.result.selectedIndex= 0;");
     out.println("  document.form1.player.selectedIndex= 0;");
     out.println("  document.form1.course.selectedIndex= 0;");
     out.println("  document.form1.event1.selectedIndex= 0;");
     out.println("  document.form1.username.value= \"\";");
     out.println("  document.form1.userpass.value= \"\";");
     out.println("}");
     out.println("</script>");
     out.println("<BODY>");
     out.println("<HR>");
     out.println("<H1 align=\"center\">" + eventName + "</H1>");
     out.println("<HR>");
     out.println("<FORM name=\"form1\">");
     out.println("<TABLE>");
     out.println("  <COLGROUP span=\"5\"></COLGROUP>");
     out.println("  <TBODY>");

     //-----------------------------------------------------------------------
     // viewStats
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    View Statistics");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"viewStats('"+ eventID + "')\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // playStats
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    ESC Scores");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"playStats('"+ eventID + "')\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // viewEvents
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    Daily Result");
     out.println("    </TD><TD>");
     out.println("    <SELECT name=\"result\">");
     out.println("      <OPTION selected>");

     dateOption(out, eventID);

     out.println("    </SELECT>");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"viewEvents('"+ eventID + "',result)\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // viewHandicap
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    View Handicap");
     out.println("    </TD><TD>");
     out.println("    <SELECT name=\"player\">");
     out.println("      <OPTION selected>");

     findOption(out, "PLAYER");

     out.println("    </SELECT>");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"viewHandicap('"+ eventID + "',player)\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // viewCourse
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    View Course");
     out.println("    </TD><TD>");
     out.println("    <SELECT name=\"course\">");
     out.println("      <OPTION selected>");

     findOption(out, "COURSE");

     out.println("    </SELECT>");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"viewCourse(course)\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // Change Event
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    Change Event");
     out.println("    </TD><TD>");
     out.println("    <SELECT name=\"event1\">");
     out.println("      <OPTION selected>");

     findOption(out, "EVENTS");

     out.println("    </SELECT>");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"changeEvent(event1)\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // Maintenance
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    Maintenance");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"maintEvents('"+ eventID + "',username,userpass)\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR><TR>");
     out.println("    <TD><TD>");
     out.println("    <INPUT type=\"TEXT\" NAME=\"username\" SIZE=\"32\">");
     out.println("    </TD><TD>");
     out.println("    Enter Userid");
     out.println("    </TD></TD></TR><TR><TD><TD>");
     out.println("    <INPUT type=\"TEXT\" NAME=\"userpass\" SIZE=\"32\">");
     out.println("    </TD><TD>");
     out.println("    Enter Password");
     out.println("    </TD></TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // FOOTING
     //-----------------------------------------------------------------------
     out.println("  </TBODY>");
     out.println("</TABLE>");
     out.println("</FORM>");
     out.println("</HTML>");
     return;
   }

   //=========================================================================
   // Maintenance request
   if( q.startsWith("maint=") )
   {
     String credentials= dbGet("USERID.FIND", "MAINT");
     if( credentials == null )
     {
       putError(out, "No credentials in database");
       return;
     }

     QuotedTokenizer t= new QuotedTokenizer(credentials);
     String userName= t.nextToken();
     String userPass= null;
     if( t.hasMoreTokens() )
       userPass= t.nextToken();

     //-------------------------------------------------------------------------
     // Validate maintenance request
     //-------------------------------------------------------------------------
     String id= getParm("username", q);
     if( id == null || !id.equals(userName) )
     {
       putError(out, "Invalid username/password");
       return;
     }

     if( userPass != null )
     {
       id= getParm("password", q);
       if( id == null || !id.equals(userPass) )
       {
         putError(out, "Invalid username/password");
         return;
       }
     }

     id= getParm("maint", q);
     if( id.equalsIgnoreCase("DBSTART") )
     {
       dbServer.start();
       putError(out, "Database STARTED");
       return;
     }

     if( id.equalsIgnoreCase("DBSTOP") )
     {
       dbServer.reset();
       putError(out, "Database STOPPED");
       return;
     }

     if( id.equalsIgnoreCase("PANEL") )
     {
       if( eventID == null )
         eventID= "DEFAULT";

       //-----------------------------------------------------------------------
       // HEADING
       //-----------------------------------------------------------------------
       out.println("<HTML>");
       out.println("<HEAD><TITLE>Maintenance Page</TITLE></HEAD>");
       out.println(versionID);
       out.println("<script language=\"javascript\" src=\"applet.js\">");
       out.println("</script>");
       out.println("<script language=\"javascript\">");
       out.println("function reset()");
       out.println("{");
       out.println("  document.form1.result.selectedIndex= 0;");
       out.println("  document.form1.player.selectedIndex= 0;");
       out.println("  document.form1.course.selectedIndex= 0;");
       out.println("  document.form1.event1.selectedIndex= 0;");
       out.println("  document.form1.score1.selectedIndex= 0;");
       out.println("  document.form1.event2.selectedIndex= 0;");
       out.println("}");
       out.println("</script>");
       out.println("<BODY>");
       out.println("<HR>");
       if( eventName != null )
         out.println("<H1 align=\"center\">" + eventName + "</H1>");
       out.println("<H1 align=\"center\">Maintenance Page</H1>");
       out.println("<HR>");
       out.println("<FORM name=\"form1\">");
       out.println("<TABLE>");
       out.println("  <COLGROUP span=\"5\"></COLGROUP>");
       out.println("  <TBODY>");

       //-----------------------------------------------------------------------
       // cardEvents
       //-----------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    Update Result");
       out.println("    </TD><TD>");
       out.println("    <SELECT name=\"result\">");
       out.println("      <OPTION selected>");

       dateOption(out, eventID);

       out.println("    </SELECT>");
       out.println("    </TD><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"cardEvents('"+ eventID + "',result)\">Go</BUTTON>");
       out.println("    </TD>");
       out.println("  </TR>");

       //-----------------------------------------------------------------------
       // editPlayer
       //-----------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    Update Player");
       out.println("    </TD><TD>");
       out.println("    <SELECT name=\"player\">");
       out.println("      <OPTION selected>");

       findOption(out, "PLAYER");

       out.println("    </SELECT>");
       out.println("    </TD><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"editPlayer(player)\">Go</BUTTON>");
       out.println("    </TD>");
       out.println("  </TR>");

       //-----------------------------------------------------------------------
       // editCourse
       //-----------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    Update Course");
       out.println("    </TD><TD>");
       out.println("    <SELECT name=\"course\">");
       out.println("      <OPTION selected>");

       findOption(out, "COURSE");

       out.println("    </SELECT>");

       out.println("    </TD><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"editCourse(course)\">Go</BUTTON>");
       out.println("    </TD>");
       out.println("  </TR>");

       //-----------------------------------------------------------------------
       // editEvent
       //-----------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    Update Event");
       out.println("    </TD><TD>");
       out.println("    <SELECT name=\"event1\">");
       out.println("      <OPTION selected>");

       findOption(out, "EVENTS");

       out.println("    </SELECT>");
       out.println("    </TD><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"editEvents(event1)\">Go</BUTTON>");
       out.println("    </TD>");
       out.println("  </TR>");

       //-----------------------------------------------------------------------
       // DATABASE inserts
       //-----------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"insertPlayer()\">Add Player</BUTTON>");
       out.println("    </TD></TR><TR><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"insertEvents()\">Add Event </BUTTON>");
       out.println("    </TD></TR><TR><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"insertCourse()\">Add Course</BUTTON>");
       out.println("    </TD></TR><TR><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"setDefaults()\">Set Defaults</BUTTON>");
       out.println("    </TD></TR><TR><TD>");
       out.println("    </TD>");
       out.println("  </TR>");

       //-----------------------------------------------------------------------
       // DATABASE controls
       //-----------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    <A href=\"/golfer/Events?maint=DBSTOP," +
                   userPass(userName, userPass) + "\">" +
                   "Database STOP</A>");
       out.println("    </TD><TD>");
       out.println("    </TR><TR>");
       out.println("    <TD>");
       out.println("    <A href=\"/golfer/Events?maint=DBSTART," +
                   userPass(userName, userPass) + "\">" +
                   "Database START</A>");
       out.println("    </TD><TD>");
       out.println("    </TR><TR>");
       out.println("    <TD>");
       out.println("    <A href=\"debug.html\">Debugging page</A>");
       out.println("    </TD>");
       out.println("  </TR>");

       //-----------------------------------------------------------------------
       // Change Event
       //-----------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    Change Event");
       out.println("    </TD><TD>");
       out.println("    <SELECT name=\"event2\">");
       out.println("      <OPTION selected>");

       findOption(out, "EVENTS");

       out.println("    </SELECT>");
       out.println("    </TD><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"changeEventsMaint(event2,'" + userName + "','" + userPass + "')\">Go</BUTTON>");
       out.println("    </TD>");
       out.println("  </TR>");

       //-----------------------------------------------------------------------
       // FOOTING
       //-----------------------------------------------------------------------
       out.println("  </TBODY>");
       out.println("</TABLE>");
       out.println("</FORM>");
       out.println("</HTML>");
       return;
     }
   }

   //=========================================================================
   // BOGUS request
   putError(out, BOGUS);
}
} // Class EventsServlet

