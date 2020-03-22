//----------------------------------------------------------------------------
//
//       Copyright (C) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Diagnostic.java
//
// Purpose-
//       Diagnostic Memory.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Diagnostic
//
// Purpose-
//       Diagnostic Memory.
//
// Reference-
//       (None: Added for debugging)
//
//----------------------------------------------------------------------------
class Diagnostic extends Debug {    // Diagnostic Memory
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//
// Method-
//       Diagnostic.Diagnostic
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   Diagnostic( )                    // Default constructor
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Diagnostic.diag
//
// Purpose-
//       Diagnose a memory object
//
//----------------------------------------------------------------------------
public static void
   diag(                            // Diagnose an object
     String            s,           // Optional message
     Object            o)           // The object in question
{
   debugln("Diagnostic.diag(" + s + "," + o + ")");
   DebugMap map= new DebugMap();    // Display allocated memory

   if( s == null )
     s= "Object: ";

   map.debug(s, o);
   if( o instanceof List )
   {
     List list= (List)o;
     int x= 0;
     for(Iterator i= list.iterator(); i.hasNext();)
     {
       Object item= i.next();
       map.debug("[" + x + "]: ", item);
       x++;
     }
   }

   map.unwind(false);
}

public static void
   diag(                            // Diagnose an object
     Object            o)           // The object in question
{
   diag("Object: ", o);
}
} // class Diagnostic

