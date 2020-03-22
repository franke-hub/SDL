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
//       ObjectOB.java
//
// Purpose-
//       Sample RMI server interface.
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.rmi.*;
import java.rmi.server.*;

public class ObjectOB extends    java.rmi.server.UnicastRemoteObject
                      implements ObjectIF
{
//----------------------------------------------------------------------------
// ObjectOB.Attributes
//----------------------------------------------------------------------------
int                    callcount;

//----------------------------------------------------------------------------
// ObjectOB.Constructor
//----------------------------------------------------------------------------
public
   ObjectOB( )
   throws Exception
{
   super();
   callcount= 0;
// Naming.bind("ObjectOB", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectOB.method
//
// Purpose-
//       Sample method.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   method(                          // Sample method
     Object            object)      // Input Object
   throws Exception
{
   callcount++;
   System.out.println("MethodOB.method: Object: '" +
                      object.toString() + "'");

   return new String("ObjectOB.method resultant " + callcount);
}
} // Class ObjectOB

