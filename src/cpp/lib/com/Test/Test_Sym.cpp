//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_Sym.cpp
//
// Purpose-
//       Test Symbol Table functions.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Symtab.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_SYM" // Source file, for debugging

//----------------------------------------------------------------------------
// Structures
//----------------------------------------------------------------------------
struct Symval : public Symtab::Symbol {
   long              addr;
}; // struct Symval

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Symtab        symbolTable(sizeof(struct Symval));

//----------------------------------------------------------------------------
//
// Subroutine-
//       thisName
//
// Purpose-
//       Extract the symbol name from a symbol.
//
//----------------------------------------------------------------------------
inline const char*
   thisName(                        // Get the qualifier name
     Symtab&         table,         // The symbol table
     const Symval*   ptrSymbol)     // -> Symval
{
   return table.getSymbolName(ptrSymbol);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       qualName
//
// Purpose-
//       Extract the qualifier name from a symbol.
//
//----------------------------------------------------------------------------
inline const char*
   qualName(                        // Get the qualifier name
     Symtab&         table,         // The symbol table
     const Symval*   ptrSymbol)     // -> Symval
{
   Symval*           qualifier;
   const char*       qualName= "";

   qualifier= (Symval*)table.getSymbolQual(ptrSymbol);
   if( qualifier != NULL )
     qualName= thisName(table, qualifier);

   return qualName;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       compName
//
// Purpose-
//       Compare symbol names
//
//----------------------------------------------------------------------------
inline int                          // <0, 0, >0
   compName(                        // Get the qualifier name
     Symtab&         table,         // The symbol table
     const Symval*   a,             // -> Symval
     const Symval*   b)             // -> Symval
{
   int               rc;

   rc= strcmp(qualName(table,a), qualName(table,b));
   if( rc != 0 )
     return rc;

   rc= strcmp(thisName(table,a), thisName(table,b));
   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show
//
// Purpose-
//       Show a the symbol table address, value, qualifier and name
//
//----------------------------------------------------------------------------
static void
   show(                            // Show symbol's name and value
     Symtab&         table,         // The symbol table
     const Symval*   ptrSymbol)     // -> Symval
{
   debugf("%s Addr(%p) Value(0x%.8lX) Qual(%s) Symbol(%s)\n", __SOURCE__,
          ptrSymbol, ptrSymbol->addr,
          qualName(table, ptrSymbol),
          thisName(table, ptrSymbol)
          );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       showme
//
// Purpose-
//       Show a symbol's name and value.
//
//----------------------------------------------------------------------------
static void
   showme(                          // Show symbol's name and value
     const void*     qual,          // Symbol qualifier
     const char*     name)          // Symbol name
{
   Symval*           ptrSymbol;     // For display of symbol table

   ptrSymbol= (Symval*)symbolTable.locate(qual, name);

   if (ptrSymbol == NULL)
     debugf("%s Addr(%p) Value(0x%.8lX) Symbol '%s'\n", __SOURCE__,
            ptrSymbol, (long)(0), name);
   else
     debugf("%s Addr(%p) Value(0x%.8lX) Symbol '%s'\n", __SOURCE__,
            ptrSymbol, ptrSymbol->addr, name);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       iterateInOrder
//
// Purpose-
//       Test symbol table iterator.
//
//----------------------------------------------------------------------------
static void
   iterateInOrder(                  // Test symbol table iterator
     Symtab&         table)         // The symbol table
{
   SymtabIterator    iter;          // My iterator
   Symval*           curr;          // My element
   Symval*           elem;          // My element
   Symval*           next;          // My element
   int               rc;            // Called routine return code

   debugf("Symbol table in order\n");
   curr= NULL;
   for(iter.begin(table); iter.isValid(); iter.next() )
   {
     elem= (Symval*)iter.current();
     if( curr == NULL )
     {
       curr= elem;
       continue;
     }

     rc= compName(table, curr, elem);
     if( rc > 0 )
       curr= elem;
   }

   show(table, curr);
   for(;;)
   {
     next= NULL;
     for(iter.begin(table); iter.isValid(); iter.next() )
     {
       elem= (Symval*)iter.current();
       rc= compName(table, curr, elem);
       if( rc >= 0 )
         continue;

       if( next == NULL )
       {
         next= elem;
         continue;
       }

       rc= compName(table, next, elem);
       if( rc > 0 )
         next= elem;
     }

     if( next == NULL )
       break;

     show(table, next);
     curr= next;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       iterate
//
// Purpose-
//       Test symbol table iterator.
//
//----------------------------------------------------------------------------
static void
   iterate(                         // Test symbol table iterator
     Symtab&         table)         // The symbol table
{
   SymtabIterator    iter;          // My iterator
   Symval*           elem;          // My element

   debugf("Symbol table iterator\n");
   for(iter.begin(table); iter.isValid(); iter.next() )
   {
     elem= (Symval*)iter.current();
     debugf("%s Addr(%p) Value(0x%.8lX) Symbol '%s'\n", __SOURCE__,
            elem, elem->addr, table.getSymbolName(elem) );
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count
//   char*           argv[])        // Argument array
{
   Symval            symbolValue;   // For construction of symbol table

   const void*       q1;            // Pointer to "qual1" symbol
   const void*       q2;            // Pointer to "qual2" symbol

   //-------------------------------------------------------------------------
   // Initialize debugging
   //-------------------------------------------------------------------------
   debugSetIntensiveMode();
   debugf("%s Started\n", __SOURCE__);

   //-------------------------------------------------------------------------
   // Construct the symbol table
   //-------------------------------------------------------------------------
   debugf("%s First Insert\n", __SOURCE__);
   symbolValue.addr= 0x12345678;
   q1= symbolTable.insert(NULL, "qualifier 1", &symbolValue);
   if (symbolTable.getIdent() != 0)
   {
     debugf("%s Insert error(%d)\n", __SOURCE__,
            symbolTable.getIdent());
     exit(EXIT_FAILURE);
   }

   showme(NULL, "qualifier 1");
   symbolValue.addr= 0xFE000001;
   symbolTable.insert(q1,   "FE", &symbolValue);
// return 0;

   symbolValue.addr= 0x87654321;
   q2= symbolTable.insert(q1,   "qualifier 2", &symbolValue);

   symbolValue.addr= 0x200000FE;
   symbolTable.insert(q2,   "FE", &symbolValue);

   symbolValue.addr= 0xdddddddd;
   symbolTable.insert(q2,   "D", &symbolValue);

   symbolValue.addr= 0xeeeeeeee;
   symbolTable.insert(q2,   "E", &symbolValue);

   symbolValue.addr= 0xffffffff;
   symbolTable.insert(q2,   "F", &symbolValue);
   symbolTable.insert(q2,   "F", &symbolValue);
   if (symbolTable.getIdent() != Symtab::EventDuplicateSymbol)
   {
     debugf("%s Expected error(%d), got(%d)\n", __SOURCE__,
            Symtab::EventDuplicateSymbol, symbolTable.getIdent());
   }

   symbolValue.addr= 0xaaaaaaaa;
   symbolTable.insert(q2,   "A", &symbolValue);

   symbolValue.addr= 0xbbbbbbbb;
   symbolTable.insert(q2,   "B", &symbolValue);

   symbolValue.addr= 0xcccccccc;
   symbolTable.insert(q2,   "C", &symbolValue);

   //-------------------------------------------------------------------------
   // See if it worked
   //-------------------------------------------------------------------------
   showme(NULL, "This Symbol Doesn't exist!");
   showme(NULL, "Next Symbol Doesn't exist!");
   showme(NULL, "qualifier 2");
   showme(NULL, "qualifier 1");
   showme(q1,   "qualifier 2");
   showme(q2,   "A");
   showme(q2,   "B");
   showme(q2,   "C");
   showme(q2,   "D");
   showme(q2,   "E");
   showme(q2,   "F");
   showme(q2,   "FE");
   showme(q1,   "FE");
   showme(q2,   "This Symbol Doesn't exist!");

   //-------------------------------------------------------------------------
   // Test symbol table iterator
   //-------------------------------------------------------------------------
   iterate(symbolTable);
   iterateInOrder(symbolTable);

   return 0;
}

