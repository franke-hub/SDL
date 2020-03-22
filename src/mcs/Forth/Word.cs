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
//       Word.cs
//
// Purpose-
//       Define the data stack elements.
//
// Last change date-
//       2015/01/13
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
using System;
using System.Collections.Generic;   // For IEnumerable<T>
using System.Globalization;         // For NumberStyles
using System.Runtime.InteropServices;  // For Word

namespace Forth {
    //------------------------------------------------------------------------
    // struct Word - DataStack element
    //
    // Since we can't derive from the built-in classes, we have to build
    // the quivalent structure.
    //------------------------------------------------------------------------
    public struct Word: IComparable, IEquatable<Word>, IFormattable
    {
        public const int Size = 8;  // Number of bytes in Word
        internal     long value;    // The value

        const string HEXTAB = "0123456789ABCDEF";
        const string hextab = "0123456789abcdef";

        // Forth methods
        public static Word Parse(string s, Context c) { // Parse using Context
            int  negate = 1;
            int  radix  = (int)c.BASE;
            if( s[0] == '-' )
            {
                negate = (-1);
                s = s.Substring(1);
            }

            Debug.assert( s.Length > 0 );
            Word w = new Word();
            foreach(char C in s)
            {
                int x = hextab.IndexOf(C);
                if( x < 0 )
                    x = HEXTAB.IndexOf(C);
                Debug.assert( x >= 0 && x < radix );
                w *= radix;
                w += x;
            }

            w *= negate;
            return w;
        }

        public string ToString(Context c) { // Convert to string using Context
            bool negate = false;
            int  radix  = (int)c.BASE;
            long value  = this.value;

            char[] invert = new char[32];
            int    used   = 0;

            // Special case for radix 16
            if( radix == 16 )
            {
                for(int i= 0; i<16; i++)
                {
                    int x = (int)value & 0x0000000f;
                    invert[used++] = hextab[x];
                    value >>= 4;
                }
                // invert[used++] = 'x';
                // invert[used++] = '0';
            }
            else
            {
                if( value < 0 )
                {
                    negate = true;
                    value  = (-value);
                    if( value < 0 ) // Special case, minvalue
                    {
                        int x = (int)(value % radix);
                        invert[used++] = hextab[-x];
                        value = -(value / radix);
                    }
                }
                else if( value == 0 )
                    invert[used++] = '0';

                while( value > 0 )
                {
                    int x = (int)(value % radix);
                    invert[used++] = hextab[x];
                    value /= radix;
                }

                if( negate )
                    invert[used++] = '-';
            }

            char[] output = new char[used];
            for(int i= 0; i<used; i++)
                output[i] = invert[used - i - 1];

            return new string(output);
        }

        public void Peek(Context c, int addr) { // Fetch from UserMemory
            long value = 0;
            for(int i= 0; i<Size; i++)
            {
                value <<= 8;
                value  |= (long)c.UserMemory[addr+i];
            }
            this.value = value;
        }

        public void Poke(Context c, int addr) { // Store into UserMemory
            long value = this.value;
            for(int i= Size-1; i>=0; i--)
            {
                c.UserMemory[addr+i] = (byte)value;
                value >>= 8;
            }
        }

        // Cast from/into long (Assignment alias)
        public static implicit operator Word(long l) {
            Word w = new Word();
            w.value = l;
            return w;
        }

        public static implicit operator long(Word w) {
            return w.value;
        }

        // Interface IComparable
        public int CompareTo(object o) {
            if( o == null )
                return 1;

            if( o is Word ) {
                long w = (long)o;
                if( w < value ) return -1;
                if( w > value ) return 1;
                return 0;
            }

            throw new ArgumentException("Word CompareTo(NonWord)");
        }

        public int CompareTo(long w) {
            if( w < value ) return -1;
            if( w > value ) return 1;
            return 0;
        }

        // Interface IEquateable
        public bool Equals(long w) {
            return w == value;
        }

        public bool Equals(Word w) {
//          if( w == null )
//              return false;

            return w.value == value;
        }

        public override bool Equals(object o) {
            if( o == null )
                return false;

            if( o is Word ) {
                long w = (long)o;
                return w == value;
            }

            throw new ArgumentException("Word Equals(NonWord)");
        }

        public override int GetHashCode() {
            return (int)value;
        }

        // Interface IFormattable
        public override String ToString() {
            Int64 i = value;
            return i.ToString();
        }

        public String ToString(String format) {
            Int64 i = value;
            return i.ToString(format);
        }

        public String ToString(IFormatProvider provider) {
            Int64 i = value;
            return i.ToString(provider);
        }

        public String ToString(String format, IFormatProvider provider) {
            Int64 i = value;
            return i.ToString(format, provider);
        }

        public static long Parse(String s) {
            return Int64.Parse(s);
        }

        public static long Parse(String s, NumberStyles style) {
            return Int64.Parse(s, style);
        }

        public static long Parse(String s, IFormatProvider provider) {
            return Int64.Parse(s, provider);
        }
    }
} // namespace Forth

