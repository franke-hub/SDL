%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%%       Copyright (C) 2007 Frank Eskesen.
%%
%%       This file is free content, distributed under cc by-sa version 3.0,
%%       with attribution required.
%%       (See accompanying file LICENSE.BY_SA-3.0 or the original contained
%%       within https://creativecommons.org/licenses/by-sa/3.0/us/legalcode)
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Title-
%%       StaffAndClef.ly
%%
%% Purpose-
%%       Staff and clef layouts, needs work.
%%
%% Last change date-
%%       2007/01/01
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
  title = "Staffs and Clefs"
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
softBreak = { \break }

signature = {
    \key c \major
    \override Staff.TimeSignature #'style = #'()
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Staffs
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
<<
  \new PianoStaff
  {
    \cadenzaOn
    \autochange

    \relative c,,
    {
      a^\markup{a,,,}
      b^\markup{b,,,}

      \bar "|"
      c^\markup{c,,} % c,,
      d^\markup{d,,}
      e^\markup{e,,}
      f^\markup{f,,}
      g^\markup{g,,}
      a^\markup{a,,}
      b^\markup{b,,}

      \bar "|"
      c^\markup{c,} % c,
      d^\markup{d,}
      e^\markup{e,}
      f^\markup{f,}
      g^\markup{g,}
      a^\markup{a,}
      b^\markup{b,}

      \bar "|"
      c^\markup{c} % c
      d^\markup{d}
      e^\markup{e}
      f^\markup{f}
      g^\markup{g}
      a^\markup{a}
      b^\markup{b}

      \bar "|"
      c^\markup{c'} % c'
      d_\markup{d'}
      e_\markup{e'}
      f_\markup{f'}
      g_\markup{g'}
      a_\markup{a'}
      b_\markup{b'}

      \bar "|"
      c_\markup{c''} % c''
      d_\markup{d''}
      e_\markup{e''}
      f_\markup{f''}
      g_\markup{g''}
      a_\markup{a''}
      b_\markup{b''}

      \bar "|"
      c_\markup{c'''} % c'''
      d_\markup{d'''}
      e_\markup{e'''}
      f_\markup{f'''}
      g_\markup{g'''}
      a_\markup{a'''}
      b_\markup{b'''}

      \bar "|"
      c_\markup{c''''} % c''''
      d_\markup{d''''}
      e_\markup{e''''}
      f_\markup{f''''}
      g_\markup{g''''}
      a_\markup{a''''}
      b_\markup{b''''}

      \bar "|"
      c_\markup{c'''''} % c'''''
    }

    \cadenzaOff
  }
>>
}

\score
{
<<
  \new StaffGroup = "StaffGroup"
  <<
    \new Staff = "Alpha"
    \relative c'
    {
      \set Staff.instrumentName = #"Alpha"
      \set Staff.shortInstrumentName = #"Al"
      c1^\markup{Working on instrument names} c
    }
    \new Staff = "Beta"
    \relative c'
    {
      \set Staff.instrumentName = #"Beta"
      \set Staff.shortInstrumentName = #"B."
      c1 c
    }
  >>
>>
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Clefs
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
<<
  \new Staff
  {
    \cadenzaOn
    s_\markup{The default treble clef}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \clef treble
      \bar "" \break
      s_\markup{treble}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \bar ""
    \clef percussion s
    \clef violin
      s_\markup{violin}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \bar ""
    \clef percussion s
    \clef G
      s_\markup{G}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \bar ""
    \clef percussion s
    \clef "G2"
      s_\markup{"G2"}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \clef bass
      \bar "" \break
      s_\markup{bass}
      \relative c  { c_\markup{c} }
      \relative c  { c'_\markup{c'} }

    \bar ""
    \clef percussion s
    \clef F
      s_\markup{F}
      \relative c  { c_\markup{c} }
      \relative c  { c'_\markup{c'} }

    \clef alto
      \bar "" \break
      s_\markup{alto}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \bar ""
    \clef percussion s
    \clef C
      s_\markup{C}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \clef tenor
      \bar "" \break
      s_\markup{tenor}
      \relative c  { c_\markup{c} }
      \relative c  { c'_\markup{c'} }

    \clef french
      \bar "" \break
      s_\markup{french}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \clef soprano
      \bar "" \break
      s_\markup{soprano}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \clef mezzosoprano
      \bar "" \break
      s_\markup{mezzosoprano}
      \relative c  { c'_\markup{c'} }
      \relative c  { c''_\markup{c''} }

    \clef baritone
      \bar "" \break
      s_\markup{baritone}
      \relative c  { c_\markup{c} }
      \relative c  { c'_\markup{c'} }

    \clef varbaritone
      \bar "" \break
      s_\markup{varbaritone}
      \relative c  { c_\markup{c} }
      \relative c  { c'_\markup{c'} }

    \clef subbass
      \bar "" \break
      s_\markup{subbass}
      \relative c  { c_\markup{c} }
      \relative c  { c'_\markup{c'} }

    \clef percussion
      \bar "" \break
      s_\markup{percussion}
      \relative c  { c_\markup{c} }
      \relative c  { c'_\markup{c'} }

    \clef tab
      \bar "" \break
      s_\markup{tablature}
      \relative c  { c_\markup{c} }
      \relative c  { c'_\markup{c'} }

    \cadenzaOff
  }
>>
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% MIDI output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% No MIDI output

