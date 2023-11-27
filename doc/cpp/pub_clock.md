<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/pub_clock.md
//
// Purpose-
//       Clock.h reference manual
//
// Last change date-
//       2023/11/16
//
-------------------------------------------------------------------------- -->
## Defined in header <pub/Clock.h>

##### pub::Clock:: get, set, now

####
---
### <a id="attributes">Attributes:</a>

#### private: double `time`;

Contains the time set by a constructor, operator, or the set method.
This value is returned by the get method and the double cast operator.

---
### <a id="constructors">Constructors:</a>

#### Clock( void );
The default constructor sets `time` to the current time.

#### Clock(const Clock& source );
The copy constructor copies `source.time` into the `time` attribute.

#### Clock(double source );
The copy constructor sets the `time` attribute from the source.

---
### <a id="operators">Operators:</a>

#### Clock& operator=(const Clock& source);
The assignment operator copies `source.time` into the `time` attribute.

#### Clock& operator=(double source);
The assignment operator copies source into the `time` attribute.

#### explicit operator double( void );
The assignment operator returns the `time` attribute, and is equivalent to the
get method.

#### Clock& operator+=(const Clock& rhs);
Adds `rhs.time` to the `time` attribute.

#### Clock& operator-=(const Clock& rhs);
Subtracts `rhs.time` from the `time` attribute.

#### friend Clock operator+(const Clock& lhs, const Clock& rhs);
Returns a Clock with its `time` attribute set to the sum of
`lhs.time` + `rhs.time`.

#### friend Clock operator-(const Clock& lhs, const Clock& rhs);
Returns a Clock with its `time` attribute set to `lhs.time` - `rhs.time`.

#### bool operator!=(const Clock& rhs) const;
#### bool operator==(const Clock& rhs) const;
#### bool operator>=(const Clock& rhs) const;
#### bool operator<=(const Clock& rhs) const;
#### bool operator> (const Clock& rhs) const;
#### bool operator< (const Clock& rhs) const;
Compares the `time` attribute to the `rhs.time` attribute.

---
### Methods:

#### <a id="get">double get( void ) const;</a>

Return value: (The `time` attribute.) The number of seconds since the epoch,
as set by the constructor, an operator, or the set method.

---
#### <a id="set">void set(double time);</a>
Sets the clock `time` attribute. Use set(now()) to set the current time.

---
#### <a id="now">static double now( void );</a>

Note that this is a static method. It does not modify the `time` attribute.

Return value: The number of seconds since the epoch.

---
