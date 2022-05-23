//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       SampleBIO.h
//
// Purpose-
//       BIO debugging routines.
//
// Last change date-
//       2022/05/07
//
//----------------------------------------------------------------------------
#ifndef SAMPLEBIO_H_INCLUDED
#define SAMPLEBIO_H_INCLUDED

#include <openssl/opensslv.h>       // For OPENSSL_VERSION_NUMBER
#include <openssl/bio.h>            // For openssl BIO methods

#include <pub/Debug.h>              // For debugging
using namespace _PUB_NAMESPACE;     // For Debug, ...
using namespace _PUB_NAMESPACE::debugging; // For debugging

#if (OPENSSL_VERSION_NUMBER > 0x01000210fL)
// Definitions removed sometime after 1.0.2p
typedef struct bio_method_st {
    int type;
    const char *name;
    int (*bwrite) (BIO *, const char *, int);
    int (*bread) (BIO *, char *, int);
    int (*bputs) (BIO *, const char *);
    int (*bgets) (BIO *, char *, int);
    long (*ctrl) (BIO *, int, long, void *);
    int (*create) (BIO *);
    int (*destroy) (BIO *);
    long (*callback_ctrl) (BIO *, int, bio_info_cb *);
} BIO_METHOD;

struct bio_st {
    BIO_METHOD *method;
    /* bio, mode, argp, argi, argl, ret */
    BIO_callback_fn callback;
    BIO_callback_fn_ex callback_ex;
    char *cb_arg;               /* first argument for the callback */
    int init;
    int shutdown;
    int flags;                  /* extra storage */
    int retry_reason;
    int num;
    void *ptr;
    struct bio_st *next_bio;    /* used by filter BIOs */
    struct bio_st *prev_bio;    /* used by filter BIOs */
    int references;
    unsigned long num_read;
    unsigned long num_write;
    CRYPTO_EX_DATA ex_data;
};
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       Debugging displays
//
//----------------------------------------------------------------------------
static inline void
   debug(                           // Debugging display
     BIO_METHOD*       method)      // For this BIO_METHOD
{
   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());

   if( options::pub_verbose > 0 )
     debugh("debug BIO_METHOD(%p) %d\n", method, method->type);
   else
     traceh("debug BIO_METHOD(%p) %d\n", method, method->type);
   traceh("..name(%s)\n",     method->name);
   traceh("..bwrite(%p)\n",   method->bwrite);
   traceh("..bread(%p)\n",    method->bread);
   traceh("..bputs(%p)\n",    method->bputs);
   traceh("..bgets(%p)\n",    method->bgets);
   traceh("..ctrl(%p)\n",     method->ctrl);
   traceh("..create(%p)\n",   method->create);
   traceh("..destroy(%p)\n",  method->destroy);
   traceh("..callback(%p)\n", method->callback_ctrl);
}

static inline void
   debug(                           // Debugging display
     BIO*              bio)         // For this BIO
{
   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());

   tracef("BIO(%p) N(%p) P(%p) %p:%s\n", bio, bio->next_bio, bio->prev_bio
         , bio->method, bio->method->name);
// traceh("..callback(%p)\n", bio->callback);
// traceh("..cb_arg(%p)\n",   bio->cb_arg);
// traceh("..init(%d)\n",     bio->init);
// traceh("..shutdown(%d)\n", bio->shutdown);
// traceh("..flags(0x%x)\n",  bio->flags);
// traceh("..reason(%d)\n",   bio->retry_reason);
// traceh("..num(%d)\n",      bio->num);
// traceh("..ptr(%p)\n",      bio->ptr);
// traceh("..refs(%d)\n",     bio->references);
// traceh("..reads(%ld)\n",   bio->num_read);
// traceh("..writes(%ld)\n",  bio->num_write);
// traceh("..\n"); debug(bio->method);
}

static inline void
   debug_chain(                     // Debugging display
     BIO*              bio,         // For this BIO chain
     const char*       info= "")    // For this description
{
   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());
   if( options::pub_verbose > 0 )
     debugh("%s: debug_chain(%p)\n", info, bio);
   else
     traceh("%s: debug_chain(%p)\n", info, bio);

   while( bio )
   {
     debug(bio);
     if( bio->next_bio )
     {
       if( true  ) {
         if( bio->next_bio->prev_bio != bio ) {
           debugf("BIO(%p)->next_bio(%p)->prev_bio(%p) **ERROR**\n",
                  bio, bio->next_bio, bio->next_bio->prev_bio);
         }
       }

       if( bio->next_bio == bio ) {
         debugf("BIO(%p)->next_bio(%p) **ERROR**\n", bio, bio->next_bio);
         break;
       }
     }

     bio= bio->next_bio;
   }
}
#endif // SAMPLEBIO_H_INCLUDED
