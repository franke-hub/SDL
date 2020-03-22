//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
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
//       HTTP Client.
//
// Last change date-
//       2020/01/12
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.*;
import java.util.*;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
class Main extends Debug {
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
   int                 i;

   Client client= new Client();

   client.readURL("http://localhost:8080/sample/Sample?A");
   client.readURL("http://localhost:8080/sample/Sample?B=queryString");
   client.readURL("http://localhost:8080/sample/Sample?errorString");
   client.readURL("http://localhost:8080/sample/Sample?postString", "POST");
   for(i= 0; i<args.length; i++)
     client.readURL(args[i]);
}
} // Class Main
