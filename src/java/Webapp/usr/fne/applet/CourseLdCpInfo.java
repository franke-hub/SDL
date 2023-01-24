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
//       CourseLdCpInfo.java
//
// Purpose-
//       [Long drive, Closest to pin] information container class.
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
//       CourseLdCpInfo
//
// Purpose-
//       [Long drive, Closest to pin] information container class.
//
//----------------------------------------------------------------------------
public class CourseLdCpInfo  extends HoleInfo {
CourseNearInfo         close;       // The closest to pin hole data array
CourseLongInfo         drive;       // The longest drive hole data array

// Default value, neither LD nor CP
public static final String DEFAULT_VALUE= "-";

//----------------------------------------------------------------------------
//
// Method-
//       CourseLdCpInfo.debug
//
// Purpose-
//       Extend DebuggingAdaptor.
//
//----------------------------------------------------------------------------
public synchronized void
   debug( )                         // Debugging display
{
   super.debug();
   System.out.println("HoleInfo.close: " + close.toString());
   System.out.println("HoleInfo.drive: " + drive.toString());
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseLdCpInfo.CourseLdCpInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   CourseLdCpInfo(                  // Constructor
     int               format,      // The HolePanel format
     CourseLongInfo    drive,       // The COURSE.LONG information
     CourseNearInfo    close)       // The COURSE.NEAR information
{
   super(format, null, empty);

   this.drive= drive;
   this.close= close;

   for(int hole= 1; hole<=18; hole++)
   {
     array[hole-1]= DEFAULT_VALUE;
     if( drive.array[hole-1].equals("+") )
       if( close.array[hole-1].equals("+") )
         array[hole-1]= "LD/CP";
       else
         array[hole-1]= "LD";
     else if( close.array[hole-1].equals("+") )
       array[hole-1]= "CP";
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseLdCpInfo.genPanel
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
   panel= new HolePanel(format, "LD/CP", array);
   DataField[] df= panel.getField();

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
//       CourseLdcpeInfo.isDefault
//
// Purpose-
//       Do the arrays contain defaulted values?
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff default
   isDefault( )                     // Is array default?
{
   return (close.isDefault() && drive.isDefault());
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseLdCpInfo.isValid
//
// Purpose-
//       Validate the panel.
//
// Validators-
//       super()
//       set('-','CP','LD','LD/CP','CP/LD')
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff valid
   isValid(                         // Is panel valid
     Validator         v)           // Validator
{
   if( !isRemoved && panel != null)
   {
     DataField df[]= panel.getField();
     for(int hole= 1; hole<=18; hole++)
     {
       String text= df[hole].getText().trim();
       df[hole].setText(text);
       if( text.equals("") )
       {
         text= DEFAULT_VALUE;
         df[hole].setText(text);
       }
       if( !text.equals(DEFAULT_VALUE)
           && !text.equalsIgnoreCase("CP")
           && !text.equalsIgnoreCase("LD")
           && !text.equalsIgnoreCase("LD/CP")
           && !text.equalsIgnoreCase("CP/LD") )
         return v.invalid("Hole[" + hole + "] '" + df[hole].getText()
                        + "' not in {'-','CP','LD','LD/CP','CP/LD'}");
     }
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseLdCpInfo.output
//
// Purpose-
//       Change the database data, update the state.
//
//----------------------------------------------------------------------------
public synchronized void
   output(                          // Output to database
     DbClient          client,      // The database client
     String            item)        // The database item key
{
   drive.output(client, "COURSE.LONG", item);
   close.output(client, "COURSE.NEAR", item);

   isChanged= false;
   isPresent= (drive.isPresent || close.isPresent);
   isRemoved= (drive.isRemoved && close.isRemoved);
}

//----------------------------------------------------------------------------
//
// Method-
//       CourseLdCpInfo.update
//
// Purpose-
//       Update the internal fields, setting isChanged as appropriate.
//
//----------------------------------------------------------------------------
public synchronized void
   update( )                        // Update from panel
{
   if( panel != null )
   {
     DataField df[]= panel.getField();
     for(int hole= 1; hole<=18; hole++)
     {
       if( !array[hole-1].equalsIgnoreCase(df[hole].getText()) )
       {
         isChanged= true;
         array[hole-1]= df[hole].getText().toUpperCase();

         if( array[hole-1].equals("CP") )
         {
           close.select(hole, true);
           drive.select(hole, false);
         }
         else if( array[hole-1].equals("LD") )
         {
           close.select(hole, false);
           drive.select(hole, true);
         }
         else if( array[hole-1].equals("LD/CP")
             || array[hole-1].equals("CP/LD") )
         {
           array[hole-1]= "LD/CP";
           close.select(hole, true);
           drive.select(hole, true);
         }
         else
         {
           array[hole-1]= DEFAULT_VALUE;
           close.select(hole, false);
           drive.select(hole, false);
         }
       }
     }
   }

   isChanged= (close.isChanged || drive.isChanged);
}
} // class CourseLdCpInfo
