//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/Stress/Common.h
//
// Purpose-
//       Main::Task sequencing controls, including a trace table.
//
// Last change date-
//       2020/07/05
//
// Implementation notes-
//       Defines class Main and class Task.
//       Only useful in this subdirectory
//         Invoked from Module.h, which enumerates compile-time options.
//
// Implementation notes-
//       opt_verbose:
//         -1 Default, NO verbosity
//         0  Display options
//         1  (Unused)
//         2  Display iteration progress
//         3  Display diagnostic information
//         4  (Unused)
//         5  Diagnostics in signal handler (Set on exception or fault)
//
// Main/Task Synchronization logic-
//       [ Task][ Main] State sequence
//       -if-
//       [ busy][ busy] (initial, both running)
//       [ busy][2idle] Main:wait(): setfsm(); wait(main2idle);
//       [2idle][2idle] Task:done(): setfsm();
//           if(++maincount) post(main2idle); wait(task2idle);
//       [2idle][ idle] Main:wait(): setfsm()
//       -or-
//       [2idle][2busy] (initial, Task waiting, Main:post() posted, running)
//       [2idle][2busy] Main:post(): setfsm();
//       [2idle][ busy] Main:post(): return
//       -or-
//       [2idle][ busy] (initial, Task waiting, Main running)
//       [2idle][ busy] Task:done(): setfsm();
//           if(++taskcount) post(main2idle); wait(task2idle);
//       [2idle][ idle] Main:wait(): setfsm(); wait(main2idle); setfsm();
//       -VV-
//       [2idle][ idle] Main:wait():
//           reset(maincount, main2busy, main2idle);
//           reset(taskcount, task2busy, task2done);
//           post(task2idle); wait(task2done); return;
//       [ idle][ idle] Task:done(): setfsm();
//           if(++taskcount) reset(task_count, task2idle); post(task2done)
//           else wait(task2done)
//           wait(task2busy)
//       -VV-
//       [2busy][ idle] Main:post(): setfsm(); post(task2busy); wait(main2busy)
//       [ busy][2busy] Task:done(): setfsm();
//           if(++taskcount) post(main2busy); return
//       [ busy][ busy] Main:post(): setfsm(); return
//
//----------------------------------------------------------------------------
#ifndef MAINTASK_H_INCLUDED
#define MAINTASK_H_INCLUDED

//----------------------------------------------------------------------------
// Finite states enumberate (and FSM_NAME[] array defined)
//----------------------------------------------------------------------------
enum FSM                            // Finite State Machine State
{  FSM_BUSY                         // Running, BUSY
,  FSM_IDLE                         // Running, IDLE
,  FSM_INTO_BUSY                    // Waiting for BUSY
,  FSM_INTO_IDLE                    // Waiting for IDLE
,  FSM_COUNT                        // Number of states
}; // enum FSM

static const char*     FSM_NAME[]=  // FSM state names
{  "BUSY"
,  "IDLE"
,  "INTO_BUSY"
,  "INTO_IDLE"
}; // FSM_NAME[]

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
enum // constants
{  GIGA_VALUE= 1'000'000'000        // Giga value, 1 billion
,  MEGA_VALUE= 1'000'000            // Mega value, 1 million
}; // constants

static size_t           PAGE_MASK= ~4095; // Page mask ~(PAGE_SIZE - 1)
static size_t           PAGE_SIZE= 4096; // Page size (using sysconf)

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define IFVERBOSE(n, stmt) if( opt_verbose >= n ) { stmt }

//----------------------------------------------------------------------------
// Module options: (Defaults enumerated in .h, options initialized in .cpp)
//----------------------------------------------------------------------------
static size_t          opt_iterations= ITERATIONS; // arg[0] Iteration count
static int             opt_multi= TASK_COUNT; // arg[1] Task count

