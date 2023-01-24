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
//       StaticJApplet.java
//
// Purpose-
//       Common static applet information.
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
//       StaticJApplet
//
// Purpose-
//       GUI static definitions.
//
//----------------------------------------------------------------------------
public class StaticJApplet extends JApplet {
//----------------------------------------------------------------------------
// StaticJApplet.Attributes
//----------------------------------------------------------------------------
// Command lookup types
static final String    CMD_COMMENT=     "##";
static final String    CMD_DEFAULT=     "DEFAULT";
static final String    CMD_COURSE_FIND= "COURSE.FIND";
static final String    CMD_EVENTS_FIND= "EVENTS.FIND";
static final String    CMD_PLAYER_FIND= "PLAYER.FIND";

// Command types requiring associated identifier
static final String    CMD_COURSE_HDCP= "COURSE.HDCP";
static final String    CMD_COURSE_HOLE= "COURSE.HOLE";
static final String    CMD_COURSE_HTTP= "COURSE.HTTP";
static final String    CMD_COURSE_LONG= "COURSE.LONG";
static final String    CMD_COURSE_NAME= "COURSE.NAME";
static final String    CMD_COURSE_NEAR= "COURSE.NEAR";
static final String    CMD_COURSE_PARS= "COURSE.PARS";
static final String    CMD_COURSE_SHOW= "COURSE.SHOW";
static final String    CMD_COURSE_TBOX= "COURSE.TBOX";

static final String    CMD_EVENTS_CARD= "EVENTS.CARD";
static final String    CMD_EVENTS_DATE= "EVENTS.DATE";
static final String    CMD_EVENTS_HDCP= "EVENTS.HDCP";
static final String    CMD_EVENTS_LONG= "EVENTS.LONG";
static final String    CMD_EVENTS_NAME= "EVENTS.NAME";
static final String    CMD_EVENTS_NEAR= "EVENTS.NEAR";
static final String    CMD_EVENTS_PLAY= "EVENTS.PLAY";
static final String    CMD_EVENTS_POST= "EVENTS.POST";
static final String    CMD_EVENTS_SHOW= "EVENTS.SHOW";
static final String    CMD_EVENTS_TEAM= "EVENTS.TEAM";
static final String    CMD_EVENTS_TIME= "EVENTS.TIME";

static final String    CMD_PLAYER_CARD= "PLAYER.CARD";
static final String    CMD_PLAYER_HDCP= "PLAYER.HDCP";
static final String    CMD_PLAYER_MAIL= "PLAYER.MAIL";
static final String    CMD_PLAYER_NAME= "PLAYER.NAME";
static final String    CMD_PLAYER_NICK= "PLAYER.NICK";
static final String    CMD_PLAYER_POST= "PLAYER.POST";
static final String    CMD_PLAYER_SHOW= "PLAYER.SHOW";

// Command CMD_DEFAULT codes
static final String    DEFAULT_CODE_DT= "DATE_TIME";
static final String    DEFAULT_CODE_CI= "COURSE_ID";
static final String    DEFAULT_CODE_TI= "TEEBOX_ID";
static final String    DEFAULT_CODE_EI= "EVENTS_ID";
static final String    DEFAULT_CODE_PI= "PLAYER_ID";

// Field constants
static final String    FIELD_DP=  DataField.FIELD_DP; // Double par data entry
static final String    FIELD_ERR= DataField.FIELD_ERR; // Field error

// Format constants
static final int       FORMAT_USER= HolePanel.FORMAT_USER;
static final int       FORMAT_BASE= HolePanel.FORMAT_BASE;
static final int       FORMAT_EVNT= HolePanel.FORMAT_EVNT;
static final int       FORMAT_SKIN= HolePanel.FORMAT_SKIN;

// Hole constants
static final int       HOLE_ID=  HolePanel.HOLE_ID;  // Hole title
static final int       HOLE_OUT= HolePanel.HOLE_OUT; // OUT index
static final int       HOLE_IN=  HolePanel.HOLE_IN;  // IN  index
static final int       HOLE_TOT= HolePanel.HOLE_TOT; // TOT index
static final int       HOLE_ESC= HolePanel.HOLE_ESC; // ESC index
static final int       HOLE_HCP= HolePanel.HOLE_HCP; // HCP index
static final int       HOLE_NET= HolePanel.HOLE_NET; // NET index

static final int       HOLE_LDO= HolePanel.HOLE_LDO; // LDO index (Longest Drive OUT)
static final int       HOLE_CPO= HolePanel.HOLE_CPO; // CPO index (Closest to Pin OUT)
static final int       HOLE_LDI= HolePanel.HOLE_LDI; // LDI index (Longest Drive IN)
static final int       HOLE_CPI= HolePanel.HOLE_CPI; // CPI index (Closest to Pin IN)

static final int       HOLE_FWH= HolePanel.HOLE_FWH; // FWH index (Fairways hit)
static final int       HOLE_GIR= HolePanel.HOLE_GIR; // GIR index (Greens in Regulation)

static final int       HOLE_SKIN= HolePanel.HOLE_SKIN; // SKINS index

// Utility constants
static final String[]  empty= null; // Differentiates String and String[] null

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
public static int                   // The concatenator index
   catcon(                          // Inverse concatenate
     String            item,        // The item
     int               index)       // The item index
{
   return DbStatic.catcon(item, index);
}

public static int                   // The concatenator index
   catcon(                          // Inverse concatenate
     String            item)        // The item
{
   return DbStatic.catcon(item);
}

public static String                // The concatenated String
   concat(                          // Concatenate
     String            item,        // The item
     String            qual)        // The qualifier
{
   return DbStatic.concat(item, qual);
}

public static double                // ABSOLUTE(inp)
   fabs(                            // Return absolute value
     double            inp)         // For this value
{
   if( inp < 0 )
     inp= (-inp);

   return inp;
}

public static int
   max(int L, int R)
{
   if( L > R )
     R= L;
   return R;
}

public static int
   min(int L, int R)
{
   if( L < R )
     R= L;
   return R;
}

public static String                // Resultant
   stripQuotes(                     // Strip quotes
     String            string)      // From this String
{
   return DbStatic.stripQuotes(string);
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

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.fieldColor
//
// Purpose-
//       Return color based on data value.
//
//----------------------------------------------------------------------------
public static Color                 // Resultant Color
   fieldColor(                      // Get Color for
     int               value)       // This value
{
   Color result= Color.BLACK;
   if( value < 0 )
     result= Color.RED;
   else if( value == 0 )
     result= Color.GREEN.darker();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.fixDate
//
// Purpose-
//       Repair a date field.
//
//----------------------------------------------------------------------------
public static String                // Date (Format MM/DD/YYYY)
   fixDate(                         // Fix a date string
     String            date)        // Date (Format M/D/YY)
{
   int index1= date.indexOf('/');
   if( index1 < 0 )
     return "12/31/9999";

   int index2= date.substring(index1+1).indexOf('/');
   if( index2 < 0 )
     return "12/31/9999";

   index2 += index1 + 1;
   String mm= date.substring(0, index1);
   String dd= date.substring(index1+1, index2);
   String yyyy= date.substring(index2+1);

   if( yyyy.length() == 2 )
     yyyy= "20" + yyyy;

   while( mm.length() < 2 )
     mm= "0" + mm;

   while( dd.length() < 2 )
     dd= "0" + dd;

   while( yyyy.length() < 4 )
     yyyy= "0" + yyyy;

   while( mm.length() > 2 )
     mm= mm.substring(1);

   while( dd.length() > 2 )
     dd= dd.substring(1);

   while( yyyy.length() > 4 )
     yyyy= yyyy.substring(1);

   return mm + "/" + dd + "/" + yyyy;
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.fixTime
//
// Purpose-
//       Repair a time field
//
//----------------------------------------------------------------------------
public static String                // Date (Format HH:MM)
   fixTime(                         // Fix a time string
     String            time)        // Time (Format H:M)
{
   int index1= time.indexOf(':');
   if( index1 < 0 )
     return "00:00";

   int index2= time.substring(index1+1).indexOf(':');
   if( index2 > 0 )
     time= time.substring(0, index1 + index2 + 1);;

   String hh= time.substring(0, index1);
   String mm= time.substring(index1+1);

   while( hh.length() < 2 )
     hh= "0" + hh;

   while( mm.length() < 2 )
     mm= "0" + mm;

   while( hh.length() > 2 )
     hh= hh.substring(1);

   while( mm.length() > 2 )
     mm= mm.substring(1);

   return hh + ":" + mm;
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.showDate
//
// Purpose-
//       Convert a SortDate into a ShowDate.
//
//----------------------------------------------------------------------------
public static String                // Date (Format MM/DD/YYYY)
   showDate(                        // SortDate => ShowDate
     String            date)        // Date (Format YYYY/MM/DD)
{
   if( date.length() < 10 )
     return date;

   String head= date.substring(0,10);
   String tail= "";
   if( date.length() > 10 )
     tail= date.substring(10);

   return head.substring(5) + "/" + head.substring(0,4) + tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       StaticJApplet.sortDate
//
// Purpose-
//       Convert a ShowDate into a SortDate.
//
//----------------------------------------------------------------------------
public static String                // Date (Format YYYY/MM/DD)
   sortDate(                        // ShowDate => SortDate
     String            date)        // Date (Format MM/DD/YYYY)
{
   if( date.length() < 10 )
     return date;

   String head= date.substring(0,10);
   String tail= "";
   if( date.length() > 10 )
     tail= date.substring(10);

   return head.substring(6) + "/" + head.substring(0,5) + tail;
}
} // class StaticJApplet
