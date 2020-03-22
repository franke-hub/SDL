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
//       DbStatic.java
//
// Purpose-
//       Golfer Database Common static attributes.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.util.Vector;
import usr.fne.common.QuotedTokenizer;

public class DbStatic extends DebuggingAdaptor {
//----------------------------------------------------------------------------
// DbStatic.Attributes
//----------------------------------------------------------------------------
static final String    VERSION= "DbServer 1.0 2008.05.01";
static final int       PORT= 0x0000fe01; // Listener port number

// Commands
static final String    CMD_GET=         "GET";
static final String    CMD_NEXT=        "NEXT";
static final String    CMD_PUT=         "PUT";
static final String    CMD_REMOVE=      "REM";
static final String    CMD_NULL=        "<NULL>";

static final String    COMMENT=         "##";

// The separator character
static final char      SEP= '@';    // Separator character

// Thread states
static final int       FSM_RESET= 0;// Reset, not initialized
static final int       FSM_READY= 1;// Running
static final int       FSM_FINAL= 2;// Terminated
static final int       FSM_ERROR= 3;// Terminated in error

//----------------------------------------------------------------------------
//
// Method-
//       DbStatic.addQuotes
//
// Purpose-
//       Convert a String into a quoted String
//
//----------------------------------------------------------------------------
protected static String             // Resultant
   addQuotes(                       // Add quotes
     String            string)      // To this String
{
   return "\"" + string + "\"";
}

//----------------------------------------------------------------------------
//
// Method-
//       DbStatic.catcon
//
// Purpose-
//       Invert concatenation of an item.qualifier
//
//----------------------------------------------------------------------------
public static int                   // The concatenation index
   catcon(                          // Deconcatenate
     String            itemqual,    // The item.qualifier
     int               index)       // The qualifier number
{
   int offset= itemqual.indexOf('.');
   if( offset < 0 )
     return offset;

   for(--index; index > 0; --index)
   {
     offset++;
     int next= itemqual.substring(offset).indexOf('.');
     if( next < 0 )
       return next;

     offset += next;
   }

   return offset;
}

public static int                   // The concatenation index
   catcon(                          // Deconcatenate
     String            itemqual)    // The item.qualifier
{
   return itemqual.indexOf('.');
}

//----------------------------------------------------------------------------
//
// Method-
//       DbStatic.concat
//
// Purpose-
//       Concatenate an item and a qualifier.
//
//----------------------------------------------------------------------------
public static String                // The concatenated String
   concat(                          // Concatenate
     String            item,        // The item
     String            qual)        // The qualifier
{
   return item + '.' + qual;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbStatic.stripQuotes
//
// Purpose-
//       Remove quotes from a String.
//
//----------------------------------------------------------------------------
public static String                // Resultant
   stripQuotes(                     // Strip quotes
     String            string)      // From this String
{
   if( string == null )
     return null;

   int length= string.length();
   if( length < 2 )
     return null;

   if( string.charAt(0) == '\"' && string.charAt(length-1) == '\"' )
     return string.substring(1, length-1);

   return string;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbStatic.tokenize
//
// Purpose-
//       Tokenize a String.
//
//----------------------------------------------------------------------------
public static String[]              // Resultant String[]
   tokenize(                        // Extract tokens from
     String            string)      // This String
{
   QuotedTokenizer     tokenizer= new QuotedTokenizer(string);
   Vector<String>      vector= new Vector<String>();

   while( tokenizer.hasMoreTokens() )
   {
     String token= tokenizer.nextToken();
     if( token == null )
       break;

     // Remove quotes from quoted strings
     int length= token.length();
     if( length > 1
         && token.charAt(0) == '\"' && token.charAt(length-1) == '\"' )
       token= token.substring(1, length-1);

     vector.add(token);
   }

   // Vector.toArray() does not do enough
   String[] result= new String[vector.size()];
   for(int i= 0; i<result.length; i++)
     result[i]= vector.elementAt(i);

   return result;
}
} // class DbStatic
