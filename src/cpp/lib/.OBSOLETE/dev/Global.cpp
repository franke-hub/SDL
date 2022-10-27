//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Global.cpp
//
// Purpose-
//       Implement http/Global.h
//
// Last change date-
//       2022/10/21
//
//----------------------------------------------------------------------------
#include <cinttypes>                // For integer types
#include <cstdio>                   // For fprintf
#include <cstring>                  // For memcmp, memset
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/http/Recorder.h"      // For pub::Recorder

#include "pub/http/Global.h"        // For pub::http::Global, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using std::string;

namespace _LIBPUB_NAMESPACE::http {  // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
struct Global*         Global::global= nullptr; // *THE* Global data area
static int             ouch= 8;

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char*     ix2name[TimingRecord::IX_LENGTH]=
{  "Total delay"
,  "Client create Stream"
,  "Client Request write"
,  "Client ENQ write"
,  "Client DEQ write"
,  "Client write request"
// "Client wait"

,  "Server create Stream"
,  "Server read"
,  "Server do_request"
,  "Server write response"
,  "Server request done"
,  "Server stream end"

,  "Client async"
,  "Client read response"
,  "Client ENQ response"
,  "Client DEQ response"
,  "Client Response read"
,  "Client Response post"
,  "Client Stream end"
}; // ix2name

//----------------------------------------------------------------------------
//
// Subroutine-
//       c2d
//
// Purpose-
//       TimingRecord::clock_t to double
//
//----------------------------------------------------------------------------
static inline double
   c2d(TimingRecord::clock_t c)
{  return double(c.tv_sec) + double(c.tv_nsec) / 1'000'000'000.0; }

//----------------------------------------------------------------------------
//
// Method-
//       TimingRecord::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   TimingRecord::debug(const char* info) const
{
   debugf("TimingRecord::debug(%s)\n", info);

   for(int i= 0; i<IX_LENGTH; ++i)
     debugf("[%2d] %12.9f\n", i, c2d(clock[i]));
}

//----------------------------------------------------------------------------
//
// Method-
//       DelayRecord::DelayRecord
//       DelayRecord::~DelayRecord
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   DelayRecord::DelayRecord( void ) // Constructor
{
   on_report([this]() {
     char buffer[128];              // (More than large enough)
     double avg_delay= 0.0;
     if( counter.load() > 0)
       avg_delay= tot_delay.load() / counter.load();
     else
       min_delay.store(0.0);
     sprintf(buffer, "%'8zd {%'10.6f,%'10.6f,%'10.6f,%'10.6f}: "
            , counter.load(), tot_delay.load()
            , min_delay.load(), avg_delay, max_delay.load());
     return std::string(buffer) + ix2name[index];
   }); // on_report

   on_reset([this]() {
     counter.store(0);
     tot_delay.store(0);
     max_delay.store(0);
     min_delay.store(0);
   }); // on_reset

   // Add entry to Recorder
   Recorder::get()->insert(this);
}  // DelayRecord::DelayRecord

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   DelayRecord::~DelayRecord( void ) // Destructor
{
   // Remove entry from Recorder
   Recorder::get()->remove(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       DelayRecord::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   DelayRecord::debug(const char* info) const
{
   debugf("DelayRecord::debug(%s)\n", info);

   debugf("%'8zd {%'12.9f,%'12.9f,%'12.9f} %s\n"
          , counter.load(), tot_delay.load(), min_delay.load()
          , max_delay.load(), ix2name[index]);
}

//----------------------------------------------------------------------------
//
// Method-
//       DelayRecord::update
//
// Purpose-
//       Update using TimingRecord
//
//----------------------------------------------------------------------------
void
   DelayRecord::update(             // Update this DelayRecord
     const TimingRecord& record)    // Using this TimingRecord
{
   double time;
   if( index == 0 ) {
     if( c2d(record.clock[1]) == 0.0
         || c2d(record.clock[TimingRecord::IX_LENGTH-1]) == 0.0 )
       return;

     time= c2d(record.clock[TimingRecord::IX_LENGTH-1]) - c2d(record.clock[1]);
   } else {
     if( c2d(record.clock[index]) == 0.0 || c2d(record.clock[index-1]) == 0.0 )
       return;

     time= c2d(record.clock[index]) - c2d(record.clock[index-1]);
   }

   if( time < 0 && ouch > 0 ) {
     debugf("\n\n[%2d] < 0\n", index);
     record.debug("negative");
     --ouch;
   }

   ++counter;
   double old_value= tot_delay.load();
   for(;;) {
     double new_value= old_value + time;
     if( tot_delay.compare_exchange_weak(old_value, new_value) )
       break;
   }

   old_value= max_delay.load();
   while( time > old_value ) {
     if( max_delay.compare_exchange_weak(old_value, time) )
       break;
   }

   old_value= min_delay.load();
   while( time < old_value ) {
     if( min_delay.compare_exchange_weak(old_value, time) )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Global::Global
//       Global::~Global
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Global::Global( void )           // Default constructor
{
   if( global )
     throw std::runtime_error("Only one Global allowed");
   global= this;

   // Initialize the record indexes
   for(int i= 0; i<TimingRecord::IX_LENGTH; ++i) {
     record[i].index= i;
     record[i].min_delay.store(99.0); // (We expect well under a second)
   }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Global::~Global( void )          // Destructor
{
   global= nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Global::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Global::debug(const char* info) const
{
   debugf("Global::debug(%s)\n", info);
   // Not really coded yet
}

//----------------------------------------------------------------------------
//
// Method-
//       Global::update
//
// Purpose-
//       Update all records using TimingRecord
//
//----------------------------------------------------------------------------
void
   Global::update(const TimingRecord& tr)
{
   for(int i= 0; i<TimingRecord::IX_LENGTH; ++i)
     record[i].update(tr);
}
}  // namespace _LIBPUB_NAMESPACE::http
