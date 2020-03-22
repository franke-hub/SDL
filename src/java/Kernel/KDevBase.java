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
//       KDevBase.java
//
// Purpose-
//       BASE device driver.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       KDevBase
//
// Purpose-
//       BASE device driver.
//
//----------------------------------------------------------------------------
public class KDevBase implements KDev
{
//----------------------------------------------------------------------------
// KDevBase.attributes
//----------------------------------------------------------------------------
static int             serial= 0;   // Device serial number

   String              devName;     // Device name
   int                 devIdent;    // Device identifier
   boolean             debugging;   // Debugging state

//----------------------------------------------------------------------------
// KDevBase.contructor
//----------------------------------------------------------------------------
public
   KDevBase( )                      // Default constructor
{
   devName=  "BASE";
   devIdent= serial++;
   debugging= true;
}

public
   KDevBase(                        // Constructor
     String            name)        // Device name
{
   devName=  name;
   devIdent= serial++;
   debugging= true;
}

public void
   reject(                          // Reject an operation
     String            message)     // Reject code
   throws KioException
{
   throw new KioException(devName + ": " + devIdent + ": " + message);
}

public String                       // The String
   toString( )                      // Get String representation
{
   return new String(devName + ": " + devIdent );
}

//----------------------------------------------------------------------------
// KDevBase.methods
//----------------------------------------------------------------------------
public void
   error( )                         // 00 Operation
   throws KioException
{
   reject("Op(00) Rejected");
}

public void
   charOut(                         // 01 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   reject("Op(01) Rejected");
}

public void
   charInp(                         // 02 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   reject("Op(02) Rejected");
}

public void
   nop( )                           // 03 Operation
   throws KioException
{
   reject("Op(03) Rejected");
}

public void
   sense(                           // 04 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   reject("Op(04) Rejected");
}

public void
   pageOut(                         // 05 Operation
     int               offset,      // Offset
     RealPage          page)        // Data (implied length)
   throws KioException
{
   reject("Op(05) Rejected");
}

public void
   pageInp(                         // 06 Operation
     int               offset,      // Offset
     RealPage          page)        // Data (implied length)
   throws KioException
{
   reject("Op(06) Rejected");
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
   reject("Op(0F) Rejected");
}
} // class KDevBase

