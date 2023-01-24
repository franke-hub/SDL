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
//       WhenDone.java
//
// Purpose-
//       Base class for "when done" function.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       WhenDone
//
// Purpose-
//       Base class for "when done" function.
//
//----------------------------------------------------------------------------
public class WhenDone {
//----------------------------------------------------------------------------
// WhenDone.Attributes
//----------------------------------------------------------------------------
static final boolean   DEBUG= true; // DEBUG control

//----------------------------------------------------------------------------
//
// Method-
//       WhenDone.UTILITIES
//
// Purpose-
//       Utility functions.
//
//----------------------------------------------------------------------------
protected static void debug(String string) {
   if( DEBUG )
     System.out.println("WhenDone: " + string);
}

//----------------------------------------------------------------------------
//
// Method-
//       WhenDone.WhenDone
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   WhenDone( )                      // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       WhenDone.done
//
// Purpose-
//       Override this method in your derived class.
//
//----------------------------------------------------------------------------
public void
   done( )                          // Completion action
{
}
} // class WhenDone
