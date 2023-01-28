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
//       PlayerServlet.java
//
// Purpose-
//       Player HttpServlet.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
package usr.fne.golfer;

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
//       PlayerServlet
//
// Purpose-
//       Player HttpServlet.
//
//----------------------------------------------------------------------------
public class PlayerServlet extends HttpServlet implements LoggingService
{
//----------------------------------------------------------------------------
// PlayerServlet.Attributes
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
//       PlayerServlet.UTILITIES
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

public static String                // Date (Format MM/DD/YYYY)
   showDate(                        // SortDate => ShowDate
     String            date)        // Date (Format YYYY/MM/DD)
{
   if( date.length() < 10 )
     return date;

   String head= date.substring(0,10);
   String tail= "";
   if( date.length() > 10 )
     tail= date.substring(10);

   return head.substring(5) + "/" + head.substring(0,4) + tail;
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
//       PlayerServlet.log
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
     System.out.println("PlayerServlet: " + message);
     if( e != null )
       e.printStackTrace();
   }
   else
   {
     if( e != null )
       context.log("PlayerServlet: " + message, e);
     else
       context.log("PlayerServlet: " + message);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerServlet.init
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
//       PlayerServlet.doGet
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
//       PlayerServlet.doPost
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
   out.println("<HEAD><TITLE>PlayerServlet POST</TITLE></HEAD>");
   out.println("<BODY>POST NOT EXPECTED</BODY>");
   out.println("</HTML>");
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerServlet.destroy
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
//       PlayerServlet.getParm
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
//       PlayerServlet.putError
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
//       PlayerServlet.userPass
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
//       PlayerServlet.cardOption
//
// Purpose-
//       Generate a card OPTION list
//
//----------------------------------------------------------------------------
protected void
   cardOption(                      // Generate a date OPTION list
     PrintWriter         out,       // The output PrintWriter
     String              playerNN)  // The associated PLAYER_NN
{
   TreeMap<String,String> treeMap= new TreeMap<String,String>();
   String CMD= "PLAYER.CARD";
   String ITEM= playerNN;
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

     if( !playerNN.equals(ITEM.substring(0,index)) )
       break;

     // Process the date
     String dateTime= ITEM.substring(index+1);
     String date= dateTime;
     String time= null;
     index= catcon(date, 1);
     if( index > 0 )
     {
       time= date.substring(index+1);
       date= date.substring(0, index);
     }

     String show= showDate(date);
     if( time != null )
       show= show + " " + time;

     treeMap.put(dateTime, show);
   }

   CMD= "EVENTS.CARD";
   ITEM= "";
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
     int index= catcon(ITEM, 3);
     if( index < 0 )
       continue;

     if( !playerNN.equals(ITEM.substring(index+1)) )
       continue;

     // Process the date
     String dateTime= ITEM.substring(catcon(ITEM,1)+1,index);
     String date= dateTime;
     String time= null;
     index= catcon(date, 1);
     if( index > 0 )
     {
       time= date.substring(index+1);
       date= date.substring(0, index);
     }

     String show= showDate(date);
     if( time != null )
       show= show + " " + time;

     treeMap.put(dateTime, show);
   }

