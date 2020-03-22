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
//       PlayerEdit.java
//
// Purpose-
//       Edit player data.
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
//       PlayerEdit
//
// Purpose-
//       Edit player data.
//
//----------------------------------------------------------------------------
public class PlayerEdit extends CommonJApplet implements ActionListener {
//----------------------------------------------------------------------------
// PlayerEdit.Attributes
//----------------------------------------------------------------------------
DataPanel              addrPanel;   // The player address Panel
DataPanel              identPanel;  // The player ID Panel
DataPanel              mailPanel;   // The player email Panel
DataPanel              nickPanel;   // The player nick Panel
DataPanel              namePanel;   // The player name Panel
DataPanel              showPanel;   // The player show Panel

DataField              head;        // Head entry element
DataField              tail;        // Tail entry element
PlayerEdit             owner;       // Owner Object
PostPanel              postPanel;   // The POST button Panel
FocusListener          postListener;// The POST Listener
Validator              validator;   // Our Validator

// Database items
String                 playerID;    // Player ID
String                 playerNN;    // Player NickName
GenericListInfo        playerFind;  // Player FIND indicator
GenericListInfo        playerShow;  // Player display name
GenericNameInfo        playerName;  // Player name, location
GenericListInfo        playerNick;  // Player nickname
GenericListInfo        playerMail;  // Player Mail address
GenericListInfo        removeNick;  // Player nickname removal

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.PlayerEdit
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   PlayerEdit( )                    // Default Constructor
{
   super("PlayerEdit");
   owner= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.createGUI
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
//       PlayerEdit.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: PlayerEdit v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display Player data.";
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info=
   {   {"player-nick", "string", "The player identifier, "
                               + "default is database default"}
   };
   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.init
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
//       PlayerEdit.isUnique
//
// Purpose-
//       Test whether the nickname is unique.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff nickname is unique
   isUnique( )
{
   String string= playerNick.getPanel().getField(1).getText();
   String target= dbGet(CMD_PLAYER_FIND, string);
   if( target != null && !target.equals(playerID) )
     return validator.invalid("Nick(" + string + ") in use for ID(" + target + ")");

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.isUpdateValid
//
// Purpose-
//       Test whether all data are valid.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff all data valid
   isUpdateValid( )
{
   debug("isUpdateValid()");
   postPanel.setMessage("");

   // Make sure the player info is present and valid
   if( !playerName.isValid(validator) )
     return false;

   if( !playerShow.isValid(validator) )
     return false;

   if( !playerNick.isValid(validator) )
     return false;

   if( !playerNick.list[0].equals(playerNick.getPanel().getFieldText(1))
       && !isUnique() )
     return false;

   if( !playerMail.isValid(validator) )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.loadAppletParameters
//
// Purpose-
//       Called in background to load the Applet parameters.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters()
{
   playerNN= getParameter("player-nick");
   debug("Param: player-nick: " + playerNN);
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.output
//
// Purpose-
//       Output the data.
//
//----------------------------------------------------------------------------
public void
   output( )                        // Output the data
{
   debug("output");

   dbReady();
   removeNick.output(client, CMD_PLAYER_FIND, removeNick.toString());
   playerFind.output(client, CMD_PLAYER_FIND, playerID);
   if( playerNick.isChanged || !playerNick.isPresent )
     dbPut(CMD_PLAYER_FIND, playerNick.toString(), playerID);
   playerNick.output(client, CMD_PLAYER_NICK, playerID);
   playerName.output(client, CMD_PLAYER_NAME, playerID);
   playerShow.output(client, CMD_PLAYER_SHOW, playerID);
   playerMail.output(client, CMD_PLAYER_MAIL, playerID);

   removeNick.list[0]= playerNick.list[0];
   removeNick.isPresent= true;
   removeNick.isRemoved= false;
   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.update
//
// Purpose-
//       Update the data.
//
//----------------------------------------------------------------------------
public void
   update( )                        // Update the data
{
   debug("update");

   removeNick.isRemoved=
       !removeNick.list[0].equals(playerNick.getPanel().getFieldText(1))
       && !removeNick.list[0].equals(playerID);

   playerFind.update();
   playerName.update();
   playerShow.update();
   playerNick.update();
   playerMail.update();
}

//----------------------------------------------------------------------------
//
// Class-
//       PlayerEdit.Format
//
// Purpose-
//       Create a background task to format the data.
//
//----------------------------------------------------------------------------
class Format extends SwingWorker<Object, Void> {
//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.Format.doInBackground
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

   // Indicate not initialized
   synchronized(owner)
   {{{{
     // Format the data
     try {
       // Generate the base panels
       tail= new DataField();
       tail= playerName.genPanel(postListener, tail);
       head= playerName.getPanel().getField(0);
       tail= playerShow.genPanel(postListener, tail);
       tail= playerNick.genPanel(postListener, tail);
       tail= playerMail.genPanel(postListener, tail);

       tail.addFocusListener(new NextFocusAdapter(head));

       // Activate the POST panel
       postPanel.setOperational(true);
     } catch( Exception e ) {
       setERROR("Format: Client exception: " + e);
       e.printStackTrace();
       return null;
     }
   }}}}

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.Format.done
//
// Purpose-
//       Replace the content.
//
// Notes-
//       Called by SwingWorker in EWT after doInBackground completes.
//
//----------------------------------------------------------------------------
protected DataPanel                 // (The input DataPanel)
   format(                          // Format
     DataPanel         panel)       // This Panel
{
   DataField[] df= panel.getField();
   df[0].setColumns(10);
   df[1].setColumns(90);
   return panel;
}

@Override
public void done()
{
   debug("Format.done()");
   content.removeAll();

   // If error, put message in the loaderPanel
   if( errorString != null )
   {
     loaderPanel.getField(0).setText(errorString);
     content.add(loaderPanel);
   }
   else
   {
     content.setLayout(new GridLayout(0,1));
     content.add(playerName.getPanel());
     content.add(format(playerShow.getPanel()));
     content.add(format(playerNick.getPanel()));
     content.add(format(playerMail.getPanel()));
     content.add(postPanel);
     head.requestFocusInWindow();
   }

   content.revalidate();
   content.repaint();
}
} // class PlayerEdit.Format

//----------------------------------------------------------------------------
//
// Class-
//       PlayerEdit.Loader
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
//       PlayerEdit.Loader.doInBackground
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

     loadAppletParameters();

     //-----------------------------------------------------------------------
     // Extract associated data
     playerID= dbGet(CMD_PLAYER_FIND, playerNN);
     if( playerID == null )
       throw new Exception("Cannot find PLAYER_ID");
     playerNN= dbGet(CMD_PLAYER_NICK, playerID);
     if( playerNN == null )
       throw new Exception("Cannot find PLAYER_NICK " + playerID);

     playerFind= new GenericListInfo("PlayerID", playerID);
     playerNick= new GenericListInfo("Database name", playerNN);
     playerShow= new GenericListInfo("Display name", dbGet(CMD_PLAYER_SHOW, playerID));
     playerShow.isQuoted= true;
     playerName= new GenericNameInfo(dbGet(CMD_PLAYER_NAME, playerID));
     playerMail= new GenericListInfo("E-mail", dbGet(CMD_PLAYER_MAIL, playerID));
     removeNick= new GenericListInfo("UNUSED", playerNN);
     removeNick.isPresent= true;
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
//       PlayerEdit.Loader.done
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
     Format format= new Format();
     format.execute();
   }
}
} // class PlayerEdit.Loader

//----------------------------------------------------------------------------
//
// Class-
//       PlayerEdit.Output
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
//       PlayerEdit.Output.doInBackground
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
   StringBuffer        buffer;      // Working StringBuffer
   boolean             isEqual;     // Working boolean

   debug("Output.doInBackground()");

   synchronized(owner)
   {{{{
     update();
     output();
   }}}}

   dbReset();

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.Output.done
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

   Format format= new Format();
   format.execute();
}
} // class PlayerEdit.Output

//----------------------------------------------------------------------------
//
// Method-
//       PlayerEdit.actionPerformed
//
// Purpose-
//       Called when POST button pressed in EntryPanel.
//
//----------------------------------------------------------------------------
public void
   actionPerformed(ActionEvent e)
{
   debug("actionPerformed()");

   Output output= new Output();
   output.execute();
}
} // class PlayerEdit
