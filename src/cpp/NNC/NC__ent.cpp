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
//       NC__ent.cpp
//
// Purpose-
//       Neural Net Compiler: ENTRY statement
//
// Last change date-
//       2007/01/01
//
// Entry statement format-
//       ENTRY {[{qual:}name{[dim1]{[dim2]...}}]} ;
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
#include <com/syslib.h>

#include "hcdm.h"
#include "NC_com.h"
#include "NC_op.h"
#include "NC_sym.h"
#include "NC_sys.h"

#include "NN_com.h"
#include "NN_psv.h"

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC__ENT " // Source file, for debugging

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?
#include "NCdiag.h"                 // Diagnostic macros

//----------------------------------------------------------------------------
//
// Class-
//       NC_opEntry
//
// Purpose-
//       The Entry operation.
//
//----------------------------------------------------------------------------
class NC_opEntry : public NC_op     // Entry operator
{
//----------------------------------------------------------------------------
// NC_opEntry::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opEntry( void );             // Destructor
   NC_opEntry( void );              // Constructor

//----------------------------------------------------------------------------
// NC_opEntry::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opEntry::Attributes
//----------------------------------------------------------------------------
public:
   NC_opNeuronAddr*    into;        // -> Entry Neuron
}; // class NC_opEntry

//----------------------------------------------------------------------------
//
// Method-
//       NC_opEntry::~NC_opEntry
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opEntry::~NC_opEntry( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opEntry::NC_opEntry
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opEntry::NC_opEntry( void )   // Constructor
:  NC_op()
,  into(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opEntry::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opEntry::toStream(            // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this << ") "
      << "Entry(" << into << ")\n"
      << *into;

   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opEntry::operate
//
// Purpose-
//       Entry operation.
//
//----------------------------------------------------------------------------
void
   NC_opEntry::operate( void )      // Perform Entry operation
{
   struct NN_psv*      ptrpsv;      // -> Process State Vector

   //-------------------------------------------------------------------------
   // Set the process state vector
   //-------------------------------------------------------------------------
   into->operate();                 // Resolve the Neuron address

   ptrpsv= (struct NN_psv*)nnuchg(PSV_FILE, PSV_PART, PSV_OFFSET);
                                    // Address the state vector
   if (ptrpsv == NULL)              // If VPS error
   {
     NCmess(NC_msg::ID_VPSFault, 0);
     return;
   }

   memcpy(ptrpsv->psvcbid, PSV_CBID, sizeof(ptrpsv->psvcbid)); // Set CBID
   ptrpsv->psvfileno= into->getFileId();
   ptrpsv->psvpartno= NN::PartNeuron;
   ptrpsv->psvoffset= into->getOffset();
   ptrpsv->clock= 0;
   ptrpsv->train= 0;

   BRINGUP( cout << "OPERATE: " << *this
                 << "File(" << ptrpsv->psvfileno << ") "
                 << "Offset(" << ptrpsv->psvoffset << ")\n"
          );

   nnurel(PSV_FILE, PSV_PART, PSV_OFFSET); // Release the entry
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
     NC_opEntry*       op)          // -> ENTRY operator
{
   //-------------------------------------------------------------------------
   // Handle End of statement
   //-------------------------------------------------------------------------
   if( inpbuf[inpndx] == ';' )      // If end of statement
   {
     return(EOF);                   // Statement complete
   }

   //-------------------------------------------------------------------------
   // Handle invalid clause
   //-------------------------------------------------------------------------
   NCmess(NC_msg::ID_SynGeneric, 0);// Syntax error
   return(ERR);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc__ent
//
// Purpose-
//       Process Entry statement.
//
//----------------------------------------------------------------------------
extern void
   nc__ent(                         // Process Entry statement
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   NC_opEntry*         op= new NC_opEntry(); // Generated Entry operator

   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Extract the neuron name
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Skip blanks
   if( inpbuf[stmtix] == '(' )      // If not default
   {
     op->into= NC_opNeuronAddr::generate(inpbuf, stmtix);
     if( op->into == NULL )
       return;
   }

   //-------------------------------------------------------------------------
   // Extract the Entry parameters
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
   // Verify the INTO neuron
   //-------------------------------------------------------------------------
   if( op->into == NULL )           // If the INTO neuron is omitted
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

     op->into= NC_opNeuronAddr::generate(NC_COM.begroup->current_N);
   }

   //-------------------------------------------------------------------------
   // Add the operator onto the execution list
   //-------------------------------------------------------------------------
   if( NC_COM.initial_N == TRUE )
     NCmess(NC_msg::ID_EntDuplicate, 0);

   NC_COM.initial_N= TRUE;
   NC_COM.pass2.fifo(op);
}

