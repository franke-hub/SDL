//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ifmacro.h
//
// Purpose-
//       IFMACRO defaults and macros.
//
// Last change date-
//       2022/09/02
//
// Controls-
//       HCDM     Hard Core Debug Mode (Intensive tracing)
//       IODM     Input/Output Debug mode (I/O tracing)
//       SCDM     Soft Core Debug Mode (Moderate tracing)
//       SCDM     A.K.A. Special Case Debug Mode (Special case debugging)
//       CHECK    Enhanced error checking
//       STATS    Statistics trace
//       TRACE    Memory trace activation
//
// Implementation note-
//       To be deprecated and replaced by enum declarations.
//       See detail/Trace.h for example usage.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// IFHCDM, IFIODM IFSCDM, and IFTRACE Macros
//----------------------------------------------------------------------------
#undef IFHCDM                       // Allow multiple inclusions
#undef ELHCDM
#undef IFIODM
#undef ELIODM
#undef IFSCDM
#undef ELSCDM
#undef IFCHECK
#undef IFSTATS
#undef IFTRACE

#ifdef HCDM                         // If defined, Hard Core Debug Mode
  #define IFHCDM(x) {x}
  #define ELHCDM(x) {}
#else
  #define IFHCDM(x) {}
  #define ELHCDM(x) {x}
#endif

#ifdef IODM                         // If defined, Input/Output Debug Mode
  #define IFIODM(x) {x}
  #define ELIODM(x) {}
#else
  #define IFIODM(x) {}
  #define ELIODM(x) {x}
#endif

#ifdef SCDM                         // If defined, Soft Core Debug Mode
  #define IFSCDM(x) {x}
  #define ELSCDM(x) {}
#else
  #define IFSCDM(x) {}
  #define ELSCDM(x) {x}
#endif

//----------------------------------------------------------------------------
// Note: No ELSE clause for these facilities
#ifdef CHECK                        // If defined, enable enhanced checks
  #define IFCHECK(x) {x}
#else
  #define IFCHECK(x) {}
#endif

#ifdef STATS                        // If defined, enable statistics counters
  #define IFSTATS(x) {x}
#else
  #define IFSTATS(x) {}
#endif

#ifdef TRACE                        // If defined, enable in-memory tracing
  #define IFTRACE(x) {x}            // Does NOT test trace active
#else
  #define IFTRACE(x) {}
#endif
