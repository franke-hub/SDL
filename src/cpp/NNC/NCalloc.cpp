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
//       NCalloc.cpp
//
// Purpose-
//       Neural Net Compiler: Space allocation.
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
#include "NC_ofd.h"
#include "NC_sys.h"

#include "NN.h"
#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NCALLOC " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // BRINGUP mode? (for debugging)

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#include "NCdiag.h"                 // Diagnostic macros

//----------------------------------------------------------------------------
// Static areas
//----------------------------------------------------------------------------
static unsigned int  size_by_part[NN::PartCount]=
{
   1,                               // 0 (PartControl)
   1,                               // 1 (PartBundle)
   sizeof(Neuron),                  // 2 (PartNeuron)
   sizeof(Fanin)                    // 3 (PartFanin)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       NCALLOC
//
// Purpose-
//       Allocate space from group.
//
//----------------------------------------------------------------------------
extern int
   ncalloc(                         // Allocate group space
     NN::FPO*          ptrfpo,      // -> Resultant address
     NN::FileId        fileno,      // File identifier
     NN::PartId        partno,      // Partition identifier
     long              count)       // Number of elements to allocate
{
   NC_ofd*             ptrofd;      // -> Object file descriptor
   NN::Vaddr           oldpart;     // Symbol origin before rounding
   NN::Vaddr           newpart;     // Symbol origin after 1 unit

   //-------------------------------------------------------------------------
   // Locate the assocated OFD
   //-------------------------------------------------------------------------
   for(ptrofd= (NC_ofd*)NC_COM.objlist.getHead();
       ptrofd != NULL;
       ptrofd= (NC_ofd*)ptrofd->getNext())
   {
     if( ptrofd->fileno == fileno )
       break;
   }
   if( ptrofd == NULL )
   {
     NCfault(__SOURCE__, __LINE__);
     return(ERR);
   }

   //-------------------------------------------------------------------------
   // Allocate space for the symbol
   //-------------------------------------------------------------------------
   oldpart= ptrofd->paddr[partno];  // Get symbol origin
   newpart= oldpart + size_by_part[partno] * count;

   ptrfpo->f= ptrofd->fileno;       // Set the symbol value
   ptrfpo->p= partno;
   ptrfpo->o= oldpart;

   //-------------------------------------------------------------------------
   // Check for overflow
   //-------------------------------------------------------------------------
   if (newpart < oldpart)           // If overflow
     NCmess(NC_msg::ID_FixFileSpace, 0);

   //-------------------------------------------------------------------------
   // Update the allocation address
   //-------------------------------------------------------------------------
   ptrofd->paddr[partno]= newpart;  // Allocate the space
   return AOK;
}

