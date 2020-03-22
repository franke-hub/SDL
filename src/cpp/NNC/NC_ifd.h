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
//       NC_ifd.h
//
// Purpose-
//       (NC) Neural Net Compiler: Input File Descriptor
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_IFD_H_INCLUDED
#define NC_IFD_H_INCLUDED

#ifndef LIST_H_INCLUDED
#include <com/List.h>
#endif

#ifndef NNTYPE_H_INCLUDED
#include "NNtype.h"
#endif

//----------------------------------------------------------------------------
// Local fix
//----------------------------------------------------------------------------
#undef  offsetof
#define offsetof(s_name,s_elem) ((size_t)&(((s_name*)nullptr)->s_elem))

//----------------------------------------------------------------------------
//
// Struct-
//       NC_ifd
//
// Purpose-
//       Describe an input file
//
//----------------------------------------------------------------------------
struct NC_ifd {                     // Input file descriptor
   SHSL_List<void>::Link
                       actlink;     // Active file link [srcstak]
   SHSL_List<void>::Link
                       srclink;     // Source file link [srclist]
   NN::FileName        filenm;      // Source file name

   unsigned char*      buffer;      // Pointer to input buffer
   int                 buffsz;      // Current buffer size
   int                 buffix;      // Current buffer index

   int                 fh;          // File handle
   int                 lineno;      // Line number
   int                 column;      // Column number

inline static NC_ifd*
   fromActlink(
     SHSL_List<void>::Link*
                       link)
{
   char*               source;
   unsigned            offset;

   source= (char*)link;
   offset= offsetof(NC_ifd, actlink);
   source -= (unsigned)offset;

   return (NC_ifd*)source;
}

inline static NC_ifd*
   fromSrclink(
     SHSL_List<void>::Link*
                       link)
{
   char*               source;
   unsigned            offset;

   source= (char*)link;
   offset= offsetof(NC_ifd, srclink);
   source -= offset;

   return (NC_ifd*)source;
}
}; // struct NC_ifd

//----------------------------------------------------------------------------
//
// Methods
//
//----------------------------------------------------------------------------
extern NC_ifd*
   NC_opn(                          // Open source file
     const char*       filenm);     // Pointer to source file name

extern void
   NC_cls(                          // Close source file
     NC_ifd*           inpifd);     // Pointer to source descriptor

extern int
   NC_rd(                           // Read from the source file
     NC_ifd*           inpifd);     // Pointer to source file descriptor

#endif // NC_IFD_H_INCLUDED
