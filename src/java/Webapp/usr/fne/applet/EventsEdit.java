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
//       EventsEdit.java
//
// Purpose-
//       Edit event information.
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
//       EventsEdit
//
// Purpose-
//       Edit event information.
//
//----------------------------------------------------------------------------
public class EventsEdit extends EventsTeam implements ActionListener {
//----------------------------------------------------------------------------
// EventsEdit.Attributes
//----------------------------------------------------------------------------
DataPanel              panelDate;   // The date descriptor Panel
DataPanel              panelHdcp;   // The HDCP descriptor Panel
FocusListener          postListener;// The POST Listener
PostPanel              postPanel;   // The POST button Panel

DataField              head= null; // Head entry element
DataField              tail= null; // Tail entry element

// Database items
GenericItemInfo        eventsShow;  // Events display name
GenericNameInfo        eventsName;  // Event name

Vector<ByDateInfo>     byDateVector;// The ByDate information
ByDateInfo[]           byDate_info; // Sorted byDateVector

ByTimeInfo             byTimeInfo;  // The ByTime information

Vector<CourseInfo>     courseVector;// The Course information
CourseInfo[]           course_info; // Sorted courseVector

Vector<PlayerInfo>     playerVector;// The Player information
PlayerInfo[]           player_info; // Sorted playerVector

// Internal data areas
EventsEdit             owner;       // The EventsEdit object
TeamButton             teamButton;  // The TEAM Button
Validator              validator;   // The Validator

//----------------------------------------------------------------------------
// EventsEdit.Constants
//----------------------------------------------------------------------------
static final Color     LIGHT_BLUE= new Color(223, 223, 255);

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.EventsEdit
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   EventsEdit( )                    // Constructor
{
   super();
   appletName= "EventsEdit";        // Set applet name
   owner= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
protected int                       // The course index
   courseIndex(                     // Get course index
     String            show)        // For this course name
{
   int                 index= 0;

   for(int i= 0; i<course_info.length; i++)
   {
     if( show.equalsIgnoreCase(course_info[i].courseShow) )
     {
       index= i;
       break;
     }
   }

   return index;
}

protected void
   debugByDateInfo( )               // Debug the byDate_info array
{
   debug("debugByDateInfo()");
   for(int i= 0; i<byDate_info.length; i++)
   {
     debug("\n");
     debug("byDate_info[" + i + "]");
     byDate_info[i].debug();
   }
}

protected void
   debugByDateVector( )             // Debug the byDateVector
{
   debug("debugByDateVector()");
   for(int i= 0; i<byDateVector.size(); i++)
   {
     debug("\n");
     debug("byDateVector[" + i + "]");
     byDateVector.elementAt(i).debug();
   }
}

protected int                       // The player index
   playerIdIndex(                   // Get player index by ID
     String            playerID)    // For this playerID
{
   int                 index= 0;

   for(int i= 0; i<player_info.length; i++)
   {
     if( playerID.equalsIgnoreCase(player_info[i].playerID) )
     {
       index= i;
       break;
     }
   }

   return index;
}

protected int                       // The teebox index
   teeboxIndex(                     // Get teebox index
     CourseInfo        info,        // For this course
     String            show)        // And this teebox name
{
   int                 index= 0;

   for(int i= 0; i<info.teebox_info.length; i++)
   {
     if( show.equalsIgnoreCase(info.teebox_info[i].teeboxID) )
     {
       index= i;
       break;
     }
   }

   return index;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.actionPerformed
//
// Purpose-
//       Handle POST event.
//
// Notes-
//       Called from validator.actionPerformed()
//       This method implements the ActionListener interface.
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
//       EventsEdit.createGUI
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
   }; // new Validator()

   // Create the TeamButton
   teamButton= new TeamButton();

   // Create the PostPanel
   postPanel= new PostPanel(validator);
   postListener= postPanel.getListener();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.getAppletInfo
//
// Purpose-
//       Returns information about this Applet.
//
//----------------------------------------------------------------------------
public String getAppletInfo()
{
   return "Title: EventsEdit v1.0, 01 Jan 2008\n"
          + "Author: Frank Eskesen\n"
          + "Display golf course info.";
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.init
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
//       EventsEdit.isUpdateValid
//
// Purpose-
//       Test whether all data are valid.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff all data valid
   isUpdateValid( )
{
   JTextField          field;       // Working JTextField
   DataPanel           panel;       // Working HolePanel
   int[]               i18= new int[18]; // Hole validation array

   debug("isUpdateValid()");
   postPanel.setMessage();
   // Marker

   // Make sure the events show is present and valid
   if( !eventsShow.isValid(validator) )
     return false;

   // Make sure the events name is present and valid
   if( !eventsName.isValid(validator) )
     return false;

   try {                            // For debugging
     // Verify the Player information
     for(int i= 0; i<player_info.length; i++)
       if( !player_info[i].isValid(validator) )
         return false;

     // Verify the ByDate information
     for(int i= 0; i<byDateVector.size(); i++)
     {
       ByDateInfo date= byDateVector.elementAt(i);
       if( date.isRemoved )
         continue;

       if( !date.isValid(validator) )
         return false;

       // Check for duplicate dates
       for(int j= i+1; j<byDateVector.size(); j++)
       {
         ByDateInfo info= byDateVector.elementAt(j);
         if( !info.isRemoved && date.date.equals(info.date) )
           return validator.invalid("DATE(" + showDate(date.date) + ") duplicated");
       }
     }
   } catch(Exception e) {
     e.printStackTrace();
     return validator.invalid("Events: " + e);
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.loadStaticInformation
//
// Purpose-
//       Called in background to load course and player information.
//
//----------------------------------------------------------------------------
protected void
   loadStaticInformation( )
{
   // Collect Course information
   courseVector= new Vector<CourseInfo>();
   String CMD=  CMD_COURSE_FIND;
   String NEXT= "";
   for(;;)
   {
     String string= dbNext(CMD, NEXT);
     if( string == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(string);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(CMD) )
       break;

     NEXT= item;

     CourseInfo info= new CourseInfo();
     info.courseID= t.remainder();
     info.courseShow= stripQuotes(dbGet(CMD_COURSE_SHOW, info.courseID));
     if( info.courseShow == null )
       info.courseShow= info.courseID;

     courseVector.add(info);
   }

   // Sort the CourseInfo vector by name
   course_info= new CourseInfo[courseVector.size()];
   for(int i= 0; i<course_info.length; i++)
     course_info[i]= courseVector.elementAt(i);

   for(int i= 0; i<course_info.length; i++)
   {
     for(int j= i+1; j<course_info.length; j++)
     {
       if( course_info[i].courseShow.compareTo(course_info[j].courseShow) > 0 )
       {
         CourseInfo temp= course_info[i];
         course_info[i]= course_info[j];
         course_info[j]= temp;
       }
     }
   }

   // Collect Teebox information
   for(int i= 0; i<course_info.length; i++)
   {
     Vector<TeeboxInfo> teeboxVector= new Vector<TeeboxInfo>();
     CourseInfo info= course_info[i];

     CMD=  CMD_COURSE_TBOX;
     NEXT= info.courseID;
     for(;;)
     {
       String string= dbNext(CMD, NEXT);
       if( string == null )
         break;

       QuotedTokenizer t= new QuotedTokenizer(string);
       String type= t.nextToken();
       String item= t.nextToken();

       if( !type.equals(CMD) )
         break;

       NEXT= item;
       int index= catcon(item);
       if( index < 0 )
         continue;

       if( item.substring(0, index).compareTo(info.courseID) != 0)
         break;

       TeeboxInfo teeboxInfo= new TeeboxInfo();
       teeboxInfo.teeboxID= item.substring(index+1);

       try {
         teeboxInfo.mr= Double.parseDouble(t.nextToken());
         teeboxInfo.ms= Integer.parseInt(t.nextToken());
         t.nextToken();
       } catch(Exception e) {
       }

       try {
         teeboxInfo.wr= Double.parseDouble(t.nextToken());
         teeboxInfo.ws= Integer.parseInt(t.nextToken());
       } catch(Exception e) {
       }

       teeboxVector.add(teeboxInfo);
     }

     info.teebox_info= new TeeboxInfo[teeboxVector.size()];
     for(int j= 0; j<info.teebox_info.length; j++)
       info.teebox_info[j]= teeboxVector.elementAt(j);

     for(int j= 0; j<info.teebox_info.length; j++)
     {
       for(int k= j+1; k<info.teebox_info.length; k++)
       {
         if( (info.teebox_info[j].mr < info.teebox_info[k].mr)
            || (info.teebox_info[j].mr == info.teebox_info[k].mr
                && info.teebox_info[j].wr < info.teebox_info[k].wr) )
         {
           TeeboxInfo temp= info.teebox_info[j];
           info.teebox_info[j]= info.teebox_info[k];
           info.teebox_info[k]= temp;
         }
       }
     }
   }

   // Collect Player information
   playerVector= new Vector<PlayerInfo>();
   CMD=  CMD_PLAYER_FIND;
   NEXT= "";
   for(;;)
   {
     String string= dbNext(CMD, NEXT);
     if( string == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(string);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(CMD) )
       break;

     NEXT= item;

     String playerID= t.remainder();
     String playerNN= dbGet(CMD_PLAYER_NICK, playerID);
     if( playerNN == null         // Skip iff playerID is nickname
         || playerNN.equalsIgnoreCase(item) )
       continue;

     PlayerInfo info= new PlayerInfo();
     info.playerID= playerID;
     info.playerNN= playerNN;
     info.playerShow= stripQuotes(dbGet(CMD_PLAYER_SHOW, playerID));
     if( info.playerShow == null )
       info.playerShow= playerID;

     info.playerHdcp= dbGet(CMD_EVENTS_HDCP,
                            concat(eventsID, playerNN));
     if( info.playerHdcp == null )
       info.playerHdcp= dbGet(CMD_EVENTS_HDCP,
                              concat(eventsID, playerID));
     if( info.playerHdcp == null )
     {
       info.isRemoved= true;
       PlayerPostInfo[] post_info= loadPlayerPostInfo(info.playerNN, null);
       info.playerHdcp= "" + loadPlayerHdcp(post_info);
     }
     else
       info.isPresent= true;

     playerVector.add(info);
   }

   // Sort the PlayerInfo vector by name
   player_info= new PlayerInfo[playerVector.size()];
   for(int i= 0; i<player_info.length; i++)
     player_info[i]= playerVector.elementAt(i);

   for(int i= 0; i<player_info.length; i++)
   {
     for(int j= i+1; j<player_info.length; j++)
     {
       if( player_info[i].playerShow.compareTo(player_info[j].playerShow) > 0 )
       {
         PlayerInfo temp= player_info[i];
         player_info[i]= player_info[j];
         player_info[j]= temp;
       }
     }
   }

   // Create the byDateVector
   byDateVector= new Vector<ByDateInfo>();
   byDateVector.add(new ByDateInfo(true));
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.outputAdd
//
// Purpose-
//       Generate EVENTS_FIND database entry.
//
//----------------------------------------------------------------------------
public void
   outputAdd( )                     // Different in EventsAdd
{
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.outputDel
//
// Purpose-
//       Delete EVENTS_FIND database entry.
//
//----------------------------------------------------------------------------
public void
   outputDel( )                     // Different in EventsAdd
{
   dbReady();

   dbRemove(CMD_EVENTS_FIND, eventsID);
   dbRemove(CMD_EVENTS_SHOW, eventsID);
   dbRemove(CMD_EVENTS_NAME, eventsID);

   eventsShow.isPresent= false;
   eventsName.isPresent= false;

   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.output
//
// Purpose-
//       Output the data.
//
//----------------------------------------------------------------------------
public void
   output( )                        // Output the data
{
   StringBuffer        buffer;      // Working StringBuffer

   debug("output()");

   // Check for players
   boolean isEmpty= true;
   for(int i= 0; i<player_info.length; i++)
   {
     PlayerInfo info= player_info[i];
     if( !info.isRemoved )
     {
       isEmpty= false;
       break;
     }
   }
   if( isEmpty )                    // No players, delete event
   {
     byTimeInfo.isRemoved= true;
     byTimeInfo.output();

     outputDel();
     return;
   }

   // Output the data, updating the database
   dbReady();

   // Events find
   outputAdd();

   // Events SHOW
   if( eventsShow.isChanged || !eventsShow.isPresent )
   {
     dbPut(CMD_EVENTS_SHOW, eventsID, eventsShow.toString());
     eventsShow.isChanged= false;
     eventsShow.isPresent= true;
   }

   // Events NAME
   if( eventsName.isChanged || !eventsName.isPresent )
   {
     dbPut(CMD_EVENTS_NAME, eventsID, eventsName.toString());
     eventsName.isChanged= false;
     eventsName.isPresent= true;
   }

   // Handle added and removed Players
   for(int i= 0; i<player_info.length; i++)
   {
     PlayerInfo info= player_info[i];
     if( info.isRemoved )
     {
       if( info.isPresent )
       {
         dbRemove(CMD_EVENTS_HDCP, concat(eventsID,info.playerNN));
         info.isPresent= false;
       }
     }
     else
     {
       if( info.isChanged )
       {
         dbPut(CMD_EVENTS_HDCP, concat(eventsID,info.playerNN), info.playerHdcp);
         info.isChanged= false;
         info.isPresent= true;
       }
     }
   }

   // Handle ByDateInfo modifications
   for(int i= 0; i<byDateVector.size(); i++)
   {
     ByDateInfo info= byDateVector.elementAt(i);
     if( info.isRemoved )
     {
       if( info.isPresent )
       {
         boolean remove= true;
         for(int j= 0; j<byDateVector.size(); j++)
         {
           if( i != j
               && !byDateVector.elementAt(j).isRemoved
               && info.date.equals(byDateVector.elementAt(j).date) )
           {
             remove= false;
             break;
           }
         }

         if( remove )
         {
           dbRemove(CMD_EVENTS_DATE, concat(eventsID, info.date));
           dbRemove(CMD_EVENTS_PLAY, concat(eventsID, info.date));

           byTimeInfo.isRemoved= true;
         }

         info.isPresent= false;
       }
     }
     else                         // if !info.isRemoved
     {
       if( info.isChanged || !info.isPresent )
       {
         CourseInfo courseInfo= course_info[info.courseIX];

         buffer= new StringBuffer();
         buffer.append(courseInfo.courseID);
         buffer.append(" ");
         buffer.append(courseInfo.teebox_info[info.teeboxIX].teeboxID);
         for(int j= 0; j<info.time.length; j++)
         {
           buffer.append(" ");
           buffer.append(info.time[j]);
         }
         dbPut(CMD_EVENTS_DATE, concat(eventsID, info.date), buffer.toString());

         buffer= new StringBuffer();
         for(int j= 0; j<info.player_IX.length; j++)
         {
           if( j != 0 )
             buffer.append(" ");
           int x= info.player_IX[j];
           if( !player_info[x].isRemoved )
             buffer.append(player_info[x].playerNN);
         }

         dbPut(CMD_EVENTS_PLAY, concat(eventsID, info.date), buffer.toString());
         info.isChanged= false;
         info.isPresent= true;

         byTimeInfo.isRemoved= true;
       }
     }
   }

   // Handle ByTimeInfo modifications
   byTimeInfo.output();

   dbReset();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.update
//
// Purpose-
//       Update the data.
//
//----------------------------------------------------------------------------
public void
   update( )                        // Update the data
{
   StringBuffer        buffer;      // Working StringBuffer

   debug("update()");

   // Events show
   eventsShow.update();

   // Events name
   eventsName.update();

   // Update PlayerInfo
   for(int i= 0; i<player_info.length; i++)
     player_info[i].update();

   // Update ByDateInfo
   for(int i= 0; i<byDate_info.length; i++)
     byDate_info[i].update();
}

//----------------------------------------------------------------------------
// EventsEdit.TeeboxInfo
//----------------------------------------------------------------------------
class TeeboxInfo {
   String              teeboxID;    // Teebox ID
   double              mr;          // Men's rating
   int                 ms;          // Men's slope
   double              wr;          // Women's rating
   int                 ws;          // Women's slope
}; // class TeeboxInfo

//----------------------------------------------------------------------------
// EventsEdit.CourseInfo
//----------------------------------------------------------------------------
class CourseInfo {
   String              courseID;    // Course ID
   String              courseShow;  // Course Name

   TeeboxInfo[]        teebox_info; // TeeboxInfo array

public void
   debug()
{
   System.out.println("CourseInfo.courseID: " + courseID);
   System.out.println("CourseInfo.courseShow: " + courseShow);
   for(int i= 0; i<teebox_info.length; i++)
   {
     TeeboxInfo info= teebox_info[i];
     System.out.println("CourseInfo.teebox_info[" + i + "] " + info.teeboxID
                      + " " + info.mr + "/" + info.ms
                      + " " + info.wr + "/" + info.ws);
   }
}
}; // class CourseInfo

//----------------------------------------------------------------------------
//
// Subclass-
//       EventsEdit.ByDateInfo
//
// Purpose-
//       Describe event information by date.
//
//----------------------------------------------------------------------------
public DataPanel                    // Resultant DataPanel
   ByDateInfo_topPanel( )           // Generate heading panel
{
   // Generate the date selection popup and  panel
   DataPanel panelDate= new DataPanel(0);
   DataPanel inner= new DataPanel(3);
   DataField[] df= inner.getField();
   DataField field= df[0];
   field.setText("Date");
   field.setColumns(30);
   new EventsPopup(field);
   field= df[1];
   field.setText("Course");
   field.setColumns(50);
   field= df[2];
   field.setText("Tee");
   field.setColumns(20);
   inner.setColor(LIGHT_BLUE, Color.BLACK);
   panelDate.add(inner);

   inner= new DataPanel(1);
   df= inner.getField();
   field= df[0];
   field.setText("Times");
   field.setColumns(100);
   inner.setColor(LIGHT_BLUE, Color.BLACK);
   panelDate.add(inner);

   return panelDate;
}

class ByDateInfo extends DatabaseInfo {
   String              date;        // Round date
   String[]            time;        // Round times

   int                 courseIX;    // Course index
   int                 teeboxIX;    // Teebox index
   int[]               player_IX;   // Player index

   DataPanel           panel;       // DataPanel for this date
   EventsButton        button;      // Date selector button

   ByDateInfo()                     // Default constructor (Not initialized)
{
}

   ByDateInfo(boolean b)            // Initialized constructor
{
   isRemoved= true;

   date= "yyyy/mm/dd";
   time= new String[2];
   time[0]= "09:00";
   time[1]= "09:10";
   courseIX= 0;
   teeboxIX= 0;

   addAllPlayers();
}

   ByDateInfo(                      // Copy constructor
     ByDateInfo        source)      // Source ByDateInfo
{
   isChanged= true;
   date= source.date;

   time= new String[source.time.length];
   for(int i= 0; i<time.length; i++)
     time[i]= source.time[i];

   courseIX= source.courseIX;
   teeboxIX= source.teeboxIX;

   player_IX= new int[source.player_IX.length];
   for(int i= 0; i<player_IX.length; i++)
     player_IX[i]= source.player_IX[i];

   panel= null;                     // The panel is NOT copied
}

public void
   addAllPlayers( )                 // Add all players to date
{
   int active= 0;
   for(int i= 0; i<player_info.length; i++)
   {
     if( !player_info[i].isRemoved )
       active++;
   }

   player_IX= new int[active];
   active= 0;
   for(int i= 0; i<player_info.length; i++)
   {
     if( !player_info[i].isRemoved )
       player_IX[active++]= i;
   }
}

public void
   debug( )
{
   super.debug("ByDateInfo");
   System.out.println("ByDateInfo.date: " + date);
   for(int i= 0; i<time.length; i++)
     System.out.println("ByDateInfo.time[" + i + "] " + time[i]);
   System.out.println("ByDateInfo.courseIX: " + courseIX);
   System.out.println("ByDateInfo.teeboxIX: " + teeboxIX);
   for(int i= 0; i<player_IX.length; i++)
     System.out.println("ByDateInfo.player_IX[" + i + "] " + player_IX[i]);
}

public DataField                    // Resultant tail
   genPanel(                        // Generate new panel
     FocusListener     listener,    // FocusListener
     DataField         tail)        // The current tail DataField
{
   panel= null;
   if( !isRemoved )
   {
     CourseInfo courseInfo= course_info[courseIX];

     panel= new DataPanel(0);
     DataPanel inner= new DataPanel(3);
     DataField[] df= inner.getField();
     DataField field= df[0];
     field.setText(showDate(date));
     field.setColumns(30);
     new UpdateListener(field);
     field= df[1];
     field.setText(courseInfo.courseShow);
     field.setColumns(50);
     MenuScroller.setScrollerFor(new CoursePopup(this, field));
     field= df[2];
     field.setColumns(20);
     field.setText(courseInfo.teebox_info[teeboxIX].teeboxID);
     new TeeboxPopup(this, field);
     panel.add(inner);

     inner= new DataPanel(time.length);
     df= inner.getField();
     for(int j= 0; j<time.length; j++)
     {
       field= df[j];
       field.setText(time[j]);
       field.setColumns(100/time.length);
       new TimePopup(field, this, j);
     }
     panel.add(inner);

     if( listener != null )
     {
       Component[] component= panel.getComponents();
       inner= (DataPanel)component[0];
       df= inner.getField();
       for(int j= 0; j<df.length; j++)
       {
         df[j].setEditable(true);
         df[j].addFocusListener(listener);
       }

       inner= (DataPanel)component[1];
       df= inner.getField();
       for(int j= 0; j<df.length; j++)
       {
         df[j].setEditable(true);
         df[j].addFocusListener(listener);
       }
     }

     if( tail != null )
     {
       tail.addFocusListener(new NextFocusAdapter(df[0]));
       tail= df[df.length-1];
     }
   }

   return tail;
}

public DataField                    // The associated date field
   getDateField( )                  // Get date field
{
   DataField result= null;

   if( panel != null )
   {
     Component[] component= panel.getComponents();
     DataPanel inner= (DataPanel)component[0];
     result= inner.getField(0);
   }

   return result;
}

public boolean                      // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( !isRemoved )
   {
     DataPanel panel= this.panel;
     Component[] component= panel.getComponents();
     panel= (DataPanel)component[0];
     if( !v.isValidDate(panel.getField(0)) )
       return false;

     panel= (DataPanel)component[1];
     DataField[] df= panel.getField();
     for(int j= 0; j<df.length; j++)
     {
       if( !v.isValidTime(df[j]) )
         return false;
     }

     // Check for duplicated tee times
     for(int i= 0; i<time.length; i++)
     {
       for(int j= i+1; j<time.length; j++)
         if( df[i].getText().equals(df[j].getText()) )
           return validator.invalid("DATE(" + showDate(date)
                                + ") TIME(" + df[i].getText() + ") duplicated");
     }

     // Do we have more tee times than players?
     if( time.length > player_IX.length )
       return validator.invalid("DATE(" + showDate(date) + ") more tee times than players");

     // Do we have enough tee times for players?
     if( (4*time.length) < player_IX.length )
       return validator.invalid("DATE(" + showDate(date) + ") not enough tee times for players");
   }

   return true;
}

protected int                       // The index in the player list
   playerIndex(                     // Get player index in player_IX
     int               playerIX)    // The player index
{
   int index= (-1);
   for(int i= 0; i<player_IX.length; i++)
   {
     if( player_IX[i] == playerIX )
     {
       index= i;
       break;
     }
   }

   return index;
}


public void
   playerInsert(                    // Insert player onto list
     int               playerIX)    // The player index
{
   int index= playerIndex(playerIX);
   if( index < 0 )
   {
     int[] player_IX= new int[this.player_IX.length+1];
     for(index= 0; index<this.player_IX.length; index++)
     {
       player_IX[index]= this.player_IX[index];
       if( player_IX[index] > playerIX )
         break;
     }
     player_IX[index]= playerIX;
     for(int i= index+1; i<player_IX.length; i++)
       player_IX[i]= this.player_IX[i-1];

     this.player_IX= player_IX;
     isChanged= true;

     byTimeInfo.isRemoved= true;
   }
}

public void
   playerRemove(                    // Remove player from list
     int               playerIX)    // The player index
{
   int index= playerIndex(playerIX);
   if( index >= 0 )
   {
     int[] player_IX= new int[this.player_IX.length-1];
     for(int i= 0; i<index; i++)
       player_IX[i]= this.player_IX[i];
     for(int i= index+1; i<this.player_IX.length; i++)
       player_IX[i-1]= this.player_IX[i];

     this.player_IX= player_IX;
     isChanged= true;

     byTimeInfo.isRemoved= true;
   }
}

public void
   update( )                        // Update from panel
{
   if( isRemoved || panel == null )
     return;

   DataPanel panel= this.panel;
   Component[] component= panel.getComponents();

   // Update event times
   panel= (DataPanel)component[1];
   DataField[] df= panel.getField();
   if( df.length == time.length )   // Insert/delete override text change
   {
     for(int j= 0; j<df.length; j++)
     {
       if( !df[j].getText().equals(time[j]) )
       {
         isChanged= true;
         time[j]= df[j].getText();

         byTimeInfo.isRemoved= true;
       }
     }
   }

   // If date changed, a replacement EventInfo is required
   panel= (DataPanel)component[0];
   if( !date.equals(sortDate(panel.getFieldText(0))) ) // If date changed
   {
     ByDateInfo insert= new ByDateInfo(this);

     insert.date= sortDate(((DataPanel)component[0]).getFieldText(0));
     insert.addAllPlayers();
     panel= null;
     isRemoved= true;
     byDateVector.add(insert);

     byTimeInfo.isRemoved= true;
   }
}
}; // class ByDateInfo

//----------------------------------------------------------------------------
//
// Subclass-
//       EventsEdit.ByTimeInfo
//
// Purpose-
//       Describe event information by time.
//
//----------------------------------------------------------------------------
class ByTimeInfo extends DatabaseInfo {
   Vector<String>      round;       // Round date+time

   ByTimeInfo(                      // Default constructor (Loads the database)
     String            eventsID)    // For this EVENTSID
{
   boolean onReset= dbReady();

   round= new Vector<String>();

   String CMD=  CMD_EVENTS_TIME;
   String NEXT= eventsID;
   for(;;)
   {
     String string= dbNext(CMD, NEXT);
     if( string == null )
       break;

     QuotedTokenizer t= new QuotedTokenizer(string);
     String type= t.nextToken();
     String item= t.nextToken();

     if( !type.equals(CMD) )
       break;

     NEXT= item;
     round.add(item);
     isPresent= true;
   }

   dbReset(onReset);
}

public boolean                      // TRUE iff removed
   output( )                        // Update database
{
   if( isPresent && isRemoved )
   {
     Iterator<String> i= round.iterator();
     while( i.hasNext() )
     {
       dbRemove(CMD_EVENTS_TIME, i.next());
     }

     isPresent= false;
     return true;
   }

   return false;
}
}

//----------------------------------------------------------------------------
//
// Subclass-
//       EventsEdit.PlayerInfo
//
// Purpose-
//       Describe player information.
//
//----------------------------------------------------------------------------
public DataPanel                    // Resultant DataPanel
   PlayerInfo_topPanel( )           // Generate heading panel
{
   DataPanel panelHdcp= new DataPanel(2);
   DataField[] df= panelHdcp.getField();
   DataField field= df[0];
   field.setText("Player");
   field.setColumns(50);
   MenuScroller.setScrollerFor(new HdcpPlayerPopup(field));
   field= df[1];
   field.setText("Hdcp");
   field.setColumns(50);
   panelHdcp.setColor(LIGHT_BLUE, Color.BLACK);

   return panelHdcp;
}

class PlayerInfo extends DatabaseInfo {
String                 playerID;    // The PLAYER_ID
String                 playerNN;    // The PLAYER_NN
String                 playerShow;  // The PLAYER_SHOW
String                 playerHdcp;  // The PLAYER_HDCP <OPTIONAL>

DataPanel              panel;       // Player panel
JRadioButtonMenuItem   button;      // Player button

public void
   debug()
{
   super.debug();
   System.out.println("PlayerInfo.playerID: " + playerID);
   System.out.println("PlayerInfo.playerShow: " + playerShow);
   System.out.println("PlayerInfo.playerHdcp: " + playerHdcp);
   System.out.println("PlayerInfo.button: " +
       (button == null ? "null" : button.isSelected()));
}

public DataField                    // Resultant tail DataField
   genPanel(                        // Generate new panel
     FocusListener     listener,    // FocusListener
     DataField         tail)        // The current tail DataField
{
   panel= null;
   if( !isRemoved )
   {
     panel= new DataPanel(2);
     DataField[] df= panel.getField();
     DataField field= df[0];
     field.setColumns(50);
     field.setEditable(false);
     field.setText(playerShow);
     field= df[1];
     field.setColumns(50);
     field.setText(playerHdcp);

     if( listener != null )
     {
       field.addFocusListener(listener);
       field.setEditable(true);
     }

     if( tail != null )
     {
       tail.addFocusListener(new NextFocusAdapter(field));
       tail= field;
     }
   }

   return tail;
}

public boolean                      // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( !isRemoved
       && !v.isValidHdcp(panel.getField(1)) )
     return false;

   return true;
}

public void
   update( )                        // Update from panel
{
   if( panel != null && !playerHdcp.equals(panel.getFieldText(1)) )
   {
     isChanged= true;
     playerHdcp= panel.getFieldText(1);
   }
}
}; // class PlayerInfo

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.CourseButton
//
// Purpose-
//       Button for CoursePopup. (Course selector for date.)
//
//----------------------------------------------------------------------------
private class CourseButton extends JRadioButtonMenuItem {
CoursePopup            popup;       // The popup menu
int                    index;       // The course index

   CourseButton(                    // Constructor
     String            string,      // Button string
     boolean           select,      // Selected indicator
     CoursePopup       menu,        // The popup menu
     int               item)        // The course index
{
   super(string, select);
// debug("CourseButton.CourseButton" + "(" + string + "," + select + "," + (menu != null) + "," + item + ")");

   popup= menu;
   index= item;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( popup != null )              // If not called from super(..)
   {
     debug("CourseButton.fireItemStateChanged(" + isSelected() + ") " + index );

     if( isSelected() )
     {
       popup.select(index);
       super.fireItemStateChanged(e);
     }
   }
}
} // class CourseButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.CourseListener
//
// Purpose-
//       Listener for CoursePopup.
//
//----------------------------------------------------------------------------
private class CourseListener extends PopupListener
     implements ItemListener {
   CourseListener(                  // Constructor
     CoursePopup       popup)       // The popup menu
{
   super(popup);
}

public void
   itemStateChanged(                // ItemListener event
     ItemEvent       e)
{
   debug("CourseListener.itemStateChanged()");
   if( ((CoursePopup)getPopup()).change() )
     new Update().execute();
}
} // class CourseListener

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.CoursePopup
//
// Purpose-
//       CoursePopup object.
//
//----------------------------------------------------------------------------
private class CoursePopup extends JPopupMenu {
Vector<CourseButton>   vector= new Vector<CourseButton>();
ByDateInfo             byDateInfo;  // The associated ByDateInfo
boolean                changed;     // The changed indicator
int                    index;       // The selected byDate_info index

   CoursePopup(                     // Constructor
     ByDateInfo        info,        // The associated ByDateInfo
     DataField         field)       // The associated DataField
{
   CourseListener courseListener= new CourseListener(this);

   this.byDateInfo= info;
   index= info.courseIX;

   for(int i= 0; i<course_info.length; i++)
   {
     CourseButton button= new CourseButton(course_info[i].courseShow, index == i, this, i);
     button.addItemListener(courseListener);
     add(button);
     vector.add(button);
   }

   field.addMouseListener(courseListener);
}

public boolean                      // The change indicator
   change( )                        // Has data changed?
{
   debug("CoursePopup.change()");

   boolean result= changed;
   changed= false;

   return result;
}

public boolean                      // The change indicator
   select(                          // Update the selected index
     int               index)       // With this index
{
   debug("CoursePopup.select(" + index + ") " + this.index);

   if( index != this.index )
   {
     changed= true;

     vector.elementAt(this.index).setSelected(false);
     this.index= index;

     byDateInfo.isChanged= true;
     byDateInfo.courseIX= index;
     byDateInfo.teeboxIX= 0;
   }

   return changed;
}
} // class CoursePopup

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.EventsButton
//
// Purpose-
//       Button for EventsPopup. (Date activation selector.)
//
//----------------------------------------------------------------------------
private class EventsButton extends JRadioButtonMenuItem {
   EventsButton(                    // Constructor
     String            string,      // Button string
     boolean           select)      // Selected indicator
{
   super(string, select);
}
} // class EventsButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.EventsListener
//
// Purpose-
//       Listener for EventsPopup.
//
//----------------------------------------------------------------------------
private class EventsListener extends PopupListener
     implements ItemListener {

   EventsListener(                  // Constructor
     EventsPopup       popup)       // The popup menu
{
   super(popup);
}

public void
   itemStateChanged(                // ItemListener event
     ItemEvent       e)
{
   debug("EventsListener.itemStateChanged()");

   for(int i= 0; i<byDate_info.length; i++)
     byDate_info[i].isRemoved= !byDate_info[i].button.isSelected();

   new Update().execute();
}
} // class EventsListener

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.EventsPopup
//
// Purpose-
//       EventsPopup object.
//
//----------------------------------------------------------------------------
private class EventsPopup extends JPopupMenu {
   EventsPopup(                     // Constructor
     DataField         field)       // The associated DataField
{
   EventsListener eventsListener= new EventsListener(this);

   for(int i= 0; i<byDate_info.length; i++)
   {
     ByDateInfo byDateInfo= byDate_info[i];
     byDateInfo.button= new EventsButton(showDate(byDateInfo.date), !byDateInfo.isRemoved);
     byDateInfo.button.addItemListener(eventsListener);
     add(byDateInfo.button);
   }

   field.addMouseListener(eventsListener);
}
} // class EventsPopup

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.DatePlayerButton
//
// Purpose-
//       Button for DatePlayerPopup. (List of Dates for Player)
//
//----------------------------------------------------------------------------
private class DatePlayerButton extends JRadioButtonMenuItem {
ByDateInfo             byDateInfo;  // The associated byDateInfo
int                    playerIX;    // The player index

   DatePlayerButton(                // Constructor
     String            string,      // Button string
     boolean           select,      // Selected indicator
     ByDateInfo        info,        // Associated ByDateInfo
     int               index)       // The player index
{
   super(string, select);

   byDateInfo= info;
   playerIX= index;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( byDateInfo != null )         // If not called from super(..)
   {
     debug("DatePlayerButton.fireItemStateChanged(" + isSelected() + ") " + playerIX );

     if( isSelected() )
       byDateInfo.playerInsert(playerIX);
     else
       byDateInfo.playerRemove(playerIX);

     new Update().execute();
   }
}
} // class DatePlayerButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.DatePlayerPopup
//
// Purpose-
//       DatePlayerPopup object.
//
//----------------------------------------------------------------------------
private class DatePlayerPopup extends JPopupMenu {
   DatePlayerPopup(                 // Constructor
     DataField         field,       // The associated DataField
     int               index)       // The associated PlayerInfo index
{
   PopupListener playerListener= new PopupListener(this);

   PlayerInfo info= player_info[index];
   for(int i= 0; i<byDate_info.length; i++)
   {
     ByDateInfo byDateInfo= byDate_info[i];
     if( !byDateInfo.isRemoved )
     {
       boolean selected= false;
       for(int j= 0; j<byDateInfo.player_IX.length; j++)
       {
         if( byDateInfo.player_IX[j] == index )
         {
           selected= true;
           break;
         }
       }

       DatePlayerButton button= new DatePlayerButton(showDate(byDateInfo.date), selected, byDateInfo, index);
       add(button);
     }
   }

   field.addMouseListener(playerListener);
}
} // class DatePlayerPopup

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.HdcpPlayerButton
//
// Purpose-
//       Button for HdcpPlayerPopup. (Player selector.)
//
//----------------------------------------------------------------------------
private class HdcpPlayerButton extends JRadioButtonMenuItem {
PlayerInfo             playerInfo;  // Associated PlayerInfo

   HdcpPlayerButton(                // Constructor
     PlayerInfo        info)        // Associated PlayerInfo
{
   super(info.playerShow, !info.isRemoved);

   playerInfo= info;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( playerInfo != null )
   {
     debug("HdcpPlayerButton.fireItemStateChanged(" + isSelected() + ") " + playerInfo.playerShow);

     if( isSelected() )
     {
       playerInfo.isChanged= true;
       playerInfo.isRemoved= false;
     }
     else
     {
       playerInfo.isChanged= false;
       playerInfo.isRemoved= true;
     }

     int index= playerIdIndex(playerInfo.playerID);
     for(int i= 0; i<byDateVector.size(); i++)
     {
       ByDateInfo info= byDateVector.elementAt(i);
       if( isSelected() )
         info.playerInsert(index);
       else
         info.playerRemove(index);
     }

     super.fireItemStateChanged(e);
   }
}
} // class HdcpPlayerButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.HdcpPlayerListener
//
// Purpose-
//       Listener for HdcpPlayerPopup.
//
//----------------------------------------------------------------------------
private class HdcpPlayerListener extends PopupListener
     implements ItemListener {
   HdcpPlayerListener(              // Constructor
     HdcpPlayerPopup   popup)       // The popup menu
{
   super(popup);
}

public void
   itemStateChanged(                // ItemListener event
     ItemEvent       e)
{
   debug("HdcpPlayerListener.itemStateChanged()");

   new Update().execute();
}
} // class HdcpPlayerListener

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.HdcpPlayerPopup
//
// Purpose-
//       HdcpPlayerPopup object.
//
//----------------------------------------------------------------------------
private class HdcpPlayerPopup extends JPopupMenu {
   HdcpPlayerPopup(                 // Constructor
     DataField         field)       // The associated DataField
{
   HdcpPlayerListener playerListener= new HdcpPlayerListener(this);

   for(int i= 0; i<player_info.length; i++)
   {
     PlayerInfo playerInfo= player_info[i];

     playerInfo.button= new HdcpPlayerButton(playerInfo);
     playerInfo.button.addItemListener(playerListener);
     add(playerInfo.button);
   }

   field.addMouseListener(playerListener);
}
} // class HdcpPlayerPopup

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.PlayerDatePopup
//
// Purpose-
//       PlayerDatePopup object. (List of players on date.)
//
//----------------------------------------------------------------------------
private class PlayerDatePopup extends JPopupMenu {
   PlayerDatePopup(                 // Constructor
     DataField         field,       // The associated DataField
     ByDateInfo        byDateInfo)  // The associated ByDateInfo
{
   PopupListener playerListener= new PopupListener(this);

   for(int i= 0; i<player_info.length; i++)
   {
     PlayerInfo playerInfo= player_info[i];
     if( !playerInfo.isRemoved )
     {
       boolean selected= false;
       for(int j= 0; j<byDateInfo.player_IX.length; j++)
       {
         if( byDateInfo.player_IX[j] == i )
         {
           selected= true;
           break;
         }
       }

       DatePlayerButton button= new DatePlayerButton(playerInfo.playerShow, selected, byDateInfo, i);
       add(button);
     }
   }

   field.addMouseListener(playerListener);
}
} // class PlayerDatePopup

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.PostPanel
//
// Purpose-
//       Golf data Panel.
//
//----------------------------------------------------------------------------
private class PostPanel extends DataPanel {
boolean                operational; // The OPERATIONAL control
PostButton             postButton;  // The POST Button
PostListener           postListener;// The POST Listener
JTextField             postMessage; // The POST Message
Validator              validator;   // The POST Button Validator

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.PostPanel.PostPanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PostPanel(                       // Constructor
     Validator         validator)   // The POST Button Validator
{
   super(0);

   this.operational= false;
   this.postButton= new PostButton();
   this.postListener= new PostListener();
   this.postMessage= new DataField(100);
   this.postMessage.setForeground(Color.RED);
   this.validator= validator;

   add(new DataField(0));           // Prevents tab into PostButton
   add(postButton);
   add(postMessage);
   add(teamButton);
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.PostPanel.getListener()
//       EventsEdit.PostPanel.setMessage(String)
//       EventsEdit.PostPanel.setOperational(boolean)
//
// Purpose-
//       Accessors.
//
//----------------------------------------------------------------------------
public FocusListener                // The PostListener
   getListener( )                   // Get PostListener
{
   return postListener;
}

public void
   setMessage(                      // Set PostMessage text
     String            string)      // To this String
{
   postMessage.setText(string);
}

public void
   setMessage( )                    // Set PostMessage text
{
   postMessage.setText("");
}

public void
   setOperational(                  // Set OPERATIONAL control
     boolean           operational) // To this
{
   this.operational= operational;
   postMessage.setText("");

   if( operational )
     postButton.validityCheck();
}

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.PostPanel.PostButton
//
// Purpose-
//       The POST Button.
//
//----------------------------------------------------------------------------
protected class PostButton extends JButton implements FocusListener {
   PostButton( )                    // The POST Button
{
   super("POST");

   setEnabled(false);
   addFocusListener(this);
}

public void
   focusGained(                     // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
   if( validator.isValid() )        // This check is needed!
   {
     setText("DONE");
     setEnabled(false);
     validator.actionPerformed();
   }
}

public void
   focusLost(                       // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
}

public void
   validityCheck( )                 // Check Validator status
{
   setEnabled(false);
   teamButton.setEnabled(false);
   if( operational && validator.isValid() )
   {
     setText("POST");
     setEnabled(true);
     if( !teamButton.isWorking )
       teamButton.setEnabled(true);
   }
}
} // class PostButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.PostPanel.PostListener
//
// Purpose-
//       PostButton FocusAdapter.
//
//----------------------------------------------------------------------------
protected class PostListener extends FocusAdapter {
public void
   focusLost(                       // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
   postButton.validityCheck();
}
} // class PostListener
} // class PostPanel

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.TeamButton
//
// Purpose-
//       The TEAM Button. (Recalculate Teams)
//
//----------------------------------------------------------------------------
private class TeamButton extends JButton implements FocusListener {
   boolean             isWorking;   // TRUE while working

   TeamButton( )                    // The POST Button
{
   super("TEAM calculator");

   setEnabled(false);
   addFocusListener(this);
}

public void
   focusGained(                     // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
   debug("TeamButton: focusGained() " + isEnabled());

   //-------------------------------------------------------------------------
   // It is possible to get focus while not enabled. If so, we ignore it.
   if( isEnabled() && !isWorking )
   {
     isWorking= true;
     new Reteam().execute();
   }

   setEnabled(false);
}

public void
   focusLost(                       // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
}
} // class TeamButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.TeeboxButton
//
// Purpose-
//       Button for TeeboxPopup.
//
//----------------------------------------------------------------------------
private class TeeboxButton extends JRadioButtonMenuItem {
TeeboxPopup            popup;       // The popup menu
int                    index;       // The Teebox index

   TeeboxButton(                    // Constructor
     String            string,      // Button string
     boolean           select,      // Selected indicator
     TeeboxPopup       menu,        // The popup menu
     int               item)        // The Teebox index
{
   super(string, select);
// debug("TeeboxButton.TeeboxButton" + "(" + string + "," + select + "," + (menu != null) + "," + item + ")");

   popup= menu;
   index= item;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( popup != null )
   {
     debug("TeeboxButton.fireItemStateChanged(" + isSelected() + ") " + index );

     if( isSelected() )
       popup.select(index);
     super.fireItemStateChanged(e);
   }
}
} // class TeeboxButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.TeeboxListener
//
// Purpose-
//       Listener for TeeboxPopup.
//
//----------------------------------------------------------------------------
private class TeeboxListener extends PopupListener
     implements ItemListener {
   TeeboxListener(                  // Constructor
     TeeboxPopup       popup)       // The popup menu
{
   super(popup);
}

public void
   itemStateChanged(                // ItemListener event
     ItemEvent       e)
{
   debug("TeeboxListener.itemStateChanged()");
   if( ((TeeboxPopup)getPopup()).change() )
     new Update().execute();
}
} // class TeeboxListener

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.TeeboxPopup
//
// Purpose-
//       TeeboxPopup object.
//
//----------------------------------------------------------------------------
private class TeeboxPopup extends JPopupMenu {
Vector<TeeboxButton>   vector= new Vector<TeeboxButton>();
ByDateInfo             byDateInfo;  // The associated ByDateInfo
boolean                changed;     // The changed indicator
int                    index;       // The selected byDate_info index

   TeeboxPopup(                     // Constructor
     ByDateInfo        info,        // The associated ByDateInfo
     DataField         field)       // The associated DataField
{
   TeeboxListener teeboxListener= new TeeboxListener(this);

   this.byDateInfo= info;
   CourseInfo courseInfo= course_info[info.courseIX];
   TeeboxInfo[] teebox_info= courseInfo.teebox_info;
   index= info.teeboxIX;

   for(int i= 0; i<teebox_info.length; i++)
   {
     TeeboxButton button= new TeeboxButton(teebox_info[i].teeboxID, index == i, this, i);
     button.addItemListener(teeboxListener);
     add(button);
     vector.add(button);
   }

   field.addMouseListener(teeboxListener);
}

public boolean                      // The change indicator
   change( )                        // Has data changed?
{
   debug("TeeboxPopup.change()");

   boolean result= changed;
   changed= false;

   return result;
}

public boolean                      // The change indicator
   select(                          // Update the selected index
     int               index)       // With this index
{
   debug("TeeboxPopup.select(" + index + ") " + this.index);

   if( index != this.index )
   {
     changed= true;

     vector.elementAt(this.index).setSelected(false);
     this.index= index;

     byDateInfo.isChanged= true;
     byDateInfo.teeboxIX= index;
   }

   return changed;
}
} // class TeeboxPopup

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.TimeDeleteButton
//
// Purpose-
//       Button for TimePopup.
//
//----------------------------------------------------------------------------
private class TimeDeleteButton extends JRadioButtonMenuItem {
ByDateInfo             byDateInfo;  // Associated ByDateInfo
int                    index;       // Element index

   TimeDeleteButton(                // Constructor
     ByDateInfo        info,        // Associated ByDateInfo
     int               index)       // Time index
{
   super("Delete", false);

   byDateInfo= info;
   this.index= index;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( byDateInfo != null && byDateInfo.time.length > 1 )
   {
     debug("TimeDeleteButton.fireItemStateChanged(" + isSelected() + ")");

     byDateInfo.isChanged= true;
     String[] array= new String[byDateInfo.time.length - 1];
     for(int i= 0; i<index; i++)
       array[i]= byDateInfo.time[i];
     for(int i= index+1; i<byDateInfo.time.length; i++)
       array[i-1]= byDateInfo.time[i];
     byDateInfo.time= array;

     new Update().execute();
   }
   else if( byDateInfo != null && isSelected() )
     postPanel.setMessage("At least one tee time is required");
}
} // class TimeDeleteButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.TimeInsertButton
//
// Purpose-
//       Button for TimePopup.
//
//----------------------------------------------------------------------------
private class TimeInsertButton extends JRadioButtonMenuItem {
ByDateInfo             byDateInfo;  // Associated ByDateInfo
int                    index;       // Element index

   TimeInsertButton(                // Constructor
     ByDateInfo        info,        // Associated ByDateInfo
     int               index)       // Time index
{
   super("Insert", false);

   byDateInfo= info;
   this.index= index;
}

public void
   fireItemStateChanged(            // Fire ItemListener event
     ItemEvent       e)
{
   if( byDateInfo != null )
   {
     debug("TimeInsertButton.fireItemStateChanged(" + isSelected() + ")");

     byDateInfo.update();
     byDateInfo.isChanged= true;
     String[] array= new String[byDateInfo.time.length + 1];
     for(int i= 0; i<index; i++)
       array[i]= byDateInfo.time[i];
     array[index]= "hh:mm";
     for(int i= index+1; i<array.length; i++)
       array[i]= byDateInfo.time[i-1];
     byDateInfo.time= array;

     new Update().execute();
   }
}
} // class TimeInsertButton

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.TimePopup
//
// Purpose-
//       TimePopup object.
//
//----------------------------------------------------------------------------
private class TimePopup extends JPopupMenu {
   TimePopup(                       // Constructor
     DataField         field,       // The associated DataField
     ByDateInfo        info,        // The associated ByDateInfo
     int               index)       // The associated time index
{
   PopupListener popupListener= new PopupListener(this);
   add(new TimeDeleteButton(info, index));
   add(new TimeInsertButton(info, index));
   field.addMouseListener(popupListener);
}
} // class TimePopup

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.UpdateListener
//
// Purpose-
//       Check whether a field update has occurred.
//
//----------------------------------------------------------------------------
private class UpdateListener implements FocusListener {
DataField              field;       // Associated DataField
String                 string;      // Initial String

   UpdateListener(                  // Constructor
     DataField         field)       // Associated DataField
{
   this.field= field;
   this.string= field.getText();
   field.addFocusListener(this);
}

public void
   focusGained(                     // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
}

public void
   focusLost(                       // Implement FocusListener
     FocusEvent        e)           // The FocusEvent
{
   debug("UpdateListener.focusLost()");

   if( !field.getText().equals(string) )
     new Update().execute();
}
} // class UpdateListener

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.Format
//
// Purpose-
//       Create a background task to format the data.
//
//----------------------------------------------------------------------------
class Format extends SwingWorker<Object, Void> {
//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Format.doInBackground
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

     // Generate the byDate_info array
     byDate_info= new ByDateInfo[byDateVector.size()];
     for(int i= 0; i<byDate_info.length; i++)
       byDate_info[i]= byDateVector.elementAt(i);

     // Sort the byDate_info array by date
     for(int i= 0; i<byDate_info.length; i++)
     {
       for(int j= i+1; j<byDate_info.length; j++)
       {
         if( byDate_info[i].date.compareTo(byDate_info[j].date) > 0 )
         {
           ByDateInfo temp= byDate_info[i];
           byDate_info[i]= byDate_info[j];
           byDate_info[j]= temp;
         }
       }
     }

     // Format the data
     try {
       // Generate the base panels
       eventsShow.genPanel(postListener, null);
       DataPanel panel= eventsShow.getPanel();
       DataField[] df= panel.getField();
       DataField field= df[0];
       field.setBackground(Color.YELLOW);
       head= tail= field;

       tail= eventsName.genPanel(postListener, tail);

       // Generate the player handicap panels
       panelHdcp= PlayerInfo_topPanel();
       for(int i= 0; i<player_info.length; i++)
       {
         tail= player_info[i].genPanel(postListener, tail);
         panel= player_info[i].panel;
         if( panel != null )
           MenuScroller.setScrollerFor(new DatePlayerPopup(panel.getField(0), i));
       }

       // Generate the date informational panels
       panelDate= ByDateInfo_topPanel();
       for(int i= 0; i<byDate_info.length; i++)
       {
         tail= byDate_info[i].genPanel(postListener, tail);
         field= byDate_info[i].getDateField();
         if( field != null )
           MenuScroller.setScrollerFor(new PlayerDatePopup(field, byDate_info[i]));
       }

       // Tail goes to head
       tail.addFocusListener(new NextFocusAdapter(head));

       // Activate the POST panel
       postPanel.setOperational(true);
     } catch( Exception e ) {
       setERROR("Format: Client exception: " + e);
       e.printStackTrace();
       waitingDone();
       return null;
     }
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Format.done
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
       content.add(eventsShow.getPanel());
       content.add(eventsName.getPanel());

       content.add(panelHdcp);
       for(int i= 0; i<player_info.length; i++)
       {
         if( !player_info[i].isRemoved )
           content.add(player_info[i].panel);
       }

       content.add(panelDate);
       for(int i= 0; i<byDate_info.length; i++)
       {
         if( !byDate_info[i].isRemoved )
           content.add(byDate_info[i].panel);
       }

       content.add(postPanel);

       head.requestFocusInWindow();
       content.revalidate();
       content.repaint();
     }
   }}}} // Synchronized(owner)

   setVisible(true);
   waitingDone();
}
} // class EventsEdit.Format

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.Loader
//
// Purpose-
//       Create a background task to load the data.
//
//----------------------------------------------------------------------------
class Loader extends SwingWorker<Object, Void> {
//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Loader.doInBackground
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
   String[]            sarray;      // Working temporary String[]
   String              string;      // Working temporary String

