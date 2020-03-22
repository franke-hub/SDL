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
//       NC__fan.cpp
//
// Purpose-
//       Neural Net Compiler: FANIN statement
//
// Last change date-
//       2007/01/01
//
// Fanin statement format-
//       FANIN {({qual::}fetchName{[dim1]{[dim2]...}})}
//           {WEIGHT(expr)}
//           {qual::}storeName{[dim1]{[dim2]...}}
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

#include "hcdm.h"
#include "NC_com.h"
#include "NC_dim.h"
#include "NC_op.h"
#include "NC_sym.h"
#include "NC_sys.h"
#include "NN_com.h"

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC__FAN " // Source file, for debugging

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?
#include "NCdiag.h"                 // Diagnostic macros

//----------------------------------------------------------------------------
//
// Class-
//       NC_opFanin
//
// Purpose-
//       The Fanin operation.
//
//----------------------------------------------------------------------------
class NC_opFanin : public NC_op     // Fanin operator
{
//----------------------------------------------------------------------------
// NC_opFanin::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opFanin( void );             // Destructor
   NC_opFanin( void );              // Constructor

//----------------------------------------------------------------------------
// NC_opFanin::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opFanin::Attributes
//----------------------------------------------------------------------------
public:
   NC_opNeuronAddr*    fetch;       // -> Neuron being fetched
   NC_opNeuronAddr*    store;       // -> Neuron being updated
   NC_opFloat*         weight;      // Weight expression
}; // class NC_opFanin

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFanin::~NC_opFanin
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opFanin::~NC_opFanin( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFanin::NC_opFanin
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opFanin::NC_opFanin( void )   // Constructor
:  NC_op()
,  fetch(NULL)
,  store(NULL)
,  weight(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFanin::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opFanin::toStream(            // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this << ") FANIN\n"; os.flush();
   os << "..Fetch:" << *fetch;             os.flush();
   os << "..Store:" << *store;             os.flush();

   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFanin::operate
//
// Purpose-
//       Fanin operation.
//
//----------------------------------------------------------------------------
void
   NC_opFanin::operate( void )      // Perform Fanin operation
{
   Neuron*             ptrN;        // -> STORE Neuron
   Fanin*              ptrF;        // -> Fanin*

   NN::FileId          fileId;      // FileID(STORE Neuron)
   NN::Offset          offset;      // Fanin offset

   #if 0
     cout << "Fanin@(" << this << ").operate()\n"; cout.flush();
     cout << *this; cout.flush();
   #endif

   if( NC_COM.pass == NC_com::Pass2 )
     return;

   store->operate();
   fileId= store->getFileId();
   ptrN= chg_neuron(fileId, store->getOffset());
   if( ptrN == NULL )
   {
     NCmess(NC_msg::ID_VPSFault, 0);
     return;
   }
   switch(NC_COM.pass)
   {
     case NC_com::Pass3:            // Here we count the fanins
       ptrN->faninCount++;          // Count the fanin
       break;

     case NC_com::Pass4:            // Here we set the fanins
       offset= Fanin::index(ptrN->faninVaddr, ptrN->faninCount);
       ptrF= chg_fanin(fileId, offset);
       if( ptrF == NULL )
       {
         NCmess(NC_msg::ID_VPSFault, 0);
         return;
       }

       fetch->operate();
       weight->operate();
       ptrF->fileId= fetch->getFileId();
       ptrF->neuron= fetch->getOffset();
       ptrF->weight= weight->getFloat();

       BRINGUP( cout << "Neuron(" << fileId << ":" << store->getOffset() << ") "
                     << "FaninX(" << ptrN->faninCount << ") "
                     << "Fetch(" << ptrF->neuron.f << ":"
                                 << ptrF->neuron.o << ") "
                     << "Weight(" << ptrF->weight << ")\n";
              )

       rel_fanin(fileId, offset);

       ptrN->faninCount++;
       break;

     default:
       fprintf(stderr, "Error Data(%d)\n", NC_COM.pass);
       NCfault(__SOURCE__, __LINE__);
       break;
   }

   rel_neuron(fileId, store->getOffset());
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
     NC_opFanin*       op)          // -> FANIN operator
{
   NC_opFloat*         weightOp;    // Weight operator

   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   stmtix= inpndx;                  // Initialize statement index

   //-------------------------------------------------------------------------
   // Handle STORE name, dimensionality 0
   //-------------------------------------------------------------------------
   if( inpbuf[stmtix] == ';' )      // If end of statement
     goto locateSymbol;

   //-------------------------------------------------------------------------
   // Handle WEIGHT clause
   //-------------------------------------------------------------------------
   if( stricmp(NC_COM.word0, "WEIGHT") == 0 ) // If WEIGHT clause
   {
     if( op->weight != NULL )       // If duplicate WEIGHT clause
       NCmess(NC_msg::ID_FanDupClause, 1, "WEIGHT");

     weightOp= NC_opFloat::generate(inpbuf, stmtix);

     if( weightOp == NULL )
       return(ERR);

     op->weight= weightOp;
     return(stmtix);
   }

   //-------------------------------------------------------------------------
   // Handle STORE name, dimensionality > 0
   //-------------------------------------------------------------------------
locateSymbol:
   if( NC_COM.word0[0] == '\0' )    // If no name was specified
     return(EOF);                   // (Use default)

   strcpy(NC_COM.exprbuff, "(");
   strcat(NC_COM.exprbuff, NC_COM.word0);
   strcat(NC_COM.exprbuff, &inpbuf[stmtix]);
   NC_COM.exprbuff[strlen(NC_COM.exprbuff)-1]= ')'; // Convert ';' to ')'

   stmtix=0;
   op->store= NC_opNeuronAddr::generate(NC_COM.exprbuff, stmtix);
   if( op->store == NULL )
     return(ERR);

   return(EOF);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc__fan
//
// Purpose-
//       Process FANIN statement.
//
//----------------------------------------------------------------------------
extern void
   nc__fan(                         // Process FANIN statement
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   NC_opFanin*         op= new NC_opFanin(); // Generated Fanin operator

   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Add a debug operator onto the execution list
   //-------------------------------------------------------------------------
   NC_COM.passN.fifo(NC_opDebug::generate());

   //-------------------------------------------------------------------------
   // Extract the neuron name
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Skip blanks
   if( inpbuf[stmtix] == '(' )      // If not default
   {
     op->fetch= NC_opNeuronAddr::generate(inpbuf, stmtix);
     if( op->fetch == NULL )
       return;
   }

   //-------------------------------------------------------------------------
   // Extract the fanin parameters
   //-------------------------------------------------------------------------
   stmtix= ncnextw(inpbuf, stmtix, NC_COM.word0); // Extract next word
   stmtix= extract(inpbuf, stmtix, op); // Process the word
   while (stmtix != EOF && stmtix != ERR) // If more parameters remain
   {
     stmtix= ncnextw(inpbuf, stmtix, NC_COM.word0); // Extract next word
     stmtix= extract(inpbuf, stmtix, op); // Process the word
   }

   //-------------------------------------------------------------------------
   // If error, exit
   //-------------------------------------------------------------------------
   if( stmtix == ERR )              // If error
     return;                        // Exit

   //-------------------------------------------------------------------------
   // Verify the FETCH neuron
   //-------------------------------------------------------------------------
   if( op->fetch == NULL )          // If the FETCH neuron is omitted
   {
     if( NC_COM.begroup == NULL )   // If no file specified
     {
       NCmess(NC_msg::ID_SeqNoBegin, 0);
       return;
     }
     if( NC_COM.begroup->current_N == NULL ) // If no default neuron exists
     {
       NCmess(NC_msg::ID_SeqNoNeuron, 0);
       return;
     }

     op->fetch= NC_opNeuronAddr::generate(NC_COM.begroup->current_N);
   }

   //-------------------------------------------------------------------------
   // Verify the STORE neuron
   //-------------------------------------------------------------------------
   if( op->store == NULL )          // If the STORE neuron is omitted
   {
     if( NC_COM.begroup == NULL )   // If no file specified
     {
       NCmess(NC_msg::ID_SeqNoBegin, 0);
       return;
     }
     if( NC_COM.begroup->current_N == NULL ) // If no default neuron exists
     {
       NCmess(NC_msg::ID_SeqNoNeuron, 0);
       return;
     }

     op->store= NC_opNeuronAddr::generate(NC_COM.begroup->current_N);
   }

   //-------------------------------------------------------------------------
   // Verify the WEIGHT clause
   //-------------------------------------------------------------------------
   if( op->weight == NULL )          // If no WEIGHT clause
     op->weight= NC_opFloat::generate(1.0); // Generate default weight

   //-------------------------------------------------------------------------
   // Add the operator onto the execution list
   //-------------------------------------------------------------------------
   #if( __BRINGUP__ )
     cout << *op;
     cout.flush();
   #endif
   NC_COM.passN.fifo(op);
}

