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
//       CourseView.java
//
// Purpose-
//       View course information.
//
// Last change date-
//       2023/01/19
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
//       CourseView
//
// Purpose-
//       View course information.
//
//----------------------------------------------------------------------------
public class CourseView extends CommonJFrame implements ActionListener {
//----------------------------------------------------------------------------
// CourseView.Attributes
//----------------------------------------------------------------------------
// Internal data areas
CourseView             owner;       // The CourseView object
FocusListener          postListener;// The POST Listener
PostPanel              postPanel;   // The POST button Panel
DataField              head= null;  // Head entry element
DataField              tail= null;  // Tail entry element
Validator              validator;   // Validator object

// Database items
String                 courseID;    // Course ID
String                 courseNN;    // Course NickName
GenericFindInfo        courseFind;  // Course FIND indicator
GenericItemInfo        courseShow;  // Course display name
GenericNameInfo        courseName;  // Course name, location
GenericItemInfo        courseHttp;  // Course HTTP name

CourseHdcpInfo         courseHcpM;  // Handicap information (Men's);
CourseHdcpInfo         courseHcpW;  // Handicap information (Women's);
CourseHoleInfo         courseHole;  // Course hole information
CourseLdCpInfo         courseLdCp;  // Course LONG/NEAR information
CourseLongInfo         courseLong;  // Course LONG information
CourseNearInfo         courseNear;  // Course NEAR information
CourseParsInfo         courseParM;  // Par information (Men's);
CourseParsInfo         courseParW;  // Par information (Women's);

Vector<TeeboxInfo>     teeboxVector;// The teebox information
TeeboxInfo[]           teebox_info; // Sorted teeboxVector

//----------------------------------------------------------------------------
// CourseView.Constants
//----------------------------------------------------------------------------
static final int       HOLE_MRATE= HolePanel.FORMAT_USER;
static final int       HOLE_SLOPE= HOLE_MRATE+1;
static final int       HOLE_WRATE= HOLE_SLOPE+1;
static final int       HOLE_WOMEN= HOLE_WRATE+1;
static final int       HOLE_COUNT= HOLE_WOMEN+1;

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.CourseView
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   CourseView( )                    // Constructor
{
   super("CourseView");             // Set applet name
   owner= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.actionPerformed
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
   throw new RuntimeException("ShouldNotOccur"); // Valid in CourseEdit
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.createGUI
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
// postListener= postPanel.getListener();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: CourseView v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display golf course info.";
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.getParameterInfo
//
// Purpose-
//       Returns information about this Applet's parameters.
//
//----------------------------------------------------------------------------
public String[][]
   getParameterInfo()
{
   String[][] info=
   {   {"course-nick", "string", "The course identifier, "
                               + "default is database default"}
   };
   return info;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.init
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
//       CourseView.isUpdateValid
//
// Purpose-
//       Test whether all data are valid.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff all data valid
   isUpdateValid( )
{
   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.loadAppletParameters
//
// Purpose-
//       Called in background to load the Applet parameters.
//
//----------------------------------------------------------------------------
protected void
   loadAppletParameters()
{
   courseNN= getParameter("course-nick");
   debug("Param: course-nick: " + courseNN);
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.sortTeeboxInfo
//
// Purpose-
//       Sort teeboxVector by rating, result in teebox_info
//
//----------------------------------------------------------------------------
protected void
   sortTeeboxInfo()
{
   // Generate the TeeboxInfo array
   teebox_info= new TeeboxInfo[teeboxVector.size()];
   for(int i= 0; i<teeboxVector.size(); i++)
     teebox_info[i]= teeboxVector.elementAt(i);

   // Sort the TeeboxInfo array by rating
   for(int i= 0; i<teebox_info.length; i++)
   {
     for(int j= i+1; j<teebox_info.length; j++)
     {
       if( teebox_info[i].compareTo(teebox_info[j]) < 0 )
       {
         TeeboxInfo temp= teebox_info[i];
         teebox_info[i]= teebox_info[j];
         teebox_info[j]= temp;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.update
//
// Purpose-
//       Update the internal and, optionally, the external data.
//
//----------------------------------------------------------------------------
public void
   update( )                        // Update the data
{
   throw new RuntimeException("ShouldNotOccur"); // Valid in CourseEdit
}

//----------------------------------------------------------------------------
//
// Class-
//       CourseView.Format
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
//       CourseView.Format.doInBackground
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
     postPanel.setOperational(false); // Not initialized

     if( errorString != null )
       return null;

     // Generate the sorted TeeboxInfo array
     sortTeeboxInfo();

     // Format the data
     try {
       // Generate the base panels
       courseShow.genPanel(postListener, null);
       DataPanel panel= courseShow.getPanel();
       DataField[] df= panel.getField();
       df[0].setBackground(Color.GREEN.brighter());
       if( postListener != null )
         head= tail= df[0];

       tail= courseName.genPanel(postListener, tail);
       tail= courseHttp.genPanel(postListener, tail);

       tail= courseHole.genPanel(postListener, tail);
       panel= courseHole.getPanel();
       df= panel.getField();
       df[HOLE_MRATE].setText("RateM");
       df[HOLE_SLOPE].setText("SLOPE");
       df[HOLE_WRATE].setText("RateW");
       df[HOLE_WOMEN].setText("SLOPE");

       tail= courseHcpM.genPanel(postListener, tail);
       tail= courseParM.genPanel(postListener, tail);
       if( postListener != null )   // Add insert button to courseParM.panel
       {
         JPopupMenu popup= new JPopupMenu();
         popup.add(new JRadioButtonMenuItem("Insert", false)
         {
           public void
              fireItemStateChanged(       // Fire ItemListener event
                ItemEvent       e)
           {
              teeboxVector.add(new TeeboxInfo());
              new Update().execute();
           }
         });

         courseParM.getPanel().getField(0).
             addMouseListener(new PopupListener(popup));
       }

       // Generate the info panels
       for(int i= 0; i<teebox_info.length; i++)
         tail= teebox_info[i].genPanel(postListener, tail);

       tail= courseParW.genPanel(postListener, tail);
       tail= courseHcpW.genPanel(postListener, tail);
       tail= courseLdCp.genPanel(postListener, tail);

       if( tail != null && head != null )
         tail.addFocusListener(new NextFocusAdapter(head));

       // Activate the POST panel
       postPanel.setOperational(true);
     } catch( Exception e ) {
       setERROR("Format: Client exception: " + e);
       e.printStackTrace();
     }
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.Format.done
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
       content.add(courseName.getPanel());
       content.add(courseHttp.getPanel());
       content.add(courseHole.getPanel());

       content.add(courseHcpM.getPanel());
       content.add(courseParM.getPanel());

       for(int i= 0; i<teebox_info.length; i++)
       {
         if( !teebox_info[i].isRemoved )
           content.add(teebox_info[i].panel);
       }

       content.add(courseParW.getPanel());
       content.add(courseHcpW.getPanel());
       content.add(courseLdCp.getPanel());
       if( postListener != null )
         content.add(postPanel);

       if( head != null )
         head.requestFocusInWindow();
       content.revalidate();
       content.repaint();
     }
   }}}} // Synchronized(owner)

   setVisible(true);
   waitingDone();
}
} // class CourseView.Format

//----------------------------------------------------------------------------
//
// Class-
//       CourseView.Loader
//
// Purpose-
//       Create a background task to load the data.
//
//----------------------------------------------------------------------------
class Loader extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       CourseView.Loader.doInBackground
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

   synchronized(owner)
   {{{{
     // Load the data
     try {
       if( client.fsm != DbStatic.FSM_READY )
         throw new Exception("DbServer offline");

       // Extract associated data
       String string;

       courseID= dbGet(CMD_COURSE_FIND, courseNN);
       if( courseID == null )
         courseID= dbGet(CMD_DEFAULT, DEFAULT_CODE_CI);
       if( courseID == null )
         throw new Exception("Cannot find COURSE_ID");
       courseID= dbGet(CMD_COURSE_FIND, courseID);
       if( courseID == null )
         throw new Exception("Invalid COURSE_ID: " + courseID);

       courseFind= new GenericFindInfo(courseID);
       courseShow= new GenericItemInfo(dbGet(CMD_COURSE_SHOW, courseID));
       courseName= new GenericNameInfo(dbGet(CMD_COURSE_NAME, courseID));
       courseHttp= new GenericItemInfo(dbGet(CMD_COURSE_HTTP, courseID));

       courseHcpM= new CourseHdcpInfo(HOLE_COUNT, dbGet(CMD_COURSE_HDCP, courseID));
       string= dbGet(CMD_COURSE_HDCP, concat(courseID, "WOMEN"));
       if( string == null )
         courseHcpW= new CourseHdcpInfo(courseHcpM);
       else
         courseHcpW= new CourseHdcpInfo(HOLE_COUNT, string);

       courseHole= new CourseHoleInfo(HOLE_COUNT, dbGet(CMD_COURSE_HOLE, courseID));

       courseLong= new CourseLongInfo(HOLE_COUNT, dbGet(CMD_COURSE_LONG, courseID));
       courseNear= new CourseNearInfo(HOLE_COUNT, dbGet(CMD_COURSE_NEAR, courseID));
       courseLdCp= new CourseLdCpInfo(HOLE_COUNT, courseLong, courseNear);

       courseParM= new CourseParsInfo(HOLE_COUNT, dbGet(CMD_COURSE_PARS, courseID));
       string= dbGet(CMD_COURSE_PARS, concat(courseID, "WOMEN"));
       if( string == null )
         courseParW= new CourseParsInfo(courseParM);
       else
         courseParW= new CourseParsInfo(HOLE_COUNT, string);

       // Collect teebox information
       teeboxVector= new Vector<TeeboxInfo>();
       String CMD=  CMD_COURSE_TBOX;
       String NEXT= courseID;
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
         if( index < 0 )
           continue;

         String itemNick= item.substring(0, index);
         if( !itemNick.equalsIgnoreCase(courseNN)
             && !itemNick.equalsIgnoreCase(courseID) )
         {
           itemNick= dbGet(CMD_COURSE_FIND, itemNick);
           if( !itemNick.equalsIgnoreCase(courseNN)
               && !itemNick.equalsIgnoreCase(courseID) )
             break;
         }

         String[] content= tokenize(t.remainder());
         TeeboxInfo info=
             new TeeboxInfo(HOLE_COUNT, courseID, courseShow.toString(),
                            item.substring(index+1), content);
         teeboxVector.add(info);
       }
     } catch( Exception e ) {
       setERROR("Loader: Client exception: " + e);
       e.printStackTrace();
       return null;
     }
   }}}} // Synchronized(owner)

