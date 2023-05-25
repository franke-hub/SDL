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
//       dev/Recorder.h
//
// Purpose-
//       Statistical event recorder
//
// Last change date-
//       2022/10/19
//
// Implementation notes-
//       Records contain statistical information that can be displayed by the
//       Recorder or reset. The Recorder provides mechanisms for controlling
//       a list of these Records. It's used when performance testing to track
//       events that might be of interest, wherever they may be.
//       These recording tests are normally been used for experimentation and
//       are generally unused in production code.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_RECORDER_H_INCLUDED
#define _LIBPUB_RECORDER_H_INCLUDED

#include <functional>               // For std::function
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string

#include <pub/List.h>               // For pub::List

#include "dev/bits/devconfig.h"     // For HTTP config controls

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Recorder
//
// Purpose-
//       (Globally lockable) event recorder
//
//----------------------------------------------------------------------------
class Recorder {                    // Event recorder
//----------------------------------------------------------------------------
// Recorder::Record
//----------------------------------------------------------------------------
public:
struct Record {
typedef std::function<std::string(void)>      f_report;
typedef std::function<void(void)>             f_reset;

std::string            name= "";    // The name of the Record
f_report               h_report= []() { return ""; }; // Report recording
f_reset                h_reset=  []() { }; // Reset this Record

void on_report(const f_report& f)   // Set reporter function
{  h_report= f; }

void on_reset(const f_reset& f)     // Set reset function
{  h_reset= f; }
}; // class Record

//----------------------------------------------------------------------------
// Recorder::RecordItem
//----------------------------------------------------------------------------
struct RecordItem : public List<RecordItem>::Link {
Record*                record;      // The associated Record
   RecordItem(Record* R)            // Constructor
:  record(R) {}
}; // struct RecordItem

//----------------------------------------------------------------------------
// Recorder::typedefs and enumerations
//----------------------------------------------------------------------------
typedef std::function<void(Record&)>
                       f_reporter;  // The reporter function

typedef std::string    string;      // Import std::string
typedef std::mutex     mutex_t;     // The mutex type

//----------------------------------------------------------------------------
// Recorder::Attributes
//----------------------------------------------------------------------------
protected:
static Recorder*       common;      // -> The Common Recorder instance
static mutex_t         mutex;       // -> The global Recorder mutex

List<RecordItem>       list;        // The RecordItem list

//----------------------------------------------------------------------------
// Recorder::Constructors/Denstructor
//----------------------------------------------------------------------------
public:
   Recorder( void );                // Default constructor
   ~Recorder( void );               // Destructor

//----------------------------------------------------------------------------
// Recorder::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display

//----------------------------------------------------------------------------
// Recorder::Accessor methods
//----------------------------------------------------------------------------
static mutex_t&                     // The global Recorder mutex
   get_mutex( void )                // Get global Recorder mutex
{  return mutex; }

static Recorder*                    // -> The common Recorder instance
   get( void );                     // Get the common Recorder instance

static Recorder*                    // (The old common Recorder instance)
   set(                             // Set
     Recorder*         debug);      // This new common Recorder instance)

static Recorder*                    // (The current common Recorder instance)
   show( void )                     // Get the current common Recorder instance
{  return common; }                 // Without trying to create it

//----------------------------------------------------------------------------
// Recorder::Methods
//----------------------------------------------------------------------------
void insert(Record*);               // Insert Record* into List, NO duplicates
void remove(Record*);               // Remove Record* from List

void report(f_reporter);            // Generate report
void reset( void );                 // Reset recording data
}; // class Recorder
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_RECORDER_H_INCLUDED
