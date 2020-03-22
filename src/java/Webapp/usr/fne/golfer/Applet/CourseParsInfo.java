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
//       CourseParsInfo.java
//
// Purpose-
//       Pars information container class.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.Color;
import java.awt.event.FocusListener;

import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       CourseParsInfo
//
// Purpose-
//       Pars information container class.
//
//----------------------------------------------------------------------------
public class CourseParsInfo  extends SigmaHoleInfo {
//----------------------------------------------------------------------------
//
// Method-
//       CourseParsInfo.CourseParsInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   CourseParsInfo(                  // Constructor
     int               format,      // The HolePanel format
     String[]          array)       // The hole data array
{
   super(format, null, array);

   if( array == null )
   {
     for(int hole= 1; hole<=18; hole++)
       this.array[hole-1]= "4";
   }
}

   CourseParsInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            string)      // The hole String
{
   this(format, tokenize(string));
}

   CourseParsInfo(                  // Constructor
     int               format)      // The HolePanel format
{
   this(format, empty);
}

   CourseParsInfo(                  // Copy constructor
     CourseParsInfo    source)      // Source CourseParsInfo
{
   super(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseParsInfo.genPanel
//
// Purpose-
//       Generate the internal panel, optional FocusListener and NextFocusAdapter
//
//----------------------------------------------------------------------------
public synchronized DataField       // Resultant tail DataField
   genPanel(                        // Generate new Panel
     FocusListener     listener,    // (OPTIONAL) FocusListener
     DataField         tail)        // (OPTIONAL) The current tail DataField
{
   // The panel is always generated
   tail= super.genPanel(listener, tail);
   panel.setColor(Color.ORANGE.darker(), Color.BLACK);
   panel.getField(HOLE_ID).setText("Par");

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseParsInfo.isDefault
//
// Purpose-
//       Does the ARRAY contain defaulted values?
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff default
   isDefault( )                     // Is ARRAY default?
{
   boolean result= true;

   for(int hole= 1; hole<=18; hole++)
   {
     if( !array[hole-1].equals("4") )
     {
       result= false;
       break;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseParsInfo.isValid
//
// Purpose-
//       Validate the panel.
//
// Validators-
//       super()
//       range(3 .. 5)
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( !super.isValid(v) )
     return false;

   if( !isRemoved && panel != null)
   {
     int[] par= toInt();
     for(int hole= 1; hole<=18; hole++)
     {
       int p= par[hole-1];
       if( p < 3 || p > 5 )
         return v.invalid("Hole[" + hole + "] PAR(" + p + ") range(3 .. 5)");
     }
   }

   return true;
}
} // class CourseParsInfo
