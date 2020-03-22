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
//       ControlServlet.java
//
// Purpose-
//       NewsReader Control HttpServlet.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
package usr.fne.newsreader;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Enumeration;
import java.util.Properties;
import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.ServletConfig;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.ServletInputStream;
import javax.servlet.ServletRequest;

import usr.fne.common.LoggingService;

//----------------------------------------------------------------------------
//
// Class-
//       ControlServlet
//
// Purpose-
//       Control HttpServlet.
//
//----------------------------------------------------------------------------
public class ControlServlet extends HttpServlet implements LoggingService
{
//----------------------------------------------------------------------------
// ControlServlet.Attributes
//----------------------------------------------------------------------------
boolean                debug;       // Debugging control
int                    verbose;     // Verbosity control
ServletConfig          config;      // ServletConfig
ServletContext         context;     // ServletContext
int                    visitor;     // Visitor count

//----------------------------------------------------------------------------
// ControlServlet.Attributes
//----------------------------------------------------------------------------
NewsDriver             driver;      // NewsReader Thread
NewsReader             reader;      // NewsReader

//----------------------------------------------------------------------------
//
// Method-
//       ControlServlet.LoggingService
//
// Purpose-
//       Implement the LoggingService.
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
     System.out.println("NewsReaderServlet: " + message);
     if( e != null )
       e.printStackTrace();
   }
   else
   {
     if( e != null )
       context.log("NewsReaderServlet: " + message, e);
     else
       context.log("NewsReaderServlet: " + message);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ControlServlet.init
//
// Purpose-
//       Called when the Servlet is first started for one-time initialization.
//
//----------------------------------------------------------------------------
public void
   init()
   throws ServletException
{
   boolean             error= false;// TRUE iff error encountered
   String              string;      // Working String

   // Get configuration parameters
   config  = getServletConfig();
   context = config.getServletContext();
   log("init()..");

   String  propertyPath = config.getInitParameter("property-path");
   String  propertyFile = config.getInitParameter("property-file");
   String  propertyName = context.getRealPath(propertyPath+"/"+propertyFile);

   // Set defaults
   visitor= 0;

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

   // Start the NewsReader
   try {
     reader= new NewsReader(this, props);
     reader.init();

     driver= new NewsDriver(reader);
     driver.start();
   } catch( Exception e ) {
     error= true;
     log("init: Error starting reader", e);
     throw new ServletException("Initialization failed");
   }

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

     string= "";
     try {
       string= context.getResource("/.").toString();
     } catch( Exception e ) {
       string= e.toString();
     }
     log("x.ResourceName: " + string);
     log("  x.ServerInfo: " + context.getServerInfo());
     log(" x.ContextName: " + context.getServletContextName());
     log("Application:");
     log("    NewsReader: " + (error ? "Failed" : "Ready"));
   }

   if( error )
     throw new ServletException("Initialization failed");

   log("..init()");
}

//----------------------------------------------------------------------------
//
// Method-
//       ControlServlet.doGet
//
// Purpose-
//       Called for each HTTP GET request.
//
//       Since we do not handle GET requests, we generate a canned response.
//
//----------------------------------------------------------------------------
public void
   doGet(                           // Handle HTTP "GET" request
     HttpServletRequest  req,       // Request information
     HttpServletResponse res)       // Response information
   throws ServletException, IOException
{
   String q= req.getQueryString();
   if( debug ) log("doGet("+q+")..");

   res.setContentType("text/html");
   PrintWriter out = res.getWriter();

   out.println("<HTML>");
   out.println("<HEAD><TITLE>NewsReaderServlet GET</TITLE></HEAD>");
   out.println("<BODY>");

   String name = req.getParameter("name");
   if( name == null || name.equals("") )
     name=req.getRemoteHost();
   out.println("<BIG>Hello "+name+"</BIG>");
   out.println("from "+req.getRequestURI()+" on "+req.getHeader("host"));
   synchronized(this) {
     visitor++;
   }
   out.println("<p>You are visitor number " + visitor + "</p>");

   query(req, res);

   out.println("</BODY></HTML>");

   if( debug ) log("..doGet()");
}

//----------------------------------------------------------------------------
//
// Method-
//       ControlServlet.doPost
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
   if( debug ) log("doPost("+q+")..");

   res.setContentType("text/html");
   PrintWriter out = res.getWriter();

   out.println("<HTML>");
   out.println("<HEAD><TITLE>NewsReaderServlet POST</TITLE></HEAD>");
   out.println("<BODY>");

   query(req, res);

   out.println("</BODY></HTML>");
   if( debug ) log("..doPost()");
} 

//----------------------------------------------------------------------------
//
// Method-
//       ControlServlet.destroy
//
// Purpose-
//       Clean up when Servlet is stopped.
//
//----------------------------------------------------------------------------
public void
   destroy()                        // Stop this Servlet
{
   if( debug ) log("destroy()..");

   // Terminate the client Thread
   driver.abandon();

   try {
     driver.join();
   } catch(Exception e) {
     log("destroy()", e);
   }
   driver= null;

   reader.term();

   if( debug ) log("..destroy()");
}

