//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       T_Stream.hpp
//
// Purpose-
//       T_Stream.cpp classes
//
// Last change date-
//       2024/03/04
//
//----------------------------------------------------------------------------
#ifndef T_STREAM_HPP_INCLUDED
#define T_STREAM_HPP_INCLUDED

//----------------------------------------------------------------------------
//
// Subroutine-
//       i2v
//
// Purpose-
//       Integer to void, shorthand for (void*)(intptr_t(i))
//
//----------------------------------------------------------------------------
static void*                        // The void*
   i2v(                             // Convert intptr_t to void*
     intptr_t          i)           // The intptr_t
{  return (void*)i; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       do_JOIN
//
// Purpose-
//       Join strings
//
//----------------------------------------------------------------------------
static string                       // The JOINed string
   do_JOIN(                         // Join a string list
     const char**      args,        // The string array
     string            ins= "")     // The insertion string
{
   string out;
   for(size_t i= 0; args[i]; ++i) {
     string line= args[i];
     size_t x= line.find("{}");
     while( x != string::npos ) {
       line= line.substr(0, x) + ins + line.substr(x+2);
       x= line.find("{}");
     }
     out += line;
     out += "\r\n";
   }

   return out;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       page200
//       page403
//       page404
//       page405
//       page500
//
// Purpose-
//       Generate response message
//
//----------------------------------------------------------------------------
static string                       // The 200 Dummy File message
   page200(string body)
{  static const char* args[]=
               { "<html><head><title>PAGE 200</title></head>"
               , "<body><h1 align=\"center\">Default Response Page</h1>"
               , "Body[{}]"
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, body);
}

static string                       // The 403 forbidden message
   page403(string file)
{  static const char* args[]=
               { "<html><head><title>FORBIDDEN</title></head>"
               , "<body><h1 align=\"center\">FORBIDDEN</h1>"
               , "File[{}] access forbidden."
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, file);
}

static string                       // The 404 file not found message
   page404(string file)
{  static const char* args[]=
               { "<html><head><title>FILE NOT FOUND</title></head>"
               , "<body><h1 align=\"center\">FILE NOT FOUND</h1>"
               , "File[{}] not found."
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, file);
}

static string                       // The 405 method not supported message
   page405(string meth)
{  static const char* args[]=
               { "<html><head><title>METHOD NOT ALLOWED</title></head>"
               , "<body><h1 align=\"center\">METHOD NOT ALLOWED</h1>"
               , "Method[{}] is not supported."
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, meth);
}

static string                       // The 500 server error message
   page500(string info)
{  static const char* args[]=
               { "<html><head><title>SERVER ERROR</title></head>"
               , "<body><h1 align=\"center\">SERVER ERROR</h1>"
               , "[{}]"
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, info);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       logger
//       log_request
//
// Purpose-
//       Write message to log
//       Log a request/response
//
//----------------------------------------------------------------------------
static void
   logger(string mess)              // Write message to log
{
   debugh("\n%s\n", mess.c_str());  // (For now)
}

static void
   log_request(Request& Q, Response& S) // Log a request/response
{
   if( USE_LOGGER ) {
     string mess=
     utility::to_string("{peer} [{time}] {http} %3d %s %s {}"
                       , S.get_code(), Q.method.c_str(), Q.path.c_str());
     logger(mess);
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       TimerThread
//
// Purpose-
//       Background Thread that sets and clears `running`
//
//----------------------------------------------------------------------------
class TimerThread : public Thread {
public:
   ~TimerThread( void ) = default;
   TimerThread( void ) = default;

virtual void
   run( void )
{
   running= true;
   test_start.post();

   Thread::sleep(opt_runtime);      // (Run the test)

   running= false;
   test_start.reset();
}
}; // class TimerThread

//----------------------------------------------------------------------------
//
// Class-
//       ClientThread
//
// Purpose-
//       The T_Stream client Thread.
//
// Implementation notes-
//       Although each client thread operates asynchronously and independently,
//       the ClientThread simplifies initialization.
//
//----------------------------------------------------------------------------
class ClientThread : public Named, public Thread { // The client Thread
public:
std::shared_ptr<Client>client;      // The Client

std::atomic_size_t     cur_op_count= 0; // The number of running requests
std::mutex             mutex;       // Protects client

Event                  ready;       // Thread ready event
Event                  send_end;    // Send completion event (for run_one)

static std::atomic_int client_serial; // Global serial number
int                    serial= -1;  // Serial number

// Callback handlers
std::function<void(void)>
                       do_NEXT;     // The next completion handler

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::ClientThread
//       ClientThread::~ClientThread
//
// Function-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   ClientThread( void )
:  Named("ClientThread"), Thread()
,  serial(client_serial++)
{  INS_DEBUG_OBJ("ClientThread"); }

   ~ClientThread( void )
{  REM_DEBUG_OBJ("ClientThread"); }

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   debug(const char* info= "")      // Debugging display
{  debugf("ClientThread(%p)::debug(%s)\n", this, info);

   debugf("..[%d] cur_op_count(%'zd)\n", serial, cur_op_count.load());
   debugf("..ready(%d) send_end(%d)\n"
         , ready.is_post(), send_end.is_post());
   if( client ) client->debug("ClientThread");
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::close
//
// Purpose-
//       Close the Client
//
//----------------------------------------------------------------------------
void
   close( void )                    // Close the Client
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] ClientThread::close\n", serial);

   std::lock_guard<decltype(mutex)> lock(mutex);
   if( client ) {
     client->close();
     if( opt_minor > 0 )
       client->wait();
     client= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::do_POST
//
// Function-
//       Create/write a POST request
//
//----------------------------------------------------------------------------
void
   do_POST(                         // Create a POST request
     std::string       path,        // The URL
     std::string       data)        // The POST data
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] do_POST(%s,%s)\n", serial, path.c_str(), data.c_str());

   std::shared_ptr<ClientStream> stream= client->make_stream();
   error_count += VERIFY( stream.get() != nullptr);
   if( stream.get() == nullptr )
     return;

   std::shared_ptr<ClientRequest> Q= stream->get_request();
   Q->method= HTTP_POST;
   Q->path= path;

   std::shared_ptr<ClientResponse> S= stream->get_response();
   do_RESP(S);

   if( opt_iodm ) {
     debugf("do_POST(%s,%s)\n", path.c_str(), data.c_str());
     Options& opts= Q->get_opts();
     for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
       debugf("%s: %s\n", it->first.c_str(), it->second.c_str());
   }

   Q->write(data.c_str(), data.size()); // Write the POST data
   Q->write();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::do_RESP
//
// Function-
//       Handle a response (Define response handlers)
//
//----------------------------------------------------------------------------
void
   do_RESP(                         // Handle
     std::shared_ptr<Response> S)   // This response
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] do_RESP(%p)\n", serial, S.get());

   // Google: "c++ shared_ptr lambda function" to see why a weak_ptr MUST be
   // captured instead of a shared_ptr.
   std::weak_ptr<Response> weak= S; // (Passed to lambda functions)

   S->on_error([this, weak](const std::string& mess) {
     std::shared_ptr<Response> L= weak.lock();
     if( opt_hcdm && opt_verbose )
       debugh("[%2d] on_error(%s) Response(%p)\n", serial
             , mess.c_str(), L.get());
     if( L ) {
       std::shared_ptr<Request> Q= L->get_request();
       debugh("Request(%p) %s %s error %s\n", Q.get(), Q->method.c_str()
             , Q->path.c_str(), mess.c_str());
     }
   });

   S->on_ioda([this, weak](Ioda& data) {
     std::shared_ptr<Response> L= weak.lock();
     if( opt_hcdm && opt_verbose )
       debugh("[%2d] on_ioda(%p) Response(%p)\n", serial, &data, L.get());
     if( L ) {
       Ioda& ioda= L->get_ioda();
       if( ioda.get_used() <= MAX_RESPONSE_SIZE )
         ioda += std::move(data);
     }
   });

   S->on_end([this, weak]( void ) {
     std::shared_ptr<Response> L= weak.lock();
     if( opt_hcdm && opt_verbose )
       debugh("[%2d] on_end Response(%p)\n", serial, L.get());
     if( L ) {
       std::shared_ptr<Request> Q= L->get_request();
       if( opt_iodm ) {
         debugh("Response code %d\n", L->get_code());
         Options& opts= (Options&)*L.get();
         for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
            debugf("%s: %s\n", it->first.c_str(), it->second.c_str());
       }
       if( L->get_code() == 200 ) {
         if( opt_verify && Q->method == HTTP_GET ) {
           std::string path= Q->path;
           if( path == "/" )
             path= "/index.html";
           std::string have_string= (std::string)L->get_ioda();
           std::string want_string= page200(path);
           if( want_string != have_string ) {
             ++error_count;
             have_string= visify(have_string);
             want_string= visify(want_string);
             debugh("%4d %s Data verify error:\n", __LINE__, __FILE__);
             debugh("Have '%s'\n", have_string.c_str());
             debugh("Want '%s'\n", want_string.c_str());
           }
         }

         if( opt_iodm ) {
           Ioda& ioda= L->get_ioda();
           std::string data_string= (std::string)ioda;
           if( ioda.get_used() > MAX_RESPONSE_SIZE )
             data_string=
             utility::to_string("<<Response data error: length(%ld) > %d>>"
                               , ioda.get_used(), MAX_RESPONSE_SIZE);
           data_string= visify(data_string);
           debugh("Data: \n%s\n", data_string.c_str());
         }
       }
     }
   });
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::do_SEND
//
// Function-
//       Create/write a request (method "GET" or "HEAD")
//
//----------------------------------------------------------------------------
void
   do_SEND(                         // Create a request
     std::string       meth,        // The Request method
     std::string       path)        // The URL
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] do_SEND(%s,%s)\n", serial, meth.c_str(), path.c_str());

   ++cur_op_count;

   std::shared_ptr<ClientStream> stream= client->make_stream();
   error_count += VERIFY( stream.get() != nullptr);
   if( stream.get() == nullptr ) {
     send_end.post();
     return;
   }

   std::shared_ptr<ClientRequest> Q= stream->get_request();
   Q->method= meth;
   Q->path= path;

   std::shared_ptr<ClientResponse> S= stream->get_response();
   do_RESP(S);

   Q->on_end([this]() {
     if( opt_hcdm && opt_verbose )
       traceh("Q.on_end current(%zd) total(%zd) running(%d)\n"
             , cur_op_count.load(), send_op_count.load(), running);
     if( running ) {                // (Only count running send completions)
       size_t test_op_count= ++send_op_count;
       if( USE_REPORT && USE_REPORT_ITERATION > 2
           && (test_op_count % USE_REPORT_ITERATION) == 0 ) {
         debugf("\n\n");
         Reporter::get()->report([](Reporter::Record& record) {
           debugf("%s\n", record.h_report().c_str());
         }); // reporter.report
       }
     }

     --cur_op_count;
     do_NEXT();
   });

   if( opt_iodm ) {
     debugh("do_SEND(%s,%s)\n", meth.c_str(), path.c_str());
//   (Options not set)
//   Options& opts= (Options&)*Q.get();
//   for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
//      debugf("%s: %s\n", it->first.c_str(), it->second.c_str());
   }

   Q->write();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::get_client
//
// Purpose-
//       Activate the Client
//
//----------------------------------------------------------------------------
void
   get_client( void )               // Activate the client
{
// Options opts;                    // TODO: Options TBD
   client= client_agent->connect(host + port); // Create the client

   if( !client ) {
     debugf("Unable to connect %s%s\n", host.c_str(), port.c_str());
     exit(EXIT_FAILURE);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run_one
//
// Purpose-
//       Process a complete client connection.
//
//----------------------------------------------------------------------------
virtual void
   run_one( void )                  // Process a complete operation
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] ClientThread::run_one...\n", serial);

   // Initialize
   send_end.reset();
   do_NEXT= [this](void) {
     if( !send_end.is_post() )
       send_end.post();
   };

   try {                            // Run the stress test
     get_client();
     do_SEND(HTTP_GET, test_url);
     send_end.wait();
     close();
   } catch(Exception& X) {
     ++error_count;
     debugf("%4d Exception: %s\n", __LINE__, X.to_string().c_str());
   } catch(std::exception& X) {
     ++error_count;
     debugf("%4d std::Exception what(%s)\n", __LINE__, X.what());
   }

   if( opt_hcdm && opt_verbose )
     debugf("...[%2d] ClientThread.run_one\n", serial);
   if( USE_ITRACE )
     Trace::trace(".RUN", "_one", this, i2v(serial));
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run
//
// Purpose-
//       Operate the client Thread stress test
//
//----------------------------------------------------------------------------
virtual void
   run( void )                      // Operate the client Thread
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] ClientThread::run...\n", serial);

// debugf("%4d %s HCDM\n", __LINE__, __FILE__);
   //-------------------------------------------------------------------------
   // Client per connection version - in test -- -- -- -- -- -- -- -- -- -- --
   if( opt_major > 0 ) {            // Use run_one()
     ready.post();                  // Indicate ready
     test_start.wait();             // Wait for start signal

     try {
       while( running && error_count == 0 )
         run_one();
     } catch(Exception& X) {
       ++error_count;
       debugf("%4d Exception: %s\n", __LINE__, X.to_string().c_str());
     } catch(std::exception& X) {
       ++error_count;
       debugf("%4d std::Exception what(%s)\n", __LINE__, X.what());
     } catch(...) {
       ++error_count;
       debugf("%4d catch(...)\n", __LINE__);
     }

     ready.reset();                 // Not ready

     return;
   }

   //-------------------------------------------------------------------------
   // Single client version (DEFAULT)
   get_client();
   do_NEXT= [this](void) {
     if( opt_hcdm && opt_verbose )
       debugh("[%2d] do_NEXT current(%zd) total(%zd)\n", serial
             , cur_op_count.load(), send_op_count.load());

     // debugh("stress.next... %s\n", running ? "running" : "quiesced");
     statistic::Active* stat= &Request::obj_count;
     while( running && cur_op_count.load() < MAX_REQUEST_COUNT ) {
       if( opt_hcdm && opt_verbose ) // Use detailed tracking?
         debugh("%4ld {%2ld,%2ld,%2ld} cur_op_count %zd\n"
                , stat->counter.load(), stat->minimum.load()
                , stat->current.load(), stat->maximum.load()
                , cur_op_count.load());
       do_SEND(HTTP_GET, test_url);
     }

     if( opt_hcdm && opt_verbose ) // Use detailed tracking?
       debugh("%4ld {%2ld,%2ld,%2ld} %srunning, cur_op_count %zd\n"
              , stat->counter.load(), stat->minimum.load()
              , stat->current.load(), stat->maximum.load()
              , running ? "" : "NOT "
              , cur_op_count.load());
   };

   ready.post();                    // Indicate ready
   test_start.wait();               // Wait for start signal

   try {                            // Run the stress test
     do_NEXT();                     // Prime the pump
     test_ended.wait();             // Wait for the test to complete
   } catch(Exception& X) {
     ++error_count;
     debugf("%4d Exception: %s\n", __LINE__, X.to_string().c_str());
   } catch(std::exception& X) {
     ++error_count;
     debugf("%4d std::Exception what(%s)\n", __LINE__, X.what());
   } catch(...) {
     ++error_count;
     debugf("%4d catch(...)\n", __LINE__);
   }

   client->wait();                  // Wait for operations to complete
   ready.reset();                   // Not ready

   if( opt_hcdm && opt_verbose )
     debugf("...[%2d] ClientThread.run\n", serial);

   if( USE_ITRACE )
     Trace::trace(".TXT", __LINE__, "CT.run exit");
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::start
//
// Function-
//       Start the ClientThread.
//
//----------------------------------------------------------------------------
virtual void
   start( void )                    // Start the ClientThread
{
   ready.reset();
   Thread::start();
   ready.wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::statistics
//
// Purpose-
//       Statistics display
//
//----------------------------------------------------------------------------
static void
   statistics( void )               // Display statistics
{
   statistic::Active* stat= nullptr;

   // Delay, allowing threads to complete
   Thread::sleep(0.25);

   // Verify the object counters
   error_count += VERIFY( Stream::obj_count.current.load() == 0 );
   error_count += VERIFY( Request::obj_count.current.load() == 0 );
   error_count += VERIFY( Response::obj_count.current.load() == 0 );

   // Display the object counters
   if( opt_verbose > 1 || error_count ) {
     debugf("\n");
     debugf("           Total {   Cur,    Min,    Max}: Description\n");
     stat= &Stream::obj_count;
     debugf("%'16ld {%'6ld, %'6ld, %'6ld}: Stream counts\n"
           , stat->counter.load(), stat->current.load()
           , stat->minimum.load(), stat->maximum.load());

     stat= &Request::obj_count;
     debugf("%'16ld {%'6ld, %'6ld, %'6ld}: Request counts\n"
           , stat->counter.load(), stat->current.load()
           , stat->minimum.load(), stat->maximum.load());

     stat= &Response::obj_count;
     debugf("%'16ld {%'6ld, %'6ld, %'6ld}: Response counts\n"
           , stat->counter.load(), stat->current.load()
           , stat->minimum.load(), stat->maximum.load());
   }

   // Display Reporter records
   if( opt_verbose > 1 || error_count ) {
     Reporter::get()->report([](Reporter::Record& record) {
       debugf("%s\n", record.h_report().c_str());
     }); // reporter.report
   }

   // Display WorkerPool statistics
   if( opt_verbose > 1 ) {
     debugf("\n");
     WorkerPool::debug();
   }

   // Reset the statistics
   stat= &Stream::obj_count;
   stat->counter.store(0);
   stat->minimum.store(0);
   stat->current.store(0);
   stat->maximum.store(0);

   stat= &Request::obj_count;
   stat->counter.store(0);
   stat->minimum.store(0);
   stat->current.store(0);
   stat->maximum.store(0);

   stat= &Response::obj_count;
   stat->counter.store(0);
   stat->minimum.store(0);
   stat->current.store(0);
   stat->maximum.store(0);

   Reporter::get()->reset();
   WorkerPool::reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::test_client
//
// Purpose-
//       Client functional test
//
//----------------------------------------------------------------------------
static void
   test_client( void )              // Client functional test
{  debugf("\nClientThread.test_client...\n");

// debugf("%4d %s HCDM\n", __LINE__, __FILE__);
   error_count= 0;

   ClientThread client;
   client.do_NEXT= [](void) {
     if( opt_hcdm && opt_verbose ) debugf("test_client.do_NEXT NOP\n");
   };
   client.get_client();

   // Bringup tests
   client.do_SEND(HTTP_GET, "/");
   client.do_SEND(HTTP_HEAD, "/index.htm");

   client.do_POST("/post-test", "This is the post data, all of it.");

   client.do_SEND(HTTP_GET, "/403-test");
   client.do_SEND(HTTP_GET, "/404-test");
   client.do_SEND("MOVE", "/405-test");

   client.do_SEND(HTTP_GET, "/tiny.html"); // Used in stress test
   client.do_SEND(HTTP_GET, "/utf8.html"); // Regression test

   // Error tests
#if 0  // TODO: CLIENT RECOVERY NEEDED FOR ERROR TESTS
   if( opt_verbose ) {
     client.wait();
     debugf("Malformed method...\n");
   }
   do_SEND(" GET", "/");            // Malformed method
   if( opt_verbose ) {
     client.wait();
     debugf("Malformed path...\n");
   }
   client.do_SEND("GET", " /");     // Malformed path
// client.do_SEND( TODO: CODE );    // Malformed protocol
#endif

   client.do_SEND(HTTP_GET, "/last.html"); // The last request
   client.wait();                   // Wait for Client to complete

   Trace::trace(".TXT", __LINE__, "TC.client close");
   client.close();                  // Close the ClientThread

   client_agent->stop();
   client_agent->reset();
   listen_agent->stop();
   listen_agent->reset();

   debugf("...ClientThread.test_client\n");
   Trace::trace(".TXT", __LINE__, "TC.client exit");
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::test_stress
//
// Purpose-
//       Stress test
//
//----------------------------------------------------------------------------
static void
   test_stress( void )              // Stress test
{  debugf("\nClientThread.test_stress... (%.1f seconds)\n", opt_runtime);

// debugf("%4d %s HCDM\n", __LINE__, __FILE__);
   error_count= 0;

   //-------------------------------------------------------------------------
   // Client per connection version - BRINGUP -- -- -- -- -- -- -- -- -- -- --
   if( opt_major > 1 ) {            // run_one() bringup
     // Run the the client per connection test opt_stress times
     ClientThread ct;

     running= true;
     opt_runtime= Clock::now();
     for(int i= 0; i<opt_stress; ++i) {
       ct.run_one();
     }
     opt_runtime= Clock::now() - opt_runtime;
     running= false;

     double op_count= double(send_op_count.load());
     debugf("%'16.3f operations\n", op_count);
     debugf("%'16.3f milliseconds\n", opt_runtime * 1000.0);
     debugf("%'16.3f operations/second\n", op_count/opt_runtime);
//debugf("\n\n");
//std::pub_diag::Debug_ptr::debug("Test complete");

     client_agent->stop();
     client_agent->reset();
     listen_agent->stop();
     listen_agent->reset();
//debugf("\nClient/Listen reset complete ******************************\n\n\n");
//client_agent->debug("Test complete");
//listen_agent->debug("Test complete");
//debugf("\n\n");
//std::pub_diag::Debug_ptr::debug("Test complete");
//debugf("\nTest diagnostics complete *********************************\n\n\n");

     return;
   }

   //-------------------------------------------------------------------------
   // Client thread version (DEFAULT, run stress test for opt_runtime seconds)
   ClientThread* client[opt_stress];
   for(int i= 0; i<opt_stress; i++) {
     client[i]= new ClientThread();
     client[i]->start();
   }

   if( opt_verbose )
     debugh("--%s_stream test: Started\n", opt_ssl ? "ssl" : "std");
   test_ended.reset();
   TimerThread timer_thread;
   timer_thread.start();
   timer_thread.join();
   test_ended.post();

   if( opt_verbose || error_count )
     debugh("--%s_stream test: %s\n", opt_ssl ? "ssl" : "std"
           , error_count ? "FAILED" : "Complete");

   double op_count= double(send_op_count.load());
   debugf("%'16.3f operations\n", op_count);
   debugf("%'16.3f operations/second\n", op_count/opt_runtime);

   Trace::Record* R= Trace::trace(64);
   if( R ) {
     for(int i= 0; i<64; ++i)
       ((char*)R)[i]= (char)(i % 32);
     strcpy(R->value, ">>Stress test<<");
     R->trace(".END");
   }

   for(int i= 0; i<opt_stress; i++) {
     if( USE_ITRACE )
       Trace::trace(".TST", "WAIT", client[i], (void*)intptr_t(i));
     client[i]->wait();
   }

   client_agent->stop();
   client_agent->reset();
   listen_agent->stop();
   listen_agent->reset();

   for(int i= 0; i<opt_stress; i++) {
     client[i]->close();
     client[i]->join();
     delete client[i];
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::wait
//
// Purpose-
//       Wait for all outstanding requests to complete
//
//----------------------------------------------------------------------------
void
   wait( void )                     // Wait for outstanding request completion
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] wait ClientThread\n", serial);

   std::lock_guard<decltype(mutex)> lock(mutex);
   if( client )
     client->wait();
}
}; // class ClientThread

std::atomic_int        ClientThread::client_serial= 0; // Global serial number

//----------------------------------------------------------------------------
//
// Class-
//       ServerThread
//
// Purpose-
//       The T_Stream listener Thread
//
// Implementation notes-
//       The ServerThread is not a Thread. It's driven asynchronously.
//
//----------------------------------------------------------------------------
class ServerThread {                // The listener Thread
public:
std::shared_ptr<Listen>
                       listen;      // Our Listener

Event                  ready;       // Thread ready Event
Event                  ended;       // Thread ended Event

bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::ServerThread
//       ServerThread::~ServerThread
//
// Function-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
public:
   ServerThread( void )
{
   Options opts;                    // Server Options
   opts.insert("cert", cert_file);  // The public certificate file
   opts.insert("key",  priv_file);  // The private key file
   opts.insert("http1", "true");    // HTTP1 allowed

   listen= listen_agent->connect(port, AF_INET, &opts); // Create Listener
   if( listen.get() == nullptr ) {
     int ERRNO= errno;
     debugf("T_Stream: cannot connect port(%s) %d:%s\n"
           , port.substr(1, string::npos).c_str(), errno, strerror(ERRNO));
     throw std::runtime_error(strerror(ERRNO));
   }

   // Initialize the Listen handlers
   listen->on_close([this]( void ) {
     if( opt_hcdm && opt_verbose )
       debugf("ServerThread(%p)::on_close\n", this);
   });

   listen->on_request([this](ServerRequest& Q) {
     try {
       if( opt_iodm || (opt_hcdm && opt_verbose) )
         debugh("ServerThread(%p)::on_request(%s)\n", this
               , Q.method.c_str());
       if( Q.method == HTTP_GET || Q.method == HTTP_HEAD )
         do_HGET(Q);
       else if( Q.method == HTTP_POST )
         do_POST(Q);
       else
         do_HTML(Q, 405, page405(Q.method));
     } catch(Exception& X) {
       do_HTML(Q, 500, page500((std::string)X));
     } catch(std::exception& X) {
       string info("std::exception(");
       info += X.what();
       info += ")";
       do_HTML(Q, 500, page500(info));
     } catch(...) {
       do_HTML(Q, 500, "catch(...)");
     }
   });

   ended.reset();
   ready.post();
   operational= true;

   INS_DEBUG_OBJ("ServerThread");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~ServerThread( void )
{
   listen->on_close([]( void ) {});
   listen->on_request([](ServerRequest&) {});

   REM_DEBUG_OBJ("ServerThread");
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::do_FILE
//
// Function-
//       Read data file
//
//----------------------------------------------------------------------------
void
   do_FILE(ServerRequest& Q)
{
   if( opt_hcdm && opt_verbose )
     debugf("ServerThread(%p)::do_FILE(%s)\n", this, Q.path.c_str());

   string path= Q.path;
   if( path[0] != '/' || path.find("/../") != string::npos ) {
     do_HTML(Q, 500, page500("parser fault"));
     return;
   }

   // SCAFFOLDED <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   if( path == "/403-test" )
     do_HTML(Q, 403, page403(path));
   else if( path == "/404-test" )
     do_HTML(Q, 404, page404(path));
   else if( path == "/405-test" )
     do_HTML(Q, 405, page405(path));
   else if( path == "/500-test" )
     do_HTML(Q, 500, page500(path));
   else {
     if( path == "/" )
       path= "/index.html";
     path= "html" + path;

     do_HTML(Q, 200, page200(path));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::do_HGET
//
// Function-
//       Handle "GET" or "HEAD" request
//
//----------------------------------------------------------------------------
void
   do_HGET(ServerRequest& Q)
{  if( opt_hcdm && opt_verbose )
     debugf("ServerThread(%p)::do_%s\n", this, Q.method.c_str());

   do_FILE(Q);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::do_HTML
//
// Function-
//       Generate HTML response
//
//----------------------------------------------------------------------------
void
   do_HTML(ServerRequest& Q, int code, string html)
{  if( opt_hcdm && opt_verbose )
     debugf("ServerThread(%p)::do_HTML(%d)\n", this, code);

   ServerResponse& S= *Q.get_response();
   S.set_code(code);                // Set response code
   log_request(Q, S);

   S.insert(HTTP_TYPE, "text/html; charset=utf-8");
   S.insert(HTTP_SIZE, std::to_string(html.size()));
   if( Q.method != HTTP_HEAD )
     S.write(html);

   S.write();
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::do_POST
//
// Function-
//       Handle "POST" request
//
//----------------------------------------------------------------------------
void
   do_POST(ServerRequest& Q)
{  if( opt_hcdm && opt_verbose )
     debugf("ServerThread(%p)::do_POST\n", this);

   string body=
   utility::to_string("POST[%s]", ((string)Q.get_ioda()).c_str());
   do_HTML(Q, 200, page200(body));
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::run
//
// Function-
//       Operate the server Thread
//
// Implementation notes-
//       The ServerThread actually runs completely asynchronously.
//       This implementation allows an alternate mechanism to be added.
//
//----------------------------------------------------------------------------
void
   run( void )
{  // NOT CODED YET
debugf("%4d %s HCDM\n", __LINE__, __FILE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::stop
//
// Function-
//       Terminate the server Thread
//
//----------------------------------------------------------------------------
void
   stop( void )                     // Shut down the ServerThread
{  if( opt_hcdm ) debugf("ServerThread(%p)::stop\n", this);

   operational= false;
   listen->close();
   ended.post();
}
}; // class ServerThread
#endif // T_STREAM_HPP_INCLUDED