static int             opt_hcdm= false; // --hcdm (Hard Core Debug Mode)
static int             opt_first= false; // --first (task disables trace)
static uint32_t        opt_trace= TRACE_SIZE; // --trace (table size) argument
static int             opt_verbose= -1; // --verbose

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
class Task;  // Forward reference for task_array declaration

static Task**          task_array= nullptr; // The allocated Task table
static void*           trace_table= nullptr; // The allocated trace table

//----------------------------------------------------------------------------
// Main/Task synchronization controls
//----------------------------------------------------------------------------
typedef std::atomic<uint32_t> ATOMIC32_t;
static int             main_fsm= FSM_BUSY; // Main FSM

////////////////////////////////////// Task controls
static ATOMIC32_t      task_count(0); // Number of IDLE Tasks completed
static pub::Event      task_2busy;  // Posted in Main, Tasks => busy
static pub::Event      task_2idle;  // Posted in Main, Tasks => idle
static pub::Event      task_2done;  // Posted in Task, (all Tasks idle)

////////////////////////////////////// Main controls
static ATOMIC32_t      main_count(0); // Number of BUSY Tasks completed
static pub::Event      main_2idle;  // Posted in Task, Main => idle

//----------------------------------------------------------------------------
// Exceptions
//----------------------------------------------------------------------------
class should_not_occur : public std::exception {
public:
virtual
   ~should_not_occur() noexcept {}
   should_not_occur() = default;

virtual const char*
   what() const noexcept { return "should_not_occur"; }
}; // class should_not_occur

//----------------------------------------------------------------------------
// Initializer
//----------------------------------------------------------------------------
static struct MainTask_initializer { // Static constructor
   MainTask_initializer( void )     // Constructor
{
   PAGE_SIZE= sysconf(_SC_PAGE_SIZE);
   PAGE_MASK= ~(PAGE_SIZE - 1);
}
}  static_initializer; // struct MainTask_Initializer

//----------------------------------------------------------------------------
// debug_event: Display pub::Event status
//----------------------------------------------------------------------------
static inline void
   debug_event(                     // Display status of
     pub::Event&       event,       // This event, which has
     const char*       name)        // This event name
{
   const char* status= "wait";      // Default, wait status
   if( event.test() )               // If posted
     status= "post";

   debugh("%s %s\n", name, status);
}

//----------------------------------------------------------------------------
// epoch_nano: (Return current time in nanoseconds since epoch)
//----------------------------------------------------------------------------
static uint64_t                     // Time since epoch (manoseconds)
   epoch_nano( void )               // Get current time (nanoseconds)
{
   struct timespec     time;        // UTC time base

   clock_gettime(CLOCK_REALTIME, &time); // Get UTC time base
   uint64_t nano= time.tv_sec * 1000000000;
   nano += time.tv_nsec;

   return nano;
}

//----------------------------------------------------------------------------
// epoch_secs: (Return current time in seconds since epoch)
//----------------------------------------------------------------------------
static inline double                // Time since epoch (seconds)
   epoch_secs( void )               // Get current time (seconds)
{  return (double)epoch_nano() / 1000000000.0; }

//----------------------------------------------------------------------------
// vtos: Convert void* to size_t
//----------------------------------------------------------------------------
static inline size_t                // The associated size_t
   vtos(                            // Get associated size_t
     void*             addr)        // For this address
{  return size_t(uintptr_t(addr)); }

