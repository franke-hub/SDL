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
//       2023/01/28
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.lang.*;
import java.util.*;
import javax.swing.*;

import usr.fne.common.*;

class Data { // OldMain Data =================================================
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
} // class OldMain ===========================================================

//----------------------------------------------------------------------------
//
// Class-
//       ButtonPanel
//
// Purpose-
//       Golf Button Panel.
//
//----------------------------------------------------------------------------
class ButtonPanel extends JPanel {
//----------------------------------------------------------------------------
// ButtonPanel.Attributes
//----------------------------------------------------------------------------
JButton                array[];     // The JButton Array

//----------------------------------------------------------------------------
//
// Method-
//       ButtonPanel.ButtonPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ButtonPanel(                     // Constructor
     int               count)       // The number of Buttons
{
   super();
   setLayout(new FlowLayout());
   setOpaque(true);

   array= new JButton[count];
   for(int i= 0; i<count; i++)
     array[i]= new JButton();
   setArray(array);
}

public
   ButtonPanel( )                   // Constructor
{
   this(1);
}

//----------------------------------------------------------------------------
//
// Method-
//       ButtonPanel.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   System.out.println("ButtonPanel.debug()");
   for(int i= 0; i<array.length; i++)
     System.out.println("[" + i + "] '" + array[i].getText() + "'");
}

//----------------------------------------------------------------------------
//
// Method-
//       ButtonPanel.getArray()
//       ButtonPanel.getArray(int)
//       ButtonPanel.getArrayText
//       ButtonPanel.getArrayValue
//
// Purpose-
//       Extract value.
//
//----------------------------------------------------------------------------
public JButton[]                    // The JButton[]
   getArray( )                      // Get JButton[]
{
   return array;
}

public JButton                      // The JButton
   getArray(                        // Get JButton
     int               index)       // For this index
{
   JButton             result= null; // Resultant

   if( index < array.length )
     result= array[index];

   return result;
}

public String                       // The JButton text
   getArrayText(                    // Get JButton text
     int               index)       // For this index
{
   return getArray(index).getText().trim();
}

//----------------------------------------------------------------------------
//
// Method-
//       ButtonPanel.setColor
//
// Purpose-
//       Update all array colors.
//
//----------------------------------------------------------------------------
public void
   setColor(                        // Update array colors
     Color             bg,          // Background color
     Color             fg)          // Foreground color
{
   if( array != null )
   {
     for(int i= 0; i<array.length; i++)
     {
       array[i].setBackground(bg);
       array[i].setForeground(fg);
     }
   }
}

public void
   setColor(                        // Update array colors
     Color[]           bgfg)        // [Background, foregound] colors
{
   setColor(bgfg[0], bgfg[1]);
}

//----------------------------------------------------------------------------
//
// Method-
//       ButtonPanel.setArray
//
// Purpose-
//       Set the JButton array, adding each JButton to the panel.
//
//----------------------------------------------------------------------------
public void
   setArray( )                      // Set the JButton array
{
   removeAll();
   for(int i= 0; i<array.length; i++)
   {
     array[i].setBorder(BorderFactory.createLineBorder(Color.BLACK, 2));
     add(array[i]);
   }
}

public void
   setArray(                        // Set the JButton array
     JButton           jbutton[])   // To this
{
   array= jbutton;
   setArray();
}
} // class ButtonPanel

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
String                 eventsID;    // The event identifier
JPanel                 panel[];     // JPanel array

// The executable program
String[]               parg;        // The program arguments
Program                prog;        // The program

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
   super();
   setTitle("Main");
}

