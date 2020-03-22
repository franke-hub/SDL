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
//       DispatchDone.cs
//
// Purpose-
//       Dispatcher Done callback Object
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
//       (Shared.Dispatch.)Done
//
// Purpose-
//       Dispatcher Done callback Object
//
//-----------------------------------------------------------------------------
public class Done {                 // Dispacher Done callback Object
//-----------------------------------------------------------------------------
// Done.Attributes
//-----------------------------------------------------------------------------
// None defined

//-----------------------------------------------------------------------------
// Done.Methods
//-----------------------------------------------------------------------------
public virtual void
   done( )                          // OVERRIDE this method
{ }                                 // This one does nothing
}  // class Done

//-----------------------------------------------------------------------------
//
// Class-
//       (Shared.Dispatch.)Wait
//
// Purpose-
//       Dispatcher Wait until done Object.
//
//-----------------------------------------------------------------------------
public class Wait : Done {          // Dispatcher Wait until Done Object
//-----------------------------------------------------------------------------
// Wait.Attributes
//-----------------------------------------------------------------------------
ManualResetEvent       handle= new ManualResetEvent(false);

//-----------------------------------------------------------------------------
// Wait.Methods
//-----------------------------------------------------------------------------
public override void
   done( )                          // Indicate complete
{  handle.Set(); }                  // Set the event

public void
   reset( )                         // Reset the event
{  handle.Reset(); }                // Reset the event

public void
   wait( )                          // Wait for event
{  handle.WaitOne(); }              // Wait for event
}  // class Wait
}  // namespace Shared.Dispatch
