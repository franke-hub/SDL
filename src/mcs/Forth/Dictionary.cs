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
//       Dictionary.cs
//
// Purpose-
//       Stack<Code> indexibile by Code.Token.
//
// Last change date-
//       2015/01/26
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
using System;                       // (Univerally required)
using System.Collections.Generic;   // For SortedDictionary

namespace Forth {
    //------------------------------------------------------------------------
    // Dictionary
    //------------------------------------------------------------------------
    public class Dictionary: Stack<Code> // Dictionary
    {
        SortedDictionary<string, long> map;

        public Code this[string that] // Indexer
        {
            get {
                try {
                    return this[map[that]];
                } catch {
                    // Exceptions reported as null result
                }

                return null;
            }
        }

        public Dictionary() : base("Dictionary") {
            map = new SortedDictionary<string,long>();
        }

        public override Code Pop() {
            Code code = base.Pop();
            map.Remove(code.Token);
            return code;
        }

        public override void Push(Code code) {
            string that = code.Token;
            if( map.ContainsKey(that) )
                throw new IndexOutOfRangeException("DuplicateKey(" +that+ ")");

            map.Add(code.Token, Length);
            base.Push(code);
        }

        public override void Reset() {
            if( map != null )       // (Called from Stack<Code>.Constructor)
                map.Clear();

            base.Reset();
        }
    }
} // namespace Forth
