//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       detail/Trace.h
//
// Purpose-
//       Trace table storage allocator, implementation controls
//
// Last change date-
//       2020/06/28
//
// Usage notes-
//       This contains implementation controls for ~/lib/Trace.cpp.
//       These are included here so that test cases can display Trace.cpp
//       compile time options. This allows these test cases to react when
//       these controls are modified, if only to log what they are.
//
//       Use #include "detail/Trace.h" *AFTER* including Trace.h.
//       (The quoted include is required for automated dependency control.)
//
//----------------------------------------------------------------------------
#ifndef _PUB_DETAIL_TRACE_H_INCLUDED
#define _PUB_DETAIL_TRACE_H_INCLUDED

namespace _PUB_NAMESPACE::detail::Trace { // Detail namespace for Trace.cpp
//----------------------------------------------------------------------------
// Special Case Debug Mode-
//   SCDM > 0 enables the special case logic in allocate(), which counts the
//   compare_exchange retries in spins.
//   When spins > SCDM, nullptr is returned to the caller, and we replace the
//   allocated record a .TAF (Trace Allocation Failure) record. Additionally,
//   if USE_DEACTIVATE, we invoke deactivate() (to terminate tracing.)
//   (USE_DEACTIVATE has no effect unless SCDM is enabled.)
//----------------------------------------------------------------------------
enum // Compile-time options
{  CHECK= false                     // Check for should not occur conditions?
,  HCDM= false                      // Hard Core Debug Mode?
,  SCDM= 0                          // Special Case Debug Mode, spin limit
,  USE_DEACTIVATE= false            // SCDM(Deactivate trace option)
}; // Compile-time options
}  // namespace _PUB_NAMESPACE::detail::Trace
#endif // _PUB_DETAIL_TRACE_H_INCLUDED
