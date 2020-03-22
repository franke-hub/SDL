//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       StringFormat.java
//
// Purpose-
//       String formatter.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

import java.lang.String;
import java.lang.StringBuffer;

//----------------------------------------------------------------------------
//
// Class-
//       StringFormat
//
// Purpose-
//       String formatter.
//
//----------------------------------------------------------------------------
public class StringFormat {
//----------------------------------------------------------------------------
// StringFormat.attributes
//----------------------------------------------------------------------------
static String          hextable= "0123456789abcdefghijklmnopqrstuvwxyz";
static String          HEXTABLE= "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

StringBuffer           string;      // Resultant String
int                    radix= 10;   // Numeric radix
String                 table= hextable; // Conversion table

//----------------------------------------------------------------------------
//
// Method-
//       StringFormat.StringFormat
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   StringFormat( )                  // Default constructor
{
   reset();
}

public
   StringFormat(                    // Append constructor
     long              value,       // Number to append
     int               fieldwidth,  // Field width
     int               precision)   // Precision
{
   reset();
   append(value, fieldwidth, precision);
}

public
   StringFormat(                    // Append constructor
     long              value,       // Number to append
     int               fieldwidth)  // Field width
{
   reset();
   append(value, fieldwidth);
}

public
   StringFormat(                    // Append constructor
     long              value)       // Number to append
{
   reset();
   append(value);
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormat.reset
//
// Purpose-
//       Reset the object, restoring defaults.
//
//----------------------------------------------------------------------------
public StringFormat                 // Resultant (this Object)
   reset( )                         // Reset the StringFormat
{
   string= new StringBuffer();      // Restore StringBuffer
   return this;
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormat.setRadix
//
// Purpose-
//       Set converion radix.
//
// Unchecked errors-
//       radix >  table.length()
//       radix <= 1
//
//----------------------------------------------------------------------------
public StringFormat                 // Resultant (this Object)
   setRadix(                        // Set conversion radix
     int               radix,       // Conversion radix
     String            table)       // Conversion table
{
   if( table == null )
     table= hextable;

   this.radix= radix;
   this.table= table;
   return this;
}

public StringFormat                 // Resultant (this Object)
   setRadix(                        // Set conversion radix
     int               radix)       // Conversion radix
{
   setRadix(radix, table);
   return this;
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormat.append
//
// Purpose-
//       Append long.
//
//----------------------------------------------------------------------------
public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     long              value,       // Number to append
     int               fieldwidth,  // Field width
     int               precision)   // Precision
{
   char[]              c= new char[256]; // Working array
   long                t;           // Absolute(value)
   int                 n;           // Number of characters in string

   int                 i;

   t= value;                        // Initialize
   if( (radix&(radix-1)) != 0 )     // If radix not a power of two
   {
     if( value < 0 )
       t= -t;
   }

   n= 0;                            // No resultant
   if( t >= 0 )                     // If positive value
   {
     while( t != 0 )                // Convert to (reversed) string
     {
       i= (int)(t%radix);
       t= t/radix;
       c[n++]= table.charAt(i);
     }
   }
   else                             // If negative (and power of two)
   {
     if( fieldwidth < 8 )
       fieldwidth= 8;
     t++;
     for(;;)                        // Convert to (reversed) string
     {
       i= (int)(t%radix);
       t= t/radix;
       i= radix + i - 1;
       c[n++]= table.charAt(i);
       if( n >= fieldwidth || n > c.length )
         break;
     }
   }

   if( n == 0 )
     c[n++]= '0';

   if( precision < c.length )
   {
     if( value < 0 )
       while( n<(precision-1) )
         c[n++]= '0';
     else
       while( n<precision )
         c[n++]= '0';
   }

   if( value < 0 && (radix&(radix-1)) != 0 )
     c[n++]= '-';

   if( fieldwidth < c.length )
   {
     while( n<fieldwidth )
       c[n++]= ' ';
   }

   while( n > 0 )
   {
     n--;
     string.append(c[n]);
   }

   return this;
}

public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     long              value,       // Number to append
     int               fieldwidth)  // Field width
{
   return append(value, fieldwidth, 0);
}

public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     long              value)       // Number to append
{
   return append(value, 0, 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormat.append
//
// Purpose-
//       Append int.
//
//----------------------------------------------------------------------------
public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     int               value,       // Number to append
     int               fieldwidth,  // Field width
     int               precision)   // Precision
{
   return append((long)value, fieldwidth, precision);
}

public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     int               value,       // Number to append
     int               fieldwidth)  // Field width
{
   return append((long)value, fieldwidth, 0);
}

public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     int               value)       // Number to append
{
   return append((long)value, 0, 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormat.append
//
// Purpose-
//       Append String.
//
//----------------------------------------------------------------------------
public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     String            value,       // String to append
     int               fieldwidth,  // Field width
     int               precision)   // Precision
{
   int                 length= value.length();

   if( fieldwidth >= 0 )
   {
     string.append(value);

     while( fieldwidth > length )
     {
       string.append(' ');
       fieldwidth--;
     }
   }
   else
   {
     fieldwidth= (-fieldwidth);
     while( fieldwidth > length )
     {
       string.append(' ');
       fieldwidth--;
     }

     string.append(value);
   }

   return this;
}

public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     String            value,       // Number to append
     int               fieldwidth)  // Field width
{
   return append(value, fieldwidth, 0);
}

public StringFormat                 // Resultant (this Object)
   append(                          // Append number
     String            value)       // Number to append
{
   return append(value, 0, 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormat.toString
//
// Purpose-
//       Return String resultant.
//
//----------------------------------------------------------------------------
public String                       // The String resultant
   toString( )                      // Get String resultant
{
   return string.toString();
}
} // class StringFormat

