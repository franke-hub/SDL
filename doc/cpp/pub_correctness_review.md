# PUB Library Correctness Review

Repository: `franke-hub/SDL`, library under review: `src/cpp/lib/pub`
(with headers in `src/cpp/inc/pub`). Reviewed at commit `6704b679`
("Merge branch 'maint' into trunk").

Scope: every `.cpp`/`.h` file in the library (91 files, ~35,000 lines of
logic plus the 146,097-entry `Cal400.cpp` data table). Method: manual
reading of every module, independent programmatic verification of the
calendar arithmetic, and a full clean build of all 40 `.cpp` files with
`g++ -Wall -Wextra` followed by compiling and running the existing
`Test/` suite under AddressSanitizer + UndefinedBehaviorSanitizer.

## Summary

The library is in good shape. All 40 source files compile cleanly with
`-Wall -Wextra` (zero warnings) once the two prerequisites below are
supplied, and the calendar/date arithmetic — the most numerically
intricate part of the library — checks out exactly against an
independent reference algorithm across a wide range of dates. Two
small, confirmed, one-line bugs were found and have since been fixed.
A third item — a sanitizer diagnostic against
`AI_list<T>` — turned out, after a fairly deep investigation, not to be
a code defect at all; it's recorded below as a documented, currently-
believed sanitizer false positive so the diagnostic doesn't get
rediscovered and re-investigated from scratch later.

## Confirmed issues (both fixed)

### 1. `Ioda::discard()` uses `memcpy` on overlapping memory (Ioda.cpp:788)

```cpp
memcpy(page->data, page->data+page_used, page_left); // Move remainder
```

Both the source and destination are the *same* buffer (`page->data`),
shifted by `page_used` bytes. Whenever the remaining data is longer
than the shift distance (`page_left > page_used`), the source and
destination ranges overlap — `memcpy`'s behavior is undefined in that
case (the C standard requires non-overlapping regions; `memmove` is
the function meant for this). AddressSanitizer's `memcpy-param-overlap`
detector caught this ten times during a single run of the existing
`Test/TestIoda.cpp` suite, so this is a live path, not a theoretical
corner case. On glibc it currently happens to produce the right answer
(the implementation in use here copies forward), but that is not
guaranteed by any standard and can silently corrupt data with a
different libc, a different glibc version/CPU (some vectorized
`memcpy` paths copy in chunks that are not safe for this pattern), or
under `_FORTIFY_SOURCE` hardening.

**Fix (applied):** changed `memcpy` to `memmove` at line 788. (The
very similar call at line 954, in the page-splitting method, copies
between two *different* `Page` objects and is not affected.)

### 2. `Hardware::getTSC()` fallback doesn't compile (Hardware.cpp:59)

```cpp
#if !defined(__GNUC__) || !defined(_HW_X86) // GNU compiler, x86 required
uint64_t Hardware::getTSC( void )
{  static atomic_uint64_t tsc= 0; return ++tsc; }
```

`atomic_uint64_t` is used unqualified, but only `<atomic>` is included
(which defines `std::atomic_uint64_t`) — there's no `using` declaration
and no `std::` prefix. This branch fails to compile with a hard error
("`atomic_uint64_t` does not name a type").

In practice this is masked today because the project's own
`Makefile.OPT` (both `ctl/BSD` and `ctl/WIN`) always defines
`-D_HW_X86`, so the `#else` (inline assembly `rdtsc`) branch is the one
actually built. But this fallback exists specifically to support
non-x86 hardware or non-GNU compilers, and as written it can't: an ARM
build, or any invocation that doesn't route through those Makefiles
(as I found simply compiling the library directly with `g++ -Wall
-Wextra -I inc`), fails outright.

**Fix (applied):** `static std::atomic_uint64_t tsc= 0;`.

## Documented: UBSan false positive on `AI_list<T>` downcast (no action needed)

`AI_list<T>::fifo()`, `get_tail()`, and `reset()` (List.h) all end with
a downcast from the internal, non-polymorphic `__detail::_BISL_link*`
hook pointer back to `T*`, e.g.:

```cpp
return static_cast<pointer>(_Base::fifo(link));   // pointer == T*
```

`Test/TestList.cpp` contains a deliberately adversarial case
(`AI_block`, guarded by the comment "This replicates a problem
environment found in testing") where the payload type inherits `Link`
as a non-first base alongside an unrelated polymorphic base (`Vclass`).
Running that test under UndefinedBehaviorSanitizer's vptr check flags
the downcast:

```
List.h:123:17: runtime error: downcast of address 0x... which does not
point to an object of type 'AI_block'
0x...: note: object has invalid vptr
```

**Status, as of this review: believed to be a sanitizer false
positive, not a code defect.** This was investigated thoroughly,
including three targeted experiments, specifically because it sits
under Dispatch's task queue and Thread's worker pool:

1. A virtual destructor on `AI_list<void>` (the *container* class) —
   no effect. `AI_list<void>` isn't even part of the pointer's actual
   inheritance chain (`_BISL_link → AI_list<T>::Link → T`), so this
   couldn't have mattered, and confirming that empirically ruled it
   out cleanly.
2. A virtual destructor on `__detail::_BISL_link` itself (the actual
   hook type) — also no effect. This was the more plausible fix
   candidate and it didn't change the diagnostic at all. (We are
   deliberately *not* doing this anyway: it would add a vtable pointer
   to every single list node, doubling the hook's size purely to
   satisfy a sanitizer.)
