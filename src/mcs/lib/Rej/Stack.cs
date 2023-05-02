//----------------------------------------------------------------------------
//
//       Copyright (c) 2015 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Stack.cs
//
// Purpose-
//       Extendable Stack.
//
// Last change date-
//       2015/01/13
//
// Implementation notes-
//       Improperly formatted
//
//----------------------------------------------------------------------------
#if false                           // Hard-Core Debug Mode?
    #define HCDM
#endif

using System;
using System.Collections;           // For IEnumerator, IEnumerable

namespace Rejected.System {
    //------------------------------------------------------------------------
    //
    // Class-
    //   Stack
    //
    // Purpose-
    //   Extendable Stack.
    //
    //------------------------------------------------------------------------
    public class Stack<T>: Index<T> // Extendable Stack, implements foreach
    {
        protected Leaf tail;        // Tail Leaf

        //--------------------------------------------------------------------
        // Stack: Implementation
        //--------------------------------------------------------------------
        public Stack(string name = null) : base(name) {} // Default constructor

        //--------------------------------------------------------------------
        // Stack[] Implementation
        //--------------------------------------------------------------------
        public new T this[long index]   // Indexer
        {
            get { return base[index]; }  // AKA Stack.get_Item
            protected set { base[index] = value; } // AKA Stack.set_Item
        }

        //--------------------------------------------------------------------
        // Stack: Methods
        //--------------------------------------------------------------------
        public T Peek()             // Examine top element
        {
            if( length == 0 )
                throw indexOutOfRangeException(-1);

            return tail.data[Leaf.index(length - 1)];
        }

        public void Poke(T t)       // Replace top element
        {
            if( length == 0 )
                throw indexOutOfRangeException(-1);

            int leafX = Leaf.index(length - 1);
            tail.data[leafX] = t;
            changeID++;
        }

        public virtual T Pop()      // Remove from Stack
        {
            if( length <= 0 )
                throw indexOutOfRangeException(0);

            int leafX = Leaf.index(--length);
            T result = tail.data[leafX];

            tail.data[leafX] = default(T); // Remove object reference
            if( leafX == 0 && length > 0 )
                tail = getLeaf(length - 1);

            changeID++;
            return result;
        }

        public virtual void Push(T t) // Append to Stack
        {
            if( (length & (HUNK_SIZE - 1)) == 0 )
                grow(length + 1);

            int leafX = Leaf.index(length++);
            if( leafX == 0 )
                tail = getLeaf(length - 1);

            tail.data[leafX] = t;
            changeID++;
        }
    } // class Stack<T>
} // namespace Rejected.System

