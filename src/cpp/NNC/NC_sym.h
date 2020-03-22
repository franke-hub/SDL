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
//       NC_sym.h
//
// Purpose-
//       (NC) Neural Net Compiler: Symbol table entry.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_SYM_H_INCLUDED
#define NC_SYM_H_INCLUDED

#include <ostream>

#ifndef LIST_H_INCLUDED
#include <com/List.h>
#endif

#ifndef NC_TAB_H_INCLUDED
#include "NC_tab.h"
#endif

#ifndef NEURON_H_INCLUDED
#include "Neuron.h"
#endif

#ifndef NN_H_INCLUDED
#include "NN.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct NC_ifd;
struct NC_ofd;
struct NCstmt;

class NC_opFor;
class NC_tab;

//----------------------------------------------------------------------------
//
// Class-
//       NC_sym
//
// Purpose-
//       Symbol table entry.
//
//----------------------------------------------------------------------------
class NC_sym : public Symtab::Symbol { // Symbol table entry
//----------------------------------------------------------------------------
// NC_sym::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum
{
   MAX_DIM=                      32,// Maximum dimensionality
   MAX_QUAL=                     32 // Maximum number of qualifiers
}; // enum

enum Type                           // Symbol Type
{
   TypeError=                     0,// Invalid symbol type
   TypeFixed,                       // FixedSymbol
   TypeFloat,                       // FloatSymbol
   TypeBeGroup,                     // BeginGroupSymbol
   TypeDoGroup,                     // DoGroupSymbol
   TypeNeuron,                      // NeuronSymbol
   TypeNumberOfTypes                // Number of types, must be last
}; // enum Type

//----------------------------------------------------------------------------
// NC_sym::Constructors and destructors
//----------------------------------------------------------------------------
public:
   ~NC_sym( void ) {}               // Destructor
   NC_sym( void ) {}                // Constructor

//----------------------------------------------------------------------------
// NC_sym::Methods
//----------------------------------------------------------------------------
public:
ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

static int                          // TRUE if name is valid
   nameIsValid(                     // Is symbol name valid?
     const char*       symbolName); // The name to validate

//----------------------------------------------------------------------------
// NC_sym::Attributes
//----------------------------------------------------------------------------
public:
   Type                type;        // Type identifier
}; // class NC_sym

//----------------------------------------------------------------------------
//
// Class-
//       NC_FixedSymbol
//
// Purpose-
//       A FixedSymbol represents an integer variable.
//
//----------------------------------------------------------------------------
class NC_FixedSymbol : public NC_sym // Symbol table entry
{
//----------------------------------------------------------------------------
// NC_FixedSymbol::Attributes
//----------------------------------------------------------------------------
public:
   int                 value;       // Symbol value
}; // class NC_FixedSymbol

//----------------------------------------------------------------------------
//
// Class-
//       NC_FloatSymbol
//
// Purpose-
//       A FloatSymbol represents an double variable.
//
//----------------------------------------------------------------------------
class NC_FloatSymbol : public NC_sym // Symbol table entry
{
//----------------------------------------------------------------------------
// NC_FloatSymbol::Attributes
//----------------------------------------------------------------------------
public:
   double              value;       // Symbol value
}; // class NC_FloatSymbol

//----------------------------------------------------------------------------
//
// Class-
//       NC_GroupSymbol
//
// Purpose-
//       A GroupSymbol is a group symbol.
//
//----------------------------------------------------------------------------
class NC_GroupSymbol : public NC_sym, public SHSL_List<NC_GroupSymbol>::Link {
//----------------------------------------------------------------------------
// NC_GroupSymbol::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ~NC_GroupSymbol( void );         // Destructor
   NC_GroupSymbol( void );          // Constructor

//----------------------------------------------------------------------------
// NC_GroupSymbol::Attributes
//----------------------------------------------------------------------------
public:
   NC_ifd*             source;      // Origin file descriptor
   int                 lineno;      // Origin line number
   int                 column;      // Origin column number
}; // class NC_GroupSymbol

