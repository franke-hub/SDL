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
//       NC__neu.cpp
//
// Purpose-
//       Neural Net Compiler: NEURON statement
//
// Last change date-
//       2007/01/01
//
// Neuron statement format-
//       NEURON {(TYPE)}
//           { {VALUE[expr]} | {STRING["string"]} }
//           {name{[dim]{[dim]...}}}
//           ;
//
//----------------------------------------------------------------------------
#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/istring.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_op.h"
#include "NC_sym.h"
#include "NC_sys.h"
#include "NN_com.h"

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC__NEU " // Source file, for debugging

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?
#include "NCdiag.h"                 // Diagnostic macros

//----------------------------------------------------------------------------
//
// Class-
//       NC_opNeuron
//
// Purpose-
//       The Neuron operation.
//
//----------------------------------------------------------------------------
class NC_opNeuron : public NC_op     // Neuron operator
{
//----------------------------------------------------------------------------
// NC_opNeuron::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opNeuron( void );            // Destructor
   NC_opNeuron( void );             // Constructor

//----------------------------------------------------------------------------
// NC_opNeuron::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opNeuron::Attributes
//----------------------------------------------------------------------------
public:
   NC_NeuronSymbol*    symbol;      // -> Symbol table entry
   NC_opFloat*         value;       // Value expression
}; // class NC_opNeuron

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static unsigned        defaultBound[32]= { // Default bounding array
   0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
   0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
   0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
   0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
   0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
   0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
   0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
   0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff
   };

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuron::~NC_opNeuron
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opNeuron::~NC_opNeuron( void )// Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuron::NC_opNeuron
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opNeuron::NC_opNeuron( void ) // Constructor
:  NC_op()
,  symbol(NULL)
,  value(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuron::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opNeuron::toStream(           // Write the object
     ostream&          os) const    // On this stream
{
   value->operate();

   os << "NC_op@(" << this << ") NEURON "
      << "Symbol(" << NC_COM.xst.getSymbolName(symbol) << ") "
      << "Value(" << value->getFloat() << ")\n";

   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuron::operate
//
// Purpose-
//       Neuron operation.
//
//----------------------------------------------------------------------------
void
   NC_opNeuron::operate( void )     // Perform Neuron operation
{
   NC_NeuronSymbol*    symbol= this->symbol; // Address the NeuronSymbol
   unsigned            count= symbol->count; // Number of elements

   Neuron*             ptrN;        // -> Neuron

   NN::FileId          fileId;      // File identifier
   NN::Offset          offset;      // Offset

   long                i;

   //-------------------------------------------------------------------------
   // Initialize the neuron(s) in the output file
   //-------------------------------------------------------------------------
   value->operate();                // Evaluate the Value

   fileId= symbol->addr.f;          // Get FileId
   offset= symbol->addr.o;          // Get Offset
   for(i=0; i<count; i++)           // Initialize the neuron(s)
   {
     ptrN= chg_neuron(fileId, offset);
     if( ptrN == NULL )             // If VPS error
     {
       NCmess(NC_msg::ID_VPSFault, 0);
       return;
     }

     memset(ptrN, 0, sizeof(Neuron));
     ptrN->cbid= Neuron::CBID;
     ptrN->type= symbol->subType;
     ptrN->value= value->getFloat();

     rel_neuron(fileId, offset);
     offset += sizeof(Neuron);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       extract
//
// Purpose-
//       Extract a parameter.
//
//----------------------------------------------------------------------------
static int
   extract(                         // Extract a parameter
     const char*       inpbuf,      // Current buffer
     int               inpndx,      // Current buffer index
     NC_opNeuron*      op)          // Pointer to operation
{
   NC_opFloat*         valueOp;     // Value operator
   NC_NeuronSymbol*    symbol;      // -> NeuronSymbol

   long                count;       // Number of elements
   unsigned            dim;         // Dimensionality
   NC_opFixed*         bound[NC_dim::MAX_DIM]; // Bounding element array
   int                 stmtix;      // Current statement index

   int                 i;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Initialize statement index

   //-------------------------------------------------------------------------
   // Handle neuron name, dimensionality 0
   //-------------------------------------------------------------------------
   if( inpbuf[stmtix] == ';' )      // If end of statement
     goto insertSymbol;

   //-------------------------------------------------------------------------
   // Handle VALUE clause
   //-------------------------------------------------------------------------
   if( stricmp(NC_COM.word0, "VALUE") == 0 ) // If VALUE clause
   {
     if( op->value != NULL )        // If duplicate VALUE clause
       NCmess(NC_msg::ID_FanDupClause, 1, "VALUE");

     valueOp= NC_opFloat::generate(inpbuf, stmtix);

     if( valueOp == NULL )
       return(ERR);

     op->value= valueOp;
     return(stmtix);
   }

   //-------------------------------------------------------------------------
   // Handle neuron name, dimensionality > 0
   //-------------------------------------------------------------------------
insertSymbol:
   if( NC_COM.word0[0] == '\0' )    // If no name was specified
   {
     NCmess(NC_msg::ID_NeuNoName, 0); // Syntax error
     return(ERR);
   }

   symbol= (NC_NeuronSymbol*)NC_COM.xst.insert(NC_sym::TypeNeuron,
                                               NC_COM.begroup,
                                               NC_COM.word0);
   if( symbol == NULL )
     return(ERR);

   //-------------------------------------------------------------------------
   // Generate dimensionality expressions
   //-------------------------------------------------------------------------
   dim= 0;
   for(i= 0; ; i++)                 // Extract the dimensions
   {
     stmtix= ncskipb(inpbuf, stmtix); // Skip over blanks
     if( inpbuf[stmtix] == ';' )    // If end of statement
       break;

     if( inpbuf[stmtix] != '[' )    // If not a new dimension
     {
       NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
       return(ERR);
     }

     if( i >= NC_sym::MAX_DIM )     // If too many dimensions
     {
       NCmess(NC_msg::ID_DimTooManyDim, 0);
       return(ERR);
     }

     bound[i]= NC_opFixed::generate(inpbuf, stmtix);
     if( bound[i] == NULL )
       return(ERR);
   }
   dim= i;                          // Number of dimensions
   symbol->dim= i;                  // Set dimensionality

   //-------------------------------------------------------------------------
   // Set the dimensionality array
   //-------------------------------------------------------------------------
   symbol->bound= defaultBound;     // Set default bounding array
   if( dim > 1 )
   {
     symbol->bound= (unsigned*)malloc(dim * sizeof(unsigned*));
     if( symbol->bound == NULL )
     {
       NCmess(NC_msg::ID_StgSkipStmt, 0);
       return(ERR);
     }
   }

   //-------------------------------------------------------------------------
   // Evaluate the dimensions
   //-------------------------------------------------------------------------
   count= 1;                        // Number of elements
   symbol->count= 1;                // (Number of elements)
   for(i= 0; i<dim; i++)            // Extract the dimensions
   {
     bound[i]->operate();           // Evaluate the expression
     symbol->bound[i]= bound[i]->getFixed(); // Set the bound
     delete bound[i];

     count *= symbol->bound[i];     // Count the elements in this dimension
     if( count < symbol->count )    // If overflow
     {
       NCmess(NC_msg::ID_DimTooManyElements, 0);
       return(ERR);
     }

     symbol->count= count;
   }

   //-------------------------------------------------------------------------
   // Set the symbol information
   //-------------------------------------------------------------------------
   symbol->addr.f= NC_COM.objfile->fileno;
   if( NC_COM.debug == NULL )
     symbol->fileName= "*UndefinedFile*";
   else if( NC_COM.debug->ifd == NULL )
     symbol->fileName= "*UndefinedFile*";
   else
   {
     symbol->fileName= NC_COM.debug->ifd->filenm;
     symbol->fileLine= NC_COM.debug->lineNumber;
   }

   //-------------------------------------------------------------------------
   // End of statement
   //-------------------------------------------------------------------------
   op->symbol= symbol;

   return(EOF);                     // Statement complete
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc__neu
//
// Purpose-
//       Process NEURON statement.
//
//----------------------------------------------------------------------------
extern void
   nc__neu(                         // Process NEURON statement
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   NC_opNeuron*        op= new NC_opNeuron(); // Generated Neuron operator

   Neuron::Type        type;        // Neuron type

   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Extract the neuron type
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Skip blanks
   type= Neuron::TypeDefault;       // Default, neuron[default]
   if( inpbuf[stmtix] == '(' )      // If not default
   {
     stmtix= ncstring(inpbuf, stmtix, NC_COM.word0, 256);
     if( stmtix == ERR_LENGTH )     // If length error
       NCmess(NC_msg::ID_SynSymbolTooLong, 1, NC_COM.word0);
     if( stmtix < 0 )               // If error or EOF
       return;

     makeUPPER(NC_COM.word0);       // Convert to upper case
     if( strcmp(NC_COM.word0, "DEFAULT") == 0 )
       type= Neuron::TypeDefault;
     else if( strcmp(NC_COM.word0, "ABS") == 0 )
       type= Neuron::TypeAbs;
     else if( strcmp(NC_COM.word0, "ADD") == 0 )
       type= Neuron::TypeAdd;
     else if( strcmp(NC_COM.word0, "AND") == 0 )
       type= Neuron::TypeAnd;
     else if( strcmp(NC_COM.word0, "CLOCK") == 0 )
       type= Neuron::TypeClock;
     else if( strcmp(NC_COM.word0, "CONSTANT") == 0 )
       type= Neuron::TypeConstant;
     else if( strcmp(NC_COM.word0, "DEC") == 0 )
       type= Neuron::TypeDec;
     else if( strcmp(NC_COM.word0, "DIV") == 0 )
       type= Neuron::TypeDiv;
     else if( strcmp(NC_COM.word0, "FILERD") == 0 )
       type= Neuron::TypeFileRD;
     else if( strcmp(NC_COM.word0, "FILEWR") == 0 )
       type= Neuron::TypeFileWR;
     else if( strcmp(NC_COM.word0, "IF") == 0 )
       type= Neuron::TypeIf;
     else if( strcmp(NC_COM.word0, "INC") == 0 )
       type= Neuron::TypeInc;
     else if( strcmp(NC_COM.word0, "MUL") == 0 )
       type= Neuron::TypeMul;
     else if( strcmp(NC_COM.word0, "NOP") == 0 )
       type= Neuron::TypeNop;
     else if( strcmp(NC_COM.word0, "OR") == 0 )
       type= Neuron::TypeOr;
     else if( strcmp(NC_COM.word0, "NAND") == 0 )
       type= Neuron::TypeNand;
     else if( strcmp(NC_COM.word0, "NOR") == 0 )
       type= Neuron::TypeNor;
     else if( strcmp(NC_COM.word0, "NEG") == 0 )
       type= Neuron::TypeNeg;
     else if( strcmp(NC_COM.word0, "SIGMOID") == 0 )
       type= Neuron::TypeSigmoid;
     else if( strcmp(NC_COM.word0, "STORE") == 0 )
       type= Neuron::TypeStore;
     else if( strcmp(NC_COM.word0, "SUB") == 0 )
       type= Neuron::TypeSub;
     else if( strcmp(NC_COM.word0, "TRAIN") == 0 )
       type= Neuron::TypeTrain;
     else if( strcmp(NC_COM.word0, "UNTIL") == 0 )
       type= Neuron::TypeUntil;
     else if( strcmp(NC_COM.word0, "WHILE") == 0 )
       type= Neuron::TypeWhile;
     else
     {
       NCmess(NC_msg::ID_NeuInvalid, 1, NC_COM.word0);
       return;
     }
   }

   //-------------------------------------------------------------------------
   // Extract the neuron parameters
   //-------------------------------------------------------------------------
   stmtix= ncnextw(inpbuf, stmtix, NC_COM.word0); // Extract next word
   stmtix= extract(inpbuf, stmtix, op); // Process the word
   while (stmtix != EOF && stmtix != ERR) // If more parameters remain
   {
     stmtix= ncnextw(inpbuf, stmtix, NC_COM.word0);
                                    // Extract next word
     stmtix= extract(inpbuf, stmtix, op); // Process the word
   }

   //-------------------------------------------------------------------------
   // If error, exit
   //-------------------------------------------------------------------------
   if( stmtix == ERR )              // If error
     return;                        // Exit
   if( NC_COM.objfile == NULL )     // If no file specified
   {
     NCmess(NC_msg::ID_SeqNoBeginFile, 0);
     return;
   }

   //-------------------------------------------------------------------------
   // Verify the VALUE clause
   //-------------------------------------------------------------------------
   if( op->value == NULL )           // If no VALUE clause
   {
     op->value= NC_opFloat::generate(0.0); // Generate default value
   }

   //-------------------------------------------------------------------------
   // Set the type
   //-------------------------------------------------------------------------
   op->symbol->subType= type;       // Set the Neuron type

   //-------------------------------------------------------------------------
   // Make this Neuron the default
   //-------------------------------------------------------------------------
   if( NC_COM.begroup == NULL )     // If no begin group active
   {
     NCmess(NC_msg::ID_BugFileLine, 2, __SOURCE__, "0001");
     return;
   }

   NC_COM.begroup->current_N= op->symbol; // Set the new default Neuron symbol

   //-------------------------------------------------------------------------
   // Add the operator onto the execution list
   //-------------------------------------------------------------------------
   #if( __BRINGUP__ )
     cout << *op;
     cout.flush();
   #endif
   NC_COM.pass2.fifo(op);
}

