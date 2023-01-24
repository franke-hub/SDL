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
//       FgValidator.java
//
// Purpose-
//       Validate all data. (Validator portion.)
//
// Last change date-
//       2023/01/19
//
// Usage notes-
//       Only called from BgValidator.java.
//
// Implementation notes-
//       If this type of class is required again, this should be split into
//       AppletDebuggingAdaptor + remainder, where AppletDebuggingAdaptor
//       contains the applet utility functions and StaticJApplet copies.
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import java.lang.*;
import java.util.*;

import usr.fne.common.QuotedTokenizer;

//----------------------------------------------------------------------------
//
// Class-
//       FgValidator
//
// Purpose-
//       Select teams using Monte Carlo evaluation.
//
//----------------------------------------------------------------------------
public class FgValidator extends DebuggingAdaptor {
//----------------------------------------------------------------------------
// FgValidator.Attributes
//----------------------------------------------------------------------------
boolean                DEBUG= true; // <DEBUG> DEBUGGING active?
boolean                DEBUG_EVAL= false; // <DEBUG_EVAL> (Now unused)
boolean                DEBUG_HCDM= false; // <DEBUG_HCDM> Hard Core Debug Mode

// Construction data
CommonJFrame           applet;      // The calling applet

// Validation data areas, exposed for debugging
int                    errorCount= 0; // Number of errors detected

//----------------------------------------------------------------------------
// FgValidator.Constants
//----------------------------------------------------------------------------
// From StaticJApplet
static final String    CMD_COURSE_FIND= StaticJApplet.CMD_COURSE_FIND;
static final String    CMD_COURSE_HTTP= StaticJApplet.CMD_COURSE_HTTP;
static final String    CMD_COURSE_LONG= StaticJApplet.CMD_COURSE_LONG;
static final String    CMD_COURSE_NAME= StaticJApplet.CMD_COURSE_NAME;
static final String    CMD_COURSE_NEAR= StaticJApplet.CMD_COURSE_NEAR;
static final String    CMD_COURSE_PARS= StaticJApplet.CMD_COURSE_PARS;
static final String    CMD_COURSE_SHOW= StaticJApplet.CMD_COURSE_SHOW;
static final String    CMD_COURSE_TBOX= StaticJApplet.CMD_COURSE_TBOX;

static final String    CMD_EVENTS_CARD= StaticJApplet.CMD_EVENTS_CARD;
static final String    CMD_EVENTS_POST= StaticJApplet.CMD_EVENTS_POST;

static final String    CMD_PLAYER_CARD= StaticJApplet.CMD_PLAYER_CARD;
static final String    CMD_PLAYER_POST= StaticJApplet.CMD_PLAYER_POST;

static final String    CMD_PLAYER_FIND= StaticJApplet.CMD_PLAYER_FIND;
static final String    CMD_PLAYER_MAIL= StaticJApplet.CMD_PLAYER_MAIL;
static final String    CMD_PLAYER_NAME= StaticJApplet.CMD_PLAYER_NAME;
static final String    CMD_PLAYER_NICK= StaticJApplet.CMD_PLAYER_NICK;
static final String    CMD_PLAYER_SHOW= StaticJApplet.CMD_PLAYER_SHOW;

// From HoleInfo
static final int       HOLE_COURSE= HoleInfo.ARRAY_COURSE;
static final int       HOLE_TEEBOX= HoleInfo.ARRAY_TEEBOX;
static final int       HOLE_FIELDS= HOLE_TEEBOX + 1;

// From HolePanel
static final int       HOLE_MRATE= HolePanel.FORMAT_USER;
static final int       HOLE_SLOPE= HOLE_MRATE+1;
static final int       HOLE_WRATE= HOLE_SLOPE+1;
static final int       HOLE_WOMEN= HOLE_WRATE+1;
static final int       HOLE_COUNT= HOLE_WOMEN+1;

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.DebuggingInterface
//
// Purpose-
//       Implement DebuggingInterface
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   debug("FgValidator");
}

