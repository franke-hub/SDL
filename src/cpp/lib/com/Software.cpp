//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Software.cpp
//
// Purpose-
//       System software interface implementation.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#if defined(_OS_BSD)
#include <pthread.h>
#endif

#include <errno.h>
#include <unistd.h>

#if defined(_OS_WIN)
#include <direct.h>
#include <process.h>
#include <winsock.h>
#endif

#if defined(_OS_WIN)
#include <windows.h>
#endif

#include <com/Debug.h>

#include "com/Software.h"

//----------------------------------------------------------------------------
//
// Table-
//       convertEC
//
// Purpose-
//       OS error code to constant error code conversion table.
//
//----------------------------------------------------------------------------
struct ConvertEC
{
   int                 sc;          // The system error code
   Software::SystemEC  ec;          // The associated SystemEC
};

static const ConvertEC  convertEC[]=// System error code conversion table
{  {0,                   Software::SystemEC(0)}
,  {EPERM,               Software::EC_PERM}
,  {ENOENT,              Software::EC_NOENT}
,  {ESRCH,               Software::EC_SRCH}
,  {EINTR,               Software::EC_INTR}
,  {EIO,                 Software::EC_IO}
,  {ENXIO,               Software::EC_NXIO}
,  {E2BIG,               Software::EC_2BIG}
,  {ENOEXEC,             Software::EC_NOEXEC}
,  {EBADF,               Software::EC_BADF}
,  {ECHILD,              Software::EC_CHILD}
,  {EAGAIN,              Software::EC_AGAIN}
,  {ENOMEM,              Software::EC_NOMEM}
,  {EACCES,              Software::EC_ACCES}
,  {EFAULT,              Software::EC_FAULT}
#ifdef ENOTBLK
,  {ENOTBLK,             Software::EC_NOTBLK}
#endif
,  {EBUSY,               Software::EC_BUSY}
,  {EEXIST,              Software::EC_EXIST}
,  {EXDEV,               Software::EC_XDEV}
,  {ENODEV,              Software::EC_NODEV}
,  {ENOTDIR,             Software::EC_NOTDIR}
,  {EISDIR,              Software::EC_ISDIR}
,  {EINVAL,              Software::EC_INVAL}
,  {ENFILE,              Software::EC_NFILE}
,  {EMFILE,              Software::EC_MFILE}
,  {ENOTTY,              Software::EC_NOTTY}
#ifdef ETXTBSY
,  {ETXTBSY,             Software::EC_TXTBSY}
#endif
,  {EFBIG,               Software::EC_FBIG}
,  {ENOSPC,              Software::EC_NOSPC}
,  {ESPIPE,              Software::EC_SPIPE}
,  {EROFS,               Software::EC_ROFS}
,  {EMLINK,              Software::EC_MLINK}
,  {EPIPE,               Software::EC_PIPE}
,  {EDOM,                Software::EC_DOM}
,  {ERANGE,              Software::EC_RANGE}
#ifdef ENOMSG
,  {ENOMSG,              Software::EC_NOMSG}
#endif
#ifdef EIDRM
,  {EIDRM,               Software::EC_IDRM}
#endif
#ifdef ECHRNG
,  {ECHRNG,              Software::EC_CHRNG}
#endif
#ifdef EL2NSYNC
,  {EL2NSYNC,            Software::EC_L2NSYNC}
#endif
#ifdef EL3HLT
,  {EL3HLT,              Software::EC_L3HLT}
#endif
#ifdef EL3RST
,  {EL3RST,              Software::EC_L3RST}
#endif
#ifdef ELNRNG
,  {ELNRNG,              Software::EC_LNRNG}
#endif
#ifdef EUNATCH
,  {EUNATCH,             Software::EC_UNATCH}
#endif
#ifdef ENOCSI
,  {ENOCSI,              Software::EC_NOCSI}
#endif
#ifdef EL2HLT
,  {EL2HLT,              Software::EC_L2HLT}
#endif
#ifdef EDEADLK
,  {EDEADLK,             Software::EC_DEADLK}
#endif
#ifdef ENOLCK
,  {ENOLCK,              Software::EC_NOLCK}
#endif
#ifdef ENOSYS
,  {ENOSYS,              Software::EC_NOSYS}
#endif
#ifdef ENOTEMPTY
,  {ENOTEMPTY,           Software::EC_NOTEMPTY}
#endif
#ifdef ENAMETOOLONG
,  {ENAMETOOLONG,        Software::EC_NAMETOOLONG}
#endif
#ifdef ELOOP
,  {ELOOP,               Software::EC_LOOP}
#endif
#ifdef ENOTREADY
,  {ENOTREADY,           Software::EC_NOTREADY}
#endif
#ifdef EWRPROTECT
,  {EWRPROTECT,          Software::EC_WRPROTECT}
#endif
#ifdef EFORMAT
,  {EFORMAT,             Software::EC_FORMAT}
#endif
#ifdef ENOCONNECT
,  {ENOCONNECT,          Software::EC_NOCONNECT}
#endif
#ifdef ESTALE
,  {ESTALE,              Software::EC_STALE}
#endif
#ifdef EINPROGRESS
,  {EINPROGRESS,         Software::EC_INPROGRESS}
#endif
#ifdef EALREADY
,  {EALREADY,            Software::EC_ALREADY}
#endif
#ifdef ENOTSOCK
,  {ENOTSOCK,            Software::EC_NOTSOCK}
#endif
#ifdef EDESTADDRREQ
,  {EDESTADDRREQ,        Software::EC_DESTADDRREQ}
#endif
#ifdef EMSGSIZE
,  {EMSGSIZE,            Software::EC_MSGSIZE}
#endif
#ifdef EPROTOTYPE
,  {EPROTOTYPE,          Software::EC_PROTOTYPE}
#endif
#ifdef ENOPROTOOPT
,  {ENOPROTOOPT,         Software::EC_NOPROTOOPT}
#endif
#ifdef EPROTONOSUPPORT
,  {EPROTONOSUPPORT,     Software::EC_PROTONOSUPPORT}
#endif
#ifdef ESOCKTNOSUPPORT
,  {ESOCKTNOSUPPORT,     Software::EC_SOCKTNOSUPPORT}
#endif
#ifdef EOPNOTSUPP
,  {EOPNOTSUPP,          Software::EC_OPNOTSUPP}
#endif
#ifdef EPFNOSUPPORT
,  {EPFNOSUPPORT,        Software::EC_PFNOSUPPORT}
#endif
#ifdef EAFNOSUPPORT
,  {EAFNOSUPPORT,        Software::EC_AFNOSUPPORT}
#endif
#ifdef EADDRINUSE
,  {EADDRINUSE,          Software::EC_ADDRINUSE}
#endif
#ifdef EADDRNOTAVAIL
,  {EADDRNOTAVAIL,       Software::EC_ADDRNOTAVAIL}
#endif
#ifdef ENETDOWN
,  {ENETDOWN,            Software::EC_NETDOWN}
#endif
#ifdef ENETUNREACH
,  {ENETUNREACH,         Software::EC_NETUNREACH}
#endif
#ifdef ENETRESET
,  {ENETRESET,           Software::EC_NETRESET}
#endif
#ifdef ECONNABORTED
,  {ECONNABORTED,        Software::EC_CONNABORTED}
#endif
#ifdef ECONNRESET
,  {ECONNRESET,          Software::EC_CONNRESET}
#endif
#ifdef ENOBUFS
,  {ENOBUFS,             Software::EC_NOBUFS}
#endif
#ifdef EISCONN
,  {EISCONN,             Software::EC_ISCONN}
#endif
#ifdef ENOTCONN
,  {ENOTCONN,            Software::EC_NOTCONN}
#endif
#ifdef ESHUTDOWN
,  {ESHUTDOWN,           Software::EC_SHUTDOWN}
#endif
#ifdef ETIMEDOUT
,  {ETIMEDOUT,           Software::EC_TIMEDOUT}
#endif
#ifdef ECONNREFUSED
,  {ECONNREFUSED,        Software::EC_CONNREFUSED}
#endif
#ifdef EHOSTDOWN
,  {EHOSTDOWN,           Software::EC_HOSTDOWN}
#endif
#ifdef EHOSTUNREACH
,  {EHOSTUNREACH,        Software::EC_HOSTUNREACH}
#endif
#ifdef ERESTART
,  {ERESTART,            Software::EC_RESTART}
#endif
#ifdef EPROCLIM
,  {EPROCLIM,            Software::EC_PROCLIM}
#endif
#ifdef EUSERS
,  {EUSERS,              Software::EC_USERS}
#endif
#ifdef EWOULDBLOCK
,  {EWOULDBLOCK,         Software::EC_WOULDBLOCK}
#endif

#ifdef _OS_WIN
,  {WSAEINTR,            Software::EC_INTR}
,  {WSAEBADF,            Software::EC_BADF}
,  {WSAEACCES,           Software::EC_ACCES}
,  {WSAEFAULT,           Software::EC_FAULT}
,  {WSAEINVAL,           Software::EC_INVAL}
,  {WSAEMFILE,           Software::EC_MFILE}
,  {WSAESTALE,           Software::EC_STALE}
,  {WSAEWOULDBLOCK,      Software::EC_WOULDBLOCK}
,  {WSAEINPROGRESS,      Software::EC_INPROGRESS}
,  {WSAEALREADY,         Software::EC_ALREADY}
,  {WSAENOTSOCK,         Software::EC_NOTSOCK}
,  {WSAEDESTADDRREQ,     Software::EC_DESTADDRREQ}
,  {WSAEMSGSIZE,         Software::EC_MSGSIZE}
,  {WSAEPROTOTYPE,       Software::EC_PROTOTYPE}
,  {WSAENOPROTOOPT,      Software::EC_NOPROTOOPT}
,  {WSAEPROTONOSUPPORT,  Software::EC_PROTONOSUPPORT}
,  {WSAESOCKTNOSUPPORT,  Software::EC_SOCKTNOSUPPORT}
,  {WSAEOPNOTSUPP,       Software::EC_OPNOTSUPP}
,  {WSAEPFNOSUPPORT,     Software::EC_PFNOSUPPORT}
,  {WSAEAFNOSUPPORT,     Software::EC_AFNOSUPPORT}
,  {WSAEADDRINUSE,       Software::EC_ADDRINUSE}
,  {WSAEADDRNOTAVAIL,    Software::EC_ADDRNOTAVAIL}
,  {WSAENETDOWN,         Software::EC_NETDOWN}
,  {WSAENETUNREACH,      Software::EC_NETUNREACH}
,  {WSAENETRESET,        Software::EC_NETRESET}
,  {WSAECONNABORTED,     Software::EC_CONNABORTED}
,  {WSAECONNRESET,       Software::EC_CONNRESET}
,  {WSAENOBUFS,          Software::EC_NOBUFS}
,  {WSAEISCONN,          Software::EC_ISCONN}
,  {WSAENOTCONN,         Software::EC_NOTCONN}
,  {WSAESHUTDOWN,        Software::EC_SHUTDOWN}
,  {WSAETIMEDOUT,        Software::EC_TIMEDOUT}
,  {WSAECONNREFUSED,     Software::EC_CONNREFUSED}
,  {WSAEHOSTDOWN,        Software::EC_HOSTDOWN}
,  {WSAEHOSTUNREACH,     Software::EC_HOSTUNREACH}
,  {WSAEPROCLIM,         Software::EC_PROCLIM}
,  {WSAEUSERS,           Software::EC_USERS}
,  {WSAELOOP,            Software::EC_LOOP}
,  {WSAENAMETOOLONG,     Software::EC_NAMETOOLONG}
,  {WSAENOTEMPTY,        Software::EC_NOTEMPTY}
,  {WSAEDQUOT,           Software::EC_DQUOT}
,  {WSAEREMOTE,          Software::EC_REMOTE}
,  {WSAETOOMANYREFS,     Software::EC_TOOMANYREFS}
#endif

,  {(-1),                Software::SystemEC(-1)} // MUST BE LAST
}; // convertEC

