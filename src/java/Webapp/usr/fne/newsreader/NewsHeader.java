//----------------------------------------------------------------------------
//
//       Copyright (C) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NewsHeader.java
//
// Purpose-
//       Java News Reader: Header descriptor
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
package usr.fne.newsreader;

import java.lang.*;
import java.text.ParseException;
import java.util.*;

import usr.fne.common.*;

public class NewsHeader {
//----------------------------------------------------------------------------
// NewsHeader.Attributes
//----------------------------------------------------------------------------
protected String       ident;       // The associated Message-ID

//----------------------------------------------------------------------------
//
// Method-
//       NewsHeader.NewsHeader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   NewsHeader(                      // Constructor
     String            string)      // The associated Message-ID
{
   ident= string;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsHeader.debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   System.out.println("NewsHeader: '" + ident + "'");
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsHeader.getIdentifier
//
// Purpose-
//       Extract Identifier.
//
//----------------------------------------------------------------------------
public String                       // The associated identifier
   getIdentifier( )                 // Extract Identifier
{
   return ident;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsHeader.extract
//
// Purpose-
//       Extract NewsHeader from a message.
//
//----------------------------------------------------------------------------
public static NewsHeader            // The associated NewsHeader
   extract(                         // Extract NewsHeader
     String            line)        // The associated message line
{
   int                 size= line.length();

   if( size > 12 && line.substring(0,12).equals("Message-ID: ") )
   {
     line= line.substring(12).trim();
     size= line.length();
     if( size > 2 && line.charAt(0) == '<' && line.charAt(size-1) == '>' )
       return new NewsHeader(line);
   }

   return null;
}
} // Class NewsHeader

