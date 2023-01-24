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
//       PlayerHdcp.java
//
// Purpose-
//       Display handicap data for player.
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
//       PlayerHdcp
//
// Purpose-
//       Display handicap data for player.
//
//----------------------------------------------------------------------------
public class PlayerHdcp extends CommonJFrame {

//----------------------------------------------------------------------------
// PlayerHdcp.Attributes
//----------------------------------------------------------------------------
int                    MAX_DISPLAY; // The maximum display count

DataPanel              eventsPanel; // The events show Panel
DataPanel              playerPanel; // The player show Panel
DataPanel              headTitle;   // The overview heading Panel
DataPanel              headPanel;   // The overview data Panel
DataPanel              hdcpTitle;   // The handicap title Panel
DataPanel[]            hdcpPanel;   // The handicap Panels

// Internal data areas
String                 eventsID;    // Events ID
String                 eventsNN;    // Events nickname
GenericListInfo        eventsHdcp;  // Events handicap info
GenericItemInfo        eventsShow;  // Events name

String                 playerID;    // Player ID
String                 playerNN;    // Player nickname
String                 playerDate;  // Newest date to consider
GenericItemInfo        playerShow;  // Player name

//----------------------------------------------------------------------------
//
// Method-
//       PlayerHdcp.PlayerHdcp
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PlayerHdcp( )
{
   super("PlayerHdcp");
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerHdcp.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: PlayerHdcp v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Player handicap information.";
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerHdcp.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info=
   {   {"player-nick", "string", "The player's name, "
                               + "default is database default"}
   ,   {"player-date", "string", "The oldest score date to consider, "
                               + "default is consider all scores"}
   ,   {"events-nick", "string", "The event's name, "
                               + "default is NONE"}
   ,   {"MAX_DISPLAY", "string", "The maximum number of entries to display, "
                               + "default is ALL"}
   };
   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerHdcp.init
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
   new Worker().execute();
   waitUntilDone();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerHdcp.loadAppletParameters
//
// Purpose-
//       Called in background to load the Applet parameters.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters()
{
   playerNN= getParameter("player-nick");
   playerDate= getParameter("player-date");
   String MAX_DISPLAY= getParameter("MAX_DISPLAY");
   eventsNN= getParameter("events-nick");

   debug("Param: player-nick: " + playerNN);
   debug("Param: player-date: " + playerDate);
   debug("Param: MAX_DISPLAY: " + MAX_DISPLAY);
   debug("Param: events-nick: " + eventsNN);

   this.MAX_DISPLAY= Integer.MAX_VALUE;
   try {
     this.MAX_DISPLAY= Integer.parseInt(MAX_DISPLAY);
   } catch(Exception e) {
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       PlayerHdcp.Worker
//
// Purpose-
//       Create a background task to load the images.
//
//----------------------------------------------------------------------------
class Worker extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       PlayerHdcp.Worker.doInBackground
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
   debug("Worker.doInBackground()");

   dbReady();

   // Load the data
   try {
     if( client.fsm != DbStatic.FSM_READY )
       throw new Exception("DbServer offline");

     //-----------------------------------------------------------------------
     // Extract associated data
     String playerID= dbGet(CMD_PLAYER_FIND, playerNN);
     if( playerID == null )
       throw new Exception("Cannot find PLAYER_ID");
     playerID= dbGet(CMD_PLAYER_FIND, playerID);
     if( playerID == null )
       throw new Exception("Invalid PLAYER_ID.ID " + playerID);
     playerNN= dbGet(CMD_PLAYER_NICK, playerID);
     if( playerNN == null )
       throw new Exception("Invalid PLAYER_ID.NN " + playerID);
     playerShow= new GenericItemInfo(dbGet(CMD_PLAYER_SHOW, playerID));
     String playerHdcp= dbGet(CMD_PLAYER_HDCP, playerNN);
     if( playerHdcp == null )
       playerHdcp= dbGet(CMD_PLAYER_HDCP, playerID);

     String eventsHdcp= "";
     eventsID= dbGet(CMD_EVENTS_FIND, eventsNN);
     if( eventsID != null )
     {
       eventsShow= new GenericItemInfo(dbGet(CMD_EVENTS_SHOW, eventsID));
       eventsHdcp= dbGet(CMD_EVENTS_HDCP, concat(eventsID, playerNN));
       if( eventsHdcp == null )
         eventsHdcp= dbGet(CMD_EVENTS_HDCP, concat(eventsID, playerID));
     }

     // Collect POST data, calculate handicap
     PlayerPostInfo[] post_info= loadPlayerPostInfo(playerNN, playerDate);
     double handicap= loadPlayerHdcp(post_info);

     //-----------------------------------------------------------------------
     // Base panels
     if( eventsID != null )
     {
       eventsPanel= eventsShow.genPanel();
       eventsPanel.getField(0).setBackground(Color.GREEN.brighter());
     }
     playerPanel= playerShow.genPanel();

     headTitle= new DataPanel(2);
     DataField[] df= headTitle.getField();
     df[0].setColumns(50);
     df[0].setText(playerDate == null ? "As computed today" : "As of " + showDate(playerDate));
     df[1].setColumns(50);
     df[1].setText(eventsID == null ? "Player handicap" : "Event handicap");

     headPanel= new DataPanel(2);
     df= headPanel.getField();
     df[0].setColumns(50);
     df[0].setText("" + handicap);
     df[1].setColumns(50);
     df[1].setText("" + (eventsID == null ? playerHdcp : eventsHdcp));

     // Handicap data panels
     hdcpTitle= new HdcpPanel("Used", "Type", "Date", "ESC", "Rate", "Slope", "Diff", "Course");
     if( MAX_DISPLAY > post_info.length )
       MAX_DISPLAY= post_info.length;
     hdcpPanel= new HdcpPanel[MAX_DISPLAY];
     for(int i= 0; i<MAX_DISPLAY; i++)
     {
       PlayerPostInfo info= post_info[i];
       hdcpPanel[i]= new HdcpPanel(info.used ? "*" : "",
                                   info.type,
                                   info.date,
                                   info.score,
                                   info.rating,
                                   info.slope,
                                   "" + info.courseDiff(),
                                   info.tboxInfo.courseShow);
     }
   } catch( Exception e ) {
     setERROR("Worker: doInBackground: Exception: " + e);
     e.printStackTrace();
     return null;
   }

   dbReset();

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerHdcp.Worker.done
//
// Purpose-
//       Complete the image load.
//
// Notes-
//       Called by SwingWorker in EWT after doInBackground completes.
//
//----------------------------------------------------------------------------
@Override
public void done()
{
   debug("Worker.done()");

   // If error, put message in the loaderPanel
   if( errorString != null )
   {
     loaderPanel.getField(0).setText(errorString);
     content.revalidate();
   }
   else
   {
     content.removeAll();
     loaderPanel= null;

     content.setLayout(new GridLayout(0,1));
     if( eventsPanel != null )
       content.add(eventsPanel);
     content.add(playerPanel);
     content.add(headTitle);
     content.add(headPanel);
     content.add(hdcpTitle);
     for(int i= 0; i<hdcpPanel.length; i++)
       content.add(hdcpPanel[i]);

     // Reshow the content
     content.revalidate();
     content.repaint();
   }
   setVisible(true);
   waitingDone();
}
} // class PlayerHdcp.Worker

//----------------------------------------------------------------------------
//
// Class-
//       PlayerHdcp.HdcpPanel
//
// Purpose-
//       Create a Handicap line panel
//
//----------------------------------------------------------------------------
protected class HdcpPanel extends DataPanel
{
public
   HdcpPanel(
     String            used,        // Used string
     String            type,        // Type
     String            date,        // Date played
     String            score,       // ESC score
     String            rating,      // Course rating
     String            slope,       // Course slope
     String            diff,        // Course differential
     String            course)      // Course name
{
   super(7);

   field[0].setColumns(4);          // USED
   field[0].setText(used);

   field[1].setColumns(4);          // TYPE
   field[1].setText(type);

   field[2].setColumns(16);         // DATE
   field[2].setText(showDate(date));

   field[3].setColumns(4);          // SCORE
   field[3].setText(score);

   field[4].setColumns(8);          // RATING/SLOPE
   field[4].setText(rating + "/" + slope);

   field[5].setColumns(4);          // DIFFERENTIAL
   field[5].setText(diff);

   field[6].setColumns(16);         // COURSE NAME
   field[6].setText(course);
}
} // class HdcpPanel
} // class PlayerHdcp
