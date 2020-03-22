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
//       KioEofException.java
//
// Purpose-
//       Define end of file kernel I/O Exception.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       KioEofException
//
// Purpose-
//       Generic Kernel I/O End of File Exception
//
//----------------------------------------------------------------------------
public class KioEofException extends KioException
{
//----------------------------------------------------------------------------
//
// Method-
//       KioEofException.KioEofException
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   KioEofException(                 // Constructor
     String            message)     // Associated message
{
   super(message);
}
} // class KioEofException

