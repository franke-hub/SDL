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
//       NewsGroup.java
//
// Purpose-
//       Java News Reader: Group descriptor
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
package usr.fne.newsreader;

import java.lang.*;
import java.util.*;

import usr.fne.common.*;

public class NewsGroup {
//----------------------------------------------------------------------------
// NewsGroup.Attributes
//----------------------------------------------------------------------------
protected boolean      changed;     // TRUE iff last changed
protected String       name;        // The name of the NewsGroup
protected int          last;        // Last  Article number read

//----------------------------------------------------------------------------
//
// Method-
//       NewsGroup.NewsGroup
//
// Purpose-
//       Constructor.
//
// Usage-
//       The group descriptor is in either of two formats:
//         a) GroupName
//         b) GroupName last
//
//       Where
//         GroupName is the name of the news group.
//         last is the last known group message number.
//
//----------------------------------------------------------------------------
public
   NewsGroup(                       // Constructor
     String            group)       // Group descriptor
{
   name=    "";
   changed= false;
   last=    0;

   try {
     StringTokenizer st= new StringTokenizer(group);
     name= st.nextToken();
     if( st.hasMoreTokens() )
       last= Integer.parseInt(st.nextToken());
   } catch( Exception e ) {
     System.err.println("Malformed Group: '" + group + "'");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsGroup.Accessors
//
// Purpose-
//       Accessors.
//
//----------------------------------------------------------------------------
public String                       // The NAME attribute
   getName( )                       // Get NAME attribute
{
   return name;
}

public boolean                      // The CHANGED attribute
   getChanged( )                    // Get CHANGED attribute
{
   return changed;
}

public int                          // The LAST (article number) attribute
   getLast( )                       // Get LAST (article number) attribute
{
   return last;
}

public void
   setLast(                         // Set LAST (article number) attribute
     int               last)        // The LAST (article number) attribute
{
   changed= true;
   this.last= last;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsGroup.toString
//
// Purpose-
//       Display String.
//
//----------------------------------------------------------------------------
public String
   toString( )                      // Display String
{
   return name  + " " + last;
}
} // Class NewsGroup

