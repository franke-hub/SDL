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
//       CourseTboxInfo.java
//
// Purpose-
//       Course teebox yardage information container class.
//
// Last change date-
//       2020/01/15
//
// Implementation notes-
//       Note that this class contains rating and slope information but
//       does not display or update it. The update and display functions
//       are performed in CourseView.java by an extention of this class.
//
//----------------------------------------------------------------------------
import java.awt.Color;
import java.awt.event.FocusListener;

import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       CourseTboxInfo
//
// Purpose-
//       Course teebox yardage information container class.
//
//----------------------------------------------------------------------------
public class CourseTboxInfo  extends SigmaHoleInfo {
String                 courseID;    // (OPTIONAL) Course ID
String                 courseShow;  // (OPTIONAL) Course name
String                 color;       // Teebox color
double[]               rating;      // Rating {mens, womens}
String                 stringMR;    // Men's rating
String                 stringMS;    // Men's slope
String                 stringWR;    // Women's rating
String                 stringWS;    // Women's slope

// TBOX input data indexes
protected static final int  TBOX_COURSE_RATE=  0; // Mens rating
protected static final int  TBOX_COURSE_SLP=   1; // Mens slope
protected static final int  TBOX_WOMENS_RATE=  2; // Womens rating
protected static final int  TBOX_WOMENS_SLP=   3; // Womens slope
protected static final int  TBOX_YARDAGE_00=   3; // Yardage[ 0]
///////// static final int  TBOX_YARDAGE_01=   4; // Yardage[ 1]
///////// static final int  TBOX_YARDAGE_02=   5; // Yardage[ 2]
///////// static final int  TBOX_YARDAGE_03=   6; // Yardage[ 3]
///////// static final int  TBOX_YARDAGE_04=   7; // Yardage[ 4]
///////// static final int  TBOX_YARDAGE_05=   8; // Yardage[ 5]
///////// static final int  TBOX_YARDAGE_06=   9; // Yardage[ 6]
///////// static final int  TBOX_YARDAGE_07=  10; // Yardage[ 7]
///////// static final int  TBOX_YARDAGE_08=  11; // Yardage[ 8]
///////// static final int  TBOX_YARDAGE_09=  12; // Yardage[ 9]
///////// static final int  TBOX_YARDAGE_10=  13; // Yardage[10]
///////// static final int  TBOX_YARDAGE_11=  14; // Yardage[11]
///////// static final int  TBOX_YARDAGE_12=  15; // Yardage[12]
///////// static final int  TBOX_YARDAGE_13=  16; // Yardage[13]
///////// static final int  TBOX_YARDAGE_14=  17; // Yardage[14]
///////// static final int  TBOX_YARDAGE_15=  18; // Yardage[15]
///////// static final int  TBOX_YARDAGE_16=  29; // Yardage[16]
///////// static final int  TBOX_YARDAGE_17=  20; // Yardage[17]
///////// static final int  TBOX_YARDAGE_18=  21; // Yardage[18]
protected static final int  TBOX_COLOR_NAME=  22; // Teebox color name

// colorIX data array
static final Color[][] COLOR_BGFG=
{  {new Color(255,255,240),  Color.BLACK} // Default (Ivory)
,  {Color.GRAY,              Color.BLACK} // Black
,  {new Color(128,128,255),  Color.BLACK} // Blue (light)
,  {new Color(255,255,240),  Color.BLACK} // White (Ivory)
,  {Color.LIGHT_GRAY,        Color.BLACK} // Silver
,  {Color.ORANGE,            Color.BLACK} // Gold
,  {Color.YELLOW,            Color.BLACK} // Yellow
,  {Color.GREEN.darker(),    Color.BLACK} // Green
,  {Color.ORANGE.darker(),   Color.BLACK} // Brown
,  {Color.RED,               Color.BLACK} // Red
,  {Color.PINK,              Color.BLACK} // Pink
};

static final String[]  COLOR_NAME=
{  "DEFAULT"
,  "BLACK"
,  "BLUE"
,  "WHITE"
,  "SILVER"
,  "GOLD"
,  "YELLOW"
,  "GREEN"
,  "BROWN"
,  "RED"
,  "PINK"
};

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.DebuggingAdaptor
//
// Purpose-
//       Extend DebuggingAdaptor
//
//----------------------------------------------------------------------------
public void
   debug()
{
   super.debug();
   System.out.println(".color: " + color);
   System.out.println(".rating: " + rating[0] + "," + rating[1]);
   System.out.println(".mens: " + stringMR + "/" + stringMS);
   System.out.println(".womens: " + stringWR + "/" + stringWS);
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.CourseTboxInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   CourseTboxInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            courseID,    // The course name
     String            courseShow,  // The course name
     String            teeboxShow,  // The teebox name (kept in .title)
     String[]          array)       // The hole data array
{
   super(format, defaultString(teeboxShow), courseYardage(array));

   this.courseID= courseID;
   this.courseShow= courseShow;
   color= this.title;
   if( array == null )
   {
     for(int hole= 1; hole<=18; hole++)
       this.array[hole-1]= "400";

     stringMR= "72.0";
     stringMS= "113";
     stringWR= "-";
     stringWS= "-";
   }
   else
   {
     stringMR= array[TBOX_COURSE_RATE];
     stringMS= array[TBOX_COURSE_SLP];
     stringWR= array[TBOX_WOMENS_RATE];
     stringWS= array[TBOX_WOMENS_SLP];
     if( array.length > TBOX_COLOR_NAME )
       color= array[TBOX_COLOR_NAME];
   }

   rating= new double[2];
   updateRating();
}

   CourseTboxInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            courseID,    // The course name
     String            courseShow,  // The course name
     String            teeboxShow,  // The teebox name
     String            string)      // The hole String
{
   this(format, courseID, courseShow, teeboxShow, tokenize(string));
}

   CourseTboxInfo(                  // Constructor
     int               format)      // The HolePanel format
{
   this(format, null, null, null, empty);
}

   CourseTboxInfo(                  // Copy constructor
     CourseTboxInfo    source)      // Source CourseTboxInfo
{
   super(source);

   courseID= source.courseID;
   courseShow= source.courseShow;
   color= source.color;
   rating= new double[2];
   rating[0]= source.rating[0];
   rating[1]= source.rating[1];
   stringMR= source.stringMR;
   stringMS= source.stringMS;
   stringWR= source.stringWR;
   stringWS= source.stringWS;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.colorIX
//
// Purpose-
//       Convert Color name to color index.
//
//----------------------------------------------------------------------------
public static int                   // Associated Color index
   colorIX(                         // Convert Color name to Color index
     String            string)      // The Color name
{
   int result= 0;

   for(int i= 1; i<COLOR_NAME.length; i++)
   {
     if( string.equalsIgnoreCase(COLOR_NAME[i]) )
     {
       result= i;
       break;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.compareTo
//
// Purpose-
//       Implement Comparable<CourseTboxInfo>
//
//----------------------------------------------------------------------------
public int                          // <0, =0, >0
   compareTo(                       // Implement Comparable<CourseTboxInfo>
     CourseTboxInfo    o)           // The other CourseTboxInfo
{
   if( rating[0] == o.rating[0] )
   {
     if( rating[1] < o.rating[1] )
       return (-1);
     else if( rating[1] > o.rating[1] )
       return (+1);
     return 0;
   }

   if( rating[0] < o.rating[0] )
     return (-1);

   return +1;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.courseYardage
//
// Purpose-
//       Hide the course_tbox[] variable.
//
//----------------------------------------------------------------------------
public static String[]              // The course's yardages
   courseYardage(                   // Get course yardages
     String[]          course_tbox) // From teebox data
{
   String[] result= null;

   if( course_tbox != null )
   {
     result= new String[18];
     for(int hole= 1; hole<=18; hole++)
     {
       result[hole-1]= "400";
       if( course_tbox.length > (hole+TBOX_YARDAGE_00) )
         result[hole-1]= course_tbox[hole+TBOX_YARDAGE_00];
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.courseDiff
//
// Purpose-
//       Extract the men's course score differential.
//
//----------------------------------------------------------------------------
public double                       // The Course differential
   courseDiff(                      // Get Course differential
     String            escScore)    // For this score
{
   int score= Integer.parseInt(escScore);
   int slope= Integer.parseInt(stringMS);
   int diff= (Integer.parseInt(escScore)*10) - (int)(rating[0]*10);
   diff *= 113;
// diff += (slope/2);               // If rounding
   diff /= slope;

   return ((double)diff)/10.0;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.courseHdcp
//
// Purpose-
//       Extract the course handicap value.
//
//----------------------------------------------------------------------------
public int                          // The Course handicap
   courseHdcp(                      // Get Course handicap
     String            playerHdcp)  // The Player handicap
{
   return courseHdcp(playerHdcp, stringMS);
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.equals
//
// Purpose-
//       Extend Object.equals()
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff courseID matches
   equals(                          // Test for equality
     Object            o)           // With another Object
{
   boolean result= false;

   if( o instanceof CourseTboxInfo )
   {
     String courseID= ((CourseTboxInfo)o).courseID;
     if( courseID != null && this.courseID != null )
     {
       result= courseID.equals(this.courseID);
       String teeboxID= ((CourseTboxInfo)o).title;
       if( result && teeboxID != null && title != null )
         result= teeboxID.equals(title);
     }
   }

   return result;
}

//----------------------------------------------------------------------------

//
// Method-
//       CourseTboxInfo.genPanel
//
// Purpose-
//       Generate the internal panel, optional FocusListener and NextFocusAdapter
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   panel= null;                     // The panel is conditionally generated
   if( !isRemoved )
   {
     tail= super.genPanel(listener, tail);
     panel.setColor(toColor(color));
   }

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.isValid
//
// Purpose-
//       Validate the panel.
//
// Validators-
//       super()
//       sequence(hole[1..9]), sequence(hole[10..18])
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( !isRemoved && panel != null)
   {
     DataField[] df= panel.getField();
     if( !super.isValid(v) )
       return false;

     int[] ti= toInt();
     for(int hole= 1; hole<=18; hole++)
     {
       int yard= ti[hole-1];
       if( yard < 50 || yard > 700 )
         return v.invalid("Teebox(" + title
                        + ") Hole(" + hole
                        + ") Yard(" + yard
                        + ") range(50..700)");
     }
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.toColor
//
// Purpose-
//       Convert Color name to [BG,FG] Color.
//
//----------------------------------------------------------------------------
public static Color[]               // Resultant teebox color [FG,BG]
   toColor(                         // Initialize for teebox color
     String            string)      // The teebox color
{
   return COLOR_BGFG[colorIX(string)];
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.toString
//
// Purpose-
//       Generate COURSE_TBOX output string
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   String result= null;

   synchronized(this)
   {{{{
     StringBuffer buff= new StringBuffer();
     buff.append(stringMR + " ");
     buff.append(stringMS + " ");
     buff.append(stringWR + " ");
     buff.append(stringWS + " ");
     buff.append(super.toString());
     if( !title.equalsIgnoreCase(color) )
       buff.append(" " + color);
     result= buff.toString();
   }}}} // synchronized(this)

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseTboxInfo.updateRating
//
// Purpose-
//       Update the rating values.
//
//----------------------------------------------------------------------------
protected void
   updateRating( )                  // Update rating values
{
   try {
     rating[0]= 0.0;
     rating[0]= Double.parseDouble(stringMR);
   } catch(Exception e) {
   }

   try {
     rating[1]= 0.0;
     rating[1]= Double.parseDouble(stringWR);
   } catch(Exception e) {
   }
}
} // class CourseTboxInfo
