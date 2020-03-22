//----------------------------------------------------------------------------
//
//       Copyright (C) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PosCondition.java
//
// Purpose-
//       Positive Condition descriptor
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       PosCondition
//
// Purpose-
//       Positive Condition descriptor
//
// Reference-
//       (Positive) condition
//
//----------------------------------------------------------------------------
public class PosCondition extends Condition { // Positive Condition
//----------------------------------------------------------------------------
//
// Method-
//       PosCondition.PosCondition
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   PosCondition(                    // Constructor
     WME_Key           key)         // Associated key
{
   super(key);
}

public
   PosCondition(                    // Copy constructor
     Condition         copy)        // Source
{
   super(copy);
}

public
   PosCondition(                    // Constructor
     String            id,          // Identifier
     String            attr,        // Attribute
     String            value)       // Value
{
   super(new WME_Key(id, attr, value));
}
} // class PosCondition

