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
//       NNrdval.cpp
//
// Purpose-
//       Neural Net: Read neuron value.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       NNREADV    Read neuron value
//       NNREADS    Read neuron string
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include <com/Debug.h>

#include "NN_com.h"
#include "NNtype.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NNRDVAL " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If not already defined
#define HCDM                   TRUE // Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// External routine prototypes
//----------------------------------------------------------------------------
   extern NN_READSTR_T NN_abort_S;  // Abort

   extern NN_READVAL_T NN_abort_V;  // Abort
   extern NN_READVAL_T NN_add_V;    // Add
   extern NN_READVAL_T NN_and_V;    // And
   extern NN_READVAL_T NN_const_V;  // Constant
   extern NN_READVAL_T NN_clock_V;  // Clock
   extern NN_READVAL_T NN_div_V;    // Divide
   extern NN_READVAL_T NN_decr1_V;  // Decrement
   extern NN_READVAL_T NN_incr1_V;  // Increment
   extern NN_READVAL_T NN_mul_V;    // Multiply
   extern NN_READVAL_T NN_nand_V;   // Nand
   extern NN_READVAL_T NN_nor_V;    // Nor
   extern NN_READVAL_T NN_or_V;     // Or
   extern NN_READVAL_T NN_sub_V;    // Subtract
   extern NN_READVAL_T NN_sigmd_V;  // Sigmoid
   extern NN_READVAL_T NN_store_V;  // Store
   extern NN_READVAL_T NN_until_V;  // Until
   extern NN_READVAL_T NN_while_V;  // While

//----------------------------------------------------------------------------
// Static areas
//----------------------------------------------------------------------------
static const char*     typeval[Neuron::TypeCOUNT]= { // Type vector
     "Abort",                       // 00 Abort
     "Constant",                    // 01 Constant
     "Clock",                       // 02 Clock
     "Type_003",                    // 03
     "FileRD",                      // 04
     "FileWR",                      // 05
     "Store",                       // 06 Store
     "Type_007",                    // 07
     "Type_008",                    // 08
     "Type_009",                    // 09
     "Type_010",                    // 10
     "Type_011",                    // 11
     "Type_012",                    // 12
     "Type_013",                    // 13
     "Type_014",                    // 14
     "Type_015",                    // 15
     "Type_016",                    // 16
     "Type_017",                    // 17
     "Type_018",                    // 18
     "Train",                       // 19

     // Arithmetics
     "Inc",                         // 20 Inc
     "Dec",                         // 21 Dec
     "Add",                         // 22 Add
     "Sub",                         // 23 Sub
     "Mul",                         // 24 Mul
     "Div",                         // 25 Div
     "Type_026",                    // 26
     "Abs",                         // 27 Abs
     "Neg",                         // 28 Neg
     "Sigmoid",                     // 29 Sigmoid
     "Type_030",                    // 30
     "Type_031",                    // 31
     "Type_032",                    // 32
     "Type_033",                    // 33
     "Type_034",                    // 34
     "Type_035",                    // 35
     "Type_036",                    // 36
     "Type_037",                    // 37
     "Type_038",                    // 38
     "Type_039",                    // 39

     // Booleans
     "And",                         // 40 And
     "Or",                          // 41 Or
     "Nand",                        // 42 Nand
     "Nor",                         // 43 Nor
     "Type_044",                    // 44
     "Type_045",                    // 45
     "Type_046",                    // 46
     "Type_047",                    // 47
     "Type_048",                    // 48
     "Type_049",                    // 49

     // Logics
     "If",                          // 50 If
     "While",                       // 51 While
     "Until",                       // 52 Until
     "Type_053",                    // 53
     "Type_054",                    // 54
     "Type_055",                    // 55
     "Type_056",                    // 56
     "Type_057",                    // 57
     "Type_058",                    // 58
     "Type_059",                    // 59
     };

