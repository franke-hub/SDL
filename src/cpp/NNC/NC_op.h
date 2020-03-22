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
//       NC_op.h
//
// Purpose-
//       (NC) Neural Net Compiler: Operator table.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_OP_H_INCLUDED
#define NC_OP_H_INCLUDED

#include <ostream>
using std::ostream;

#ifndef LIST_H_INCLUDED
#include <com/List.h>
#endif

#ifndef NC_DIM_H_INCLUDED
#include "NC_dim.h"
#endif

#ifndef NN_H_INCLUDED
#include "NN.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct NC_ifd;

class NC_BeGroupSymbol;
class NC_FixedSymbol;
class NC_FloatSymbol;
class NC_NeuronSymbol;
class NC_sym;

class NC_OpFixed;
class NC_OpFloat;

//----------------------------------------------------------------------------
//
// Class-
//       NC_op
//
// Purpose-
//       Operator
//
//----------------------------------------------------------------------------
class NC_op : public DHSL_List<NC_op>::Link { // Operator
//----------------------------------------------------------------------------
// NC_op::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_op( void );                  // Destructor
   NC_op( void );                   // Constructor

//----------------------------------------------------------------------------
// NC_op::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_op::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class NC_op

//----------------------------------------------------------------------------
//
// Class-
//       NC_opArith
//
// Purpose-
//       Operator, returns arithmetic resultant.
//
//----------------------------------------------------------------------------
class NC_opArith : public NC_op     // Arithmetic Operator
{
//----------------------------------------------------------------------------
// NC_opArith::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum Op                             // Operation code
{  OpErr= 0                         // Invalid operation code

   // Constant operator
,  OpConst                          // Resultant= constant

   // Symbol operators
,  OpGet                            // Resultant= VALUE(symbol)
,  OpSet                            // VALUE(symbol)= Resultant

   // Unary operators
,  OpNegate                         // Resultant= -(operand[0])
,  OpInc                            // Resultant= operand[0] + 1
,  OpDec                            // Resultant= operand[0] - 1
,  OpFixed                          // Resultant= Fixed(operand[0])
,  OpFloat                          // Resultant= Float(operand[0])

   // Binary operators
,  OpAdd                            // Resultant= operand[0] + operand[1]
,  OpSub                            // Resultant= operand[0] - operand[1]
,  OpMul                            // Resultant= operand[0] * operand[1]
,  OpDiv                            // Resultant= operand[0] / operand[1]

,  OpCOUNT                          // Number of operation codes

   // Special operation codes (not part of count)
,  OP_LHP= OpCOUNT                  // Left Hand Parenthesis
,  OP_RHP                           // Right Hand Parenthesis
,  OP_RHB                           // Right Hand Bracket
,  OP_EOS                           // End of Statement
}; // enum Op
//----------------------------------------------------------------------------
// NC_opArith::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opArith( void );             // Destructor

protected:                          // Cannot be generated directly
   NC_opArith( void );              // Constructor

//----------------------------------------------------------------------------
// NC_opArith::Generator
//----------------------------------------------------------------------------
public:
static NC_opArith*                  // Resultant operator
   generate(                        // Generate a Fixed/Float expression
     const char*       inpbuf,      // Current buffer
     int&              inpndx);     // Running buffer index

static NC_opArith*                  // Resultant operator
   generate(                        // Generate a Fixed/Float expression
     NC_sym*           symbol);     // Symbol source

static NC_opArith*                  // Resultant operator
   generate(                        // Generate a Fixed expression
     const int         value);      // Constant value

static NC_opArith*                  // Resultant operator
   generate(                        // Generate a Float expression
     const double      value);      // Constant value

//----------------------------------------------------------------------------
// NC_op::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // TRUE if NC_opFixed
   isFixed( void ) const;           // TRUE if NC_opFixed

virtual int                         // TRUE if NC_opFloat
   isFloat( void ) const;           // TRUE if NC_opFloat

//----------------------------------------------------------------------------
// NC_op::Attributes
//----------------------------------------------------------------------------
public:
   Op                  op;          // Operator
   NC_opArith*         operand[2];  // Operands

