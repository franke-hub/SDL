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
//       Bringup.java
//
// Purpose-
//       Bringup hack controls
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
//       Bringup
//
// Purpose-
//       Bringup hack controls
//
// Reference-
//       Bringup
//
//----------------------------------------------------------------------------
public class Bringup extends Debug {// Bringup hack controls
//----------------------------------------------------------------------------
// Bringup.Dynamic attributes
//----------------------------------------------------------------------------
static boolean         hcdm= false; // Hard-Core-Debug-Mode (dynamic)
static int             sequence= 0; // Sequence counter (dynamic)

//----------------------------------------------------------------------------
// Bringup.Static attributes
//----------------------------------------------------------------------------
// This looks wrong but does not change result
// static final boolean correct_insertJoin= true;     // HACK
   static final boolean correct_insertJoin= false;    // SPEC

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
static public void
   debug( )                         // Debugging display
{
   debugln("Bringup.correct_insertJoin: " + correct_insertJoin);
}
} // class Bringup

