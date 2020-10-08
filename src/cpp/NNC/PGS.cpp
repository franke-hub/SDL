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
//       PGS.cpp
//
// Purpose-
//       PGS object methods.
//
// Last change date-
//       2009/01/01
//
// ControlFile-
//       PGS.INI
//
//           [Debug]
//           filename= filename   ; Trace filename, default "PGS.OUT")
//           traceLevel = 0..19   ; Less..More tracing (> 10 HCDM)
//
//----------------------------------------------------------------------------
#define __STDC_FORMAT_MACROS        // For linux inttypes.h

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>               // For PRId64
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <com/syslib.h>
#include <com/Debug.h>
#include <com/FileInfo.h>
#include <com/ParseINI.h>
#include <com/nativeio.h>

#include "PGS.h"

//----------------------------------------------------------------------------
// Compilation controls
//----------------------------------------------------------------------------
#define PGS_consistency_checking    // Bringup consistency checking logic
#define PGS_hash_move_to_front      // Move hash to front of list

#ifdef _OS_CYGWIN
//#ifdef __x86_64__                 // GCC specific
//  #define open64  _open64
//  #define lseek64 _lseek64
//
    #define open64  open
    #define lseek64 lseek
//
//  extern "C" {
//    extern int open64 _PARAMS ((const char*, int, ...));
//  } // extern "C"
//#else
//  #define open64  _open64
//  #define lseek64 _lseek64
//
//  extern "C" {
//    extern int open64 _PARAMS ((const char*, int, ...));
//    _off64_t _EXFUN(lseek64, (int __filedes, _off64_t __offset, int __whence ));
//  } // extern "C"
//#endif
#endif

#ifdef _OS_WIN
  #define open64  _open
  #define lseek64 _lseek
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define CONTROL_FRAMESZ         256 // The minimum framesize
#define DIAGFILE_NAME     "PGS.INI" // The control file's name

#define PGSINIT_DUPINIT        (-1) // Error: already initialized
#define PGSINIT_COMPLETE          0 // Normal completion
#define PGSINIT_MEMORY            1 // Storage not available
#define PGSINIT_CONTROL           2 // Control file error
#define PGSINIT_DATAFILE          3 // Data file error
#define PGSINIT_FRAMESZ         901 // Invalid framesz

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define PGSDIAG0                  0 // Level 0 diagnostic
                                    // Built-in self checks,
                                    // cannot be disabled.

#define PGSDIAG1                  1 // Level 1 diagnostic
                                    // Built-in self checks,
                                    // may be disabled.

#define PGSDIAG2                  2 // Level 2 diagnostic
                                    // Statistical display.

#define PGSDIAG3                  3 // Level 3 diagnostic
                                    // Informational display.

#define PGSDIAG4                  4 // Level 4 diagnostic

#define PGSDIAG5                  5 // Level 5 diagnostic

#define PGSDIAG6                  6 // Level 6 diagnostic

#define PGSDIAG7                  7 // Level 7 diagnostic

#define PGSDIAG8                  8 // Level 8 diagnostic

#define PGSDIAG9                  9 // Level 9 diagnostic
                                    // Hard-core debug mode.
                                    // Must be explicitly enabled.

//----------------------------------------------------------------------------
// Diagnostic assignments
//----------------------------------------------------------------------------
#define PGSDIAGL_HCDM      PGSDIAG9 // Hard Core debug mode
#define PGSDIAGL_STAT      PGSDIAG2 // Statistical display
#define PGSDIAGL_INFO      PGSDIAG3 // Information display

#define PGSDIAGL_NULL      PGSDIAG0 // NULL, has no effect
#define PGSDIAGN_NULL             0
#define PGSDIAGM_NULL          0x00

#define PGSDIAGL_PAGE      PGSDIAG7 // Page-in/Page-out
#define PGSDIAGN_PAGE             1
#define PGSDIAGM_PAGE          0x80

#define PGSDIAGL_OPEN      PGSDIAG7 // Open/close
#define PGSDIAGN_OPEN             1
#define PGSDIAGM_OPEN          0x40

//----------------------------------------------------------------------------
// Trace field definitions
//----------------------------------------------------------------------------
#define TRACE_CHG_WORD 0
#define TRACE_REF_WORD 0
#define TRACE_SCI_WORD 0
#define TRACE_REL_WORD 0

#define TRACE_CHG_MASK 0x01
#define TRACE_REF_MASK 0x02
#define TRACE_SCI_MASK 0x04
#define TRACE_REL_MASK 0x10

//----------------------------------------------------------------------------
//
// Struct-
//       CFH
//
// Purpose-
//       Define the Control File Header
//
// Usage-
//       The control file layout is as follows:
//         CFH          (This structure)
//         FND          File Name Descriptor array[cfhfiles]
//         VFD          Virtual Frame Descriptor array[cfhframeno]
//
//----------------------------------------------------------------------------
#define CFH_CBID      "PGSMFILE"    // Control block identifier
#define CFH_VBID      "V1.0    "    // Version identifier
#define CFH_RBID      "R1.0    "    // Release identifier
#define CFH_ENID 0x0123456789abcdefLL // Endian identifier

struct CFH {                        // Control file header
   unsigned char       cbid[8];     // File identifier
   unsigned char       user[8];     // User area
   unsigned char       vbid[8];     // Version identifier
   unsigned char       rbid[8];     // Release identifier
   uint64_t            enid;        // Endian identifier
   unsigned char       _0028[24];   // Reserved for expansion

   //                               // Offset 0x0040
   uint32_t            files;       // Number of files
   uint32_t            framesz;     // Frame size
   uint32_t            frameno;     // Frame count

   unsigned char       _0002[52];   // Reserved for expansion
}; // struct CFH

//----------------------------------------------------------------------------
//
// Struct-
//       FrameInfo
//
// Purpose-
//       Frame size information (for list of valid frame sizes)
//
//----------------------------------------------------------------------------
struct FrameInfo {                  // Frame information
   uint32_t            infoLog2;    // Log2(frame size)
   uint32_t            _0001;       // Reserved
   uint32_t            infoSize;    // The frame size
   uint32_t            infoMask;    // The frame size mask
}; // struct FrameInfo

//----------------------------------------------------------------------------
//
// Class-
//       PGS_File
//
// Purpose-
//       Paging Space file descriptor
//
//----------------------------------------------------------------------------
class PGS_File {                    // Paging Space file descriptor
//----------------------------------------------------------------------------
// PGS_File::Constructors/Destructors
//----------------------------------------------------------------------------
public:
   ~PGS_File( void );               // Destructor
   PGS_File( void );                // Constructor

int                                 // Return code (0 OK)
   openCold(                        // Initialize PGS_File, cold start
     const char*       filename);   // The file name

int                                 // Return code (0 OK)
   openWarm(                        // Initialize PGS_File, warm start
     const char*       filename);   // The file name

void
   close( void );                   // Terminate PGS_File

//----------------------------------------------------------------------------
// PGS_File::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   rd(                              // Read frame
     void*             daddr,       // Data address
     int32_t           dsize,       // Data size
     PGSXADDR_T        xaddr);      // Data offset

int                                 // Return code (0 OK)
   wr(                              // Write frame
     const void*       daddr,       // Data address
     int32_t           dsize,       // Data size
     PGSXADDR_T        xaddr);      // Data offset

inline int                          // The handle
   getHandle( void )                // Get handle
{
   return handle;
}

//----------------------------------------------------------------------------
// PGS_File::Attributes
//----------------------------------------------------------------------------
protected:
   long                handle;      // The file handle
   PGSXADDR_T          maxXaddr;    // The largest offset written

   static char         zeros[4096]; // Constant null area
}; // class PGS_File

//----------------------------------------------------------------------------
//
// Struct-
//       PGS::IOD
//
// Purpose-
//       File Descriptor
//
//----------------------------------------------------------------------------
struct PGS::IOD {                   // File Descriptor
   char                name[PGS_FNSIZE]; // The file name
   PGSXADDR_T          allocFrameNo;// The largest frame allocated
   PGS_File            file;        // File descriptor
}; // struct PGS::IOD

//----------------------------------------------------------------------------
//
// Struct-
//       PGS::RFD
//
// Purpose-
//       Real Frame Descriptor
//
//----------------------------------------------------------------------------
struct PGS::RFD {                   // Real Frame Descriptor
enum FSM                            // Finite State Machine values
{  RFD_RESET= 0                     // Reset, available
,  RFD_AVAIL= RFD_RESET             // Available
,  RFD_ALLOC                        // Allocated
,  RFD_ONLRU                        // On LRU List
}; // enum FSM

   RFD*                next;        // Chain pointer
   RFD*                prev;        // Chain pointer
   VFD*                vfd;         // -> Virtual Frame Descriptor
   PGSRADDR_T          raddr;       // Associated Real Address
   int32_t             _001;        // Filler (Needed in 64-bit mode)
   unsigned char       fsm;         // State machine
   unsigned char       chgi;        // Change indicator
   uint16_t            refc;        // Reference counter
#define RFD_MAXREFC    0xFFFF       // Largest possible reference count
}; // struct PGS::RFD

