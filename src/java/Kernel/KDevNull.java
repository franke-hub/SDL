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
//       KDevNull.java
//
// Purpose-
//       NULL device driver.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       KDevNull
//
// Purpose-
//       NULL device driver.
//
//----------------------------------------------------------------------------
public class KDevNull extends KDevBase
{
//----------------------------------------------------------------------------
// KDevNull.contructor
//----------------------------------------------------------------------------
public
   KDevNull( )                      // Default constructor
{
   super("NULL");
}

public
   KDevNull(                        // Constructor
     String            name)        // Device name
{
   super(name);
}

//----------------------------------------------------------------------------
// KDevNull.methods
//----------------------------------------------------------------------------
public void
   charOut(                         // 01 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   size.set(data.length);
}

public void
   charInp(                         // 02 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   size.set(0);
   throw new KioEofException("KDevNull");
}

public void
   nop( )                           // 03 Operation
   throws KioException
{
}

public void
   sense(                           // 04 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   size.set(0);
}

public void
   pageOut(                         // 05 Operation
     int               offset,      // Offset
     RealPage          page)        // Data (implied length)
   throws KioException
{
}

public void
   pageInp(                         // 06 Operation
     int               offset,      // Offset
     RealPage          page)        // Data (implied length)
   throws KioException
{
   page.zero();
}

public void
   chase( )                         // 07 Operation
   throws KioException
{
   reject("Op(07) Rejected");
}

public void
   op08( )                          // 08 Operation
   throws KioException
{
   reject("Op(08) Rejected");
}

public void
   ctrlOut(                         // 09 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   reject("Op(09) Rejected");
}

public void
   ctrlInp(                         // 0A Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   reject("Op(0A) Rejected");
}

public void
   op0B( )                          // 0B Operation
   throws KioException
{
   reject("Op(0B) Rejected");
}

public void
   op0C( )                          // 0C Operation
   throws KioException
{
   reject("Op(0C) Rejected");
}

public void
   bootOut(                         // 0D Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   reject("Op(0D) Rejected");
}

public void
   bootInp(                         // 0E Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   reject("Op(0E) Rejected");
}

public void
   reset( )                         // 0F Operation
   throws KioException
{
}
} // class KDevNull