   debug("EditLoader.doInBackground()");
   dbReady();

   synchronized(owner)
   {{{{
     // Load the data
     try {
       if( client.fsm != DbStatic.FSM_READY )
         throw new Exception("DbServer offline");

       // Extract associated data
       eventsID= dbGet(CMD_EVENTS_FIND, eventsNN);
       if( eventsID == null )
         eventsID= dbGet(CMD_DEFAULT, DEFAULT_CODE_EI);
       if( eventsID == null )
         throw new Exception("Cannot find EVENTS_ID");
       eventsID= dbGet(CMD_EVENTS_FIND, eventsID);

       eventsShow= new GenericItemInfo(dbGet(CMD_EVENTS_SHOW, eventsID));
       eventsName= new GenericNameInfo(dbGet(CMD_EVENTS_NAME, eventsID));

       // Collect Course, Teebox and Player information
       loadStaticInformation();

       // Collect event information by date
       String CMD=  CMD_EVENTS_DATE;
       String NEXT= eventsID;
       for(;;)
       {
         string= dbNext(CMD, NEXT);
         if( string == null )
           break;

         QuotedTokenizer t= new QuotedTokenizer(string);
         String type= t.nextToken();
         String item= t.nextToken();

         if( !type.equals(CMD) )
           break;

         NEXT= item;
         int index= catcon(item);
         if( index < 0 )
           continue;

         String itemNick= item.substring(0, index);
         if( !itemNick.equalsIgnoreCase(eventsID)
             && !itemNick.equalsIgnoreCase(eventsNN) )
         {
           itemNick= dbGet(CMD_EVENTS_FIND, itemNick);
           if( !itemNick.equalsIgnoreCase(eventsID)
               && !itemNick.equalsIgnoreCase(eventsNN) )
             break;
         }

         ByDateInfo byDateInfo= new ByDateInfo();
         byDateInfo.isPresent= true;
         byDateInfo.date= item.substring(index+1);
         String courseID= t.nextToken();
         String courseNM= stripQuotes(dbGet(CMD_COURSE_SHOW, courseID));
         if( courseNM == null )
           courseNM= courseID;
         byDateInfo.courseIX= courseIndex(courseNM);
         CourseInfo courseInfo= course_info[byDateInfo.courseIX];

         String teeboxID= t.nextToken();
         byDateInfo.teeboxIX= teeboxIndex(courseInfo, teeboxID);
         byDateInfo.time= tokenize(t.remainder());

         // Player list for this date
         String[] player_ID= dbRetrieve(CMD_EVENTS_PLAY, item);
         if( player_ID == null )
           player_ID= new String[0];
         byDateInfo.player_IX= new int[player_ID.length];
         for(int i= 0; i<byDateInfo.player_IX.length; i++)
         {
           String playerID= dbGet(CMD_PLAYER_FIND, player_ID[i]);
           if( playerID == null )
             playerID= player_ID[i];
           byDateInfo.player_IX[i]= playerIdIndex(playerID);
         }

         byDateVector.add(byDateInfo);
       }

       // Collect event information by time
       byTimeInfo= new ByTimeInfo(eventsID);
     } catch( Exception e ) {
       setERROR("Loader: Client exception: " + e);
       e.printStackTrace();
       waitingDone();
       return null;
     }
   }}}} // Synchronized(owner)

