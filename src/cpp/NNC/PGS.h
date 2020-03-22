//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2009 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PGS.h
//
// Purpose-
//       Define Paging Space (PGS) Object
//
// Last change date-
//       2009/01/01
//
//----------------------------------------------------------------------------
#ifndef PGS_H_INCLUDED
#define PGS_H_INCLUDED

#include <stdint.h>

#ifndef DEBUG_H_INCLUDED
#include <com/Debug.h>
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define PGS_FNSIZE              256 // Maximum file name size

#define PGSFILE_T      PGS::FileId  // Typedef synonyms
#define PGSPART_T      PGS::PartId
#define PGSOFFSET_T    PGS::Offset
#define PGSRADDR_T     PGS::Raddr
#define PGSRSIZE_T     PGS::Rsize
#define PGSVADDR_T     PGS::Vaddr
#define PGSVSIZE_T     PGS::Vsize
#define PGSXADDR_T     PGS::Xaddr

//----------------------------------------------------------------------------
//
// Class-
//       PGS
//
// Purpose-
//       Paging Space.
//
//       A paging space is a virtual storage area. Each paging space is
//       independent. Users access frames which are then brought into real
//       storage for use. They remain in real storage until they are released.
//
// Usage notes-
//       Since frames are brought into real storage individually, no single
//       data item may cross a frame boundary.
//
//       The cold and warm methods define the frame size and the number of
//       frames that can be resident at any point. The default frame size is
//       4096 and the default frame count is determined by the amount of real
//       storage available when the method is invoked.
//
//       The accessChg and accessRef methods bring an external frame into
//       real storage, allocating external (disk) storage if required.
//
//       The allocate method allocates disk storage from a particular file,
//       but does not access it.
//
//----------------------------------------------------------------------------
class PGS {                         // Paging Space
//----------------------------------------------------------------------------
// PGS::Enumerations
//----------------------------------------------------------------------------
public:
enum RC                             // Return codes
{  RC_InternalLogic= (-1)           // Internal logic error
,  RC_NORMAL= 0                     // Normal, no error
,  RC_Vaddr_On_Disk                 // Virtual address not accessed
,  RC_Vaddr_Invalid                 // Virtual address not valid
,  RC_InvalidParameter              // Invalid parameter
,  RC_NoRealStorage                 // Real storage not available
,  RC_NoStorage                     // Working storage not available
,  RC_NotInitialized                // Not initialized
,  RC_Paging_IO                     // Paging I/O error
}; // enum RC

//----------------------------------------------------------------------------
// PGS::Typedefs
//----------------------------------------------------------------------------
public:
typedef void*          Raddr;       // Real storage address
typedef uint32_t       Rsize;       // Real storage length
typedef uint64_t       Vaddr;       // Virtual storage address
typedef uint32_t       Vsize;       // Virtual storage length
typedef uint64_t       Xaddr;       // External (frame) address

//----------------------------------------------------------------------------
// PGS::Constructors
//----------------------------------------------------------------------------
public:
   ~PGS( void );                    // Destructor
   PGS( void );                     // Constructor

int                                 // Return code, (0 OK)
   cold(                            // Initialize PGS, cold start
     const char*       control,     // The control file name
     uint32_t          framesz= 0,  // Page Space frame size
     uint32_t          frameno= 0); // Page Space real frame count

int                                 // Return code, (0 OK)
   warm(                            // Initialize PGS, warm start
     const char*       control,     // The control file name
     uint32_t          framesz= 0,  // Page Space frame size
     uint32_t          frameno= 0); // Page Space real frame count

void
   term( void );                    // Terminate PGS

protected:
int                                 // Return code, (0 OK)
   init(                            // Initialize PGS
     int               files,       // Number of files
     uint32_t          framesz,     // Page Space frame size
                                    // (Power of 2, range 256..524288)
     uint32_t          realframeno, // Page Space real frame count
                                    // (Minimum value= 64)
     uint32_t          virtframeno);// Page Space virtual frame count
                                    // (Minimum value= realframeno

//----------------------------------------------------------------------------
// PGS::Accessor Methods
//----------------------------------------------------------------------------
public:
long                                // The frame count
   getFrameCount( void );           // Retrieve the number of frames

long                                // The frame size
   getFrameSize( void );            // Retrieve the size of each frame

//----------------------------------------------------------------------------
// PGS::Methods
//----------------------------------------------------------------------------
public:
Raddr                               // Associated Real Address
   accessChg(                       // Access virtual address (for update)
     Vaddr             vaddr);      // Virtual Address

Raddr                               // Associated Real Address
   accessRef(                       // Access virtual address (for read)
     Vaddr             vaddr);      // Virtual Address

Raddr                               // Associated Read Address
   accessSCI(                       // Set change indicator (promote to update)
     Vaddr             vaddr);      // Virtual Address

void
   release(                         // Release access
     Vaddr             vaddr);      // Virtual Address

RC                                  // Return code
   status(                          // Verify virtual frame status
     Vaddr             vaddr);      // Virtual Address

//----------------------------------------------------------------------------
// Allocators
RC                                  // Return code
   allocate(                        // Allocate virtual storage
     int               fileno,      // From this file (0 if any)
     Vaddr             vaddr);      // Virtual Address

int                                 // The inserted file number, 0 if error
   insFile(                         // Insert a new File
     const char*       filenm);     // The file's name

//----------------------------------------------------------------------------
// Diagnostics (may update diag_trace, so methods cannot be const)
int                                 // Return code (0 OK)
   check( void );                   // Internal diagnostics

int                                 // Return code (0 OK)
   debug( void );                   // Internal diagnostics (to trace file)

void
   statistics( void );              // Write statistics

//----------------------------------------------------------------------------
// PGS::Internal Methods
//----------------------------------------------------------------------------
protected:
struct IOD;                         // Forward reference, internal object
struct RFD;                         // Forward reference, internal object
struct VFD;                         // Forward reference, internal object

void
   abort(                           // PGS abort
     const char*       filenm,      // Failing file name
     int               lineno,      // Failing line number
     const char*       msg,         // Error message
                       ...);        // PRINTF arguments

void
   error(                           // PGS error
     const char*       filenm,      // Failing file name
     int               lineno,      // Failing line number
     const char*       msg,         // Error message
                       ...);        // PRINTF arguments

void
   debugf(                          // Write debug message
     const char*       msg,         // Error message
                       ...);        // PRINTF arguments

void
   errorf(                          // Write error message
     const char*       msg,         // Error message
                       ...);        // PRINTF arguments

void
   tracef(                          // Write trace message
     const char*       msg,         // Trace message
                       ...);        // PRINTF arguments

RFD*                                // -> RFD
   accessLoad(                      // Access a frame, loading if required
     Vaddr             vaddr);      // Virtual Address

RFD*                                // -> RFD
   accessRead(                      // Access a frame without loading it
     Vaddr             vaddr);      // Virtual Address

VFD*                                // -> VFD
   allocateVFD(                     // Allocate virtual storage
     Vaddr             vaddr,       // Virtual Address
     int               fileno= 0);  // File number (0 if any)

void
   buildHashArray( void );          // (Re)Build the VFD hash array

void
   frameRD(                         // Read a frame
     RFD*              rfd);        // -> RFD

void
   frameWR(                         // Write a frame
     RFD*              rfd);        // -> RFD

void
   traceOp(                         // Trace an access operation
     const char*       opCode,      // Operation code
     RFD*              ptrrfd,      // -> RFD
     Vaddr             vaddr);      // Virtual Address

//----------------------------------------------------------------------------
// PGS::Attributes
//----------------------------------------------------------------------------
protected:
   //-------------------------------------------------------------------------
   // Configuration controls
   //-------------------------------------------------------------------------
   uint32_t            framesize;   // Frame size
   uint32_t            framemask;   // Frame element offset mask
   unsigned int        framelog2;   // LOG2(framesize)