//----------------------------------------------------------------------------
//
// Struct-
//       PGS::VFD
//
// Purpose-
//       Virtual Frame Descriptor
//
// Usage notes-
//       The VFD array is saved externally in the control file.
//       When on disk, the real storage addresses have no meaning.
//
//----------------------------------------------------------------------------
struct PGS::VFD {                   // Virtual Frame Descriptor
   VFD*                next;        // Hash chain pointer
   RFD*                rfd;         // -> Real Frame Descriptor
   PGSVADDR_T          vaddr;       // Associated Virtual Address
   PGSXADDR_T          xaddr;       // Associated External Address
}; // struct PGS::VFD

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
char                   PGS_File::zeros[sizeof(PGS_File::zeros)];

#define MAX_FRAMELIST  12           // Number of valid framesizes
static struct FrameInfo frameList[MAX_FRAMELIST]= // List of valid framesizes
{    { 19, 0x00000000UL, 0x00080000UL, 0x0007FFFFUL}
,    { 18, 0x00000000UL, 0x00040000UL, 0x0003FFFFUL}
,    { 17, 0x00000000UL, 0x00020000UL, 0x0001FFFFUL}
,    { 16, 0x00000000UL, 0x00010000UL, 0x0000FFFFUL}
,    { 15, 0x00000000UL, 0x00008000UL, 0x00007FFFUL}
,    { 14, 0x00000000UL, 0x00004000UL, 0x00003FFFUL}
,    { 13, 0x00000000UL, 0x00002000UL, 0x00001FFFUL}
,    { 12, 0x00000000UL, 0x00001000UL, 0x00000FFFUL}
,    { 11, 0x00000000UL, 0x00000800UL, 0x000007FFUL}
,    { 10, 0x00000000UL, 0x00000400UL, 0x000003FFUL}
,    {  9, 0x00000000UL, 0x00000200UL, 0x000001FFUL}
,    {  8, 0x00000000UL, 0x00000100UL, 0x000000FFUL}
};

#define MAX_HSLIST     20           // Number of valid hashtable sizes
static uint32_t        hslist[MAX_HSLIST]= { // List of valid hashtable sizes
     0x00000101,                    // Minimum size (257)
     0x000001FD,
     0x000003FD,
     0x00000805,
     0x00001003,
     0x00001FFF,
     0x00003FFD,
     0x00008003,
     0x00010001,
     0x0001FFFF,
     0x0003FFFB,
     0x0007FFFF,
     0x00100007,                    // largest verified prime
     0x001FFFFF,
     0x003FFFFF,
     0x007FFFFF,
     0x00FFFFFF,
     0x01FFFFFF,
     0x03FFFFFF,
     0x07FFFFFF                     // Maximum size
}; // hslist

