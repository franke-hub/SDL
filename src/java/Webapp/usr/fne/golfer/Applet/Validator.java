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
//       Validator.java
//
// Purpose-
//       Container for field validation functions.
//
// Last change date-
//       2020/01/15
//
// Usage-
//       Override method invalid, use resultant as input parameter.
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       Validator
//
// Purpose-
//       Container for field validation functions.
//
//----------------------------------------------------------------------------
public class Validator {
//----------------------------------------------------------------------------
//
// Method-
//       Validator.actionPerformed
//
// Purpose-
//       Override this method in your Validator, if used for PostPanel.
//       This method is called when the POST button is depressed.
//
//----------------------------------------------------------------------------
public void
   actionPerformed( )               // POST Button was depressed
{
   System.out.println("actionPerformed()");
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.invalid
//
// Purpose-
//       Display error message.
//       Override this method in your Validator.
//
//----------------------------------------------------------------------------
public boolean                      // FALSE (constant)
   invalid(                         // Invalid data found
     String            message)     // Error message
{
   System.out.println("invalid("+ message + ")");
   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValid
//
// Purpose-
//       Override this method in your Validator, if used for PostPanel.
//       Test the validity of all fields.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff content is valid
   isValid( )                       // Test content validity
{
   System.out.println("isValid()");
   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isBlank
//
// Purpose-
//       Test whether a character is a blank.
//
//----------------------------------------------------------------------------
protected boolean                   // TRUE iff numeric digit
   isBlank(                         // Is character a blank?
     char              C)           // The character
{
   if( C == ' ' || C == '\t' || C == '\n' || C == '\r' )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isNumber
//
// Purpose-
//       Test whether a character is numeric.
//
//----------------------------------------------------------------------------
protected boolean                   // TRUE iff numeric digit
   isNumber(                        // Is character numeric?
     char              digit)       // The character
{
   if( digit >= '0' && digit <= '9' )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.trim
//
// Purpose-
//       Trim leading and trailing blanks from field.
//
//----------------------------------------------------------------------------
public String                       // The resultant String
   trim(                            // Remove leading and trailing blanks
     JTextField        field)       // From this JTextField
{
   String string= field.getText().trim();
   field.setText(string);
   return string;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidCP
//
// Purpose-
//       Validate a closest to the pin field.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   isValidCP(                       // Is CP field valid?
     String            name,        // The field name
     JTextField        field)       // The CP field
{
   if( trim(field).equals("-") )
     return true;

   if( isValidDouble(name, field) ) {
     double result= Double.parseDouble(trim(field));
     if( result >= 0.0 && result <= 100.0 )
       return true;

     return invalid(name + "(" + trim(field) + ") range(0..100)");
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidDate
//
// Purpose-
//       Validate a date field, form MM/DD/YYYY
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   isValidDate(                     // Is date field valid?
     JTextField        field)       // The date field
{
   String string= trim(field);
   if( string.length() != 10
       || !isNumber(string.charAt(0))
       || !isNumber(string.charAt(1))
       || string.charAt(2) != '/'
       || !isNumber(string.charAt(3))
       || !isNumber(string.charAt(4))
       || string.charAt(5) != '/'
       || !isNumber(string.charAt(6))
       || !isNumber(string.charAt(7))
       || !isNumber(string.charAt(8))
       || !isNumber(string.charAt(9)) )
     return invalid("DATE(" + string + ")");

   int mm= Integer.parseInt(string.substring(0,2));
   int dd= Integer.parseInt(string.substring(3,5));

   switch(mm)
   {
     case 1:
     case 3:
     case 5:
     case 7:
     case 8:
     case 10:
     case 12:
       if( dd < 1 || dd > 31 )
         return invalid("DATE(" + string + ")");
       break;

     case 4:
     case 6:
     case 9:
     case 11:
       if( dd < 1 || dd > 30 )
         return invalid("DATE(" + string + ")");
       break;

     case 2:
       if( dd < 1 || dd > 29 )
         return invalid("DATE(" + string + ")");
       break;

     default:
       return invalid("DATE(" + string + ")");
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidDouble
//
// Purpose-
//       Validate a floating point numeric field, which cannot contain blanks.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff floating point field valid
   isValidDouble(                   // Is floating point field valid?
     String            name,        // The field name
     JTextField        field)       // The name field
{
   String string= trim(field);
   try {
     double result= Double.parseDouble(string);
   } catch(NumberFormatException e) {
     return invalid(name + "(" + string + ")");
   } catch(Exception e) {
     e.printStackTrace();
     return invalid(name + "(" + string + ") exception: " + e);
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidHdcp
//
// Purpose-
//       Validate a handicap, specified in tenths.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   isValidHdcp(                     // Is handicap field valid?
     JTextField        field)       // The handicap field
{
   String string= trim(field);
   int L= string.length();          // String length
   int digits= 0;                   // Digit counter
   int dots= 0;                     // Period counter
   int x= 0;                        // Field index

   if( L == 0 )
     return invalid("HDCP(" + string + ") empty");

   if( string.charAt(0) == '-' )
     x= 1;

   while( x < L )
   {
     char C= string.charAt(x++);
     if( C == '.' )
     {
       if( digits == 0 )
         return invalid("HDCP(" + string + ")");

       dots++;
       digits= 0;
     }
     else if( !isNumber(C) )
       return invalid("HDCP(" + string + ")");
     else
       digits++;
   }

   if( digits != 1 && dots != 1 )
     return invalid("HDCP(" + string + ")");

   double hdcp= Double.parseDouble(string);
   if( hdcp < (-18.0) || hdcp > 45 )
     return invalid("HDCP(" + string + ") range(-18 .. 45)" );

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidInteger
//
// Purpose-
//       Validate an integer numeric field, which cannot contain blanks.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff integer field valid
   isValidInteger(                  // Is integer field valid?
     String            name,        // The field name
     JTextField        field)       // The name field
{
   String string= trim(field);
   try {
     int number= Integer.parseInt(string);
   } catch(NumberFormatException e) {
     return invalid(name + "(" + string + ")");
   } catch(Exception e) {
     e.printStackTrace();
     return invalid(name + "(" + string + ") exception: " + e);
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidLD
//
// Purpose-
//       Validate a long drive field.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   isValidLD(                       // Is LD field valid?
     String            name,        // The field name
     JTextField        field)       // The LD field
{
   if( trim(field).equals("-") )
     return true;

   if( isValidInteger(name, field) ) {
     int result= Integer.parseInt(trim(field));
     if( result > 100 && result < 500 )
       return true;

     return invalid(name + "(" + trim(field) + ") range(101..499)");
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidName
//
// Purpose-
//       Validate a name field, which cannot contain blanks.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff name field valid
   isValidName(                     // Is name field valid?
     String            name,        // The field name
     JTextField        field)       // The name field
{
   String string= trim(field);
   if( string.equals("") )
     return invalid(name + " field is required");

   for(int i= 0; i<string.length(); i++)
   {
     if( isBlank(string.charAt(i)) )
       return invalid(name + " field contains blanks");
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidRating
//
// Purpose-
//       Validate a rating
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   isValidRating(                   // Is rating field valid?
     JTextField        field)       // The rating field
{
   String string= trim(field);
   if( string.equals("-") )
     return true;

   if( string.length() != 4
       || ! isNumber(string.charAt(0))
       || ! isNumber(string.charAt(1))
       ||   string.charAt(2) != '.'
       || ! isNumber(string.charAt(3)) )
     return invalid("RATING(" + string + ") format");

   double rating= Double.parseDouble(string);
   if( rating < 52.0 || rating > 92.0 )
     return invalid("RATING(" + string + ") range(52.0 .. 92.0)");

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseEdit.isValidSlope
//
// Purpose-
//       Validate a slope
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   isValidSlope(                    // Is slope field valid?
     JTextField        field)       // The slope field
{
   String string= field.getText();
   if( string.equals("-") )
     return true;

   try {
     int slope= Integer.parseInt(string);
     if( slope < 55 | slope > 155 )
       return invalid("SLOPE(" + string + ") range(55 .. 155)");
   } catch(NumberFormatException e) {
     return invalid("SLOPE(" + string + ")");
   } catch(Exception e) {
     e.printStackTrace();
     return invalid("SLOPE(" + string + ") exception: " + e);
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidText
//
// Purpose-
//       Validate a text field, which cannot be empty.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff text field valid
   isValidText(                     // Is text field valid?
     String            name,        // The field name
     JTextField        field)       // The text field
{
   String string= trim(field);
   if( string.equals("") )
     return invalid(name + " field is required");

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Validator.isValidTime
//
// Purpose-
//       Validate a time field, form hh:mm
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   isValidTime(                     // Is time field valid?
     JTextField        field)       // The time field
{
   String string= trim(field);
   if( string.length() != 5
       || !isNumber(string.charAt(0))
       || !isNumber(string.charAt(1))
       || string.charAt(2) != ':'
       || !isNumber(string.charAt(3))
       || !isNumber(string.charAt(4)) )
     return invalid("TIME(" + string + ")");

   int hh= Integer.parseInt(string.substring(0,2));
   int mm= Integer.parseInt(string.substring(3,5));

   if( hh > 12 || mm > 59 )
     return invalid("TIME(" + string + ")");

   return true;
}
} // class Validator
