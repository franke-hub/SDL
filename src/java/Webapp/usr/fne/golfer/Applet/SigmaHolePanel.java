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
//       SigmaHolePanel.java
//
// Purpose-
//       The SIGMA golf hole panel.
//
// Last change date-
//       2020/01/15
//
// Implementation notes-
//       Sigma HOLE[ 1.. 9] => HOLE_OUT
//       Sigma HOLE[10..18] => HOLE_IN
//       Sigma HOLE[HOLE_OUT, HOLE_IN] => HOLE_TOT
//
//----------------------------------------------------------------------------
import java.awt.*;
import java.lang.*;
import java.util.*;
import javax.swing.*;

//----------------------------------------------------------------------------
//
// Class-
//       SigmaHolePanel
//
// Purpose-
//       SIGMA golf hole Panel.
//
//----------------------------------------------------------------------------
public class SigmaHolePanel extends HolePanel {
//----------------------------------------------------------------------------
//
// Method-
//       SigmaHolePanel.SigmaHolePanel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   SigmaHolePanel(                  // Constructor
     int               format,      // The number of fields
     String            title,       // Panel title
     String[]          array)       // Panel hole value array
{
   super(format, title, array);

   field[HOLE_IN].change();
   field[HOLE_OUT].change();
   field[HOLE_TOT].change();
}

public
   SigmaHolePanel(                  // Constructor
     int               format)      // The number of fields
{
   this(format, null, null);
}

public
   SigmaHolePanel( )                // Constructor
{
   this(HOLE_TOT+1);
}

//----------------------------------------------------------------------------
//
// Method-
//       SigmaHolePanel.setField
//
// Purpose-
//       Set the field array, adding each field to the panel.
//
//----------------------------------------------------------------------------
public void
   setField( )                      // Set the Field array
{
   removeAll();
   add(field[HOLE_ID]);

   for(int hole= 1; hole<=9; hole++)
     add(field[hole]);

   add(field[HOLE_OUT]);

   for(int hole= 10; hole<=18; hole++)
     add(field[hole]);

   for(int i= HOLE_IN; i<field.length; i++)
     add(field[i]);

   field[HOLE_IN].change();
   field[HOLE_OUT].change();
   field[HOLE_TOT].change();
}

public void
   setField(                        // Set the Field array
     DataField[]       array)       // To this
{
   field= array;

   // Set OUT, IN, and TOT SigmaField Objects
   DataField[] sigma= new DataField[9];
   int x= 0;
   for(int hole= 1; hole<=9; hole++)
     sigma[x++]= field[hole];
   field[HOLE_OUT]= SigmaField.toSigmaField(sigma, field[HOLE_OUT]);

   sigma= new DataField[9];
   x= 0;
   for(int hole= 10; hole<=18; hole++)
     sigma[x++]= field[hole];
   field[HOLE_IN]=  SigmaField.toSigmaField(sigma, field[HOLE_IN]);

   sigma= new DataField[2];
   sigma[0]= field[HOLE_OUT];
   sigma[1]= field[HOLE_IN];
   field[HOLE_TOT]= SigmaField.toSigmaField(sigma, field[HOLE_TOT]);

   // Reset the Panel
   setField();
}
} // class SigmaHolePanel
