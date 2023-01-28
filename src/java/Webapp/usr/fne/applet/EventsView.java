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
//       EventsView.java
//
// Purpose-
//       View results for event on date.
//
// Last change date-
//       2023/01/28
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.lang.*;
import java.util.*;
import javax.swing.*;

// import usr.fne.common.SwingWorker;
import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       EventsView
//
// Purpose-
//       View results for event on date.
//
//----------------------------------------------------------------------------
public class EventsView extends CommonJFrame {
//----------------------------------------------------------------------------
// EventsView.Attributes
//----------------------------------------------------------------------------
GenericItemInfo        courseItem;  // The course information container
CourseHdcpInfo         courseHdcp;  // The course handicap information
CourseHoleInfo         courseHole;  // The course hole number information
CourseLdCpInfo         courseLdcp;  // The course LD/CP info
CourseParsInfo         coursePars;  // The course par information
CourseTboxInfo         courseTbox;  // The course teebox information

String                 courseID;    // Course ID
String                 courseShow;  // The course display name
String                 eventsID;    // Events ID
String                 eventsNN;    // Events nickname
String                 eventsDate;  // Events date
String                 teeboxID;    // Teebox name
Vector<EventsTeamInfo> teamVector;  // The EventsTeamInfo Vector

// EventsView items
EventsSkinPanel        skinPanel;   // Events skin panel (for team coloring)
Vector<HolePanel>      skinVector;  // The team skins panels BEER FUND

// Hole constants
static final int       HOLE_COUNT= FORMAT_SKIN;

//----------------------------------------------------------------------------
//
// Method-
//       EventsView.EventsView
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   EventsView( )                    // Constructor
{
   super("EventsView");             // Set applet name
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsView.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: EventsView v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Event round data.";
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsView.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info=
   {   {"event-ID",    "string", "The event identifier, "
                               + "default is database default"}
   ,   {"event-date",  "string", "The round identifier, "
                               + "default is database default"}
   };
   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsView.init
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
//       EventsView.loadAppletParameters
//
// Purpose-
//       Called in background to load the Applet parameters.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters()
{
   eventsNN= getParameter("events-nick");
   eventsDate= getParameter("events-date");
   debug("Param: events-nick: " + eventsNN);
   debug("Param: events-date: " + eventsDate);
}

//----------------------------------------------------------------------------
//
// Class-
//       EventsView.loader
//
// Purpose-
//       Load the team data.
//
//----------------------------------------------------------------------------
public void
   loader(                          // Load team data
     int               format)      // The Panel format
   throws Exception
{
   debug("EventsView.loader(" + format + ")");

   // Load the data
   String[] array;
   if( eventsNN == null )
     eventsNN= dbGet(CMD_DEFAULT, DEFAULT_CODE_EI);

   eventsID= dbGet(CMD_EVENTS_FIND, eventsNN);
   if( eventsID == null )
     throw new Exception("Cannot find EVENTS_ID");

   if( eventsDate == null )
   {
     array= dbRetrieve(CMD_DEFAULT, DEFAULT_CODE_DT);
     if( array == null )
       throw new Exception("Cannot find EVENTS_DATE");
     eventsDate= array[0];
   }

   array= dbRetrieve(CMD_EVENTS_DATE, concat(eventsID,eventsDate));
   if( array == null )
     throw new Exception("No such event: "+eventsNN+" on "+eventsDate);
   courseID= array[0];
   teeboxID= array[1];

   String S;
   courseHdcp= new CourseHdcpInfo(format, dbGet(CMD_COURSE_HDCP, courseID));
   courseHole= new CourseHoleInfo(format, dbGet(CMD_COURSE_HOLE, courseID));
   S= dbGet(CMD_COURSE_LONG, courseID);
   CourseLongInfo longInfo= new CourseLongInfo(format, S);
   S= dbGet(CMD_COURSE_NEAR, courseID);
   CourseNearInfo nearInfo= new CourseNearInfo(format, S);
   courseLdcp= new CourseLdCpInfo(format, longInfo, nearInfo);
   coursePars= new CourseParsInfo(format, dbGet(CMD_COURSE_PARS, courseID));
   courseShow= dbGet(CMD_COURSE_SHOW, courseID);
   S= dbGet(CMD_COURSE_TBOX, concat(courseID, teeboxID));
   courseTbox= new CourseTboxInfo(format, courseID, courseShow, teeboxID, S);

   S= showDate(eventsDate) + " -- " + courseShow + " -- "
    + courseTbox.stringMR + "/" + courseTbox.stringMS;
   courseItem= new GenericItemInfo(S);

   // Extract the TEAM data
   teamVector= new Vector<EventsTeamInfo>();
   String CMD=  CMD_EVENTS_TIME;
   String BASE= concat(eventsID, eventsDate);
   String TIME= BASE;
   for(;;)
   {
     String text= dbNext(CMD, TIME);
     if( text == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(text);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(CMD) )
       break;

     int index= catcon(item, 2);
     if( index < 0 )
       continue;
     if( !BASE.equals(item.substring(0, index)) )
       break;
     TIME= item;

     EventsTeamInfo team= new EventsTeamInfo();
     team.time= item.substring(index+1);

     array= tokenize(t.remainder());
     int size= array.length;
     if( size == 0 )
     {
       debug("No players for: " + text);
       continue;
     }

     team.player_data= new PlayerData[size];
     team.player_card= new PlayerCardInfo[size];
     team.player_nets= new PlayerNetsInfo[size];

     // Add the team data
     for(int i= 0; i<size; i++)
     {
       String playerNN= PlayerData.getNickName(array[i]);
       String playerID= dbGet(CMD_PLAYER_FIND, playerNN);
       String playerShow= stripQuotes(dbGet(CMD_PLAYER_SHOW, playerID));
       String playerHdcp= dbGet(CMD_EVENTS_HDCP, concat(eventsID, playerNN));
       boolean isGuest= PlayerData.isGuestName(array[i]);
       if( isGuest )
         playerShow += " (G)";

       if( playerHdcp == null )
         playerHdcp= dbGet(CMD_EVENTS_HDCP, concat(eventsID, playerID));
       if( playerHdcp == null )
         playerHdcp= "0";
       team.player_data[i]= new PlayerData(playerID, playerNN, playerShow, playerHdcp, isGuest);

       String[] player_card= dbRetrieve(CMD_EVENTS_CARD, concat(TIME, array[i]));
       if( player_card == null )
         player_card= dbRetrieve(CMD_EVENTS_CARD, concat(TIME, playerID));
       if( player_card != null )
       {
         team.player_card[i]=
             new PlayerCardInfo(format, playerShow, player_card,
                                playerHdcp, coursePars, courseTbox);
         team.player_nets[i]=
             new PlayerNetsInfo(format, playerShow, player_card,
                                playerHdcp, coursePars, courseTbox, courseHdcp);
       }
     }

     teamVector.add(team);
     debug("Added team: " + HoleInfo.arrayToString(array));
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       EventsView.Loader
//
// Purpose-
//       Create a background task to load the images.
//
//----------------------------------------------------------------------------
class Loader extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       EventsView.Loader.doInBackground
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
   dbReady();

   // Load the data
   try {
     if( client.fsm != DbStatic.FSM_READY )
       throw new Exception("DbServer offline");

     //-----------------------------------------------------------------------
     // Extract associated data
     loader(HOLE_COUNT);

     //-----------------------------------------------------------------------
     // Base panels
     courseItem.genPanel();
     courseHdcp.genPanel();
     courseHole.genPanel();
     coursePars.genPanel();
     courseTbox.genPanel();
     DataField[] df= courseHole.getPanel().getField();
     df[HOLE_ESC].setText("ESC");
     df[HOLE_HCP].setText("Hdcp");
     df[HOLE_NET].setText("Net");
     df[HOLE_LDO].setText("LD out");
     df[HOLE_CPO].setText("CP out");
     df[HOLE_LDI].setText("LD in");
     df[HOLE_CPI].setText("CP in");
     df[HOLE_FWH].setText("FWH");
     df[HOLE_GIR].setText("GIR");
     df[HOLE_SKIN].setText("Skins");

     // Generate the TEAM panels
     skinVector= new Vector<HolePanel>();
     for(int i= 0; i<teamVector.size(); i++)
     {
       EventsTeamInfo team= teamVector.elementAt(i);
       for(int j= 0; j<team.player_data.length; j++)
       {
         if( team.player_card[j] != null )
           team.player_card[j].genPanel();

         if( team.player_nets[j] != null )
           team.player_nets[j].genPanel();
       }

       team.skinPanel= new EventsSkinPanel(HOLE_COUNT, "Skins", team);
       team.bestPanel= new EventsBestPanel(HOLE_COUNT, "Best2", coursePars, team);

       skinVector.add(team.bestPanel);
     }

     // Update the TEAM skin colors
     skinPanel= new EventsSkinPanel(HOLE_COUNT, "Skins", skinVector);
   } catch( Exception e ) {
     setERROR("Loader: doInBackground: Exception: " + e);
     e.printStackTrace();
   }

   dbReset();

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsView.Loader.done
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

   // If error, put message in the loaderPanel
   if( errorString != null )
   {
     loaderPanel.getField(0).setText(errorString);
   }
   else
   {
     content.removeAll();

     content.setLayout(new GridLayout(0,1));
     content.add(courseItem.getPanel());
     content.add(courseHole.getPanel());
     content.add(courseTbox.getPanel());
     content.add(coursePars.getPanel());

     // The score panels
     for(int i= 0; i<teamVector.size(); i++)
     {
       if( i != 0 )
         content.add(new DataPanel(0));

       EventsTeamInfo team= teamVector.elementAt(i);
       for(int j= 0; j<team.player_data.length; j++)
       {
         if( team.player_card[j] == null )
           content.add(new ItemPanel(team.player_data[j].playerShow + " NO SCORE POSTED"));
         else
           content.add(team.player_card[j].getPanel());
       }

       content.add(courseHdcp.genPanel());
       for(int j= 0; j<team.player_data.length; j++)
       {
         if( team.player_nets[j] == null )
           content.add(new ItemPanel(team.player_data[j].playerShow + " NO SCORE POSTED"));
         else
           content.add(team.player_nets[j].getPanel());
       }

//     content.add(team.skinPanel);
       content.add(team.bestPanel);
     }
//   content.add(skinPanel);
   }

   // Reshow the content
   content.revalidate();
   setVisible(true);
   waitingDone();
}
} // class EventsView.Loader
} // class EventsView
