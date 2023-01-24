//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       CommonJFrame.java
//
// Purpose-
//       Common JFrame GUI, replaces CommonJApplet
//
// Last change date-
//       2023/01/19
//
//----------------------------------------------------------------------------
import java.applet.Applet;          // JApplet extends this
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.accessibility.Accessible; // JApplet implements this
import javax.swing.*;

import usr.fne.common.Program;
import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       CommonJFrame
//
// Purpose-
//       Edit event information.
//
//----------------------------------------------------------------------------
public class CommonJFrame
   extends JFrame implements DebuggingInterface, Program {
//----------------------------------------------------------------------------
// CommonJFrame.Attributes
//----------------------------------------------------------------------------
boolean                DEBUG= true;  // <DEBUGGING> DEBUG control
boolean                DEBUG_HCDM= true;  // <DEBUG_HDCM> Hard Core Debug Mode
boolean                DEBUG_IODM= false; // <DEBUG_IODM> Test, don't output

String                 appletName= "CommonJFrame"; // The name of the applet
Map<String,String>     arguments;   // The Program arguments
JPanel                 content;     // The applet's content panel
DbClient               client;      // The DbClient object
String                 errorString; // The ERROR String
String                 hostName= "localhost"; // The applet's host name
DataPanel              loaderPanel; // The bringup Panel

boolean                wait_complete= false; // Has active wait completed?

//----------------------------------------------------------------------------
// StaticJApplet.Attributes
//----------------------------------------------------------------------------
// Command lookup types
static final String    CMD_COMMENT=     "##";
static final String    CMD_DEFAULT=     "DEFAULT";
static final String    CMD_COURSE_FIND= "COURSE.FIND";
static final String    CMD_EVENTS_FIND= "EVENTS.FIND";
static final String    CMD_PLAYER_FIND= "PLAYER.FIND";

// Command types requiring associated identifier
static final String    CMD_COURSE_HDCP= "COURSE.HDCP";
static final String    CMD_COURSE_HOLE= "COURSE.HOLE";
static final String    CMD_COURSE_HTTP= "COURSE.HTTP";
static final String    CMD_COURSE_LONG= "COURSE.LONG";
static final String    CMD_COURSE_NAME= "COURSE.NAME";
static final String    CMD_COURSE_NEAR= "COURSE.NEAR";
static final String    CMD_COURSE_PARS= "COURSE.PARS";
static final String    CMD_COURSE_SHOW= "COURSE.SHOW";
static final String    CMD_COURSE_TBOX= "COURSE.TBOX";

static final String    CMD_EVENTS_CARD= "EVENTS.CARD";
static final String    CMD_EVENTS_DATE= "EVENTS.DATE";
static final String    CMD_EVENTS_HDCP= "EVENTS.HDCP";
static final String    CMD_EVENTS_LONG= "EVENTS.LONG";
static final String    CMD_EVENTS_NAME= "EVENTS.NAME";
static final String    CMD_EVENTS_NEAR= "EVENTS.NEAR";
static final String    CMD_EVENTS_PLAY= "EVENTS.PLAY";
static final String    CMD_EVENTS_POST= "EVENTS.POST";
static final String    CMD_EVENTS_SHOW= "EVENTS.SHOW";
static final String    CMD_EVENTS_TEAM= "EVENTS.TEAM";
static final String    CMD_EVENTS_TIME= "EVENTS.TIME";

static final String    CMD_PLAYER_CARD= "PLAYER.CARD";
static final String    CMD_PLAYER_HDCP= "PLAYER.HDCP";
static final String    CMD_PLAYER_MAIL= "PLAYER.MAIL";
static final String    CMD_PLAYER_NAME= "PLAYER.NAME";
static final String    CMD_PLAYER_NICK= "PLAYER.NICK";
static final String    CMD_PLAYER_POST= "PLAYER.POST";
static final String    CMD_PLAYER_SHOW= "PLAYER.SHOW";

// Command CMD_DEFAULT codes
static final String    DEFAULT_CODE_DT= "DATE_TIME";
static final String    DEFAULT_CODE_CI= "COURSE_ID";
static final String    DEFAULT_CODE_TI= "TEEBOX_ID";
static final String    DEFAULT_CODE_EI= "EVENTS_ID";
static final String    DEFAULT_CODE_PI= "PLAYER_ID";

// Field constants
static final String    FIELD_DP=  DataField.FIELD_DP; // Double par data entry
static final String    FIELD_ERR= DataField.FIELD_ERR; // Field error

// Format constants
static final int       FORMAT_USER= HolePanel.FORMAT_USER;
static final int       FORMAT_BASE= HolePanel.FORMAT_BASE;
static final int       FORMAT_EVNT= HolePanel.FORMAT_EVNT;
static final int       FORMAT_SKIN= HolePanel.FORMAT_SKIN;

// Hole constants
static final int       HOLE_ID=  HolePanel.HOLE_ID;  // Hole title
static final int       HOLE_OUT= HolePanel.HOLE_OUT; // OUT index
static final int       HOLE_IN=  HolePanel.HOLE_IN;  // IN  index
static final int       HOLE_TOT= HolePanel.HOLE_TOT; // TOT index
static final int       HOLE_ESC= HolePanel.HOLE_ESC; // ESC index
static final int       HOLE_HCP= HolePanel.HOLE_HCP; // HCP index
static final int       HOLE_NET= HolePanel.HOLE_NET; // NET index

static final int       HOLE_LDO= HolePanel.HOLE_LDO; // LDO index (Longest Drive OUT)
static final int       HOLE_CPO= HolePanel.HOLE_CPO; // CPO index (Closest to Pin OUT)
static final int       HOLE_LDI= HolePanel.HOLE_LDI; // LDI index (Longest Drive IN)
static final int       HOLE_CPI= HolePanel.HOLE_CPI; // CPI index (Closest to Pin IN)

static final int       HOLE_FWH= HolePanel.HOLE_FWH; // FWH index (Fairways hit)
static final int       HOLE_GIR= HolePanel.HOLE_GIR; // GIR index (Greens in Regulation)

static final int       HOLE_SKIN= HolePanel.HOLE_SKIN; // SKINS index

// Utility constants
static final String[]  empty= null; // Differentiates String and String[] null

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.CommonJFrame
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
protected
   CommonJFrame(                    // Constructor
     String            string)      // The applet name
{
   super();
   appletName= string;              // Set applet name

   setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
   setMaximizedBounds(null);
   setExtendedState(Frame.MAXIMIZED_BOTH);
}

protected
   CommonJFrame(                    // Copy constructor
     CommonJFrame      copy)        // The source CommonJFrame
{
   this(copy.appletName);

   DEBUG= copy.DEBUG;               // Copy debugging controls
   DEBUG_HCDM= copy.DEBUG_HCDM;
   DEBUG_IODM= copy.DEBUG_IODM;

   client= copy.client;
   hostName= copy.hostName;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface methods.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Object debugging message
{
   print("CommonJFrame.debug()");
   print(".appletName: " + appletName);
   print(".errorString: " + errorString);
}

public boolean                      // TRUE iff debug should write
   isDebug( )                       // Is debugging active?
{
   return DEBUG;
}

public void
   print(                           // Debugging message
     String            string)      // The message String
{
   System.out.println(appletName + ": " + string);
}

public void
   debug(                           // Debugging message
     String            string)      // The message String
{
   if( isDebug() )
     print(string);
}

public void
   error(                           // Debugging error message
     String            string)      // The message String
{
   System.err.println(appletName + ": " + string);
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.getParameter
//
// Purpose-
//       Extract parameter
//
//----------------------------------------------------------------------------
public String                       // Tye parameter value (null if undefined)
   getParameter(                    // Get parameter value
     String            parm)        // For this parameter name
{
   return arguments.get(parm);
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.init
//
// Purpose-
//       Initialize. (Overridden in subclass.)
//
//----------------------------------------------------------------------------
public void
   init()
{  }

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.run
//
// Purpose-
//       Implement Program interface
//
//----------------------------------------------------------------------------
public void
   run(
     String[]          args)
{
   arguments= new HashMap<>();
   for(int i= 0; i<args.length; ++i) {
     int x= args[i].indexOf('=');
     if( x > 0 ) {
       String n= args[i].substring(0,x);
       String v= args[i].substring(x+1);
       arguments.put(n,v);
     }
   }

   init();
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
public void
   setERROR(                        // Set the error message
     String            string)      // The message String
{
   errorString= string;
   error(string);
}

public String                       // The database item
   dbGet(                           // Get database item
       String          type,        // The item type
       String          item)        // The item name
{
   boolean onReset= dbReady();
   String result= client.get(type, item);
   dbReset(onReset);

   return result;
}

public String                       // The database item
   dbNext(                          // Get next database item
       String          type,        // The item type
       String          item)        // The item name
{
   boolean onReset= dbReady();
   String result= client.next(type, item);
   dbReset(onReset);

   return result;
}

public String                       // The prior data
   dbPut(                           // Put database item
       String          type,        // The item type
       String          item,        // The item name
       String          data)        // The item data
{
   String result= null;

   if( DEBUG_IODM )
   {
     debug("DBPUT(" + type + ") item(" + item + ") data(" + data + ")");
     String key= type + client.SEP + item;
     client.cache.put(key.toUpperCase(), data);
   }
   else
   {
     try {
       boolean onReset= dbReady();
       result= client.put(type, item, data);
       dbReset(onReset);
     } catch(Exception e) {
       debug("DBPUT(" + type + ") item(" + item + ") Exception: " + e);
     }
   }

   return result;
}

public boolean                      // TRUE iff was made ready
   dbReady( )                       // Ready the client
{
   boolean result= client.ready(hostName);
   if( result )
     debug("" + result + "= dbReady()");

   return result;
}


public String                       // The prior data
   dbRemove(                        // Remove database item
       String          type,        // The item type
       String          item)        // The item name
{
   String result= null;

   if( DEBUG_IODM )
   {
     debug("DBREMOVE(" + type + ") item(" + item + ")");
     String key= type + client.SEP + item;
     client.cache.put(key.toUpperCase(), null);
   }
   else
   {
     try {
       boolean onReset= dbReady();
       result= client.remove(type, item);
       dbReset(onReset);
     } catch(Exception e) {
       debug("DBREMOVE(" + type + ") item(" + item + ") Exception: " + e);
     }
   }

   return result;
}

public void
   dbReset(                         // Reset the client
     boolean           ready)       // Result from ready
{
   if( ready )
   {
     debug("dbReset(" + ready + ")");
     client.reset();
   }
}

public void
   dbReset( )                       // Reset the client
{
   dbReset(true);
}

public String[]                     // The result String
   dbRetrieve(                      // Retrieve database information
     String            command,     // Database command
     String            item,        // Database item
     String            qual)        // Qualifier (may be NULL)
{
   String[]            result= null;
   String              string= null;

   boolean onReset= dbReady();

   if( qual != null )
   {
     string= client.get(command, concat(item, qual));
     if( string != null )
       result= tokenize(string);
   }

   if( result == null )
   {
     string= client.get(command, item);
     if( string != null )
       result= tokenize(string);
   }

   dbReset(onReset);

   return result;
}

public String[]                     // The result String
   dbRetrieve(                      // Retrieve database information
     String            command,     // Database command
     String            item)        // Database item
{
   return dbRetrieve(command, item, null);
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.createGUI
//
// Purpose-
//       Create the GUI.
//
// Notes-
//       Invoke only from EDT.
//
//----------------------------------------------------------------------------
protected void
   createGUI()
{
   debug("createGUI()");

   // Set the Layout
   setLayout(new GridLayout(1,1));

   // Create the content
   content= new DataPanel(0);
   add(content);

   // Create the Loading Panel
   loaderPanel= new ItemPanel("Loading");
   content.add(loaderPanel);
// setVisible(true);
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.initCommon
//
// Purpose-
//       Common initialization functions.
//
//----------------------------------------------------------------------------
protected void
   initCommon( )
{
   // Initialize debugging controls
   String string= getParameter("DEBUGGING");
   if( string != null )
     DEBUG= Boolean.parseBoolean(string);

   string= getParameter("DEBUG_HCDM");
   if( string != null )
     DEBUG_HCDM= Boolean.parseBoolean(string);

   string= getParameter("DEBUG_IODM");
   if( string != null )
     DEBUG_IODM= Boolean.parseBoolean(string);

   // Initialize
   debug("initCommon()");
   loadAppletParameters();
// hostName= getCodeBase().getHost();
   if( DEBUG_IODM )
   {
     client= new DbClient(null)
     {
        public String                       // The prior data
           put(                             // Put database item
             String            type,        // The item type
             String            item,        // The item name
             String            data)        // The item data
        {
           return dbPut(type, item, data);
        }

        public String                       // The prior data
           remove(                          // Remove database item
             String            type,        // The item type
             String            item)        // The item name
        {
           return dbRemove(type, item);
        }
     }; // new DbClient(...)
   } // if( DEBUG_IODM)
   else
     client= new DbClient(null);

   // Execute a job on the event-dispatching thread,
   // creating this applet's GUI.
   try {
       SwingUtilities.invokeAndWait(new Runnable() {
           public void run() {
               createGUI();
           }
       });
   } catch (Exception e) {
       e.printStackTrace();
       setERROR("init: createGUI Exception: " + e);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.loadAppletParameters
//
// Purpose-
//       Placeholder for subclass functions. Does nothing.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters( )          // Load JApplet parameters
{
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.loadPlayerHdcp(String)
//
// Purpose-
//       Calculate player handicap.
//
//----------------------------------------------------------------------------
public String                       // The resultant handicap
   loadPlayerHdcp(                  // Calculate handicap
     String            playerNN,    // The Player nickname
     String            playDate)    // The sort format date (may be NULL)
   throws Exception
{
   String              handicap= null; // Resultant

   if( playDate == null || playDate.trim().equals("") ) // If no date
   {
//   Date date= new Date();
//   playDate= "" + (date.getMonth()+1);
//   playDate += "/" + date.getDate();
//   playDate += "/" + (date.getYear() + 1900);
     Calendar date= new GregorianCalendar();
     playDate= "" + (date.get(Calendar.MONTH)+1);
     playDate += "/" + date.get(Calendar.DAY_OF_MONTH);
     playDate += "/" + date.get(Calendar.YEAR);

     playDate= fixDate(playDate);
     playDate= sortDate(playDate);
   }

   String playerID= dbGet(CMD_PLAYER_FIND, playerNN);
   if( playerID == null )
     throw new Exception("Cannot find PLAYER_ID");
   playerNN= dbGet(CMD_PLAYER_NICK, playerID);
   if( playerNN == null )
     throw new Exception("Cannot find PLAYER_NICK " + playerID);

   // Look for PLAYER.HDCP PLAYER_ID{.DATE}
   String CMD=  CMD_PLAYER_HDCP;
   String NEXT= "";
   for(;;)
   {
     String string= dbNext(CMD, NEXT);
     if( string == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(string);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(CMD) )
       break;

     NEXT= item;

     String itemPlayer= item;
     int index1= catcon(item, 1);
     if( index1 >= 0 )
     {
       String date= item.substring(index1+1);
       if( date.compareTo(playDate) > 0 )
         break;

       itemPlayer= item.substring(0, index1);
     }

     if( itemPlayer.equals(playerID) || itemPlayer.equals(playerNN) )
       handicap= t.remainder();
   }

   return handicap;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.loadPlayerHdcp(PlayerPostInfo[])
//
// Purpose-
//       Calculate player handicap.
//
//----------------------------------------------------------------------------
public static double                // The resultant handicap
   loadPlayerHdcp(                  // Calculate handicap
     PlayerPostInfo[]  round_info)  // The PlayerPostInfo, sorted by date
{
   // Calculate handicap
   int size= round_info.length;
   if( size > 20 )
     size= 20;

   double handicap= 0.0;
   double[] dArray= new double[size]; // Differential array
   for(int i= 0; i<size; i++)
     dArray[i]= round_info[i].courseDiff();

   int uMax= 0; // The number of scoring rounds to be counted
   if( size > 19 )
     uMax= 10;
   else if( size > 18 )
     uMax= 9;
   else if( size > 17 )
     uMax= 8;
   else if( size > 16 )
     uMax= 7;
   else if( size > 14 )
     uMax= 6;
   else if( size > 12 )
     uMax= 5;
   else if( size > 10 )
     uMax= 4;
   else if( size >  8 )
     uMax= 3;
   else if( size >  6 )
     uMax= 2;
   else if( size >  4 )
     uMax= 1;

   if( uMax > 0 )
   {
     int[] lowest= new int[uMax]; // INDEX of lowest value
     int used= 0;
     for(int i= 0; i<size; i++)
     {
       if( used < uMax )
       {
         lowest[used++]= i;
         continue;
       }

       double maxValue= dArray[lowest[0]];
       int    maxIndex= 0;
       for(int j= 1; j<uMax; j++)
       {
         int x= lowest[j];
         if( dArray[x] > maxValue )
         {
           maxValue= dArray[x];
           maxIndex= j;
         }
       }

       if( dArray[i] < maxValue )
         lowest[maxIndex]= i;
     }

     double diff= 0.0;
     for(int i= 0; i<uMax; i++)
     {
       int x= lowest[i];
       round_info[x].used= true;
       diff += dArray[x];
     }
     diff /= (double)uMax;
     diff *= 9.6;
     diff= (double)((int)diff);
     diff /= 10.0;
     handicap= diff;
   }

   return handicap;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.loadPlayerPostInfo
//
// Purpose-
//       Extract PlayerPostInfo for player up until specified date.
//
// Notes-
//       Resultant is sorted by date/time; newest first.
//       Should only be called in background.
//
//----------------------------------------------------------------------------
public PlayerPostInfo[]             // Resultant PlayerPostInfo[]
   loadPlayerPostInfo(              // Load PlayerPostInfo
     String            playerNN,    // For this PLAYER_NN
     String            untilDate)   // Up until this date

{
   Vector<CourseTboxInfo> tboxVector= new Vector<CourseTboxInfo>();
   Vector<PlayerPostInfo> postVector= new Vector<PlayerPostInfo>(); // Result Vector

   CourseTboxInfo defaultCourse= new CourseTboxInfo(FORMAT_USER);

   // Load the PLAYER data
   String CMD=  CMD_PLAYER_POST;
   String NEXT= playerNN;
   for(;;)
   {
     String text= dbNext(CMD, NEXT);
     if( text == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(text);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(CMD) )
       break;

     NEXT= item;
     int index1= catcon(item, 1);
     if( index1 < 0 )
       continue;

     String player= item.substring(0, index1);
     if( !player.equalsIgnoreCase(playerNN) )
       break;

     String date= item.substring(index1+1);
     String time= "";
     int index2= catcon(item, 2);
     if( index2 > 0 )
     {
       date= item.substring(index1+1, index2);
       time= item.substring(index2+1);
     }

     if( untilDate != null && date.compareTo(untilDate) > 0 )
       break;

     String[] df= tokenize(t.remainder());
     if( df.length != 5 )
       continue;

     String courseID= df[3];
     String teeboxID= df[4];

     CourseTboxInfo tboxInfo= new CourseTboxInfo(FORMAT_USER);
     tboxInfo.courseID= courseID;
     tboxInfo.title= teeboxID;
     if( tboxVector.contains(tboxInfo) )
       tboxInfo= tboxVector.elementAt(tboxVector.indexOf(tboxInfo));
     else
     {
       String courseShow= stripQuotes(dbGet(CMD_COURSE_SHOW, courseID));
       String[] courseHole= dbRetrieve(CMD_COURSE_TBOX, concat(courseID, teeboxID));

       tboxInfo= new CourseTboxInfo(FORMAT_USER, courseID, courseShow,
                                    teeboxID, courseHole);
       tboxVector.add(tboxInfo);
     }

     PlayerPostInfo postInfo=
         new PlayerPostInfo(tboxInfo, null, date, time, df);

     postVector.add(postInfo);
   }

   // Load the EVENTS data
   CMD=  CMD_EVENTS_POST;
   NEXT= "";
   for(;;)
   {
     String text= dbNext(CMD, NEXT);
     if( text == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(text);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(CMD) )
       break;

     NEXT= item;
     int index3= catcon(item, 3);
     if( index3 < 0 )
       continue;
     int index2= catcon(item, 2);
     int index1= catcon(item, 1);

     String player= item.substring(index3+1);
     if( !player.equalsIgnoreCase(playerNN) )
       continue;

     String date= item.substring(index1+1, index2);
     String time= item.substring(index2+1, index3);

     if( untilDate != null && date.compareTo(untilDate) > 0 )
     {
       NEXT= concat(item.substring(0, index1), "9999/12/31");
       continue;
     }

     String[] df= tokenize(t.remainder());
     if( df.length != 5 )
       continue;

     String courseID= df[3];
     String teeboxID= df[4];

     CourseTboxInfo tboxInfo= new CourseTboxInfo(FORMAT_USER);
     tboxInfo.courseID= courseID;
     tboxInfo.title= teeboxID;
     if( tboxVector.contains(tboxInfo) )
       tboxInfo= tboxVector.elementAt(tboxVector.indexOf(tboxInfo));
     else
     {
       String courseShow= stripQuotes(dbGet(CMD_COURSE_SHOW, courseID));
       String[] courseHole= dbRetrieve(CMD_COURSE_TBOX, concat(courseID, teeboxID));

       tboxInfo= new CourseTboxInfo(FORMAT_USER, courseID, courseShow,
                                    teeboxID, courseHole);
       tboxVector.add(tboxInfo);
     }

     PlayerPostInfo postInfo=
         new PlayerPostInfo(tboxInfo, null, date, time, df);

     postVector.add(postInfo);
   }

   // Convert vector to array
   PlayerPostInfo[] result= new PlayerPostInfo[postVector.size()];
   for(int i= 0; i<postVector.size(); i++)
     result[i]= postVector.elementAt(i);

   // Sort by date and time
   for(int i= 0; i<result.length; i++)
   {
     PlayerPostInfo iInfo= result[i];
     for(int j= i+1; j<result.length; j++)
     {
       PlayerPostInfo jInfo= result[j];
       boolean swap= false;
       if( iInfo.date.compareTo(jInfo.date) < 0 )
         swap= true;
       else if( iInfo.date == jInfo.date
                && iInfo.time.compareTo(jInfo.time) < 0 )
         swap= true;

       if( swap )
       {
         result[j]= iInfo;
         result[i]= iInfo= jInfo;
       }
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.start
//
// Purpose-
//       Start execution.
//
// Purpose-
//       Called by the browser or applet viewer to inform this Applet that
//       it should begin its execution. It is called after the init method
//       and each time the Applet is revisited in a web page.
//
//----------------------------------------------------------------------------
public void
   start()
{
   debug("start()");
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJFrame.stop
//
// Purpose-
//       Stop execution.
//
// Purpose-
//       Called by the browser or applet viewer to inform this Applet that
//       it should stop its execution. It is called when the web page that
//       contains the applet has been replaced by onother page, and also
//       just before the applet is to be destroyed
//
//----------------------------------------------------------------------------
public void
   stop()
{
   debug("stop()");
}

//----------------------------------------------------------------------------
//
// Method-
//       waitingDone
//
// Purpose-
//       Indicate wait complete
//
//----------------------------------------------------------------------------
public void
   waitingDone()
{
   setVisible(true);
   wait_complete= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       waitUntilDone
//
// Purpose-
//       Wait for waitingDone invocation
//
// Implementation notes-
//       Poor exception handling. Maybe more is needed
//
//----------------------------------------------------------------------------
public void
   waitUntilDone()
{
   try {                            // Thread.sleep() needs Throwable handler
     for(;;) {
       if( wait_complete )
         break;
       Thread.sleep(125);
     }
   } catch(Exception e) {
     print("Exception: " + e);
     // throw e; // waitUntilComplete(); // Pick one or code something else
   } catch(Throwable t) {
     print("Throwable: " + t);
     // throw t; // waitUntilComplete(); // Pick one or code something else
   }

   wait_complete= false;            // Clear for next time
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
public static int                   // The concatenator index
   catcon(                          // Inverse concatenate
     String            item,        // The item
     int               index)       // The item index
{
   return DbStatic.catcon(item, index);
}

public static int                   // The concatenator index
   catcon(                          // Inverse concatenate
     String            item)        // The item
{
   return DbStatic.catcon(item);
}

public static String                // The concatenated String
   concat(                          // Concatenate
     String            item,        // The item
     String            qual)        // The qualifier
{
   return DbStatic.concat(item, qual);
}

public static double                // ABSOLUTE(inp)
   fabs(                            // Return absolute value
     double            inp)         // For this value
{
   if( inp < 0 )
     inp= (-inp);

   return inp;
}

public static int
   max(int L, int R)
{
   if( L > R )
     R= L;
   return R;
}

public static int
   min(int L, int R)
{
   if( L < R )
     R= L;
   return R;
}

public static String                // Resultant
   stripQuotes(                     // Strip quotes
     String            string)      // From this String
{
   return DbStatic.stripQuotes(string);
}

public static String[]              // Resultant
   tokenize(                        // Tokenize
     String            string)      // This String
{
   String[] result= null;

   if( string != null )
     result= DbStatic.tokenize(string);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.fieldColor
//
// Purpose-
//       Return color based on data value.
//
//----------------------------------------------------------------------------
public static Color                 // Resultant Color
   fieldColor(                      // Get Color for
     int               value)       // This value
{
   Color result= Color.BLACK;
   if( value < 0 )
     result= Color.RED;
   else if( value == 0 )
     result= Color.GREEN.darker();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.fixDate
//
// Purpose-
//       Repair a date field.
//
//----------------------------------------------------------------------------
public static String                // Date (Format MM/DD/YYYY)
   fixDate(                         // Fix a date string
     String            date)        // Date (Format M/D/YY)
{
   int index1= date.indexOf('/');
   if( index1 < 0 )
     return "12/31/9999";

   int index2= date.substring(index1+1).indexOf('/');
   if( index2 < 0 )
     return "12/31/9999";

   index2 += index1 + 1;
   String mm= date.substring(0, index1);
   String dd= date.substring(index1+1, index2);
   String yyyy= date.substring(index2+1);

   if( yyyy.length() == 2 )
     yyyy= "20" + yyyy;

   while( mm.length() < 2 )
     mm= "0" + mm;

   while( dd.length() < 2 )
     dd= "0" + dd;

   while( yyyy.length() < 4 )
     yyyy= "0" + yyyy;

   while( mm.length() > 2 )
     mm= mm.substring(1);

   while( dd.length() > 2 )
     dd= dd.substring(1);

   while( yyyy.length() > 4 )
     yyyy= yyyy.substring(1);

   return mm + "/" + dd + "/" + yyyy;
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.fixTime
//
// Purpose-
//       Repair a time field
//
//----------------------------------------------------------------------------
public static String                // Date (Format HH:MM)
   fixTime(                         // Fix a time string
     String            time)        // Time (Format H:M)
{
   int index1= time.indexOf(':');
   if( index1 < 0 )
     return "00:00";

   int index2= time.substring(index1+1).indexOf(':');
   if( index2 > 0 )
     time= time.substring(0, index1 + index2 + 1);;

   String hh= time.substring(0, index1);
   String mm= time.substring(index1+1);

   while( hh.length() < 2 )
     hh= "0" + hh;

   while( mm.length() < 2 )
     mm= "0" + mm;

   while( hh.length() > 2 )
     hh= hh.substring(1);

   while( mm.length() > 2 )
     mm= mm.substring(1);

   return hh + ":" + mm;
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.showDate
//
// Purpose-
//       Convert a SortDate into a ShowDate.
//
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.sortDate
//
// Purpose-
//       Convert a ShowDate into a SortDate.
//
//----------------------------------------------------------------------------
public static String                // Date (Format YYYY/MM/DD)
   sortDate(                        // ShowDate => SortDate
     String            date)        // Date (Format MM/DD/YYYY)
{
   if( date.length() < 10 )
     return date;

   String head= date.substring(0,10);
   String tail= "";
   if( date.length() > 10 )
     tail= date.substring(10);

   return head.substring(6) + "/" + head.substring(0,5) + tail;
}
} // class CommonJFrame
