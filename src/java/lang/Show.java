//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Show.java
//
// Purpose-
//       Language tester.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Show
//
// Purpose-
//       Java language test.
//
//----------------------------------------------------------------------------
public class Show
{
//----------------------------------------------------------------------------
//
// Method-
//       Show.Show
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Show(                            // Constructor
     Object            o)           // Object to show
{
   show(o);
}

//----------------------------------------------------------------------------
//
// Method-
//       Show.show
//
// Purpose-
//       Display an Object.
//
//----------------------------------------------------------------------------
public void
   show(                            // Display an Object
     String            prefix,      // Prefix String
     Object            o)           // Object to show
{
   String              s;
   int                 i;

   System.out.println(prefix + ": " + o);
   if( !(o instanceof Object[]) )
     return;

   for(i=0; i<((Object[])o).length; i++)
   {
     s= prefix + "[" + i + "]";
     show(s, ((Object[])o)[i]);
   }
}

public void
   show(                            // Display an Object
     Object            o)           // Object to show
{
   String              prefix;
   int                 i;

   System.out.println("" + o);
   if( !(o instanceof Object[]) )
     return;

   for(i=0; i<((Object[])o).length; i++)
   {
     prefix= "[" + i + "]";
     show(prefix, ((Object[])o)[i]);
   }
}
} // Class Show

