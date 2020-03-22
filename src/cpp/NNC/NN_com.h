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
//       NN_com.h
//
// Purpose-
//       (NN) Neural Net: Globals
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NN_COM_H_INCLUDED
#define NN_COM_H_INCLUDED

#include <com/FileName.h>
#include <com/Zeroed.h>

#ifndef FANIN_H_INCLUDED
#include "Fanin.h"
#endif

#ifndef NEURON_H_INCLUDED
#include "Neuron.h"
#endif

#ifndef NNTYPE_H_INCLUDED
#include "NNtype.h"
#endif

#ifndef NN_CFG_H_INCLUDED
#include "NN_cfg.h"
#endif

//----------------------------------------------------------------------------
// NN_COM: Common area
//----------------------------------------------------------------------------
struct NN_com : public Zeroed       // Neural Net Common area
{
   NN::Tick            clock;       // Current clock tick
   NN::Tick            train;       // Current training subtick

   //-------------------------------------------------------------------------
   // File controls
   //-------------------------------------------------------------------------
   PGS                 pgs;         // Paging space
   char*               inpname;     // Input  filename
   char*               outname;     // Output filename

   char                inpfile[FILENAME_MAX+1];
   char                outfile[FILENAME_MAX+1];

   //-------------------------------------------------------------------------
   // Display controls
   //-------------------------------------------------------------------------
   NN::Color           disp_bg;     // Background color
   NN::Color           disp_master_online; // Master state: online
   NN::Color           disp_master_error; // Master state: error

   NN::Color           disp_used_lower; // Used status: up to 75%
   NN::Color           disp_used_warn;  // Used status: next  15%
   NN::Color           disp_used_upper; // Used status: upper 10%

   NN::Color           disp_rw_idle; // R/W state: idle
   NN::Color           disp_rw_read; // R/W state: read
   NN::Color           disp_rw_write; // R/W state: write

   NN::Color           disp_neuron_rd; // Neuron state: read
   NN::Color           disp_neuron_wr; // Neuron state: write
   NN::Color           disp_neuron_un; // Neuron state: dereferenced

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   int32_t             read_val[2]; // Number of read_val()s
   int32_t             read_str[2]; // Number of read_str()s

   //-------------------------------------------------------------------------
   // Operation con  trols
   //-------------------------------------------------------------------------
   unsigned char       sw_debug;    // Debugging traces
   unsigned char       sw_graph;    // Graphics traces
   unsigned char       sw_timer;    // Timing trace
   unsigned char       sw_trace;    // General traces
   int                 sw_jig;      // (Used for code development)
}; // struct NN_com

extern NN_com*         nn_com;      // The pointer to the globals
#define NN_COM         (*nn_com)    // Resolved pointer to globals

#define NN_debug NN_COM.sw_debug    // Aliases for (some) globals
#define NN_graph NN_COM.sw_graph
#define NN_timer NN_COM.sw_timer
#define NN_trace NN_COM.sw_trace
#define NN_jig   NN_COM.sw_jig

#define units_frame NN_COM.units_frame
#define units_sizes NN_COM.units_sizes

//----------------------------------------------------------------------------
// Neuron function typedefs
//----------------------------------------------------------------------------
typedef NN::Value                   // Resultant
   (NN_READVAL_T)(                  // ->READVAL function
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN);      // Neuron file identifier

typedef NN::String                  // Resultant
   (NN_READSTR_T)(                  // ->READSTR function
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN);      // Neuron file identifier

//----------------------------------------------------------------------------
// Exernal routines
//----------------------------------------------------------------------------
extern NN::Value                    // Resultant
   nnreadv(                         // Read Neuron value
     NN::FileId        fileId,      // FileId (external)
     NN::Offset        offset);     // Offset (external)

extern NN::Value                    // Resultant
   nndamage(                        // Indicate NEURON damage
     NN::FileId        fileN,       // Source file identifier
     Neuron*           ptr_N,       // -> Source neuron
     NN::Vaddr         offset);     // Damage offset

extern NN::Value                    // Resultant
   nnfanin(                         // Read weight[i] * fanin[i]
     Neuron*           ptr_N,       // -> Source neuron
     NN::FileId        fileN,       // Source file identifier
     unsigned          index);      // Fanin index(i)

extern void
   nnfinop(                         // Read (but ignore) fanins
     Neuron*           ptr_N,       // -> Source neuron
     NN::FileId        fileN);      // Source file identifier

extern NN::Value                    // Resultant
   nnsigma(                         // Sum(Value[i] * Weight[i]
     Neuron*           ptr_N,       // -> Source neuron
     NN::FileId        fileN);      // Source file identifier

extern NN::Value                    // Resultant
   nnsigm1(                         // Sum(Value[i] * Weight[i]
                                    // (Excluding element[0])
     Neuron*           ptr_N,       // -> Source neuron
     NN::FileId        fileN);      // Source file identifier

extern void*
   nnuref(                          // Access unit for reference
     NN::FileId        file,        // Target file identifier
     NN::PartId        part,        // Target partition identifier
     NN::Offset        offset);     // Target offset

extern void*
   nnuchg(                          // Access unit for update
     NN::FileId        file,        // Target file identifier
     NN::PartId        part,        // Target partition identifier
     NN::Offset        offset);     // Target offset

extern void
   nnurel(                          // Release unit access
     NN::FileId        file,        // Target file identifier
     NN::PartId        part,        // Target partition identifier
     NN::Offset        offset);     // Target offset

#define chg_fanin(fileId, offset) \
   (Fanin*)nnuchg(fileId, NN::PartFanin, offset)

#define ref_fanin(fileId, offset) \
   (Fanin*)nnuref(fileId, NN::PartFanin, offset)

#define rel_fanin(fileId, offset) \
   nnurel(fileId, NN::PartFanin, offset)

#define chg_neuron(fileId, offset) \
   (Neuron*)nnuchg(fileId, NN::PartNeuron, offset)

#define ref_neuron(fileId, offset) \
   (Neuron*)nnuref(fileId, NN::PartNeuron, offset)

#define rel_neuron(fileId, offset) \
   nnurel(fileId, NN::PartNeuron, offset)

#endif // NN_COM_H_INCLUDED
