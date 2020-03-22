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
//       NC__end.cpp
//
// Purpose-
//       Neural Net Compiler: END statement
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
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_sym.h"
#include "NC_sys.h"

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC__END " // Source file, for debugging

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // BRINGUP mode? (for debugging)
#include "NCdiag.h"

//----------------------------------------------------------------------------
//
// Class-
//       NC_opEndGroup
//
// Purpose-
//       The EndGroup operation.
//
//----------------------------------------------------------------------------
class NC_opEndGroup : public NC_op  // EndGroup operator
{
//----------------------------------------------------------------------------
// NC_opEndGroup::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_opEndGroup( void );          // Destructor
   NC_opEndGroup( void );           // Constructor

//----------------------------------------------------------------------------
// NC_opEndGroup::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operate( void );                 // Perform operation

virtual ostream&
   toStream(                        // Write this object
     ostream&          os) const;   // On this stream

//----------------------------------------------------------------------------
// NC_opEndGroup::Attributes
//----------------------------------------------------------------------------
public:
   NC_BeGroupSymbol* group;         // -> Ending group
}; // class NC_opEndGroup

//----------------------------------------------------------------------------
//
// Method-
//       NC_opEndGroup::~NC_opEndGroup
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opEndGroup::~NC_opEndGroup( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opEndGroup::NC_opEndGroup
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opEndGroup::NC_opEndGroup( void ) // Constructor
:  NC_op()
,  group(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opEndGroup::operate
//
// Purpose-
//       EndGroup operation.
//
//----------------------------------------------------------------------------
void
   NC_opEndGroup::operate( void )   // Perform EndGroup operation
{
   NC_BeGroupSymbol* symbol;        // -> BeGroup symbol

   assert( NC_COM.begroup= group );

   //-------------------------------------------------------------------------
   // Pop the active entry
   //-------------------------------------------------------------------------
   NC_COM.begroup= NULL;            // Default, no active group
   NC_COM.objfile= NULL;            // Default, no active file

   NC_COM.grpstak.remq();           // Remove the active element
   symbol= (NC_BeGroupSymbol*)NC_COM.grpstak.getHead();
   if( symbol == NULL )
     return;

   NC_COM.begroup= symbol;
   NC_COM.objfile= symbol->ofd;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opEndGroup::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opEndGroup::toStream(         // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this << ") "
      << "EndGroup(" << group << ") ";
   if( group->current_G == group )
     os << "Name(" << NC_COM.ist.getSymbolName(group) <<")\n";
   else
     os << "Name(*NONE*)\n";

   return os;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       EXTRACT
//
// Purpose-
//       Extract a parameter.
//
//----------------------------------------------------------------------------
static int
   extract(                         // Extract a parameter
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   stmtix= inpndx;                  // Initialize statement index

   //-------------------------------------------------------------------------
   // Handle group name
   //-------------------------------------------------------------------------
   if( inpbuf[stmtix] == ';' )      // If end of statement
     return(EOF);                   // Statement complete

   //-------------------------------------------------------------------------
   // Handle invalid clause
   //-------------------------------------------------------------------------
   NCmess(NC_msg::ID_SynGeneric, 0);// Syntax error
   return(ERR);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       END_BEGIN
//
// Purpose-
//       Process END.BEGIN statement.
//
//----------------------------------------------------------------------------
static void
   end_begin(                       // Process END of BEGIN
     NC_BeGroupSymbol* ptrgrp)      // Pointer to group descriptor
{
   NC_opEndGroup*      op= new NC_opEndGroup();
   NC_ifd*             ptrifd;      // Pointer to input descriptor

   //-------------------------------------------------------------------------
   // Validate that this end matches the active begin
   //-------------------------------------------------------------------------
   ptrifd= NC_COM.srcfile;          // Address the source file
   if( ptrgrp->source != ptrifd )   // If the end is not for the active file
   {
     NCmess(NC_msg::ID_EndWithoutBeg, 0);
     return;
   }

   //-------------------------------------------------------------------------
   // Add the operator onto the execution list
   //-------------------------------------------------------------------------
   op->group= NC_COM.begroup;
   NC_COM.pass1.fifo(op);

   op->operate();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       END_DO
//
// Purpose-
//       Process END.DO statement.
//
//----------------------------------------------------------------------------
static void
   end_do(                          // Process END of DO
     NC_DoGroupSymbol* ptr_do)      // Pointer to group descriptor
{
   NC_GroupSymbol*     ptrlink;     // Pointer to link

   //-------------------------------------------------------------------------
   // Move the compound statement to the DO list
   //
   // 1) Put the passN statements after the DO statement onto the DO list.
   // 2) Empty the passN list.
   // 3) Recreate the passN list, ending with the DO statement itself.
   //-------------------------------------------------------------------------
   if( ptr_do->op != NULL )         // Only process valid DO statements
   {
     ptr_do->op->stmt= (NC_op*)ptr_do->op->getNext();
     if( ptr_do->op->stmt != NULL ) // If the list is not empty
     {
       NC_op* head= NC_COM.passN.getHead(); // Save the list head
       NC_COM.passN.reset();        // Reset (empty) the list
       NC_COM.passN.insert(NULL, head, ptr_do->op); // Recreate the list
     }
   }

   //-------------------------------------------------------------------------
   // Delete the active entry
   //-------------------------------------------------------------------------
   NC_COM.grpstak.remq();           // Remove the active element
   delete ptr_do;                   // And release it

   //-------------------------------------------------------------------------
   // Set the new active entry
   //-------------------------------------------------------------------------
   NC_COM.dogroup= NULL;            // Default, no active group
   ptrlink= NC_COM.grpstak.getHead();
   while( ptrlink != NULL )         // If a link exists
   {
     if( ptrlink->type == NC_sym::TypeDoGroup ) // If this is a do group
     {
       NC_COM.dogroup= (NC_DoGroupSymbol*)ptrlink; // Set the do group pointer
       break;
     }
     ptrlink= ptrlink->getNext();   // Follow the chain
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NC__END
//
// Purpose-
//       Process END statement.
//
//----------------------------------------------------------------------------
extern void
   nc__end(                         // Process END statement
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   NC_GroupSymbol*     ptrgrp;      // Pointer to group descriptor
   NC_GroupSymbol*     ptrlink;     // Pointer to link

   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   ptrgrp= NULL;                    // Default, no active group
   ptrlink= NC_COM.grpstak.getHead();
   if( ptrlink != NULL )            // If a link exists
     ptrgrp= (NC_GroupSymbol*)ptrlink;

   //-------------------------------------------------------------------------
   // Extract any parameters
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Skip blanks
   stmtix= ncnextw(inpbuf, stmtix, NC_COM.word0);
                                    // Extract next word
   stmtix= extract(inpbuf, stmtix); // Process the word
   while( stmtix != EOF && stmtix != ERR ) // If more parameters remain
   {
     stmtix= ncnextw(inpbuf, stmtix, NC_COM.word0);
                                    // Extract next word
     stmtix= extract(inpbuf, stmtix); // Process the word
   }

   //-------------------------------------------------------------------------
   // Delete the active entry
   //-------------------------------------------------------------------------
   switch( ptrgrp->type )           // Delete the active entry
   {
     case NC_sym::TypeBeGroup:
       end_begin((NC_BeGroupSymbol*)ptrgrp);
       break;

     case NC_sym::TypeDoGroup:
       end_do((NC_DoGroupSymbol*)ptrgrp);
       break;

     default:
       NCmess(NC_msg::ID_BugFileLine, 2, __FILE__, "0001");
       break;
   }
}