public static void main(String[] args)
{
   Main main= new Main();

   if( args.length > 0 ) {
     OldMain.main(args);
     return;
   }

   main.run(args);
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
   setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

   // Set Frame size
   int width=  512;
   int height= 172;
   setSize(width, height);

   // Set Frame location
   Dimension size= Toolkit.getDefaultToolkit().getScreenSize();
   int screen_w= (int)size.getWidth();
   int screen_h= (int)size.getHeight();
   setLocation(screen_w/2 - width/2, 20);

   // Build the GUI
   for(;;)
   {
     new Loader().execute();
     waitUntilDone();
     if( prog != null )
     {
       prog.run(parg);              // Run the selected program
       return;
     }
   }
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
{  }

//----------------------------------------------------------------------------
//
// Class-
//       Main.InvokeCourseEdit
//
// Purpose-
//       Invoke CourseEdit(course-nick), update course information
//
//----------------------------------------------------------------------------
class InvokeCourseEdit implements ActionListener
{
String                 courseNick;  // The Associated Course

   InvokeCourseEdit(                // Constructor
     String            s)           // The course nickname
{  courseNick= s; }

public void
   actionPerformed(                 // Handle event date selected
     ActionEvent       e)           // The ActionEvent
{
   debug("InvokeCourseEdit.actionPerformed");

   prog= new CourseEdit();
   parg= new String[1];
   parg[0]= "course-nick=" + courseNick;
   waitingDone();
}
} // class InvokeCourseEdit

//----------------------------------------------------------------------------
//
// Class-
//       Main.InvokeEventsCard
//
// Purpose-
//       Invoke EventsCard(events-nick,events-date), update event scorecard
//
//----------------------------------------------------------------------------
class InvokeEventsCard implements ActionListener
{
public void
   actionPerformed(                 // Handle event date selected
     ActionEvent       e)           // The ActionEvent
{
   debug("InvokeEventsCard.actionPerformed");

   prog= new EventsCard();
   parg= new String[2];
   parg[0]= "events-nick=" + eventsID;
   parg[1]= "events-date=" + sortDate(e.getActionCommand());
   waitingDone();
}
} // class InvokeEventsCard

//----------------------------------------------------------------------------
//
// Class-
//       Main.InvokeEventsStat
//
// Purpose-
//       Invoke EventsStat(events-nick), display event statistical summary
//
//----------------------------------------------------------------------------
class InvokeEventsStat implements ActionListener
{
public void
   actionPerformed(                 // Handle event date selected
     ActionEvent       e)           // The ActionEvent
{
   debug("InvokeEventsStat.actionPerformed");

   prog= new EventsStat();
   parg= new String[1];
   parg[0]= "events-nick=" + eventsID;
   waitingDone();
}
} // class InvokeEventsCard

//----------------------------------------------------------------------------
//
// Class-
//       Main.InvokeEventsView
//
// Purpose-
//       Invoke EventsView(events-nick,events-date), view event team scorecard
//
//----------------------------------------------------------------------------
class InvokeEventsView implements ActionListener
{
public void
   actionPerformed(                 // Handle event date selected
     ActionEvent       e)           // The ActionEvent
{
   debug("InvokeEventsView.actionPerformed");

   prog= new EventsView();
   parg= new String[2];
   parg[0]= "events-nick=" + eventsID;
   parg[1]= "events-date=" + sortDate(e.getActionCommand());
   waitingDone();
}
} // class InvokeEventsView

//----------------------------------------------------------------------------
//
// Class-
//       Main.UpdateEvent
//
// Purpose-
//       Update Event
//
//----------------------------------------------------------------------------
class UpdateEvent implements ActionListener
{
String                 eventsNick;  // The Associated Event

   UpdateEvent(                     // Constructor
     String            s)           // The event identifier
{  eventsNick= s; }

public void
   actionPerformed(                 // Handle event date selected
     ActionEvent       e)           // The ActionEvent
{
   debug("UpdateEvent.actionPerformed");

   eventsID= eventsNick;
   waitingDone();
}
} // class UpdateEvent

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
     // Extract current data
     String[] string0= new String[0]; // (vector.toArray parameter)

     if( eventsID == null )
     {
       eventsID= dbGet(CMD_DEFAULT, DEFAULT_CODE_EI);
       if( eventsID == null )
         throw new Exception("Cannot find :" + DEFAULT_CODE_EI);
     }

     // Get all event identifiers
     Vector<String> vector= new Vector<String>();
     String CMD=  CMD_EVENTS_FIND;
     String NEXT= "00000";
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
       vector.add(item);
     }
     String eventsList[]= vector.toArray(new String[0]);

     // Get all courses
     vector= new Vector<String>();
     CMD=  CMD_COURSE_FIND;
     NEXT= "00000";
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
       vector.add(item);
     }

     String courseList[]= vector.toArray(new String[0]);
     String courseShow[]= new String[courseList.length];
     for(int i= 0; i<courseList.length; ++i)
       courseShow[i]= dbGet(CMD_COURSE_SHOW, courseList[i]);

     // Sort the courseShow array with courseList correlation
     for(int i= 0; i<courseList.length; ++i)
     {
       for(int j= i+1; j<courseList.length; ++j)
       {
         if( courseShow[j].compareTo(courseShow[i]) < 0 )
         {
           String tempList= courseList[i];
           String tempShow= courseShow[i];
           courseList[i]= courseList[j];
           courseShow[i]= courseShow[j];
           courseList[j]= tempList;
           courseShow[j]= tempShow;
         }
       }
     }

     // Get all event dates
     vector= new Vector<String>();
     CMD=  CMD_EVENTS_DATE;
     NEXT= eventsID;
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
       if( index >= 0 ) {
         String E= item.substring(0, index);
         if( !E.equals(eventsID) )
           break;
         vector.add(item.substring(index+1));
       }
     }

     String eventsDate[]= vector.toArray(new String[0]);
     String mdyyyyDate[]= new String[eventsDate.length];
     for(int i= 0; i<eventsDate.length; ++i)
       mdyyyyDate[i]= showDate(eventsDate[i]);

     //-----------------------------------------------------------------------
     // Generate panels
     panel= new JPanel[4];

     //-----------------------------------------------------------------------
     // Panel 0, Event selector
     String spacer= new String("                                ");
     MenuPanel p0= new MenuPanel(1);
     String show= stripQuotes(dbGet(CMD_EVENTS_SHOW, eventsID));
     JMenu jm= p0.getArray(0);
     jm.setText(spacer + show + spacer);
     for(int i= 0; i<eventsList.length; ++i) {
       show= stripQuotes(dbGet(CMD_EVENTS_SHOW, eventsList[i]));
       JMenuItem mi= new JMenuItem(show);
       mi.addActionListener(new UpdateEvent(eventsList[i]));
       jm.add(mi);
     }

     p0.getMenuBar().setBackground(Color.GREEN.brighter());
     p0.setLayout(new FlowLayout());
     panel[0]= p0;

     //-----------------------------------------------------------------------
     // Panel 1, Program menu selectors
     MenuPanel p1= new MenuPanel(3);

     // CourseEdit(course-nick)
     jm= p1.getArray(0);
     jm.setText("     Edit Course    ");
     for(int i= 0; i<courseShow.length; ++i) {
       JMenuItem mi= new JMenuItem(stripQuotes(courseShow[i]));
       mi.addActionListener(new InvokeCourseEdit(courseList[i]));
       jm.add(mi);
     }

     // EventsCard(events-nick, events-date)
     ActionListener invoke= new InvokeEventsCard();
     jm= p1.getArray(1);
     jm.setText("   Edit Score Card  ");
     for(int i= 0; i<mdyyyyDate.length; ++i) {
       JMenuItem mi= new JMenuItem(mdyyyyDate[i]);
       mi.addActionListener(invoke);
       jm.add(mi);
     }
     p1.setLayout(new FlowLayout());
     panel[1]= p1;

     // EventsView(events-nick, events-date)
     invoke= new InvokeEventsView();
     jm= p1.getArray(2);
     jm.setText(" View Net Scorecard ");
     for(int i= 0; i<mdyyyyDate.length; ++i) {
       JMenuItem mi= new JMenuItem(mdyyyyDate[i]);
       mi.addActionListener(invoke);
       jm.add(mi);
     }
     p1.setLayout(new FlowLayout());
     panel[1]= p1;

     //-----------------------------------------------------------------------
     // Panel 2, Button selectors
     ButtonPanel p2= new ButtonPanel(1);

     // EventsStat(events-nick)
     JButton jb= p2.getArray(0);
     jb.setText("  Event Statistics   ");
     jb.addActionListener(new InvokeEventsStat());
     p2.setLayout(new FlowLayout());
     panel[2]= p2;

     //-----------------------------------------------------------------------
     // Panel 3, Undefined
     DataPanel p3= new DataPanel(2);
     DataField df[]= p3.getField();
     df[0].setColumns(20);
     df[0].setText("TBD3a");
     df[1].setColumns(20);
     df[1].setText("TBD3b");
     p3.setLayout(new FlowLayout());
     panel[3]= p3;
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
     for(int i= 0; i<panel.length; i++)
       content.add(panel[i]);

     // Reshow the content
     content.revalidate();
     content.repaint();
   }

   setVisible(true);
// waitingDone();
}
} // class Main.Loader
} // class Main
