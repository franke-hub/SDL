//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.java
//
// Purpose-
//       Command line interface
//
// Last change date-
//       2023/01/23
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.lang.*;
import java.util.*;
import javax.swing.*;

import usr.fne.common.*;

class Data {
static Program         p_code[]=

{  new CourseEdit()      // 00 Edit course info
,  new EventsCard()      // 01 Edit Event scorecard rounds
,  new EventsEdit()      // 02 Edit Event information
,  new EventsStat()      // 03 View Event statistics
,  new EventsAdd()       // 04 Generate a new Event
,  new EventsView()      // 05 View Event result for date
,  new PlayerHdcp()      // 06 View player handicap
,  new PlayerStat()      // 07 View player result
,  new PlayerView()      // 08 View player result for date

// CourseAdd     Add Course
// CourseView()  View course info
// EventsAdd     Add Event
// PlayerAdd     Add Player
// PlayerCard    Edit Player scorecard data
// PlayerEdit    Edit Player information
// PlayerPost    Edit Player result for date
// DefaultsEdit  Set defaults
}; // p_code[]

static String          p_name[]=
{  "CourseEdit"          // 00
,  "EventsCard"          // 01
,  "EventsEdit"          // 02
,  "EventsStat"          // 03
,  "EventsAdd"           // 04
,  "EventsView"          // 05
,  "PlayerHdcp"          // 06
,  "PlayerStat"          // 07
,  "PlayerView"          // 08
}; // p_name[]

static String          p_parm[]=
{  "course-nick=C0001"   // 00 courseedit   OK
,  ""                    // 01 eventscard   OK
+  "events-nick=GG2023,events-date=2023/02/25"
,  "events-nick=GG2023"  // 02 eventsedit   OK
,  "events-nick=GG2020"  // 03 eventsstat   OK
,  "events-nick=GG2023"  // 04 eventsadd    OK
,  ""                    // 05 eventsview   OK
+  "events-nick=GG2020,events-date=2020/02/25"
,  ""                    // 06 playerhdcp   OK
+  "player-nick=frank,MAX_DISPLAY=20"
,  "events-nick=GG2020"  // 07 playerstat   OK
,  ""                    // 08 playerview   OK
+  "events-nick=GG2020,player-nick=frank,"
+  "player-date=02/25/2020,player-time=09:10"
}; // p_parm[]
}  // class Data

//----------------------------------------------------------------------------
//
// Class-
//       OldMain
//
// Purpose-
//       Command line interface
//
//----------------------------------------------------------------------------
class OldMain {
public static void debug(String s)
{  System.out.println(s); }

public static void main(String[] args)
{
   for(int i=0; i<args.length; ++i) {
     debug("[" + i + "] '" + args[i] + "'");
   }
   if( args.length < 1 ) {
     System.err.println("No program specified");
     return;
   }
   String name= args[0];

   String  parm= null;
   Program code= null;
   int index= -1;
   for(int i=0; i<Data.p_name.length; ++i) {
     if( Data.p_name[i].equals(name) ) {
       index= i;
       code= Data.p_code[i];
       parm= Data.p_parm[i];
       break;
     }
   }
   if( code == null ) {
     System.err.println("Don't know how to run " + name);
     return;
   }

   ArrayList<String> plist= new ArrayList<String>();
   while( parm.length() > 0 ) {
     index= parm.indexOf(',');
     if( index < 0 )
       index= parm.length();
     plist.add(parm.substring(0, index));

     if( index >= parm.length() )
       parm= "";
     else
       parm= parm.substring(index+1);
   }
   String[] pargs= plist.toArray(new String[0]);
   code.run(pargs);
} // main()
} // class OldMain

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Main control window.
//
//----------------------------------------------------------------------------
public class Main extends CommonJFrame {

//----------------------------------------------------------------------------
// Main.Attributes
//----------------------------------------------------------------------------
DataPanel              dataPanel[]; // Data panel array

//----------------------------------------------------------------------------
//
// Method-
//       Main.Main
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Main( )
{
   super("Main");
}

public static void main(String[] args)
{
   Main main= new Main();

   if( args.length > 0 ) {
     OldMain.main(args);
     return;
   }

   String run_args[]= { "name=value" };
   main.run(run_args);
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.init
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
//       Main.loadAppletParameters
//
// Purpose-
//       Called in background to load the Applet parameters.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters()
{
}

//----------------------------------------------------------------------------
//
// Class-
//       Main.Loader
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
//       Main.Loader.doInBackground
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

     //-----------------------------------------------------------------------
     // Extract initial data
     String eventsID= dbGet(CMD_DEFAULT, DEFAULT_CODE_EI);
     if( eventsID == null )
       throw new Exception("Cannot find :" + DEFAULT_CODE_EI);
     eventsID= dbGet(CMD_EVENTS_SHOW, eventsID);

     //-----------------------------------------------------------------------
     // Generate panels
     dataPanel= new DataPanel[4];

     GenericItemInfo eventsShow= new GenericItemInfo(eventsID);
     DataPanel eventsPanel= eventsShow.genPanel();
     eventsPanel.getField(0).setBackground(Color.GREEN.brighter());
     dataPanel[0]= eventsPanel;

     DataPanel p1= new DataPanel(2);
     DataField[] df= p1.getField();
     df[0].setColumns(20);
     df[0].setText("Edit score card");
     df[1].setColumns(20);
     df[1].setText("TBD1b");
     dataPanel[1]= p1;

     DataPanel p2= new DataPanel(2);
     df= p2.getField();
     df[0].setColumns(20);
     df[0].setText("TBD2a");
     df[1].setColumns(20);
     df[1].setText("TBD2b");
     dataPanel[2]= p2;

     DataPanel p3= new DataPanel(2);
     df= p3.getField();
     df[0].setColumns(20);
     df[0].setText("TBD3a");
     df[1].setColumns(20);
     df[1].setText("TBD3b");
     dataPanel[3]= p3;
   } catch( Exception e ) {
     setERROR("Loader: doInBackground: Exception: " + e);
     e.printStackTrace();
     waitingDone();
     return null;
   }

   dbReset();

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.Loader.done
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
     content.removeAll();
     loaderPanel= null;

     content.setLayout(new GridLayout(0,1));
     for(int i= 0; i<dataPanel.length; i++)
       content.add(dataPanel[i]);

     // Reshow the content
     content.revalidate();
     content.repaint();
   }

   setVisible(true);
   waitingDone();
}
} // class Main.Loader
} // class Main
