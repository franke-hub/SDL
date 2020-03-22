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
//       EventsStat.java
//
// Purpose-
//       Show event statistical data.
//
// Last change date-
//       2020/01/15
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
//       EventsStat
//
// Purpose-
//       Event statistics.
//
//----------------------------------------------------------------------------
public class EventsStat extends CommonJApplet {
//----------------------------------------------------------------------------
// EventsStat.Attributes
//----------------------------------------------------------------------------
DataPanel              outputPanel; // The (single) output Panel
EventsStat             owner;       // THIS Object

// Database items
String                 courseID;    // Course ID
String                 eventsID;    // Events ID
String                 eventsNN;    // Events nickname
String                 teeboxID;    // Teebox name

GenericItemInfo        eventsShow;  // The events information
TreeMap<String,Player> playerMap;   // The player Map
Vector<Team>           teamVector;  // The team Vector

// Information used in methods
CourseHdcpInfo         courseHdcp;  // Course handicap information
CourseHoleInfo         courseHole;  // Course hole number information
CourseParsInfo         coursePars;  // Course par information
CourseTboxInfo         courseTbox;  // Teebox yardage information

//----------------------------------------------------------------------------
// EventsStat.Constants for parameterization
//----------------------------------------------------------------------------
static final Color     COLOR_WINNER= Color.ORANGE; // GOLD
static final Color     COLOR_LOSER= new Color(255,192,192); // PINK
static final Color     LIGHT_BLUE= new Color(223, 223, 255);

static final int       CLOSE_MAX= DataField.CLOSE_MAX; // Maximum CP value

static final int       HOLE_COUNT= FORMAT_SKIN;

//----------------------------------------------------------------------------
//
// Method-
//       EventsStat.EventsStat
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
public
   EventsStat( )                    // Constructor
{
   super("EventsStat");             // Set applet name
   owner= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsStat.UTILITIES
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

protected static double             // The single digit precision average
   average(                         // Get average
     int               num,         // The numerator
     int               den)         // The denominator
{
   double result= 0.0;
   if( den > 0 )
   {
     int avg= num * 10;
     avg /= den;
     result= ((double)avg / 10.0);
   }

   return result;
}

protected static double             // The single digit precision average
   average(                         // Get average
     double            num,         // The numerator
     int               den)         // The denominator
{
   double result= 0.0;
   if( den > 0 )
   {
     int avg= (int)(num * 10.0 + 0.5);
     avg /= den;
     result= ((double)avg / 10.0);
   }

   return result;
}

protected static double             // The closest to pin rating
   cpRating(                        // Get CP rating
     Player            player)      // For this player
{
   double result= CLOSE_MAX;
   if( player.days > 0 )
   {
     double rating= (2*player.days) - player.cpCount;
     rating *= CLOSE_MAX;           // CLOSE_MAX for each miss
     rating += player.cpTotal;      // Add computed total
     result= (rating / (2*player.days));
   }

   return tenths(result);
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

protected static double             // The tenths precision
   tenths(                          // Get tenths precision
     double            v)           // For this value
{
   int i= (int)(v * 10.0 + 0.5);
   return ((double)i / 10.0);
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsStat.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: EventsStat v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Event statistics.";
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsStat.getParameterInfo
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
//       EventsStat.init
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
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsStat.loadAppletParameters
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
//       EventsStat.Player
//
// Purpose-
//       Player information container.
//
//----------------------------------------------------------------------------
class Player extends PlayerData {
int                    days;        // Number of days played
int                    nHighest;    // Highest net score (Relative to par)
int                    nLowest;     // Lowest net score  (Relative to par)
int                    nTotal;      // Net total strokes (Relative to par)
double                 nAverage;    // Net average strokes (Relative to par)

int                    eTotal;      // ESC total strokes (Relative to par)
int                    gHighest;    // Highest gross score (Relative to par)
int                    gLowest;     // Lowest gross score  (Relative to par)
int                    gTotal;      // Gross total strokes (Relative to par)
double                 gAverage;    // Gross average strokes (Relative to par)

int[]                  gStroke;     // <-2, -2, -1, 0, +1, +2, >2 GROSS
int[]                  nStroke;     // <-2, -2, -1, 0, +1, +2, >2 NET

int                    ldCount;     // Number of long drives
int                    ldTotal;     // Total long drive distance
int                    ldMax;       // Longest drive
double                 ldAverage;   // Longest drive average
double                 ldRating;    // Longest drive rating
int                    cpCount;     // Number close to pin
double                 cpTotal;     // Total close to pin
double                 cpMin;       // Closest to pin
double                 cpAverage;   // Closest to pin average
double                 cpRating;    // Closest to pin rating
int                    fwhCount;    // Number of fairways hit
double                 fwhAverage;  // Average number of fairways hit
int                    girCount;    // Number of greens in regulation
double                 girAverage;  // Average number greens in regulation

int                    playSkins;   // Intra-team skins
double                 avgPSkins;   // Intra-team skins (average)
int                    teamSkins;   // Inter-team skins
double                 avgTSkins;   // Inter-team skins (average)
int                    skinPoints;  // Inter-team points

public void
   debug( )                         // Debugging display
{
   debug("Player(" + playerNN + ")");
   debug("Player.show: "       + playerShow);
   debug("Player.days: "       + days);
   debug("Player.nHighest: "   + nHighest);
   debug("Player.nLowest: "    + nLowest);
   debug("Player.nTotal: "     + nTotal);
   debug("Player.nAverage: "   + nAverage);
   debug("Player.eTotal: "     + eTotal);
   debug("Player.gHighest: "   + gHighest);
   debug("Player.gLowest: "    + gLowest);
   debug("Player.gTotal: "     + gTotal);
   debug("Player.gAverage: "   + gAverage);
   debug("Player.gStroke: "    + arrayToString(gStroke));
   debug("Player.nStroke: "    + arrayToString(nStroke));
   debug("Player.ldCount: "    + ldCount);
   debug("Player.ldTotal: "    + ldTotal);
   debug("Player.ldMax: "      + ldMax);
   debug("Player.ldAverage: "  + ldAverage);
   debug("Player.ldRating: "   + ldRating);
   debug("Player.cpCount: "    + cpCount);
   debug("Player.cpTotal: "    + cpTotal);
   debug("Player.cpMin: "      + cpMin);
   debug("Player.cpAverage: "  + cpAverage);
   debug("Player.cpRating: "   + cpRating);
   debug("Player.fwhCount: "   + fwhCount);
   debug("Player.fwhAverage: " + fwhAverage);
   debug("Player.girCount: "   + girCount);
   debug("Player.girAverage: " + girAverage);
   debug("Player.playSkins: "  + playSkins);
   debug("Player.avgPSkins: "  + avgPSkins);
   debug("Player.teamSkins: "  + teamSkins);
   debug("Player.avgTSkins: "  + avgTSkins);
   debug("Player.skinPoints: " + skinPoints);
}

   Player(                          // Constructor
     String            playerID,    // The PLAYER_ID
     String            playerNN)    // The PLAYER_NN
{
   this.playerID= playerID;
   this.playerNN= playerNN;
   gStroke= new int[7];
   nStroke= new int[7];
}
} // class Player

//----------------------------------------------------------------------------
//
// Class-
//       EventsStat.Team
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
//       EventsStat.Loader
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
//       EventsStat.Loader.loadTeamVector
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
//       EventsStat.Loader.doInBackground
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

     // Get the list of players
     playerMap= new TreeMap<String,Player>();

     String CMD= CMD_EVENTS_HDCP;
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

       String playerNN= item.substring(index1+1);
       if( PlayerData.isGuestName(playerNN) )
         continue;

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

       // Load the course data for this date
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
           if( playerData != null && (! playerData.isGuest) )
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
           if( playerData == null || playerData.isGuest )
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
             int gStroke=
                 PlayerCardInfo.escScore(courseHdcp,
                                         playerCard.array[hole-1],
                                         coursePars.array[hole-1]) - par;
             int nStroke=
                 PlayerNetsInfo.netScore(courseHdcp,
                                         playerCard.array[hole-1],
                                         coursePars.array[hole-1],
                                   owner.courseHdcp.array[hole-1]) - par;

             gStroke= max(gStroke, -3);
             gStroke= min(gStroke, +3);
             nStroke= max(nStroke, -3);
             nStroke= min(nStroke, +3);

             player.gStroke[gStroke+3]++;
             player.nStroke[nStroke+3]++;
           }

           HolePanel panel= (HolePanel)team.player_card[j].getPanel();
           int e= holeValue(panel, HOLE_ESC) - tot;
           int g= holeValue(panel, HOLE_TOT) - tot;
           int n= holeValue(panel, HOLE_NET) - tot;
           player.eTotal += e;
           player.gTotal += g;
           player.nTotal += n;
           player.playSkins += holeValue((HolePanel)team.player_nets[j].getPanel(), HOLE_SKIN);
           player.teamSkins += holeValue(team.bestPanel, HOLE_SKIN);
           player.skinPoints += team.skinPoints;

           if( player.days == 1 )
           {
             player.gLowest= g;
             player.gHighest= g;
             player.nLowest= n;
             player.nHighest= n;
           }
           else
           {
             if( player.gLowest > g )
               player.gLowest= g;
             if( player.gHighest < g )
               player.gHighest= g;
             if( player.nLowest > n )
               player.nLowest= n;
             if( player.nHighest < n )
               player.nHighest= n;
           }

           string= panel.getFieldText(HOLE_LDO);
           if( string != null && !string.equals("-") )
           {
             int ld= Integer.parseInt(string);
             player.ldCount++;
             player.ldTotal += ld;
             if( player.ldCount == 1 || ld > player.ldMax )
               player.ldMax= ld;
           }

           string= panel.getFieldText(HOLE_LDI);
           if( string != null && !string.equals("-") )
           {
             int ld= Integer.parseInt(string);
             player.ldCount++;
             player.ldTotal += ld;
             if( player.ldCount == 1 || ld > player.ldMax )
               player.ldMax= ld;
           }

           string= panel.getFieldText(HOLE_CPO);
           if( string != null && !string.equals("-") )
           {
             double cp= Double.parseDouble(string);
             player.cpCount++;
             player.cpTotal += cp;
             if( player.cpCount == 1 || cp < player.cpMin )
               player.cpMin= cp;
           }

           string= panel.getFieldText(HOLE_CPI);
           if( string != null && !string.equals("-") )
           {
             double cp= Double.parseDouble(string);
             player.cpCount++;
             player.cpTotal += cp;
             if( player.cpCount == 1 || cp < player.cpMin )
               player.cpMin= cp;
           }

           string= panel.getFieldText(HOLE_FWH);
           if( string != null && !string.equals("-") )
           {
             int count= Integer.parseInt(string);
             player.fwhCount += count;
           }

           string= panel.getFieldText(HOLE_GIR);
           if( string != null && !string.equals("-") )
           {
             int count= Integer.parseInt(string);
             player.girCount += count;
           }
         }
       } // for(int i=0; i<teamVector.size(); i++)
     }

     // Calculate minPlayer and maxPlayer, and Player averages
     Player minPlayer= new Player("MinPlayer", "MinPlayer");
     minPlayer.nHighest= Integer.MAX_VALUE;
     minPlayer.nLowest= Integer.MAX_VALUE;
     minPlayer.nTotal= Integer.MAX_VALUE;
     minPlayer.nAverage= Double.MAX_VALUE;
     minPlayer.eTotal= Integer.MAX_VALUE;
     minPlayer.gHighest= Integer.MAX_VALUE;
     minPlayer.gLowest= Integer.MAX_VALUE;
     minPlayer.gTotal= Integer.MAX_VALUE;
     minPlayer.gAverage= Double.MAX_VALUE;
     for(int i= 0; i<minPlayer.gStroke.length; i++)
       minPlayer.gStroke[i]= minPlayer.nStroke[i]= Integer.MAX_VALUE;
     minPlayer.ldCount= Integer.MAX_VALUE;
     minPlayer.ldTotal= Integer.MAX_VALUE;
     minPlayer.ldMax= Integer.MAX_VALUE;
     minPlayer.ldAverage= Double.MAX_VALUE;
     minPlayer.ldRating= Double.MAX_VALUE;
     minPlayer.cpCount= Integer.MAX_VALUE;
     minPlayer.cpTotal= Double.MAX_VALUE;
     minPlayer.cpMin= Double.MAX_VALUE;
     minPlayer.cpAverage= Double.MAX_VALUE;
     minPlayer.cpRating= Double.MAX_VALUE;
     minPlayer.fwhAverage= Double.MAX_VALUE;
     minPlayer.fwhCount= Integer.MAX_VALUE;
     minPlayer.girAverage= Double.MAX_VALUE;
     minPlayer.girCount= Integer.MAX_VALUE;
     minPlayer.playSkins= Integer.MAX_VALUE;
     minPlayer.avgPSkins= Double.MAX_VALUE;
     minPlayer.teamSkins= Integer.MAX_VALUE;
     minPlayer.avgTSkins= Double.MAX_VALUE;
     minPlayer.skinPoints= Integer.MAX_VALUE;

     Player maxPlayer= new Player("MaxPlayer", "MaxPlayer");
     maxPlayer.nHighest= Integer.MIN_VALUE;
     maxPlayer.nLowest= Integer.MIN_VALUE;
     maxPlayer.nTotal= Integer.MIN_VALUE;
     maxPlayer.nAverage= Double.MIN_VALUE;
     maxPlayer.eTotal= Integer.MIN_VALUE;
     maxPlayer.gHighest= Integer.MIN_VALUE;
     maxPlayer.gLowest= Integer.MIN_VALUE;
     maxPlayer.gTotal= Integer.MIN_VALUE;
     maxPlayer.gAverage= Double.MIN_VALUE;
     for(int i= 0; i<maxPlayer.gStroke.length; i++)
       maxPlayer.gStroke[i]= maxPlayer.nStroke[i]= Integer.MIN_VALUE;
     maxPlayer.ldCount= Integer.MIN_VALUE;
     maxPlayer.ldTotal= Integer.MIN_VALUE;
     maxPlayer.ldMax= Integer.MIN_VALUE;
     maxPlayer.ldAverage= Double.MIN_VALUE;
     maxPlayer.ldRating= Double.MIN_VALUE;
     maxPlayer.cpCount= Integer.MIN_VALUE;
     maxPlayer.cpTotal= Double.MIN_VALUE;
     maxPlayer.cpMin= Double.MIN_VALUE;
     maxPlayer.cpAverage= Double.MIN_VALUE;
     maxPlayer.cpRating= Double.MIN_VALUE;
     maxPlayer.fwhAverage= Double.MIN_VALUE;
     maxPlayer.fwhCount= Integer.MIN_VALUE;
     maxPlayer.girAverage= Double.MIN_VALUE;
     maxPlayer.girCount= Integer.MIN_VALUE;
     maxPlayer.playSkins= Integer.MIN_VALUE;
     minPlayer.avgPSkins= Double.MIN_VALUE;
     maxPlayer.teamSkins= Integer.MIN_VALUE;
     minPlayer.avgTSkins= Double.MIN_VALUE;
     maxPlayer.skinPoints= Integer.MIN_VALUE;

     for(Iterator iter= playerMap.entrySet().iterator(); iter.hasNext();)
     {
       Map.Entry me= (Map.Entry)iter.next();
       Player player= (Player)me.getValue();
       if( player.days == 0 )
         continue;

       player.nAverage= average(player.nTotal,player.days);
       player.gAverage= average(player.gTotal,player.days);
       player.ldAverage= average(player.ldTotal, player.ldCount);
       player.ldRating= average(player.ldTotal, 2*player.days);
       player.cpAverage= average(player.cpTotal, player.cpCount);
       player.cpRating= cpRating(player);
       if( player.cpCount == 0 )
       {
         player.cpAverage= CLOSE_MAX;
         player.cpMin= CLOSE_MAX;
       }
       player.fwhAverage= average(player.fwhCount, player.days);
       player.girAverage= average(player.girCount, player.days);
       player.avgPSkins= average(player.playSkins, player.days);
       player.avgTSkins= average(player.teamSkins, player.days);

       //---------------------------------------------------------------------
       // Calculate MINIMUM result
       //---------------------------------------------------------------------
       if( player.nHighest < minPlayer.nHighest )
         minPlayer.nHighest= player.nHighest;
       if( player.nLowest < minPlayer.nLowest )
         minPlayer.nLowest= player.nLowest;
       if( player.nTotal < minPlayer.nTotal )
         minPlayer.nTotal= player.nTotal;
       if( player.nAverage < minPlayer.nAverage )
         minPlayer.nAverage= player.nAverage;

       if( player.eTotal < minPlayer.eTotal )
         minPlayer.eTotal= player.eTotal;
       if( player.gHighest < minPlayer.gHighest )
         minPlayer.gHighest= player.gHighest;
       if( player.gLowest < minPlayer.gLowest )
         minPlayer.gLowest= player.gLowest;
       if( player.gTotal < minPlayer.gTotal )
         minPlayer.gTotal= player.gTotal;
       if( player.gAverage < minPlayer.gAverage )
         minPlayer.gAverage= player.gAverage;

       for(int i= 0; i<minPlayer.gStroke.length; i++)
       {
         if( player.gStroke[i] < minPlayer.gStroke[i] )
            minPlayer.gStroke[i]= player.gStroke[i];
         if( player.nStroke[i] < minPlayer.nStroke[i] )
            minPlayer.nStroke[i]= player.nStroke[i];
       }

       if( player.ldCount < minPlayer.ldCount )
         minPlayer.ldCount= player.ldCount;
       if( player.ldTotal < minPlayer.ldTotal )
         minPlayer.ldTotal= player.ldTotal;
       if( player.ldMax < minPlayer.ldMax )
         minPlayer.ldMax= player.ldMax;
       if( player.ldAverage < minPlayer.ldAverage )
         minPlayer.ldAverage= player.ldAverage;
       if( player.ldRating < minPlayer.ldRating )
         minPlayer.ldRating= player.ldRating;

       if( player.cpCount < minPlayer.cpCount )
         minPlayer.cpCount= player.cpCount;
       if( player.cpTotal < minPlayer.cpTotal )
         minPlayer.cpTotal= player.cpTotal;
       if( player.cpMin < minPlayer.cpMin )
         minPlayer.cpMin= player.cpMin;
       if( player.cpAverage < minPlayer.cpAverage )
         minPlayer.cpAverage= player.cpAverage;
       if( player.cpRating < minPlayer.cpRating )
         minPlayer.cpRating= player.cpRating;

       if( player.fwhAverage < minPlayer.fwhAverage )
         minPlayer.fwhAverage= player.fwhAverage;
       if( player.fwhCount < minPlayer.fwhCount )
         minPlayer.fwhCount= player.fwhCount;
       if( player.girAverage < minPlayer.girAverage )
         minPlayer.girAverage= player.girAverage;
       if( player.girCount < minPlayer.girCount )
         minPlayer.girCount= player.girCount;

       if( player.playSkins < minPlayer.playSkins )
         minPlayer.playSkins= player.playSkins;
       if( player.avgPSkins < minPlayer.avgPSkins )
         minPlayer.avgPSkins= player.avgPSkins;
       if( player.teamSkins < minPlayer.teamSkins )
         minPlayer.teamSkins= player.teamSkins;
       if( player.avgTSkins < minPlayer.avgTSkins )
         minPlayer.avgTSkins= player.avgTSkins;
       if( player.skinPoints < minPlayer.skinPoints )
         minPlayer.skinPoints= player.skinPoints;

       //---------------------------------------------------------------------
       // Calculate MAXIMUM result
       //---------------------------------------------------------------------
       if( player.nHighest > maxPlayer.nHighest )
         maxPlayer.nHighest= player.nHighest;
       if( player.nLowest > maxPlayer.nLowest )
         maxPlayer.nLowest= player.nLowest;
       if( player.nTotal > maxPlayer.nTotal )
         maxPlayer.nTotal= player.nTotal;
       if( player.nAverage > maxPlayer.nAverage )
         maxPlayer.nAverage= player.nAverage;

       if( player.eTotal > maxPlayer.eTotal )
         maxPlayer.eTotal= player.eTotal;
       if( player.gHighest > maxPlayer.gHighest )
         maxPlayer.gHighest= player.gHighest;
       if( player.gLowest > maxPlayer.gLowest )
         maxPlayer.gLowest= player.gLowest;
       if( player.gTotal > maxPlayer.gTotal )
         maxPlayer.gTotal= player.gTotal;
       if( player.gAverage > maxPlayer.gAverage )
         maxPlayer.gAverage= player.gAverage;

       for(int i= 0; i<maxPlayer.gStroke.length; i++)
       {
         if( player.gStroke[i] > maxPlayer.gStroke[i] )
           maxPlayer.gStroke[i]= player.gStroke[i];
         if( player.nStroke[i] > maxPlayer.nStroke[i] )
           maxPlayer.nStroke[i]= player.nStroke[i];
       }

       if( player.ldCount > maxPlayer.ldCount )
         maxPlayer.ldCount= player.ldCount;
       if( player.ldTotal > maxPlayer.ldTotal )
         maxPlayer.ldTotal= player.ldTotal;
       if( player.ldMax > maxPlayer.ldMax )
         maxPlayer.ldMax= player.ldMax;
       if( player.ldAverage > maxPlayer.ldAverage )
         maxPlayer.ldAverage= player.ldAverage;
       if( player.ldRating > maxPlayer.ldRating )
         maxPlayer.ldRating= player.ldRating;

       if( player.cpCount > maxPlayer.cpCount )
         maxPlayer.cpCount= player.cpCount;
       if( player.cpTotal > maxPlayer.cpTotal )
         maxPlayer.cpTotal= player.cpTotal;
       if( player.cpMin > maxPlayer.cpMin )
         maxPlayer.cpMin= player.cpMin;
       if( player.cpAverage > maxPlayer.cpAverage )
         maxPlayer.cpAverage= player.cpAverage;
       if( player.cpRating > maxPlayer.cpRating )
         maxPlayer.cpRating= player.cpRating;

       if( player.fwhAverage > maxPlayer.fwhAverage )
         maxPlayer.fwhAverage= player.fwhAverage;
       if( player.fwhCount > maxPlayer.fwhCount )
         maxPlayer.fwhCount= player.fwhCount;
       if( player.girAverage > maxPlayer.girAverage )
         maxPlayer.girAverage= player.girAverage;
       if( player.girCount > maxPlayer.girCount )
         maxPlayer.girCount= player.girCount;

       if( player.playSkins > maxPlayer.playSkins )
         maxPlayer.playSkins= player.playSkins;
       if( player.avgPSkins < minPlayer.avgPSkins )
         minPlayer.avgPSkins= player.avgPSkins;
       if( player.teamSkins > maxPlayer.teamSkins )
         maxPlayer.teamSkins= player.teamSkins;
       if( player.avgTSkins < minPlayer.avgTSkins )
         minPlayer.avgTSkins= player.avgTSkins;
       if( player.skinPoints > maxPlayer.skinPoints )
         maxPlayer.skinPoints= player.skinPoints;
     }
