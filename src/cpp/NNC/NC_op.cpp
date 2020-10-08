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
//       NC_op.cpp
//
// Purpose-
//       Neural Net Compiler - Operator table.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_op.h"
#include "NC_msg.h"
#include "NC_sym.h"
#include "NC_sys.h"

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC_op   " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // BRINGUP mode? (for debugging)
#include "NCdiag.h"                 // Diagnostic macros

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_DEPTH               128 // Maximum expression depth

//----------------------------------------------------------------------------
// Internal Data areas
//----------------------------------------------------------------------------
static short           opprec[]={   // Operator precedence
   255,                             // Invalid
   255,                             // Constant
   255,                             // Get
   255,                             // Set

   9,                               // Uniary Minus

   5,                               // Increment
   5,                               // Decrement

   255,                             // Fixed()
   255,                             // Float()

   3,                               // Add
   3,                               // Subtract
   4,                               // Multiply
   4,                               // Divide

                                    // COUNT
   2,                               // Left Parenthesis
   1,                               // Right Parenthesis
   0,                               // Right bracket
   0                                // (End of Statement)
};

#if __BRINGUP__
static char*           opname[]=    // Operator name
{
   "ERROR",                         // Error
   "CONST",                         // Constant
   "GET",                             // Get
   "SET",                             // Set

   "-()",                           // Uniary Minus

   "++",                            // Increment
   "--",                            // Decrement

   "Fixed()",                       // Fixed()
   "Float()",                       // Float()

   "+",                             // Add
   "-",                             // Subtract
   "*",                             // Multiply
   "/",                             // Divide

   "(",                             // Left Parenthesis
   ")",                             // Right Parenthesis
   "]",                             // Right bracket
   ";"                              // (End of Statement)
};
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       opConstant
//
// Purpose-
//       Generate a constant arithmetic operator.
//
//----------------------------------------------------------------------------
static NC_opArith*                  // Resultant
   opConstant(                      // Evaluate a constant string
     const char*       inpbuf,      // Current buffer
     int&              inpndx)      // Running buffer index
{
   NN::Value           result= 0.0; // Working resultant
   int                 stmtix;      // Current character index
   char                c;           // Current character
   char                d= 0;        // TRUE if decimal point found
   NN::Value           P= 10.0;     // Multiplier 1
   NN::Value           Q= 1.0;      // Multiplier 2

   for(stmtix=inpndx; ;stmtix++)    // Extract word
   {
     c= inpbuf[stmtix];             // Next character

     if( c == '\0' )                // If end of string
       break;                       // Break, end of string
     if( c >= '0' && c <= '9' )     // Process character
       result= result * P + ((c - '0') * Q);
     else if( c == '.' )            // Decimal point
     {
       if( d != 0 )
       {
         inpndx= ERR_SYNTAX;        // Syntax error
         NCmess(NC_msg::ID_SynGeneric, 0); // Too many decimal points
         BRINGF(("%s %d: '%s'\n", __FILE__, __LINE__, &inpbuf[stmtix]));
         return NULL;
       }
       d= 1;
       P= 1.0;
     }
     else
       break;

     if( d != 0 )
       Q *= .1;
   }

   inpndx= stmtix;                  // Return updated index
   if( d == 0 )                     // If FIXED expression
     return NC_opFixed::generate((int)result);

   return NC_opFloat::generate((double)result);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       opSymbol
//
// Purpose-
//       Evaluate a symbol reference operator.
//
//----------------------------------------------------------------------------
static NC_opArith*                  // Resultant
   opSymbol(                        // Evaluate a constant string
     const char*       inpbuf,      // Current buffer
     int&              inpndx)      // Running buffer index
{
   char                wordx[NC_com::WORK_SIZE]; // Symbol accumulator

   NC_sym*             symbol;      // Symbol resultant

   char                c;           // Current character
   unsigned            symbix;      // Resultant symbol index
   int                 stmtix;      // Current character index

   symbix= 0;                       // Initialize the symbol index
   for(stmtix=inpndx; ;stmtix++)    // Extract word
   {
     c= inpbuf[stmtix];             // Next character
     if( c == '\0' )                // If end of string
       break;                       // Break, end of string
     if( isalnum(c) == FALSE        // If not an alphanumeric
         &&c != '_' )               // and not an underscore
       break;

     wordx[symbix++]= c;            // Copy the character
     if( symbix >= sizeof(wordx) )  // If the symbol's too long
     {
       inpndx= ERR;
       wordx[sizeof(wordx)-1]= '\0';
       NCmess(NC_msg::ID_SynSymbolTooLong, 1, wordx);
       return NULL;
     }
   }

   wordx[symbix]= '\0';
   symbol= (NC_sym*)NC_COM.ist.locate(NC_COM.begroup, wordx);
   if( symbol == NULL )
   {
     inpndx= ERR;
     NCmess(NC_msg::ID_SymNotFound, 1, wordx);
     return NULL;
   }

   inpndx= stmtix;
   return NC_opArith::generate(symbol);
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_op::~NC_op
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_op::~NC_op( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_op::NC_op
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_op::NC_op( void )             // Constructor
:  DHSL_List<NC_op>::Link()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_op::operate
//
// Purpose-
//       Operate (NOP)
//
//----------------------------------------------------------------------------
void
   NC_op::operate( void )           // Operate
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_op::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_op::toStream(                 // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this <<") NC_op\n";
   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opArith::~NC_opArith
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opArith::~NC_opArith( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opArith::NC_opArith
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opArith::NC_opArith( void )   // Constructor
:  NC_op()
,  op(OpErr)
,  symbol(NULL)
{
   operand[0]= NULL;
   operand[1]= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opArith::isFixed
//
// Purpose-
//       Is this an NC_opFixed
//
//----------------------------------------------------------------------------
int
   NC_opArith::isFixed( void ) const // Class(NC_opFixed)?
{
   return FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opArith::isFloat
//
// Purpose-
//       Is this an NC_opFloat
//
//----------------------------------------------------------------------------
int
   NC_opArith::isFloat( void ) const // Class(NC_opFloat)?
{
   return FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opArith::generate
//
// Purpose-
//       Generate an NC_op, either Fixed or Float.
//
//----------------------------------------------------------------------------
NC_opArith*                         // Resultant operator
   NC_opArith::generate(            // Generate a Fixed/Float operation
     const char*       inpbuf,      // Current buffer
     int&              inpndx)      // Running buffer index
{
   NC_opArith*         operand;     // Working operand
   NC_opArith*         vstack[MAX_DEPTH]; // Expression value stack
   Op                  ostack[MAX_DEPTH]; // Operator stack, each element is an
                                    // optab index

   char                c;           // Current character
   int                 op;          // Current opcode
   int                 endop;       // Expected ending opcode
   int                 stmtix;      // Current statement index
   int                 osix;        // Operator stack index
   int                 vsix;        // Value stack index

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Initial index value
   osix= 0;                         // Initial stack indexes
   vsix= 0;

   endop= OP_EOS;                   // Default, expect ';' terminator
   if( inpbuf[stmtix] == '(' )      // If starting with '('
   {
     endop= OP_RHP;                 // Expect ')' terminator
     stmtix++;                      // Skip the '(' character
   }

   else if( inpbuf[stmtix] == '[' ) // If starting with '['
   {
     endop= OP_RHB;                 // Expect ']' terminator
     stmtix++;                      // Skip the '[' character
   }

   //-------------------------------------------------------------------------
   // Evaluate the expression
   //-------------------------------------------------------------------------
   for (;;)                         // Evaluate an expression
   {
     //-----------------------------------------------------------------------
     // Handle uniary operator
     //-----------------------------------------------------------------------
     stmtix= ncskipb(inpbuf, stmtix); // Find the next non-blank
     if( inpbuf[stmtix] == '+' )    // If uniary plus
     {
       stmtix++;                    // Just ignore it
     }
     else if( inpbuf[stmtix] == '-' ) // If uniary minus
     {
       if( osix >= MAX_DEPTH )      // If stack depth error
       {
         NCmess(NC_msg::ID_FixComplex, 0); // Expression too complex
         return NULL;
       }
       stmtix++;                    // Skip the operator
       ostack[osix++]= OpNegate;    // Add it to operator stack
     }

     //-----------------------------------------------------------------------
     // Handle left parenthesis
     //-----------------------------------------------------------------------
     stmtix= ncskipb(inpbuf, stmtix); // Find the next non-blank
     c= inpbuf[stmtix];             // Save the character
     if( c == '(' )                 // If left parenthesis
     {
       if( osix >= MAX_DEPTH )      // If stack depth error
       {
         NCmess(NC_msg::ID_FixComplex, 0); // Expression too complex
         return NULL;
       }
       ostack[osix++]= Op(OP_LHP);  // Add this operator to the stack
       stmtix++;                    // Skip the operator
       continue;                    // And continue
     }

     //-----------------------------------------------------------------------
     // Extract symbol
     //-----------------------------------------------------------------------
     if( c == '.' || (c >= '0' && c <= '9') ) // If numeric constant
       operand= opConstant(inpbuf, stmtix); // Evaluate it
     else if( isalpha(c)            // If alphabetic
              ||c == '_' )          // (including underscore)
       operand= opSymbol(inpbuf, stmtix); // Evaluate it
     else                           // If invalid character
     {
       NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
       BRINGF(("%s %d: '%s'\n", __FILE__, __LINE__, &inpbuf[stmtix]));
       return NULL;                 // Indicate error
     }

     if( operand == NULL )          // If error detected
       return NULL;                 // Exit, syntax error

     if( vsix >= MAX_DEPTH )        // If stack depth error
     {
       NCmess(NC_msg::ID_FixComplex, 0); // Expression too complex
       return NULL;
     }
     vstack[vsix++]= operand;

     //-----------------------------------------------------------------------
     // Extract an operator
     //-----------------------------------------------------------------------
next_operator:
     stmtix= ncskipb(inpbuf, stmtix); // Find the next non-blank
     c= inpbuf[stmtix];             // Save the character
     switch (c)                     // Process operator
     {
       case '+':
         op= OpAdd;
         break;

       case '-':
         op= OpSub;
         break;

       case '*':
         op= OpMul;
         break;

       case '/':
         op= OpDiv;
         break;

       case ']':
         op= OP_RHB;
         break;

       case ')':
         op= OP_RHP;
         break;

       case '\0':
       case ';':
         op= OP_EOS;
         break;

       default:
         NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
         BRINGF(("%s %d: '%s'\n", __FILE__, __LINE__, &inpbuf[stmtix]));
         return NULL;               // Indicate error
     }

     //-----------------------------------------------------------------------
     // Drain an operator
     //-----------------------------------------------------------------------
     while (osix > 0 && opprec[op] < opprec[ostack[osix-1]])
     {
       osix--;                      // Account for operator
       #if( __BRINGUP__ )
         cout << __SOURCE__ ":" << __LINE__ << ": "
              << "Drain(" << (int)ostack[osix]
              << "," << opname[ostack[osix]] << ")\n";
         cout.flush();
       #endif
       switch (ostack[osix])      // Drain an operator
       {
         case OpAdd:
         case OpSub:
         case OpMul:
         case OpDiv:
           if( vsix < 2 )
           {
             NCmess(NC_msg::ID_BugFileLine, 2, __SOURCE__, "0001");
             return NULL;
           }

           #if( __BRINGUP__ )
             operand= vstack[vsix-1];
             if( operand->op == OpGet )
               cout << "  Symbol(" << NC_COM.ist.getSymbolName(operand->symbol)
                    << ") ";
             if( operand->isFixed() )
             {
               operand->operate();
               cout << "  Value(" << ((NC_opFixed*)operand)->getFixed()
                    << ")\n";
             }
             else
             {
               operand->operate();
               cout << "  Value(" << ((NC_opFloat*)operand)->getFloat()
                    << ")\n";
             }

             operand= vstack[vsix-2];
             if( operand->op == OpGet )
               cout << "  Symbol(" << NC_COM.ist.getSymbolName(operand->symbol)
                    << ") ";
             if( operand->isFixed() )
             {
               operand->operate();
               cout << "  Value(" << ((NC_opFixed*)operand)->getFixed()
                    << ")\n";
             }
             else
             {
               operand->operate();
               cout << "  Value(" << ((NC_opFloat*)operand)->getFloat()
                    << ")\n";
             }
           #endif

           if( vstack[vsix-1]->isFloat() || vstack[vsix-2]->isFloat() )
           {
             vstack[vsix-1]= NC_opFloat::generate(vstack[vsix-1]);
             vstack[vsix-2]= NC_opFloat::generate(vstack[vsix-2]);
             operand= new NC_opFloat();
           }
           else
             operand= new NC_opFixed();

           operand->op= ostack[osix];

           operand->operand[0]= vstack[vsix-2];
           operand->operand[1]= vstack[vsix-1];
           vstack[vsix-2]= operand;

           vsix--;
           break;

         case OP_LHP:
           if( op != OP_RHP )
           {
             NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
             BRINGF(("%s %d: '%s'\n", __FILE__, __LINE__,
                    &inpbuf[stmtix]));
             return NULL;           // Indicate error
           }

           stmtix++;                // Skip the operator
           goto next_operator;

         case OP_RHP:
           NCmess(NC_msg::ID_BugFileLine, 2, __SOURCE__, "0005");
           return NULL;

         case OpInc:
         case OpDec:
         case OpNegate:
           if( vsix < 1 )
           {
             NCmess(NC_msg::ID_BugFileLine, 2, __SOURCE__, "0007");
             return NULL;
           }

           if( vstack[vsix-1]->isFloat() )
             operand= new NC_opFloat();
           else
             operand= new NC_opFixed();

           operand->op= ostack[osix];
           operand->operand[0]= vstack[vsix-1];
           vstack[vsix-1]= operand;

           break;

         case OP_EOS:
           break;

         default:
           break;
         }
       }

     if( op == endop && osix == 0 )
     {
       if( inpbuf[stmtix] != '\0' )
         stmtix++;
       break;
     }

     if( op == OP_EOS )
     {
       NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
       return NULL;
     }

     if( osix >= MAX_DEPTH )        // If stack depth error
     {
       NCmess(NC_msg::ID_FixComplex, 0); // Expression too complex
       return NULL;
     }

     ostack[osix++]= Op(op);        // Add this operator to the stack
     stmtix++;                      // Skip the operator
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   operand= vstack[0];
   inpndx= stmtix;

   return(operand);                 // Return, expression generated
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opArith::generate
//
// Purpose-
//       Generate an NC_opArith.
//
//----------------------------------------------------------------------------
NC_opArith*                         // Resultant operator
   NC_opArith::generate(            // Generate an Arith operation
     NC_sym*           symbol)      // Symbol source
{
   if( symbol->type == NC_sym::TypeFixed )
     return NC_opFixed::generate((NC_FixedSymbol*)symbol);
   else if( symbol->type == NC_sym::TypeFloat )
     return NC_opFloat::generate((NC_FloatSymbol*)symbol);

   NCfault(__SOURCE__, __LINE__);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opArith::generate
//
// Purpose-
//       Generate an NC_opArith
//
//----------------------------------------------------------------------------
NC_opArith*                         // Resultant operator
   NC_opArith::generate(            // Generate an Arith constant
     const int         value)       // Constant value
{
   return NC_opFixed::generate(value);
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opArith::generate
//
// Purpose-
//       Generate an NC_opArith
//
//----------------------------------------------------------------------------
NC_opArith*                         // Resultant operator
   NC_opArith::generate(            // Generate an Arith constant
     const double      value)       // Constant value
{
   return NC_opFloat::generate(value);
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opDebug::~NC_opDebug
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opDebug::~NC_opDebug( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opDebug::NC_opDebug
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opDebug::NC_opDebug( void )   // Constructor
:  NC_op()
,  ifd(NC_COM.srcfile)
,  lineNumber(NC_COM.lineno)
,  column(NC_COM.column)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opDebug::generate
//
// Purpose-
//       Generate an NC_opDebug.
//
//----------------------------------------------------------------------------
NC_opDebug*                         // Resultant operator
   NC_opDebug::generate( void )     // Generate a Debug operation
{
   NC_opDebug*         resultant= new NC_opDebug();

   NC_COM.debug= resultant;
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opDebug::operate
//
// Purpose-
//       Make this the current debug object.
//
//----------------------------------------------------------------------------
void
   NC_opDebug::operate( void )      // Reset File/Line/Column
{
   NC_COM.debug= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opDebug::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opDebug::toStream(            // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this <<") Debug "
      << "File(" << ifd->filenm << ") "
      << "Line(" << lineNumber << ") "
      << "Column(" << column << ")\n";
   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::~NC_opFixed
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opFixed::~NC_opFixed( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::NC_opFixed
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opFixed::NC_opFixed( void )   // Constructor
:  NC_opArith()
,  resultant(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::isFixed
//
// Purpose-
//       Is this an NC_opFixed
//
//----------------------------------------------------------------------------
int
   NC_opFixed::isFixed( void ) const// Class(NC_opFixed)?
{
   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::generate
//
// Purpose-
//       Generate an NC_opFixed.
//
//----------------------------------------------------------------------------
NC_opFixed*                         // Resultant operator
   NC_opFixed::generate(            // Generate a Fixed operation
     const char*       inpbuf,      // Current buffer
     int&              inpndx)      // Running buffer index
{
   return NC_opFixed::generate(NC_opArith::generate(inpbuf,inpndx));
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::generate
//
// Purpose-
//       Generate an NC_opFixed.
//
//----------------------------------------------------------------------------
NC_opFixed*                         // Resultant operator
   NC_opFixed::generate(            // Generate a Fixed operation
     NC_FixedSymbol*   symbol)      // Symbol source
{
   NC_opFixed*         resultant= NULL;

   if( symbol != NULL )
   {
     resultant= new NC_opFixed();
     resultant->op= OpGet;
     resultant->symbol= symbol;
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::generate
//
// Purpose-
//       Generate an NC_opFixed.
//
//----------------------------------------------------------------------------
NC_opFixed*                         // Resultant operator
   NC_opFixed::generate(            // Generate a Fixed operation
     const int         value)       // Constant value
{
   NC_opFixed*         resultant= new NC_opFixed();

   resultant->op= OpConst;
   resultant->resultant= value;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::generate
//
// Purpose-
//       Generate an NC_opFixed.
//
//----------------------------------------------------------------------------
NC_opFixed*                         // Resultant operator
   NC_opFixed::generate(            // Generate a Fixed operation
     NC_opArith*       op)          // Source operator
{
   NC_opFixed*         resultant= NULL; // Resultant

   if( op != NULL )
   {
     if( op->isFixed() )
       resultant= (NC_opFixed*)op;

     else if( op->isFloat() )
     {
       resultant= new NC_opFixed();

       resultant->op= OpFixed;
       resultant->operand[0]= op;
     }
     else
       NCfault(__SOURCE__, __LINE__);
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::operate
//
// Purpose-
//       Calculate the resultant.
//
//----------------------------------------------------------------------------
void
   NC_opFixed::operate( void )      // Calculate the Resultant
{
   if( operand[0] != NULL )
     operand[0]->operate();

   if( operand[1] != NULL )
     operand[1]->operate();

   switch(op)                       // Perform the operation
   {
     case OpConst:
       break;

     case OpGet:
       resultant= ((NC_FixedSymbol*)symbol)->value;
       break;

     case OpSet:
       ((NC_FixedSymbol*)symbol)->value=
           ((NC_opFixed*)operand[0])->getFixed();
       break;

     case OpAdd:
       resultant=
           ((NC_opFixed*)operand[0])->getFixed() +
           ((NC_opFixed*)operand[1])->getFixed();
       break;

     case OpSub:
       resultant=
           ((NC_opFixed*)operand[0])->getFixed() -
           ((NC_opFixed*)operand[1])->getFixed();
       break;

     case OpMul:
       resultant=
           ((NC_opFixed*)operand[0])->getFixed() *
           ((NC_opFixed*)operand[1])->getFixed();
       break;

     case OpDiv:
       resultant=
           ((NC_opFixed*)operand[0])->getFixed() /
           ((NC_opFixed*)operand[1])->getFixed();
       break;

     case OpInc:
       resultant=
           ((NC_opFixed*)operand[0])->getFixed() + 1;
       break;

     case OpDec:
       resultant=
           ((NC_opFixed*)operand[0])->getFixed() - 1;
       break;

     case OpNegate:
       resultant=
           -(((NC_opFixed*)operand[0])->getFixed());
       break;

     case OpFixed:
       resultant= (int)(((NC_opFloat*)operand[0])->getFloat());
       break;

     default:
       fprintf(stderr, "Error data(%d)\n", op);
       NCfault(__SOURCE__, __LINE__);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opFixed::toStream(            // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this <<") Fixed(" << resultant << ")\n";
   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::~NC_opFloat
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opFloat::~NC_opFloat( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::NC_opFloat
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opFloat::NC_opFloat( void )   // Constructor
:  NC_opArith()
,  resultant(0.0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::isFloat
//
// Purpose-
//       Is this an NC_opFloat
//
//----------------------------------------------------------------------------
int
   NC_opFloat::isFloat( void ) const// Class(NC_opFloat)?
{
   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::generate
//
// Purpose-
//       Generate an NC_opFloat.
//
//----------------------------------------------------------------------------
NC_opFloat*                         // Resultant operator
   NC_opFloat::generate(            // Generate a Float operation
     const char*       inpbuf,      // Current buffer
     int&              inpndx)      // Running buffer index
{
   return NC_opFloat::generate(NC_opArith::generate(inpbuf,inpndx));
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::generate
//
// Purpose-
//       Generate an NC_opFloat.
//
//----------------------------------------------------------------------------
NC_opFloat*                         // Resultant operator
   NC_opFloat::generate(            // Generate a Float operation
     NC_FloatSymbol*   symbol)      // Symbol source
{
   NC_opFloat*         resultant= new NC_opFloat();

   resultant->op= OpGet;
   resultant->symbol= symbol;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::generate
//
// Purpose-
//       Generate an NC_opFloat.
//
//----------------------------------------------------------------------------
NC_opFloat*                         // Resultant operator
   NC_opFloat::generate(            // Generate a Float operation
     const double      value)       // Constant value
{
   NC_opFloat*         resultant= new NC_opFloat();

   resultant->op= OpConst;
   resultant->resultant= value;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::generate
//
// Purpose-
//       Generate an NC_opFloat.
//
//----------------------------------------------------------------------------
NC_opFloat*                         // Resultant operator
   NC_opFloat::generate(            // Generate a Float operation
     NC_opArith*       op)          // Source operator
{
   NC_opFloat*         resultant= NULL; // Resultant

   if( op != NULL )
   {
     if( op->isFloat() )
       resultant= (NC_opFloat*)op;

     else if( op->isFixed() )
     {
       resultant= new NC_opFloat();

       resultant->op= OpFloat;
       resultant->operand[0]= op;
     }
     else
       NCfault(__SOURCE__, __LINE__);
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::operate
//
// Purpose-
//       Calculate the resultant.
//
//----------------------------------------------------------------------------
void
   NC_opFloat::operate( void )      // Calculate the Resultant
{
   if( operand[0] != NULL )
     operand[0]->operate();

   if( operand[1] != NULL )
     operand[1]->operate();

   switch(op)                       // Perform the operation
   {
     case OpConst:
       break;

     case OpGet:
       resultant= ((NC_FloatSymbol*)symbol)->value;
       break;

     case OpSet:
       ((NC_FloatSymbol*)symbol)->value=
           ((NC_opFloat*)operand[0])->getFloat();
       break;

     case OpAdd:
       resultant=
           ((NC_opFloat*)operand[0])->getFloat() +
           ((NC_opFloat*)operand[1])->getFloat();
       break;

     case OpSub:
       resultant=
           ((NC_opFloat*)operand[0])->getFloat() -
           ((NC_opFloat*)operand[1])->getFloat();
       break;

     case OpMul:
       resultant=
           ((NC_opFloat*)operand[0])->getFloat() *
           ((NC_opFloat*)operand[1])->getFloat();
       break;

     case OpDiv:
       resultant=
           ((NC_opFloat*)operand[0])->getFloat() /
           ((NC_opFloat*)operand[1])->getFloat();
       break;

     case OpInc:
       resultant=
           ((NC_opFloat*)operand[0])->getFloat() + 1;
       break;

     case OpDec:
       resultant=
           ((NC_opFloat*)operand[0])->getFloat() - 1;
       break;

     case OpNegate:
       resultant=
           -(((NC_opFloat*)operand[0])->getFloat());
       break;

     case OpFloat:
       resultant= (double)(((NC_opFixed*)operand[0])->getFixed());
       break;

     default:
       fprintf(stderr, "Error data(%d)\n", op);
       NCfault(__SOURCE__, __LINE__);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opFloat::toStream(            // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this <<") Float(" << resultant << ")\n";
   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFor::~NC_opFor
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opFor::~NC_opFor( void )      // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFor::NC_opFor
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opFor::NC_opFor( void )       // Constructor
:  NC_op()
,  symbol(NULL)
,  initial(NULL)
,  final(NULL)
,  increment(NULL)
,  stmt(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFor::generate
//
// Purpose-
//       Generate an NC_opFor.
//
//----------------------------------------------------------------------------
NC_opFor*                           // Resultant operator
   NC_opFor::generate( void )       // Generate a For operation
{
   return new NC_opFor();
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFor::operate
//
// Purpose-
//       Calculate the resultant.
//
//----------------------------------------------------------------------------
void
   NC_opFor::operate( void )        // Calculate the Resultant
{
   NC_op*              op;          // Current operation

   initial->operate();
   final->operate();
   increment->operate();

   symbol->value= initial->getFixed();
   if( increment->getFixed() > 0 )
   {
     while( symbol->value <= final->getFixed() )
     {
       for( op= stmt; op != NULL; op= (NC_op*)op->getNext() )
         op->operate();

       symbol->value += increment->getFixed();
     }
   }

   else if( increment->getFixed() < 0 )
   {
     while( symbol->value >= final->getFixed() )
     {
       for( op= stmt; op != NULL; op= (NC_op*)op->getNext() )
         op->operate();

       symbol->value += increment->getFixed();
     }
   }

   else                             // Increment 0
   {
     NCmess(NC_msg::ID_ForInfinite, 0);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFor::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opFor::toStream(              // Write the object
     ostream&          os) const    // On this stream
{
   NC_op*              op;          // Current operation

   initial->operate();
   final->operate();
   increment->operate();

   os << "NC_op@(" << this << ") "
      << "DO(" << NC_COM.ist.getSymbolName(symbol) << ") "
      << "Is(" << initial->getFixed() << ") "
      << "To(" << final->getFixed() << ") "
      << "By(" << increment->getFixed() << ")\n";

   for( op= stmt; op != NULL; op= (NC_op*)op->getNext() )
   {
     os << *op;
   }
   os << "NC_op@(" << this << ") "
      << "END(" << NC_COM.ist.getSymbolName(symbol) << ")\n\n";
   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opGroup::~NC_opGroup
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opGroup::~NC_opGroup( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opGroup::NC_opGroup
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opGroup::NC_opGroup( void )   // Constructor
:  NC_op()
,  group(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opGroup::generate
//
// Purpose-
//       Generate an NC_opGroup.
//
//----------------------------------------------------------------------------
NC_opGroup*                         // Resultant operator
   NC_opGroup::generate( void )     // Generate a Group operation
{
   NC_opGroup*         resultant= new NC_opGroup();

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opGroup::operate
//
// Purpose-
//       Make this the current Group object.
//
//----------------------------------------------------------------------------
void
   NC_opGroup::operate( void )      // Reset begroup
{
   NC_COM.begroup= group;           // This block is now active
   NC_COM.grpstak.lifo(group);

   if( group->ofd != NULL )         // If an associated file exists
     NC_COM.objfile= group->ofd;    // Make it active
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opGroup::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opGroup::toStream(            // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this <<") "
      << "Group(" << group << ") ";
   if( group->current_G == group )
     os << "Name(" << NC_COM.ist.getSymbolName(group) <<")\n";
   else
     os << "Name(*NONE*)\n";

   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuronAddr::~NC_opNeuronAddr
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opNeuronAddr::~NC_opNeuronAddr( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuronAddr::NC_opNeuronAddr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opNeuronAddr::NC_opNeuronAddr( void ) // Constructor
:  NC_op()
{
   resultant.f= 0;
   resultant.o= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuronAddr::generate
//
// Purpose-
//       Generate an NC_opNeuronAddr.
//
//----------------------------------------------------------------------------
NC_opNeuronAddr*                    // Resultant operator
   NC_opNeuronAddr::generate(       // Generate a NeuronAddr operation
     const char*       inpbuf,      // Current buffer
     int&              inpndx)      // Running buffer index
{
   NC_opNeuronAddr*    resultant;   // Resultant

   char                symName[1024]; // Symbol name accumulator
   NC_NeuronSymbol*    ptrSymbol;   // -> Associated symbol

   unsigned            dim;         // Number of dimensions
   int                 stmtix;      // Current statement index

   size_t              i;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   stmtix= inpndx+1;                // Skip over the '('

   //-------------------------------------------------------------------------
   // Extract the symbol name
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, stmtix); // Skip over blanks
   for(i=0; ;i++)                   // Extract the symbol
   {
     if( i >= sizeof(symName) )     // If symbol too large
     {
       NCmess(NC_msg::ID_SynSymbolTooLong, 0); // Symbol too long
       return(NULL);
     }

     if( !isalnum(inpbuf[stmtix])
         &&inpbuf[stmtix] != '_'
         &&inpbuf[stmtix] != ':' )  // If symbol delimiter found
     {
       symName[i]= '\0';
       break;
     }

     symName[i]= inpbuf[stmtix];
     stmtix++;
   }
                                    // Extract the symbol
   if( symName[0] == '\0' )         // If no name was specified
   {
     NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
     return(NULL);
   }

   //-------------------------------------------------------------------------
   // Generate resultant
   //-------------------------------------------------------------------------
   resultant= new NC_opNeuronAddr();// Generate the operator

   //-------------------------------------------------------------------------
   // Extract the dimensions
   //-------------------------------------------------------------------------
   for(dim= 0; ; dim++)             // Extract the dimensions
   {
     stmtix= ncskipb(inpbuf, stmtix); // Skip over blanks
     if( inpbuf[stmtix] == ')' )    // If end of name
       break;

     if( inpbuf[stmtix] != '[' )    // If not a new dimension
     {
       NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
       return(NULL);
     }

     if( dim >= NC_sym::MAX_DIM )     // If too many dimensions
     {
       NCmess(NC_msg::ID_DimTooManyDim, 0);
       return(NULL);
     }

     // Generate the dimension expression
     resultant->bound[dim]= NC_opFixed::generate(inpbuf, stmtix);
     if( resultant->bound[dim] == NULL )
       return(NULL);
   }

   //-------------------------------------------------------------------------
   // Resolve the symbol name
   //-------------------------------------------------------------------------
   ptrSymbol= (NC_NeuronSymbol*)NC_COM.xst.locate(symName);
   resultant->source= ptrSymbol;
   if( ptrSymbol == NULL )
   {
     NC_COM.pass1.fifo(NC_opDebug::generate());
     NC_COM.pass1.fifo(new NC_opResolveNeuronAddr(resultant, symName, dim) );
                                    // Add resolver to pass1 list
   }
   else
   {
     if( ptrSymbol->dim != dim )    // If dimension mismatch
     {
       NCmess(NC_msg::ID_DimMismatch, 0);
       return(NULL);
     }
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   inpndx= stmtix+1;                // Update statement index
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuronAddr::generate
//
// Purpose-
//       Generate an NC_opNeuronAddr.
//
//----------------------------------------------------------------------------
NC_opNeuronAddr*                    // Resultant operator
   NC_opNeuronAddr::generate(       // Generate a NeuronAddr operation
     NC_NeuronSymbol*  symbol)      // Current symbol, dimensionality 0
{
   NC_opNeuronAddr*    resultant;   // Resultant

   //-------------------------------------------------------------------------
   // Generate resultant
   //-------------------------------------------------------------------------
   resultant= new NC_opNeuronAddr();// Generate the operator

   //-------------------------------------------------------------------------
   // Resolve the symbol name
   //-------------------------------------------------------------------------
   resultant->source= symbol;
   if( symbol->dim != 0 )           // If dimension mismatch
   {
     NCmess(NC_msg::ID_DimMismatch, 0);
     return(NULL);
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuronAddr::operate
//
// Purpose-
//       Calculate the neuron address.
//
//----------------------------------------------------------------------------
void
   NC_opNeuronAddr::operate( void ) // Calculate the Neuron address
{
   long                e;           // Element number
   long                f;           // Element factor
   long                x;           // Index value

   int                 i;

   #if 0
     cout << "NeuronAddr@(" << this << ").operate()\n"; cout.flush();
     cout << *this; cout.flush();
   #endif

   //-------------------------------------------------------------------------
   // Account for dimensionality
   //-------------------------------------------------------------------------
   for(i= 0; i<source->dim; i++)
   {
     bound[i]->operate();
   }

   e= 0;                            // Initialize element number
   if( source->dim > 0 )            // If Neuron is dimensioned
   {
     f= 1;                          // Initialize element factor
     for(i= source->dim - 1; i >=0; i--) // Compute element number
     {
       x= bound[i]->getFixed();
       if( x <= 0 || x > source->bound[i] )   // If out of range
       {
         NCmess(NC_msg::ID_DimRange, 0);
         return;
       }
       e += f * (x - 1);
       f *= source->bound[i];
     }
   }

   resultant.f= source->addr.f;
   resultant.o= Neuron::index(source->addr.o, e);
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuronAddr::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opNeuronAddr::toStream(       // Write the object
     ostream&          os) const    // On this stream
{
   int                 i;

   #if 0
     os << "NC_op@(" << this << ") NeuronAddr "
        << "Source(" << source << ")\n";
     os.flush();
     if( source != NULL )
     {
       os << "Dim(" << source->dim << ")\n"; os.flush();
       for(i=0; i<source->dim; i++)
       {
         os << "Bound[" << i << "](" << bound[i] << ")\n"; os.flush();
       }
     }
   #endif

   if( source == NULL )
     os << "NC_op@(" << this << ") NeuronAddr "
        << "Source(*Unresolved*)\n";
   else
   {
     os << "NC_op@(" << this << ") NeuronAddr "
        << "F(" << resultant.f << ") "
        << "O(" << resultant.o << ") "
        << "Dim(" << source->dim << ") "
        << "Source(" << NC_COM.xst.getSymbolName(source);
     for(i=0; i<source->dim; i++)
     {
       os << "[" << bound[i]->getFixed() << "]";
     }
     os << ")\n";
   }

   return os;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opResolveNeuronAddr::~NC_opResolveNeuronAddr
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NC_opResolveNeuronAddr::~NC_opResolveNeuronAddr( void ) // Destructor
{
   if( source != NULL )
     free(source);

   source= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opResolveNeuronAddr::NC_opResolveNeuronAddr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_opResolveNeuronAddr::NC_opResolveNeuronAddr( // Constructor
     NC_opNeuronAddr*  target,      // Target operator
     const char*       source,      // Source name
     unsigned          dim)         // Dimensionality
:  NC_op()
,  target(target)
,  source(NULL)
,  dim(dim)
{
   this->source= (char*)malloc(strlen(source)+1);
   if( this->source == NULL )
   {
     fprintf(stderr, "No working storage\n");
     exit(EXIT_FAILURE);
   }

   strcpy(this->source, source);
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opResolveNeuronAddr::operate
//
// Purpose-
//       Calculate the neuron address.
//
//----------------------------------------------------------------------------
void
   NC_opResolveNeuronAddr::operate( void ) // Calculate the Neuron address
{
   NC_NeuronSymbol*    target;      // Target name

   target= (NC_NeuronSymbol*)NC_COM.xst.locate(source); // Locate the symbol
   if( target == NULL )
   {
     NCmess(NC_msg::ID_SymNotFound, 1, source); // Symbol not found
     return;
   }

   if( target->dim != dim )
   {
     NCmess(NC_msg::ID_DimMismatch, 1, source); // Dimension mismatch
     return;
   }

   this->target->source= target;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opResolveNeuronAddr::toStream
//
// Purpose-
//       Write the object.
//
//----------------------------------------------------------------------------
ostream&
   NC_opResolveNeuronAddr::toStream(       // Write the object
     ostream&          os) const    // On this stream
{
   os << "NC_op@(" << this << ") ResolveNeuronAddr "
      << "target(" << target << ") "
      << "source(" << source << ") "
      << "dim[" << dim << "]\n";

   return os;
}