   for(Iterator i= treeMap.entrySet().iterator(); i.hasNext();)
   {
     Map.Entry me= (Map.Entry)i.next();
     String findID= (String)me.getKey();
     String showID= (String)me.getValue();

     out.println("      <OPTION value='" + showID + "'>" + showID + "</OPTION>");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerServlet.findOption
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
//       PlayerServlet.findTeebox
//
// Purpose-
//       Generate a TEEBOX OPTION list
//
//----------------------------------------------------------------------------
protected void
   findTeebox(                      // Generate a TEEBOX OPTION list
     PrintWriter         out,       // The output PrintWriter
     String              courseID)  // The COURSE_ID
{
   TreeMap<String,String> treeMap= new TreeMap<String,String>();
   String CMD= "COURSE.TBOX";
   String ITEM= courseID;
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

     if( !courseID.equals(ITEM.substring(0,index)) )
       break;

     String teeboxID= ITEM.substring(index+1);
     treeMap.put(teeboxID, teeboxID);
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
//       PlayerServlet.query
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
   // Get playerID, playerNN, and playerName
   String playerID= getParm("player", q).toUpperCase();
   String playerNN= null;
   String playerName= null;

   if( playerID == null || playerID.equalsIgnoreCase("DEFAULT") )
     playerID= dbGet("DEFAULT", "PLAYER_ID");

   playerID= dbGet("PLAYER.FIND", playerID);
   playerNN= dbGet("PLAYER.NICK", playerID);

   if( playerID != null )
     playerID= playerID.toUpperCase();
   if( playerNN != null )
     playerNN= playerNN.toUpperCase();
   if( playerID != null )
     playerName= stripQuotes(dbGet("PLAYER.SHOW", playerID));

   //=========================================================================
   // FIRSTPAGE request
   // Player?FIRSTPAGE
   if( q.equalsIgnoreCase("FIRSTPAGE") )
   {
     if( playerID != null )
       q= "player=" + playerID;
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
   // Player request
   // Player?player=<id> || Player?player=DEFAULT
   if( q.startsWith("player=") )
   {
     if( playerID == null )
     {
       putError(out, "No DEFAULT player");
       return;
     }

     if( playerNN == null || playerName == null )
     {
       putError(out, "Invalid Player: " + playerID);
       return;
     }

     //-----------------------------------------------------------------------
     // HEADING
     //-----------------------------------------------------------------------
     out.println("<HTML>");
     out.println("<HEAD><TITLE>" + playerName + "</TITLE></HEAD>");
     out.println(versionID);
     out.println("<script language=\"javascript\" src=\"applet.js\">");
     out.println("</script>");
     out.println("<script language=\"javascript\">");
     out.println("function reset()");
     out.println("{");
     out.println("  document.form1.courseP.selectedIndex= 0;");
     out.println("  document.form1.result.selectedIndex= 0;");
     out.println("  document.form1.player.selectedIndex= 0;");
     out.println("  document.form1.courseV.selectedIndex= 0;");
     out.println("  document.form1.change.selectedIndex= 0;");
     out.println("  document.form1.username.value= \"\";");
     out.println("  document.form1.userpass.value= \"\";");
     out.println("}");
     out.println("</script>");
     out.println("<BODY>");
     out.println("<HR>");
     out.println("<H1 align=\"center\">" + playerName + "</H1>");
     out.println("<HR>");
     out.println("<FORM name=\"form1\">");
     out.println("<TABLE>");
     out.println("  <COLGROUP span=\"5\"></COLGROUP>");
     out.println("  <TBODY>");

     //-----------------------------------------------------------------------
     // postCourse
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    Post Result");
     out.println("    </TD><TD>");
     out.println("    <SELECT name=\"courseP\">");
     out.println("      <OPTION selected>");

     findOption(out, "COURSE");

     out.println("    </SELECT>");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick='postCourse(\"" + playerID + "\",courseP)'>Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // viewResult
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    View Scorecard");
     out.println("    </TD><TD>");
     out.println("    <SELECT name=\"result\">");
     out.println("      <OPTION selected>");

     cardOption(out, playerNN);

     out.println("    </SELECT>");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"viewResult('"
                     + playerNN + "',result)\">Go</BUTTON>");
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
     out.println("    <BUTTON type=BUTTON onclick=\"viewHandicap('"+ playerID + "',player)\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // viewCourse
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    View Course");
     out.println("    </TD><TD>");
     out.println("    <SELECT name=\"courseV\">");
     out.println("      <OPTION selected>");

     findOption(out, "COURSE");

     out.println("    </SELECT>");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"viewCourse(courseV)\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // Change Player
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    Change Player");
     out.println("    </TD><TD>");
     out.println("    <SELECT name=\"change\">");
     out.println("      <OPTION selected>");

     findOption(out, "PLAYER");

     out.println("    </SELECT>");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"changePlayer(change)\">Go</BUTTON>");
     out.println("    </TD>");
     out.println("  </TR>");

     //-----------------------------------------------------------------------
     // Maintenance
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
     out.println("    Maintenance");
     out.println("    </TD><TD>");
     out.println("    <BUTTON type=BUTTON onclick=\"maintPlayer('"+ playerID + "',username,userpass)\">Go</BUTTON>");
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
   // Course request
   // Player?course=<id>
   if( q.startsWith("course=") )
   {
     //-------------------------------------------------------------------------
     // Get courseID, courseNN, and courseName
     String courseID= getParm("course", q);
     if( courseID == null )
     {
       putError(out, "No DEFAULT course");
       return;
     }

     String courseNN= null;
     String courseName= null;

     courseID= dbGet("COURSE.FIND", courseID);
     courseNN= dbGet("COURSE.NICK", courseID);
     courseName= stripQuotes(dbGet("COURSE.SHOW", courseID));

     //-----------------------------------------------------------------------
     // HEADING
     //-----------------------------------------------------------------------
     out.println("<HTML>");
     out.println("<HEAD><TITLE>" + courseName + "</TITLE></HEAD>");
     out.println(versionID);
     out.println("<script language=\"javascript\" src=\"applet.js\">");
     out.println("</script>");
     out.println("<script language=\"javascript\">");
     out.println("function reset()");
     out.println("{");
     out.println("  document.form1.postDate.value= null;");
     out.println("  document.form1.postTime.value= null;");
     out.println("  document.form1.postTeebox.selectedIndex= 0;");
     out.println("}");
     out.println("</script>");
     out.println("<BODY>");
     out.println("<HR>");
     out.println("<H1 align=\"center\">" + courseName + "</H1>");
     out.println("<HR>");
     out.println("<H1 align=\"center\">" + playerName + "</H1>");
     out.println("<HR>");
     out.println("<FORM name=\"form1\">");
     out.println("<TABLE>");
     out.println("  <COLGROUP span=\"5\"></COLGROUP>");
     out.println("  <TBODY>");

     //-----------------------------------------------------------------------
     // Date, time, and teebox selector
     //-----------------------------------------------------------------------
     out.println("  <TR>");
     out.println("    <TD>");
//   out.println("    Select:</BR>");
     out.println("    <SELECT name=\"postTeebox\">");
     out.println("      <OPTION selected>");

     findTeebox(out, courseID);

     out.println("    </SELECT> Teebox</BR></BR>");
     out.println("    <INPUT type=\"TEXT\" NAME=\"postDate\" SIZE=\"12\"> Date</BR></BR>");
     out.println("    <INPUT type=\"TEXT\" NAME=\"postTime\" SIZE=\"12\"> Time</BR></BR>");

     out.println("    <BUTTON type=BUTTON onclick=\"cardPlayer('"+ playerNN + "','"+ courseID + "',postTeebox,postDate,postTime)\">Hole by hole</BUTTON></BR></BR>");
     out.println("    <BUTTON type=BUTTON onclick=\"postPlayer('"+ playerNN + "','"+ courseID + "',postTeebox,postDate,postTime)\">ESC Score</BUTTON></BR></BR>");
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

     //-----------------------------------------------------------------------
     // Validate maintenance request
     //-----------------------------------------------------------------------
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
       if( playerID == null )
         playerID= "DEFAULT";

       //---------------------------------------------------------------------
       // HEADING
       //---------------------------------------------------------------------
       out.println("<HTML>");
       out.println("<HEAD><TITLE>Maintenance Page</TITLE></HEAD>");
       out.println(versionID);
       out.println("<script language=\"javascript\" src=\"applet.js\">");
       out.println("</script>");
       out.println("<script language=\"javascript\">");
       out.println("function reset()");
       out.println("{");
       out.println("  document.form1.player.selectedIndex= 0;");
       out.println("  document.form1.course.selectedIndex= 0;");
       out.println("  document.form1.change.selectedIndex= 0;");
       out.println("}");
       out.println("</script>");
       out.println("<BODY>");
       out.println("<HR>");
       if( playerName != null )
         out.println("<H1 align=\"center\">" + playerName + "</H1>");
       out.println("<H1 align=\"center\">Maintenance Page</H1>");
       out.println("<HR>");
       out.println("<FORM name=\"form1\">");
       out.println("<TABLE>");
       out.println("  <COLGROUP span=\"5\"></COLGROUP>");
       out.println("  <TBODY>");

       //---------------------------------------------------------------------
       // editPlayer
       //---------------------------------------------------------------------
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

       //---------------------------------------------------------------------
       // editCourse
       //---------------------------------------------------------------------
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

       //---------------------------------------------------------------------
       // DATABASE inserts
       //---------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"insertPlayer()\">Add Player </BUTTON>");
       out.println("    </TD></TR><TR><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"insertCourse()\">Add Course</BUTTON>");
       out.println("    </TD></TR><TR><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"setDefaults()\">Set Defaults</BUTTON>");
       out.println("    </TD></TR><TR><TD>");
       out.println("    </TD>");
       out.println("  </TR>");

       //---------------------------------------------------------------------
       // DATABASE controls
       //---------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    <A href=\"/golfer/Player?maint=DBSTOP," +
                   userPass(userName, userPass) + "\">" +
                   "Database STOP</A>");
       out.println("    </TD><TD>");
       out.println("    </TR><TR>");
       out.println("    <TD>");
       out.println("    <A href=\"/golfer/Player?maint=DBSTART," +
                   userPass(userName, userPass) + "\">" +
                   "Database START</A>");
       out.println("    </TD><TD>");
       out.println("    </TR><TR>");
       out.println("    <TD>");
       out.println("    <A href=\"debug.html\">Debugging page</A>");
       out.println("    </TD>");
       out.println("  </TR>");

       //---------------------------------------------------------------------
       // Change Player
       //---------------------------------------------------------------------
       out.println("  <TR>");
       out.println("    <TD>");
       out.println("    Change Player");
       out.println("    </TD><TD>");
       out.println("    <SELECT name=\"change\">");
       out.println("      <OPTION selected>");

       findOption(out, "PLAYER");

       out.println("    </SELECT>");
       out.println("    </TD><TD>");
       out.println("    <BUTTON type=BUTTON onclick=\"changePlayerMaint(change,'" + userName + "','" + userPass + "')\">Go</BUTTON>");
       out.println("    </TD>");
       out.println("  </TR>");

       //---------------------------------------------------------------------
       // FOOTING
       //---------------------------------------------------------------------
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
} // Class PlayerServlet

