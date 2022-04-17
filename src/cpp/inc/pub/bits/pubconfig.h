//----------------------------------------------------------------------------
//
//       Copyright (c) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/bits/pubconfig.h
//
// Purpose-
//       Configuration control macros.
//
// Last change date-
//       2022/04/08
//
// Implementer notes-
//       Function attributes should appear on a function's declaration and also
//       on its instantiation.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_VERSION
#define _LIBPUB_VERSION 20220408

#define _LIBPUB_CPP03 199711L
#define _LIBPUB_CPP11 201103L
#define _LIBPUB_CPP14 201402L
#define _LIBPUB_CPP17 201703L
#define _LIBPUB_CPP20 202002L

#if __cplusplus < _LIBPUB_CPP11
#  error "Library pub requires C++ version >= 11"
#endif

#define _LIBPUB_NAMESPACE pub
#define _LIBPUB_BEGIN_NAMESPACE namespace _LIBPUB_NAMESPACE {
#define _LIBPUB_BEGIN_NAMESPACE_VISIBILITY(v) \
   namespace _LIBPUB_NAMESPACE _LIBPUB_VISIBILITY(v) {

#define _LIBPUB_END_NAMESPACE }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#if __cplusplus >= _LIBPUB_CPP17
#  define _LIBPUB_NODISCARD [[nodiscard]]
#else
#  define _LIBPUB_NODISCARD
#endif

#if defined(__GNUC__)
#  define _LIBPUB_COLD __attribute__ ((cold))
#  define _LIBPUB_DEPRECATED __attribute__ ((deprecated))
#  define _LIBPUB_DEPRECATED_USE(alt) __attribute__ ((deprecated (alt)))
#  define _LIBPUB_FLATTEN __attribute__ ((flatten))
#  define _LIBPUB_HOT __attribute__ ((hot))
#  define _LIBPUB_MAYBE_UNUSED __attribute__ ((unused))
#  define _LIBPUB_NOCLONE __attribute__ ((noclone))
#  define _LIBPUB_NOINLINE __attribute__ ((noinline))
#  define _LIBPUB_NONULL_RETURN __attribute__ ((returns_nonnull))
#  define _LIBPUB_NORETURN __attribute__ ((noreturn)) // Just use [[noreturn]]
#  define _LIBPUB_PRINTF(fmt_parm, arg_parm) \
       __attribute__ ((format (printf, fmt_parm, arg_parm)))
#  define _LIBPUB_PURE __attribute__ ((pure))
#  define _LIBPUB_USED __attribute__ ((used))
#  define _LIBPUB_VISIBILITY(v) __attribute__ ((visibility (#v)))
#  define _LIBPUB_WARN_UNUSED __attribute__ ((warn_unused_result))
#else
#  define _LIBPUB_COLD
#  define _LIBPUB_DEPRECATED
#  define _LIBPUB_DEPRECATED_USE(alt)
#  define _LIBPUB_FLATTEN
#  define _LIBPUB_HOT
#  define _LIBPUB_MAYBE_UNUSED
#  define _LIBPUB_NOCLONE
#  define _LIBPUB_NOINLINE
#  define _LIBPUB_NONULL_RETURN
#  define _LIBPUB_NORETURN          // Just use [[noreturn]]
#  define _LIBPUB_PRINTF(fmt_parm, arg_parm)
#  define _LIBPUB_PURE
#  define _LIBPUB_USED
#  define _LIBPUB_VISIBILITY(v)
#  define _LIBPUB_WARN_UNUSED
#endif

#endif // _LIBPUB_VERSION
