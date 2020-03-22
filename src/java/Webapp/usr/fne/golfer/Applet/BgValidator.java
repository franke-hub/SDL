//----------------------------------------------------------------------------
//
//       Copyright (C) 2017-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       BgValidator.java
//
// Purpose-
//       Validate all data. (Window management portion.)
//
// Last change date-
//       2020/01/15
//
// Usage notes-
//       This program displays database anomolies.
//       Use "make BgValidator" to run it.
//
//       This program only runs in the build envionment. There is no
//       provision for running this program in production mode.
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
//       BgValidator
//
// Purpose-
//       Validate all data.
//
//----------------------------------------------------------------------------
public class BgValidator extends CommonJApplet {
//----------------------------------------------------------------------------
// BgValidator.Attributes
//----------------------------------------------------------------------------
int                    CLEAN_COUNT= 0;    // Number of clean evaluations
int                    PROBE_COUNT= 0;    // Number of evaluation probes
boolean                DEBUG_EVAL= false; // <DEBUG_EVAL> DEBUG evaluation methods?

// Database items
String                 eventsID;    // Events ID
String                 eventsNN;    // <events-nick> Event identifier

//----------------------------------------------------------------------------
//
// Method-
//       BgValidator.BgValidator
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
public
   BgValidator( )                   // Constructor
{
   super("BgValidator");            // Set applet name
}

//----------------------------------------------------------------------------
//
// Method-
//       BgValidator.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: BgValidator v1.0, 01 Jan 2017\n"
          + "Author: Frank Eskesen\n"
          + "Display Player handicap information.";
}

//----------------------------------------------------------------------------
//
// Method-
//       BgValidator.getParameterInfo
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
   ,   {"CLEAN_COUNT", "integer", "Number of clean iterations required"}
   ,   {"PROBE_COUNT", "integer", "The number of evaluation probes"}
   ,   {"DEBUG_EVAL", "boolean", "TRUE to debug evaluation method, "
                               + "default is false"}
   };

   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       BgValidator.init
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
   content.removeAll();
   content.setLayout(new GridLayout(0,1));
   loaderPanel= new ItemPanel("Working");
   content.add(loaderPanel);
   new Loader().execute();
}

//----------------------------------------------------------------------------
//
// Method-
//       BgValidator.loadAppletParameters
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
//       BgValidator.foreground
//
// Purpose-
//       Called in background to run the Database validator.
//
//----------------------------------------------------------------------------
protected void
   foreground( )                    // Run the database validator
{
   // Run the validator
   FgValidator validator= new FgValidator(this);

   validator.DEBUG= DEBUG;
   validator.DEBUG_EVAL= DEBUG_EVAL;
   validator.DEBUG_HCDM= DEBUG_HCDM;
   validator.validate();
}

//----------------------------------------------------------------------------
//
// Class-
//       BgValidator.Loader
//
// Purpose-
//       Create a background task to load the images.
//
//----------------------------------------------------------------------------
class Loader extends SwingWorker<Object, Void> {
//----------------------------------------------------------------------------
//
// Method-
//       BgValidator.Loader.doInBackground
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

     // Run the Database validator
     foreground();
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
//       BgValidator.Loader.done
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
   debug("Loader.done()");

   // If error, put message in the loaderPanel
   if( errorString != null )
   {
     loaderPanel.getField(0).setText(errorString);
     content.revalidate();
   }
   else
   {
     // Reshow the content
     content.revalidate();
   }
}
} // class BgValidator.Loader
} // class BgValidator
