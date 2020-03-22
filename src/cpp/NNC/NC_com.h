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
//       NC_com.h
//
// Purpose-
//       (NC) Neural Net Compiler: Globals
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_COM_H_INCLUDED
#define NC_COM_H_INCLUDED

#include <com/FileName.h>
#include <com/List.h>
#include <com/Zeroed.h>

#ifndef NNTYPE_H_INCLUDED
#include "NNtype.h"
#endif

#ifndef NC_CFG_H_INCLUDED
#include "NC_cfg.h"
#endif

#ifndef NC_IFD_H_INCLUDED
#include "NC_ifd.h"
#endif

#ifndef NC_MSG_H_INCLUDED
#include "NC_msg.h"
#endif

#ifndef NC_OFD_H_INCLUDED
#include "NC_ofd.h"
#endif

#ifndef NC_OP_H_INCLUDED
#include "NC_op.h"
#endif

#ifndef NC_SYM_H_INCLUDED
#include "NC_sym.h"
#endif

#ifndef NC_TAB_H_INCLUDED
#include "NC_tab.h"
#endif

//----------------------------------------------------------------------------
// Constants for parameterization      (Error codes)
//----------------------------------------------------------------------------
#define AOK                     (0) // Normal
#define ERR                    (-2) // Error encountered
#define ERR_EOF                (-1) // End of file
#define ERR_SYNTAX             (-2) // Error encountered
#define ERR_LENGTH             (-3) // Invalid field length

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class CountingReader;

//----------------------------------------------------------------------------
//
// Struct-
//       NC_com
//
// Purpose-
//       NC (Neural Net Compiler) Common area
//
//----------------------------------------------------------------------------
struct NC_com : public Zeroed        // NC: Common area
{
//----------------------------------------------------------------------------
// NC_com::Enumerations and typedefs
//----------------------------------------------------------------------------
enum Pass                           // Pass number
{
   Pass0= 0,                        // File scan
   Pass1= 1,                        // Resolve symbols
   Pass2= 2,                        // Write Neurons
   Pass3= 3,                        // Count Fanins
   Pass4= 4                         // Write Fanins
}; // enum Pass

enum                                // Generic
{
   WORK_SIZE=                   512 // Sizeof wordN
}; // enum

//----------------------------------------------------------------------------
// NC_com::Constructor
//----------------------------------------------------------------------------
   NC_com( void );                  // Constructor

//----------------------------------------------------------------------------
// NC_com::Methods
//----------------------------------------------------------------------------
inline static NC_com&               // Common area
   get( void )                      // Return pointer to common area
{
   return *nc_com;
}

//----------------------------------------------------------------------------
// NC_com::File controls
//----------------------------------------------------------------------------
   NC_msg              message;     // Message table
   CountingReader*     reader;      // Source file

   char*               inpname;     // Input filename
   char*               outname;     // Output (control) filename

//----------------------------------------------------------------------------
// NC_com::VPS controls
//----------------------------------------------------------------------------
   int32_t             vps_framesize; // VPS frame size
   int32_t             vps_fileno;  // VPS number of files
   int32_t             vps_partno;  // VPS partitions per file
   int32_t             vps_framemask; // VPS frame mask

//----------------------------------------------------------------------------
// NC_com::State controls
//----------------------------------------------------------------------------
   char                initial_N;   // TRUE if neuron[initial]
                                    // statement was encountered
   int                 pass;        // Current pass
   int                 redo;        // Number of redo's in effect.
                                    // Incremented and decremented by
                                    // nc__end(), used by ncincl() to
                                    // determine whether to read from
                                    // the input file or the
                                    // statement list

//----------------------------------------------------------------------------
// NC_com::Statement accumulator
//----------------------------------------------------------------------------
   char*               exprbuff;    // Expression accumulator
   char*               stmtbuff;    // Statement accumulator
   int                 lineno;      // Statement accumulator origin
   int                 column;      // Statement accumulator origin

//----------------------------------------------------------------------------
// NC_com::Data anchors
//----------------------------------------------------------------------------
   DHSL_List<NC_op>    pass1;       // Pass 1 anchor
   DHSL_List<NC_op>    pass2;       // Pass 2 anchor
   DHSL_List<NC_op>    passN;       // Pass >2 anchor

   NC_tab              ist;         // Internal Symbol Table
   NC_tab              xst;         // External Symbol Table

   NC_opDebug*         dummyDebug;  // -> File/Line/Column descriptor
   NC_opDebug*         debug;       // -> File/Line/Column descriptor
   NC_ifd*             srcfile;     // Source file descriptor
   SHSL_List<void>     srcstak;     // Source file stack (NC_ifd.actlink)
   SHSL_List<void>     srclist;     // Source file list  (NC_ifd.srclink)

   NC_ofd*             objfile;     // Object file descriptor
   SHSL_List<NC_ofd>   objlist;     // Object file list
   int                 obj_no;      // The next file number

   NC_BeGroupSymbol*   begroup;     // Begin group descriptor
   NC_DoGroupSymbol*   dogroup;     // Do group descriptor
   SHSL_List<NC_GroupSymbol>
                       grpstak;     // Group stack

   NN::FPO             current_N;   // Current neuron

//----------------------------------------------------------------------------
// NC_com::Default values which can be overridden
//----------------------------------------------------------------------------
   int                 max_stmt;    // Sizeof(NC_COM.stmtbuff)

//----------------------------------------------------------------------------
// NC_com::Global work areas
//----------------------------------------------------------------------------
   char                inpfile[FILENAME_MAX+1];
   char                outfile[FILENAME_MAX+1];

//----------------------------------------------------------------------------
// NC_com::Statement work areas - not valid across statements
//----------------------------------------------------------------------------
   char                word0[WORK_SIZE];
   char                word1[WORK_SIZE];
   char                word2[WORK_SIZE];
   char                word3[WORK_SIZE];

//----------------------------------------------------------------------------
// NC_com::Compilation controls
//----------------------------------------------------------------------------
   char                sw_listing;  // Listing?
   char                sw_symtab;   // Symbol table?
   char                sw_msghdr;   // Message identifiers?
   int                 sw_debug;    // Debugging level
   int                 sw_jig;      // (Used for code development)

//----------------------------------------------------------------------------
// NC_com::Static attributes
//----------------------------------------------------------------------------
static NC_com*         nc_com;      // This object
};

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define NC_COM (NC_com::get())      // Resolved pointer to globals

#define NC_jig         NC_COM.sw_jig // Aliases for (some) globals
#define NC_debug       NC_COM.sw_debug

#define NCmess         NC_COM.message.message
#define NCfault        NC_COM.message.internalError

#endif // NC_COM_H_INCLUDED
