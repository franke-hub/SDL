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
//       PlayerCard.java
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

// import usr.fne.common.SwingWorker;
import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       PlayerCard
//
// Purpose-
//       Score data entry.
//
//----------------------------------------------------------------------------
public class PlayerCard extends PlayerView {
//----------------------------------------------------------------------------
// PlayerCard.Attributes
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
//       PlayerCard.PlayerCard
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PlayerCard( )                    // Constructor
{
   super();
   appletName= "PlayerCard";        // Set applet name
   owner= this;
   editable= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCard.actionPerformed
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
//       PlayerCard.createGUI
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
//       PlayerCard.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: PlayerCard v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Player round info.";
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCard.init
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
//       PlayerCard.isUpdateValid
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

   if( !playerCard.isValid(validator) )
     return false;

   if( !playerPost.isValid(validator) )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCard.output
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

   playerCard.isRemoved= playerPost.isRemoved= playerCard.isDefault();

   playerCard.output(client, CMD_PLAYER_CARD, key);
   playerPost.output(client, CMD_PLAYER_POST, key);

   playerCard.isRemoved= playerPost.isRemoved= false;

   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCard.update
//
// Purpose-
//       Update the data.
//
//----------------------------------------------------------------------------
public void
   update( )                        // Update the data
{
   debug("update()");

   playerCard.update();
   playerPost.update();
}

//----------------------------------------------------------------------------
//
// Class-
//       PlayerCard.Format
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
//       PlayerCard.Format.doInBackground
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

     DataPanel panel= courseShow.getPanel();
     DataField[] df= panel.getField();
     df[0].setBackground(Color.GREEN.brighter());

     panel= courseHole.getPanel();
     df= panel.getField();
     df[HOLE_ESC].setText("ESC");
     df[HOLE_HCP].setText("Hdcp");
     df[HOLE_NET].setText("Net");

     // Note that since playerNets and playerCard use the same data array,
     // the playerNets object data was automatically updated.
     playerShow.genPanel();
     tail= playerCard.genPanel(postListener, new DataField());
     playerNets.genPanel();

     head= playerCard.getPanel().getField(1);

     postPanel.setOperational(true);
   }}}} // synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCard.Format.done
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
       content.add(playerNets.getPanel());

       content.add(postPanel);

       tail.addFocusListener(new NextFocusAdapter(head));
       head.requestFocusInWindow();

       content.revalidate();
       content.repaint();
     }
   }}}} // Synchronized(owner)
}
} // class PlayerCard.Format

//----------------------------------------------------------------------------
//
// Class-
//       PlayerCard.Loader
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
//       PlayerCard.Loader.doInBackground
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
   }}}} // synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerCard.Loader.done
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
} // class PlayerCard.Loader

//----------------------------------------------------------------------------
//
// Class-
//       PlayerCard.Output
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
//       PlayerCard.Output.doInBackground
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
//       PlayerCard.Output.done
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
} // class PlayerCard.Output
} // class PlayerCard