public boolean                      // If debugging active
   isDebug( )                       // Is debugging active?
{
   return DEBUG;
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.FgValidator
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   FgValidator(                     // Constructor
     CommonJFrame      applet)      // Caller data
{
   this.applet= applet;             // Save applet data

   debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.UTILITY
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
// DBSTATIC utility functions
public static int                   // The concatenator index
   catcon(                          // Inverse concatcatenate
     String            item,        // The item
     int               index)       // The item index
{
   return DbStatic.catcon(item, index);
}

public static int                   // The concatenator index
   catcon(                          // Inverse concatcatenate
     String            item)        // The item
{
   return DbStatic.catcon(item);
}

public static String[]              // Resultant
   tokenize(                        // Tokenize
     String            string)      // This String
{
   String[] result= null;

   if( string != null )
     result= DbStatic.tokenize(string);

   return result;
}

// APPLET utility functions
public void
   setERROR(                        // Set the error message
     String            string)      // The message String
{
   applet.setERROR(string);
}

public String                       // The database item
   dbGet(                           // Get database item
       String          type,        // The item type
       String          item)        // The item name
{
   return applet.dbGet(type,item);
}

public String                       // The database item
   dbNext(                          // Get next database item
       String          type,        // The item type
       String          item)        // The item name
{
   return applet.dbNext(type,item);
}

public String                       // The prior data
   dbPut(                           // Put database item
       String          type,        // The item type
       String          item,        // The item name
       String          data)        // The item data
{
   return applet.dbPut(type,item,data);
}

public boolean                      // TRUE iff was made ready
   dbReady( )                       // Ready the client
{
   return applet.dbReady();
}


public String                       // The prior data
   dbRemove(                        // Remove database item
       String          type,        // The item type
       String          item)        // The item name
{
   return applet.dbRemove(type,item);
}

public void
   dbReset(                         // Reset the client
     boolean           ready)       // Result from ready
{
   applet.dbReset(ready);
}

public void
   dbReset( )                       // Reset the client
{
   applet.dbReset(true);
}

public String[]                     // The result String
   dbRetrieve(                      // Retrieve database information
     String            command,     // Database command
     String            item,        // Database item
     String            qual)        // Qualifier (may be NULL)
{
   return applet.dbRetrieve(command,item,qual);
}

public String[]                     // The result String
   dbRetrieve(                      // Retrieve database information
     String            command,     // Database command
     String            item)        // Database item
{
   return applet.dbRetrieve(command, item, null);
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.error
//
// Purpose-
//       Add error message content.
//
//----------------------------------------------------------------------------
public synchronized void
   error(                           // Applet error message
     String            string)      // The message
{
   write(string);
   errorCount++;
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.write
//
// Purpose-
//       Add output content.
//
//----------------------------------------------------------------------------
public synchronized void
   write(                           // Applet normal message
     String            string)      // The message
{
   ItemPanel panel= new ItemPanel(string);
   applet.content.add(panel);
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.isFieldValid
//
// Purpose-
//       Database field validator.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   isFieldValid(                    // Is field name valid?
     String            field)       // The field name
{
   if( field == null )
     return false;

   if( field.equals("") )
     return false;

   if( field.startsWith("Unknown") )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.sortTeeboxInfo
//
// Purpose-
//       Sort teeboxVector by rating.
//
//----------------------------------------------------------------------------
public CourseTboxInfo[]             // The sorted vector
   sortTeeboxInfo(                  // Sort Vector<CourseTboxInfo>
     Vector<CourseTboxInfo> vector) // The input Vector
{
   CourseTboxInfo[] result= new CourseTboxInfo[vector.size()];
   for(int i= 0; i<vector.size(); i++)
     result[i]= vector.elementAt(i);

   // Sort by rating
   for(int i= 0; i<result.length; i++)
   {
     for(int j= i+1; j<result.length; j++)
     {
       if( result[i].compareTo(result[j]) < 0 )
       {
         CourseTboxInfo temp= result[i];
         result[i]= result[j];
         result[j]= temp;
       }
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.validateCourse
//
// Purpose-
//       Database course validator.
//
//----------------------------------------------------------------------------
public void
   validateCourse( )                // Database Course validator
{
   String COURSE= "";
   for(;;)
   {
     String CMD= CMD_COURSE_FIND;
     String string= dbNext(CMD, COURSE);
     if( string == null )
       break;

     QuotedTokenizer tc= new QuotedTokenizer(string);
     String type= tc.nextToken();
     String item= tc.nextToken();
     if( !type.equals(CMD) )
       break;

     COURSE= item;
     String courseFind= dbGet(CMD_COURSE_FIND, COURSE);
     String courseHttp= dbGet(CMD_COURSE_HTTP, COURSE);
     String courseName= dbGet(CMD_COURSE_NAME, COURSE);
     String courseShow= dbGet(CMD_COURSE_SHOW, COURSE);
     if( !courseFind.equals(COURSE) )
       error("Course(" + COURSE + ") courseFind(" + courseFind + ")");
     if( !isFieldValid(courseHttp) )
       error("Course(" + COURSE + ") courseHttp(" + courseHttp + ")");
     if( !isFieldValid(courseName) )
       error("Course(" + COURSE + ") courseName(" + courseName + ")");
     if( !isFieldValid(courseShow) )
       error("Course(" + COURSE + ") courseShow(" + courseShow + ")");

     // Extract course information
     CourseLongInfo longInfo= new CourseLongInfo(HOLE_COUNT, dbGet(CMD_COURSE_LONG, COURSE));
     CourseNearInfo nearInfo= new CourseNearInfo(HOLE_COUNT, dbGet(CMD_COURSE_NEAR, COURSE));
     CourseParsInfo parsInfo= new CourseParsInfo(HOLE_COUNT, dbGet(CMD_COURSE_PARS, COURSE));

     // Validate course information
     for(int hole= 0; hole<18; hole++)
     {
       int par= Integer.parseInt(parsInfo.array[hole]);
       if( par < 3 || par > 5 )
         error("Course(" + COURSE + ") Hole(" + (hole+1) + ") " +
               "Par(" + par + ") range(3,4,5)");

       if( longInfo.array[hole].equals("+") && par != 5 )
         error("Course(" + COURSE + ") Hole(" + (hole+1) + ") " +
               "Par(" + par + ") but LD");

       if( nearInfo.array[hole].equals("+") && par != 3 )
         error("Course(" + COURSE + ") Hole(" + (hole+1) + ") " +
               "Par(" + par + ") but CP");
     }

     // Extract teebox information
     Vector<CourseTboxInfo> tboxVector; // The teebox information
     tboxVector= new Vector<CourseTboxInfo>();
     CMD= CMD_COURSE_TBOX;
     String TEEBOX= COURSE;
     for(;;)
     {
       string= dbNext(CMD, TEEBOX);
       if( string == null )
         break;

       QuotedTokenizer tt= new QuotedTokenizer(string);
       type= tt.nextToken();
       item= tt.nextToken();
       if( !type.equals(CMD) )
         break;

       TEEBOX= item;
       int index= catcon(item);
       if( index < 0 )
         continue;
       if( !COURSE.equals(TEEBOX.substring(0,index)) )
         break;

       String[] content= tokenize(tt.remainder());
       CourseTboxInfo info=
           new CourseTboxInfo(HOLE_COUNT, COURSE, courseShow,
                              TEEBOX.substring(index+1), content);
       tboxVector.add(info);
     }
     CourseTboxInfo[] tboxSorted= sortTeeboxInfo(tboxVector);

     // Verify teebox yardages
     for(int i= 0; i<tboxSorted.length; i++)
     {
       CourseTboxInfo info= tboxSorted[i];
       TEEBOX= COURSE + "." + info.title;

       for(int hole= 0; hole<18; hole++)
       {
         int yds= Integer.parseInt(info.array[hole]);
         int par= Integer.parseInt(parsInfo.array[hole]);
         boolean mens= (info.rating[0] > 0.0);

         switch( par )
         {
           case 3:
             if( yds < 50 || yds > 275 )
             {
               error("Course(" + TEEBOX + ") Hole(" + (hole+1) + ") " +
                     "Par(" + par + ") Yards(" + yds + ") range(50..250)");
             }
             break;

           case 4:
             if( info.rating[1] == 0.0 )
             {
               if( yds < 250 || yds > 550 )
                 error("Course(" + TEEBOX + ") Hole(" + (hole+1) + ") " +
                       "Par(" + par + ") Yards(" + yds + ") range(250..550)");
             } else {
               if( yds < 200 || yds > 500 )
                 error("Course(" + TEEBOX + ") Hole(" + (hole+1) + ") " +
                       "Par(" + par + ") Yards(" + yds + ") range(200..500)");
             }
             break;

           case 5:
             if( info.rating[1] == 0.0 )
             {
               if( yds < 375 || yds > 650 )
                 error("Course(" + TEEBOX + ") Hole(" + (hole+1) + ") " +
                       "Par(" + par + ") Yards(" + yds + ") range(375..650)");
             } else {
               if( yds < 300 || yds > 600 )
                 error("Course(" + TEEBOX + ") Hole(" + (hole+1) + ") " +
                       "Par(" + par + ") Yards(" + yds + ") range(300..500)");
             }
             break;

           default:
             break;
         }
       }

       if( i > 0 )
       {
         CourseTboxInfo high= tboxSorted[i-1]; // Higher rated teebox
         for(int hole= 0; hole<18; hole++)
         {
           int highHole= Integer.parseInt(high.array[hole]);
           int infoHole= Integer.parseInt(info.array[hole]);
           if( infoHole > highHole )
           {
             error("Course(" + COURSE + ") Hole(" + (hole+1) + ") " +
                   "Teebox(" + high.color + ") Par(" + highHole + ") " +
                   "Teebox(" + info.color + ") Par(" + infoHole + ")");
           }
         }
       }
     }

     if( errorCount > 10 )
     {
       debug("Too many errors...");
       break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.validatePlayer
//
// Purpose-
//       Database player validator.
//
//----------------------------------------------------------------------------
public void
   validatePlayer( )                // Database Player validator
{
   String PLAYER= "P0";
   for(;;)
   {
     String CMD= CMD_PLAYER_FIND;
     String string= dbNext(CMD, PLAYER);
     if( string == null )
       break;

     QuotedTokenizer tc= new QuotedTokenizer(string);
     String type= tc.nextToken();
     String item= tc.nextToken();
     if( !type.equals(CMD) )
       break;

     PLAYER= item;
     if( !PLAYER.startsWith("P0") )
       break;

     String playerFind= dbGet(CMD_PLAYER_FIND, PLAYER);
     String playerNick= dbGet(CMD_PLAYER_NICK, PLAYER);
     String playerMail= dbGet(CMD_PLAYER_MAIL, PLAYER);
     String playerName= dbGet(CMD_PLAYER_NAME, PLAYER);
     String playerShow= dbGet(CMD_PLAYER_SHOW, PLAYER);
     if( !isFieldValid(playerFind) || !playerFind.equals(PLAYER) )
       error("Player(" + PLAYER + ") playerFind(" + playerFind + ")");
     if( !isFieldValid(playerMail) )
       error("Player(" + PLAYER + ") playerMail(" + playerMail + ")");
     if( !isFieldValid(playerName) )
       error("Player(" + PLAYER + ") playerName(" + playerName + ")");
     if( !isFieldValid(playerNick) )
       error("Player(" + PLAYER + ") playerNick(" + playerNick + ")");
     if( !isFieldValid(playerShow) )
       error("Player(" + PLAYER + ") playerShow(" + playerShow + ")");

     playerFind= dbGet(CMD_PLAYER_FIND, playerNick);
     if( !isFieldValid(playerFind) || !playerFind.equals(PLAYER) )
       error("Player(" + PLAYER + ") playerNick(" + playerNick + ")" +
             "'" + playerFind + "'= dbGet(PLAYER_FIND," + playerNick + ")");

     if( errorCount > 10 )
     {
       debug("Too many errors...");
       break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.validateScores
//
// Purpose-
//       Database scorecard validator.
//
//----------------------------------------------------------------------------
public void
   validateScores( )                // Database scorecard validator
{
   String EVENTS= "";
   for(;;)
   {
     String CMD= CMD_EVENTS_CARD;
     String string= dbNext(CMD, EVENTS);
     if( string == null )
       break;

     QuotedTokenizer tc= new QuotedTokenizer(string);
     String type= tc.nextToken();
     String item= tc.nextToken();
     if( !type.equals(CMD) )
       break;

     EVENTS= item;

     int index= catcon(EVENTS,3);
     if( index < 0 )
     {
       error("EVENTS.CARD(" + EVENTS + ") missing playerNick");
       break;
     }

     String playerNick= EVENTS.substring(index+1);
     String playerFind= dbGet(CMD_PLAYER_FIND, playerNick);
     if( !isFieldValid(playerFind) )
     {
       error("EVENTS.CARD(" + EVENTS + ") playerFind(" + playerFind + ")");
       continue;
     }

     String playerShow= dbGet(CMD_PLAYER_SHOW, playerFind);
     if( !isFieldValid(playerShow) )
     {
       error("EVENTS.CARD(" + EVENTS + ") playerShow(" + playerShow + ")");
       continue;
     }

     // Verify the LD: and CP: fields
     Validator v= new Validator();
     String[] content= tokenize(tc.remainder());
     String section= "XX:";
     for(int i= HOLE_FIELDS; i<content.length; i++)
     {
       String hole= content[i];
       if( hole.indexOf(":") >= 0 )
         section= hole;
       else if( section.equals("CP:") )
       {
         JTextField field= new JTextField(hole);
         String desc= "EVENTS.CARD(" + EVENTS + ")";
         if( !v.isValidCP(desc, field) )
           error(desc + " CP:(" + hole + ")");
       }
       else if( section.equals("LD:") )
       {
         JTextField field= new JTextField(hole);
         String desc= "EVENTS.CARD(" + EVENTS + ")";
         if( !v.isValidLD(desc, field) )
           error("EVENTS.CARD(" + EVENTS + ") LD:(" + hole + ")");
       }
     }

     // Check that the associated EVENTS.POST entry exists
     string= dbGet(CMD_EVENTS_POST, EVENTS);
     if( !isFieldValid(string) )
       error("EVENTS.CARD(" + EVENTS + ") missing EVENTS.POST");
     else
     {
       QuotedTokenizer tp= new QuotedTokenizer(string);
       tp.nextToken();              // Skip ESC
       tp.nextToken();              // Skip rating
       tp.nextToken();              // Skip slope
       String courseName= tp.nextToken(); // Get course name
       String courseTbox= tp.nextToken(); // Get course tbox

       if( !isFieldValid(courseName) || !isFieldValid(courseTbox) )
         error("EVENTS.POST(" + EVENTS + ") invalid course/tbox");
       else
       {
         if( !courseName.equals(content[HOLE_COURSE]) )
           error("EVENTS.POST(" + EVENTS + ") " +
                 "Course(" + courseName + ") != '" + content[HOLE_COURSE] + "'");
         if( !courseTbox.equals(content[HOLE_TEEBOX]) )
           error("EVENTS.POST(" + EVENTS + ") " +
                 "Teebox(" + courseTbox + ") != '" + content[HOLE_TEEBOX] + "'");
       }
     }

     if( errorCount > 10 )
     {
       debug("Too many errors...");
       break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       FgValidator.validate
//
// Purpose-
//       Database validation.
//
//----------------------------------------------------------------------------
public synchronized void
   validate( )                      // Database validator
{
   debug("FgValidator.validate");

   // Validate the database
   validateCourse();
   validatePlayer();
   validateScores();

   // Display error count
   if( errorCount == 0 )
     write("Validation complete, NO errors.");
   else if( errorCount == 1 )
     write("Validation complete, " + errorCount + " error.");
   else
     write("Validation complete, " + errorCount + " errors.");
}
} // class FgValidator
