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
//       EventsCard.java
//
// Purpose-
//       Scorecard data entry for event on date.
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
//       EventsCard
//
// Purpose-
//       Score data entry.
//
//----------------------------------------------------------------------------
public class EventsCard extends EventsView implements ActionListener {
//----------------------------------------------------------------------------
// EventsCard.Attributes
//----------------------------------------------------------------------------
EventsCard             owner;       // The EventsCard object
FocusListener          postListener;// The POST Listener
PostPanel              postPanel;   // The POST button Panel
DataField              head= null;  // Head entry element
DataField              tail= null;  // Tail entry element
Validator              validator;   // Validator object

// Hole constants
static final int       HOLE_COUNT= FORMAT_EVNT;

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.EventsCard
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   EventsCard( )                    // Constructor
{
   super();
   appletName= "EventsCard";        // Set applet name
   setTitle(appletName);
   owner= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.actionPerformed
//
// Purpose-
//       Called when POST button pressed in EntryPanel.
//
//----------------------------------------------------------------------------
public void
   actionPerformed(ActionEvent e)
{
   debug("actionPerformed()");
   new Output().execute();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.createGUI
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
   super.createGUI();

   // Create the Validator
   validator= new Validator()
   {
     public void actionPerformed( )
     {
       owner.actionPerformed(null);
     }

     public boolean invalid(String text)
     {
       debug("Validator.invalid(" + text + ")");
       postPanel.setMessage("ERROR: " + text);
       return false;
     }

     public boolean isValid()
     {
       return isUpdateValid();
     }
   };

   // Create the PostPanel
   postPanel= new PostPanel(validator);
   postListener= postPanel.getListener();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: EventsCard v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Event round data.";
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.init
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
//       EventsCard.isUpdateValid
//
// Purpose-
//       Check content validity.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff content is valid
   isUpdateValid( )                 // Check content validity
{
   debug("isUpdateValid()");
   postPanel.setMessage();

   for(int i= 0; i<teamVector.size(); i++)
   {
     EventsTeamInfo team= teamVector.elementAt(i);
     for(int j= 0; j<team.player_data.length; j++)
     {
       if( !team.player_card[j].isValid(validator) )
         return false;
       if( !team.player_post[j].isValid(validator) )
         return false;
     }
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.output
//
// Purpose-
//       Output the data.
//
//----------------------------------------------------------------------------
public void
   output( )                        // Update the data
{
   debug("output()");

   dbReady();

   for(int i= 0; i<teamVector.size(); i++)
   {
     EventsTeamInfo team= teamVector.elementAt(i);
     String key= eventsID;
     key= concat(key, eventsDate);
     key= concat(key, team.time);
     for(int j= 0; j<team.player_data.length; j++)
     {
       team.player_card[j].isRemoved= team.player_post[j].isRemoved=
           team.player_card[j].isDefault();

       String playerKey= concat(key, team.player_data[j].playerNN);
       team.player_card[j].output(client, CMD_EVENTS_CARD, playerKey);
       team.player_post[j].output(client, CMD_EVENTS_POST, playerKey);

       team.player_card[j].isRemoved= team.player_post[j].isRemoved= false;
     }
   }

   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.update
//
// Purpose-
//       Update the data.
//
//----------------------------------------------------------------------------
public void
   update( )                        // Update the data
{
   debug("update()");

   for(int i= 0; i<teamVector.size(); i++)
   {
     EventsTeamInfo team= teamVector.elementAt(i);
     for(int j= 0; j<team.player_data.length; j++)
     {
       if( !team.player_card[j].isRemoved )
       {
         team.player_card[j].update();
         team.player_post[j].update();
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       EventsCard.Format
//
// Purpose-
//       Format the content.
//
//----------------------------------------------------------------------------
class Format extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.Format.doInBackground
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
   debug("Format.doInBackground()");

   synchronized(owner)
   {{{{
     postPanel.setOperational(false);

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

     // Generate the TEAM panels
     head= null;
     tail= new DataField();
     for(int i= 0; i<teamVector.size(); i++)
     {
       EventsTeamInfo team= teamVector.elementAt(i);
       for(int j= 0; j<team.player_data.length; j++)
       {
         PlayerData player= team.player_data[j];
         PlayerCardInfo cardInfo= team.player_card[j];
         tail= cardInfo.genPanel(postListener, tail);
         if( head == null )
           head= cardInfo.getPanel().getField(1);
       }

       team.ldcpPanel= (HolePanel)courseLdcp.genPanel();
       team.highlight(courseLdcp);
       // team.teamPanel= new ItemPanel(showDate(eventsDate) + " -- " + team.time);
       // team.teamPanel.getField(0).setBackground(Color.GREEN.brighter());
     }

     postPanel.setOperational(true);
   }}}} // synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.Format.done
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
   debug("Format.done()");

   // If error, put message in the loaderPanel
   if( errorString != null )
   {
     loaderPanel.getField(0).setText(errorString);
     content.revalidate();
   }
   else
   {
     content.removeAll();
     content.setLayout(new GridLayout(0,1));

     // The TEAM panels
     for(int i= 0; i<teamVector.size(); i++)
     {
       EventsTeamInfo team= teamVector.elementAt(i);

       if( i != 0 )
       {
         content.add(new ItemPanel()); // Spacer
       }

       String S= showDate(eventsDate) + " " + team.time
               + " -- " + courseShow + " -- "
               + courseTbox.stringMR + "/" + courseTbox.stringMS;
       courseItem= new GenericItemInfo(S);
       courseItem.genPanel();
       courseHole.genPanel();
       courseTbox.genPanel();
       coursePars.genPanel();

       content.add(courseItem.getPanel());
       content.add(courseHole.getPanel());
       content.add(courseTbox.getPanel());
       content.add(coursePars.getPanel());

       for(int j= 0; j<team.player_data.length; j++)
         content.add(team.player_card[j].getPanel());

       content.add(team.ldcpPanel);
     }

     // The POST panel
     content.add(postPanel);

     if( head != null )
     {
       tail.addFocusListener(new NextFocusAdapter(head));
       head.requestFocusInWindow();
     }

     // Reshow the content
     content.revalidate();
     setVisible(true);
     waitingDone();
   }
}
} // class EventsCard.Format

//----------------------------------------------------------------------------
//
// Class-
//       EventsCard.Loader
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
//       EventsCard.Loader.doInBackground
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

       for(int i= 0; i<teamVector.size(); i++)
       {
         EventsTeamInfo team= teamVector.elementAt(i);
         team.player_post= new PlayerPostInfo[team.player_data.length];
         for(int j= 0; j<team.player_data.length; j++)
         {
           PlayerData player= team.player_data[j];
           PlayerCardInfo cardInfo= team.player_card[j];
           if( cardInfo == null )
             team.player_card[j]= cardInfo=
                 new PlayerCardInfo(HOLE_COUNT, player.playerShow, empty,
                                    player.playerHdcp, coursePars, courseTbox);

           String key= concat(eventsID, eventsDate);
           key= concat(key, team.time);
           key= concat(key, player.playerNN);
           team.player_post[j]=
                 new PlayerPostInfo(courseTbox, cardInfo, team.date, team.time,
                                    dbRetrieve(CMD_EVENTS_POST, key));
         }
       }
     } catch( Exception e ) {
       setERROR("Loader: doInBackground: Exception: " + e);
       e.printStackTrace();
     }

     dbReset();
   }}}} // synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.Loader.done
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
} // class EventsCard.Loader

//----------------------------------------------------------------------------
//
// Class-
//       EventsCard.Output
//
// Purpose-
//       Create a background task to write the data.
//
//----------------------------------------------------------------------------
class Output extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.Output.doInBackground
//
// Purpose-
//       Write the data.
//
// Notes-
//       Called by SwingWorker via execute().
//
//----------------------------------------------------------------------------
@Override
public Object
   doInBackground()
{
   debug("Output.doInBackground()");

   synchronized(owner)
   {{{{
     postPanel.setOperational(false);
     update();
     output();
     postPanel.setOperational(true);
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsCard.Output.done
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
   debug("Output.done()");

   new Format().execute();
}
} // class EventsCard.Output
} // class EventsCard
