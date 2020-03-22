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
//       include.cpp
//
// Purpose-
//       Test Includes.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>

#include "NC_cfg.h"
#include "NC_com.h"
#include "NC_ifd.h"
#include "NC_ofd.h"
#include "NC_sym.h"
#include "NC_sys.h"
#include "NCdiag.h"
#include "NCtype.h"
#include "NN_cfg.h"
#include "NN_com.h"
#include "NN_psv.h"
#include "NNtype.h"

#include "Fanin.h"
#include "Neuron.h"

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define SIZEOF(foo) \
   printf("%8ld %.4lX " #foo "\n", long(sizeof(foo)), long(sizeof(foo)))

//----------------------------------------------------------------------------
//
// Subroutine-
//       MAIN
//
// Function-
//       Mainline code, display size of selected structures.
//
//----------------------------------------------------------------------------
extern int
   main( void )                     // Mainline
{
   printf("     Dec  Hex Name\n");
   printf("   ----- ---- -------------\n");
   SIZEOF(Fanin);
   SIZEOF(Neuron);

   printf("\n");
   SIZEOF(NC_com);
   SIZEOF(NN_com);

   printf("\n");
   SIZEOF(NN::FileId);
   SIZEOF(NN::PartId);
   SIZEOF(NN::Offset);
   SIZEOF(NN::FO);
   SIZEOF(NN::FPO);
   SIZEOF(NN::Weight);
   SIZEOF(NC_SizeofSymbol);

   return(0);
}
