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
//       Checkstop.java
//
// Purpose-
//       Throw RuntimeException, providing a way to get to the Java console.
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
//       Checkstop
//
// Purpose-
//       View course information.
//
//----------------------------------------------------------------------------
public class Checkstop extends CommonJApplet implements ActionListener {
//----------------------------------------------------------------------------
// Checkstop.Attributes
//----------------------------------------------------------------------------
// Internal data areas
Checkstop              owner;       // The Checkstop object
FocusListener          postListener;// The POST Listener
PostPanel              postPanel;   // The POST button Panel
Validator              validator;   // Validator object

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.Checkstop
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Checkstop( )                     // Constructor
{
   super("Checkstop");              // Set applet name
   owner= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.actionPerformed
//
// Purpose-
//       Handle POST Button depress event.
//
// Notes-
//       Called from Validator.actionPerformed().
//       This method implements the ActionListener interface.
//
//----------------------------------------------------------------------------
public void
   actionPerformed(ActionEvent e)
{
   new Abort().execute(); // Abort in background
}

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.createGUI
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

     public boolean isValid()
     {
       return true;
     }
   };

   // Create the PostPanel
   postPanel= new PostPanel(validator);
   postListener= postPanel.getListener();
}

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: Checkstop v1.0, 01 Feb 2013\n"
          + "Author: Frank Eskesen\n"
          + "Throw a RuntimeException.";
}

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info=
   {   {"checkstop", "string", "Has no meaning"}
   };
   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.init
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
   new Format().execute();
}

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.loadAppletParameters
//
// Purpose-
//       Called in background to load the Applet parameters.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters()
{
   debug("loadAppletParameters");
}

//----------------------------------------------------------------------------
//
// Class-
//       Checkstop.Format
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
//       Checkstop.Format.doInBackground
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
     postPanel.setOperational(true); // Initialization complete
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.Format.done
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
       loaderPanel.getField(0).setText("Exceptional APP ready");

       content.removeAll();
       content.setLayout(new GridLayout(0,1));
       content.add(loaderPanel);
       if( postListener != null )
         content.add(postPanel);
       content.revalidate();
       content.repaint();
   }}}} // Synchronized(owner)
}
} // class Checkstop.Format

//----------------------------------------------------------------------------
//
// Class-
//       Checkstop.Abort
//
// Purpose-
//       Create a background task to actually checkstop.
//
//----------------------------------------------------------------------------
class Abort extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.Abort.doInBackground
//
// Purpose-
//       Abort.
//
// Notes-
//       Called by SwingWorker via execute().
//
//----------------------------------------------------------------------------
@Override
public Object
   doInBackground()
{
   debug("Abort.doInBackground()");

   synchronized(owner)
   {{{{
       loaderPanel.getField(0).setText("CHECKSTOP");

       content.removeAll();
       content.setLayout(new GridLayout(0,1));
       content.add(loaderPanel);
       if( postListener != null )
         content.add(postPanel);
       content.revalidate();
       content.repaint();

       if( true )
         throw new RuntimeException("Checkstop"); // Our job is done
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       Checkstop.Abort.done
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
   debug("Abort.done()");
}
} // class Checkstop.Format
} // class Checkstop
