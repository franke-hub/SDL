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
//       NC__beg.cpp
//
// Purpose-
//       Neural Net Compiler: BEGIN statement
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
#include <com/istring.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_op.h"
#include "NC_sym.h"
#include "NC_sys.h"
#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC__BEG " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#include "NCdiag.h"

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
     NC_opGroup*       op)          // Group operator
{
   NC_BeGroupSymbol*   symbol;      // GroupName symbol table entry
   NC_ofd*             ptrofd;      // Pointer to file descriptor
   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   ptrofd= op->group->ofd;          // Address the OFD
   stmtix= inpndx;                  // Initialize statement index

   //-------------------------------------------------------------------------
   // Handle group name
   //-------------------------------------------------------------------------
   if( inpbuf[stmtix] == ';' )      // If end of statement
   {
     if( NC_COM.word0[0] != '\0' )  // If a name was specified
     {
       symbol= (NC_BeGroupSymbol*)NC_COM.ist.insert(NC_sym::TypeBeGroup,
                                                    NC_COM.begroup,
                                                    NC_COM.word0,
                                                    op->group);
       if( symbol == NULL )
         return(ERR);

       op->group->current_G= symbol;
       symbol->current_G= symbol;
     }
     return(EOF);                   // Statement complete
   }

   //-------------------------------------------------------------------------
   // Allocate an OFD, if required
   //-------------------------------------------------------------------------
   if( ptrofd == NULL )
   {
     ptrofd= (NC_ofd*)malloc(sizeof(NC_ofd));
     if( ptrofd == NULL )
     {
       NCmess(NC_msg::ID_StgSkipStmt, 0);
       return(ERR);
     }
     memset(ptrofd, 0, sizeof(NC_ofd));

     op->group->ofd= ptrofd;
   }

   //-------------------------------------------------------------------------
   // Handle FILE clause
   //-------------------------------------------------------------------------
   if( stricmp(NC_COM.word0, "FILE") == 0 ) // If FILE clause
   {
     if( ptrofd->fname[0] != '\0' )
     {
       NCmess(NC_msg::ID_BegDupClause, 1, "FILE");
       return(ERR);
     }

     stmtix= ncstring(inpbuf, stmtix, ptrofd->fname, PGS_FNSIZE);
                                    // Extract the string
     if( stmtix < 0 )               // If error encountered
     {
       if( stmtix == ERR_LENGTH )   // If length error
         NCmess(NC_msg::ID_SynFileNameTooLong, 1, ptrofd->fname); // Write
                                    // error message
       return(ERR);                 // Do not continue
     }

     return(stmtix);                // Clause complete
   }

   //-------------------------------------------------------------------------
   // Handle INFO clause
   //-------------------------------------------------------------------------
   if( stricmp(NC_COM.word0, "INFO") == 0 ) // If INFO clause
   {
     if( ptrofd->finfo[0] != '\0' )
     {
       NCmess(NC_msg::ID_BegDupClause, 1, "INFO");
       return(ERR);
     }

     stmtix= ncstring(inpbuf, stmtix, ptrofd->finfo, PGS_FNSIZE);
                                    // Extract the string
     if( stmtix < 0 )               // If error encountered
     {
       if( stmtix == ERR_LENGTH )   // If length error
         NCmess(NC_msg::ID_SynInfoTooLong, 1, ptrofd->finfo); // Write
                                    // error message
       return(ERR);                 // Do not continue
     }

     return(stmtix);                // Clause complete
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
//       NC__BEG
//
// Purpose-
//       Process BEGIN statement.
//
//----------------------------------------------------------------------------
extern void
   nc__beg(                         // Process BEGIN statement
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   NC_opGroup*         op= NC_opGroup::generate(); // Generated Group operator

   NC_BeGroupSymbol*   oldbeg;      // Pointer to old begin group
   NC_BeGroupSymbol*   ptrbeg;      // Pointer to begin group
   NC_ofd*             oldofd;      // Pointer to old file descriptor
   NC_ofd*             ptrofd;      // Pointer to file descriptor

   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Allocate a begin block
   //-------------------------------------------------------------------------
   ptrbeg= new NC_BeGroupSymbol();  // Allocate a begin block

   ptrbeg->source= NC_COM.srcfile;  // Set file origin
   ptrbeg->lineno= NC_COM.lineno;
   ptrbeg->column= NC_COM.column;
   op->group= ptrbeg;

   //-------------------------------------------------------------------------
   // Extract the begin parameters
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Skip blanks
   stmtix= ncnextw(inpbuf, stmtix, NC_COM.word0); // Extract next word
   stmtix= extract(inpbuf, stmtix, op); // Process the word
   while( stmtix != EOF && stmtix != ERR )
   {
     stmtix= ncnextw(inpbuf, stmtix, NC_COM.word0); // Extract next word
     stmtix= extract(inpbuf, stmtix, op); // Process the word
   }

   //-------------------------------------------------------------------------
   // Use symbol table entry, if group replaced
   //-------------------------------------------------------------------------
   if( op->group->current_G != NULL )
   {
     op->group= ptrbeg->current_G;
     delete ptrbeg;
     ptrbeg= op->group;
   }
   else
     op->group->current_G= NC_COM.begroup;

   //-------------------------------------------------------------------------
   // Error checks
   //-------------------------------------------------------------------------
   ptrofd= op->group->ofd;          // Address the OFD
   if( stmtix != ERR                // If no error encountered
       && ptrofd != NULL )          // and a clause was specified
   {
     if( ptrofd->fname[0] == '\0' ) // If FILE not specified
     {
       NCmess(NC_msg::ID_InfWithoutFile, 0); // INFO without FILE
       stmtix= ERR;                 // Indicate error
     }
   }

   //-------------------------------------------------------------------------
   // Activate the file
   //-------------------------------------------------------------------------
   if( ptrofd != NULL )             // If a FILE clause was specified
   {
     //-------------------------------------------------------------------------
     // Locate an existing file block
     //-------------------------------------------------------------------------
     for(oldofd= (NC_ofd*)NC_COM.objlist.getHead();
         oldofd != NULL;
         oldofd= (NC_ofd*)oldofd->getNext())
     {
       if( strcmp(ptrofd->fname, oldofd->fname) == 0 ) // If FILE match
       {
         if( strcmp(ptrofd->finfo, oldofd->finfo) != 0 ) // If INFO differs
           NCmess(NC_msg::ID_InfChanged, 0);

         break;
       }
     }
     if( oldofd != NULL )
     {
       free(ptrofd);
       ptrofd= oldofd;
     }
     else
     {
       NC_COM.objlist.lifo(ptrofd); // Add the OFD to the list of OFDs
       ptrofd->fileno= NN_COM.pgs.insFile(ptrofd->fname); // Activate the file
////   NN_COM.pgs.info(ptrofd->fileno, ptrofd->finfo);
     }

     //-------------------------------------------------------------------------
     // Make this file active
     //-------------------------------------------------------------------------
     ptrbeg->ofd= ptrofd;           // Add file to stack
   }

   //-------------------------------------------------------------------------
   // Inherit parameters from existing block
   //-------------------------------------------------------------------------
   oldbeg= NC_COM.begroup;          // Get old block
   assert( oldbeg != NULL );        // If prior block exists

   if( ptrbeg->ofd == NULL )        // If no file[] clause
   {
     ptrbeg->ofd= oldbeg->ofd;      // Keep current output file
     ptrbeg->current_N= oldbeg->current_N; // Keep current default neuron
   }

   //-------------------------------------------------------------------------
   // Add the operator onto the execution list
   //-------------------------------------------------------------------------
   NC_COM.pass1.fifo(op);

   op->operate();
}

