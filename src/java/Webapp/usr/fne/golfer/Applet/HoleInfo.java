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
//       HoleInfo.java
//
// Purpose-
//       Basic hole information container class.
//
// Last change date-
//       2020/01/15
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
//       HoleInfo
//
// Purpose-
//       Basic hole information container class.
//
//----------------------------------------------------------------------------
public class HoleInfo  extends DatabaseInfo {
String[]               array;       // The hole data array
int                    format;      // The HolePanel format
HolePanel              panel;       // The associated HolePanel
String                 title;       // The hole title

// Hole panel field index
static final int       HOLE_ID=  HolePanel.HOLE_ID;  // Hole title
static final int       HOLE_OUT= HolePanel.HOLE_OUT; // OUT index
static final int       HOLE_IN=  HolePanel.HOLE_IN;  // IN  index
static final int       HOLE_TOT= HolePanel.HOLE_TOT; // TOT index

// Array index constants
static final int       ARRAY_COURSE= 18;
static final int       ARRAY_TEEBOX= 19;

// Data field constants
static final String    FIELD_DP= DataField.FIELD_DP;
static final String    FIELD_ERR= DataField.FIELD_ERR;

// The default hole value
static final String    DEFAULT_VALUE= "0"; // The default hole value

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.HoleInfo
//
// Purpose-
//       Constructors.
//
// Notes-
//       If the array is present and long enough, it is not copied.
//       This allows a single data array to be used to update multiple
//       HoleInfo objects, most notably PlayerCardInfo and PlayerNetsInfo.
//
//----------------------------------------------------------------------------
   HoleInfo(                        // Constructor
     int               format,      // The HolePanel format
     String            title,       // The ID string, iff editable
     String[]          array)       // The hole data array
{
   this.format= format;
   this.array= array;
   this.title= title;

   if( array == null )
     array= new String[0];
   else
     isPresent= true;

   if( array.length < 18 )
   {
     String[] copy= new String[18];
     for(int i= 0; i<18; i++)
     {
       copy[i]= DEFAULT_VALUE;
       if( array.length > i )
         copy[i]= array[i];
     }
     this.array= copy;
   }
}

   HoleInfo(                        // Constructor
     int               format,      // The HolePanel format
     String            title,       // The ID string, iff editable
     String            string)      // The hole String
{
   this(format, title, tokenize(string));
}

