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
//       ObjectIF.java
//
// Purpose-
//       Sample RMI server interface.
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.rmi.*;

public interface ObjectIF extends java.rmi.Remote
{
//----------------------------------------------------------------------------
//
// Method-
//       ObjectIF.method
//
// Purpose-
//       Sample method.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   method(                          // Sample method
     Object            object)      // Input Object
   throws Exception;
} // Class ObjectIF