3. Splitting the single long-distance `static_cast` into two explicit
   single-level hops (`_BISL_link* → Link* → T*`) — also no effect,
   flagged at the same place. This rules out "the cast is phrased
   badly"; the trigger is specifically downcasting to a polymorphic,
   multiply-inherited type through a *non-primary* base (here, `Link`
   is neither first nor the source of `AI_block`'s vtable — `Vclass`,
   listed first, is).

On top of that, a minimal standalone reproduction of the identical
layout (protected inheritance, friend access, an unrelated first
polymorphic base, even a cross-translation-unit call to mirror
`_Base::fifo()` living in a separately-compiled object file) computes
the *correct* address and produces *no* sanitizer complaint at all
when built and run in isolation. Combined with the facts that ASan
reports no invalid memory access anywhere in this path and the test's
own internal correctness checks (`VERIFY`/`error_count`) pass, the
actual pointer arithmetic looks sound; something specific to linking
the full library (most likely how GCC handles vtable/typeinfo identity
for a class whose virtual functions are all defined inline, with no
out-of-line "key function", across a prebuilt static archive) is what
the checker dislikes, not the address computation itself.

`Dispatch::Item` is unaffected either way — it inherits
`AI_list<Item>::Link` as its sole base with no other polymorphism
involved, so production code never exercises this combination.

**No code change is planned or recommended.** This entry exists so
that if this diagnostic resurfaces (a newer compiler, a different
sanitizer version, or someone re-running `TestList.cpp` under UBSan)
it's recognized as already investigated rather than re-chased from
scratch. If it's ever seen to correlate with an *actual* wrong value
or an ASan-reported memory error, that would change this conclusion
and warrant a fresh look.

## Verified correct (worth noting, since it's easy to assume otherwise)

- **`Cal400.cpp`'s 146,097-entry day-to-{year,month,day} table** is
  exactly correct. I independently regenerated the same 400-year
  Gregorian cycle using Howard Hinnant's well-known `civil_from_days`
  algorithm and diffed it against every entry in the table: zero
  mismatches.
- **`Calendar.cpp`'s Julian/Gregorian conversion arithmetic**
  (`ymd2julian_g/j`, `julian2ymd_g/j`, the leap-year and century-
  correction logic) round-trips exactly across a 2,500+ year span
  spanning both calendars, and matches real, independently-verifiable
  Julian Day Numbers at several anchor points: `1582/10/15 → JD
  2299161` (the historical Julian→Gregorian switch), `2000/1/1 → JD
  2451545` (J2000), and `1970/1/1 → JD 2440588` (Unix epoch).
- **The `AI_list<void>` lock-free MPSC algorithm itself** (the Treiber-
  stack push in `fifo()`, the "install a sentinel, detach, reverse"
  drain in `reset()`/`_AI_iter`, and the "reschedule only when the
  previous tail was null" handoff in `dispatch::Task::enqueue()`) is
  correct under careful manual race analysis. I specifically checked
  the empty→non-empty handoff race (a worker finishing its drain right
  as a new item arrives) and confirmed the two cases are always either
  cleanly sequential or cleanly disjoint — no double dispatch, no
  dropped work.
- **All 40 library source files compile with zero warnings** under
  `g++ -std=gnu++17 -Wall -Wextra` (after supplying `-D_HW_X86`, which
  the project's own Makefiles always provide, and Boost, which is a
  documented prerequisite).
- `Test/TestList.cpp`, `TestData.cpp` (once given a required
  argument), `Test_mem.cpp`, and `Test_utf.cpp` all ran clean under
  ASan+UBSan with no other findings.
- `Test/TestMisc.cpp` runs to completion (~98 seconds: 6 billion
  duplicate-check iterations plus 50 million each for sequential-run
  and per-bit-frequency counting) — it isn't hung, it's just a long,
  thorough statistical test of `Random`.
- `Test/Test_num.cpp` (updated to use `pub/Random.h` instead of the
  older `com::Random`) runs fine against `pub` standalone.
- `Random::get64()`'s `new_seed &= INT64_MAX;` (Random.cpp) is not a
  no-op. Without it, bit 63 of the computed value does get set — both
  from the hardcoded `initializer` and from arbitrary seeds (confirmed
  by simulation) — and since the CAS loop feeds `new_seed` back in as
  the next `old_seed`, clearing that bit changes the entire subsequent
  trajectory almost immediately (masked/unmasked sequences diverge by
  the second call). `Random::get()` truncates to the low 32 bits, so
  this particular bit is never part of a single returned value
  directly, but it's still live internal state that shapes every later
  32-bit output through the feedback loop. (Worth a doc note on its
  own: `INT64_MAX` is `0x7FFF...FFFF`, the signed 64-bit maximum —
  *not* the unsigned 64-bit maximum. It's easy to misread at a glance
  since it's defined numerically rather than as a self-explanatory hex
  literal like `0x7fffffffffffffffLL` in at least one toolchain's
  header (Cygwin), so the name alone doesn't make the value obvious.)

## Not fully investigated (flagging rather than dropping)

- Socket/Select (network I/O) and Console (terminal I/O) were read
  carefully but not exercised at runtime here (no usable network/TTY
  in this sandbox). Nothing suspicious turned up on inspection, but
  that's a weaker form of review than the programmatic/sanitizer
  verification the rest of this report relies on.