   HoleInfo(                        // Copy constructor
     HoleInfo          source)      // Source HoleInfo
{
   synchronized(source)
   {{{{
     isChanged= true;
     array= new String[source.array.length];
     for(int i= 0; i<array.length; i++)
       array[i]= source.array[i];
     this.format= source.format;
     title= source.title;
   }}}} // synchronized(source)
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.debug
//
// Purpose-
//       Extend DebuggingAdaptor.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   super.debug();
   print(".array: " + arrayToQuotedString());
   print(".format: " + format);
   print(".panel: " + (panel != null));
   print(".title: " + title);
   print(".isDefault: " + isDefault());
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.arrayToQuotedString
//
// Purpose-
//       Extract the array elements into space separated quoted String tokens.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   arrayToQuotedString( )           // Convert to quoted String
{
   String result= "";

   synchronized(this)
   {{{{
     result= arrayToQuotedString(array);
   }}}} // synchronized(this)

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.arrayToString
//
// Purpose-
//       Extract the array elements into space separated String tokens.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   arrayToString( )                 // Convert to String
{
   String result= "";

   synchronized(this)
   {{{{
     result= arrayToString(array);
   }}}} // synchronized(this)

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.courseHdcp
//
// Purpose-
//       Extract the course handicap value.
//
//----------------------------------------------------------------------------
public static int                   // The Course handicap
   courseHdcp(                      // Get Course handicap
     String            playerHdcp,  // The Player handicap
     String            courseSlope) // The Course slope
{
   if( playerHdcp == null )
     playerHdcp= "0";

   if( courseSlope == null )
     courseSlope= "113";

   double hdcp=   Double.parseDouble(playerHdcp);
   double slope=  Double.parseDouble(courseSlope);

   return (int)((hdcp*slope)/113.0 + 0.5);
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.defaultString
//
// Purpose-
//       Convert null string to "DEFAULT", leave other strings alone.
//
//----------------------------------------------------------------------------
protected static String             // Resultant String
   defaultString(                   // Get default non-null String
     String            string)      // For this String
{
   if( string == null )
     string= "DEFAULT";

   return string;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.equals
//
// Purpose-
//       Test for equality.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff ARRAY elements are equal
   equals(                          // Test for equality
     Object            o)           // With this Object
{
   boolean result= false;

   synchronized(this)
   {{{{
     if( o instanceof HoleInfo )
     {
       HoleInfo comparand= (HoleInfo)o;
       result= true;
       for(int i= 0; i<18; i++)
       {
         if( !array[i].equals(comparand.array[i]) )
         {
           result= false;
           break;
         }
       }
     }
   }}}}

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.escScore
//
// Purpose-
//       Calculate ESC score for hole.
//
//----------------------------------------------------------------------------
public static int                   // The ESC hole score
   escScore(                        // Get ESC hole score
     int               courseHdcp,  // The player's course handicap
     String            playerCard,  // Carded score for hole
     String            courseHole)  // Par for hole
{
   int strokes= Integer.parseInt(courseHole) * 2;
   if( !playerCard.equalsIgnoreCase(FIELD_DP) )
   {
     try {
       strokes= Integer.parseInt(playerCard);
     } catch(Exception e) {
     }
   }

   // USGA ESC FORMULA
   if( courseHdcp <= 9 )
     strokes= Math.min(strokes, Integer.parseInt(courseHole) + 2);
   else if( courseHdcp <= 19 )
     strokes= Math.min(strokes, 7);
   else if( courseHdcp <= 29 )
     strokes= Math.min(strokes, 8);
   else if( courseHdcp <= 39 )
     strokes= Math.min(strokes, 9);
   else
     strokes= Math.min(strokes, 10);

   return strokes;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.fieldColor
//
// Purpose-
//       Return color based on data value.
//
//----------------------------------------------------------------------------
public static Color                 // Resultant Color
   fieldColor(                      // Get Color for
     int               value)       // This value
{
   return HolePanel.fieldColor(value);
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.genPanel
//
// Purpose-
//       Generate the internal PANEL, optional FocusListener and NextFocusAdapter
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   // The panel is always generated
   panel= new HolePanel(format, title, array);
   DataField[] df= panel.getField();
   if( listener != null )
   {
     if( title != null )
     {
       df[HOLE_ID].addFocusListener(listener);
       df[HOLE_ID].setEditable(true);
     }

     for(int hole= 1; hole<=18; hole++)
     {
       df[hole].addFocusListener(listener);
       df[hole].setEditable(true);
     }
   }

   if( tail != null )
   {
     tail.addFocusListener(new NextFocusAdapter(title != null ? df[HOLE_ID] : df[1]));
     df[9].addFocusListener(new NextFocusAdapter(df[10]));
     tail= df[18];
   }

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.getCourseID
//       HoleInfo.getTeeboxID
//
// Purpose-
//       Extract the COURSE_ID
//       Extract the TEEBOX_ID
//
//----------------------------------------------------------------------------
public static String                // The COURSE_ID
   getCourseID(                     // Get COURSE_ID
     String[]          array)       // From this array
{
   return (array.length > ARRAY_COURSE ? array[ARRAY_COURSE] : null);
}

public String                       // The COURSE_ID
   getCourseID( )                   // Get COURSE_ID
{
   return getCourseID(array);
}

public static String                // The TEEBOX_ID
   getTeeboxID(                     // Get TEEBOX_ID
     String[]          array)       // From this array
{
   return (array.length > ARRAY_TEEBOX ? array[ARRAY_TEEBOX] : null);
}

public String                       // The TEEBOX_ID
   getTeeboxID( )                   // Get TEEBOX_ID
{
   return getTeeboxID(array);
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.getIndex
//       HoleInfo.getLDO
//       HoleInfo.getLDI
//       HoleInfo.getCPO
//       HoleInfo.getCPI
//       HoleInfo.getFWH
//       HoleInfo.getGIR
//
// Purpose-
//       Internal array element accessors.
//
//----------------------------------------------------------------------------
private int                         // The array element index
   getIndex(                        // Get array element index
     String            string)      // For this element value
{
   for(int i= 18; i<array.length; i++)
   {
     if( string.equalsIgnoreCase(array[i]) )
       return i;
   }

   return (-1);
}

public String                       // The LDO array element
   getLDO( )                        // Get LDO array element
{
   int index= getIndex("LD:");
   if( index > 0 && array.length > (index+1) )
     return array[index+1];

   return "-";
}

public String                       // The LDI array element
   getLDI( )                        // Get LDI array element
{
   int index= getIndex("LD:");
   if( index > 0 && array.length > (index+2) )
     return array[index+2];

   return "-";
}

public String                       // The CPO array element
   getCPO( )                        // Get CPO array element
{
   int index= getIndex("CP:");
   if( index > 0 && array.length > (index+1) )
     return array[index+1];

   return "-";
}

public String                       // The CPI array element
   getCPI( )                        // Get CPI array element
{
   int index= getIndex("CP:");
   if( index > 0 && array.length > (index+2) )
     return array[index+2];

   return "-";
}

public String                       // The FWH array element
   getFWH( )                        // Get FWH array element
{
   int index= getIndex("FWH:");
   if( index > 0 && array.length > (index+1) )
     return array[index+1];

   return "-";
}

public String                       // The GIR array element
   getGIR( )                        // Get GIR array element
{
   int index= getIndex("GIR:");
   if( index > 0 && array.length > (index+1) )
     return array[index+1];

   return "-";
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.getPanel
//
// Purpose-
//       Access the internal PANEL
//
//----------------------------------------------------------------------------
public DataPanel                    // The current DataPanel
   getPanel( )                      // Get current DataPanel
{
   return panel;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.isDefault
//
// Purpose-
//       Does the ARRAY contain defaulted values?
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff default
   isDefault( )                     // Is ARRAY default?
{
   boolean result= true;

   for(int i= 0; i<array.length; i++)
   {
     if( !array[i].equals(DEFAULT_VALUE) )
     {
       result= false;
       break;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.isValid
//
// Purpose-
//       Validate the PANEL.
//
// Validators-
//       TITLE, if present, must contain a valid name.
//       PANEL fields must contain numeric values.
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is PANEL valid
     Validator         v)           // Validator
{
   if( !isRemoved && panel != null)
   {
     DataField df[]= panel.getField();
     if( title != null && !v.isValidName("Name", df[HOLE_ID]) )
       return false;

     for(int hole= 1; hole<=18; hole++)
     {
       String string= "Hole[" + hole + "]"; // Error string
       if( !v.isValidName(string, df[hole]) )
         return false;
       try {
         df[hole].intValue();
       } catch(Exception e) {
         return v.invalid(string + " " + e);
       }
     }
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.expand
//       HoleInfo.setLDO
//       HoleInfo.setLDI
//       HoleInfo.setCPO
//       HoleInfo.setCPI
//       HoleInfo.setFWH
//       HoleInfo.setGIR
//
// Purpose-
//       Internal array element accessors.
//
//----------------------------------------------------------------------------
private int                         // The array element index
   expand(                          // Get array element index
     String            string,      // For this element value
     int               count)       // And this element count
{
   int result= (-1);
   for(int i= 18; i<array.length; i++)
   {
     if( string.equalsIgnoreCase(array[i]) )
     {
       result= i;
       break;
     }
   }

   if( result < 0 || array.length < (result+count) )
   {
     int length= array.length + count + 1;
     if( result > 0 )
       length= result + count;

     String[] copy= new String[length];
     for(int i= 0; i<array.length; i++)
       copy[i]= array[i];

     if( result < 0 )
     {
       copy[array.length]= string;
       result= array.length;
     }

     for(int i= result+1; i<copy.length; i++)
       copy[i]= "-";

     array= copy;
   }

   return result;
}

public synchronized void
   setLDO(                          // Set LDO array element
     String            string)      // To this String
{
   int index= expand("LD:", 2);
   array[index+1]= string;
}

public synchronized void
   setLDI(                          // Set LDI array element
     String            string)      // To this String
{
   int index= expand("LD:", 2);
   array[index+2]= string;
}

public synchronized void
   setCPO(                          // Set CPO array element
     String            string)      // To this String
{
   int index= expand("CP:", 2);
   array[index+1]= string;
}

public synchronized void
   setCPI(                          // Set CPI array element
     String            string)      // To this String
{
   int index= expand("CP:", 2);
   array[index+2]= string;
}

public synchronized void
   setFWH(                          // Set FWH array element
     String            string)      // To this String
{
   int index= expand("FWH:", 1);
   array[index+1]= string;
}

public synchronized void
   setGIR(                          // Set GIR array element
     String            string)      // To this String
{
   int index= expand("GIR:", 1);
   array[index+1]= string;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.toInt
//
// Purpose-
//       Extract the PANEL into int[18]
//
//----------------------------------------------------------------------------
public synchronized int[]           // Resultant int[18]
   toInt( )                         // Convert to int[18]
{
   int[] result= null;

   synchronized(this)
   {{{{
     if( panel != null )
     {
       DataField[] df= panel.getField();
       try {
         result= new int[18];
         for(int hole= 1; hole<=18; hole++)
           result[hole-1]= Integer.parseInt(df[hole].getText());
       } catch(NumberFormatException e) {
         result= null;
       } catch(Exception e) {
         System.out.println("HoleInfo.toInt() Exception: " + e);
         e.printStackTrace();
         result= null;
       }
     }
   }}}} // synchronized(this)

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.toString
//
// Purpose-
//       Extract the ARRAY into space separated String objects.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   String result= null;

   synchronized(this)
   {{{{
     StringBuffer buff= new StringBuffer();
     for(int hole= 1; hole<=18; hole++)
       buff.append((hole == 1 ? "" : " ") + array[hole-1]);
     result= buff.toString();
   }}}} // synchronized(this)

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HoleInfo.update
//
// Purpose-
//       Update the internal fields, setting isChanged as appropriate.
//
//----------------------------------------------------------------------------
public synchronized void
   update( )                        // Update from PANEL
{
   if( panel != null )
   {
     DataField df[]= panel.getField();
     if( title != null && !title.equalsIgnoreCase(df[HOLE_ID].getText()) )
     {
       isChanged= true;
       title= df[HOLE_ID].getText();
     }

     for(int hole= 1; hole<=18; hole++)
     {
       if( !array[hole-1].equals(df[hole].getText()) )
       {
         isChanged= true;
         array[hole-1]= df[hole].getText();
       }
     }
   }
}
} // class HoleInfo
