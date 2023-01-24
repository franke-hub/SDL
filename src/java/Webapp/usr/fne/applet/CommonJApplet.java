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
//       CommonJApplet.java
//
// Purpose-
//       JApplet interface (DEPRECATED)
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;

import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       CommonJApplet
//
// Purpose-
//       Edit event information.
//
//----------------------------------------------------------------------------
public class CommonJApplet extends StaticJApplet implements DebuggingInterface {
//----------------------------------------------------------------------------
// CommonJApplet.Attributes
//----------------------------------------------------------------------------
boolean                DEBUG= false;// <DEBUGGING> DEBUG control
boolean                DEBUG_HCDM= false;// <DEBUG_HDCM> Hard Core Debug Mode
boolean                DEBUG_IODM= false;// <DEBUG_IODM> Test, don't output

String                 appletName= "CommonJApplet"; // The name of the applet
JPanel                 content;     // The applet's content panel
DbClient               client;      // The DbClient object
String                 errorString; // The ERROR String
String                 hostName;    // The applet's host name
DataPanel              loaderPanel; // The bringup Panel

//----------------------------------------------------------------------------
//
// Method-
//       CommonJApplet.CommonJApplet
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
protected
   CommonJApplet(                   // Constructor
     String            string)      // The applet name
{
   super();

   appletName= string;              // Set applet name
}

protected
   CommonJApplet(                   // Copy constructor
     CommonJApplet     copy)        // The source CommonJApplet
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
//       CommonJApplet.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface methods.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Object debugging message
{
   print("CommonJApplet.debug()");
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
//       CommonJApplet.UTILITIES
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
//       CommonJApplet.createGUI
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
}

//----------------------------------------------------------------------------
//
// Method-
//       CommonJApplet.initCommon
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
   hostName= getCodeBase().getHost();
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
//       CommonJApplet.loadAppletParameters
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
//       CommonJApplet.loadPlayerHdcp(String)
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
//       CommonJApplet.loadPlayerHdcp(PlayerPostInfo[])
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
//       CommonJApplet.loadPlayerPostInfo
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
//       CommonJApplet.start
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
//       CommonJApplet.stop
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
} // class CommonJApplet
