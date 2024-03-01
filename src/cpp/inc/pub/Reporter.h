//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       pub/Reporter.h
//
// Purpose-
//       Statistical event reporter
//
// Last change date-
//       2024/02/16
//
// Implementation notes-
//       Records contain statistical information that can be displayed by the
//       Reporter or reset. The Reporter provides mechanisms for controlling
//       a list of these Records. It's used when performance testing to track
//       events that might be of interest, wherever they may be.
//       Recording tests are normally used for experimentation and are not
//       generally used in production code.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_REPORTER_H_INCLUDED
#define _LIBPUB_REPORTER_H_INCLUDED

#include <functional>               // For std::function
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string

#include <pub/Latch.h>              // For pub::Latch
#include <pub/List.h>               // For pub::List

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Reporter
//
// Purpose-
//       (Globally lockable) event reporter
//
//----------------------------------------------------------------------------
class Reporter {                    // Event reporter
//----------------------------------------------------------------------------
// Reporter::Record
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

void operator()(Record&);           // (A default report function)
}; // class Record

//----------------------------------------------------------------------------
// Reporter::RecordItem
//----------------------------------------------------------------------------
struct RecordItem : public List<RecordItem>::Link {
Record*                record;      // The associated Record
   RecordItem(Record* R)            // Constructor
:  record(R) {}
}; // struct RecordItem

//----------------------------------------------------------------------------
// Reporter::typedefs and enumerations
//----------------------------------------------------------------------------
typedef std::function<void(Record&)>
                       f_reporter;  // The reporter function

typedef std::string    string;      // Import std::string
typedef Latch          mutex_t;     // The mutex type

//----------------------------------------------------------------------------
// Reporter::Attributes
//----------------------------------------------------------------------------
protected:
static Reporter*       common;      // -> The Common Reporter instance
static mutex_t         mutex;       // -> The global Reporter mutex

List<RecordItem>       list;        // The RecordItem list

//----------------------------------------------------------------------------
// Reporter::Constructors/Denstructor
//----------------------------------------------------------------------------
public:
   Reporter( void );                // Default constructor
   ~Reporter( void );               // Destructor

//----------------------------------------------------------------------------
// Reporter::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display

//----------------------------------------------------------------------------
// Reporter::Accessor methods
//----------------------------------------------------------------------------
static mutex_t&                     // The global Reporter mutex
   get_mutex( void )                // Get global Reporter mutex
{  return mutex; }

static Reporter*                    // -> The common Reporter instance
   get( void );                     // Get the common Reporter instance

static Reporter*                    // (The old common Reporter instance)
   set(                             // Set
     Reporter*         replace);    // This new common Reporter instance)

static Reporter*                    // (The current common Reporter instance)
   show( void )                     // Get the current common Reporter instance
{  return common; }                 // Without trying to create it

//----------------------------------------------------------------------------
// Reporter::Methods
//----------------------------------------------------------------------------
void insert(Record*);               // Insert Record* into List
void remove(Record*);               // Remove Record* from List

void report(f_reporter);            // Generate report
void reset( void );                 // Reset recording data
}; // class Reporter
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_REPORTER_H_INCLUDED
