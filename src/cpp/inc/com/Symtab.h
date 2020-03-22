//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Symtab.h
//
// Purpose-
//       SYMbol TABle control.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SYMTAB_H_INCLUDED
#define SYMTAB_H_INCLUDED

#ifndef HANDLER_H_INCLUDED
#include "Handler.h"
#endif

#ifndef SUBPOOL_H_INCLUDED
#include "Subpool.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Symtab
//
// Purpose-
//       Symbol table.
//
//----------------------------------------------------------------------------
class Symtab : public Handler {     // Symbol table
   friend class SymtabIterator;
//----------------------------------------------------------------------------
// Symtab::Attributes
//----------------------------------------------------------------------------
private:
Subpool                subpool;     // Symbol space
int                    sSize;       // Sizeof(Symbol)
int                    tSize;       // Sizeof(Symbol) + sizeof(Prefix)
void**                 hash;        // -> Hash table array

//----------------------------------------------------------------------------
// Symtab::Symbol
//----------------------------------------------------------------------------
public:
struct Symbol                       // A symbol in the symbol table
{
}; // enum Symbol

//----------------------------------------------------------------------------
// Symtab::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum Events                         // Event values
{  EventNone= 0                     // No event
,  EventNotFound                    // Symbol not found
,  EventDuplicateSymbol             // Duplicate symbol
,  EventSymbolLength                // Invalid symbol length
,  EventNoStorage                   // Symbol table full
}; // enum Events

//----------------------------------------------------------------------------
// Symtab::Constructors
//----------------------------------------------------------------------------
public:
   ~Symtab( void );                 // Destructor

   Symtab(                          // Constructor
     int               vSize);      // Sizeof(Symbol)

//----------------------------------------------------------------------------
// Symtab::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Diagnostic debugging display

//----------------------------------------------------------------------------
// Symtab::Methods
//----------------------------------------------------------------------------
public:
const char*                         // -> Symbol name
   getSymbolName(                   // Get the symbol name
     const Symbol*     ptrSymbol) const; // For this symbol

const void*                         // -> Symbol qualifier
   getSymbolQual(                   // Get the symbol qualifier
     const Symbol*     ptrSymbol) const; // For this symbol

const Symbol*                       // -> Inserted Symbol
   insert(                          // Insert Symbol into table
     const void*       qual,        // -> Qualifier
     const char*       name,        // -> Symbol name
     const void*       value = NULL); // -> Symbol value

const Symbol*                       // -> Symbol
   locate(                          // Locate Symbol in table
     const void*       qual,        // -> Qualifier
     const char*       name);       // -> Symbol name

const Symbol*                       // -> Symbol
   replace(                         // Replace Symbol value
     const void*       qual,        // -> Qualifier
     const char*       name,        // -> Symbol name
     const void*       value);      // -> (New) Symbol value
}; // class Symtab

//----------------------------------------------------------------------------
//
// Class-
//       SymtabIterator
//
// Purpose-
//       Symbol table iterator.
//
//----------------------------------------------------------------------------
class SymtabIterator {               // Symbol table iterator
//----------------------------------------------------------------------------
// SymtabIterator::Attributes
//----------------------------------------------------------------------------
protected:
const Symtab*          symtab;      // The symbol table (NULL if complete)
void*                  symbol;      // The current symbol

//----------------------------------------------------------------------------
// SymtabIterator::Constructors
//----------------------------------------------------------------------------
public:
   ~SymtabIterator( void ) {}       // Destructor

   SymtabIterator( void );          // Constructor

   SymtabIterator(                  // Constructor
     const Symtab&   source);       // Source Symtab

//----------------------------------------------------------------------------
// SymtabIterator::Methods
//----------------------------------------------------------------------------
public:
const Symtab::Symbol*               // The current symbol
   current( void ) const;           // Retrieve the current symbol

void
   begin(                           // Reset the Iterator
     const Symtab&   source);       // Using this Symtab

int                                 // TRUE if current() is valid
   isValid( void ) const;           // Is the Iterator finished?

void
   next( void );                    // Position at the next element
}; // class SymtabIterator

#endif                              // SYMTAB_H_INCLUDED