//----------------------------------------------------------------------------
//
// Class-
//       Task
//
// Purpose-
//       Task used to drive tests.
//
//----------------------------------------------------------------------------
class Task : public pub::Named, public pub::Thread { // Task driver
//----------------------------------------------------------------------------
// Task::Attributes
//----------------------------------------------------------------------------
public:
uint16_t               fsm;         // Finite State Machine
char                   ident[6];    // The Task identifier
size_t                 iteration;   // Our iteration number
uint64_t               time;        // Task run time, in nanoseconds

//----------------------------------------------------------------------------
// Task::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Task( void ) = default;         // Default destructor
   Task(                            // Constructor
     const char*       ident)       // The Task identifier
:  pub::Named(ident), pub::Thread()
,  fsm(FSM_BUSY)
{
   strcpy(this->ident, ident);      // Set the Task identifier
   start();                         // Auto-start the Task
}

//----------------------------------------------------------------------------
// Task::Methods
//----------------------------------------------------------------------------
public:
void
   debug(                           // Debugging display
     int               line)        // Caller's line number
{
   debugh("%4d Task(%s)::debug() Task[%s] Main[%s]\n\t\t    %zd of %zd\n"
          , line, ident, FSM_NAME[fsm], FSM_NAME[main_fsm]
          , iteration, opt_iterations);
}

void
   done(                            // Task completed
     int               line)        // Caller's line number
{  if( HCDM )
     debugh("%4d Task(%s).done() Task[%s] Main[%s]>>>>>>>>>>>>>>\n"
            , line, ident, FSM_NAME[fsm], FSM_NAME[main_fsm]);

   uint32_t            count;       // Working counter

   // State: Task[busy]; Main[into_idle]
   count= ++main_count;
   if( HCDM ) debugh("%4d HCDM.m:done %u/%u\n", __LINE__, count, opt_multi);
   if( count >= opt_multi ) {       // If all Tasks are complete
     if( HCDM ) debugh("%4d HCDM.m:done post(main2idle)\n", __LINE__);
     main_2idle.post();
   }

   set_fsm(__LINE__, FSM_INTO_IDLE);
   task_2idle.wait();
   set_fsm(__LINE__, FSM_IDLE);

   // State: Task[idle...]; Main[idle]
   count= ++task_count;
   if( HCDM ) debugh("%4d HCDM.m:done %u/%u\n", __LINE__, count, opt_multi);
   if( count >= opt_multi ) {       // If all Tasks are idle
     if( HCDM ) debugh("%4d HCDM.m:done post(task2done)\n", __LINE__);
     task_count= 0;
     task_2idle.reset();
     task_2done.post();
   }
   if( HCDM ) debugh("%4d HCDM.m:done wait(task2done)\n", __LINE__);
   task_2done.wait();
   // State: Task[idle]; Main[idle]

   set_fsm(__LINE__, FSM_INTO_BUSY);
   task_2busy.wait();
   set_fsm(__LINE__, FSM_BUSY);

   // State: Task[busy], Main[busy]
   if( HCDM )
     debugh("%4d Task(%s).done() Task[%s] Main[%s]<<<<<<<<<<<<<<\n"
            , line, ident, FSM_NAME[fsm], FSM_NAME[main_fsm]);
}

virtual void
   run( void )                      // Run the Task
{  if( opt_verbose >= 3 )           // Task to ident correlation
     tracef("%14.3f <@%.12zX> Task(%s)::run()\n", epoch_secs()
            , vtos(this), ident);

   test_prefix();                   // Pre-test callback

   if( HCDM ) traceh("%4d HCDM.m\n", __LINE__);
   done(__LINE__);                  // Wait for all Tasks to start
   if( HCDM ) debugh("%4d HCDM.m\n", __LINE__);

   //-------------------------------------------------------------------------
   // Run the test
   time= epoch_nano();              // Start time
   TRY_CATCH(
     try {
       test();                      // Run the test
     } catch(...) {
       unsigned size= sizeof(Trace::Record) + 16;
       Trace::Record* record= (Trace::Record*)Trace::storage_if(size);
       Trace::trace->deactivate();  // Terminate testing
       if( record ) {               // (Trace termination trace entry.)
         memset(record, 0, size);
         strcpy((char*)record + sizeof(Trace::Record), "Exception");
         record->init(ident, __LINE__);
       }

       opt_verbose= 5;              // (Force trace table dump)
       throw;                       // Rethrow the exception
     }
   )

   time= epoch_nano() - time;       // Elapsed time

   //-------------------------------------------------------------------------
   // Test complete
   if( opt_first) {
     // Option: the first Task completion deactivates tracing
     // Note: It's possible for multiple Tasks to finish "simultaneously"
     Trace::Record* record= (Trace::Record*)Trace::storage_if(sizeof(Trace));
     Trace::trace->deactivate();    // Deactivate the Trace
     if( record ) {                 // (Trace termination trace entry.)
       memset(record, 0, sizeof(Trace));
       record->init(".HLT", __LINE__);
     }
   }

   if( HCDM ) debugh("%4d HCDM.m\n", __LINE__); // MUST follow deactivate
   test_suffix();                   // Post-test callback

   // Tell Main that we're done
   done(__LINE__);
}

void
   set_fsm(                         // Update the state
     int               line,        // From this line number
     int               fsm)         // To this new state
{  if( HCDM )
     debugh("%4d Task(%s).set_fsm(%s=>%s)\n", line, ident
            , FSM_NAME[this->fsm], FSM_NAME[fsm]);

   this->fsm= fsm;
}

virtual void                        // (( OVERRIDE THIS METHOD ))
   test( void )                     // Run the test
{  debugf("Task(%s)::test() !!UNDEFINED!!\n", ident); }

virtual void                        // (( OPTIONALLY: OVERRIDE THIS METHOD ))
   test_prefix( void )              // Pre-test callback
{  }                                // Optional, serialization allowed

virtual void                        // (( OPTIONALLY: OVERRIDE THIS METHOD ))
   test_suffix( void )              // Post-test callback
{  }                                // Optional, serialization allowed
}; // class Task

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Common static functions.
//
//----------------------------------------------------------------------------
class Main {                        // Static functions
public:
//----------------------------------------------------------------------------
// Main::debug (Global debugging display)
//----------------------------------------------------------------------------
static void
   debug(                           // Global debugging display
     int               line)        // Caller's line number
{
   int zero_fsm= task_array[0]->fsm;
   const char* task_fsm= FSM_NAME[zero_fsm];
   for(int i=1; i<opt_multi; i++) {
     if( task_array[i]->fsm != zero_fsm ) { // If mixed state
       task_fsm= "*MIX*";
       break;
     }
   }

   debugh("%4d Main::debug() Main[%s] Task[%s]\n"
          , line, FSM_NAME[main_fsm], task_fsm);
   debugh("..task_count(%d) main_count(%d) opt_multi(%u)\n"
          , task_count.load(), main_count.load(), opt_multi);
   debug_event(task_2busy, "task_2busy");
   debug_event(task_2idle, "task_2idle");
   debug_event(task_2done, "task_2done");
   debug_event(main_2idle, "main_2idle");

   debugh("task_array(%p).%u\n", task_array, opt_multi);
   for(int i= 0; i<opt_multi; i++) {
     task_array[i]->debug(__LINE__);
   }

   if( opt_verbose >= 5 ) {         // (Optionally) dump trace table
#if true
     debugh("Trace::trace(%p)->dump() (See debug.out)\n", Trace::trace);
     Trace::trace->dump();
#else
     debugh("trace_table(%p).%u (See debug.out)\n", trace_table, opt_trace);
     Debug* debug= Debug::get();
     pub::utility::dump(debug->get_FILE(), trace_table, opt_trace, nullptr);
#endif
     if( opt_hcdm ) debug_flush();  // (Force log completion)
   }
}

//----------------------------------------------------------------------------
// Main::init (Initialization)
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{  if( HCDM ) debugh("%4d Main::init\n", __LINE__);

   //-------------------------------------------------------------------------
   // Allocate the trace table
   trace_table= malloc(opt_trace);  // Allocate the trace table
   if( trace_table == nullptr ) throw std::bad_alloc();
   Trace::trace= Trace::make(trace_table, opt_trace); // Initialize the table

   //-------------------------------------------------------------------------
   // Allocate and initialize the task_array
// main_fsm= FSM_BUSY;              // (Not needed, it's the initial value)

   task_array= (Task**)malloc(sizeof(Task*) * opt_multi);
   if( task_array == nullptr ) throw std::bad_alloc();
   for(int i= 0; i<opt_multi; i++)  // Initialize, empty task_array
     task_array[i]= nullptr;

   for(int i= 0; i<opt_multi; i++) {
     char buffer[16];
     sprintf(buffer, ".%.3d", i);
     task_array[i]= make(buffer);   // Main::make(), creates Task superclass
   }
}

//----------------------------------------------------------------------------
// Main::set_fsm (Update fsm)
//----------------------------------------------------------------------------
static void
   set_fsm(                         // Update the state
     int               line,        // From this line number
     int               fsm)         // To this new state
{  if( HCDM )
     debugh("%4d Main::set_fsm(%s=>%s)\n", line,
            FSM_NAME[main_fsm], FSM_NAME[main_fsm]);

   main_fsm= fsm;
}

//----------------------------------------------------------------------------
// Main::term (Termination)
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{  if( HCDM ) debugh("%4d Main::term\n", __LINE__);

   for(int i= 0; i<opt_multi; i++) { // Join all Tasks
     task_array[i]->join();
   }

   for(int i= 0; i<opt_multi; i++) { // Delete each Task
     delete task_array[i];
   }
   task_array= nullptr;             // All Tasks deleted

   //-------------------------------------------------------------------------
   // Free the trace table
   Trace::trace= nullptr;           // Disable tracing
   free(trace_table);
   trace_table= nullptr;
}

//----------------------------------------------------------------------------
// Main::post (Start all Tasks)
//----------------------------------------------------------------------------
static void
   post(                            // From Main, start all Tasks
     int               line)        // Caller's line number
{  if( HCDM )
     debugh("%4d Main::post() Main[%s]>>>>>>>>>>>>>>\n", line
            , FSM_NAME[main_fsm]);

   // State: Main[idle], Task[into_busy,...]
   set_fsm(__LINE__, FSM_BUSY);     // Change the FSM first
   task_2busy.post();

   // State: Main[busy], Task[busy]
   if( HCDM )
     debugh("%4d Main::post() Main[%s]<<<<<<<<<<<<<<\n", line, FSM_NAME[main_fsm]);
}

//----------------------------------------------------------------------------
// Main::wait (Wait for all Tasks)
//----------------------------------------------------------------------------
static void
   wait(                            // From Main, wait for all Tasks
     int               line)        // Caller's line number
{  if( HCDM )
     debugh("%4d Main::wait() Main[%s]>>>>>>>>>>>>>>\n", line
            , FSM_NAME[main_fsm]);

   // State: Main[busy], Task[busy,...]
   set_fsm(__LINE__, FSM_INTO_IDLE);
   main_2idle.wait();
   set_fsm(__LINE__, FSM_IDLE);

   // State: Main[idle], Task[into_idle]
   main_count= 0;
   main_2idle.reset();

   task_count= 0;
   task_2busy.reset();
   task_2done.reset();
   task_2idle.post();

   if( HCDM ) debugh("%4d HCDM.m wait(task2done)\n", __LINE__);
   task_2done.wait();

   // State: Main[idle], Task[idle]
   if( HCDM )
     debugh("%4d Main::wait() Main[%s]<<<<<<<<<<<<<<\n", line
            , FSM_NAME[main_fsm]);
}

//======== CALLBACK ==========================================================
// Main::make (Normally implmented in Module.h, called from Main::init)
//   Normally: Task* Main::make(const char* id) { return new Thread(id); }
//----------------------------------------------------------------------------
static Task*                        // The Task, implementing test()
   make(const char*    ident);      // Create specialised Task

//----------------------------------------------------------------------------
// Main::stats (Normally implmented in and only called from Module.h)
//   Statistics display
//----------------------------------------------------------------------------
static void
   stats( void );                   // Display statistics
}; // class Main
#endif // MAINTASK_H_INCLUDED
