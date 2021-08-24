//----------------------------------------------------------------------------
//
//       Copyright (C) 2010-2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       AppletServlet.java
//
// Purpose-
//       Applet HttpServlet, invokes specified Applet.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
package usr.fne.golfer;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.URLDecoder;
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
//       AppletServlet
//
// Purpose-
//       Applet HttpServlet.
//
//----------------------------------------------------------------------------
public class AppletServlet extends HttpServlet implements LoggingService
{
//----------------------------------------------------------------------------
// AppletServlet.Attributes
//----------------------------------------------------------------------------
boolean                debug;       // Debugging control
int                    verbose;     // Verbosity control

ServletConfig          config;      // ServletConfig
ServletContext         context;     // ServletContext
DbServer               dbServer;    // Our database local server

static final String    versionID= "VID 2017/03/23 14:00";

//----------------------------------------------------------------------------
//
// Method-
//       AppletServlet.UTILITIES
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
//       AppletServlet.log
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
     System.out.println("AppletServlet: " + message);
     if( e != null )
       e.printStackTrace();
   }
   else
   {
     if( e != null )
       context.log("AppletServlet: " + message, e);
     else
       context.log("AppletServlet: " + message);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       AppletServlet.init
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
//       AppletServlet.doGet
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

   query(req, res);
}

//----------------------------------------------------------------------------
//
// Method-
//       AppletServlet.doPost
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
   out.println("<HEAD><TITLE>AppletServlet POST</TITLE></HEAD>");
   out.println("<BODY>POST NOT EXPECTED</BODY>");
   out.println("</HTML>");
}

//----------------------------------------------------------------------------
//
// Method-
//       AppletServlet.destroy
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
//       AppletServlet.putError
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
//       AppletServlet.query
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

   //=========================================================================
   // Applet.jnlp?classname,title,parm=value,parm=value,...
   int index= q.indexOf(',');
   if( index < 0 || index == (q.length() - 1) )
   {
     putError(out, BOGUS);
     return;
   }
   String invoke= q.substring(0, index);

   q= q.substring(index+1);
   index= q.indexOf(',');
   if( index < 0 )
     index= q.length();
   String title= q.substring(0, index);
   title= java.net.URLDecoder.decode(title, "UTF-8");

   Vector<String> param= new Vector<String>();
   if( index < q.length() )
   {
     q= q.substring(index+1);
     for(;;)
     {
       index= q.indexOf(',');
       if( index < 0 )
         index= q.length();

       String s= q.substring(0, index);
       int x= s.indexOf('=');
       if( x < 0 )
       {
         putError(out, BOGUS);
         return;
       }

       param.add(s);
       if( index >= q.length() )
         break;

       q= q.substring(index+1);
     }
   }

   //-------------------------------------------------------------------------
   // We now have enough information to generate the response
   //-------------------------------------------------------------------------
   res.setContentType("application/x-java-jnlp-file");
   out.println("<?xml version='1.0' encoding='utf-8'?>");
   out.println("<jnlp spec='1.0+' codebase='http://localhost:8080/golfer'>");
   out.println(" <information>");
   out.println("  <title>" + title + "</title>");
   out.println("  <vendor>Frank Eskesen</vendor>");
   out.println("  <description>" + title + "</description>");
   out.println(" </information>");
   out.println(" <security><all-permissions/></security>");
   out.println(" <resources>");
   out.println("  <j2se version='1.7+'/>");
   out.println("  <jar href='applet.jar'/>");
   out.println("  <jar href='jars/common.jar'/>");
   out.println(" </resources>");
   out.println(" <applet-desc main-class='" + invoke + "' name='" + title + "'" +
                " height='90%' width='98%'>");

   for(int i= 0; i<param.size(); i++)
   {
     String s= param.elementAt(i);
     int    x= s.indexOf('=');
     String n= s.substring(0,x);
     String v= s.substring(x+1);
     out.println("  <param name='" + n+ "' value='" + v + "'/>");
   }
   out.println(" </applet-desc>");
   out.println("</jnlp>");
}
} // Class AppletServlet