//----------------------------------------------------------------------------
//
// Table-
//       convertEI
//
// Purpose-
//       SystemEC to SystemEI conversion table.
//
//----------------------------------------------------------------------------
static const char*     convertEI[]= // EC to EI conversion table
{  "No error"                       //   0: No error
,  "Operation not permitted"        //   1: Operation not permitted
,  "No such file or directory"      //   2: No such file or directory
,  "No such process"                //   3: No such process
,  "Interrupted system call"        //   4: Interrupted system call
,  "I/O error"                      //   5: I/O error
,  "No such device or address"      //   6: No such device or address
,  "Arg list too long"              //   7: Arg list too long
,  "Exec format error"              //   8: Exec format error
,  "Bad file descriptor"            //   9: Bad file descriptor
,  "No child processes"             //  10: No child processes
,  "Resource temporarily unavailable"
                                    //  11: Resource temporarily unavailable
,  "Not enough storage"             //  12: Not enough storage
,  "Permission denied"              //  13: Permission denied
,  "Bad address"                    //  14: Bad address
,  "Block device required"          //  15: Block device required
,  "Resource busy"                  //  16: Resource busy
,  "File exists"                    //  17: File exists
,  "Improper link"                  //  18: Improper link
,  "No such device"                 //  19: No such device
,  "Not a directory"                //  20: Not a directory
,  "Is a directory"                 //  21: Is a directory
,  "Invalid argument"               //  22: Invalid argument
,  "Too many open files in system"  //  23: Too many open files in system
,  "Too many open files"            //  24: Too many open files
,  "Inappropriate I/O control operation"
                                    //  25: Inappropriate I/O control operation
,  "Text file busy"                 //  26: Text file busy
,  "File too large"                 //  27: File too large
,  "No space left on device"        //  28: No space left on device
,  "Invalid seek"                   //  29: Invalid seek
,  "Read only file system"          //  30: Read only file system
,  "Too many links"                 //  31: Too many links
,  "Broken pipe"                    //  32: Broken pipe
,  "Domain error within math function"
                                    //  33: Domain error within math function
,  "Result too large"               //  34: Result too large
,  "No message of desired type"     //  35: No message of desired type
,  "Identifier removed"             //  36: Identifier removed
,  "Channel number out of range"    //  37: Channel number out of range
,  "Level 2 not synchronized"       //  38: Level 2 not synchronized
,  "Level 3 halted"                 //  39: Level 3 halted
,  "Level 3 reset"                  //  40: Level 3 reset
,  "Link number out of range"       //  41: Link number out of range
,  "Protocol driver not attached"   //  42: Protocol driver not attached
,  "No CSI structure available"     //  43: No CSI structure available
,  "Level 2 halted"                 //  44: Level 2 halted
,  "Resource deadlock avoided"      //  45: Resource deadlock avoided
,  "Device not ready"               //  46: Device not ready
,  "Write-protected media"          //  47: Write-protected media
,  "Unformatted media"              //  48: Unformatted media
,  "No locks available"             //  49: No locks available
,  "no connection"                  //  50: no connection
,  "(undefined) 51"                 //  51: (undefined)
,  "no filesystem"                  //  52: no filesystem
,  "(undefined) 53"                 //  53: (undefined)
,  "Operation would block"          //  54: Operation would block
,  "Operation now in progress"      //  55: Operation now in progress
,  "Operation already in progress"  //  56: Operation already in progress
,  "Socket operation on non-socket" //  57: Socket operation on non-socket
,  "Destination address required"   //  58: Destination address required
,  "Message too long"               //  59: Message too long
,  "Protocol wrong type for socket" //  60: Protocol wrong type for socket
,  "Protocol not available"         //  61: Protocol not available
,  "Protocol not supported"         //  62: Protocol not supported
,  "Socket type not supported"      //  63: Socket type not supported
,  "Operation not supported on socket"
                                    //  64: Operation not supported on socket
,  "Protocol family not supported"  //  65: Protocol family not supported
,  "Address family not supported by protocol family"
                                    //  66: Address family not supported
,  "Address already in use"         //  67: Address already in use
,  "Can't assign requested address" //  68: Can't assign requested address
,  "Network is down"                //  69: Network is down
,  "Network is unreachable"         //  70: Network is unreachable
,  "Network dropped connection on reset"
                                    //  71: Network dropped connection on reset
,  "Software caused connection abort"
                                    //  72: Software caused connection abort
,  "Connection reset by peer"       //  73: Connection reset by peer
,  "No buffer space available"      //  74: No buffer space available
,  "Socket is already connected"    //  75: Socket is already connected
,  "Socket is not connected"        //  76: Socket is not connected
,  "Can't send after socket shutdown"
                                    //  77: Can't send after socket shutdown
,  "Connection timed out"           //  78: Connection timed out
,  "Connection refused"             //  79: Connection refused
,  "Host is down"                   //  80: Host is down
,  "No route to host"               //  81: No route to host
,  "restart the system call"        //  82: restart the system call
,  "Too many processes"             //  83: Too many processes
,  "Too many users"                 //  84: Too many users
,  "Too many levels of symbolic links"
                                    //  85: Too many levels of symbolic links
,  "File name too long"             //  86: File name too long
,  "Directory not empty"            //  87: Directory not empty
,  "Disc quota exceeded"            //  88: Disc quota exceeded
,  "Invalid file system control data"
                                    //  89: Invalid file system control data
,  "(undefined) 90"                 //  90: (undefined)
,  "(undefined) 91"                 //  91: (undefined)
,  "(undefined) 92"                 //  92: (undefined)
,  "Item is not local to host"      //  93: Item is not local to host
,  "(undefined) 94"                 //  94: (undefined)
,  "(undefined) 95"                 //  95: (undefined)
,  "(undefined) 96"                 //  96: (undefined)
,  "(undefined) 97"                 //  97: (undefined)
,  "(undefined) 98"                 //  98: (undefined)
,  "(undefined) 99"                 //  99: (undefined)
,  "(undefined) 100"                // 100: (undefined)
,  "(undefined) 101"                // 101: (undefined)
,  "(undefined) 102"                // 102: (undefined)
,  "(undefined) 103"                // 103: (undefined)
,  "(undefined) 104"                // 104: (undefined)
,  "(undefined) 105"                // 105: (undefined)
,  "(undefined) 106"                // 106: (undefined)
,  "(undefined) 107"                // 107: (undefined)
,  "(undefined) 108"                // 108: (undefined)
,  "Function not implemented"       // 109: Function not implemented
,  "media surface error"            // 110: media surface error
,  "I/O completed but needs relocation"
                                    // 111: I/O completed but needs relocation
,  "no attribute found"             // 112: no attribute found
,  "security authentication denied" // 113: security authentication denied
,  "not a trusted program"          // 114: not a trusted program
,  "Too many references: can't splice"
                                    // 115: Too many references: can't splice
,  "Invalid wide character"         // 116: Invalid wide character
,  "asynchronous i/o cancelled"     // 117: asynchronous i/o cancelled
,  "temp out of streams resources"  // 118: temp out of streams resources
,  "I_STR ioctl timed out"          // 119: I_STR ioctl timed out
,  "wrong message type at stream head"
                                    // 120: wrong message type at stream head
,  "STREAMS protocol error"         // 121: STREAMS protocol error
,  "no message ready at stream head"
                                    // 122: no message ready at stream head
,  "fd is not a stream"             // 123: fd is not a stream
,  "threads unsupported value"      // 124: threads unsupported value
,  "multihop is not allowed"        // 125: multihop is not allowed
,  "the link has been severed"      // 126: the link has been severed
,  "value too large to be stored "  // 127: value too large to be stored
       "in required data type"      //      in required data type
};

