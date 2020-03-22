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
//       Tokenizer.java
//
// Purpose-
//       Break String into space separated substrings.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Tokenizer
//
// Purpose-
//       Break String into space separated substrings.
//
//----------------------------------------------------------------------------
class Tokenizer {                    // Tokenizer
//----------------------------------------------------------------------------
// Tokenizer.Attributes
//----------------------------------------------------------------------------
String                 string;      // The String
int                    index;       // The String index

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer.Tokenizer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Tokenizer(                       // Constructor
     String            string)      // String to tokenize
{
   this.string= string;
   this.index= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer.get
//
// Purpose-
//       Extract next substring Token.
//
//----------------------------------------------------------------------------
public String                       // The next Token, null iff none
   get( )                           // Get next Token
{
   int                 start;       // Start index

   if( string == null )
     return null;

   while( index < string.length() && string.charAt(index) == ' ' )
     index++;

   if( index >= string.length() )
     return null;

   start= index++;
   while( index < string.length() && string.charAt(index) != ' ' )
     index++;

   return string.substring(start,index);
}
} // class Tokenizer

