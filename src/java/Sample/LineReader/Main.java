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
//       LineReader test driver.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       LineReader driver.
//
//----------------------------------------------------------------------------
public class Main
{
//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       LineReader test.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]        argv)          // Arguments
   throws Exception
{
   LineReader        lr;            // LineReader object

   lr= new LineReader();
   if( argv.length > 0 )
     lr= new LineReader(argv[0]);

   lr.read();
   lr.write();
}
} // class Main