//----------------------------------------------------------------------------
//
// Method-
//       Software::getCwd
//
// Purpose-
//       Get current working directory.
//
//----------------------------------------------------------------------------
char*                               // -> resultant
   Software::getCwd(                // Get current working directory
     char*             resultant,   // Resultant string
     int               size)        // Sizeof(resultant)
{
   return ::getcwd(resultant,size); // From stdlib
}

//----------------------------------------------------------------------------
//
// Method-
//       Software::getPid
//
// Purpose-
//       Get system process identifier.
//
//----------------------------------------------------------------------------
Software::Pid_t                     // Resultant
   Software::getPid( void )         // Get process identifier
{
   return ::getpid();
}

//----------------------------------------------------------------------------
//
// Method-
//       Software::getTid
//
// Purpose-
//       Get system thread identifier.
//
//----------------------------------------------------------------------------
Software::Tid_t                     // Resultant
   Software::getTid( void )         // Get thread identifier
{
#if   defined(_OS_WIN)
   return (unsigned long)::GetCurrentThreadId();

#elif defined(_OS_BSD)
   return (unsigned long)pthread_self();

#else
   return 1;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Software::getSystemEC
//
// Purpose-
//       Get the SystemEC
//
//----------------------------------------------------------------------------
Software::SystemEC                  // The SystemEC
   Software::getSystemEC( void )    // Get SystemEC
{
   Software::SystemEC  ec;          // The SystemEC
   int                 sc;          // The system error code

   unsigned            i;

   #if defined(_OS_WIN)
     sc= ::WSAGetLastError();

   #else
     sc= errno;
   #endif

   ec= SystemEC(-1);
   for(i=0; convertEC[i].sc >= 0; i++)
   {
     if( convertEC[i].sc == sc )
     {
       ec= convertEC[i].ec;
       break;
     }
   }

   return ec;
}

//----------------------------------------------------------------------------
//
// Method-
//       Software::getSystemEI
//
// Purpose-
//       Convert SystemEC to text string.
//
//----------------------------------------------------------------------------
const char*                         // Resultant
   Software::getSystemEI(           // Convert SystemEI to string
     SystemEC          ec)          // Error Code
{
   const char*         result= "Invalid SystemEC";

   if( ec >= 0 && ec < EC_MAX )
     result= convertEI[ec];

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Software::setCwd
//
// Purpose-
//       Set current working directory.
//
//----------------------------------------------------------------------------
const char*                         // Resultant (NULL OK)
   Software::setCwd(                // Get current working directory
     const char*       path)        // Relative or absolute path
{
   const char*         rm;          // Resultant
   int                 rc;          // Return code

   rc= ::chdir(path);               // From stdlib
   rm= NULL;
   if( rc != 0 )
     rm= "Software::setCwd() ::chdir failure";

   return rm;
}

