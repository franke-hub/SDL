//----------------------------------------------------------------------------
//
//       Copyright (C) 2008-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PlayerView.java
//
// Purpose-
//       View scorecard information.
//
// Last change date-
//       2023/01/19
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.lang.*;
import java.util.*;
import javax.swing.*;

import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       PlayerView
//
// Purpose-
//       View scorecard information.
//
//----------------------------------------------------------------------------
public class PlayerView extends CommonJFrame implements ActionListener {
//----------------------------------------------------------------------------
// PlayerView.Attributes
//----------------------------------------------------------------------------
PlayerView             owner;       // The PlayerView object
boolean                editable;    // The EDITABLE control

// Applet parameters
String                 playerNN;    // Player nickname
String                 playerDate;  // Player date qualifier
String                 playerTime;  // Player time qualifier
String                 playerHdcp;  // Player handicap

// Database items
String                 courseID;    // Course ID
String                 playerID;    // Player ID
String                 playerKey;   // dbRetrieve key
String[]               player_card; // Player score card
String                 teeboxID;    // Teebox name

// GUI items
CourseHdcpInfo         courseHdcp;  // Handicap information (Men's)
CourseHoleInfo         courseHole;  // Course hole information
CourseParsInfo         coursePars;  // Par information (Men's)
GenericItemInfo        courseShow;  // Course display name
CourseTboxInfo         courseTbox;  // Teebox information

GenericItemInfo        playerShow;  // Player display name
PlayerCardInfo         playerCard;  // Player score card information
PlayerNetsInfo         playerNets;  // Player net score card information
PlayerPostInfo         playerPost;  // Player score card information

// Constants
static final int       HOLE_COUNT= FORMAT_BASE;

//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.PlayerView
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PlayerView( )                    // Constructor
{
   super("PlayerView");             // Set applet name
   owner= this;
   editable= false;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.actionPerformed
//
// Purpose-
//       Implement ActionListener, overriden in derived classes.
//
//----------------------------------------------------------------------------
public void
   actionPerformed(ActionEvent e)
{
   throw new RuntimeException("ShouldNotOccur");
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: PlayerView v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display golf score info.";
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info=
   {   {"player-nick", "string", "The name of the player, "
                               + "default is database default"}
   ,   {"player-date", "string", "The score date, "
                               + "default is database default"}
   ,   {"player-time", "string", "The score time, "
                               + "default is first time on date"}
   ,   {"player-hdcp", "string", "The player's handicap, "
                               + "default is computed value for date"}
   ,   {"course-nick", "string", "The course ID, "
                               + "default is database default"}
   ,   {"teebox-name", "string", "The teebox ID, "
                               + "default is database default"}
   };
   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.init
//
// Purpose-
//       Initialize.
//
// Notes-
//       Called when this Applet is loaded into the browser.
//
//----------------------------------------------------------------------------
public void
   init()
{
   initCommon();
   new Loader().execute();
   waitUntilDone();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.loadAppletParameters
//
// Purpose-
//       Called in background to load the Applet parameters.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters()
{
   playerNN=   getParameter("player-nick");
   playerDate= getParameter("player-date");
   playerTime= getParameter("player-time");
   playerHdcp= getParameter("player-hdcp");
   courseID=   getParameter("course-nick");
   teeboxID=   getParameter("teebox-name");

   debug("Param: player-nick: " + playerNN);
   debug("Param: player-date: " + playerDate);
   debug("Param: player-time: " + playerTime);
   debug("Param: player-hdcp: " + playerHdcp);
   debug("Param: course-nick: " + courseID);
   debug("Param: teebox-name: " + teeboxID);

   if( playerDate == null || playerDate.trim().equals("") )
   {
//   Date date= new Date();
//   playerDate= "" + (date.getMonth()+1);
//   playerDate += "/" + date.getDate();
//   playerDate += "/" + (date.getYear() + 1900);
     Calendar date= new GregorianCalendar();
     playerDate= "" + (date.get(Calendar.MONTH)+1);
     playerDate += "/" + date.get(Calendar.DAY_OF_MONTH);
     playerDate += "/" + date.get(Calendar.YEAR);
   }

   playerDate= fixDate(playerDate);
   playerDate= sortDate(playerDate);

   if( playerTime != null )
     playerTime= fixTime(playerTime);
}

//----------------------------------------------------------------------------
//
// Class-
//       PlayerView.loader
//
// Purpose-
//       Load the course and player data.
//
//----------------------------------------------------------------------------
public void
   loader(                          // Load team data
     int               format)      // The Panel format
   throws Exception
{
   debug("PlayerView.loader(" + format + ")");

   if( playerNN == null )
     throw new Exception("Required parameter missing: player-nick");
   if( playerDate == null )
     throw new Exception("Required parameter missing: player-date");

   playerID= dbGet(CMD_PLAYER_FIND, playerNN);
   if( playerID == null )
     throw new Exception("Cannot find PLAYER_ID");
   playerNN= dbGet(CMD_PLAYER_NICK, playerID);
   if( playerNN == null )
     throw new Exception("Cannot find PLAYER_NICK " + playerID);
   playerShow= new GenericItemInfo(dbGet(CMD_PLAYER_SHOW, playerID));

   playerKey= concat(playerNN, playerDate);
   if( playerTime != null )
     playerKey= concat(playerKey, playerTime);

   player_card= dbRetrieve(CMD_PLAYER_CARD, playerKey);
   if( player_card == null )
   {
     // Look for EVENTS_CARD EVENTS_ID.DATE.TIME.PLAYER_NN
     String CMD=  CMD_EVENTS_CARD;
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
       int index3= catcon(item, 3);
       if( index3 < 0 )
         continue;
       int index1= catcon(item, 1);
       int index2= catcon(item, 2);

       String date= item.substring(index1+1, index2);
       if( date.compareTo(playerDate) > 0 )
       {
         NEXT= concat(item.substring(0, index1), "9999/99/99");
         continue;
       }
       else if( date.compareTo(playerDate) < 0 )
       {
         NEXT= concat(item.substring(0, index1), playerDate);
         continue;
       }

       if( playerNN.equalsIgnoreCase(item.substring(index3+1))
            && date.equals(playerDate) )
       {
         String time= item.substring(index2+1, index3);
         if( playerTime != null && !time.equals(playerTime) )
           continue;

         player_card= tokenize(t.remainder());
         playerKey= item;
         break;
       }
     }
   }

   // Calculate player handicap on date
   if( playerHdcp == null || playerHdcp.trim().equals("") )
   {
     playerHdcp= loadPlayerHdcp(playerNN, playerDate); // Load by nickname
     if( playerHdcp == null )
     {
       PlayerPostInfo[] post_info= loadPlayerPostInfo(playerNN, playerDate);
       playerHdcp= "" + loadPlayerHdcp(post_info);
     }
   }

   if( player_card != null )
   {
     courseID= PlayerCardInfo.getCourseID(player_card);
     teeboxID= PlayerCardInfo.getTeeboxID(player_card);
   }
   else if( !editable )
   {
     if( playerTime != null )
       playerDate= playerDate + " at " + playerTime;
     throw new Exception("No scorecard for " + playerNN + " on " + showDate(playerDate));
   }

   if( courseID == null )
     throw new Exception("Missing COURSE_ID for " + playerNN + " on " + playerDate);

   if( teeboxID == null )
     throw new Exception("Missing TEEBOX_ID in scorecard for " + playerNN + " on " + playerDate);

   courseShow= new GenericItemInfo(dbGet(CMD_COURSE_SHOW, courseID));
   courseHole= new CourseHoleInfo(HOLE_COUNT, dbGet(CMD_COURSE_HOLE, courseID));
   courseHdcp= new CourseHdcpInfo(HOLE_COUNT, dbGet(CMD_COURSE_HDCP, courseID));
   coursePars= new CourseParsInfo(HOLE_COUNT, dbGet(CMD_COURSE_PARS, courseID));

   courseTbox=
       new CourseTboxInfo(HOLE_COUNT, courseID, courseShow.toString(), teeboxID,
                          dbGet(CMD_COURSE_TBOX, concat(courseID, teeboxID)));

   playerCard= new PlayerCardInfo(HOLE_COUNT, playerShow.item,
                                  player_card, playerHdcp,
                                  coursePars, courseTbox);
   playerNets= new PlayerNetsInfo(HOLE_COUNT, playerShow.item,
                                  playerCard.array, playerHdcp,
                                  coursePars, courseTbox, courseHdcp);
   playerPost= new PlayerPostInfo(courseTbox, playerCard, playerDate, playerTime,
                                  dbRetrieve(CMD_PLAYER_POST, playerKey));

   courseShow.item= courseShow.item + " -- " + courseTbox.stringMR + "/" + courseTbox.stringMS;
   playerShow.item= playerShow.item + " -- " + playerHdcp + ", " + showDate(playerDate);
   if( playerTime != null )
     playerShow.item= playerShow.item  + " " + playerTime;
}

//----------------------------------------------------------------------------
//
// Class-
//       PlayerView.Format
//
// Purpose-
//       Create a background task to format the data.
//
//----------------------------------------------------------------------------
class Format extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.Format.doInBackground
//
// Purpose-
//       Format the data.
//
// Notes-
//       Called by SwingWorker via execute().
//
//----------------------------------------------------------------------------
@Override
public Object
   doInBackground()
{
   debug("Format.doInBackground()");

   synchronized(owner)
   {{{{
     if( errorString != null )
       return null;

     // Generate the base panels
     courseShow.genPanel();
     courseHdcp.genPanel();
     courseHole.genPanel();
     coursePars.genPanel();
     courseTbox.genPanel();

     DataPanel panel= courseShow.getPanel();
     DataField[] df= panel.getField();
     df[0].setBackground(Color.GREEN.brighter());

     panel= courseHole.getPanel();
     df= panel.getField();
     df[HOLE_ESC].setText("ESC");
     df[HOLE_HCP].setText("Hdcp");
     df[HOLE_NET].setText("Net");

     playerShow.genPanel();
     playerCard.genPanel();
     playerNets.genPanel();
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.Format.done
//
// Purpose-
//       Replace the content.
//
// Notes-
//       Called by SwingWorker in EWT after doInBackground completes.
//
//----------------------------------------------------------------------------
@Override
public void done()
{
   debug("Format.done()");

   synchronized(owner)
   {{{{
     // If error, put message in the loaderPanel
     if( errorString != null )
     {
       loaderPanel.getField(0).setText(errorString);

       content.removeAll();
       content.add(loaderPanel);
       content.revalidate();
       content.repaint();
     }
     else
     {
       content.removeAll();

       content.setLayout(new GridLayout(0,1));
       content.add(courseShow.getPanel());
       content.add(playerShow.getPanel());
       content.add(courseHole.getPanel());
       content.add(courseTbox.getPanel());
       content.add(courseHdcp.getPanel());

       content.add(playerCard.getPanel());
       content.add(coursePars.getPanel());
       content.add(playerNets.getPanel());

       content.revalidate();
       content.repaint();
     }
   }}}} // Synchronized(owner)

   setVisible(true);
   waitingDone();
}
} // class PlayerView.Format

//----------------------------------------------------------------------------
//
// Class-
//       PlayerView.Loader
//
// Purpose-
//       Create a background task to load the data.
//
//----------------------------------------------------------------------------
class Loader extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.Loader.doInBackground
//
// Purpose-
//       Load the data.
//
// Notes-
//       Called by SwingWorker via execute().
//
//----------------------------------------------------------------------------
@Override
public Object
   doInBackground()
{
   debug("Loader.doInBackground()");

   synchronized(owner)
   {{{{
     dbReady();

     // Load the data
     try {
       if( client.fsm != DbStatic.FSM_READY )
         throw new Exception("DbServer offline");

       //-----------------------------------------------------------------------
       // Extract associated data
       loader(HOLE_COUNT);
     } catch( Exception e ) {
       setERROR("Loader: doInBackground: Exception: " + e);
       e.printStackTrace();
     }

     dbReset();
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerView.Loader.done
//
// Purpose-
//       Complete the image load.
//
// Notes-
//       Called by SwingWorker in EWT after doInBackground completes.
//
//----------------------------------------------------------------------------
@Override
public void done( )
{
   debug("Loader.done()");

   synchronized(owner)
   {{{{
     // If error, put message in the loaderPanel
     if( errorString != null )
     {
       loaderPanel.getField(0).setText(errorString);
       content.revalidate();
     }
     else
       new Format().execute();
   }}}} // Synchronized(owner)
}
} // class PlayerView.Loader
} // class PlayerView
