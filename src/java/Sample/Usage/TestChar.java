//----------------------------------------------------------------------------
//
//       Copyright (C) 2008 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestChar.java
//
// Purpose-
//       Test Java char.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
import java.util.*;
import java.lang.*;

public class TestChar
{
//----------------------------------------------------------------------------
//
// Method-
//       TestChar.main
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
   StringBuffer        buffer= new StringBuffer();
   int                 errorCount= 0;

   System.out.print("===============>");
   for(int i= 0; i<256; i++)
   {
     char C= (char)i;
     System.out.print(C);
     buffer.append(C);

     if( C != i )
     {
       errorCount++;
       System.out.format("Error1: char(%d,%c) != int(%d)\n", (int)C, C, i);
     }
   }
   System.out.println("<===============");

   for(int i= 0; i<256; i++)
   {
     int C= buffer.charAt(i);
     if( C != i )
     {
       errorCount++;
       System.out.format("Error2: char(%d,%c) != int(%d)\n", (int)C, C, i);
     }
   }

   System.out.println(errorCount + " Errors");
}
} // Class TestChar

