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
//       DebuggingAdaptor.java
//
// Purpose-
//       Debugging methods.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       DebuggingAdaptor
//
// Purpose-
//       Debugging methods.
//
//----------------------------------------------------------------------------
public class DebuggingAdaptor implements DebuggingInterface {
//----------------------------------------------------------------------------
//
// Method-
//       DebuggingAdaptor.print
//
// Purpose-
//       Unconditionally write message to stdout.
//
//----------------------------------------------------------------------------
public void
   print(                           // Debugging message
     String            string)      // The message String
{
   System.out.println(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingAdaptor.debug(String)
//
// Purpose-
//       Conditionally write message to stdout.
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging message
     String            string)      // The message String
{
   if( isDebug() )
     print(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingAdaptor.debug( void )
//
// Purpose-
//       Unconditionally print object debugging message.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Object debugging message
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingAdaptor.error
//
// Purpose-
//       Unconditionally write message to stderr.
//
//----------------------------------------------------------------------------
public void
   error(                           // Debugging error message
     String            string)      // The message String
{
   System.err.println(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingAdaptor.isDebug
//
// Purpose-
//       Test: Is debugging active?
//       Override this method in subclasses.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff debug should write
   isDebug( )                       // Is debugging active?
{
   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingAdaptor.verify
//
// Purpose-
//       Throw RuntimeException if condition is false.
//
//----------------------------------------------------------------------------
public void
   verify(                          // Verify
     boolean           condition)   // RuntimeException iff condition == false
{
   if( condition )
     return;

   throw new RuntimeException("VERIFY failed");
}

public void
   verify(                          // Verify
     boolean           condition,   // RuntimeException iff condition == false
     String            text)        // Diagnostic message
{
   if( condition )
     return;

   throw new RuntimeException("VERIFY failed: " + text);
}
} // class DebuggingAdaptor
