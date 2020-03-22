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
//       KDev.java
//
// Purpose-
//       Kernel device interface.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Interface-
//       KDev
//
// Purpose-
//       Define kernel device interface.
//
//----------------------------------------------------------------------------
public interface KDev
{
public void
   error( )                         // 00 Operation
   throws KioException;

public void
   charOut(                         // 01 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException;

public void
   charInp(                         // 02 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException;

public void
   nop( )                           // 03 Operation
   throws KioException;

public void
   sense(                           // 04 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException;

public void
   pageOut(                         // 05 Operation
     int               offset,      // Offset
     RealPage          page)        // Data (implied length)
   throws KioException;

public void
   pageInp(                         // 06 Operation
     int               offset,      // Offset
     RealPage          page)        // Data (implied length)
   throws KioException;

public void
   chase( )                         // 07 Operation
   throws KioException;

public void
   op08( )                          // 08 Operation
   throws KioException;

public void
   ctrlOut(                         // 09 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException;

public void
   ctrlInp(                         // 0A Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException;

public void
   op0B( )                          // 0B Operation
   throws KioException;

public void
   op0C( )                          // 0C Operation
   throws KioException;

public void
   bootOut(                         // 0D Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException;

public void
   bootInp(                         // 0E Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException;

public void
   reset( )                         // 0F Operation
   throws KioException;
} // class KDev