//   maxPlayer.debug();
//   minPlayer.debug();

     // Generate the data panels
     DataPanel panel= new DataPanel();
     DataField field= genField(8, "Player");
     field.setHorizontalAlignment(DataField.LEFT);
     panel.add(field);
     panel.add(genField(4, "Days"));

     panel.add(genField(0, ""));
     panel.add(genPanel(6, "+GROSS", "+NET"));
     panel.add(genField(4, "LOW"));
     panel.add(genField(4, "HIGH"));
     panel.add(genField(4, "AVG"));

     panel.add(genField(0, ""));
     panel.add(genField(4, "<-2"));
     panel.add(genField(4, "-2"));
     panel.add(genField(4, "-1"));
     panel.add(genField(4, "PAR"));
     panel.add(genField(4, "+1"));
     panel.add(genField(4, "+2"));
     panel.add(genField(4, ">+2"));

     panel.add(genField(0, ""));
     panel.add(genPanel(6, "Avg TSkin", "Losses"));
     panel.add(genField(6, "Avg ISkin"));

     panel.add(genField(0, ""));
     panel.add(genField(4, "# LD"));
     panel.add(genField(4, "TOT"));
     panel.add(genField(4, "MAX"));
     panel.add(genField(4, "AVG"));
     panel.add(genField(4, "RATE"));

     panel.add(genField(0, ""));
     panel.add(genField(4, "# CP"));
     panel.add(genField(4, "TOT"));
     panel.add(genField(4, "MIN"));
     panel.add(genField(4, "AVG"));
     panel.add(genField(4, "RATE"));

     panel.add(genField(0, ""));
     panel.add(genField(4, "# FWH"));
     panel.add(genField(4, "AVG"));
     panel.add(genField(4, "# GIR"));
     panel.add(genField(4, "AVG"));
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
       panel.add(genPanel(4, "" + player.gLowest,
                          minPlayer.gLowest != maxPlayer.gLowest
                          && player.gLowest == minPlayer.gLowest
                          && player.days > 0,
                          false,
                             "" + player.nLowest,
                          minPlayer.nLowest != maxPlayer.nLowest
                          && player.nLowest == minPlayer.nLowest
                          && player.days > 0,
                          false));
       panel.add(genPanel(4, "" + player.gHighest,
                          false,
                          minPlayer.gHighest != maxPlayer.gHighest
                          && player.gHighest == maxPlayer.gHighest
                          && player.days > 0,
                              "" + player.nHighest,
                          false,
                          minPlayer.nHighest != maxPlayer.nHighest
                          && player.nHighest == maxPlayer.nHighest
                          && player.days > 0));
       panel.add(genPanel(4, "" + player.gAverage,
                          player.gAverage == minPlayer.gAverage
                          && player.days > 0,
                          player.gAverage == maxPlayer.gAverage
                          && player.days > 0,
                              "" + player.nAverage,
                          player.nAverage == minPlayer.nAverage
                          && player.days > 0,
                          player.nAverage == maxPlayer.nAverage
                          && player.days > 0));

       panel.add(genField(0, ""));
       for(int i= 0; i<4; i++)
         panel.add(genPanel(4,"" + player.gStroke[i],
                          minPlayer.gStroke[i] != maxPlayer.gStroke[i]
                          && player.gStroke[i] == maxPlayer.gStroke[i]
                          && player.days > 0,
                          false,
                              "" + player.nStroke[i],
                          minPlayer.nStroke[i] != maxPlayer.nStroke[i]
                          && player.nStroke[i] == maxPlayer.nStroke[i]
                          && player.days > 0,
                          false));
       for(int i= 4; i<maxPlayer.gStroke.length; i++)
         panel.add(genPanel(4,"" + player.gStroke[i],
                          false,
                          minPlayer.gStroke[i] != maxPlayer.gStroke[i]
                          && player.gStroke[i] == maxPlayer.gStroke[i]
                          && player.days > 0,
                              "" + player.nStroke[i],
                          false,
                          minPlayer.nStroke[i] != maxPlayer.nStroke[i]
                          && player.nStroke[i] == maxPlayer.nStroke[i]
                          && player.days > 0));

       panel.add(genField(0, ""));
       panel.add(genPanel(6, "" + player.avgTSkins,
                          player.avgTSkins == maxPlayer.avgTSkins
                          && player.days > 0,
                          player.avgTSkins == minPlayer.avgTSkins
                          && player.days > 0,
                             "" + player.skinPoints));
       panel.add(genField(6, "" + player.avgPSkins,
                          player.avgPSkins == maxPlayer.avgPSkins
                          && player.days > 0,
                          player.avgPSkins == minPlayer.avgPSkins
                          && player.days > 0));

       panel.add(genField(0, ""));
       panel.add(genField(4, "" + player.ldCount,
                          player.ldCount == maxPlayer.ldCount
                          && player.days > 0,
                          player.ldCount == minPlayer.ldCount
                          && player.days > 0));
       panel.add(genField(4, "" + player.ldTotal,
                          player.ldTotal == maxPlayer.ldTotal
                          && player.days > 0,
                          player.ldTotal == minPlayer.ldTotal
                          && player.days > 0));
       panel.add(genField(4, "" + player.ldMax,
                          minPlayer.ldMax != maxPlayer.ldMax
                          && player.ldMax == maxPlayer.ldMax
                          && player.days > 0,
                          false));
       panel.add(genField(4, "" + player.ldAverage,
                          minPlayer.ldAverage != maxPlayer.ldAverage
                          && player.ldAverage == maxPlayer.ldAverage
                          && player.days > 0,
                          false));
       panel.add(genField(4, "" + tenths(player.ldRating),
                          player.ldRating == maxPlayer.ldRating
                          && player.days > 0,
                          player.ldRating == minPlayer.ldRating
                          && player.days > 0));

       panel.add(genField(0, ""));
       panel.add(genField(4, "" + player.cpCount,
                          player.cpCount == maxPlayer.cpCount
                          && player.days > 0,
                          player.cpCount == minPlayer.cpCount
                          && player.days > 0));
       panel.add(genField(4, "" + player.cpTotal));
       panel.add(genField(4, "" + player.cpMin,
                          minPlayer.cpMin != maxPlayer.cpMin
                          && player.cpMin == minPlayer.cpMin
                          && player.days > 0,
                          false));
       panel.add(genField(4, "" + player.cpAverage,
                          minPlayer.cpAverage != maxPlayer.cpAverage
                          && player.cpAverage == minPlayer.cpAverage
                          && player.days > 0,
                          false));
       panel.add(genField(4, "" + player.cpRating,
                          player.cpRating == minPlayer.cpRating
                          && player.days > 0,
                          player.cpRating == maxPlayer.cpRating
                          && player.days > 0));

       panel.add(genField(0, ""));
       panel.add(genField(4, "" + player.fwhCount,
                          player.fwhCount == maxPlayer.fwhCount
                          && player.days > 0,
                          player.fwhCount == minPlayer.fwhCount
                          && player.days > 0));
       panel.add(genField(4, "" + player.fwhAverage,
                          minPlayer.fwhAverage != maxPlayer.fwhAverage
                          && player.fwhAverage == maxPlayer.fwhAverage
                          && player.days > 0,
                          false));
       panel.add(genField(4, "" + player.girCount,
                          player.girCount == maxPlayer.girCount
                          && player.days > 0,
                          player.girCount == minPlayer.girCount
                          && player.days > 0));
       panel.add(genField(4, "" + player.girAverage,
                          minPlayer.girAverage != maxPlayer.girAverage
                          && player.girAverage == maxPlayer.girAverage
                          && player.days > 0,
                          false));
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
//       EventsStat.Loader.done
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
     content.revalidate();
   }
   else
   {
     content.removeAll();
     loaderPanel= null;

     // Generate the content
     content.setLayout(new GridLayout(0,1));
     outputPanel.setLayout(new GridLayout(0,1));
     content.add(outputPanel);

     // Display the content
     content.revalidate();
   }
}
} // class EventsStat.Loader
} // class EventsStat
