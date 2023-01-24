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
//       PlayerStat.java
//
// Purpose-
//       Show player statistical data: Gross/Net, Beer Fund, Daily ESC score.
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
//       PlayerStat
//
// Purpose-
//       Player statistics.
//
//----------------------------------------------------------------------------
public class PlayerStat extends CommonJFrame {
//----------------------------------------------------------------------------
// PlayerStat.Attributes
//----------------------------------------------------------------------------
DataPanel              outputPanel; // The (single) output Panel
PlayerStat             owner;       // THIS Object

// Database items
String                 courseID;    // Course ID
String                 eventsID;    // Events ID
String                 eventsNN;    // Events nickname
String                 teeboxID;    // Teebox name

Vector<Course>         courseVector;// Daily course information
GenericItemInfo        eventsShow;  // The events information
TreeMap<String,Player> playerMap;   // The player Map
Vector<Team>           teamVector;  // The team Vector

// Information used in methods
CourseHdcpInfo         courseHdcp;  // Course handicap information
CourseHoleInfo         courseHole;  // Course hole number information
CourseParsInfo         coursePars;  // Course par information
CourseTboxInfo         courseTbox;  // Teebox yardage information

//----------------------------------------------------------------------------
// PlayerStat.Constants for parameterization
//----------------------------------------------------------------------------
static final Color     COLOR_WINNER= Color.ORANGE; // GOLD
static final Color     COLOR_LOSER= new Color(255,192,192); // PINK
static final Color     LIGHT_BLUE= new Color(223, 223, 255);

static final int       CLOSE_MAX= DataField.CLOSE_MAX; // Maximum CP value

static final int       HOLE_COUNT= FORMAT_SKIN;

//----------------------------------------------------------------------------
//
// Method-
//       PlayerStat.PlayerStat
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
public
   PlayerStat( )                    // Constructor
{
   super("PlayerStat");             // Set applet name
   owner= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerStat.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
protected static String             // The associated String
   arrayToString(                   // Convert array to display string
     int[]             array)       // The array
{
   StringBuffer buff= new StringBuffer();
   for(int i= 0; i<array.length; i++)
     buff.append((i == 0 ? "" : " ") + array[i]);

   return buff.toString();
}

protected static String             // The associated String
   arrayToString(                   // Convert array to display string
     String[]          array)       // The array
{
   StringBuffer buff= new StringBuffer();
   for(int i= 0; i<array.length; i++)
     buff.append((i == 0 ? "" : " ") + array[i]);

   return buff.toString();
}

protected DataField                 // Resultant DataField
   genField(                        // Generate  DataField
     int               cols,        // With this column count
     String            text)        // And this text
{
   DataField field= new DataField(cols);
   field.setText(text);
   return field;
}

protected DataField                 // Resultant DataField
   genField(                        // Update  DataField
     DataField         field,       // This DataField
     boolean           isWinner,    // TRUE iff winning score
     boolean           isLoser)     // TRUE iff losing score

{
   if( isWinner != isLoser )
   {
     if( isWinner )
       field.setBackground(COLOR_WINNER);
     else
       field.setBackground(COLOR_LOSER);
   }

   return field;
}

protected DataField                 // Resultant DataField
   genField(                        // Generate  DataField
     int               cols,        // With this column count
     String            text,        // And this text
     boolean           isWinner,    // TRUE iff winning score
     boolean           isLoser)     // TRUE iff losing score
{
   return genField(genField(cols, text), isWinner, isLoser);
}

protected DataPanel                 // Resultant DataPanel
   genPanel(                        // Generate  DataPanel
     int               cols,        // With this column count
     String            text)        // Simple text
{
   DataPanel panel= new DataPanel();
   panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
   DataField field= genField(cols, text);
   panel.add(field);

   return panel;
}

protected DataPanel                 // Resultant DataPanel
   genPanel(                        // Generate  DataPanel
     int               cols,        // With this column count
     String            gross,       // Upper field
     boolean           gWinner,     // TRUE iff gross winning score
     boolean           gLoser,      // TRUE iff gross losing score
     String            net)         // Lower field
{
   DataPanel panel= new DataPanel();
   panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
   DataField field= genField(cols, gross);
   panel.add(genField(field, gWinner, gLoser));

   // MONEY field
   field= genField(cols, net);
   int value= Integer.parseInt(net);
   if( value <= 0 )
     field.setBackground(Color.GREEN);
   else if( value > 0 )
     field.setBackground(Color.RED);
   else
     field.setBackground(LIGHT_BLUE);
   panel.add(field);

   return panel;
}

protected DataPanel                 // Resultant DataPanel
   genPanel(                        // Generate  DataPanel
     int               cols,        // With this column count
     String            gross,       // Gross score
     boolean           gWinner,     // TRUE iff gross winning score
     boolean           gLoser,      // TRUE iff gross losing score
     String            net,         // Net score
     boolean           nWinner,     // TRUE iff net winning score
     boolean           nLoser)      // TRUE iff net losing score
{
   DataPanel panel= new DataPanel();
   panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
   DataField field= genField(cols, gross);
   panel.add(genField(field, gWinner, gLoser));

   field= genField(cols, net);
   field.setBackground(LIGHT_BLUE);
   field.setForeground(Color.BLACK);
   panel.add(genField(field, nWinner, nLoser));
   return panel;
}

protected DataPanel                 // Resultant DataPanel
   genPanel(                        // Generate  DataPanel
     int               cols,        // With this column count
     String            gross,       // Gross score
     String            net)         // Next score
{
   return genPanel(cols, gross, false, false, net, false, false);
}

public int                          // Resultant (0 iff error)
   holeValue(                       // Get hole value
     HolePanel         panel,       // From this panel
     int               index)       // At this index
{
   int result= 0;
   try {
     result= panel.getFieldValue(index);
   } catch(Exception e) {
   }

   return result;
}

protected int
   points(int W, int T)
{
   if( W > T )
     return +1;

   if( W < T )
     return -1;

   return 0;
}

protected int
   skinsOut(int[] S)
{
   int result= 0;
   for(int hole= 1; hole<=9; hole++)
     result += S[hole-1];

   return result;
}

protected int
   skinsIn(int[] I)
{
   int[] S= new int[18];
   for(int hole= 1; hole<=18; hole++)
     S[hole-1]= I[hole-1];

   for(int hole= 1; hole<=18; hole++)
   {
     int m= hole-1;
     int n= I[m];
     while( n > 0 )
     {
       S[m--]= 1;
       n--;
     }
   }

   int result= 0;
   for(int hole= 10; hole<=18; hole++)
     result += S[hole-1];

   return result;
}

protected int
   skinsTot(int[] S)
{
   int result= 0;
   for(int hole= 1; hole<=18; hole++)
     result += S[hole-1];

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerStat.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: PlayerStat v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Player statistics.";
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerStat.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info=
   {   {"events-nick", "string", "The event identifier, "
                               + "default is database default"}
   };
   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerStat.init
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
//       PlayerStat.loadAppletParameters
//
// Purpose-
//       Called in background to load the Applet parameters.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters()
{
   eventsNN= getParameter("events-nick");
   debug("Param: events-nick: " + eventsNN);
}

//----------------------------------------------------------------------------
//
// Class-
//       PlayerStat.Course
//
// Purpose-
//       Course information container.
//
//----------------------------------------------------------------------------
class Course {
String                 date;        // Date
String                 courseID;    // Course ID
String                 teeboxID;    // Teebox ID
String                 rating;      // Rating (double)
String                 slope;       // Slope (integer)


public void
   debug( )                         // Debugging display
{
   owner.debug("Course()");
   owner.debug("Course.date: "       + date);
   owner.debug("Course.courseID: "   + courseID);
   owner.debug("Course.teeboxID: "   + teeboxID);
   owner.debug("Course.rating: "     + rating);
   owner.debug("Course.slope: "      + slope);
}
} // class Course

//----------------------------------------------------------------------------
//
// Class-
//       PlayerStat.Player
//
// Purpose-
//       Player information container.
//
//----------------------------------------------------------------------------
class Player extends PlayerData {
int                    days;        // Number of days played
String[]               score;       // ESC score on date (or "N/A")   //@ADDED

int                    eTotal;      // ESC total strokes (Relative to par)
int                    nTotal;      // Net total strokes (Relative to par)
int                    gTotal;      // Gross total strokes (Relative to par)

int                    playSkins;   // Intra-team skins
double                 avgPSkins;   // Intra-team skins (average)
int                    teamSkins;   // Inter-team skins
double                 avgTSkins;   // Inter-team skins (average)
int                    skinPoints;  // Inter-team points

public void
   debug( )                         // Debugging display
{
   owner.debug("Player(" + playerNN + ")");
   owner.debug("Player.show: "       + playerShow);
   owner.debug("Player.days: "       + days);
   owner.debug("Player.score: "      + arrayToString(score));
   owner.debug("Player.eTotal: "     + eTotal);
   owner.debug("Player.nTotal: "     + nTotal);
   owner.debug("Player.gTotal: "     + gTotal);
   owner.debug("Player.playSkins: "  + playSkins);
   owner.debug("Player.avgPSkins: "  + avgPSkins);
   owner.debug("Player.teamSkins: "  + teamSkins);
   owner.debug("Player.avgTSkins: "  + avgTSkins);
   owner.debug("Player.skinPoints: " + skinPoints);
}

   Player(                          // Constructor
     String            playerID,    // The PLAYER_ID
     String            playerNN)    // The PLAYER_NN
{
   this.playerID= playerID;
   this.playerNN= playerNN;

   score= new String[courseVector.size()];
   for(int i= 0; i<score.length; i++)
     score[i]= "N/A";
}
} // class Player

//----------------------------------------------------------------------------
//
// Class-
//       PlayerStat.Team
//
// Purpose-
//       Team information container.
//
//----------------------------------------------------------------------------
protected class Team extends EventsTeamInfo {
int                    skinPoints;  // Team best2 skins points

public
   Team(String date, String time, int players)
{
   this.date= date;
   this.time= time;
   player_data= new PlayerData[players];
   player_card= new PlayerCardInfo[players];
   player_nets= new PlayerNetsInfo[players];
// player_post= new PlayerPostInfo[players]; // UNUSED
}
} // class Team

//----------------------------------------------------------------------------
//
// Class-
//       PlayerStat.Loader
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
//       PlayerStat.Loader.loadTeamVector
//
// Purpose-
//       Load the teamVector for the specified date.
//
//----------------------------------------------------------------------------
public Vector<Team>                 // The team Vector
   loadTeamVector(                  // Load team Vector
     String            date)        // For this EventId.date
{
   Vector<Team>        result= new Vector<Team>();

   debug("loadTeamVector(" + date + ")");

   String timeCMD= CMD_EVENTS_TIME;
   String timeNXT= date;
   for(;;)
   {
     String next= dbNext(timeCMD, timeNXT);
     if( next == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(next);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(timeCMD) )
       break;

     int index2= catcon(item, 2);
     if( index2 < 0 )
       break;
     if( !date.equals(item.substring(0,index2)) )
       break;
     int index1= catcon(item, 1);

     timeNXT= item;

     // Load the team data
     String[] array= tokenize(t.remainder());
     Team team= new Team(item.substring(0, index1),
                         item.substring(index1+1, index2), array.length);
     for(int i=0; i<array.length; i++)
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

       String[] player_card= dbRetrieve(CMD_EVENTS_CARD, concat(item, array[i]));
       if( player_card == null )
         player_card= dbRetrieve(CMD_EVENTS_CARD, concat(item, playerID));
       if( player_card != null )
       {
         team.player_card[i]=
             new PlayerCardInfo(HOLE_COUNT, playerShow, player_card,
                                playerHdcp, coursePars, courseTbox);
         team.player_nets[i]=
             new PlayerNetsInfo(HOLE_COUNT, playerShow, player_card,
                                playerHdcp, coursePars, courseTbox, courseHdcp);

         team.player_card[i].genPanel();
         team.player_nets[i].genPanel();
       }
     }

     team.skinPanel= new EventsSkinPanel(HOLE_COUNT, "Skins", team);
     team.bestPanel= new EventsBestPanel(HOLE_COUNT, "Best2", coursePars, team);

     result.add(team);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerStat.Loader.doInBackground
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
   String[]            sarray;      // Working temporary String[]
   String              string;      // Working temporary String

   debug("Loader.doInBackground()");
   dbReady();

   // Load the data
   try {
     if( client.fsm != DbStatic.FSM_READY )
       throw new Exception("DbServer offline");

     //-----------------------------------------------------------------------
     // Extract associated data
     String[] array;
     if( eventsNN == null )
       eventsNN= dbGet(CMD_DEFAULT, DEFAULT_CODE_EI);

     eventsID= dbGet(CMD_EVENTS_FIND, eventsNN);
     if( eventsID == null )
       throw new Exception("Cannot find EVENTS_ID");

     eventsShow= new GenericItemInfo(dbGet(CMD_EVENTS_SHOW, eventsID));

     //-----------------------------------------------------------------------
     // Base panels
     eventsShow.genPanel().getField(0).setBackground(Color.GREEN.brighter());
     outputPanel= new DataPanel();
     outputPanel.add(eventsShow.getPanel());

     // Get the list of courses
     courseVector= new Vector<Course>();
     String CMD= CMD_EVENTS_DATE;
     String NEXT= eventsID;
     for(;;)
     {
       string= dbNext(CMD, NEXT);
       if( string == null )
         break;

       QuotedTokenizer t= new QuotedTokenizer(string);
       String type= t.nextToken();
       String item= t.nextToken();

       if( !type.equals(CMD) )
         break;

       NEXT= item;

       int index1= catcon(item, 1);
       if( index1 < 0 )
         continue;
       if( !eventsID.equals(item.substring(0, index1)) )
         break;

       courseID= t.nextToken();
       teeboxID= t.nextToken();
       courseTbox= new CourseTboxInfo(HOLE_COUNT, courseID, null, teeboxID,
                                      dbRetrieve(CMD_COURSE_TBOX, concat(courseID, teeboxID)));

       Course course= new Course();
       course.date= item.substring(index1+1);
       course.courseID= courseID;
       course.teeboxID= teeboxID;
       course.slope= courseTbox.stringMS;
       course.rating= courseTbox.stringMR;

       courseVector.add(course);
     }

     // Get the list of players
     playerMap= new TreeMap<String,Player>();

     CMD= CMD_EVENTS_HDCP;
     NEXT= eventsID;
     for(;;)
     {
       string= dbNext(CMD, NEXT);
       if( string == null )
         break;

       QuotedTokenizer t= new QuotedTokenizer(string);
       String type= t.nextToken();
       String item= t.nextToken();

       if( !type.equals(CMD) )
         break;

       NEXT= item;

       int index1= catcon(item, 1);
       if( index1 < 0 )
         continue;
       if( !eventsID.equals(item.substring(0, index1)) )
         break;

       String playerNN= item.substring(index1+1);
       String playerID= dbGet(CMD_PLAYER_FIND, playerNN);
       if( playerID == null )
         throw new Exception("Cannot find PLAYER_ID for " + playerNN);
       playerNN= dbGet(CMD_PLAYER_NICK, playerID);
       if( playerNN == null )
         throw new Exception("Cannot find PLAYER_NN for " + playerID);

       Player player= new Player(playerID, playerNN);
       player.playerShow= stripQuotes(dbGet(CMD_PLAYER_SHOW, playerID));
       playerMap.put(player.playerNN, player);
     }

     // Calculate statistics
     CMD= CMD_EVENTS_DATE;
     NEXT= eventsID;
     for(int dateIndex= (-1);;)
     {
       string= dbNext(CMD, NEXT);
       if( string == null )
         break;

       QuotedTokenizer t= new QuotedTokenizer(string);
       String type= t.nextToken();
       String item= t.nextToken();

       if( !type.equals(CMD) )
         break;

       NEXT= item;

       int index1= catcon(item, 1);
       if( index1 < 0 )
         continue;
       if( !eventsID.equals(item.substring(0, index1)) )
         break;

       // Load the course data for this date
       dateIndex++;
       sarray= tokenize(t.remainder());
       if( sarray.length < 3 )
         throw new Exception("Invalid EVENTS.DATE: " + t.remainder());

       courseID= sarray[0];
       teeboxID= sarray[1];

       courseHdcp= new CourseHdcpInfo(HOLE_COUNT, dbRetrieve(CMD_COURSE_HDCP, courseID, teeboxID));
       courseHole= new CourseHoleInfo(HOLE_COUNT, dbRetrieve(CMD_COURSE_HOLE, courseID, teeboxID));
       coursePars= new CourseParsInfo(HOLE_COUNT, dbRetrieve(CMD_COURSE_PARS , courseID, teeboxID));
       courseTbox= new CourseTboxInfo(HOLE_COUNT, courseID, null, teeboxID,
                                      dbRetrieve(CMD_COURSE_TBOX, concat(courseID, teeboxID)));

       courseHdcp.genPanel();
       courseHole.genPanel();
       coursePars.genPanel();
       courseTbox.genPanel();

       // Load the team data for this date
       teamVector= loadTeamVector(item);

       // Calculate the number of players on each team for this date
       int maxPlayers= Integer.MIN_VALUE;
       int minPlayers= Integer.MAX_VALUE;
       for(int i=0; i<teamVector.size(); i++)
       {
         Team team= teamVector.elementAt(i);

         int playerCount= 0;
         for(int j= 0; j<team.player_data.length; j++)
         {
           PlayerData playerData= team.player_data[j];
           if( playerData != null )
             playerCount++;
         }

         if( playerCount > maxPlayers )
           maxPlayers= playerCount;

         if( playerCount < minPlayers )
           minPlayers= playerCount;
       }

       // Calculate skin points if teams are roughly equal in size.
       // (There are no more than 4 players on a team.)
       if( minPlayers >= 2 )        // Must have at least two players
       {
         if( minPlayers == maxPlayers || minPlayers > 2 )
         {
           HolePanel[] panel_nets= new HolePanel[teamVector.size()];
           for(int i= 0; i<panel_nets.length; i++)
             panel_nets[i]= teamVector.elementAt(i).bestPanel;
           EventsSkinPanel skinPanel= new EventsSkinPanel(HOLE_COUNT, "Skins", panel_nets);

           if( true )
           {
             // Computation 3 - can only lose 3 points, no wins
             int maxOut= 0;
             int maxIn=  0;
             int maxTot= 0;
             for(int i= 0; i<panel_nets.length; i++)
             {
               int[] iSkins= skinPanel.countSkins(i, panel_nets);
               int teamOut= skinsOut(iSkins);
               int teamIn=  skinsIn(iSkins);
               int teamTot= skinsTot(iSkins);
               if( teamOut > maxOut )
                 maxOut= teamOut;
               if( teamIn > maxIn )
                 maxIn= teamIn;
               if( teamTot > maxTot )
                 maxTot= teamTot;
             }

             for(int i= 0; i<panel_nets.length; i++) // Points are BAD
             {
               int[] iSkins= skinPanel.countSkins(i, panel_nets);
               if( skinsOut(iSkins) < maxOut )
                 teamVector.elementAt(i).skinPoints++;
               if( skinsIn(iSkins) < maxIn )
                 teamVector.elementAt(i).skinPoints++;
               if( skinsTot(iSkins) < maxTot )
                 teamVector.elementAt(i).skinPoints++;
             }
           }
           else if( false )
           {
             // Computation 2 - can only lose 3 points
             int maxOut= 0;
             int maxIn=  0;
             int maxTot= 0;
             for(int i= 0; i<panel_nets.length; i++)
             {
               int[] iSkins= skinPanel.countSkins(i, panel_nets);
               int teamOut= skinsOut(iSkins);
               int teamIn=  skinsIn(iSkins);
               int teamTot= skinsTot(iSkins);
               if( teamOut > maxOut )
                 maxOut= teamOut;
               if( teamIn > maxIn )
                 maxIn= teamIn;
               if( teamTot > maxTot )
                 maxTot= teamTot;
             }

             for(int i= 0; i<panel_nets.length; i++)
             {
               int[] iSkins= skinPanel.countSkins(i, panel_nets);
               if( skinsOut(iSkins) < maxOut )
                 teamVector.elementAt(i).skinPoints--;
               else
               {
                 for(int j= 0; j<panel_nets.length; j++)
                 {
                   int[] jSkins= skinPanel.countSkins(j, panel_nets);
                   if( i != j )
                     teamVector.elementAt(i).skinPoints += points(maxOut, skinsOut(jSkins));
                 }
               }
               if( skinsIn(iSkins) < maxIn )
                 teamVector.elementAt(i).skinPoints--;
               else
               {
                 for(int j= 0; j<panel_nets.length; j++)
                 {
                   int[] jSkins= skinPanel.countSkins(j, panel_nets);
                   if( i != j )
                     teamVector.elementAt(i).skinPoints += points(maxIn, skinsIn(jSkins));
                 }
               }
               if( skinsTot(iSkins) < maxTot )
                 teamVector.elementAt(i).skinPoints--;
               else
               {
                 for(int j= 0; j<panel_nets.length; j++)
                 {
                   int[] jSkins= skinPanel.countSkins(j, panel_nets);
                   if( i != j )
                     teamVector.elementAt(i).skinPoints += points(maxTot, skinsTot(jSkins));
                 }
               }
             }
           }
           else if( false )
           {
             // Computation 1 - team by team
             for(int i= 0; i<panel_nets.length; i++)
             {
               int[] iSkins= skinPanel.countSkins(i, panel_nets);
               for(int j= 0; j<panel_nets.length; j++)
               {
                 if( i != j )
                 {
                   int[] jSkins= skinPanel.countSkins(j, panel_nets);
                   teamVector.elementAt(i).skinPoints += points(skinsOut(iSkins), skinsOut(jSkins));
                   teamVector.elementAt(i).skinPoints += points(skinsIn(iSkins),  skinsIn(jSkins));
                   teamVector.elementAt(i).skinPoints += points(skinsTot(iSkins), skinsTot(jSkins));
                 }
               }
             }
           }
         }
       }

       // Count each player who played on this date
       for(int i=0; i<teamVector.size(); i++)
       {
         Team team= teamVector.elementAt(i);

         int playerCount= 0;
         for(int j= 0; j<team.player_data.length; j++)
         {
           PlayerData playerData= team.player_data[j];
           if( playerData == null )
             continue;

           String playerNN= playerData.playerNN;
           Player player= playerMap.get(playerNN);
           if( player == null )
           {
             error("Player '" + playerNN + "' not in map");
             continue;
           }

           PlayerCardInfo playerCard= team.player_card[j];
           if( playerCard == null )
             continue;

           player.days++;
           String playerHdcp= team.player_data[j].playerHdcp;
           int courseHdcp= courseTbox.courseHdcp(playerHdcp);
           int tot= 0;
           for(int hole= 1; hole<=18; hole++)
           {
             int par= Integer.parseInt(coursePars.array[hole-1]);
             tot += par;
           }

           HolePanel panel= (HolePanel)team.player_card[j].getPanel();
           player.score[dateIndex]= panel.getFieldText(HOLE_ESC);
           int e= holeValue(panel, HOLE_ESC) - tot;
           int g= holeValue(panel, HOLE_TOT) - tot;
           int n= holeValue(panel, HOLE_NET) - tot;
           player.eTotal += e;
           player.gTotal += g;
           player.nTotal += n;
           player.playSkins += holeValue((HolePanel)team.player_nets[j].getPanel(), HOLE_SKIN);
           player.teamSkins += holeValue(team.bestPanel, HOLE_SKIN);
           player.skinPoints += team.skinPoints;
         }
       } // for(int i=0; i<teamVector.size(); i++)
     }

     // Generate the data panels
     DataPanel panel= new DataPanel();
     DataField field= genField(8, "Player");
     field.setHorizontalAlignment(DataField.LEFT);
     panel.add(field);
     panel.add(genField(4, "Days"));

     panel.add(genField(0, ""));
     panel.add(genPanel(6, "+GROSS", "+NET"));

     panel.add(genField(0, ""));
     panel.add(genPanel(6, "Beer Fund")); //@ADDED

     panel.add(genField(0, "")); //@ADDED
     Course[] courseArray= new Course[courseVector.size()];
     courseArray= courseVector.toArray(courseArray);
     for(int i= 0; i<courseArray.length; i++)
     {
       Course course= courseArray[i];
       panel.add(genPanel(8, showDate(course.date),
                             course.rating+"/"+course.slope));
     }

     outputPanel.add(panel);

     for(Iterator iter= playerMap.entrySet().iterator(); iter.hasNext();)
     {
       Map.Entry me= (Map.Entry)iter.next();
       Player player= (Player)me.getValue();

       int days= player.days;
       panel= new DataPanel();
       field= genField(8, player.playerShow);
       field.setHorizontalAlignment(DataField.LEFT);
       panel.add(field);
       panel.add(genField(4, "" + days));

       panel.add(genField(0, ""));
       panel.add(genPanel(6, "" + player.gTotal, "" + player.nTotal));

       panel.add(genField(0, ""));
       panel.add(genPanel(6, "" + player.skinPoints));

       panel.add(genField(0, ""));
       for(int i= 0; i<player.score.length; i++)
       {
         panel.add(genPanel(8, player.score[i]));
       }

       outputPanel.add(panel);
     }
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
//       PlayerStat.Loader.done
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
     loaderPanel= null;

     // Generate the content
     content.setLayout(new GridLayout(0,1));
     outputPanel.setLayout(new GridLayout(0,1));
     content.add(outputPanel);
   }

   // Display the content
   content.revalidate();
   setVisible(true);
   waitingDone();
}
} // class PlayerStat.Loader
} // class PlayerStat
