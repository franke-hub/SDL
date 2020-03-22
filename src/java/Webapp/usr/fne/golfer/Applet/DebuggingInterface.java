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
//       DebuggingInterface.java
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
//       DebuggingInterface
//
// Purpose-
//       Debugging methods.
//
//----------------------------------------------------------------------------
public interface DebuggingInterface {
//----------------------------------------------------------------------------
//
// Method-
//       DebuggingInterface.print
//
// Purpose-
//       Unconditionally write message to stdout.
//
//----------------------------------------------------------------------------
public void
   print(                           // Debugging message
     String            string);     // The message String

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingInterface.debug(String)
//
// Purpose-
//       Conditionally write message to stdout.
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging message
     String            string);     // The message String

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingInterface.debug( void )
//
// Purpose-
//       Unconditionally print object debugging message.
//
//----------------------------------------------------------------------------
public void
   debug( );                        // Object debugging message

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingInterface.error
//
// Purpose-
//       Unconditionally write message to stderr.
//
//----------------------------------------------------------------------------
public void
   error(                           // Debugging error message
     String            string);     // The message String

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingInterface.isDebug
//
// Purpose-
//       Test: Is debugging active?
//       Override this method in subclasses.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff debug should write
   isDebug( );                      // Is debugging active
} // class DebuggingInterface