//----------------------------------------------------------------------------
// READVAL() vector list
//----------------------------------------------------------------------------
static NN_READVAL_T*   readval[Neuron::TypeCOUNT]= { // READV vector
     NN_abort_V,                    // 00 Abort
     NN_const_V,                    // 01 Constant
     NN_clock_V,                    // 02 Clock
     NN_sigmd_V,                    // 03
     NN_sigmd_V,                    // 04
     NN_sigmd_V,                    // 05
     NN_store_V,                    // 06 Store
     NN_sigmd_V,                    // 07
     NN_sigmd_V,                    // 08
     NN_sigmd_V,                    // 09
     NN_sigmd_V,                    // 10
     NN_sigmd_V,                    // 11
     NN_sigmd_V,                    // 12
     NN_sigmd_V,                    // 13
     NN_sigmd_V,                    // 14
     NN_sigmd_V,                    // 15
     NN_sigmd_V,                    // 16
     NN_sigmd_V,                    // 17
     NN_sigmd_V,                    // 18
     NN_sigmd_V,                    // 19

     // Arithmetics
     NN_incr1_V,                    // 20 Inc
     NN_decr1_V,                    // 21 Dec
     NN_add_V,                      // 22 Add
     NN_sub_V,                      // 23 Sub
     NN_mul_V,                      // 24 Mul
     NN_div_V,                      // 25 Div
     NN_sigmd_V,                    // 26
     NN_sigmd_V,                    // 27 Abs
     NN_sigmd_V,                    // 28 Neg
     NN_sigmd_V,                    // 29 Sigmoid
     NN_sigmd_V,                    // 30
     NN_sigmd_V,                    // 31
     NN_sigmd_V,                    // 32
     NN_sigmd_V,                    // 33
     NN_sigmd_V,                    // 34
     NN_sigmd_V,                    // 35
     NN_sigmd_V,                    // 36
     NN_sigmd_V,                    // 37
     NN_sigmd_V,                    // 38
     NN_sigmd_V,                    // 39

     // Booleans
     NN_and_V,                      // 40 And
     NN_or_V,                       // 41 Or
     NN_nand_V,                     // 42 Nand
     NN_nor_V,                      // 43 Nor
     NN_sigmd_V,                    // 44
     NN_sigmd_V,                    // 45
     NN_sigmd_V,                    // 46
     NN_sigmd_V,                    // 47
     NN_sigmd_V,                    // 48
     NN_sigmd_V,                    // 49

     // Logics
     NN_until_V,                    // 50 Until
     NN_while_V,                    // 51 While
     NN_sigmd_V,                    // 52
     NN_sigmd_V,                    // 53
     NN_sigmd_V,                    // 54
     NN_sigmd_V,                    // 55
     NN_sigmd_V,                    // 56
     NN_sigmd_V,                    // 57
     NN_sigmd_V,                    // 58
     NN_sigmd_V                     // 59
     };

