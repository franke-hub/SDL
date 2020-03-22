//----------------------------------------------------------------------------
//
//       Copyright (c) 2015-2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Import.cs
//
// Purpose-
//       Import the Debug and Stack<T> objects.
//
// Last change date-
//       2019/02/15
//
//----------------------------------------------------------------------------
using System;                       // (Univerally required)
using Shared;                       // Import library

namespace Forth {
public class Debug: Shared.Debug {}

public class Stack<T>: Common.System.Stack<T> {
   public Stack(string name = null) : base(name) {}
}
} // namespace Forth
