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
//       EventsAdd.java
//
// Purpose-
//       Add event information to database.
//
// Last change date-
//       2020/01/15
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
//       EventsAdd
//
// Purpose-
//       Add event information to database.
//
//----------------------------------------------------------------------------
public class EventsAdd extends EventsEdit {
//----------------------------------------------------------------------------
// EventsAdd.Attributes
//----------------------------------------------------------------------------
boolean                initialized; // TRUE after Events initialized

//----------------------------------------------------------------------------
//
// Method-
//       EventsAdd.EventsAdd
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   EventsAdd( )                     // Constructor
{
   super();

   appletName= "EventsAdd";
}

//----------------------------------------------------------------------------
//
// Class-
//       EventsAdd.Worker
//
// Purpose-
//       Create a background task to load the images.
//
//----------------------------------------------------------------------------
class Worker extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       EventsAdd.Worker.doInBackground
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
   debug("Worker.doInBackground()");
   dbReady();

   // Load the data
   try {
     if( client.fsm != DbStatic.FSM_READY )
       throw new Exception("DbServer offline");

     // Find free eventsID
     String CMD=  CMD_EVENTS_FIND;
     String LAST= "E0";
     String NEXT= "";
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

       char C= item.charAt(0);
       if( C < 'E' )
         continue;
       if( C > 'E' )
         break;

       NEXT= item;
       try {
         int last= Integer.parseInt(LAST.substring(1));
         int next= Integer.parseInt(NEXT.substring(1));
         if( next != (last+1) )
           break;

         LAST= NEXT;
       } catch(Exception e) {
         debug("item(" + item + "): " + e);
       }
     }

     int last= Integer.parseInt(LAST.substring(1));
     eventsNN= "" + (last+1);
     while( eventsNN.length() < 4 )
       eventsNN= "0" + eventsNN;
     eventsID= eventsNN= "E" + eventsNN;

     eventsShow=  new GenericItemInfo();
     eventsName=  new GenericNameInfo();

     // Collect Course, Teebox and Player information
     loadStaticInformation();

     // Initialize the handicap information
     for(int i= 0; i<player_info.length; i++)
     {
       PlayerInfo info= player_info[i];
       PlayerPostInfo[] post_info= loadPlayerPostInfo(info.playerNN, null);
       info.playerHdcp= "" + loadPlayerHdcp(post_info);
     }
   } catch( Exception e ) {
     setERROR("Worker: Client exception: " + e);
     e.printStackTrace();
     return null;
   }

   dbReset();

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsAdd.Worker.done
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
   debug("Worker.done()");

   // If error, put message in the loaderPanel
   if( errorString != null )
   {
     loaderPanel.getField(0).setText(errorString);
     content.revalidate();
   }
   else
     new Format().execute();
}
} // class EventsAdd.Worker

//----------------------------------------------------------------------------
//
// Method-
//       EventsAdd.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: EventsAdd v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Create event info.";
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsAdd.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   return null;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsAdd.init
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
   new Worker().execute();
   waitUntilDone();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsAdd.outputAdd
//
// Purpose-
//       Called from EventsEdit; Used to initialize the event.
//
//----------------------------------------------------------------------------
public void
   outputAdd( )                     // Generate EVENTS_FIND
{
   if( !initialized )
   {
     dbPut(CMD_EVENTS_FIND, eventsID, eventsID);
     initialized= true;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsAdd.outputDel
//
// Purpose-
//       Called from EventsEdit; Used to delete the event.
//
//----------------------------------------------------------------------------
public void
   outputDel( )                     // Remove EVENTS_FIND
{
   super.outputDel();
   initialized= false;
}
} // class EventsAdd
