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
//       NC_tab.cpp
//
// Purpose-
//       Neural Net Compiler - Symbol table.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_sym.h"
#include "NC_sys.h"
#include "NC_tab.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC_tab  " // Source file, for debugging

//----------------------------------------------------------------------------
//
// Method-
//       NC_tab::~NC_tab
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_tab::~NC_tab( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_tab::NC_tab
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_tab::NC_tab(                  // Constructor
     int               vSize)       // Sizeof(Symbol)
:  Symtab(vSize)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_tab::locate
//
// Purpose-
//       Locate a NC_sym.
//
// Notes-
//       If the name is not found in the specified scope,
//       the next higher level scope is searched.
//       This iterates until all scope levels have been examined.
//
//----------------------------------------------------------------------------
NC_sym*                             // -> NC_sym (if extant)
   NC_tab::locate(                  // Locate a NC_sym
     const void*       symbolQual,  // With this qualifier
     const char*       symbolName)  // -> symbolName
{
   NC_sym*             symbol;      // Resultant

   if( !NC_sym::nameIsValid(symbolName) )
   {
     NCmess(NC_msg::ID_SymName, 1, symbolName);
     return NULL;
   }

   symbol= (NC_sym*)Symtab::locate(symbolQual, symbolName);
   if( symbol != NULL )
     return symbol;

   for(;;)
   {
     if( symbolQual == NULL )
       break;

     symbolQual= ((NC_BeGroupSymbol*)symbolQual)->current_G;
     if( symbolQual == NULL )
       break;

     symbol= (NC_sym*)Symtab::locate(symbolQual, symbolName);
     if( symbol != NULL )
       return symbol;

     symbolQual= getSymbolQual((NC_sym*)symbolQual);
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_tab::locate
//
// Purpose-
//       Locate a NC_sym.
//
// Notes-
//       Scope specifiers may be present in the symbol name.
//
//----------------------------------------------------------------------------
NC_sym*                             // -> NC_sym (if extant)
   NC_tab::locate(                  // Locate a NC_sym
     const char*       symbolName)  // -> symbolName
{
   char                qualName[NC_com::WORK_SIZE]; // Qualifier name
   NC_BeGroupSymbol*   qualGroup;   // Current qualifier group
   const char*         tempName;    // Temporary name

   int                 stmtix;      // Symbol index

   if( symbolName[0] == ':' )
   {
     if( symbolName[1] != ':' )
     {
       NCmess(NC_msg::ID_SymName, 1, symbolName);
       return NULL;
     }
     stmtix= 2;

     qualGroup= (NC_BeGroupSymbol*)NC_COM.grpstak.getHead();
     while( qualGroup->getNext() != NULL )
     {
       qualGroup= (NC_BeGroupSymbol*)qualGroup->getNext();
     }
   }
   else
   {
     stmtix= ncnextw(symbolName, 0, qualName);
     if( qualName[0] == '\0' )
     {
       NCmess(NC_msg::ID_SymName, 1, symbolName);
       return NULL;
     }
     stmtix= ncskipb(symbolName, stmtix);

     if( symbolName[stmtix] == '\0' )
       return (NC_sym*)locate(NC_COM.begroup, symbolName);

     if( symbolName[stmtix] != ':' ||  symbolName[stmtix+1] != ':' )
     {
       NCmess(NC_msg::ID_SymName, 1, symbolName);
       return NULL;
     }
     stmtix += 2;                   // Skip over "::"

     qualGroup= (NC_BeGroupSymbol*)NC_COM.grpstak.getHead();
     while( qualGroup != NULL )
     {
       if( qualGroup->current_G == qualGroup )
       {
         tempName= NC_COM.ist.getSymbolName(qualGroup);
         if( strcmp(qualName,tempName) == 0 )
           break;
       }

       qualGroup= (NC_BeGroupSymbol*)qualGroup->getNext();
     }
     if( qualGroup == NULL )
       return NULL;
   }

   // Extract qualifiers
   for(;;)
   {
     stmtix= ncnextw(symbolName, stmtix, qualName);
     if( qualName[0] == '\0' )
     {
       NCmess(NC_msg::ID_SymName, 1, symbolName);
       return NULL;
     }
     stmtix= ncskipb(symbolName, stmtix);

     if( symbolName[stmtix] == '\0' )
       break;

     if( symbolName[stmtix] != ':' ||  symbolName[stmtix+1] != ':' )
     {
       NCmess(NC_msg::ID_SymName, 1, symbolName);
       return NULL;
     }
     stmtix += 2;                   // Skip over "::"

     qualGroup= (NC_BeGroupSymbol*)NC_COM.ist.locate(qualGroup,qualName);
     if( qualGroup == NULL )
       return NULL;
   }

   return (NC_sym*)locate(qualGroup, qualName);
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_tab::insert
//
// Purpose-
//       Construct a NC_sym of the given type.
//
//----------------------------------------------------------------------------
NC_sym*                             // -> NC_sym (if valid)
   NC_tab::insert(                  // Insert a NC_sym
     unsigned          type,        // With this type
     const void*       symbolQual,  // With this qualifier
     const char*       symbolName,  // -> symbolName
     const void*       value)       // -> Symbol value
{
   NC_sym*             ptrSymbol;

   if( !NC_sym::nameIsValid(symbolName) )
   {
     NCmess(NC_msg::ID_SymName, 1, symbolName);
     return NULL;
   }

   ptrSymbol= (NC_sym*)Symtab::insert(symbolQual, symbolName, value);
   if( ptrSymbol == NULL )
   {
     if( getIdent() == Symtab::EventDuplicateSymbol )
       NCmess(NC_msg::ID_SymDuplicate, 1, symbolName);
     else
       NCmess(NC_msg::ID_SymStorage, 1, symbolName);

     return NULL;
   }

   ptrSymbol->type= NC_sym::Type(type);
   return ptrSymbol;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_tab::displayByAddr
//
// Purpose-
//       Display the symbol table, sorted by address.
//
//----------------------------------------------------------------------------
void
   NC_tab::displayByAddr( void )    // Display symbol table, by address
{
   SymtabIterator      iterator;    // Symbol table iterator
   NC_NeuronSymbol*    symbol;      // -> Symbol table entry
   NC_NeuronSymbol*    oldSym;      // -> Symbol table entry
   NC_NeuronSymbol*    newSym;      // -> Symbol table entry

   oldSym= NULL;
   for(;;)
   {
     newSym= NULL;
     for(iterator.begin(*this); iterator.isValid(); iterator.next())
     {
       symbol= (NC_NeuronSymbol*)iterator.current();
       if( symbol->type != NC_sym::TypeNeuron )
         continue;

       if( symbol->cmpAddr(oldSym) > 0 )
       {
         if( newSym == NULL || newSym->cmpAddr(symbol) > 0 )
           newSym= symbol;
       }
     }
     if( newSym == NULL )
       break;

     newSym->toStream(this);

     oldSym= newSym;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_tab::displayByName
//
// Purpose-
//       Display the symbol table, sorted by name.
//
//----------------------------------------------------------------------------
void
   NC_tab::displayByName( void )    // Display symbol table, by name
{
   SymtabIterator      iterator;    // Symbol table iterator
   NC_NeuronSymbol*    symbol;      // -> Symbol table entry
   NC_NeuronSymbol*    oldSym;      // -> Symbol table entry
   NC_NeuronSymbol*    newSym;      // -> Symbol table entry

   oldSym= NULL;
   for(;;)
   {
     newSym= NULL;
     for(iterator.begin(*this); iterator.isValid(); iterator.next())
     {
       symbol= (NC_NeuronSymbol*)iterator.current();
       if( symbol->type != NC_sym::TypeNeuron )
         continue;

       if( symbol->cmpName(oldSym) > 0 )
       {
         if( newSym == NULL || newSym->cmpName(symbol) > 0 )
           newSym= symbol;
       }
     }
     if( newSym == NULL )
       break;

     newSym->toStream(this);

     oldSym= newSym;
   }
}

