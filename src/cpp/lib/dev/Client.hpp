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
//       Client.hpp
//
// Purpose-
//       HTTP Client.cpp internal classes. (Only included from Client.cpp.)
//
// Last change date-
//       2022/07/16
//
//----------------------------------------------------------------------------
#ifndef _PUB_CLIENT_HPP_INCLUDED
#define _PUB_CLIENT_HPP_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       ClientItem
//
// Purpose-
//       The Client DispatchItem
//
//----------------------------------------------------------------------------
class ClientItem : public dispatch::Item { // Client DispatchItem
public:
enum { FC_FLUSH= FC_VALID+1 };      // FLUSH function

std::shared_ptr<ClientStream>
                       stream;      // The associated ClientStream

virtual
   ~ClientItem( void )              // Destructor
{  if( HCDM && VERBOSE > 0 ) debugh("~ClientItem(%p)\n", this); }

   ClientItem(                      // Constructor
     ClientStream*     S)           // The ClientStream
:  dispatch::Item(), stream(S->get_self())
{  if( HCDM && VERBOSE > 0 ) pub::debugging::debugh("ClientItem(%p)\n", this);

   done= nullptr;
   work= nullptr;
}
}; // class ClientItem

//----------------------------------------------------------------------------
//
// Class-
//       ClientThread
//
// Purpose-
//       The Client read Thread
//
//----------------------------------------------------------------------------
class ClientThread : public Named, public Thread { // Client thread
public:
Client*                client;      // The associated Client
Event                  event;       // Started Event
bool                   operational= false; // TRUE while in method run()

   ~ClientThread( void ) = default; // Destructor
   ClientThread(Client*);           // Constructor

void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

virtual void
   run( void );                     // Read responses
}; // class ClientThread

//----------------------------------------------------------------------------
//
// Subroutine-
//       ctx_error
//
// Purpose-
//       Handle CTX creation error
//
//----------------------------------------------------------------------------
static void
   ctx_error(                       // Handle CTX creation error
     const char*       fmt)         // Format string (with one %s)
{
   char buffer[256];                // Working buffer
   long E= ERR_get_error();         // Get last error code
   ERR_error_string(E, buffer);
   std::string S= pub::utility::to_string(fmt, buffer);
   throw SocketException(S);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ctx_password_cb
//
// Purpose-
//       Our pem_password_cb
//
//----------------------------------------------------------------------------
static int                          // Actual password length
   ctx_password_cb(                 // Our pem password callback
    char*              buff,        // Return buffer address (for password)
    int                size,        // Return buffer length  (for password)
    int                rwflag,      // FALSE:decryption, TRUE:encryption
    void*              userdata)    // User data
{
   if( false )                      // Note: only usage of userdata parameter
     debugf("%4d HCDM(%p,%d,%d,%p)\n", __LINE__, buff, size, rwflag, userdata);

   if( rwflag ) {                   // If encryption
     debugf("%4d HCDM SHOULD NOT OCCUR\n", __LINE__);
     return -1;                     // (NOT SUPPORTED)
   }

   const char* result= "xxyyz";     // Our (not so secret) password
   int L= strlen(result);           // Resultant length
   if( L > size ) L= size;          // (Cannot exceed maximum)

   memcpy(buff, result, L);         // Set the resultant
   return L;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       initialize_SSL
//
// Purpose-
//       Initialize SSL
//
//----------------------------------------------------------------------------
static void
   initialize_SSL( void )           // Initialize SSL
{
static std::mutex      mutex;       // Latch protecting initialized
static bool            initialized= false; // TRUE when initialized

   std::lock_guard<decltype(mutex)> lock(mutex);

   if( !initialized ) {
     SSL_library_init();
     SSL_load_error_strings();
     ERR_load_BIO_strings();
     OpenSSL_add_all_algorithms();

     initialized= true;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       iodm
//
// Purpose-
//       I/O debug mode message
//
//----------------------------------------------------------------------------
static void                         // NOTE: Preserves errno
   iodm(                            // I/O Debug Mode message
     int               line,        // Source code line number
     const char*       op,          // Operation
     ssize_t           L)           // Return code/length
{
   int ERRNO= errno;

   if( L < 0 )                      // If I/O error
     debugh("%4d Client %zd= %s() %d:%s\n", line, L, op
           , errno, strerror(errno));
   else if( IODM )                  // If I/O Debug Mode active
     traceh("%4d Client %zd= %s()\n", line, L, op);

   errno= ERRNO;
}

static void
   iodm(                            // I/O Debug Mode trace message
     int               line,        // Source code line number
     const char*       op,          // Operation
     const void*       addr,        // Data address
     ssize_t           size)        // Data length
{
   if( IODM && VERBOSE > 0 ) {
     string V((const char*)addr, size);
     V= visify(V);
     traceh("%4d Client::%s(addr,%zd)\n%s\n", line, op, size, V.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       new_client_CTX
//
// Purpose-
//       Create a client SSL_CTX
//
//----------------------------------------------------------------------------
static SSL_CTX*
   new_client_CTX( void )           // Create a client SSL_CTX
{
   const SSL_METHOD* method= TLS_client_method();
   SSL_CTX* context= SSL_CTX_new(method);
   if( context == nullptr )
     ctx_error("SSL_CTX_new: %s");

   SSL_CTX_set_mode(context, SSL_MODE_AUTO_RETRY);
   SSL_CTX_set_default_passwd_cb(context, ctx_password_cb);

   return context;
}
#endif // _PUB_CLIENT_HPP_INCLUDED