   NC_sym*             symbol;      // -> Symbol
}; // class NC_opArith

//----------------------------------------------------------------------------
//
// Class-
//       NC_opDebug
//
// Purpose-
//       Set File/Line/Column debugging information.
//
//----------------------------------------------------------------------------
class NC_opDebug : public NC_op     // Debug Operator
{
//----------------------------------------------------------------------------
// NC_opDebug::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opDebug( void );             // Destructor
   NC_opDebug( void );              // Constructor

//----------------------------------------------------------------------------
// NC_opDebug::Generator
//----------------------------------------------------------------------------
public:
static NC_opDebug*                  // Resultant operator
   generate( void );                // Generate a Debug expression

//----------------------------------------------------------------------------
// NC_opDebug::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opDebug::Attributes
//----------------------------------------------------------------------------
public:
   NC_ifd*             ifd;         // -> Input File Descriptor
   unsigned            lineNumber;  // Linenumber
   unsigned            column;      // Column
}; // class NC_opDebug

//----------------------------------------------------------------------------
//
// Class-
//       NC_opFixed
//
// Purpose-
//       Operator, returns integer resultant
//
//----------------------------------------------------------------------------
class NC_opFixed : public NC_opArith// Fixed Operator
{
//----------------------------------------------------------------------------
// NC_opFixed::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opFixed( void );             // Destructor
   NC_opFixed( void );              // Constructor

//----------------------------------------------------------------------------
// NC_opFixed::Generator
//----------------------------------------------------------------------------
public:
static NC_opFixed*                  // Resultant operator
   generate(                        // Generate a Fixed expression
     const char*       inpbuf,      // Current buffer
     int&              inpndx);     // Running buffer index

static NC_opFixed*                  // Resultant operator
   generate(                        // Generate a Fixed expression
     NC_FixedSymbol*   symbol);     // Symbol source

static NC_opFixed*                  // Resultant operator
   generate(                        // Generate a Fixed expression
     const int         value);      // Constant value

static NC_opFixed*                  // Resultant operator
   generate(                        // Generate a Fixed expression
     NC_opArith*       op);         // Symbol operation

//----------------------------------------------------------------------------
// NC_opFixed::Methods
//----------------------------------------------------------------------------
public:
inline int                          // Resultant
   getFixed( void ) const;          // Get resultant

virtual int                         // TRUE if NC_opFixed
   isFixed( void ) const;           // TRUE if NC_opFixed

virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opFixed::Attributes
//----------------------------------------------------------------------------
public:
   int                 resultant;   // Resultant
}; // class NC_opFixed

//----------------------------------------------------------------------------
//
// Class-
//       NC_opFloat
//
// Purpose-
//       Operator, returns double resultant
//
//----------------------------------------------------------------------------
class NC_opFloat : public NC_opArith// Float Operator
{
//----------------------------------------------------------------------------
// NC_opFloat::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opFloat( void );             // Destructor
   NC_opFloat( void );              // Constructor

//----------------------------------------------------------------------------
// NC_opFloat::Generator
//----------------------------------------------------------------------------
public:
static NC_opFloat*                  // Resultant operator
   generate(                        // Generate a Float expression
     const char*       inpbuf,      // Current buffer
     int&              inpndx);     // Running buffer index

static NC_opFloat*                  // Resultant operator
   generate(                        // Generate a Float expression
     NC_FloatSymbol*   symbol);     // Symbol resultant

static NC_opFloat*                  // Resultant operator
   generate(                        // Generate a Float expression
     const double      value);      // Constant value

static NC_opFloat*                  // Resultant operator
   generate(                        // Generate a Float expression
     NC_opArith*       op);         // Symbol operation

//----------------------------------------------------------------------------
// NC_opFloat::Methods
//----------------------------------------------------------------------------
public:
inline double                       // Resultant
   getFloat( void ) const;          // Get resultant

virtual int                         // TRUE if NC_opFloat
   isFloat( void ) const;           // TRUE if NC_opFloat

virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opFloat::Attributes
//----------------------------------------------------------------------------
protected:
   double              resultant;   // Resultant
}; // class NC_opFloat

//----------------------------------------------------------------------------
//
// Class-
//       NC_opFor
//
// Purpose-
//       The For operation.
//
//----------------------------------------------------------------------------
class NC_opFor : public NC_op       // Neuron operator
{
//----------------------------------------------------------------------------
// NC_opFor::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opFor( void );               // Destructor
   NC_opFor( void );                // Constructor

//----------------------------------------------------------------------------
// NC_opFor::Generator
//----------------------------------------------------------------------------
public:
static NC_opFor*                    // Resultant operator
   generate( void );                // Generate a For expression

//----------------------------------------------------------------------------
// NC_opFor::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opFor::Attributes
//----------------------------------------------------------------------------
public:
   NC_FixedSymbol*     symbol;      // -> Iterator symbol
   NC_opFixed*         initial;     // Initial value expression
   NC_opFixed*         final;       // Final value expression
   NC_opFixed*         increment;   // Increment expression

