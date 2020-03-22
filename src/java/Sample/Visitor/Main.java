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
//       Main.java
//
// Purpose-
//       Test visitor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

public class Main
{
//----------------------------------------------------------------------------
//
// Method-
//       Main.testUpperLower
//
// Purpose-
//       Test Upper/Lower level function
//
//----------------------------------------------------------------------------
public static void
   testUpperLower( )                // Test Upper/Lower level function
{
   Upper               root= new Upper("Root",null);

   new Upper("L1-c", root);
   Upper l1= new Upper("L1-b", root);
   new Upper("L1-a", root);

   Upper l2a= new Upper("L2-c", l1);
   Upper l2b= new Upper("L2-b", l1);
   Upper l2c= new Upper("L2-a", l1);

   new Upper("L3-ai", l2a);
   new Upper("L3-ah", l2a);
   new Upper("L3-ag", l2a);

   new Upper("L3-bf", l2b);
   new Upper("L3-be", l2b);
   new Upper("L3-bd", l2b);

   new Upper("L3-cc", l2c);
   new Upper("L3-cb", l2c);
   new Upper("L3-ca", l2c);

   root.visit();
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.testVisitor
//
// Purpose-
//       Test AbstractVisitor
//
//----------------------------------------------------------------------------
public static void
   testVisitor( )                   // Test AbstractVisitor
{
   ConcreteVisitor     visitor= new ConcreteVisitor();
   Visitee             root= new Visitee("Root",null);

   new Visitee("L1-c", root);
   Visitee l1= new Visitee("L1-b", root);
   new Visitee("L1-a", root);

   Visitee l2a= new Visitee("L2-c", l1);
   Visitee l2b= new Visitee("L2-b", l1);
   Visitee l2c= new Visitee("L2-a", l1);

   new Visitee("L3-ai", l2a);
   new Visitee("L3-ah", l2a);
   new Visitee("L3-ag", l2a);

   new Visitee("L3-bf", l2b);
   new Visitee("L3-be", l2b);
   new Visitee("L3-bd", l2b);

   new Visitee("L3-cc", l2c);
   new Visitee("L3-cb", l2c);
   new Visitee("L3-ca", l2c);

   root.visit(visitor);
   visitor.list();
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
   System.out.println("Main.main()..");

   testVisitor();
   testUpperLower();

   System.out.println("..Main.main()");
   System.exit(0);
}
} // Class Main