   dbReset();

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.Loader.done
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
} // class CourseView.Loader

//----------------------------------------------------------------------------
//
// Class-
//       CourseView.Update
//
// Purpose-
//       Update the internal data.
//
//----------------------------------------------------------------------------
class Update extends SwingWorker<Object, Void>
{
//----------------------------------------------------------------------------
//
// Method-
//       CourseView.Update.doInBackground
//
// Purpose-
//       Update the internal data.
//
// Notes-
//       Called by SwingWorker via execute().
//
//----------------------------------------------------------------------------
@Override
public Object
   doInBackground()
{
   debug("Update.doInBackground()");

   synchronized(owner)
   {{{{
     update();
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseView.Update.done
//
// Purpose-
//       Complete the update.
//
// Notes-
//       Called by SwingWorker in EWT after doInBackground completes.
//
//----------------------------------------------------------------------------
@Override
public void done()
{
   debug("Update.done()");
   new Format().execute();
}
} // class CourseView.Update

//----------------------------------------------------------------------------
//
// Subclass-
//       CourseView.TeeboxInfo
//
// Purpose-
//       Describe teebox database information.
//
//----------------------------------------------------------------------------
class TeeboxInfo extends CourseTboxInfo {
   TeeboxInfo( )                    // Default constructor
{
   super(HOLE_COUNT);
}

   TeeboxInfo(                      // Copy constructor
     TeeboxInfo        copy)        // Source
{
   super(copy);
}

   TeeboxInfo(                      // Constructor
     int               format,      // Format
     String            courseID,    // The course name
     String            courseShow,  // The course name
     String            teeboxShow,  // The teebox name (kept in .title)
     String[]          content)     // Content array
{
   super(format, courseID, courseShow, teeboxShow, content);
}

public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   tail= super.genPanel(listener, tail);

   if( panel != null )
   {
     DataField[] df= panel.getField();

     df[HOLE_MRATE].setText(stringMR);
     df[HOLE_SLOPE].setText(stringMS);
     df[HOLE_WRATE].setText(stringWR);
     df[HOLE_WOMEN].setText(stringWS);

     if( listener != null )
     {
       new TeeboxPopup(this, df[HOLE_ID]);

       df[HOLE_MRATE].setEditable(true);
       df[HOLE_SLOPE].setEditable(true);
       df[HOLE_WRATE].setEditable(true);
       df[HOLE_WOMEN].setEditable(true);

       df[HOLE_MRATE].addFocusListener(listener);
       df[HOLE_SLOPE].addFocusListener(listener);
       df[HOLE_WRATE].addFocusListener(listener);
       df[HOLE_WOMEN].addFocusListener(listener);
     }

     if( tail != null )
     {
       tail.addFocusListener(new NextFocusAdapter(df[HOLE_MRATE]));
       tail= df[HOLE_WOMEN];
     }
   }

   return tail;
}

public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( !isRemoved && panel != null)
   {
     DataField[] df= panel.getField();
     if( !super.isValid(v) )
       return false;

     // Verify slope/rating
     if( df[HOLE_MRATE].getText().equals("") )
       df[HOLE_MRATE].setText("-");
     if( df[HOLE_WRATE].getText().equals("") )
       df[HOLE_WRATE].setText("-");
     if( df[HOLE_SLOPE].getText().equals("") )
       df[HOLE_SLOPE].setText("-");
     if( df[HOLE_WOMEN].getText().equals("") )
       df[HOLE_WOMEN].setText("-");

     if( df[HOLE_MRATE].getText().equals("-")
         && df[HOLE_WRATE].getText().equals("-") )
       return v.invalid("teebox(" + title + ") missing rating");

     if( df[HOLE_SLOPE].getText().equals("-")
         && df[HOLE_WOMEN].getText().equals("-") )
       return v.invalid("teebox(" + title + ") missing slopes");

     if( df[HOLE_MRATE].getText().equals("-")
         && !df[HOLE_SLOPE].getText().equals("-") )
       return v.invalid("teebox(" + title + ") slope requires rating");

     if( df[HOLE_WRATE].getText().equals("-")
         && !df[HOLE_WOMEN].getText().equals("-") )
       return v.invalid("teebox(" + title + ") slope requires rating");

     if( df[HOLE_SLOPE].getText().equals("-")
         && !df[HOLE_MRATE].getText().equals("-") )
       return v.invalid("teebox(" + title + ") rating requires slope");

     if( df[HOLE_WOMEN].getText().equals("-")
         && !df[HOLE_WRATE].getText().equals("-") )
       return v.invalid("teebox(" + title + ") rating requires slope");

     if( !v.isValidRating(df[HOLE_MRATE])
         || !v.isValidRating(df[HOLE_WRATE]) )
       return false;

     if(!v.isValidSlope(df[HOLE_SLOPE])
         || !v.isValidSlope(df[HOLE_WOMEN]) )
       return false;
   }

   return true;
}

public synchronized void
   update( )                        // Update from panel
{
   if( panel != null )
   {
     DataField df[]= panel.getField();
     if( !title.equalsIgnoreCase(df[HOLE_ID].getText()) )
     {
       TeeboxInfo copy= new TeeboxInfo(this);
       copy.isPresent= isPresent;
       copy.isRemoved= true;
       this.isChanged= true;
       teeboxVector.add(copy);
       title= df[HOLE_ID].getText();
     }

     if( !stringMR.equals(df[HOLE_MRATE].getText())
         || !stringMS.equals(df[HOLE_SLOPE].getText())
         || !stringWR.equals(df[HOLE_WRATE].getText())
         || !stringWS.equals(df[HOLE_WOMEN].getText()) )
     {
       isChanged= true;
       stringMR= df[HOLE_MRATE].getText();
       stringMS= df[HOLE_SLOPE].getText();
       stringWR= df[HOLE_WRATE].getText();
       stringWS= df[HOLE_WOMEN].getText();
       updateRating();
     }

     super.update();
   }
}
} // class TeeboxInfo

//----------------------------------------------------------------------------
//
// Class-
//       CourseView.TeeboxDelButton
//
// Purpose-
//       The TeeboxInfo DELETE Button.
//
//----------------------------------------------------------------------------
private class TeeboxDelButton extends JRadioButtonMenuItem {
private TeeboxInfo     teeboxInfo;  // The associated TeeboxInfo

   TeeboxDelButton(                 // The TeeboxInfo DELETE Button
     TeeboxInfo        info)        // The associated TeeboxInfo
{
   super("Delete");
   teeboxInfo= info;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( teeboxInfo != null )         // If not called from super(..)
   {
     if( isSelected() )
     {
       teeboxInfo.isRemoved= true;
       new Update().execute();
     }
   }
}
} // class TeeboxDelButton

//----------------------------------------------------------------------------
//
// Class-
//       CourseView.TeeboxInsButton
//
// Purpose-
//       The TeeboxInfo DUPLICATE Button.
//
//----------------------------------------------------------------------------
private class TeeboxInsButton extends JRadioButtonMenuItem {
private TeeboxInfo     teeboxInfo;  // The associated TeeboxInfo

   TeeboxInsButton(                 // The TeeboxInfo INSERT Button
     TeeboxInfo        info)        // The associated TeeboxInfo
{
   super("Duplicate");
   teeboxInfo= info;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( teeboxInfo != null )         // If not called from super(..)
   {
     if( isSelected() )
     {
       teeboxVector.add(new TeeboxInfo(teeboxInfo));
       new Update().execute();
     }
   }
}
} // class TeeboxInsButton

//----------------------------------------------------------------------------
//
// Class-
//       CourseView.TeeboxButton
//
// Purpose-
//       The TeeboxInfo color selector Button.
//
//----------------------------------------------------------------------------
private class TeeboxButton extends JRadioButtonMenuItem {
TeeboxPopup            popup;       // The popup menu
int                    index;       // The Button index

   TeeboxButton(                    // Constructor
     String            string,      // Button string
     boolean           select,      // Selected indicator
     TeeboxPopup       menu,        // The popup menu
     int               item)        // The Button index
{
   super(string, select);

   popup= menu;
   index= item;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( popup != null )              // If not called from super(..)
   {
     if( isSelected() )
     {
       popup.select(index);
       super.fireItemStateChanged(e);
     }
   }
}
} // class TeeboxButton

//----------------------------------------------------------------------------
//
// Class-
//       CourseView.TeeboxPopup
//
// Purpose-
//       The TeeboxInfo Popup. ([] Color selector, Delete, Duplicate)
//
//----------------------------------------------------------------------------
private class TeeboxPopup extends JPopupMenu {
TeeboxInfo             teeboxInfo;  // The associated TeeboxInfo
int                    index;       // The selected byDate_info index

   TeeboxPopup(                     // Constructor
     TeeboxInfo        info,        // The associated TeeboxInfo
     DataField         field)       // The associated DataField
{
   PopupListener teeboxListener= new PopupListener(this);

   teeboxInfo= info;
   index= CourseTboxInfo.colorIX(info.color);

   for(int i= 0; i<CourseTboxInfo.COLOR_NAME.length; i++)
   {
     TeeboxButton button= new TeeboxButton(CourseTboxInfo.COLOR_NAME[i], index == i, this, i);
     add(button);
   }

   add(new TeeboxDelButton(info));
   add(new TeeboxInsButton(info));

   field.addMouseListener(teeboxListener);
}

public void
   select(                          // Update the selected index
     int               index)       // With this index
{
   debug("TeeboxPopup.select(" + index + ") " + this.index);

   if( index != this.index )
   {
     teeboxInfo.isChanged= true;
     if( index == 0 )
       teeboxInfo.color= teeboxInfo.title;
     else
       teeboxInfo.color= CourseTboxInfo.COLOR_NAME[index];
     new Update().execute();
   }
}
} // class TeeboxPopup
} // class CourseView
