//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Index.cpp
//
// Purpose-
//       Sample operator[].
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

//----------------------------------------------------------------------------
//
// Class-
//       Index
//
// Purpose-
//       Demonstrate operator[]
//
// Implementation note-
//       The ACCESSOR operator[]() would always be called if we used
//       const Index index;
//       but then we couldn't use the MUTATOR operator[]() so easily.
//
//----------------------------------------------------------------------------
class Index {                       // Operator[] demo
public:
enum {MAX= 128};                    // MAX number of elements
typedef std::string    X;           // The Index type
typedef size_t         V;           // The Value type

unsigned               used= 0;
X                      x_array[MAX];
V                      v_array[MAX];

V& operator[](const X& x)           // MUTATOR: Can create entry
{
   for(unsigned i= 0; i<used; i++)
   {
     if( x == x_array[i] )
       return v_array[i];
   }

   x_array[used]= x;                // COPY the index
   v_array[used]= 0;                // Set DEFAULT value

   return v_array[used++];          // (*NO* overflow checking in this sample)
}

const V& operator[](const X& x) const // ACCESSOR: Does not create entry
{
   for(unsigned i= 0; i<used; i++)
   {
     if( x == x_array[i] )
       return v_array[i];
   }

   throw "You botched it, kiddo.";
}
}; // class Index

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   //-------------------------------------------------------------------------
   // Initialize the Index
   //-------------------------------------------------------------------------
   Index index;

   index["0"]=  0;
   index["1"]=  1;
   index["2"]=  2;
   index["3"]=  3;
   index["4"]=  4;
   index["5"]=  5;
   index["6"]=  6;
   index["7"]=  7;
   index["8"]=  8;
   index["9"]=  9;
   index["a"]= 10;
   index["b"]= 11;
   index["c"]= 12;
   index["d"]= 13;
   index["e"]= 14;
   index["f"]= 15;
   index["F"]= 15;
   index["E"]= 14;
   index["D"]= 13;
   index["C"]= 12;
   index["B"]= 11;
   index["A"]= 10;

   printf("DEBUG %u\n", index.used);
   for(unsigned i= 0; i<index.used; i++)
   {
     std::string s= index.x_array[i];
     if( i & 1 )
       printf("[%2u] %2zu= Index[\"%s\"] A\n",
              i, ((const Index)index)[s], s.c_str());
     else
       printf("[%2u] %2zu= Index[\"%s\"] M\n",
              i, index[s], s.c_str());
   }

   try { // More clarity on which operator[]() is being called
     std::string s= "g";
     printf("[%2d] %2zu= Index[\"%s\"] M\n",
            -1, index[s], s.c_str());
     printf("[%2d] %2zu= Index[\"%s\"] A\n",
            -1, ((const Index)index)[s], s.c_str());
     printf("Used(%d)\n", index.used);

     s= "h";                        // (This new insert fails)
     printf("[%2d] %2zu= Index[\"%s\"] A\n",
            -1, ((const Index)index)[s], s.c_str());
     printf("Used(%d)\n", index.used);
   } catch(const char* X) {
     printf("Exception(%s)\n", X);
   }

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   return 0;
}

