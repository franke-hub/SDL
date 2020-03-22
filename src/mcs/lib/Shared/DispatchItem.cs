//-----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       DispatchItem.cs
//
// Purpose-
//       Dispatcher work Item descriptor
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
using System;
using System.Threading;             // For Thread

using Shared;                       // For Debug

namespace Shared.Dispatch {         // The Dispatcher namespace
//-----------------------------------------------------------------------------
//
// Class-
//       (Shared.Dispatch.)Item
//
// Purpose-
//       Work Item descriptor.
//
//-----------------------------------------------------------------------------
public class Item {                 // Work Item Descriptor
//-----------------------------------------------------------------------------
// Item.Constants
//-----------------------------------------------------------------------------
public enum CC : int                // Completion Codes
{  NORMAL= 0                        // (CC.)Normal (OK)
,  ERROR                            // Generic error
,  PURGE                            // Function purged
,  INVALID_FC                       // Function code rejected
}; // enum CC

public enum FC : int                // Function Codes
{  CHASE= (-1)                      // (FC.)Chase (NOP)
,  TRACE= (-2)                      // Trace (NOP)
,  RESET= (-3)                      // Reset the TAB
,  VALID= 0                         // All user function codes are positive
}; // enum FC

//-----------------------------------------------------------------------------
// Item.Attributes
//-----------------------------------------------------------------------------
public int             fc;          // Function code/modifier
public int             cc;          // Completion code
Done                   done;        // Done callback Object

//-----------------------------------------------------------------------------
// Item.Constructors
//-----------------------------------------------------------------------------
public Item(Done done) : this(0, done) {}
public Item(int fc= 0, Done done= null)
{
   this.fc= fc;
   this.cc= -1;
   this.done= done;
}

//-----------------------------------------------------------------------------
// Item.Methods
//-----------------------------------------------------------------------------
public void
   post(                            // Complete the Work Unit
     int               cc= 0)       // With this completion code
{
   this.cc= cc;                     // Set the completion code
   if( done != null )               // If a Done object was specified
       done.done();                 // Drive the WhenDone object
}
}  // class Item
}  // namespace Shared.Dispatch
