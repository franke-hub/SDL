//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Symtab.cpp
//
// Purpose-
//       Symbol Table control functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include "com/Symtab.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define HASHNO                 2048 // Number of entries in a hash table

//----------------------------------------------------------------------------
//
// Struct-
//       SymbolPrefix
//
// Purpose-
//       Describe the symbol table entry prefix.
//
// True format of a Symbol-
//       *--------*---------
//       |  next* |       |
//       |--------|Prefix |
//       |  qual* |       |
//       |--------*----  tSize
//       |        |  |    |
//       /  Value / sSize |
//       |        |  |    |
//       |--------*---------
//       |        |
//       /  Name  /
//       |        |
//       *--------*---------
//
//----------------------------------------------------------------------------
struct SymbolPrefix {               // Symbol prefix
   SymbolPrefix*   next;            // -> Next SymbolPrefix
   const void*     qual;            // -> Qualifier symbol
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       hashf
//
// Purpose-
//       Hash function.
//
//----------------------------------------------------------------------------
static unsigned int                 // Hash value
   hashf(                           // Hash function
     const char*       name)        // -> Symbol name
{
   unsigned int        H;           // Hash class value

   H= 0;                            // Initialize
   while(*name != '\0')             // Compute hash value
   {
     H ^= H << 8;
     H ^= *name;
     name++;
   }

   H &= (HASHNO-1);                 // Truncate at HASHNO limit

   return H;                        // Return hash value
}

//----------------------------------------------------------------------------
//
// Method-
//       Symtab::~Symtab
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Symtab::~Symtab( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Symtab::Symtab
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Symtab::Symtab(                  // Constructor
     int               sSize)       // Sizeof(Symbol)
:  sSize(sSize)
,  tSize(sSize+sizeof(SymbolPrefix))
{
   hash= (void**)subpool.allocate(sizeof(SymbolPrefix*) * HASHNO); // Allocate hash table
   if( hash == NULL )               // If initialization failure
     throwf("Symtab::Symtab(%d), cannot initialize\n", sSize);
   memset(hash, 0, sizeof(SymbolPrefix*) * HASHNO);

   #ifdef HCDM
     debugSetIntensiveMode();
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Symtab::debug
//
// Purpose-
//       Diagnostic debugging display.
//
//----------------------------------------------------------------------------
void
   Symtab::debug( void ) const      // Diagnostic debugging display
{
   debugf("Symtab(%p)::debug\n", this);
   subpool.diagnosticDump();
   debugf("sSize(%d)\n", sSize);
   debugf("tSize(%d)\n", tSize);
   debugf("hash(%p)\n", hash);
   for(unsigned int H= 0; H<HASHNO; H++)
     debugf("[%4d] %p\n", H, hash[H]);
}

//----------------------------------------------------------------------------
//
// Method-
//       Symtab::getSymbolName
//
// Purpose-
//       Get the name of a Symbol.
//
//----------------------------------------------------------------------------
const char*                         // -> Symbol name
   Symtab::getSymbolName(           // Get the symbol name
     const Symbol*     ptrSymbol) const // For this symbol
{
   return (char*)ptrSymbol + sSize; // Return the symbol name
}

//----------------------------------------------------------------------------
//
// Method-
//       Symtab::getSymbolQual
//
// Purpose-
//       Get the qualifier for a Symbol
//
//----------------------------------------------------------------------------
const void*                         // -> Symbol qualifier
   Symtab::getSymbolQual(           // Get the symbol qualifier
     const Symbol*     ptrSymbol) const // For this symbol
{
   SymbolPrefix*       ptrPrefix;   // -> SymbolPrefix

   ptrPrefix= (SymbolPrefix*)ptrSymbol;
   ptrPrefix--;

   return ptrPrefix->qual;          // Return the symbol qualifier
}

//----------------------------------------------------------------------------
//
// Method-
//       Symtab::insert
//
// Purpose-
//       Insert symbol into table.
//
//----------------------------------------------------------------------------
const Symtab::Symbol*               // -> Inserted Symbol.value
   Symtab::insert(                  // Insert Symbol into table
     const void*       qual,        // -> Qualifier
     const char*       name,        // -> Symbol name
     const void*       value)       // -> Symbol value
{
   Symbol*             ptrSymbol;   // -> Symbol
   SymbolPrefix*       ptrPrefix;   // -> SymbolPrefix
   char*               S;           // -> Symbol name

   unsigned int        H;           // Hash class value
   int                 L;           // Length field

   //-------------------------------------------------------------------------
   // Debugging
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("Symtab(%p)::insert(%p,'%s',%p)\n", this, qual, name, value);
   #endif

   //-------------------------------------------------------------------------
   // Verify that the symbol does not already exist
   //-------------------------------------------------------------------------
   if( locate(qual, name) != NULL ) // If already in table
   {
     event(EventDuplicateSymbol);   // Duplicate symbol
     return NULL;
   }

   //-------------------------------------------------------------------------
   // Allocate and initialize the symbol
   //-------------------------------------------------------------------------
   L= strlen(name);                 // Get the symbol name length
   if( L <= 0 )                     // If length too small
   {
     event(EventSymbolLength);      // Invalid symbol length
     return NULL;
   }

   L += tSize;                      // Include space for Prefix and value
   ptrPrefix= (SymbolPrefix*)subpool.allocate(L+1); // Allocate symbol
   if( ptrPrefix == NULL )          // If storage shortage
     return NULL;

   ptrSymbol= (Symbol*)(ptrPrefix+1); // Address the Symbol
   if( value == NULL)               // If the symbol does not exist
     memset((char*)ptrSymbol, 0, sSize); // Zero the symbol
   else                             // If the symbol exists
     memcpy((char*)ptrSymbol, value, sSize); // Initialize the symbol value
   S= (char*)ptrSymbol + sSize;     // Address the symbol name
   strcpy(S, name);                 // Initialize the symbol name

   ptrPrefix->qual= (SymbolPrefix*)qual; // Set the qualifier

   //-------------------------------------------------------------------------
   // Add the symbol into the table
   //-------------------------------------------------------------------------
   H= hashf(name);                  // Compute the hash class
   ptrPrefix->next= (SymbolPrefix*)hash[H]; // Set the chain pointer
   hash[H]= (void*)ptrPrefix;       // Add the symbol onto the hash class

   setIdent(EventNone);             // Indicate success
   return ptrSymbol;
}

//----------------------------------------------------------------------------
//
// Method-
//       Symtab::locate
//
// Purpose-
//       Locate symbol in table.
//
//----------------------------------------------------------------------------
const Symtab::Symbol*               // -> Symbol.value
   Symtab::locate(                  // Locate symbol in table
     const void*       qual,        // -> Qualifier
     const char*       name)        // -> Symbol name
{
   Symbol*             ptrSymbol;   // -> Symbol
   SymbolPrefix*       ptrPrefix;   // -> Symbol prefix
   char*               S;           // Pointer to symbol name

   unsigned int        H;           // Hash class value

   //-------------------------------------------------------------------------
   // Debugging
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("Symtab(%p)::locate(%p,'%s')\n", this, qual, name);
   #endif

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   setIdent(EventNone);             // Default, no event

   //-------------------------------------------------------------------------
   // Search the hash class for the entry
   //-------------------------------------------------------------------------
   H= hashf(name);                  // Compute the hash class
   ptrPrefix= (SymbolPrefix*)hash[H]; // Set the initial symbol pointer
   while(ptrPrefix != NULL)         // Search the hash class
   {
     if( ptrPrefix->qual == qual )  // If the qualifiers match
     {
       ptrSymbol= (Symbol*)(ptrPrefix+1); // Address the symbol value
       S= (char*)ptrSymbol + sSize; // Address the symbol name
       if( strcmp(S, name) == 0 )   // If symbol found
         return ptrSymbol;          // Return its address
     }

     ptrPrefix= ptrPrefix->next;    // Address the next symbol in the class
   }

   event(EventNotFound);            // Symbol not found
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Symtab::replace
//
// Purpose-
//       Replace symbol value.
//
// Notes-
//       Event is initialized by locate()
//
//----------------------------------------------------------------------------
const Symtab::Symbol*               // -> Replacement Symbol.value
   Symtab::replace(                 // Replace symbol value
     const void*       qual,        // -> Qualifier
     const char*       name,        // -> Symbol name
     const void*       value)       // -> (New) Symbol value
{
   Symbol*             ptrSymbol;   // -> Symbol value

   //-------------------------------------------------------------------------
   // Debugging
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("Symtab(%p)::insert(%p,'%s',%p)\n", this, qual, name, value);
   #endif

   //-------------------------------------------------------------------------
   // Locate the symbol
   //-------------------------------------------------------------------------
   ptrSymbol= (Symbol*)locate(qual, name); // Locate the symbol
   if( ptrSymbol == NULL )          // If not in table
     return NULL;

   //-------------------------------------------------------------------------
   // Replace the symbol value
   //-------------------------------------------------------------------------
   memcpy((char*)ptrSymbol, value, sSize); // Replace the symbol value
   return ptrSymbol;                // Return the new address
}

//----------------------------------------------------------------------------
//
// Method-
//       SymtabIterator::SymtabIterator()
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   SymtabIterator::SymtabIterator(  // Constructor
     const Symtab&     source)      // From Symtab
:  symtab(NULL)
,  symbol(NULL)
{
   begin(source);
}

   SymtabIterator::SymtabIterator( void ) // Constructor
:  symtab(NULL)
,  symbol(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       SymtabIterator::current
//
// Purpose-
//       Retrieve the current element
//
//----------------------------------------------------------------------------
const Symtab::Symbol*               // The current element
   SymtabIterator::current( void ) const // Retrieve the current element
{
   SymbolPrefix*       ptrPrefix;   // -> SymbolPrefix

   if( symbol == NULL )
   {
     debugf("SymtabIterator::current() non-existent\n");
     return (Symtab::Symbol*)symbol;
   }

   ptrPrefix= (SymbolPrefix*)symbol;
   ptrPrefix++;

   return (Symtab::Symbol*)ptrPrefix; // Return the current element
}

//----------------------------------------------------------------------------
//
// Method-
//       SymtabIterator::begin
//
// Purpose-
//       Start the Iterator.
//
//----------------------------------------------------------------------------
void
   SymtabIterator::begin(           // Start the Iterator
     const Symtab&     source)      // Using this Symtab
{
   unsigned int        H;           // Hash class value
   SymbolPrefix*       ptrPrefix;   // -> SymbolPrefix

   symtab= &source;
   symbol= NULL;
   for(H=0; H<HASHNO; H++)          // Find the first symbol
   {
     ptrPrefix= (SymbolPrefix*)source.hash[H];
     if( ptrPrefix != NULL )
     {
       symbol= ptrPrefix;
       return;
     }
   }

   symtab= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       SymtabIterator::isValid
//
// Purpose-
//       Has the Iterator completed?
//
//----------------------------------------------------------------------------
int                                 // TRUE if current() is valid
   SymtabIterator::isValid( void ) const // Is the current element valid?
{
   return (symbol != NULL);         // TRUE if symbol is present
}

//----------------------------------------------------------------------------
//
// Method-
//       SymtabIterator::next
//
// Purpose-
//       Get the next Symbol.
//
//----------------------------------------------------------------------------
void
   SymtabIterator::next( void )     // Get the next symbol
{
   unsigned int        H;           // Hash class value
   SymbolPrefix*       oldPrefix;   // -> SymbolPrefix
   SymbolPrefix*       ptrPrefix;   // -> SymbolPrefix
   Symtab::Symbol*     ptrSymbol;   // -> Symbol

   if( symtab == NULL )             // If complete
     return;

   oldPrefix= (SymbolPrefix*)symbol;// The current symbol
   ptrPrefix= oldPrefix->next;      // The next symbol
   if( ptrPrefix != NULL )
   {
     symbol= ptrPrefix;
     return;
   }

   oldPrefix++;
   ptrSymbol= (Symtab::Symbol*)oldPrefix;
   for(H= hashf(symtab->getSymbolName(ptrSymbol))+1; H<HASHNO; H++)
   {
     ptrPrefix= (SymbolPrefix*)(symtab->hash[H]);
     if( ptrPrefix != NULL )
     {
       symbol= ptrPrefix;
       return;
     }
   }

   // Iterator complete
   symtab= NULL;
   symbol= NULL;
}