   //-------------------------------------------------------------------------
   // Translation controls
   //-------------------------------------------------------------------------
   unsigned int        files;       // The number of files allocated
   unsigned int        fileu;       // The number of files used
   unsigned int        filen;       // The next file to use for allocation
   IOD*                fdlist;      // -> IOD array

   // The array of real frames is created when the page space is started
   // This array never changes after that time.
   uint32_t            rframes;     // Real Frame element count
   RFD*                rfdall;      // -> Real Frame Descriptor array
   RFD*                rfdfree;     // Free Frame list header
   void*               storage;     // The backing storage

   // The array of virtual frames is created when the page space is started
   // This array may be expanded, but not contracted, when frames are added.
   // This array should be at least 1/4 the size of of the vfdall array.
   uint32_t            vframes;     // Virtual Frame element count
   VFD**               vfdhash;     // -> Virtual Frame Hash table array

   // The array of external frames is created when the page space is started
   // This array may be expanded, but not contracted, when frames are added.
   // This array contains the complete list of virtual frames
   uint32_t            xframes;     // Number of external frames allocated
   uint32_t            xframeu;     // Number of external frames used
   VFD*                vfdall;      // -> Virtual Frame Descriptor array

   //-------------------------------------------------------------------------
   // Reclaim controls
   //-------------------------------------------------------------------------
   RFD*                reclaim_h;   // Reclaim array header
   RFD*                reclaim_t;   // Reclaim array trailer

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   uint64_t            stat_opchg;  // Number of CHG operations
   uint64_t            stat_opref;  // Number of REF operations
   uint64_t            stat_oprel;  // Number of REL operations
   uint64_t            stat_opsci;  // Number of SCI operations

   uint64_t            stat_opfrd;  // Number of FileRD operations
   uint64_t            stat_opfwr;  // Number of FileWR operations

   uint64_t            stat_alloc;  // Number of free frame allocations
   uint64_t            stat_allru;  // Number of LRU  frame allocations
   uint64_t            stat_recrd;  // REclaim (read)  counter
   uint64_t            stat_recwr;  // REclaim (write) counter
   uint64_t            stat_reuse;  // REuse (resident) counter

   uint64_t            stat_hashmiss; // Number of misses on the hash list
   uint64_t            stat_reorders; // Number of hash list reorder events

   //-------------------------------------------------------------------------
   // Diagnostic controls
   //-------------------------------------------------------------------------
   Debug               diag_trace;  // Diagnostic trace file
   uint32_t            diag_level;  // Diagnostic level
   unsigned char       diag_flags[64]; // Explicit diagnostic controls

   //-------------------------------------------------------------------------
   // Operation controls
   //-------------------------------------------------------------------------
   unsigned char       initialized; // TRUE if successfully initialized
   unsigned char       sw_debug;    // Debugging traces
   unsigned char       sw_trace;    // General traces
   unsigned char       sw_jig;      // (Used for code development)
}; // class PGS

#endif // PGS_H_INCLUDED
