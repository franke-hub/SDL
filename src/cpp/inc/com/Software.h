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
//       Software.h
//
// Purpose-
//       System software interfaces.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SOFTWARE_H_INCLUDED
#define SOFTWARE_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       Software
//
// Purpose-
//       System software accessor class.
//
//----------------------------------------------------------------------------
class Software {                    // System software accessor class
//----------------------------------------------------------------------------
// Software::Attributes
//----------------------------------------------------------------------------
private:
   // None defined

//----------------------------------------------------------------------------
// Software::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef unsigned int   Pid_t;       // Process identifier
typedef intptr_t       Tid_t;       // Thread identifier

enum SystemEC                       // Error codes
{  EC_UNSPEC=          0            // No error
,  EC_PERM=            1            // Operation not permitted
,  EC_NOENT=           2            // No such file or directory
,  EC_SRCH=            3            // No such process
,  EC_INTR=            4            // Interrupted system call
,  EC_IO=              5            // I/O error
,  EC_NXIO=            6            // No such device or address
,  EC_2BIG=            7            // Arg list too long
,  EC_NOEXEC=          8            // Exec format error
,  EC_BADF=            9            // Bad file descriptor
,  EC_CHILD=           10           // No child processes
,  EC_AGAIN=           11           // Resource temporarily unavailable
,  EC_NOMEM=           12           // Not enough space
,  EC_ACCES=           13           // Permission denied
,  EC_FAULT=           14           // Bad address
,  EC_NOTBLK=          15           // Block device required
,  EC_BUSY=            16           // Resource busy
,  EC_EXIST=           17           // File exists
,  EC_XDEV=            18           // Improper link
,  EC_NODEV=           19           // No such device
,  EC_NOTDIR=          20           // Not a directory
,  EC_ISDIR=           21           // Is a directory
,  EC_INVAL=           22           // Invalid argument
,  EC_NFILE=           23           // Too many open files in system
,  EC_MFILE=           24           // Too many open files
,  EC_NOTTY=           25           // Inappropriate I/O control operation
,  EC_TXTBSY=          26           // Text file busy
,  EC_FBIG=            27           // File too large
,  EC_NOSPC=           28           // No space left on device
,  EC_SPIPE=           29           // Invalid seek
,  EC_ROFS=            30           // Read only file system
,  EC_MLINK=           31           // Too many links
,  EC_PIPE=            32           // Broken pipe
,  EC_DOM=             33           // Domain error within math function
,  EC_RANGE=           34           // Result too large
,  EC_NOMSG=           35           // No message of desired type
,  EC_IDRM=            36           // Identifier removed
,  EC_CHRNG=           37           // Channel number out of range
,  EC_L2NSYNC=         38           // Level 2 not synchronized
,  EC_L3HLT=           39           // Level 3 halted
,  EC_L3RST=           40           // Level 3 reset
,  EC_LNRNG=           41           // Link number out of range
,  EC_UNATCH=          42           // Protocol driver not attached
,  EC_NOCSI=           43           // No CSI structure available
,  EC_L2HLT=           44           // Level 2 halted
,  EC_DEADLK=          45           // Resource deadlock avoided
,  EC_NOTREADY=        46           // Device not ready
,  EC_WRPROTECT=       47           // Write-protected media
,  EC_FORMAT=          48           // Unformatted media
,  EC_NOLCK=           49           // No locks available
,  EC_NOCONNECT=       50           // no connection
,  EC_STALE=           52           // no filesystem
,  EC_WOULDBLOCK=      54           // Operation would block
,  EC_INPROGRESS=      55           // Operation now in progress
,  EC_ALREADY=         56           // Operation already in progress
,  EC_NOTSOCK=         57           // Socket operation on non-socket
,  EC_DESTADDRREQ=     58           // Destination address required
,  EC_MSGSIZE=         59           // Message too long
,  EC_PROTOTYPE=       60           // Protocol wrong type for socket
,  EC_NOPROTOOPT=      61           // Protocol not available
,  EC_PROTONOSUPPORT=  62           // Protocol not supported
,  EC_SOCKTNOSUPPORT=  63           // Socket type not supported
,  EC_OPNOTSUPP=       64           // Operation not supported on socket
,  EC_PFNOSUPPORT=     65           // Protocol family not supported
,  EC_AFNOSUPPORT=     66           // Address family not supported by PF
,  EC_ADDRINUSE=       67           // Address already in use
,  EC_ADDRNOTAVAIL=    68           // Can't assign requested address
,  EC_NETDOWN=         69           // Network is down
,  EC_NETUNREACH=      70           // Network is unreachable
,  EC_NETRESET=        71           // Network dropped connection on reset
,  EC_CONNABORTED=     72           // Software caused connection abort
,  EC_CONNRESET=       73           // Connection reset by peer
,  EC_NOBUFS=          74           // No buffer space available
,  EC_ISCONN=          75           // Socket is already connected
,  EC_NOTCONN=         76           // Socket is not connected
,  EC_SHUTDOWN=        77           // Can't send after socket shutdown
,  EC_TIMEDOUT=        78           // Connection timed out
,  EC_CONNREFUSED=     79           // Connection refused
,  EC_HOSTDOWN=        80           // Host is down
,  EC_HOSTUNREACH=     81           // No route to host
,  EC_RESTART=         82           // restart the system call
,  EC_PROCLIM=         83           // Too many processes
,  EC_USERS=           84           // Too many users
,  EC_LOOP=            85           // Too many levels of symbolic links
,  EC_NAMETOOLONG=     86           // File name too long
,  EC_NOTEMPTY=        87           // Directory not empty
,  EC_DQUOT=           88           // Disc quota exceeded
,  EC_CORRUPT=         89           // Invalid file system control data
//                     :
,  EC_REMOTE=          93           // Item is not local to host
//                     :
,  EC_NOSYS=           109          // Function not implemented
,  EC_MEDIA=           110          // Media surface error
,  EC_SOFT=            111          // I/O completed, but needs relocation
,  EC_NOATTR=          112          // No attribute found
,  EC_SAD=             113          // Security authentication denied
,  EC_NOTRUST=         114          // Not a trusted program
,  EC_TOOMANYREFS=     115          // Too many references: can't splice
,  EC_ILSEQ=           116          // Invalid wide character
,  EC_CANCELED=        117          // Asynchronous i/o cancelled
,  EC_NOSR=            118          // Temp out of streams resources
,  EC_TIME=            119          // I_STR ioctl timed out
,  EC_BADMSG=          120          // Wrong message type at stream head
,  EC_PROTO=           121          // STREAMS protocol error
,  EC_NODATA=          122          // No message ready at stream head
,  EC_NOSTR=           123          // FD is not a stream
,  EC_NOTSUP=          124          // Threads unsupported value
,  EC_MULTIHOP=        125          // Multihop is not allowed
,  EC_NOLINK=          126          // The link has been severed
,  EC_OVERFLOW=        127          // Value too large to be stored in
                                    // required data type
,  EC_MAX=             128          // The number of ErrorCode values
}; // enum SystemEC

//----------------------------------------------------------------------------
// Software::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Software( void );               // Destructor

protected:                          // (All methods are static)
   Software( void );                // Default constructor

private:                            // Bitwise copy is prohibited
   Software(const Software&);       // Disallowed copy constructor
   Software& operator=(const Software&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// Software::Methods
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// Software::Static methods
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
//
// Method-
//       Software::getCwd
//
// Purpose-
//       Return the current working directory in the area provided.
//
//----------------------------------------------------------------------------
static char*                        // -> resultant
   getCwd(                          // Get current working directory
     char*             resultant,   // Resultant string
     int               size);       // Sizeof(resultant)

//----------------------------------------------------------------------------
//
// Method-
//       Software::getPid
//
// Purpose-
//       Return the process identifier.
//
//----------------------------------------------------------------------------
static Pid_t                        // The process identifier
   getPid( void );                  // Get process identifier

//----------------------------------------------------------------------------
//
// Method-
//       Software::getSystemEC
//
// Purpose-
//       Return the system error code.
//
//----------------------------------------------------------------------------
static SystemEC                     // The system error code
   getSystemEC( void );             // Get system error code

//----------------------------------------------------------------------------
//
// Method-
//       Software::getSystemEI
//
// Purpose-
//       Return the system error information, a SystemEC descriptor.
//
//----------------------------------------------------------------------------
static const char*                  // The system error information
   getSystemEI(                     // Get system error information
     SystemEC          ec);         // For this system error code

inline static const char*           // The system error information
   getSystemEI( void )              // Get system error information
{  return getSystemEI(getSystemEC()); }

//----------------------------------------------------------------------------
//
// Method-
//       Software::getTid
//
// Purpose-
//       Return the current thread identifier.
//
//----------------------------------------------------------------------------
static Tid_t                        // The thread identifier
   getTid( void );                  // Get thread identifier

//----------------------------------------------------------------------------
//
// Method-
//       Software::setCwd
//
// Purpose-
//       Set the current working directory.
//
//----------------------------------------------------------------------------
static const char*                  // Return string (NULL OK)
   setCwd(                          // Set current working directory
     const char*       path);       // Relative or absolute path
}; // class Software

#endif // SOFTWARE_H_INCLUDED