//----------------------------------------------------------------------------
//
// Method-
//       ControlServlet.query
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
   int                 count;       // Generic counter

   String q= req.getQueryString();
   if( debug ) log("query("+q+")..");

   PrintWriter out = res.getWriter();
   if( q == null )
   {
     // Do nothing
   }

   // Query A
   else if( q.equals("A") )
   {
     // Do nothing
   }

   // Query B
   else if( q.startsWith("B=") )
   {
     // Do nothing
   }

   // Invalid request
   else
   {
//// log("Invalid request: "+q);
//// out.println("Invalid request: "+q);
   }

   // Request data display
   out.println("<p>Request information:");
   out.println("<br> AuthType: '"       + req.getAuthType()       + "'");
   out.println("<br> ContentLength: '"  + req.getContentLength()  + "'");
   out.println("<br> ContentType: '"    + req.getContentType()    + "'");
   out.println("<br> ContextPath: '"    + req.getContextPath()    + "'");
   out.println("<br> Encoding: '" + req.getCharacterEncoding()    + "'");
   out.println("<br> isSecure: '"       + req.isSecure()          + "'");
// out.println("<br> LocalAddr: '"      + req.getLocalAddr()      + "'");
// out.println("<br> LocalName: '"      + req.getLocalName()      + "'");
// out.println("<br> LocalPort: '"      + req.getLocalPort()      + "'");
   out.println("<br> Locale: '"         + req.getLocale()         + "'");
   out.println("<br> Method: '"         + req.getMethod()         + "'");
   out.println("<br> PathInfo: '"       + req.getPathInfo()       + "'");
   out.println("<br> PathTranslated: '" + req.getPathTranslated() + "'");
   out.println("<br> Protocol: '"       + req.getProtocol()       + "'");
   out.println("<br> QueryString: '"    + req.getQueryString()    + "'");
   out.println("<br> RemoteUser: '"     + req.getRemoteUser()     + "'");
   out.println("<br> SessionId: '"  + req.getRequestedSessionId() + "'");
   out.println("<br> RemoteAddr: '"     + req.getRemoteAddr()     + "'");
   out.println("<br> RemoteHost: '"     + req.getRemoteHost()     + "'");
// out.println("<br> RemotePort: '"     + req.getRemotePort()     + "'");
   out.println("<br> RequestURI: '"     + req.getRequestURI()     + "'");
   out.println("<br> RequestURL: '"     + req.getRequestURL()     + "'");
   out.println("<br> Scheme: '"         + req.getScheme()         + "'");
   out.println("<br> ServerName: '"     + req.getServerName()     + "'");
   out.println("<br> ServerPort: '"     + req.getServerPort()     + "'");
   out.println("<br> ServletPath: '"    + req.getServletPath()    + "'");
   out.println("<br> UserPrincipal: '"  + req.getUserPrincipal()  + "'");

   out.println("<p> Attributes:");
   count= 0;
   for(Enumeration e= req.getAttributeNames(); e.hasMoreElements();)
   {
     count++;
     String string= (String)e.nextElement();
     out.println("<br> .." + string + "='" + req.getAttribute(string) + "'");
   }
   if( count == 0 )
     out.println("<br> ..NONE");
   out.println("</p>");

   out.println("<p> Cookies:");
   Cookie[] cookies= req.getCookies();
   if( cookies == null )
   {
     out.println("<br> ..NONE");
     cookies= new Cookie[0];
   }
   for(int i= 0; i<cookies.length; i++)
     out.println("<br> ..'" + cookies[i] + "'");
   out.println("</p>");

   out.println("<p> Headers:");
   count= 0;
   for(Enumeration e= req.getHeaderNames(); e.hasMoreElements();)
   {
     count++;
     String string= (String)e.nextElement();
     out.println("<br> .." + string + "='" + req.getHeader(string) + "'");
   }
   if( count == 0 )
     out.println("<br> ..NONE");
   out.println("</p>");

   out.println("<p> Parameters:");
   count= 0;
   for(Enumeration e= req.getParameterNames(); e.hasMoreElements();)
   {
     count++;
     String string= (String)e.nextElement();
     out.print("<br> .." + string + "={'");
     String[] array= req.getParameterValues(string);
     for(int i= 0; i<array.length; i++)
     {
       if( i != 0 )
         out.print("','");
       out.print(array[i]);
     }
     out.println("'}");
   }
   if( count == 0 )
     out.println("<br> ..NONE");
   out.println("</p>");

   // Response data display
   out.println("<p>Response information:");
// out.println("<br> ContentType: '"    + res.getContentType()    + "'");
   out.println("<br> Encoding: '" + res.getCharacterEncoding()    + "'");
   out.println("</p>");

   log("..query()");
} 
} // Class ControlServlet

