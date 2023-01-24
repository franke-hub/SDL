//----------------------------------------------------------------------------
//
//       Copyright (C) 2017-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DefaultsEdit.java
//
// Purpose-
//       Edit defaults data.
//
// Last change date-
//       2023/01/19
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import java.io.IOException;
import java.lang.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;

import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       DefaultsEdit
//
// Purpose-
//       Edit player data.
//
//----------------------------------------------------------------------------
public class DefaultsEdit extends CommonJFrame implements ActionListener {
//----------------------------------------------------------------------------
// DefaultsEdit.Attributes
//----------------------------------------------------------------------------
DataField              head;        // Head entry element
DataField              tail;        // Tail entry element
DefaultsEdit           owner;       // Owner Object
PostPanel              postPanel;   // The POST button Panel
FocusListener          postListener;// The POST Listener
Validator              validator;   // Our Validator

// Database items
String                 dateID;      // Current date
String                 timeID;      // Current time
String                 courseID;    // Course ID
String                 teeboxID;    // Teebox ID
String                 eventsID;    // Events ID
String                 playerID;    // Player ID
String                 playerNN;    // Player NickName
GenericListInfo        defaultDate; // Default date and time
ListPanel              coursePanel; // The course panel
ListPanel[]            teeboxArray; // The teebox panel array
ListPanel              teeboxPanel; // The teebox panel
ListPanel              eventsPanel; // The events panel
ListPanel              playerPanel; // The player panel

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.DefaultsEdit
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   DefaultsEdit( )                  // Default Constructor
{
   super("DefaultsEdit");
   owner= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.actionPerformed
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

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.createGUI
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
//       DefaultsEdit.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: DefaultsEdit v1.0, 01 Jan 2010\n"
          + "Author: Frank Eskesen\n"
          + "Edit Defaults data.";
}

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info= null;
   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.init
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
//       DefaultsEdit.isUpdateValid
//
// Purpose-
//       Test whether all data are valid.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff all data valid
   isUpdateValid( )
{
   boolean             result= true;// Resultant

   debug("isUpdateValid()");
   postPanel.setMessage("");

   // Check date and time
   DataField[] df= defaultDate.getPanel().getField();
   if( !df[1].getText().equals("") || !df[2].getText().equals("") )
   {
     if( !validator.isValidDate(df[1]) )
       return false;

     if( !df[2].getText().equals("") )
       if( !validator.isValidTime(df[2]) )
         return false;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.loadAppletParameters
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
// Method-
//       DefaultsEdit.output
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

   defaultDate.output(client, CMD_DEFAULT, DEFAULT_CODE_DT);
   coursePanel.output(client, CMD_DEFAULT, DEFAULT_CODE_CI);
   teeboxPanel.output(client, CMD_DEFAULT, DEFAULT_CODE_TI);
   eventsPanel.output(client, CMD_DEFAULT, DEFAULT_CODE_EI);
   playerPanel.output(client, CMD_DEFAULT, DEFAULT_CODE_PI);

   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.update
//
// Purpose-
//       Update the data.
//
//----------------------------------------------------------------------------
public void
   update( )                        // Update the data
{
   debug("update");
   defaultDate.update();
}

//----------------------------------------------------------------------------
//
// Class-
//       DefaultsEdit.Format
//
// Purpose-
//       Create a background task to format the data.
//
//----------------------------------------------------------------------------
class Format extends SwingWorker<Object, Void> {
//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.Format.doInBackground
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
//       DefaultsEdit.Format.done
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
     content.setLayout(new BorderLayout());
     JPanel panel= new JPanel();
     panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
     panel.add(defaultDate.getPanel());
     panel.add(eventsPanel);
     panel.add(playerPanel);
     panel.add(coursePanel);
     panel.add(teeboxPanel);

     content.add(panel, BorderLayout.CENTER);
     content.add(postPanel, BorderLayout.SOUTH);
     head.requestFocusInWindow();
   }

   content.revalidate();
   content.repaint();
   setVisible(true);
}
} // class DefaultsEdit.Format

//----------------------------------------------------------------------------
//
// Class-
//       DefaultsEdit.ListItem
//
// Purpose-
//       Define an item to be placed in the ListPanel object.
//
//----------------------------------------------------------------------------
class ListItem {
String                 itemID;      // The item identifier
String                 itemShow;    // The item display field

public String
   toString( )                      // Convert to String
{
   return itemShow;
}
} // class DefaultsEdit.ListItem

//----------------------------------------------------------------------------
//
// Class-
//       DefaultsEdit.ListPanel
//
// Purpose-
//       Define a list Panel.
//
//----------------------------------------------------------------------------
class ListPanel extends JPanel implements ListSelectionListener {
boolean                isCoursePanel; // True iff CoursePanel
JList                  list;        // The JList
DefaultListModel<ListItem>
                       listModel;   // The ListModel
int                    oldSelected; // The prior listModel selection index

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.ListPanel.ListPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ListPanel(                       // Constructor
     String            name,        // The panel name
     ListItem[]        items,       // Array of associated ListItems
     String            selected)    // The selected item
{
   super(new BorderLayout());
   isCoursePanel= false;

   // Create the ListModel
   listModel= new DefaultListModel<ListItem>();
   for(int i= 0; i < items.length; i++)
     listModel.addElement(items[i]);

   // Create the JList
//   list= new JList(listModel);
   list= new JList<>(listModel);
   list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
   list.setVisibleRowCount(5);

   // Set the selection index, then the listener
   oldSelected= (-1);
   list.setSelectedIndex(0);
   for(int i= 0; i < items.length; i++)
   {
     if( items[i].itemID.equals(selected) )
     {
       oldSelected= i;
       list.setSelectedIndex(i);
       break;
     }
   }
   list.addListSelectionListener(this);

   // Put the panel name in a JTextField, add it to the panel
   DataField field= new DataField(1);
   field.setText(name);
   add(field, BorderLayout.NORTH);

   // Put the list in a new JScrollPane, add it to the panel
   JScrollPane listScrollPane= new JScrollPane(list);
   add(listScrollPane, BorderLayout.CENTER);
}

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.ListPanel.valueChanged
//
// Purpose-
//       Implement ListSelectionListener.
//
//----------------------------------------------------------------------------
public void
   valueChanged(                    // Handle value change
     ListSelectionEvent  e)         // For this event
{
   if( isCoursePanel )
   {
     int x= list.getSelectedIndex();
     teeboxPanel= teeboxArray[x];
     teeboxPanel.oldSelected= (-1);
     teeboxPanel.list.setSelectedIndex(0);

     Format format= new Format();
     format.execute();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.ListPanel.output
//
// Purpose-
//       Write the data.
//
//----------------------------------------------------------------------------
public synchronized void
   output(                          // Output to database
     DbClient          client,      // The database client
     String            type,        // The database item type
     String            item)        // The database item key
{
   // If changed, output the new data value
   int newSelected= list.getSelectedIndex();

   if( oldSelected != newSelected )
   {
     ListItem listItem= (ListItem)list.getSelectedValue();
     try {
       client.put(type, item, listItem.itemID);
     } catch(IOException e) {
       System.out.println("client.remove: " + e);
     } catch(Exception e) {
       e.printStackTrace();
       System.out.println("client.remove: " + e);
     }
   }

   // Update the selection index
   oldSelected= newSelected;
}
} // class DefaultsEdit.ListPanel

//----------------------------------------------------------------------------
//
// Class-
//       DefaultsEdit.Loader
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
//       DefaultsEdit.Loader.loadAll
//
// Purpose-
//       Load all associated items.
//
//----------------------------------------------------------------------------
public ListItem[]                   // The associated ListItem array
   loadAll(                         // Load all associated items
     String            major)       // Of this major type
{
   Vector<ListItem>    vector= new Vector<ListItem>(); // Generate empty vector

   ListItem listItem= new ListItem();
   listItem.itemID= "";
   listItem.itemShow= "None";
   vector.add(listItem);

   String NEXT= "";
   String FIND= major + ".FIND";
   String SHOW= major + ".SHOW";
   for(;;)
   {
     String string= dbNext(FIND, NEXT);
     if( string == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(string);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(FIND) )
       break;

     NEXT= item;

     listItem= new ListItem();
     listItem.itemID= item;
     listItem.itemShow= stripQuotes(dbGet(SHOW, item));
     if( listItem.itemShow != null )
       vector.add(listItem);
   }

   // Move the Vector to an array
   ListItem[] result= new ListItem[vector.size()];
   for(int i= 0; i<result.length; i++)
     result[i]= vector.elementAt(i);

   // Sort the array
   for(int i= 1; i<result.length; i++)
   {
     for(int j= i+1; j<result.length; j++)
     {
       if( result[i].itemShow.compareToIgnoreCase(result[j].itemShow) > 0 )
       {
         listItem= result[i];
         result[i]= result[j];
         result[j]= listItem;
       }
     }
   }

   return result;
}
//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.Loader.loadTbox
//
// Purpose-
//       Load all associated teebox items.
//
//----------------------------------------------------------------------------
public ListItem[]                   // The associated ListItem array
   loadTbox(                        // Load all associated teebox items
     String            course)      // For this courseID
{
   Vector<ListItem>    vector= new Vector<ListItem>(); // Generate empty vector

   ListItem listItem= new ListItem();
   listItem.itemID= "";
   listItem.itemShow= "None";
   vector.add(listItem);

   String NEXT= course;
   String FIND= "COURSE.TBOX";
   for(;;)
   {
     String string= dbNext(FIND, NEXT);
     if( string == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(string);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(FIND) )
       break;

     NEXT= item;

     int index1= catcon(item, 1);
     if( index1 < 0 )
       break;

     if( !item.substring(0, index1).equalsIgnoreCase(course) )
       break;

     String teebox= item.substring(index1+1);
     listItem= new ListItem();
     listItem.itemID= teebox;
     listItem.itemShow= teebox;
     vector.add(listItem);
   }

   // Move the Vector to an array
   ListItem[] result= new ListItem[vector.size()];
   for(int i= 0; i<result.length; i++)
     result[i]= vector.elementAt(i);

   // Sort the array
   for(int i= 1; i<result.length; i++)
   {
     for(int j= i+1; j<result.length; j++)
     {
       if( result[i].itemShow.compareToIgnoreCase(result[j].itemShow) > 0 )
       {
         listItem= result[i];
         result[i]= result[j];
         result[j]= listItem;
       }
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DefaultsEdit.Loader.doInBackground
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
     dateID= timeID= null;
     String[] array= dbRetrieve(CMD_DEFAULT, DEFAULT_CODE_DT);
     if( array != null )
     {
       dateID= showDate(array[0]);
       if( array.length > 1 )
         timeID= array[1];
     }

     courseID= dbGet(CMD_DEFAULT, DEFAULT_CODE_CI);
     if( courseID == null )
       courseID= "";
     teeboxID= dbGet(CMD_DEFAULT, DEFAULT_CODE_TI);
     if( teeboxID == null )
       teeboxID= "";
     eventsID= dbGet(CMD_DEFAULT, DEFAULT_CODE_EI);
     if( eventsID == null )
       eventsID= "";

     playerID= dbGet(CMD_DEFAULT, DEFAULT_CODE_PI);
     if( playerID == null )
       playerID= "";
     else
     {
       playerNN= dbGet(CMD_PLAYER_NICK, playerID);
       if( playerNN == null )
         throw new Exception("Cannot find PLAYER_NICK " + playerID);

       playerID= dbGet(CMD_PLAYER_FIND, playerNN);
       if( playerID == null )
         throw new Exception("Cannot find PLAYER_FIND " + playerNN);
     }

     array= new String[2];
     if( dateID != null )
       array[0]= dateID;
     if( timeID != null )
       array[1]= timeID;
     defaultDate= new GenericListInfo("Date/Time", array);

     // Generate the base panels
     tail= new DataField();
     tail= defaultDate.genPanel(postListener, tail);
     head= defaultDate.getPanel().getField(1);

     JPanel panel= defaultDate.getPanel();
     panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

     tail.addFocusListener(new NextFocusAdapter(head));

     ListItem[] listItem;
     listItem= loadAll("EVENTS");
     eventsPanel= new ListPanel("Events", listItem, eventsID);

     listItem= loadAll("PLAYER");
     playerPanel= new ListPanel("Player", listItem, playerID);

     listItem= loadAll("COURSE");
     coursePanel= new ListPanel("Course", listItem, courseID);
     coursePanel.isCoursePanel= true;

     teeboxArray= new ListPanel[listItem.length];
     for(int i= 0; i<listItem.length; i++)
     {
       String course= listItem[i].itemID;
       teeboxArray[i]= new ListPanel("Teebox", loadTbox(course), teeboxID);
     }
     teeboxPanel= teeboxArray[coursePanel.list.getSelectedIndex()];

     // Avoid removing empty data elements
     defaultDate.isPresent= true;
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
//       DefaultsEdit.Loader.done
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
} // class DefaultsEdit.Loader

//----------------------------------------------------------------------------
//
// Class-
//       DefaultsEdit.Output
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
//       DefaultsEdit.Output.doInBackground
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
//       DefaultsEdit.Output.done
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
} // class DefaultsEdit.Output
} // class DefaultsEdit