   NC_op*              stmt;        // (Compound) statement head
}; // class NC_opFor

//----------------------------------------------------------------------------
//
// Class-
//       NC_opGroup
//
// Purpose-
//       Set Group (for symbol resolution.)
//
//----------------------------------------------------------------------------
class NC_opGroup : public NC_op     // Group Operator
{
//----------------------------------------------------------------------------
// NC_opGroup::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opGroup( void );             // Destructor
   NC_opGroup( void );              // Constructor

//----------------------------------------------------------------------------
// NC_opGroup::Generator
//----------------------------------------------------------------------------
public:
static NC_opGroup*                  // Resultant operator
   generate( void );                // Generate a Group expression

//----------------------------------------------------------------------------
// NC_opGroup::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opGroup::Attributes
//----------------------------------------------------------------------------
public:
   NC_BeGroupSymbol* group;         // -> Group Descriptor
}; // class NC_opGroup

//----------------------------------------------------------------------------
//
// Class-
//       NC_opNeuronAddr
//
// Purpose-
//       Operator, returns NN::FO resultant
//
//----------------------------------------------------------------------------
class NC_opNeuronAddr : public NC_op // Address Operator
{
//----------------------------------------------------------------------------
// NC_opNeuronAddr::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opNeuronAddr( void );        // Destructor
   NC_opNeuronAddr( void );         // Constructor

//----------------------------------------------------------------------------
// NC_opNeuronAddr::Generator
//----------------------------------------------------------------------------
public:
static NC_opNeuronAddr*             // Resultant operator
   generate(                        // Generate a NeuronAddr expression
     const char*       inpbuf,      // Current buffer
     int&              inpndx);     // Running buffer index

static NC_opNeuronAddr*             // Resultant operator
   generate(                        // Generate a NeuronAddr expression
     NC_NeuronSymbol*  symbol);     // Current symbol, dimensionality 0

//----------------------------------------------------------------------------
// NC_opNeuronAddr::Methods
//----------------------------------------------------------------------------
public:
inline NN::FileId                   // Resultant.FileId
   getFileId( void ) const;         // Get resultant

inline NN::Offset                   // Resultant.Offset
   getOffset( void ) const;         // Get resultant

virtual void
   operate( void );                 // Perform operation

virtual ostream&

   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opNeuronAddr::Attributes
//----------------------------------------------------------------------------
protected:
   NN::FO              resultant;   // Resultant

public:
   NC_NeuronSymbol*    source;      // -> Source NeuronSymbol
   NC_opFixed*         bound[NC_dim::MAX_DIM]; // Bounding array
}; // class NC_opNeuronAddr

//----------------------------------------------------------------------------
//
// Class-
//       NC_opResolveNeuronAddr
//
// Purpose-
//       Handle deferred resolution of a symbol.
//
//----------------------------------------------------------------------------
class NC_opResolveNeuronAddr : public NC_op // Deferred name resolution
{
//----------------------------------------------------------------------------
// NC_opResolveNeuronAddr::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opResolveNeuronAddr( void ); // Destructor
   NC_opResolveNeuronAddr(          // Constructor
     NC_opNeuronAddr*  target,      // Target operator
     const char*       source,      // Source name
     unsigned          dim);        // Dimensionality

//----------------------------------------------------------------------------
// NC_opResolveNeuronAddr::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opResolveNeuronAddr::Attributes
//----------------------------------------------------------------------------
public:
   NC_opNeuronAddr*    target;      // -> NC_opNeuronAddr to be resolved
   char*               source;      // Source name
   unsigned            dim;         // Number of dimensions
}; // class NC_opResolveNeuronAddr

#include "NC_op.i"

#endif // NC_OP_INCLUDED
