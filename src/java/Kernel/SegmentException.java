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
//       SegmentException.java
//
// Purpose-
//       SegmentException descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       SegmentException
//
// Purpose-
//       Segment fault descriptor.
//
//----------------------------------------------------------------------------
public class SegmentException extends PagingException
{
//----------------------------------------------------------------------------
// SegmentException.constructor
//----------------------------------------------------------------------------
   SegmentException(
     int               address)
{
   super(address);
   Cpu.tracef(toString());
}
} // Class SegmentException

