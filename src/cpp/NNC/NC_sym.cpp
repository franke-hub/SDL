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
//       NC_sym.cpp
//
// Purpose-
//       Neural Net Compiler - Symbol object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
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

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC_sym  " // Source file, for debugging

//----------------------------------------------------------------------------
// Error identifiers
//----------------------------------------------------------------------------
#define ID_INVALID_TYPE       "001" // Invalid type

//----------------------------------------------------------------------------
// Symbol type table
//----------------------------------------------------------------------------
static const char*   typeName[]=
{
   "Error",
   "Name",
   "Value",
   "Const",
   "Label",
   "Gosub",
   "Begin",
   "Do",
   "Neuron"
};

//----------------------------------------------------------------------------
//
// Method-
//       NC_GroupSymbol::~NC_GroupSymbol
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_GroupSymbol::~NC_GroupSymbol( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_GroupSymbol::NC_GroupSymbol
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_GroupSymbol::NC_GroupSymbol( void ) // Constructor
:  NC_sym(), SHSL_List<NC_GroupSymbol>::Link()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_BeGroupSymbol::~NC_BeGroupSymbol
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_BeGroupSymbol::~NC_BeGroupSymbol( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_BeGroupSymbol::NC_BeGroupSymbol
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_BeGroupSymbol::NC_BeGroupSymbol( void ) // Constructor
:  NC_GroupSymbol()
,  ofd(NULL)
,  current_G(NULL)
,  current_N(NULL)
{
   type= TypeBeGroup;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_DoGroupSymbol::~NC_DoGroupSymbol
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_DoGroupSymbol::~NC_DoGroupSymbol( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_DoGroupSymbol::NC_DoGroupSymbol
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_DoGroupSymbol::NC_DoGroupSymbol( void ) // Constructor
:  NC_GroupSymbol()
{
   type= TypeDoGroup;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_sym::nameIsValid
//
// Purpose-
//       Validate a symbol name.
//
//----------------------------------------------------------------------------
int                                 // TRUE if name is valid
   NC_sym::nameIsValid(             // Is symbol name valid?
     const char*       symbolName)  // The name to validate
{
   int                 i;

   assert( symbolName != NULL );

   if (symbolName[0] != '_' && !isalpha(symbolName[0]))
     return(FALSE);

   for(i=1; symbolName[i] != '\0'; i++)
   {
     if (symbolName[i] != '_' && !isalnum(symbolName[i]))
       return(FALSE);
   }

   return(TRUE);
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_sym::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_sym::toStream(                // Write the object
     ostream&          os) const    // On this stream
{
// const char*         C;

   os << "NC_sym@(" << this << ")";
   os << "{";

   if( type >= TypeNumberOfTypes )
     os << "Error(" << (int)type << ")" ;
   else
     os << typeName[type];

// C= NC_COM.xst.getSymbolName(this);
   switch(type)
   {
//   TypeName:
//     os << ",Name("  << C << ")";
//     break;

//   TypeLabel:
//     os << ",Label@(" << ((void*)0) << ")";
//     break;

//   TypeGosub:
//     os << ",Gosub@(" << ((void*)0) << ")";
//     break;

//   TypeBeGroup:
//     os << ",Name("  << C << ")";
//     os << ",BeGroup@(" << ((NC_BeGroupSymbol*)this) << ")";
//     break;

//   TypeDoGroup:
//     os << ",Name("  << C << ")";
//     os << ",DoGroup@(" << ((NC_DoGroupSymbol*)this) << ")";
//     break;

//   TypeNeuron:
//     os << ",Name("  << C << ")";
//     os << ",Neuron("
//        << "F(" << ((NC_NeuronSymbol*)this)->addr.f  << ")"
//        << ",O(" << ((NC_NeuronSymbol*)this)->addr.o << ")"
//        << ")";
//     break;

     default:
       os << ",Error(" << (int)type << ")" ;
       break;
   }

   os << "}" ;
   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_NeuronSymbol::cmpAddr
//
// Purpose-
//       Compare addresses.
//
//----------------------------------------------------------------------------
int                                 // Resultant (-1, =0, +1)
   NC_NeuronSymbol::cmpAddr(        // Compare address
     NC_NeuronSymbol*  other) const // With this NeuronSymbol
{
   if( other == NULL )
     return(+1);

   if( this->addr.f < other->addr.f )
     return (-1);
   if( this->addr.f > other->addr.f )
     return (+1);

   if( this->addr.o < other->addr.o )
     return (-1);
   if( this->addr.o > other->addr.o )
     return (+1);

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_NeuronSymbol::cmpName
//
// Purpose-
//       Compare names.
//
//----------------------------------------------------------------------------
int                                 // Resultant (-1, =0, +1)
   NC_NeuronSymbol::cmpName(        // Compare names
     NC_NeuronSymbol*  other) const // With this NeuronSymbol
{
   NC_tab*             const table= &NC_COM.xst;

   NC_BeGroupSymbol*   ptrQ;          // Current qualifier symbol
   NC_BeGroupSymbol*   hisQual[MAX_QUAL]; // Qualifier symbol array
   NC_BeGroupSymbol*   ourQual[MAX_QUAL]; // Qualifier symbol array

   unsigned            hisQualIndex= 0;
   unsigned            ourQualIndex= 0;

   if( other == NULL )
     return(+1);

   if( table->getSymbolQual(this) ==
       table->getSymbolQual(other) )
   {
     return strcmp(table->getSymbolName(this),
                   table->getSymbolName(other));
   }

   ptrQ= (NC_BeGroupSymbol*)table->getSymbolQual(this);
   while( ptrQ != NULL )
   {
     ptrQ= ptrQ->current_G;
     if( ptrQ == NULL )
       break;

     if( ourQualIndex > MAX_QUAL )
     {
       NCmess(NC_msg::ID_FixQualifierCount, 1,
              table->getSymbolName(this));
       return (-1);
     }

     ourQual[ourQualIndex++]= ptrQ;
     ptrQ= (NC_BeGroupSymbol*)table->getSymbolQual(ptrQ);
   }

   ptrQ= (NC_BeGroupSymbol*)table->getSymbolQual(other);
   while( ptrQ != NULL )
   {
     ptrQ= ptrQ->current_G;
     if( ptrQ == NULL )
       break;

     if( hisQualIndex > MAX_QUAL )
     {
       NCmess(NC_msg::ID_FixQualifierCount, 1,
              table->getSymbolName(other));
       return (+1);
     }

     hisQual[hisQualIndex++]= ptrQ;
     ptrQ= (NC_BeGroupSymbol*)table->getSymbolQual(ptrQ);
   }

   for(;;)
   {
     if( ourQualIndex == 0 )
       return (+1);
     if( hisQualIndex == 0 )
       return (-1);

     ourQualIndex--;
     hisQualIndex--;
     if( ourQual[ourQualIndex] != hisQual[hisQualIndex] )
     {
       return strcmp(table->getSymbolName(ourQual[ourQualIndex]),
                     table->getSymbolName(hisQual[hisQualIndex]));
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_NeuronSymbol::toStream
//
// Purpose-
//       Display symbol name.
//
//----------------------------------------------------------------------------
void
   NC_NeuronSymbol::toStream(       // Display symbol
     const NC_tab*     table) const // -> Associated table
{
   NC_BeGroupSymbol*   ptrQ;        // Current qualifier symbol
   NC_BeGroupSymbol*   qualifier[MAX_QUAL]; // Qualifier symbol array
   int                 index= 0;    // Qualifier symbol index

   ptrQ= (NC_BeGroupSymbol*)table->getSymbolQual(this);
   while( ptrQ != NULL )
   {
     ptrQ= ptrQ->current_G;
     if( ptrQ == NULL )
       break;

     if( index >= MAX_QUAL )
     {
       NCmess(NC_msg::ID_FixQualifierCount, 1,
              table->getSymbolName(this));
       break;
     }

     qualifier[index++]= ptrQ;
     ptrQ= (NC_BeGroupSymbol*)table->getSymbolQual(ptrQ);
   }

   NC_COM.exprbuff[0]= '\0';
   index--;
   while( index > 0 )
   {
     index--;
     strcat(NC_COM.exprbuff, table->getSymbolName(qualifier[index]));
     strcat(NC_COM.exprbuff, "::");
   }
   strcat(NC_COM.exprbuff, table->getSymbolName(this));
   printf("%.2x:%.2x:%.8lx.%.8lx %-32s (%s %4ld)\n",
          addr.f, NN::PartNeuron, long(addr.o>>32), long(addr.o),
          NC_COM.exprbuff, fileName, fileLine);
}

