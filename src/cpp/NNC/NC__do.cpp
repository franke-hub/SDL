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
//       NC__do.cpp
//
// Purpose-
//       Neural Net Compiler: DO statement
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
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
#define __SOURCE__       "NC__DO  " // Source file, for debugging

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?
#include "NCdiag.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       extract
//
// Purpose-
//       Extract an expression.
//
//----------------------------------------------------------------------------
static int
   extract(                         // Extract an expression
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   char                c;           // Current character
   int                 fsm;         // Finite State Machine
   int                 exprix;      // Current expression index
   int                 stmtix;      // Current statement index

#define F_IDLE                    0 // Idle (blank after symbol)
#define F_SYMB                    1 // In-symbol state
#define F_OPER                    2 // Operator detected

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   fsm= F_OPER;                     // Default, operator state
   exprix= 0;                       // Initialize expression index
   stmtix= inpndx;                  // Initialize statement index

   //-------------------------------------------------------------------------
   // Copy the expression
   //-------------------------------------------------------------------------
   for(;;)                          // Copy the expression
   {
     c= inpbuf[stmtix];             // Get next character
     switch (c)                     // Evaluate the character
     {
       case ' ':                    // If blank
         if (fsm == F_SYMB)         // If in-symbol state
           fsm= F_IDLE;             // Go idle
         break;

       case '\0':                   // If end of source
       case ';':
         NC_COM.exprbuff[exprix]= '\0'; // End the expression
         return(stmtix);            // Expression found

       case '+':                    // If operator
       case '-':
       case '*':
       case '/':
       case '=':
       case '<':
       case '>':
       case '(':
       case ')':
       case '[':
       case ']':
         fsm= F_OPER;               // Go into operator state
         break;

       default:                     // If part of symbol
         if (fsm == F_IDLE)         // If two symbols in a row
         {
           NC_COM.exprbuff[exprix]= '\0'; // End the expression
           return(stmtix);          // Expression found
         }

         fsm= F_SYMB;               // Go into in-symbol state
         break;
     }

     NC_COM.exprbuff[exprix]= c;    // Copy the character
     exprix++;
     stmtix++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NC__DO
//
// Purpose-
//       Process DO statement.
//
//----------------------------------------------------------------------------
extern void
   nc__do(                          // Process DO statement
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   NC_opFor*           op= NC_opFor::generate();
   NC_FixedSymbol*     symbol;      // Pointer to a FixedSymbol
   NC_DoGroupSymbol*   ptr_do;      // Pointer to do group symbol

   int                 exprix;      // Current expression index
   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Add a debug operator onto the execution list
   //-------------------------------------------------------------------------
   NC_COM.passN.fifo(NC_opDebug::generate());

   //-------------------------------------------------------------------------
   // Allocate a do block
   //-------------------------------------------------------------------------
   ptr_do= new NC_DoGroupSymbol();  // Allocate a do block
   ptr_do->source= NC_COM.srcfile;
   ptr_do->lineno= NC_COM.lineno;
   ptr_do->column= NC_COM.column;
   ptr_do->op= NULL;

   //-------------------------------------------------------------------------
   // Extract: do NAME= expr1 to expr2 <by expr3>
   //             ----
   //-------------------------------------------------------------------------
   stmtix= ncnextw(inpbuf, inpndx, NC_COM.word0); // Extract named variable
   symbol= (NC_FixedSymbol*)NC_COM.ist.Symtab::locate(NC_COM.begroup,
                                                      NC_COM.word0);
   if( symbol == NULL )
     symbol= (NC_FixedSymbol*)NC_COM.ist.insert(NC_sym::TypeFixed,
                                                NC_COM.begroup,
                                                NC_COM.word0);
   op->symbol= symbol;

   //-------------------------------------------------------------------------
   // Add the do block onto the stack
   //-------------------------------------------------------------------------
   NC_COM.dogroup= ptr_do;          // Make this block current
   NC_COM.grpstak.lifo(ptr_do);     // This block is now active
   if( symbol == NULL )             // If invalid DO statement
     return;                        // Ignore it, message already generated

   //-------------------------------------------------------------------------
   // TODO: Make sure that NAME is not already in use.
   //-------------------------------------------------------------------------

   //-------------------------------------------------------------------------
   // Extract: do NAME= expr1 to expr2 <by expr3>
   //                 -------
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, stmtix);
   if (inpbuf[stmtix] != '=')
   {
     NCmess(NC_msg::ID_SynGeneric, 0);
     return;
   }
   stmtix++;

   stmtix= extract(inpbuf, stmtix); // Extract the expression
   if (stmtix < 0)
   {
     NCmess(NC_msg::ID_SynGeneric, 0);
     return;
   }
   exprix= 0;
   op->initial= NC_opFixed::generate(NC_COM.exprbuff, exprix);
   if (op->initial == NULL || NC_COM.exprbuff[exprix] != '\0') // If error
     return;                        // (Message already written)

   //-------------------------------------------------------------------------
   // Extract: do NAME= expr1 to expr2 <by expr3>
   //                         --------
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, stmtix);
   if (toupper(inpbuf[stmtix]) != 'T'
       ||toupper(inpbuf[stmtix+1]) != 'O'
       ||toupper(inpbuf[stmtix+2]) != ' ')
   {
     NCmess(NC_msg::ID_SynGeneric, 0);
     return;
   }
   stmtix += 3;

   stmtix= extract(inpbuf, stmtix); // Extract the expression
   if (stmtix < 0)
   {
     NCmess(NC_msg::ID_SynGeneric, 0);
     return;
   }
   exprix= 0;
   op->final= NC_opFixed::generate(NC_COM.exprbuff, exprix);
   if (op->final == NULL || NC_COM.exprbuff[exprix] != '\0') // If error
     return;                        // (Message already written)

   //-------------------------------------------------------------------------
   // Extract: do NAME= expr1 to expr2 <by expr3>
   //                                   --------
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, stmtix);
   if (inpbuf[stmtix] == ';')
     op->increment= NC_opFixed::generate(1);

   else
   {
     if (toupper(inpbuf[stmtix]) != 'B'
         ||toupper(inpbuf[stmtix+1]) != 'Y'
         ||toupper(inpbuf[stmtix+2]) != ' ')
     {
       NCmess(NC_msg::ID_SynGeneric, 0);
       return;
     }
     stmtix += 3;

     stmtix= extract(inpbuf, stmtix); // Extract the expression
     if (stmtix < 0)
     {
       NCmess(NC_msg::ID_SynGeneric, 0);
       return;
     }
     exprix= 0;
     op->increment= NC_opFixed::generate(NC_COM.exprbuff, exprix);
     if (op->increment == NULL || NC_COM.exprbuff[exprix] != '\0') // If error
       return;                      // (Message already written)
   }

   //-------------------------------------------------------------------------
   // Add the operator onto the execution list
   //-------------------------------------------------------------------------
   ptr_do->op= op;
   NC_COM.passN.fifo(op);
}