//----------------------------------------------------------------------------
//
// Class-
//       NC_BeGroupSymbol
//
// Purpose-
//       A BeGroupSymbol is a begin group symbol.
//
//----------------------------------------------------------------------------
class NC_BeGroupSymbol : public NC_GroupSymbol // BeGroupSymbol table entry
{
//----------------------------------------------------------------------------
// NC_BeGroupSymbol::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ~NC_BeGroupSymbol( void );       // Destructor
   NC_BeGroupSymbol( void );        // Constructor

//----------------------------------------------------------------------------
// NC_BeGroupSymbol::Attributes
//----------------------------------------------------------------------------
public:
   NC_ofd*             ofd;         // -> Output file descriptor,
                                    // if group is associated with
                                    // a BEGIN FILE statement
   NC_BeGroupSymbol*   current_G;   // -> BeGroup symbol table entry.
                                    // If this group has no name of
                                    // its own, it inherits the group
                                    // from the prior group.
   NC_NeuronSymbol*    current_N;   // -> Neuron symbol.
                                    // This is the default neuron for fanins.
}; // class NC_BeGroupSymbol

//----------------------------------------------------------------------------
//
// Class-
//       NC_DoGroupSymbol
//
// Purpose-
//       A DoGroupSymbol is a do group symbol.
//
//----------------------------------------------------------------------------
class NC_DoGroupSymbol : public NC_GroupSymbol // BeGroupSymbol table entry
{
//----------------------------------------------------------------------------
// NC_DoGroupSymbol::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ~NC_DoGroupSymbol( void );       // Destructor
   NC_DoGroupSymbol( void );        // Constructor

//----------------------------------------------------------------------------
// NC_DoGroupSymbol::Attributes
//----------------------------------------------------------------------------
public:
   NC_opFor*         op;            // Operator
}; // class NC_DoGroupSymbol

//----------------------------------------------------------------------------
//
// Class-
//       NC_NeuronSymbol
//
// Purpose-
//       Neuron Symbol table entry.
//
//----------------------------------------------------------------------------
class NC_NeuronSymbol : public NC_sym  // Symbol table entry
{
//----------------------------------------------------------------------------
// NC_NeuronSymbol::Constructors
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// NC_NeuronSymbol::Methods
//----------------------------------------------------------------------------
public:
int                                 // Resultant (-1, =0, +1)
   cmpAddr(                         // Compare address
     NC_NeuronSymbol*  other) const; // With this NeuronSymbol

int                                 // Resultant (-1, =0, +1)
   cmpName(                         // Compare name
     NC_NeuronSymbol*  other) const; // With this NeuronSymbol

void
   toStream(                        // Display name
     const NC_tab*     table) const; // -> Symbol table

//----------------------------------------------------------------------------
// NC_NeuronSymbol::Attributes
//----------------------------------------------------------------------------
public:
   unsigned char       defined    :1; // TRUE if symbol is defined
   unsigned char       referenced :1; // TRUE if symbol is referenced
   unsigned char                  :6; // Reserved for alignment/expansion
   unsigned char                  :8; // Reserved for alignment/expansion
   unsigned short      subType;     // Type(Neuron)
   unsigned short      dim;         // Number of dimensions
   unsigned*           bound;       // Dimensionality array

   NN::FO              addr;        // Base symbol address
   unsigned            count;       // Number of elements

   const char*         fileName;    // FileName where referenced/defined
   unsigned long       fileLine;    // FileLine where referenced/defined

   //-------------------------------------------------------------------------
   // Initializer
   //-------------------------------------------------------------------------
   NN::Value           value;       // Value
}; // class NC_NeuronSymbol

//----------------------------------------------------------------------------
//
// Struct-
//       NC_SizeofSymbol
//
// Purpose-
//       Define the largest Symbol entry.
//
//----------------------------------------------------------------------------
struct NC_SizeofSymbol              // The size of a symbol table entry
{
   union
   {
     char              fixedSymbol  [sizeof(NC_FixedSymbol)];
     char              floatSymbol  [sizeof(NC_FloatSymbol)];
     char              beGroupSymbol[sizeof(NC_BeGroupSymbol)];
     char              doGroupSymbol[sizeof(NC_DoGroupSymbol)];
     char              neuronSymbol [sizeof(NC_NeuronSymbol)];
   };
}; // struct NC_SizeofSymbol

#include "NC_sym.i"

#endif // NC_SYM_H_INCLUDED
