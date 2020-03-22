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
//       NC_tab.h
//
// Purpose-
//       (NC) Neural Net Compiler: Symbol table.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_TAB_H_INCLUDED
#define NC_TAB_H_INCLUDED

#ifndef SYMTAB_H_INCLUDED
#include <com/Symtab.h>
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class NC_sym;

//----------------------------------------------------------------------------
//
// Class-
//       NC_tab
//
// Purpose-
//       Symbol table.
//
//----------------------------------------------------------------------------
class NC_tab : public Symtab        // Symbol table
{
//----------------------------------------------------------------------------
// NC_tab::Constructors and destructors
//----------------------------------------------------------------------------
public:
   ~NC_tab( void );                 // Destructor
   NC_tab(                          // Constructor
     int               vSize);      // Sizeof(Symbol)

//----------------------------------------------------------------------------
// NC_tab::Methods
//----------------------------------------------------------------------------
public:
void
   displayByAddr( void );           // Display the symbol table by address

void
   displayByName( void );           // Display the symbol table by name

NC_sym*                             // -> NC_sym (if valid)
   insert(                          // Make a NC_sym
     unsigned          type,        // With this type
     const void*       symbolQual,  // With this qualifier
     const char*       symbolName,  // With this name
     const void*       value = NULL); // -> Symbol value

NC_sym*                             // -> NC_sym (if extant)
   locate(                          // Locate a NC_sym
     const void*       symbolQual,  // With this qualifier
     const char*       symbolName); // With this name

NC_sym*                             // -> NC_sym (if extant)
   locate(                          // Locate a NC_sym
     const char*       symbolName); // With this (qualifier::)name

//----------------------------------------------------------------------------
// NC_tab::Attributes
//----------------------------------------------------------------------------
public:
   // None defined
}; // class NC_tab

#endif // NC_TAB_H_INCLUDED