   dbReset();

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Loader.done
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
   debug("EditLoader.done()");

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
   setVisible(true);
}
} // class EventsEdit.Loader

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.Output
//
// Purpose-
//       Create a background task to write the data.
//
//----------------------------------------------------------------------------
class Output extends SwingWorker<Object, Void> {
//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Output.doInBackground
//
// Purpose-
//       Output the data.
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
     update();
     output();
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Output.done
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
} // class EventsEdit.Output

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.Reteam
//
// Purpose-
//       Create a background task to update the teams.
//
//----------------------------------------------------------------------------
class Reteam extends SwingWorker<Object, Void> {
//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Reteam.doInBackground
//
// Purpose-
//       Update the teams.
//
// Notes-
//       Called by SwingWorker via execute().
//
//----------------------------------------------------------------------------
@Override
public Object
   doInBackground()
{
   debug("Reteam.doInBackground()");

   synchronized(teamButton)
   {{{{
     if( isUpdateValid() )
     {
       teamButton.setEnabled(false);
       update();                    // Synchronize internally
       output();                    // Synchronize database

       // Generate EventsDateInfo Array
       int count= 0;
       for(int i= 0; i<byDateVector.size(); i++)
         if( !byDateVector.elementAt(i).isRemoved )
           count++;

       EventsDateInfo[] date_info= new EventsDateInfo[count];
       int index= 0;
       for(int i= 0; i<byDateVector.size(); i++)
       {
         ByDateInfo info= byDateVector.elementAt(i);
         if( !info.isRemoved )
         {
           EventsDateInfo date= new EventsDateInfo();

           date.date= info.date;
           date.time= new String[info.time.length];
           for(int j= 0; j<date.time.length; j++)
             date.time[j]= info.time[j];

           date.player_data= new PlayerData[info.player_IX.length];
           for(int j= 0; j<date.player_data.length; j++)
           {
             PlayerInfo from= player_info[info.player_IX[j]];
             PlayerData into= new PlayerData();
             into.playerID= from.playerID;
             into.playerNN= from.playerNN;
             date.player_data[j]= into;
           }

           date_info[index++]= date;
         }
       }

       // Run the MonteCarlo evaluator
       evaluate(date_info);

       teamButton.setEnabled(true);
       byTimeInfo= new ByTimeInfo(eventsID);
     }

     teamButton.isWorking= false;
   }}}} // Synchronized(owner)

   return new Object();
}

//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Reteam.done
//
// Purpose-
//       Reteam complete.
//
// Notes-
//       Called by SwingWorker in EWT after doInBackground completes.
//
//----------------------------------------------------------------------------
@Override
public void done()
{
   debug("Reteam.done()");
   new Format().execute();
}
} // class EventsEdit.Reteam

//----------------------------------------------------------------------------
//
// Class-
//       EventsEdit.Update
//
// Purpose-
//       Create a background task to update the data.
//
//----------------------------------------------------------------------------
class Update extends SwingWorker<Object, Void> {
//----------------------------------------------------------------------------
//
// Method-
//       EventsEdit.Update.doInBackground
//
// Purpose-
//       Update the data.
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
//       EventsEdit.Update.done
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
   debug("Update.done()");

   new Format().execute();
}
} // class EventsEdit.Update
} // class EventsEdit