//----------------------------------------------------------------------------
//
// Subroutine-
//       controlRD
//
// Purpose-
//       Read the control file, in blocks
//
//----------------------------------------------------------------------------
static int32_t                      // New offset (if error, <0)
   controlRD(                       // Read from control file
     PGS_File*         file,        // -> File descriptor
     void*             caddr,       // Data address
     int32_t           xaddr,       // Data offset
     int32_t           rsize)       // Data length
{
   char*               raddr= (char*)caddr;
   char                iblock[CONTROL_FRAMESZ];
   int                 rc;

   while(rsize >= CONTROL_FRAMESZ)
   {
     rc= file->rd(raddr, CONTROL_FRAMESZ, xaddr);
     if( rc != 0 )
       return (-1);

     raddr += CONTROL_FRAMESZ;
     rsize -= CONTROL_FRAMESZ;
     xaddr += CONTROL_FRAMESZ;
   }

   if( rsize > 0 )
   {
     rc= file->rd(iblock, CONTROL_FRAMESZ, xaddr);
     if( rc != 0 )
       return (-1);

     memcpy(raddr, iblock, rsize);
     xaddr += CONTROL_FRAMESZ;
   }

   return xaddr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       controlWR
//
// Purpose-
//       Write the control file, in blocks
//
//----------------------------------------------------------------------------
static int32_t                      // New offset (if error, <0)
   controlWR(                       // Write to control file
     PGS_File*         file,        // -> File descriptor
     const void*       caddr,       // Data address
     int32_t           xaddr,       // Data offset
     int32_t           rsize)       // Data length
{
   const char*         raddr= (const char*)caddr;
   char                iblock[CONTROL_FRAMESZ];
   int                 rc;

   while(rsize >= CONTROL_FRAMESZ)
   {
     rc= file->wr(raddr, CONTROL_FRAMESZ, xaddr);
     if( rc != 0 )
       return (-1);

     raddr += CONTROL_FRAMESZ;
     rsize -= CONTROL_FRAMESZ;
     xaddr += CONTROL_FRAMESZ;
   }

   if( rsize > 0 )
   {
     memset(iblock, 0, CONTROL_FRAMESZ);
     memcpy(iblock, raddr, rsize);

     rc= file->wr(iblock, CONTROL_FRAMESZ, xaddr);
     if( rc != 0 )
       return (-1);

     xaddr += CONTROL_FRAMESZ;
   }

   return xaddr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       hashf
//
// Purpose-
//       Compute the hash index.
//
//----------------------------------------------------------------------------
static inline long                  // Hash index
   hashf(                           // Compute hash index
     PGSVADDR_T        vaddr,       // For this virtual address
     uint32_t          vframes)     // And this frame count
{
   long                result;      // Resultant
   long                word1, word2;

   word1= long(vaddr);
   word2= long(vaddr >> 32);

   result= word1 + word2;
   result &= 0x7fffffff;
   result %= vframes;
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       statistic
//
// Purpose-
//       Display a PGS statistic
//
//----------------------------------------------------------------------------
static inline void
   statistic(                       // Display a statistic
     Debug&            diag_trace,  // -> Output trace file
     int64_t           value,       // Field value
     const char*       name)        // Field name
{
   diag_trace.tracef("%10" PRId64 " %s\n", value, name);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       xaddrToFileId
//
// Purpose-
//       Convert an Xaddr to a FileID.
//
//----------------------------------------------------------------------------
static inline int                   // Resultant FileId
   xaddrToFileId(                   // Convert Xaddr to FileId
     PGS::Xaddr        xaddr,       // Source Xaddr
     uint64_t          framemask)   // Frame mask
{
   return int(xaddr & framemask);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       PGS::xaddrToOffset
//
// Purpose-
//       Convert an Xaddr to a file offset
//
//----------------------------------------------------------------------------
static inline uint64_t              // Resultant file offset
   xaddrToOffset(                   // Convert Xaddr to file offset
     PGS::Xaddr        xaddr,       // Source Xaddr
     uint64_t          framemask)   // Frame mask
{
   return xaddr & (~framemask);
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::debugf
//
// Purpose-
//       Debugging write.
//
//----------------------------------------------------------------------------
void
   PGS::debugf(                     // Debugging write
     const char*       fmt,         // Message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   diag_trace.vdebugf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::errorf
//
// Purpose-
//       Debugging error.
//
//----------------------------------------------------------------------------
void
   PGS::errorf(                     // Debugging error
     const char*       fmt,         // Message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   diag_trace.verrorf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::tracef
//
// Purpose-
//       Debugging trace.
//
//----------------------------------------------------------------------------
void
   PGS::tracef(                     // Debugging trace
     const char*       fmt,         // Message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   diag_trace.vtracef(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::abort
//
// Purpose-
//       Abnormal PGS termination.
//
//----------------------------------------------------------------------------
void
   PGS::abort(                      // Debugging trace
     const char*       filenm,      // Failing file name
     int               lineno,      // Failing line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   errorf("ABORT: %s: %d ", filenm, lineno);

   va_start(argptr, fmt);           // Initialize va_ functions
   diag_trace.verrorf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   term();
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::error
//
// Purpose-
//       Diagnostic error information.
//
//----------------------------------------------------------------------------
void
   PGS::error(                      // Debugging trace
     const char*       filenm,      // Failing file name
     int               lineno,      // Failing line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   errorf("ERROR: %s: %d ", filenm, lineno);

   va_start(argptr, fmt);           // Initialize va_ functions
   diag_trace.verrorf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::traceOp
//
// Purpose-
//       Trace an access operation.
//
//----------------------------------------------------------------------------
void
   PGS::traceOp(                    // Trace an access operation
     const char*       opCode,      // Operation code
     RFD*              ptrrfd,      // -> RFD
     PGSVADDR_T        vaddr)       // Virtual Address
{
   if( ptrrfd == NULL )
     tracef("**NULL**= %s(%.8lX.%.8lX) "
            "[--------.--------] --------\n",
            opCode, long(vaddr>>32), long(vaddr));
   else
   {
     char* raddr= (char*)ptrrfd->raddr + int32_t(vaddr & framemask);
     tracef("%.8lX= %s(%.8lX.%.8lX)"
            " [%.8lX.%.8lX]"
            " %.2x%.2x%.2x%.2x\n",
            long(raddr), opCode, long(vaddr>>32), long(vaddr),
            long(ptrrfd->vfd->xaddr>>32), long(ptrrfd->vfd->xaddr),
            *(raddr+0)&0x00ff, *(raddr+1)&0x00ff,
            *(raddr+2)&0x00ff, *(raddr+3)&0x00ff);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::check
//
// Purpose-
//       Coherency check
//
//----------------------------------------------------------------------------
int                                 // Return code
   PGS::check( void )               // Coherency check
{
   int                 errorCount= 0; // Number of errors encountered
   VFD*                ptrvfd;      // Working VFD*
   RFD*                ptrrfd;      // Working RFD*
   VFD*                vChain;      // Working VFD*
   long                H;           // Hash index
   long                count;
   long                i;

   // Validate the VFT hash table
   for(i=0; i<vframes; i++)
   {
     ptrvfd= vfdhash[i];

     count= 0;
     while(ptrvfd != NULL)
     {
       H= hashf(ptrvfd->vaddr, vframes);
       if( H != i )
       {
         errorCount++;
         error(__FILE__, __LINE__, "VFD on wrong hash list\n");
       }

       if( count > xframeu )
       {
         errorCount++;
         error(__FILE__, __LINE__, "Infinite hash list\n");
       }

       ptrvfd= ptrvfd->next;
       count++;
     }
   }

   // Validate the VFT external storage map
   for(i=0; i<xframeu; i++)
   {
     ptrvfd= &vfdall[i];

     H= hashf(ptrvfd->vaddr, vframes);
     vChain= vfdhash[H];
     count= 0;
     while(vChain != NULL && vChain != ptrvfd && count < xframeu )
     {
       vChain= vChain->next;
       count++;
     }

     if( vChain == NULL )
     {
       errorCount++;
       error(__FILE__, __LINE__, "VFD not in hash table\n");
     }

     if( ptrvfd->rfd != NULL )
     {
       if( ptrvfd->rfd->vfd != ptrvfd )
       {
         errorCount++;
         error(__FILE__, __LINE__, "vfd(%p)->rfd(%p)->vfd(%p)\n",
               ptrvfd, ptrvfd->rfd, ptrvfd->rfd->vfd);
       }
     }
   }

   // Validate the RFT internal storage map
   char* storage= (char*)this->storage;
   storage += 4095;
   storage= (char*)(intptr_t(storage) & intptr_t(-4095));
   for(i=0; i<rframes; i++)
   {
     ptrrfd= &rfdall[i];
     if( ptrrfd->raddr != storage )
     {
       errorCount++;
       error(__FILE__, __LINE__, "rfd(%p)->raddr(%p) != %p\n",
             ptrrfd, ptrrfd->raddr, storage);
     }
     storage += framesize;

     if( ptrrfd->refc == 0 )        // Unreferenced storage
     {
       if( ptrrfd->fsm != ptrrfd->RFD_ONLRU && ptrrfd->vfd != NULL )
       {
         errorCount++;
         error(__FILE__, __LINE__, "rfd(%p)->vfd(%p) refc(0) fsm(%d)\n",
               ptrrfd, ptrrfd->vfd, ptrrfd->fsm);
       }
     }
     else                           // Referenced storage
     {
       if( ptrrfd->vfd == NULL
           || ptrrfd->vfd->rfd != ptrrfd
           || ptrrfd->fsm != ptrrfd->RFD_ALLOC )
       {
         errorCount++;
         error(__FILE__, __LINE__, "rfd(%p)->vfd(%p)->vfd(%p) refc(%d) fsm(%d)\n",
               ptrrfd, ptrrfd->vfd, ptrrfd->refc, ptrrfd->fsm);
       }
     }
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
int                                 // Return code
   PGS::debug( void )               // Debugging display
{
   VFD*                ptrvfd;      // Working VFD*
   RFD*                ptrrfd;      // Working RFD*
   long                count;

   long                i;

// debugf("%4d HCDM PGS\n", __LINE__); debugSetIntensiveMode();
   tracef("\n\n");
   tracef("VHash Array\n"
          "-----------\n");
   for(i=0; i<vframes; i++)
   {
     ptrvfd= vfdhash[i];
     count= 0;
     tracef("[%5ld] ", i);
     while(ptrvfd != NULL && count <= xframeu )
     {
       ptrvfd= ptrvfd->next;
       count++;
     }
     tracef("Count(%3ld)\n", count);
   }

   tracef("\n\n");
   tracef("Vaddr Array\n"
          "-----------\n");
   for(i=0; i<xframeu; i++)
   {
     ptrvfd= &vfdall[i];
     tracef("[%5ld] %p ", i, ptrvfd);
     tracef("R(%p) ", ptrvfd->rfd);
     tracef("V(%.8lx.%.8lx) ",
            long(ptrvfd->vaddr>>32), long(ptrvfd->vaddr));
     tracef("X(%.8lx.%.8lx)\n",
            long(ptrvfd->xaddr>>32), long(ptrvfd->xaddr));
   }

   tracef("\n\n");
   tracef("Raddr Array\n"
          "-----------\n");
   for(i=0; i<rframes; i++)
   {
     ptrrfd= &rfdall[i];
     tracef("[%5ld] %p ", i, ptrrfd);
     tracef("V(%p) ", ptrrfd->vfd);
     tracef("R(%p) ", ptrrfd->raddr);
     if( ptrrfd->fsm == ptrrfd->RFD_AVAIL )
       tracef("AVAIL");
     else if( ptrrfd->fsm == ptrrfd->RFD_ALLOC )
       tracef("ALLOC");
     else if( ptrrfd->fsm == ptrrfd->RFD_ONLRU )
       tracef("ONLRU");
     else
       tracef("FSM(%d) !!ERROR!!", ptrrfd->fsm);

     tracef("\n");
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::statistics
//
// Purpose-
//       Write statistics onto trace file.
//
//----------------------------------------------------------------------------
void
   PGS::statistics( void )          // Statistics display
{
   unsigned            i;

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   tracef("\n");
   tracef("Statistics\n");

   tracef("\n");
   tracef("File information\n"
          "----------------\n");
   for(i=0; i<fileu; i++)
     tracef("%10llX [%.3d] %.3d %s\n", fdlist[i].allocFrameNo,
            i, fdlist[i].file.getHandle(), fdlist[i].name);

   tracef("\n");
   tracef("Global statistics\n"
          "-----------------\n");
   tracef("%10ld framesize\n", (long)framesize);
   tracef("%10ld rframes\n",   (long)rframes);
   tracef("%10ld vframeu\n",   (long)xframeu);
   tracef("%10ld vframes\n",   (long)vframes);

   tracef("\n");
   tracef("External op statistics\n"
          "----------------------\n");
   statistic(diag_trace, stat_opchg, "accessChg()");
   statistic(diag_trace, stat_opref, "accessRef()");
   statistic(diag_trace, stat_opsci, "accessSCI()");
   statistic(diag_trace, stat_oprel, "release()");

   tracef("\n");
   tracef("Internal op statistics\n"
          "----------------------\n");
   statistic(diag_trace, stat_alloc, "alloc");
   statistic(diag_trace, stat_allru, "allocLRU");
   statistic(diag_trace, stat_recrd, "reclaimRead");
   statistic(diag_trace, stat_recwr, "reclaimWrite");
   statistic(diag_trace, stat_reuse, "reclaimInUse()");

   tracef("\n");
   statistic(diag_trace, stat_opfrd, "frameRD()");
   statistic(diag_trace, stat_opfwr, "frameWR()");

   tracef("\n");
   tracef("Other statistics\n"
          "----------------\n");
   statistic(diag_trace, stat_hashmiss, "hashmiss");
   statistic(diag_trace, stat_reorders, "reorders");

   diag_trace.flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::~PGS
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PGS::~PGS( void )                // Destructor
{
   term();
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::PGS
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PGS::PGS( void )                 // Constructor
:  framesize(0), framemask(0), framelog2(0)
,  files(0), fileu(0), filen(0), fdlist(NULL)
,  rframes(0), rfdall(NULL), rfdfree(NULL), storage(NULL)
,  vframes(0), vfdhash(NULL)
,  xframes(0), xframeu(0), vfdall(NULL)
,  reclaim_h(NULL), reclaim_t(NULL)
,  stat_opchg(0), stat_opref(0), stat_oprel(0), stat_opsci(0)
,  stat_opfrd(0), stat_opfwr(0)
,  stat_alloc(0), stat_allru(0), stat_recrd(0), stat_recwr(0), stat_reuse(0)
,  stat_hashmiss(0), stat_reorders(0)
,  diag_trace(), diag_level(0)
,  initialized(FALSE)
,  sw_debug(FALSE)
,  sw_trace(FALSE)
,  sw_jig(0)
{
   memset(diag_flags, 0, sizeof(diag_flags));
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::frameRD
//
// Purpose-
//       Read frame from external storage.
//
//----------------------------------------------------------------------------
void
   PGS::frameRD(                    // Read frame
     RFD*              ptrrfd)      // -> Real Frame Descriptor
{
   IOD*                ptriod;      // -> File Descriptor
   VFD*                ptrvfd;      // -> Virtual Frame Descriptor

   int                 rc;          // Called routine return code

   //-------------------------------------------------------------------------
   // Validation
   //-------------------------------------------------------------------------
   ptrvfd= ptrrfd->vfd;             // Address the virtual descriptor
   long fileId= xaddrToFileId(ptrvfd->xaddr, framemask); // Extract its FileId

   #ifdef PGS_consistency_checking
     if( ptrvfd == NULL )
     {
       abort(__FILE__, __LINE__,
             "PGS::frameRD: rfd(%p)->vfd(NULL)\n", ptrrfd);
       return;
     }

     if( ptrvfd->rfd != ptrrfd )
     {
       abort(__FILE__, __LINE__,
             "PGS::frameRD: vfd(%p)->rfd(%p) != rfd(%p)\n",
             ptrvfd, ptrvfd->rfd, ptrrfd);
       return;
     }

     if( fileId >= fileu )
     {
       abort(__FILE__, __LINE__,
             "PGS::frameRD: invalid fileno(%d)\n", fileId);
       return;
     }
   #endif

   ptriod= &fdlist[fileId];

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   stat_opfrd++;                    // Increment the read counter

   //-------------------------------------------------------------------------
   // Read the frame
   //-------------------------------------------------------------------------
   rc= ptriod->file.rd(ptrrfd->raddr, framesize,
                       xaddrToOffset(ptrvfd->xaddr, framemask));
   if( rc != 0 )
   {
     errorf("PGS::frameRD [%.8lX.%.8lX] I/O error\n",
            long(ptrvfd->xaddr>>32), long(ptrvfd->xaddr));
     zero(ptrrfd->raddr, framesize);// Clear the frame
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::frameWR
//
// Purpose-
//       Write frame onto external storage.
//
//----------------------------------------------------------------------------
void
   PGS::frameWR(                    // Write frame
     RFD*              ptrrfd)      // -> Real Frame Descriptor
{
   IOD*                ptriod;      // -> File Descriptor
   VFD*                ptrvfd;      // -> Virtual Frame Descriptor

   int                 rc;          // Called routine return code

   //-------------------------------------------------------------------------
   // Validation
   //-------------------------------------------------------------------------
   ptrvfd= ptrrfd->vfd;             // Address the virtual descriptor
   long fileId= xaddrToFileId(ptrvfd->xaddr, framemask); // Extract its FileId

   #ifdef PGS_consistency_checking
     if( ptrvfd == NULL )
     {
       abort(__FILE__, __LINE__,
             "PGS::frameWR: rfd(%p)->vfd(NULL)\n", ptrrfd);
       return;
     }

     if( ptrvfd->rfd != ptrrfd )
     {
       abort(__FILE__, __LINE__,
             "PGS::frameWR: vfd(%p)->rfd(%p) != rfd(%p)\n",
             ptrvfd, ptrvfd->rfd, ptrrfd);
       return;
     }

     if( fileId >= fileu )
     {
       abort(__FILE__, __LINE__,
             "PGS::frameWR: invalid fileno(%d)\n", fileId);
       return;
     }
   #endif

   ptriod= &fdlist[fileId];

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   stat_opfwr++;                    // Increment the write counter

   //-------------------------------------------------------------------------
   // Write the frame
   //-------------------------------------------------------------------------
   rc= ptriod->file.wr(ptrrfd->raddr, framesize,
                       xaddrToOffset(ptrvfd->xaddr, framemask));
   if( rc != 0 )
     errorf("PGS::frameWR [%.8lX.%.8lX] I/O error\n",
            long(ptrvfd->xaddr>>32), long(ptrvfd->xaddr));

   ptrrfd->chgi= FALSE;             // The frame is no longer dirty
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::allocateVFD
//
// Purpose-
//       Allocate virtual storage.
//
//----------------------------------------------------------------------------
PGS::VFD*                           // -> VFD
   PGS::allocateVFD(                // Allocate virtual storage
     Vaddr             vaddr,       // Virtual Address
     int               fileno)      // File number (0 if any)
{
   VFD*                ptrvfd;      // Resultant
   Vaddr               frame;       // Virtual frame address
   long                frames;      // New frame count
   long                size;        // Working size
   long                H;           // Hash index

   unsigned            i;

   //-------------------------------------------------------------------------
   // Insure that the frame is not already mapped
   //-------------------------------------------------------------------------
   frame= vaddr & (~(PGSVADDR_T)framemask);
   H= hashf(frame, vframes);
   for(ptrvfd= vfdhash[H];
       ptrvfd != NULL;
       ptrvfd= ptrvfd->next)        // Scan the hash list for the frame
   {
     if( ptrvfd->vaddr == frame )
     {
       error(__FILE__, __LINE__, "PGS::allocateVFD, already allocated\n");
       return NULL;
     }
   }

   //-------------------------------------------------------------------------
   // If required, expand the vfdall array
   //-------------------------------------------------------------------------
   if( xframeu >= xframes )         // If a new vfdall array is required
   {
     frames= (xframes * 3) / 2;     // Make it half again as large
     if( frames < 64 )              // And at least 64
       frames= 64;

//   debugf("PGS::allocateVFD vfdall expand %ld=>%ld\n",
//          (long)xframes, (long)frames);

     size= frames * sizeof(VFD);
     ptrvfd= (VFD*)malloc(size);
     if( ptrvfd == NULL )
     {
       error(__FILE__, __LINE__, "PGS::allocateVFD, No storage\n");
       return NULL;
     }

     memcpy(ptrvfd, vfdall, xframes * sizeof(VFD));
     free(vfdall);
     vfdall= ptrvfd;
     xframes= frames;

     //-----------------------------------------------------------------------
     // Reconstruct the vfd to rfd associations (RFD->vfd has changed)
     //-----------------------------------------------------------------------
     for(i=0; i<xframeu; i++)
     {
       ptrvfd= &vfdall[i];
       if( ptrvfd->rfd != NULL )
         ptrvfd->rfd->vfd= ptrvfd;
     }

     //-----------------------------------------------------------------------
     // Unconditionally rebuild the vfdhash array
     //-----------------------------------------------------------------------
     buildHashArray();
   }

   //-------------------------------------------------------------------------
   // Allocate an external frame
   //-------------------------------------------------------------------------
   if( fileno < 0                   // If fileno too small
       ||unsigned(fileno) >= fileu ) // or fileno too large
   {
     errorf("allocateVFD invalid file(%d)\n", fileno);
     fileno= 0;
   }

   if( fileu == 1 )                 // If no user files
   {
     errorf("allocateVFD no files available\n");
     return NULL;
   }

   if( fileno == 0 )                // If round-robin allocation
   {
     filen++;
     if( filen >= fileu )
       filen= 1;

     fileno= filen;
   }

   //-------------------------------------------------------------------------
   // Allocate a frame
   //-------------------------------------------------------------------------
   IOD* ptriod= &fdlist[fileno];
   Xaddr xaddr= ptriod->allocFrameNo << framelog2;
   xaddr |= fileno;
   ptriod->allocFrameNo++;

   //-------------------------------------------------------------------------
   // Map the allocated frame
   //-------------------------------------------------------------------------
   ptrvfd= &vfdall[xframeu++];
   ptrvfd->rfd= NULL;
   ptrvfd->vaddr= frame;
   ptrvfd->xaddr= xaddr;

   H= hashf(frame, vframes);
   ptrvfd->next= vfdhash[H];
   vfdhash[H]= ptrvfd;

   return ptrvfd;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::allocate
//
// Purpose-
//       Allocate external storage.
//
//----------------------------------------------------------------------------
PGS::RC                             // Return code
   PGS::allocate(                   // Allocate external storage
     int               fileno,      // File number (0 if any)
     Vaddr             vaddr)       // Virtual Address
{
   VFD* ptrvfd= allocateVFD(vaddr, fileno);
   if( ptrvfd != NULL )
     return RC_NORMAL;

   return RC_InvalidParameter;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::accessLoad
//
// Purpose-
//       Access the RFD* associated with a virtual address
//
//----------------------------------------------------------------------------
PGS::RFD*                           // -> RFD
   PGS::accessLoad(                 // Virtual address to RFD*
     Vaddr             vaddr)       // Virtual Address
{
   RFD*                ptrrfd;      // -> Real Frame Descriptor
   VFD*                ptrvfd;      // -> Virtual Frame Descriptor
   long                H;           // Hash index

   PGSVADDR_T frame= vaddr & (~(PGSVADDR_T)framemask);
   H= hashf(frame, vframes);
   #ifdef PGS_hash_move_to_front
     VFD* prvvfd= NULL;
   #endif
   for(ptrvfd= vfdhash[H];
       ptrvfd != NULL;
       ptrvfd= ptrvfd->next)        // Scan the hash list for the frame
   {
     if( ptrvfd->vaddr == frame )
       break;
     stat_hashmiss++;               // Number of misses on the hash list

     #ifdef PGS_hash_move_to_front  // Move hash to front of list
       prvvfd= ptrvfd;
     #endif
   }

   if( ptrvfd == NULL )             // If the frame is not mapped
   {
     ptrvfd= allocateVFD(frame);
     if( ptrvfd == NULL )
       return NULL;

     #ifdef PGS_hash_move_to_front
       prvvfd= NULL;
     #endif
   }

   #ifdef PGS_hash_move_to_front    // Move hash to front of list
     if( prvvfd != NULL )           // Reorder the hashlist (for performance)
     {
       prvvfd->next= ptrvfd->next;
       ptrvfd->next= vfdhash[H];
       vfdhash[H]= ptrvfd;

       stat_reorders++;
     }
   #endif // PGS_hash_move_to_front

   ptrrfd= ptrvfd->rfd;             // Address the real frame
   if( ptrrfd != NULL )             // If already associated with storage
   {
     #ifdef PGS_consistency_checking
       if( ptrrfd->vfd != ptrvfd )  // Validate the association
       {
         abort(__FILE__, __LINE__,
               "PGS::accessLoad: rfd(%p)->vfd(%p) != vfd(%p)\n",
               ptrrfd, ptrrfd->vfd, ptrvfd);
         return NULL;
       }

       if( ptrrfd->fsm == ptrrfd->RFD_ONLRU )
       {
         if( ptrrfd->refc != 0 )
           errorf("PGS::accessLoad: rfd(%p)->refc(%d) != 0\n",
                  ptrrfd, ptrrfd->refc);
       }
     #endif // PGS_consistency_checking

     if( ptrrfd->fsm == ptrrfd->RFD_ONLRU ) // If on the LRU list
     {
       if( ptrrfd->next == NULL )
         reclaim_t= ptrrfd->prev;
       else
         ptrrfd->next->prev= ptrrfd->prev;

       if( ptrrfd->prev == NULL )
         reclaim_h= ptrrfd->next;
       else
         ptrrfd->prev->next= ptrrfd->next;

       ptrrfd->fsm= ptrrfd->RFD_ALLOC;

       if( ptrrfd->chgi )           // Reclaim statistics
         stat_recwr++;
       else
         stat_recrd++;
       }
     else
       stat_reuse++;

     return ptrrfd;                 // Function complete
   }

   //-------------------------------------------------------------------------
   // Allocate a new, virgin frame
   //-------------------------------------------------------------------------
   ptrrfd= rfdfree;                 // Allocate a frame
   if( ptrrfd != NULL )             // If storage available
   {
     rfdfree= ptrrfd->next;
     stat_alloc++;
     goto associate;
   }

   //-------------------------------------------------------------------------
   // Allocate a frame from the LRU list
   //-------------------------------------------------------------------------
   ptrrfd= reclaim_h;
   if(ptrrfd == NULL)
   {
     errorf("accessLoad: Too many frames referenced\n");
     return NULL;
   }

   #ifdef PGS_consistency_checking
     if( ptrrfd->refc != 0 )
       errorf("PGS::accessLoad ptrrfd(%p) vfd(%p) refc(%d)\n",
              ptrrfd, ptrrfd->vfd, ptrrfd->refc);
   #endif

   reclaim_h= ptrrfd->next;
   if( reclaim_h != NULL )
     reclaim_h->prev= NULL;
   else
     reclaim_t= NULL;

   if( ptrrfd->chgi )
     frameWR(ptrrfd);
   ptrrfd->vfd->rfd= NULL;

   stat_allru++;

   //-------------------------------------------------------------------------
   // Associate the new frame
   //-------------------------------------------------------------------------
associate:
   ptrrfd->fsm= ptrrfd->RFD_ALLOC;  // Indicate allocated
   ptrrfd->vfd= ptrvfd;             // Make the association
   ptrvfd->rfd= ptrrfd;
   ptrrfd->chgi= FALSE;
   ptrrfd->refc= 0;

   frameRD(ptrrfd);                 // Read the frame
   return ptrrfd;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::accessRead
//
// Purpose-
//       Access the RFD* associated with a virtual address
//
//----------------------------------------------------------------------------
PGS::RFD*                           // -> RFD
   PGS::accessRead(                 // Virtual address to RFD*
     Vaddr             vaddr)       // Virtual Address
{
   RFD*                ptrrfd;      // -> Real Frame Descriptor
   VFD*                ptrvfd;      // -> Virtual Frame Descriptor
   long                H;           // Hash index

   PGSVADDR_T frame= vaddr & (~(PGSVADDR_T)framemask);
   H= hashf(frame, vframes);
   for(ptrvfd= vfdhash[H];
       ptrvfd != NULL;
       ptrvfd= ptrvfd->next)        // Scan the hash list for the frame
   {
     if( ptrvfd->vaddr == frame )
       break;
     stat_hashmiss++;               // Number of misses on the hash list
   }

   if( ptrvfd == NULL )             // If the frame is not mapped
     return NULL;

   ptrrfd= ptrvfd->rfd;             // Address the real frame
   if( ptrrfd != NULL
       && ptrrfd->fsm == ptrrfd->RFD_ONLRU ) // Won't locate frame on LRU list
     ptrrfd= NULL;

   return ptrrfd;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::accessChg
//
// Purpose-
//       Access virtual address for update.
//
//----------------------------------------------------------------------------
PGSRADDR_T                          // Associated real address
   PGS::accessChg(                  // Access virtual address for update
     Vaddr             vaddr)       // Virtual Address
{
   char*               p= NULL;     // Resultant
   RFD*                ptrrfd;      // -> Real Frame Descriptor

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( !initialized )               // If not initialized
     return NULL;                   // Cannot continue

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   stat_opchg++;                    // Increment the operation count

   //-------------------------------------------------------------------------
   // Access for update
   //-------------------------------------------------------------------------
   ptrrfd= accessLoad(vaddr);
   if( ptrrfd != NULL )             // If we accessed the frame
   {
     if( ptrrfd->refc == RFD_MAXREFC ) // If not at limit
       errorf("PGS::accessChg: Too many references to frame\n");
     else
     {
       ptrrfd->refc++;              // Increment the reference counter
       ptrrfd->chgi= TRUE;          // Set the change indicator

       p= (char*)ptrrfd->raddr;     // Real address of frame
       p += int32_t(vaddr & framemask); // Add element offset
     }
   }

   //-------------------------------------------------------------------------
   // Trace
   //-------------------------------------------------------------------------
   if( diag_level > 5 || (diag_flags[TRACE_CHG_WORD]&TRACE_CHG_MASK) != 0 )
     traceOp("CHG", ptrrfd, vaddr);

   return(p);                       // Return, function complete
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::accessRef
//
// Purpose-
//       Access virtual address for reading.
//
//----------------------------------------------------------------------------
PGSRADDR_T                          // Associated real address
   PGS::accessRef(                  // Access virtual address for reading
     Vaddr             vaddr)       // Virtual Address
{
   char*               p= NULL;     // Resultant
   RFD*                ptrrfd;      // -> Real Frame Descriptor

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( !initialized )               // If not initialized
     return NULL;                   // Cannot continue

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   stat_opref++;                    // Increment the operation count

   //-------------------------------------------------------------------------
   // Access for reference
   //-------------------------------------------------------------------------
   ptrrfd= accessLoad(vaddr);
   if( ptrrfd != NULL )             // If we accessed the frame
   {
     if( ptrrfd->refc == RFD_MAXREFC ) // If not at limit
       errorf("PGS::accessRef: Too many references to frame\n");
     else
     {
       ptrrfd->refc++;              // Increment the reference counter

       p= (char*)ptrrfd->raddr;     // Real address of frame
       p += int32_t(vaddr & framemask); // Add element offset
     }
   }

   //-------------------------------------------------------------------------
   // Trace
   //-------------------------------------------------------------------------
   if( diag_level > 5 || (diag_flags[TRACE_REF_WORD]&TRACE_REF_MASK) != 0 )
     traceOp("REF", ptrrfd, vaddr);

   return(p);
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::accessSCI
//
// Purpose-
//       Set change indicator, promoting an accessRef to and accessChg
//       The reference count is not changed.
//
//----------------------------------------------------------------------------
PGSRADDR_T                          // Associated real address
   PGS::accessSCI(                  // Set change indicator
     Vaddr             vaddr)       // Virtual Address
{
   char*               p= NULL;     // Resultant
   RFD*                ptrrfd;      // -> Real Frame Descriptor

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( !initialized )               // If not initialized
     return NULL;                   // Cannot continue

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   stat_opsci++;                    // Increment the operation count

   //-------------------------------------------------------------------------
   // Set the change indicator
   //-------------------------------------------------------------------------
   ptrrfd= accessRead(vaddr);
   if( ptrrfd == NULL )             // If not resident
     errorf("PGS::accessSCI: Frame is not referenced\n");
   else
   {
     ptrrfd->chgi= TRUE;            // Set the change indicator
     p= (char*)ptrrfd->raddr;       // Real address of frame
     p += int32_t(vaddr & framemask); // Add element offset
   }

   //-------------------------------------------------------------------------
   // Trace
   //-------------------------------------------------------------------------
   if( diag_level > 5 || (diag_flags[TRACE_SCI_WORD]&TRACE_SCI_MASK) != 0 )
     traceOp("SCI", ptrrfd, vaddr);

   return(p);
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::release
//
// Purpose-
//       Release frame access.
//
//----------------------------------------------------------------------------
void
   PGS::release(                    // Release frame access
     Vaddr             vaddr)       // Virtual Address
{
   RFD*                ptrrfd;      // -> Real Frame Descriptor

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( !initialized )               // If not initialized
     return;                        // Cannot continue

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   stat_oprel++;                    // Increment the operation count

   //-------------------------------------------------------------------------
   // Release the frame
   //-------------------------------------------------------------------------
   ptrrfd= accessRead(vaddr);
   if( ptrrfd == NULL )             // If we cannot find the frame
     errorf("PGS::release: Frame is not referenced\n");
   else
   {
     #ifdef PGS_consistency_checking
       if( ptrrfd->refc == 0 )
       {
         error(__FILE__, __LINE__,
               "PGS::release: reference count(0)\n");
         return;
       }
     #endif

     ptrrfd->refc--;                // Decrement the reference counter
     if( ptrrfd->refc == 0 )        // If last reference
     {
       ptrrfd->next= NULL;

       if( reclaim_h == NULL )
       {
         ptrrfd->prev= NULL;
         reclaim_h= ptrrfd;
       }
       else
       {
         ptrrfd->prev= reclaim_t;
         reclaim_t->next= ptrrfd;
       }

       reclaim_t= ptrrfd;
       ptrrfd->fsm= ptrrfd->RFD_ONLRU;
     }
   }

   //-------------------------------------------------------------------------
   // Trace
   //-------------------------------------------------------------------------
   if( diag_level > 5 || (diag_flags[TRACE_REL_WORD]&TRACE_REL_MASK) != 0 )
     traceOp("REL", ptrrfd, vaddr);
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::status
//
// Purpose-
//       Determine frame state.
//
//----------------------------------------------------------------------------
PGS::RC                             // Return code
   PGS::status(                     // Determine frame status
     Vaddr             vaddr)       // Virtual Address
{
   VFD*                ptrvfd;      // -> Virtual Frame Descriptor

   PGSVADDR_T          frame;       // Virtual frame address
   long                H;           // Hash index

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( !initialized )
     return RC_NotInitialized;

   //-------------------------------------------------------------------------
   // Determine the page state
   //-------------------------------------------------------------------------
   frame= vaddr & (~(PGSVADDR_T)framemask);
   H= hashf(frame, vframes);
   for(ptrvfd= vfdhash[H];
       ptrvfd != NULL;
       ptrvfd= ptrvfd->next)        // Scan the hash list for the frame
   {
     if( ptrvfd->vaddr == frame )
       break;
   }

   if( ptrvfd == NULL )             // If the frame is not mapped
     return RC_Vaddr_Invalid;

   if( ptrvfd->rfd == NULL )        // If the frame is not resident
     return RC_Vaddr_On_Disk;

   return RC_NORMAL;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::buildHashArray
//
// Purpose-
//       Reconstruct the vfdhash array.
//
//----------------------------------------------------------------------------
void
   PGS::buildHashArray( void )      // Build the VFD hash array
{
   VFD**               ptrhash;     // -> New virtual frame hash array
   VFD*                ptrvfd;      // -> Virtual Frame Descriptor
   long                count;       // Temporary work variable
   long                size;        // Temporary work variable
   long                H;           // Hash index

   long                i;

   //-------------------------------------------------------------------------
   // Determine whether the current vfdhash array is large enough
   //-------------------------------------------------------------------------
   count= (xframes >> 3) + 31; // Default count
   if( count >= hslist[MAX_HSLIST-1] )
     count= hslist[MAX_HSLIST-1];
   else
   {
     for(i=0; i<MAX_HSLIST; i++)
     {
       if( count <= hslist[i] )
       {
         count= hslist[i];
         break;
       }
     }
   }

   //-------------------------------------------------------------------------
   // If required, allocate a new vfdhash array
   //-------------------------------------------------------------------------
   size= count * sizeof(VFD*);
   if( vfdhash == NULL )
   {
     ptrhash= (VFD**)malloc(size);
     if( ptrhash == NULL )
       return;
     vfdhash= ptrhash;
     vframes= count;
   }
   else if( count > vframes )
   {
     ptrhash= (VFD**)malloc(size);
     if( ptrhash == NULL )
     {
       errorf("buildHashArray: Unable to expand\n");
       return;
     }
     else
     {
       free(vfdhash);
       vfdhash= ptrhash;
       vframes= count;
     }
   }

   //-------------------------------------------------------------------------
   // (Re)construct the vfdhash array
   //-------------------------------------------------------------------------
   size= vframes * sizeof(VFD*);    // Sizeof(array)
   memset(vfdhash, 0, size);        // Clear the array

   for(i=0; i<xframeu; i++)         // (Re)construct the array
   {
     ptrvfd= &vfdall[i];

     H= hashf(ptrvfd->vaddr, vframes);
     ptrvfd->next= vfdhash[H];
     vfdhash[H]= ptrvfd;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::term
//
// Purpose-
//       Terminate Virtual Paging Subsystem.
//
//----------------------------------------------------------------------------
void
   PGS::term( void )                // Terminate PGS
{
   CFH                 cfh;         // The control file header

   RFD*                ptrrfd;      // Pointer to real frame descriptor

   int32_t             vaddr;       // Disk offset
   unsigned            i;           // General index variable

   //-------------------------------------------------------------------------
   // No cleanup unless initialized
   //-------------------------------------------------------------------------
   if( initialized )
   {
     //-----------------------------------------------------------------------
     // Level 0 diagnostics
     //-----------------------------------------------------------------------
     #ifdef PGS_consistency_checking
       for(i=0; i<rframes; i++)
       {
         ptrrfd= &rfdall[i];
         if( ptrrfd->refc != 0 )
         {
           error(__FILE__, __LINE__, "Dangling references exist\n");
           break;
         }
       }

       check();
     #endif

     //-----------------------------------------------------------------------
     // Level 1 diagnostics
     //-----------------------------------------------------------------------
     if( diag_level >= PGSDIAGL_HCDM ) // If hard-core debug mode
       debug();

     if( diag_level >= PGSDIAGL_STAT ) // If statistical display
       statistics();                // Write statistical information

     //-----------------------------------------------------------------------
     // Write all dirty pages
     //-----------------------------------------------------------------------
     for(i=0; i<rframes; i++)       // Write dirty frames
     {
       ptrrfd= &rfdall[i];
       if( ptrrfd->vfd != NULL      // If there is an associated virtual frame
           && ptrrfd->chgi )        // And the page is changed
         frameWR(ptrrfd);           // Write the page
     }

     //-----------------------------------------------------------------------
     // Write the control file information
     //-----------------------------------------------------------------------
     memset(&cfh, 0, sizeof(CFH));
     memcpy(cfh.cbid, CFH_CBID, sizeof(cfh.cbid));
     memcpy(cfh.vbid, CFH_VBID, sizeof(cfh.vbid));
     memcpy(cfh.rbid, CFH_RBID, sizeof(cfh.rbid));
     cfh.enid=    CFH_ENID;
     cfh.files=   fileu;
     cfh.framesz= framesize;
     cfh.frameno= xframeu;

     PGS_File* file= &fdlist[0].file;
     vaddr= controlWR(file, &cfh, 0, sizeof(CFH));

     if( vaddr > 0 )
       vaddr= controlWR(file,
                        fdlist,
                        vaddr,
                        cfh.files * sizeof(IOD));

     if( vaddr > 0 )
       vaddr= controlWR(file,
                        vfdall,
                        vaddr,
                        cfh.frameno * sizeof(VFD));

     if( vaddr < 0 )
       errorf("PGS::term: Control file '%s' error\n", fdlist[0].name);
   }

   //-------------------------------------------------------------------------
   // Release the file arrays
   //-------------------------------------------------------------------------
   if( fdlist != NULL )             // If the file handle list is present
   {
     for(i=0; i<fileu; i++)         // Close all the files
       fdlist[i].file.close();
     free(fdlist);                  // Delete it
   }
   fdlist= NULL;

   //-------------------------------------------------------------------------
   // Release the virtual frame hash table array
   //-------------------------------------------------------------------------
   if( vfdhash != NULL )            // If the hash table exists
     free(vfdhash);                 // Delete it
   vfdhash= NULL;

   //-------------------------------------------------------------------------
   // Release the virtual frame array
   //-------------------------------------------------------------------------
   if( vfdall != NULL )             // If the virtual frame array exists
     free(vfdall);                  // Delete it
   vfdall= NULL;

   //-------------------------------------------------------------------------
   // Release the real frame array
   //-------------------------------------------------------------------------
   if( storage != NULL )            // If real store exists
     free(storage);                 // Delete it
   storage= NULL;

   if( rfdall != NULL )             // If the real frame array exists
     free(rfdall);                  // Delete it
   rfdall= NULL;

   //-------------------------------------------------------------------------
   // Terminate tracing
   //-------------------------------------------------------------------------
   diag_trace.flush();

   initialized= FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::init
//
// Purpose-
//       Initialize Virtual Paging Subsystem
//
//----------------------------------------------------------------------------
int                                 // Return code
   PGS::init(                       // Initialize PGS
     int               fileno,      // Number of files
     uint32_t          framesz,     // Page Space frame size
                                    // (Power of 2, range 256..524288)
     uint32_t          realframeno, // Page Space real frame count
                                    // (Minimum value= 64)
     uint32_t          virtframeno) // Virtual frame count
                                    // (Minimum value= realframeno
{
   RFD*                ptrrfd;      // Pointer to real frame descriptor

   long                size;        // Temporary work variable
   unsigned            i;           // General index variables

   //-------------------------------------------------------------------------
   // Validate the parameters
   //-------------------------------------------------------------------------
   if( initialized )                // If already initialized
     return PGSINIT_DUPINIT;

   for(i=0; i<MAX_FRAMELIST; i++)   // Determine the frame entry
   {
     if( framesz == frameList[i].infoSize ) // If the framesize is valid
       goto framesize_OK;
   }
   return PGSINIT_FRAMESZ;          // Exit, invalid framesz

   //-------------------------------------------------------------------------
   // Initialize the common area
   //-------------------------------------------------------------------------
framesize_OK:
   framesize= frameList[i].infoSize;
   framemask= frameList[i].infoMask;
   framelog2= frameList[i].infoLog2;

   files= fileu= filen= 0; fdlist= NULL;
   rframes= 0; rfdall= NULL; rfdfree= NULL; storage= NULL;
   vframes= 0; vfdhash= NULL;
   xframes= xframeu= 0; vfdall= NULL;
   reclaim_h= reclaim_t= NULL;
   stat_opchg= stat_opref= stat_oprel= stat_opsci= 0;
   stat_opfrd= stat_opfwr= 0;
   stat_alloc= stat_allru= stat_recrd= stat_recwr= stat_reuse= 0;
   stat_hashmiss= stat_reorders= 0;
   diag_trace.setName("PGS.OUT"); diag_level= 0;
   memset(diag_flags, 0, sizeof(diag_flags));
   sw_debug= sw_trace= sw_jig= 0;

   //-------------------------------------------------------------------------
   // Allocate and initialize the real frame array
   //-------------------------------------------------------------------------
   if( realframeno < 64 )
     realframeno= 64;
   size= realframeno * sizeof(RFD); // sizeof(rfdall array)
   rfdall= (RFD*)malloc(size);      // Allocate the array
   if( rfdall == NULL )             // If storage not available
   {
     term();
     return PGSINIT_MEMORY;
   }
   memset(rfdall, 0, size);         // Clear the real frame array
   rframes= realframeno;            // Set the real frame count

   storage= malloc(realframeno * framesz + 4096);
   if( storage == NULL )
   {
     term();
     return PGSINIT_MEMORY;
   }

   char* p= (char*)storage + 4095;
   p= (char*)(intptr_t(p) & intptr_t(-4095));
   for(i=0; i<realframeno; i++)     // Initialize the array
   {
     ptrrfd= &rfdall[i];
     ptrrfd->raddr= p;
     p += framesz;
   }

   for(i=1; i<realframeno; i++)     // Create the free list
     rfdall[i-1].next= &rfdall[i];

   rfdfree= &rfdall[0];

   //-------------------------------------------------------------------------
   // Allocate and initialize the virtual frame array
   //-------------------------------------------------------------------------
   if( virtframeno < realframeno )
     virtframeno= realframeno;
   size= virtframeno * sizeof(VFD); // sizeof(vfdall array)
   vfdall= (VFD*)malloc(size);      // Allocate the array
   if( vfdall == NULL )             // If storage not available
   {
     term();
     return PGSINIT_MEMORY;
   }
   memset(vfdall, 0, size);         // Clear the virtual frame array

   xframes= virtframeno;
   xframeu= 0;

   //-------------------------------------------------------------------------
   // Allocate and initialize the virtual frame hash table array
   //-------------------------------------------------------------------------
   buildHashArray();
   if( vfdhash == NULL )            // If storage not available
   {
     term();
     return PGSINIT_MEMORY;
   }

   //-------------------------------------------------------------------------
   // Allocate and initialize the file arrays
   //-------------------------------------------------------------------------
   size= fileno * sizeof(IOD);      // sizeof(fdlist array)
   fdlist= (IOD*)malloc(size);      // Allocate the array
   if( fdlist == NULL )             // If storage not available
   {
     term();
     return PGSINIT_MEMORY;
   }
   memset((char*)fdlist, 0, size);  // Clear the array

   files= fileno;
   fileu= 0;

   //-------------------------------------------------------------------------
   // Initialize diagnostics
   //-------------------------------------------------------------------------
   ParseINI            parseINI;    // File control parameters
   const char*         parm;        // A generic parameter

   //-------------------------------------------------------------------------
   // Initialize the control file
   //-------------------------------------------------------------------------
   parseINI.construct();            // Build the object
   parseINI.open(DIAGFILE_NAME);    // Open the trace file

   //-------------------------------------------------------------------------
   // Activate the trace file
   //-------------------------------------------------------------------------
   parm= parseINI.getValue("Debug", "filename"); // Get debug filename
   if( parm != NULL )
   {
     diag_trace.setName(parm);
     tracef("<%s> %s : %s\n", DIAGFILE_NAME, "filename   ", parm);
   }

   //-------------------------------------------------------------------------
   // Set diagnostic level
   //-------------------------------------------------------------------------
   diag_level= 0;
   parm= parseINI.getValue("Debug", "traceLevel"); // Get the debug control
   if( parm != NULL )
   {
     diag_level= atoi(parm);
     tracef("<%s> %s : %s\n", DIAGFILE_NAME, "traceLevel ", parm);

     if( diag_level > 10 )
     {
       diag_level -= 10;
       diag_trace.setMode(Debug::ModeIntensive);
     }
   }

   // TODO: Set individual trace flags

   //-------------------------------------------------------------------------
   // Exit, function complete
   //-------------------------------------------------------------------------
   initialized= TRUE;
   return(PGSINIT_COMPLETE);
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::cold
//
// Purpose-
//       Virtual Paging Subsystem COLD start
//
//----------------------------------------------------------------------------
int                                 // Return code (see init)
   PGS::cold(                       // Initialize PGS, cold start
     const char*       control,     // The control file name
     uint32_t          framesz,     // Page Space frame size
     uint32_t          frameno)     // Page Space real frame count
{
   uint32_t            realframeno; // Physical frame number
   uint32_t            virtframeno; // Virtual frame number
   int                 rc;          // Called routine return code

   //-------------------------------------------------------------------------
   // Initialize PGS
   //-------------------------------------------------------------------------
   realframeno= frameno;
   virtframeno= realframeno << 1;   // Default, vframe= rframe * 2
   rc= init(16, framesz, realframeno, virtframeno);// Initialize PGS
   if( rc != PGSINIT_COMPLETE )     // If initialization complete
     return(rc);                    // Return, cannot initialize

   //-------------------------------------------------------------------------
   // Open the control file
   //-------------------------------------------------------------------------
   if( insFile(control) != 0 )       // Open the control file
   {
     term();
     return PGSINIT_CONTROL;
   }

   initialized= TRUE;                // Indicate initialized
   return(PGSINIT_COMPLETE);
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::warm
//
// Purpose-
//       Virtual Paging Subsystem WARM start
//
//----------------------------------------------------------------------------
int                                 // Return code (see init)
   PGS::warm(                       // Initialize PGS, warm start
     const char*       control,     // The control file name
     uint32_t          framesz,     // Page Space frame size
     uint32_t          frameno)     // Page Space real frame count
{
   CFH                 cfh;         // The control file header
   PGS_File            file;        // Temporary for control file

   VFD*                ptrvfd;      // -> Virtual Frame Descripttor

   int32_t             vaddr;       // File position

   int                 rc;
   unsigned            i;

   //-------------------------------------------------------------------------
   // Read the control file header
   //-------------------------------------------------------------------------
   if( file.openWarm(control) != 0 )
   {
     errorf("PGS::warm: Cannot open '%s'\n", control);
     return(PGSINIT_CONTROL);
   }

   vaddr= controlRD(&file, &cfh, 0, sizeof(CFH)); // Read the header
   if( vaddr < 0 )
   {
     errorf("PGS::warm: Cannot read '%s' HDR\n", control);
     return(PGSINIT_CONTROL);
   }

   //-------------------------------------------------------------------------
   // Verify the file header
   //-------------------------------------------------------------------------
   if( memcmp(cfh.cbid, CFH_CBID, sizeof(cfh.cbid)) )
   {
     file.close();
     return(PGSINIT_CONTROL);
   }

   if( memcmp(cfh.vbid, CFH_VBID, sizeof(cfh.vbid))
       || memcmp(cfh.rbid, CFH_RBID, sizeof(cfh.rbid))
       || cfh.enid != CFH_ENID )
   {
     file.close();
     return(PGSINIT_CONTROL);
   }

   if( cfh.files == 0 || cfh.framesz == 0 )
   {
     file.close();
     return(PGSINIT_CONTROL);
   }

   //-------------------------------------------------------------------------
   // Initialize PGS characteristics
   //-------------------------------------------------------------------------
   if( framesz != 0 && framesz != cfh.framesz )
   {
     file.close();
     return PGSINIT_FRAMESZ;
   }

   //-------------------------------------------------------------------------
   // Initialize PGS
   //-------------------------------------------------------------------------
   rc= init(cfh.files, cfh.framesz, frameno, cfh.frameno);
   if( rc != PGSINIT_COMPLETE )     // If initialization failure
   {
     file.close();
     return(rc);                    // Return, cannot initialize
   }

   //-------------------------------------------------------------------------
   // Restore the control file information
   //-------------------------------------------------------------------------
   vaddr= controlRD(&file,
                    fdlist,
                    vaddr,
                    cfh.files * sizeof(IOD) );

   if( vaddr > 0 && cfh.frameno > 0 )
     vaddr= controlRD(&file,
                      vfdall,
                      vaddr,
                      cfh.frameno * sizeof(VFD) );

   if( vaddr < 0 )
   {
     errorf("PGS::warm: Cannot read '%s' DATA\n", control);
     file.close();
     term();
     return(PGSINIT_CONTROL);
   }

   //-------------------------------------------------------------------------
   // The control file is already open - now term() can close it
   //-------------------------------------------------------------------------
   fdlist[0].file= file;            // REQUIRES ~PGS_File NOT CLOSE
   fileu= 1;

   //-------------------------------------------------------------------------
   // Open the data files
   //-------------------------------------------------------------------------
   for(i=1; i<cfh.files; i++)       // Open the data files
   {
     if( fdlist[i].file.openWarm(fdlist[i].name) != 0 )
     {
       errorf("PGS::warm: Cannot open '%s'\n", fdlist[i].name);
       term();
       return(PGSINIT_DATAFILE);
     }
     fileu= i+1;                    // term() can close this file
   }

   //-------------------------------------------------------------------------
   // Initialize the virtual frame array
   //-------------------------------------------------------------------------
   for(i=0; i<xframes; i++)         // Indicate frame not resident
   {
     ptrvfd= &vfdall[i];
     ptrvfd->rfd= NULL;
   }
   xframeu= cfh.frameno;
   buildHashArray();                // Initialize the vfdhash array

   //-------------------------------------------------------------------------
   // Check data validity
   //-------------------------------------------------------------------------
   if( check() != 0 )
   {
     term();
     return(PGSINIT_CONTROL);
   }

   //-------------------------------------------------------------------------
   // Warmstart complete
   //-------------------------------------------------------------------------
   initialized= TRUE;               // Indicate initialized
// statistics();
   return(PGSINIT_COMPLETE);
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::getFrameCount
//
// Purpose-
//       Return the number of available frames.
//
//----------------------------------------------------------------------------
long                                // The frame count
   PGS::getFrameCount( void )       // Retrieve the frame count
{
   return rframes;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::getFrameSize
//
// Purpose-
//       Return the frame size.
//
//----------------------------------------------------------------------------
long                                // The frame size
   PGS::getFrameSize( void )        // Retrieve the frame size
{
   return framesize;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS::insFile
//
// Purpose-
//       Insert a new file.
//
//----------------------------------------------------------------------------
int                                 // The inserted file number (if >0)
   PGS::insFile(                    // Insert a new File
     const char*       filenm)      // The file's name
{
   IOD*                ptriod;      // -> New I/O descriptor array
   int                 fileno;      // The allocated file index
   int                 files;       // The new file count
   long                size;        // Working size
   int                 rc;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( !initialized )               // If not initialized
     return 0;                      // Cannot continue

   //-------------------------------------------------------------------------
   // If required, expand the fdlist array
   //-------------------------------------------------------------------------
   if( fileu >= this->files )       // If a new fdlist array is required
   {
     files= (this->files * 3) / 2;  // Make it half again as large
     if( files < 16 )               // But at least 16 entries
       files= 16;

     size= files * sizeof(IOD);
     ptriod= (IOD*)malloc(size);
     if( ptriod == NULL )
     {
       errorf("insFile: Storage shortage(%ld)\n", size);
       return (-1);
     }

     memset((char*)ptriod, 0, size);
     memcpy((char*)ptriod, fdlist, fileu*sizeof(IOD));
     free(fdlist);
     fdlist= ptriod,
     this->files= files;
   }

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   ptriod= &fdlist[fileu];          // Address the new entry
   if( strlen(filenm) >= PGS_FNSIZE )
   {
     errorf("PGS::insFile(%s) Name too long(%d)\n", filenm, PGS_FNSIZE);
     return (-1);
   }

   rc= ptriod->file.openCold(filenm); // Open the file
   if( rc != 0 )                    // If open failure
     return (-1);                   // Return error indicator

   memset(ptriod->name, 0, PGS_FNSIZE);
   strcpy(ptriod->name, filenm);
   ptriod->allocFrameNo= 0;

   fileno= fileu++;
   return fileno;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS_File::~PGS_File
//
// Purpose-
//       Desctructor.
//
//----------------------------------------------------------------------------
   PGS_File::~PGS_File( void )      // Destructor
{
// We must not close the file. The control file is copied and closed later.
// close();                         // COMMENTED OUT. DO NOT CHANGE.
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS_File::PGS_File
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PGS_File::PGS_File( void )       // Constructor
:  handle(-1)
,  maxXaddr(0)
{
   memset(zeros, 0, sizeof(zeros));
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS_File::close
//
// Purpose-
//       Close a file.
//
//----------------------------------------------------------------------------
void
   PGS_File::close( void )          // Close a file
{
   if( handle >= 0 )
     ::close(handle);

   handle= (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS_File::openCold
//
// Purpose-
//       Open file, cold start.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   PGS_File::openCold(              // Initialize PGS_File, cold start
     const char*       const filename) // The file name
{
   handle=
       open64(filename,             // Open the file
            O_RDWR|O_CREAT|O_TRUNC|O_BINARY,// (in read-write binary mode)
            S_IREAD|S_IWRITE);      // Permission to read and write
   if( handle < 0 )                 // If open failure
     return handle;                 // Return error indicator

   maxXaddr= 0;

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS_File::openWarm
//
// Purpose-
//       Open file, warm start.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   PGS_File::openWarm(              // Initialize PGS_File, cold start
     const char*       const filename) // The file name
{
   FileInfo            fileInfo(filename); // File information

   handle=
       open64(filename,             // Open the file
            O_RDWR|O_BINARY,        // (in read-write binary mode)
            S_IREAD|S_IWRITE);      // Permission to read and write
   if( handle < 0 )                 // If open failure
     return handle;                 // Return error indicator

   maxXaddr= fileInfo.getFileSize();

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS_File::rd
//
// Purpose-
//       Read frame from external storage.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   PGS_File::rd(                    // Read frame
     void*             daddr,       // Data address
     int32_t           dsize,       // Data size
     PGSXADDR_T        xaddr)       // Data offset
{
   long                l;           // Read length
   int64_t             p;           // Seek position

   //-------------------------------------------------------------------------
   // If page never written, zero the frame
   //-------------------------------------------------------------------------
   if( xaddr >= maxXaddr )          // If the frame is not backed
   {
     memset(daddr, 0, dsize);       // Clear the frame
     return 0;
   }

   //-------------------------------------------------------------------------
   // Read the page
   //-------------------------------------------------------------------------
   p= lseek64(handle, xaddr, SEEK_SET); // Position the file
   if( p < 0 )                      // If seek failure
     return (-1);

   l= read(handle, daddr, dsize);   // Read a frame
   if( l != dsize )                 // If read failure
     return (-1);

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       PGS_File::wr
//
// Purpose-
//       Write frame onto external storage.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   PGS_File::wr(                    // Write frame
     const void*       daddr,       // Data address
     int32_t           dsize,       // Data size
     PGSXADDR_T        xaddr)       // Data offset
{
   long                xincr;       // External address increment

   long                l;           // Write length
   int64_t             p;           // Seek position

   //-------------------------------------------------------------------------
   // Write gap
   //-------------------------------------------------------------------------
   if( xaddr > maxXaddr )           // If a gap exists
   {
     xincr= sizeof(zeros);
     if( xincr > dsize )
       xincr= dsize;

     p= lseek64(handle, maxXaddr, SEEK_SET); // Position the file
     if( p < 0 )
       return (-1);

     while(maxXaddr < xaddr)
     {
       l= write(handle, zeros, xincr); // Write zeros
       if( l != xincr )             // If write failure
         return (-1);

       maxXaddr += xincr;
     }
   }

   //-------------------------------------------------------------------------
   // Write the frame
   //-------------------------------------------------------------------------
   p= lseek64(handle, xaddr, SEEK_SET); // Position the file
   if( p < 0 )                      // If seek failure
     return (-1);

   l= write(handle, daddr, dsize);
   if( l != dsize )                 // If write failure
     return (-1);

   if( (xaddr+dsize) > maxXaddr )   // If the file was extended
     maxXaddr= xaddr + dsize;       // Set updated size

   return 0;
}

