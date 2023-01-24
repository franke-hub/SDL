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
//       EventsTeam.java
//
// Purpose-
//       Generate the teams for an Event.
//
// Last change date-
//       2023/01/19
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
//       EventsTeam
//
// Purpose-
//       Generate the teams for an Event.
//
//----------------------------------------------------------------------------
public class EventsTeam extends CommonJFrame {
//----------------------------------------------------------------------------
// EventsTeam.Attributes
//----------------------------------------------------------------------------
// Monte Carlo evaluation attributes
int                    CLEAN_COUNT= 0;    // Number of clean evaluations
int                    PROBE_COUNT= 0;    // Number of evaluation probes
boolean                DEBUG_EVAL= false; // <DEBUG_EVAL> DEBUG evaluation methods?

// Database items
String                 eventsID;    // Events ID
String                 eventsNN;    // <events-nick> Event identifier

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeam.EventsTeam
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
public
   EventsTeam( )                    // Constructor
{
   super("EventsTeam");             // Set applet name
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeam.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: EventsTeam v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Player handicap information.";
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeam.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info=
   {   {"events-nick", "string", "The event's name, "
                               + "default is database default"}
   ,   {"CLEAN_COUNT", "integer", "Number of clean MonteCarlo iterations required"}
   ,   {"PROBE_COUNT", "integer", "The number of Monte Carlo evaluation probes"}
   ,   {"DEBUG_EVAL", "boolean", "TRUE to debug evaluation method, "
                               + "default is false"}
   };

   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeam.init
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
   loaderPanel.getField(0).setText("Working");
   new Loader().execute();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeam.loadAppletParameters
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

   String string= getParameter("CLEAN_COUNT");
   if( string != null )
   {
     StringBuffer buffer= new StringBuffer();
     for(int i= 0; i<string.length(); i++)
     {
       if( string.charAt(i) != ',' )
         buffer.append(string.charAt(i));
     }
     CLEAN_COUNT= Integer.parseInt(buffer.toString());
   }

   string= getParameter("PROBE_COUNT");
   if( string != null )
   {
     StringBuffer buffer= new StringBuffer();
     for(int i= 0; i<string.length(); i++)
     {
       if( string.charAt(i) != ',' )
         buffer.append(string.charAt(i));
     }
     PROBE_COUNT= Integer.parseInt(buffer.toString());
   }

   string= getParameter("DEBUG_EVAL");
   if( string != null )
     DEBUG_EVAL= Boolean.parseBoolean(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeam.evaluate
//
// Purpose-
//       Called in background to run the MonteCarloTeamEvaluator
//
//----------------------------------------------------------------------------
protected void
   evaluate(                        // Run the team evaluation
     EventsDateInfo[]  date_info)   // The EventsDateInfo array
{

   // Run the MonteCarlo evaluator
   boolean onReset= dbReady();

   MonteCarloTeamEvaluator evaluator=
       new MonteCarloTeamEvaluator(date_info, CLEAN_COUNT, PROBE_COUNT);

   evaluator.DEBUG= DEBUG;
   evaluator.DEBUG_EVAL= DEBUG_EVAL;
   evaluator.DEBUG_HCDM= DEBUG_HCDM;
   EventsTeamInfo[] team_info= evaluator.evaluate();

   // Add debugging information to database
   dbPut(CMD_COMMENT, "#########", "####################################################");
   dbPut(CMD_COMMENT, "captEval:", "" + evaluator.captFactor + " * " + evaluator.captEval);
   dbPut(CMD_COMMENT, "cseqEval:", "" + evaluator.cseqFactor + " * " + evaluator.cseqEval);
   dbPut(CMD_COMMENT, "diffEval:", "" + evaluator.diffFactor + " * " + evaluator.diffEval);
   dbPut(CMD_COMMENT, "dseqEval:", "" + evaluator.dseqFactor + " * " + evaluator.dseqEval);
   dbPut(CMD_COMMENT, "partEval:", "" + evaluator.partFactor + " * " + evaluator.partEval);
   dbPut(CMD_COMMENT, "pseqEval:", "" + evaluator.pseqFactor + " * " + evaluator.pseqEval);
   dbPut(CMD_COMMENT, "selfEval:", "" + evaluator.selfFactor + " * " + evaluator.selfEval);
   dbPut(CMD_COMMENT, "sseqEval:", "" + evaluator.sseqFactor + " * " + evaluator.sseqEval);
   dbPut(CMD_COMMENT, "teamEval:", "" + evaluator.teamFactor + " * " + evaluator.teamEval);
   dbPut(CMD_COMMENT, "tseqEval:", "" + evaluator.tseqFactor + " * " + evaluator.tseqEval);
   dbPut(CMD_COMMENT, "xtraEval:", "" + evaluator.xtraFactor + " * " + evaluator.xtraEval);
   dbPut(CMD_COMMENT, "bestEval:", "" + evaluator.resultant);

   // Add evaluation information to database
   for(int i= 0; i<team_info.length; i++)
   {
     EventsTeamInfo team= team_info[i];
     String key= eventsID;
     key= concat(key, team.date);
     key= concat(key, team.time);
     dbPut(CMD_EVENTS_TIME, key, team.toString());
   }

   dbReset(onReset);
}

//----------------------------------------------------------------------------
//
// Class-
//       EventsTeam.Loader
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
//       EventsTeam.Loader.loadPlayer
//
// Purpose-
//       Load PlayerData from PLAYER_NN.
//
//----------------------------------------------------------------------------
public PlayerData                   // Resultant PlayerData
   loadPlayer(                      // Load PlayerData
     String            playerNN)    // From this PLAYER_NN
   throws Exception
{
   PlayerData player= new PlayerData();
   player.playerNN= playerNN;
   player.playerID= dbGet(CMD_PLAYER_FIND, player.playerNN);
   if( player.playerID == null )
     throw new Exception("Missing data: "
                       + CMD_PLAYER_FIND + " " + player.playerNN);
   player.playerNN= dbGet(CMD_PLAYER_NICK, player.playerID);
   if( player.playerNN == null )
     throw new Exception("Missing data: "
                       + CMD_PLAYER_NICK + " " + player.playerID);

   return player;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsTeam.Loader.doInBackground
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
   debug("TeamLoader.doInBackground()");

   dbReady();

   // Load the data
   try {
     if( client.fsm != DbStatic.FSM_READY )
       throw new Exception("DbServer offline");

     //-----------------------------------------------------------------------
     // Extract associated data
     eventsID= dbGet(CMD_EVENTS_FIND, eventsNN);
     if( eventsID == null )
       eventsID= dbGet(CMD_DEFAULT, DEFAULT_CODE_EI);
     if( eventsID == null )
       throw new Exception("Cannot find EVENTS_ID");

     // Generate EventsDateInfo Vector
     Vector<EventsDateInfo> dateVector= new Vector<EventsDateInfo>();
     String CMD=  CMD_EVENTS_DATE;
     String NEXT= eventsID;
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
       int index= catcon(item);
       if( index < 0 )
         continue;

       String itemNick= item.substring(0, index);
       if( !itemNick.equalsIgnoreCase(eventsID) )
         break;

       EventsDateInfo evDateData= new EventsDateInfo();
       evDateData.courseID= t.nextToken();
       evDateData.teeboxID= t.nextToken();
       evDateData.date= item.substring(index+1);
       evDateData.time= tokenize(t.remainder());

       String[] player_NN= dbRetrieve(CMD_EVENTS_PLAY, item);
       if( player_NN == null )
         throw new Exception("Missing data: "
                           + CMD_EVENTS_PLAY + " " + item);

       evDateData.player_data= new PlayerData[player_NN.length];
       for(int i= 0; i<player_NN.length; i++)
         evDateData.player_data[i]= loadPlayer(player_NN[i]);

       dateVector.add(evDateData);
     }

     // Generate EventsDateInfo Array
     EventsDateInfo[] date_info= new EventsDateInfo[dateVector.size()];
     for(int i= 0; i<date_info.length; i++)
       date_info[i]= dateVector.elementAt(i);

     // Run the Monte Carlo evaluator
     evaluate(date_info);
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
//       EventsTeam.Loader.done
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
   debug("TeamLoader.done()");

   // If error, put message in the loaderPanel
   if( errorString != null )
   {
     loaderPanel.getField(0).setText(errorString);
   }
   else
   {
     content.removeAll();
     content.add(new ItemPanel("DONE"));
   }

   // Reshow the content
   content.revalidate();
   setVisible(true);
}
} // class EventsTeam.Loader
} // class EventsTeam
