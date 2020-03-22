//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.java
//
// Purpose-
//       Bringup sample.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Bringup.
//
//----------------------------------------------------------------------------
public class Main
{
//----------------------------------------------------------------------------
// Main.attributes
//----------------------------------------------------------------------------
   StringFormat        string;
   RealStorage         real;
   KDevPage            unit;

   Hw                  hw;

//----------------------------------------------------------------------------
//
// Method-
//       Main.Main
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Main( )                          // Constructor
{
   string= new StringFormat();

   hw= new Hw();
   real= hw.main.real;
   unit= (KDevPage)hw.dev[0];
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.testPaging
//
// Purpose-
//       Test paging devices.
//
//----------------------------------------------------------------------------
public void
   testPaging( )                    // Testcase code
   throws Exception
{
   int                 frame, index;
   Integer             value;
   RealPage            page;

   int                 i, j;

   tracef("testPaging started");
   for(frame= 0; frame<real.getSize(); frame++)
   {
     value= new Integer(frame);
     page= (RealPage)real.frame(frame);
     for(index= 0; index<Page.size; index++)
     {
       page.store(index,value);
     }
   }

   for(frame= 0; frame<256; frame++)
     unit.pageOut(frame, (RealPage)real.frame(frame));

   for(frame= 0; frame<256; frame++)
   {
     page= (RealPage)real.frame(frame);
     unit.pageInp(frame, page);

     value= new Integer(frame);
     for(index= 0; index<Page.size; index++)
     {
       if( !page.fetch(index).equals(value) )
       {
         throw new Exception("Unit[0] " +
                             "Page[" + frame + "] " +
                             "Word[" + index + "] " +
                             "Expected(" + value + ") " +
                             "Got(" + Debug.toString(page.fetch(index)) +
                                  ") "
                             );
       }
     }
   }
   tracef("testPaging complete");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.driver
//
// Purpose-
//       Testcase code.
//
//----------------------------------------------------------------------------
public void
   driver(                          // Testcase code
     String[]          args)        // Argument array
   throws Exception
{
   tracef("driver() started");
   hw.operate();
   tracef("driver() complete");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.tracef
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
public void
   tracef(                          // Debugging display
     String            string)      // String to display
{
   System.out.println(string);
   Trace.get().tracef(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   Main                main= new Main(); // Instantitaion object

   try {
     main.driver(args);
//// main.testPaging();

   } catch(Exception e) {
     System.out.println("Main: Exception: " + e);
     e.printStackTrace();
   }
}
} // class Main

