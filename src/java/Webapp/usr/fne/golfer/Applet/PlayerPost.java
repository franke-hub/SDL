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
//       PlayerPost.java
//
// Purpose-
//       Scorecard data entry for event on date.
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
//       PlayerPost
//
// Purpose-
//       Score data entry.
//
//----------------------------------------------------------------------------
public class PlayerPost extends PlayerView {
//----------------------------------------------------------------------------
// PlayerPost.Attributes
//----------------------------------------------------------------------------
FocusListener          postListener;// The POST Listener
PostPanel              postPanel;   // The POST button Panel
DataField              head= null;  // Head entry element
DataField              tail= null;  // Tail entry element
Validator              validator;   // Validator object

// Hole constants
static final int       HOLE_COUNT= FORMAT_BASE;

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPost.PlayerPost
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PlayerPost( )                    // Constructor
{
   super();
   appletName= "PlayerPost";        // Set applet name
   owner= this;
   editable= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPost.actionPerformed
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
//       PlayerPost.createGUI
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
//       PlayerPost.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: PlayerPost v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Player round info.";
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPost.init
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
//       PlayerPost.isUpdateValid
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

   if( !playerPost.isValid(validator, score()) )
     return false;

   if( !playerPost.isValid(validator) )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPost.output
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

   String key= playerNN;
   key= concat(key, playerDate);
   if( playerTime != null )
     key= concat(key, playerTime);

   playerPost.isRemoved= score().equals("");

   playerPost.output(client, CMD_PLAYER_POST, key);

   playerPost.isRemoved= false;

   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPost.score
//
// Purpose-
//       Get the panel score from playerCard
//
//----------------------------------------------------------------------------
public String                       // The current score
   score( )                         // Get current score
{
   return playerCard.getEscText();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPost.update
//
// Purpose-
//       Update the data.
//
//----------------------------------------------------------------------------
public void
   update( )                        // Update the data
{
   debug("update()");

   playerPost.update(score());
}

//----------------------------------------------------------------------------
//
// Class-
//       PlayerPost.Format
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
//       PlayerPost.Format.doInBackground
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

     if( errorString != null )
       return null;

     // Generate the base panels
     courseShow.genPanel();
     courseHdcp.genPanel();
     courseHole.genPanel();
     coursePars.genPanel();
     courseTbox.genPanel();

     playerCard.genPanel();
     playerCard.setEscText(playerPost.score);

     DataPanel panel= courseShow.getPanel();
     DataField[] df= panel.getField();
     df[0].setBackground(Color.GREEN.brighter());

     panel= courseHole.getPanel();
     df= panel.getField();
     df[HOLE_ESC].setText("ESC");
     df[HOLE_HCP].setText("Hdcp");
     df[HOLE_NET].setText("Net");

     playerShow.genPanel();

     head= tail= playerCard.getPanel().getField(HOLE_ESC);
     head.setEditable(true);

     postPanel.setOperational(true);
   }}}} // synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerPost.Format.done
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

       content.add(postPanel);

       tail.addFocusListener(new NextFocusAdapter(head));
       head.requestFocusInWindow();

       content.revalidate();
       content.repaint();
     }
   }}}} // Synchronized(owner)
}
} // class PlayerPost.Format

//----------------------------------------------------------------------------
//
// Class-
//       PlayerPost.Loader
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
//       PlayerPost.Loader.doInBackground
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

       if( player_card != null )
         throw new Exception("Cannot update POST info when CARD data exists");

       playerCard= new PlayerCardInfo(HOLE_COUNT, stripQuotes(dbGet(CMD_PLAYER_SHOW, playerID)),
                                      empty, playerHdcp,
                                      coursePars, courseTbox);
       playerPost= new PlayerPostInfo(courseTbox, null, playerDate, playerTime,
                                      dbRetrieve(CMD_PLAYER_POST, playerKey));
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
//       PlayerPost.Loader.done
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
} // class PlayerPost.Loader

//----------------------------------------------------------------------------
//
// Class-
//       PlayerPost.Output
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
//       PlayerPost.Output.doInBackground
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
//       PlayerPost.Output.done
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
} // class PlayerPost.Output
} // class PlayerPost
