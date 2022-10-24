//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Global.h
//
// Purpose-
//       HTTP Global data area.
//
// Last change date-
//       2022/10/21
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_GLOBAL_H_INCLUDED
#define _LIBPUB_HTTP_GLOBAL_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex, super class
#include <string>                   // For std::string
#include <time.h>                   // For clock_gettime

#include <pub/Socket.h>             // For pub::Socket, ...
#include "pub/http/Recorder.h"      // For pub::Recorder, ...

#include "pub/http/Agent.h"         // For pub::http::ClientConnectionPair, ...

#include "dev/bits/devconfig.h"     // For HTTP config controls

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
//
// Struct-
//       TimingRecord
//
// Purpose-
//       The Timing data record, for throughput recording.
//
// Implementation note-
//       Add a pointer to this (or this object) in Stream.
//       ClientStream::end or ~ClientStream updates the global records.
//         Note that since this is driven asynchronously, the DelayRecord
//         must use atomic operations.
//
//----------------------------------------------------------------------------
struct TimingRecord {
typedef struct timespec   clock_t;  // Monotomic time, in nanoseconds

enum                                // Timing point indexes
{  IX_ON_END                        // Prior on_end
,  IX_TOTAL= IX_ON_END              // (Use the ON_END record for total time)
,  IX_CLI_CREATE                    // Client create stream
,  IX_REQ_WRITE                     // Client write Request
,  IX_ENQ_WRITE                     // Client enqueue write
,  IX_DEQ_WRITE                     // Client dequeue write
,  IX_CLI_WRITE                     // Client write
// IX_CLI_WAIT                      // Client wait for Response

,  IX_SRV_CREATE                    // Server create stream
,  IX_SRV_READ                      // Server read Request
,  IX_SRV_REQ_DO                    // Server do_request
,  IX_SRV_WRITE                     // Server write response
,  IX_SRV_REQ_DONE                  // Server do_request done
,  IX_SRV_END                       // Server Stream end

,  IX_CLI_ASYNC                     // Client asynch
,  IX_CLI_READ                      // Client read
,  IX_ENQ_RESP                      // Client enqueue Response
,  IX_DEQ_RESP                      // Client dequeue Response
,  IX_RSP_READ                      // Client read Response
,  IX_RSP_POST                      // Client post wait complete
,  IX_CLI_END                       // Client Stream end
,  IX_LENGTH                        // Number of indexes
}; // Timing indexes

clock_t                clock[IX_LENGTH]= {}; // The timing array

//----------------------------------------------------------------------------
// TimingRecord::c2d, convert clock_t to double
//----------------------------------------------------------------------------
static double
   c2d(clock_t clock)
{  return double(clock.tv_sec) + double(clock.tv_nsec) / 1'000'000'000.0; }

//----------------------------------------------------------------------------
// TimingRecord::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display

//----------------------------------------------------------------------------
// TimingRecord::record, record event time
//----------------------------------------------------------------------------
void
   record(int index)              // Record an event
{  clock_gettime(CLOCK_REALTIME, clock + index); }

static void
   record(clock_t* clock)         // Record an event
{  clock_gettime(CLOCK_REALTIME, clock); }
}; // struct TimingRecord

//----------------------------------------------------------------------------
//
// Struct-
//       DelayRecord
//
// Purpose-
//       The code section delay record, for throughput reporting.
//
// Implementation notes-
//       All delays, including total, only apply to the particular record.
//
//----------------------------------------------------------------------------
struct DelayRecord : public Recorder::Record {
std::atomic_size_t     counter= 0;  // Operation counter
std::atomic<double>    tot_delay= 0.0; // Total delay
std::atomic<double>    max_delay= 0.0; // Maximum delay
std::atomic<double>    min_delay= 0.0; // Minimum delay

int                    index;       // Our index

   DelayRecord( void );             // Default constructor
   ~DelayRecord( void );            // Destructor

//----------------------------------------------------------------------------
// DelayRecord::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display

//----------------------------------------------------------------------------
// DelayRecord::update
//----------------------------------------------------------------------------
void
   update(const TimingRecord& record); // Update from TimingRecord
}; // struct DelayRecord

//----------------------------------------------------------------------------
//
// Struct-
//       Global
//
// Purpose-
//       The Global data area. Used for throughput performance debugging.
//
//----------------------------------------------------------------------------
struct Global {                     // Global data area
//----------------------------------------------------------------------------
// Global::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef ClientConnectionPair        key_t;
typedef TimingRecord*               value_t;
typedef Socket::sockaddr_u          sockaddr_u;

struct op_lt {                      // Less than compare operator
bool operator()(const key_t& lhs, const key_t& rhs) const
{  return lhs.operator<(rhs); }
}; // struct op_lt

typedef std::map<key_t, value_t, op_lt>
                       Map_t;       // The Client Map type
typedef Map_t::const_iterator
                       const_iterator; // The Client Map const iterator type
typedef Map_t::iterator
                       iterator;    // The Client Map const iterator type

//----------------------------------------------------------------------------
// Global::Attributes
//----------------------------------------------------------------------------
static struct Global*  global;      // The Global data area

// The Server uses the map to find the TimingRecord
Map_t                  map;         // The active Client :: TimingRecord map
DelayRecord            record[TimingRecord::IX_LENGTH];

//----------------------------------------------------------------------------
// Global::Constructor, destructor
//----------------------------------------------------------------------------
public:
   Global( void );                  // Constructor
   ~Global( void );                 // Destructor

//----------------------------------------------------------------------------
// Global::debug
//----------------------------------------------------------------------------
void
   debug(const char* info= "") const; // Debugging display

//----------------------------------------------------------------------------
// Global::Methods
//----------------------------------------------------------------------------
void
   update(const TimingRecord& record); // Update all records
}; // class Global
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_Global_H_INCLUDED
