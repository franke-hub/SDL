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
//       NN_pgs.cpp
//
// Purpose-
//       Neural Net Virtual Paging Subsystem extentions.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       nnuchg     Access unit for change
//       nnuref     Access unit for reference
//       nnurel     Release access
//
//----------------------------------------------------------------------------
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "hcdm.h"
#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NN_PGS  " // Source filename

//----------------------------------------------------------------------------
//
// Subroutine-
//       fpo
//
// Purpose-
//       Compute virtual address given file, part, offset.
//
//----------------------------------------------------------------------------
inline PGSVADDR_T
   fpo(                             // Diagnose PGS
     NN::FileId        fileno,      // Target file identifier
     NN::PartId        partno,      // Target part identifier
     NN::Offset        offset)      // Target offset
{
   PGSVADDR_T          vaddr;       // Resultant virtual address

   vaddr= ( ((PGSVADDR_T)fileno << 56)
          | ((PGSVADDR_T)partno << 48)
          | ((PGSVADDR_T)offset      )
          );

   return vaddr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnuchg
//
// Purpose-
//       Access unit for update.
//
//----------------------------------------------------------------------------
extern void*
   nnuchg(                          // Access unit for update
     NN::FileId        file,        // Target file identifier
     NN::PartId        part,        // Target part identifier
     NN::Offset        offset)      // Target offset
{
   unsigned char*      ptrunit;     // Pointer to unit

   ptrunit= (unsigned char*)NN_COM.pgs.accessChg(fpo(file, part, offset));
   if( ptrunit == NULL )
   {
     errorf("%.8lX=nnuchg(%.2ld,%.2ld,0x%.8lX)\n",
            P2L(ptrunit), (long)file, (long)part, (long)offset);
     exit(EXIT_FAILURE);
   }

   if( HCDM )
     tracef("%.8lX=nnuchg(%.2ld,%.2ld,0x%.8lX) 0x%.2x%.2x%.2x%.2x\n",
            P2L(ptrunit), (long)file, (long)part, (long)offset,
            *(ptrunit+0), *(ptrunit+1), *(ptrunit+2), *(ptrunit+3));

   return(ptrunit);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnuref
//
// Purpose-
//       Access unit for reference.
//
//----------------------------------------------------------------------------
extern void*
   nnuref(                          // Access unit for reference
     NN::FileId        file,        // Target file identifier
     NN::PartId        part,        // Target part identifier
     NN::Offset        offset)      // Target offset
{
   unsigned char*      ptrunit;     // Pointer to unit

   ptrunit= (unsigned char*)NN_COM.pgs.accessRef(fpo(file, part, offset));
   if( ptrunit == NULL )
   {
     errorf("%.8lX=nnuref(%.2ld,%.2ld,0x%.8lX)\n",
            P2L(ptrunit), (long)file, (long)part, (long)offset);
     exit(EXIT_FAILURE);
   }

   if( HCDM )
     tracef("%.8lX=nnuref(%.2ld,%.2ld,0x%.8lX) 0x%.2x%.2x%.2x%.2x\n",
            P2L(ptrunit), (long)file, (long)part, (long)offset,
            *(ptrunit+0), *(ptrunit+1), *(ptrunit+2), *(ptrunit+3));

   return(ptrunit);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnurel
//
// Purpose-
//       Release unit access.
//
//----------------------------------------------------------------------------
extern void
   nnurel(                          // Release unit access
     NN::FileId        file,        // Target file identifier
     NN::PartId        part,        // Target part identifier
     NN::Offset        offset)      // Target offset
{
   NN_COM.pgs.release(fpo(file, part, offset));
   if( HCDM )
   {
     unsigned char* ptrunit= (unsigned char*)NN_COM.pgs.accessRef(fpo(file, part, offset));
     tracef("         nnurel(%.2ld,%.2ld,0x%.8lX) 0x%.2x%.2x%.2x%.2x\n",
            (long)file, (long)part, (long)offset,
            *(ptrunit+0), *(ptrunit+1), *(ptrunit+2), *(ptrunit+3));
     NN_COM.pgs.release(fpo(file, part, offset));
   }
}

