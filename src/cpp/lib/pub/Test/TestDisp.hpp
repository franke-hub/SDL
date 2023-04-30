//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestDisp.hpp
//
// Purpose-
//       TestDisp internal classes and subroutines
//
// Last change date-
//       2023/04/27
//
// Implementation notes-
//       Only included from TestDisp.cpp
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       PassAlongTask
//       PassAlongLambdaTask
//
// Purpose-
//       Pass work to next Task in list.
//
//----------------------------------------------------------------------------
class PassAlongTask : public dispatch::Task {
public:
dispatch::Task*        next;        // Next Task in list

virtual
   ~PassAlongTask( void )
{  if( opt_hcdm ) debugf("~PassAlongTask(%p)\n", this); }

   PassAlongTask(
     dispatch::Task*   next_)
:  dispatch::Task()
,  next(next_)
{  if( opt_hcdm ) debugf("PassAlongTask(%p)\n", this); }

virtual void
   work(
     dispatch::Item*   item)
{
   if( opt_hcdm )
     debugf("PassAlongTask(%p)::work(%p) next(%p)\n", this, item, next);

   if( USE_TRACE )
     Trace::trace(".PAT", " PAT", item, next);

   if( next )
     next->enqueue(item);           // Give the work to the next Task
   else
     item->post();
}
}; // class PassAlongTask

class PassAlongLambdaTask : public dispatch::LambdaTask {
protected:
dispatch::Task*        next;        // Next Task in list

public:
virtual
   ~PassAlongLambdaTask( void )
{
   if( opt_hcdm && opt_verbose > 1 )
     debugf("~PassAlongLambdaTask(%p)\n", this);
}

   PassAlongLambdaTask(
     dispatch::Task*   next_)
:  LambdaTask([this](dispatch::Item* item)
{
   if( opt_hcdm && opt_verbose > 1 )
     debugf("PassAlongLambdaTask(%p)::work(%p) next(%p)\n", this, item, next);

   if( USE_TRACE )
     Trace::trace("WORK", ".PAL", item, next);

   next->enqueue(item);
})
,  next(next_)
{
   if( opt_hcdm && opt_verbose > 1 )
     debugf("PassAlongLambdaTask(%p)\n", this);
}
}; // class PassAlongLambdaTask

//----------------------------------------------------------------------------
//
// Class-
//       RondesvousTask
//
// Purpose-
//       Report work item received, drive Done callback.
//
//----------------------------------------------------------------------------
class RondesvousTask : public dispatch::Task {
protected:
int                    index;       // Rondesvous identifier

public:
static std::atomic<uint64_t>
                       rondesvous;  // Rondesvous bit map

virtual
   ~RondesvousTask( void )
{  if( opt_hcdm ) debugf("~RondesvousTask(%p) %2d\n", this, index); }

   RondesvousTask(
     int               index)
:  dispatch::Task()
,  index(index)
{  if( opt_hcdm ) debugf("RondesvousTask(%p) %2d\n", this, index); }

virtual void
   work(
     dispatch::Item*   item)
{
   uint64_t bitmap= ((uint64_t)1)<<index;

   uint64_t oldValue= rondesvous.load();
   for(;;)
   {
     uint64_t newValue= oldValue | bitmap;
     if( rondesvous.compare_exchange_strong(oldValue, newValue) )
       break;
   }

// debugf("%2d %.16" PRIx64 " %.16" PRIx64 "\n", index, bitmap, newValue);
   item->post();
}
}; // class RondesvousTask

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::throwf
//
// Purpose-
//       Write a diagnostic error message and throw an exception.
//
//----------------------------------------------------------------------------
[[noreturn]]
ATTRIB_PRINTF(2,3)
static void
   throwf(                          // Abort with error message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   fprintf(stderr, "%4d %s: ABORT: ", line, __FILE__);

   va_start(argptr, fmt);           // Initialize va_ functions
   vthrowf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}