//----------------------------------------------------------------------------
// READSTR() vector list
//----------------------------------------------------------------------------
static NN_READSTR_T*   readstr[Neuron::TypeCOUNT]= { // READS vector
     NN_abort_S,                    // 00 Error function
     NN_abort_S,                    // 01
     NN_abort_S,                    // 02
     NN_abort_S,                    // 03
     NN_abort_S,                    // 04
     NN_abort_S,                    // 05
     NN_abort_S,                    // 06
     NN_abort_S,                    // 07
     NN_abort_S,                    // 08
     NN_abort_S,                    // 09
     NN_abort_S,                    // 10
     NN_abort_S,                    // 11
     NN_abort_S,                    // 12
     NN_abort_S,                    // 13
     NN_abort_S,                    // 14
     NN_abort_S,                    // 15
     NN_abort_S,                    // 16
     NN_abort_S,                    // 17
     NN_abort_S,                    // 18
     NN_abort_S,                    // 19
     NN_abort_S,                    // 20
     NN_abort_S,                    // 21
     NN_abort_S,                    // 22
     NN_abort_S,                    // 23
     NN_abort_S,                    // 24
     NN_abort_S,                    // 25
     NN_abort_S,                    // 26
     NN_abort_S,                    // 27
     NN_abort_S,                    // 28
     NN_abort_S,                    // 29
     NN_abort_S,                    // 30
     NN_abort_S,                    // 31
     NN_abort_S,                    // 32
     NN_abort_S,                    // 33
     NN_abort_S,                    // 34
     NN_abort_S,                    // 35
     NN_abort_S,                    // 36
     NN_abort_S,                    // 37
     NN_abort_S,                    // 38
     NN_abort_S,                    // 39
     NN_abort_S,                    // 40
     NN_abort_S,                    // 41
     NN_abort_S,                    // 42
     NN_abort_S,                    // 43
     NN_abort_S,                    // 44
     NN_abort_S,                    // 45
     NN_abort_S,                    // 46
     NN_abort_S,                    // 47
     NN_abort_S,                    // 48
     NN_abort_S,                    // 49
     NN_abort_S,                    // 50
     NN_abort_S,                    // 51
     NN_abort_S,                    // 52
     NN_abort_S,                    // 53
     NN_abort_S,                    // 54
     NN_abort_S,                    // 55
     NN_abort_S,                    // 56
     NN_abort_S,                    // 57
     NN_abort_S,                    // 58
     NN_abort_S                     // 59
     };

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnreads
//
// Purpose-
//       Read Neuron string.
//
//----------------------------------------------------------------------------
extern NN::String
   nnreads(                         // Read Neuron string
     NN::FileId        fileId,      // FileId
     NN::Offset        offset)      // Offset
{
   NN::String          resultant;   // Resultant
   Neuron*             ptrN;        // -> Neuron (Internal address)

   //-------------------------------------------------------------------------
   // Validate the neuron
   //-------------------------------------------------------------------------
   ptrN= chg_neuron(fileId, offset);  // Access the neuron
   if( ptrN == NULL )               // If access failure
     return(NULL);

   if( ptrN->type >= Neuron::TypeCOUNT ) // If invalid type
   {
     errorf(__SOURCE__);            // Error identification
     printf("%.2ld:0x%.8lX.%.8lX Neuron type(%d) invalid\n",
            long(fileId), long(offset>>32),long(offset),
            ptrN->type);
     return(NULL);
   }

   //-------------------------------------------------------------------------
   // Read the neuron
   //-------------------------------------------------------------------------
   resultant= readstr[ptrN->type](ptrN, fileId);

   //-------------------------------------------------------------------------
   // Exit
   //-------------------------------------------------------------------------
   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnreadv
//
// Purpose-
//       Read Neuron value.
//
//----------------------------------------------------------------------------
extern NN::Value
   nnreadv(                         // Read Neuron value
     NN::FileId        fileId,      // FileId
     NN::Offset        offset)      // Offset
{
   NN::Value           resultant;   // Resultant
   Neuron*             ptrN;        // -> Neuron (Internal address)
   unsigned            type;        // Working type

   //-------------------------------------------------------------------------
   // Access the neuron
   //-------------------------------------------------------------------------
   ptrN= chg_neuron(fileId, offset);// Access the neuron
   if( ptrN == NULL )               // If access failure
   {
     printf("%.2ld:0x%.8lX.%.8lX [%-8s]\n",
            long(fileId), long(offset>>32),long(offset),
            "IO_ERROR");
     return(0);
   }

   //-------------------------------------------------------------------------
   // Validate the neuron
   //-------------------------------------------------------------------------
   type= ptrN->type;                // Extract the type
   if( type == Neuron::TypeError    // If invalid type
       ||type >= Neuron::TypeCOUNT ) // If invalid type
   {
     printf("\n");
     printf("%.2ld:0x%.8lX.%.8lX Type[%d] invalid\n",
            long(fileId), long(offset>>32), long(offset),
            type);

     rel_neuron(fileId, offset);
     return(0);
   }

   //-------------------------------------------------------------------------
   // Read the neuron
   //-------------------------------------------------------------------------
// if( (ptrN->clock == NN_COM.clock && ptrN->train == NN_COM.train )
//                                  // If resultant already computed
//     ||ptrN->ex.disabled )        // or the neuron is disabled
   if( ptrN->clock == NN_COM.clock  // If resultant already computed
       ||ptrN->ex.disabled )        // or the neuron is disabled
     resultant= ptrN->value;        // Use current resultant

   else                             // If computation required
   {
     if( NN_debug )                 // If debug is active
     {
       printf("%.2ld:0x%.8lX.%.8lX [%-8s] (rdval)\n",
              long(fileId), long(offset>>32), long(offset),
              typeval[type]);
     }

     ptrN->clock= NN_COM.clock;     // Update current tick
//   ptrN->train= NN_COM.train;

     resultant= readval[type](ptrN, fileId); // Read the neuron
     ptrN->value= resultant;        // Update the resultant
   }

   //-------------------------------------------------------------------------
   // Trace the neuron
   //-------------------------------------------------------------------------
   if( NN_trace )                   // If trace is active
   {
     printf("%.2ld:0x%.8lX.%.8lX [%-8s] %8f\n",
            long(fileId), long(offset>>32), long(offset),
            typeval[type], (double)resultant);
   }

   //-------------------------------------------------------------------------
   // Update read_val() count
   //-------------------------------------------------------------------------
   NN_COM.read_val[1]++;
   if( NN_COM.read_val[1] == 0 )
     NN_COM.read_val[0]++;

   //-------------------------------------------------------------------------
   // Exit, release neuron
   //-------------------------------------------------------------------------
   rel_neuron(fileId, offset);      // Release the neuron
   return(resultant);
}

