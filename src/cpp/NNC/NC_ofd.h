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
//       NC_ofd.h
//
// Purpose-
//       (NC) Neural Net Compiler: Output File Descriptor
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_OFD_H_INCLUDED
#define NC_OFD_H_INCLUDED

#ifndef LIST_H_INCLUDED
#include <com/List.h>
#endif

#ifndef NNTYPE_H_INCLUDED
#include "NNtype.h"
#endif

//----------------------------------------------------------------------------
// Output (object) file descriptor
//----------------------------------------------------------------------------
struct NC_ofd : public SHSL_List<void>::Link { // Output file descriptor
   NN::FileName        fname;       // Output file name
   NN::FileInfo        finfo;       // Output file information

   NN::FileId          fileno;      // File number
   NN::Offset          paddr[NN::PartCount]; // Assigned addresses
}; // struct NC_ofd

#endif                              // NC_OFD_H_INCLUDED
