<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
// SPDX-License-Identifier: CC-BY-4.0
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/Time-reference.md
//
// Purpose-
//       PUB Library, time representation
//
// Last change date-
//       2026/04/30
//
-------------------------------------------------------------------------- -->
# Time Representation: Clock, Julian, Calendar

The PUB library separates time handling into three layers:

- **Clock**: seconds since Unix epoch (1970-01-01 UTC)
- **Julian**: continuous fractional day count (floating-point, midnight-based)
- **Calendar**: civil date/time (year, month, day, hour, minute, second)

This separation allows each representation to remain simple and internally
consistent.

### Limitations

- Does not account for leap seconds (UT1/UTC differences)
- Sub-microsecond precision is not guaranteed
- As absolute time values or differences become larger, precision is lost

<!-- --------------------------------------------------------------------- -->
## Clock

Represents time as seconds since the Unix epoch (1970-01-01 00:00:00 UTC).

### Representation

- Type: `double`
- Unit: (fractional) seconds
- Epoch: Unix epoch (UTC)

### Semantics

- Continuous time scale
- Suitable for interval arithmetic and ordering
- Independent of calendar systems

### Precision

- IEEE-754 double precision (almost 16 decimal digit precision)
  - Calendar second value is in fractional seconds since the prior second
  - Clock value is in fractional seconds since 1/1/1970
  - Julian value is in fractional days since 1/1/0000 Julian
- Resolution near present epoch: 10^-5 seconds (10 digits for integer portion,
almost 6 digits for fractional portion)

### Guarantees

- Arithmetic operations preserve monotonicity
- Differences between nearby values are accurate to within floating-point
double precision

### Usage Notes

- Prefer `Clock` for:
  - Elapsed time
  - Scheduling
  - Event ordering

<!-- --------------------------------------------------------------------- -->
## Julian

Represents time as a continuous fractional count of days.

### Representation

- Type: `double`
- Unit: days
- Fractional part encodes time-of-day
- Day boundary: midnight

### Semantics

- Continuous, monotonic time scale
- Used as an intermediate form between Clock and Calendar

### Calendar Basis

- Internally uses astronomical year numbering:
  - Year 0 is valid
  - Year 0 and negative years represent BCE

### Precision

- Double precision, measured in days
- Resolution near present epoch: 10^-8 days (7 digits for integer portion,
8 digits for fractional day portion) [0.00086400 second resolution]

### Notes

- This is not the standard astronomical Modified Julian Date (MJD)
- The term “modified” refers only to midnight-based day boundary
- Internally uses astronomical year numbering:
  - Year 0 exists
  - Year 0 and negative years represent BCE

### Usage Notes

- Prefer `Julian` for:
  - Date arithmetic
  - Conversion pipelines

<!-- --------------------------------------------------------------------- -->
## Calendar

Represents civil date and time.

### Representation

- Year, month, day
- Hour, minute, (fractional) second (of minute)

### Calendar System

- Julian calendar through 1582-10-04
- Gregorian calendar from 1582-10-15 onward

### Invalid Range

- Dates between 1582-10-05 and 1582-10-14 are not representable

### Implementation

- Based on:
  - 4-year Julian cycle (1461 days)
  - 400-year Gregorian cycle (146097 days)
- Uses precomputed lookup tables for day to date conversion
- Calendar conversions use table lookup
  - More easily understood than currently available algorithms
  - Easily extendable to negative years

### Performance

- Constant-time conversion
- No iteration over months or years

### Semantics

- Represents civil time, not precise physical time
- Does not include timezone

### Usage Notes

- Prefer `Calendar` for:
  - Display
  - Input/output
  - User-facing date handling

<!-- --------------------------------------------------------------------- -->
## Conversion Relationships

Clock <=> Julian
- Linear scaling (seconds <=> days)

Julian <=> Calendar
- Cycle decomposition + table lookup

Calendar <=> Clock
- Composed conversion via intermediate Julian

<!-- --------------------------------------------------------------------- -->
### Year Numbering

The implementation uses astronomical year numbering:

- Year 1  = 1 CE
- Year 0  = 1 BCE
- Year -1 = 2 BCE

This differs from traditional historical notation, which has no year 0.

To simplify calculations, our calendar base uses day zero origins:
- Julian day zero: 1,721,058 (1/1/0000 Julian, 12/30/-001 Gregorian)
- Gregorian day zero: 1,721,060 (1/1/0000 Gregorian, 1/3/0000 Julian)

Gregorian day zero is four Gregorian cycles before 1/1/1600. The Gregorian
cycle repeats every 400 years, ie, every 146,097 days.

<!-- --------------------------------------------------------------------- -->
For a comprehensive example, see: src/cpp/lib/pub/Test/TestTime.cpp

This test exercises conversions including conversion precision verification in
the range: Year -5000 (Julian) through +5000 (Gregorian.)

It verifies:
- round-trip conversion accuracy
- correct handling of negative years
- correct behavior across the 1582 calendar transition
