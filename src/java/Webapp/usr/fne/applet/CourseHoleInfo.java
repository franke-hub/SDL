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
//       CourseHoleInfo.java
//
// Purpose-
//       Course hole number information container class.
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
//       CourseHoleInfo
//
// Purpose-
//       Course hole number information container class.
//
//----------------------------------------------------------------------------
public class CourseHoleInfo  extends HoleInfo {
//----------------------------------------------------------------------------
//
// Method-
//       CourseHoleInfo.CourseHoleInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   CourseHoleInfo(                  // Constructor
     int               format,      // The HolePanel format
     String[]          array)       // The hole data array
{
   super(format, null, array);

   if( array == null )
   {
     for(int hole= 1; hole<=18; hole++)
       this.array[hole-1]= new String("" + hole);
   }
}

   CourseHoleInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            string)      // The hole String
{
   this(format, tokenize(string));
}

   CourseHoleInfo(                  // Constructor
     int               format)      // The HolePanel format
{
   this(format, empty);
}

   CourseHoleInfo(                  // Copy constructor
     CourseHoleInfo    source)      // Source CourseHoleInfo
{
   super(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseHoleInfo.genPanel
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
   panel= new HolePanel(format, "Hole", array);
   panel.setColor(Color.ORANGE.darker(), Color.BLACK);
   DataField[] df= panel.getField();

   df[HOLE_OUT].setText("Out");
   df[HOLE_IN].setText("In");
   df[HOLE_TOT].setText("Tot");

   if( listener != null )
   {
     for(int hole= 1; hole<=18; hole++)
     {
       df[hole].setEditable(true);
       df[hole].addFocusListener(listener);
     }
   }

   if( tail != null )
   {
     tail.addFocusListener(new NextFocusAdapter(df[1]));
     df[9].addFocusListener(new NextFocusAdapter(df[10]));
     tail= df[18];
   }

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseHoleInfo.isDefault
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
     String string= "" + hole;
     if( !array[hole-1].equals(string) )
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
//       CourseHoleInfo.isValid
//
// Purpose-
//       Validate the panel.
//
// Validators-
//       super()
//       sequence(hole[1..9]), sequence(hole[10..18])
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
     int[] i18= toInt();
     int n= i18[1-1];
     for(int hole= 2; hole<=9; hole++)
     {
       if( i18[hole-1] != (n+1) )
         return v.invalid("Hole[" + hole + "] sequence error");
       n= i18[hole-1];
     }
     n= i18[10-1];
     for(int hole= 11; hole<=18; hole++)
     {
       if( i18[hole-1] != (n+1) )
         return v.invalid("Hole[" + hole + "] sequence error");
       n= i18[hole-1];
     }
   }

   return true;
}
} // class CourseHoleInfo
