//----------------------------------------------------------------------------
//
//       Copyright (C) 2008 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       QuotedTokenizer.java
//
// Purpose-
//       Similar to StringTokenizer, but quoted strings are recognized.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
package usr.fne.common;

import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       QuotedTokenizer
//
// Purpose-
//       Utility QuotedTokenizer.
//
//----------------------------------------------------------------------------
public class QuotedTokenizer
{
//----------------------------------------------------------------------------
// QuotedTokenizer.Attributes
//----------------------------------------------------------------------------
String                 string;      // The parse String
int                    offset;      // Current offset
int                    length;      // The String length

//----------------------------------------------------------------------------
//
// Method-
//       QuotedTokenizer.QuotedTokenizer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   QuotedTokenizer(                 // Constructor
     String            string)      // The source String
{
   this.string= string;
   length= string.length();
   offset= 0;
   skipBlanks();
}

//----------------------------------------------------------------------------
//
// Method-
//       QuotedTokenizer.skipBlanks
//
// Purpose-
//       Skip over whitespace.
//
//----------------------------------------------------------------------------
protected void
   skipBlanks( )                    // Skip over whitespace
{
   while( offset < length )
   {
     if( !Character.isWhitespace(string.charAt(offset)) )
       break;

     offset++;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       QuotedTokenizer.hasMoreTokens
//
// Purpose-
//       Determine whether the String has more tokens.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff nextToken() valid
   hasMoreTokens( )                 // Are more tokens available?
{
   return (offset < length);
}

//----------------------------------------------------------------------------
//
// Method-
//       QuotedTokenizer.nextToken
//
// Purpose-
//       Return the next token.
//
//----------------------------------------------------------------------------
public String                       // The next token
   nextToken( )                     // Return the next token
   throws NoSuchElementException
{
   if( offset >= length )
     throw new NoSuchElementException();

   char delim= ' ';
   char C= string.charAt(offset);
   if( C == '\"' || C == '\'' )
     delim= C;

   int first= offset;
   offset++;
   while( offset < length )
   {
     if( string.charAt(offset) == delim )
       break;

     offset++;
   }
   if( delim != ' ' && offset < length )
     offset++;

   int index= offset;
   skipBlanks();

   return string.substring(first, index);
}

//----------------------------------------------------------------------------
//
// Method-
//       QuotedTokenizer.remainder
//
// Purpose-
//       Return the remainder of the input String.
//
//----------------------------------------------------------------------------
public String                       // The String remainder
   remainder( )                     // Get String remainder
{
   return string.substring(offset);
}
} // class QuotedTokenizer

