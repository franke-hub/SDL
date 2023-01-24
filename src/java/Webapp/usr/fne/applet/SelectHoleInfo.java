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
//       SelectHoleInfo.java
//
// Purpose-
//       Hole selector information container class.
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
//       SelectHoleInfo
//
// Purpose-
//       Hole selector information container class.
//
//----------------------------------------------------------------------------
public class SelectHoleInfo  extends HoleInfo {
//----------------------------------------------------------------------------
//
// Method-
//       SelectHoleInfo.SelectHoleInfo
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   SelectHoleInfo(                  // Constructor
     int               format,      // The HolePanel format
     String[]          input)       // The hole selection data array
{
   super(format, null, empty);

   isChanged= false;
   if( input != null )
     isPresent= true;

   for(int i= 0; i<array.length; i++)
     array[i]= "-";

   if( input != null )
   {
     for(int i= 0; i<input.length; i++)
     {
       try {
         int x= Integer.parseInt(input[i]);
         array[x-1]= "+";
       } catch(Exception e) {
         e.printStackTrace();
         debug("SelectHoleInfo.arrayInit: " + e);
       }
     }
   }
}

   SelectHoleInfo(                  // Constructor
     int               format,      // The HolePanel format
     String            string)      // The hole selection  String
{
   this(format, tokenize(string));
}

   SelectHoleInfo(                  // Constructor
     int               format)      // The HolePanel format
{
   this(format, empty);
}

//----------------------------------------------------------------------------
//
// Method-
//       SelectHoleInfo.genPanel
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
   throw new RuntimeException("ShouldNotOccur");
}

//----------------------------------------------------------------------------
//
// Method-
//       SelectHoleInfo.getPanel
//
// Purpose-
//       Access the internal panel
//
//----------------------------------------------------------------------------
public DataPanel                    // The current DataPanel
   getPanel( )                      // Get current DataPanel
{
   throw new RuntimeException("ShouldNotOccur");
}

//----------------------------------------------------------------------------
//
// Method-
//       SelectHoleInfo.isDefault
//
// Purpose-
//       Do the arrays contain defaulted values?
//
//----------------------------------------------------------------------------
public synchronized boolean         // TRUE iff default
   isDefault( )                     // Is array default?
{
   boolean result= true;

   for(int i= 0; i<18; i++)
   {
     if( array[i].equals("+") )
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
//       SelectHoleInfo.output
//
// Purpose-
//       Change the database data, update the state.
//
//----------------------------------------------------------------------------
public synchronized void
   output(                          // Output to database
     DbClient          client,      // The database client
     String            type,        // The database type
     String            item)        // The database item key
{
   if( isDefault() )
     isRemoved= true;
   super.output(client, type, item);
}

//----------------------------------------------------------------------------
//
// Method-
//       SelectHoleInfo.select
//
// Purpose-
//       Update the hole selection.
//
//----------------------------------------------------------------------------
public synchronized boolean         // Duplicate selection indicator
   select(                          // Update hole selection
     int               hole,        // The hole number
     boolean           selected)    // The selection
{
   boolean result= true;

   try {
     result= array[hole-1].equals("+");
     if( selected )
     {
       if( !result )
       {
         isChanged= true;
         array[hole-1]= "+";
       }
     }
     else
     {
       if( result )
       {
         isChanged= true;
         array[hole-1]= "-";
       }
     }
   } catch(Exception e) {
     result= true;
     e.printStackTrace();
   }

   return result;
}

public boolean                      // Duplicate selection indicator
   select(                          // Update hole selection
     int               hole)        // The hole number
{
   return select(hole, true);
}

//----------------------------------------------------------------------------
//
// Method-
//       SelectHoleInfo.toInt
//
// Purpose-
//       Extract the array into int[]
//
//----------------------------------------------------------------------------
public synchronized int[]           // Resultant int[]
   toInt( )                         // Convert to int[]
{
   int count= 0;
   for(int hole= 1; hole<=array.length; hole++)
     if( array[hole-1].equals("+") )
       count++;

   int[] result= new int[count];
   for(int hole= 1; hole<=array.length; hole++)
     if( array[hole-1].equals("+") )
       result[count++]= hole;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SelectHoleInfo.toString
//
// Purpose-
//       Extract the array into space separated String objects.
//
//----------------------------------------------------------------------------
public String                       // Resultant String
   toString( )                      // Convert to String
{
   String result= null;

   synchronized(this)
   {{{{
     StringBuffer buff= null;
     for(int hole= 1; hole<=18; hole++)
     {
       if( array[hole-1].equals("+") )
       {
         if( buff == null )
           buff= new StringBuffer();
         else
           buff.append(" ");

         buff.append("" + hole);
       }
     }

     if( buff != null )
       result= buff.toString();
   }}}} // synchronized(this)

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SelectHoleInfo.update
//
// Purpose-
//       Update the internal fields, setting isChanged as appropriate.
//
//----------------------------------------------------------------------------
public synchronized void
   update( )                        // Update from panel
{
   throw new RuntimeException("ShouldNotOccur");
}
} // class SelectHoleInfo
