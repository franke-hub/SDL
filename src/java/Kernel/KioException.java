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
//       KioException.java
//
// Purpose-
//       Define generic kernel I/O Exception.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.Trace;

//----------------------------------------------------------------------------
//
// Class-
//       KioException
//
// Purpose-
//       Generic Kernel I/O Exception
//
//----------------------------------------------------------------------------
public class KioException extends Exception
{
//----------------------------------------------------------------------------
//
// Method-
//       KioException.KioException
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   KioException(                    // Constructor
     String            message)     // Associated message
{
   super(message);

   Trace.get().tracef("KioException: " + message);
}
} // class KioException

