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
%%       Instrument.ly
%%
%% Purpose-
%%       Instrument ranges, needs work.
%%
%% Last change date-
%%       2007/01/01
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header
{
   title = "Instrument Ranges"
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Piano
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
acousticGrandVoice = \relative c,, {
   \set Staff.midiInstrument = "acoustic grand"

   \bar " "           a b  % c,,,
   \bar "|" c d e f g a b  % c,,
   \bar "|" c d e f g a b  % c,
   \bar "|" c d e f g a b  % c
   \bar "|" c d e f g a b  % c'
   \bar "|" c d e f g a b  % c''
   \bar "|" c d e f g a b  % c'''
   \bar "|" c d e f g a b  % c''''
   \bar "|" c              % c'''''
   \bar " "
}
acousticGrandWords = \new Lyrics \lyricmode {
                                      a,,,   b,,,
   c,,    d,,    e,,    f,,    g,,    a,,    b,,
   c,     d,     e,     f,     g,     a,     b,
   c      d      e      f      g      a      b
   c'     d'     e'     f'     g'     a'     b'
   c''    d''    e''    f''    g''    a''    b''
   c'''   d'''   e'''   f'''   g'''   a'''   b'''
   c''''  d''''  e''''  f''''  g''''  a''''  b''''
   c'''''
}

sopranoVoice = \relative c' {
   \set Staff.midiInstrument = "soprano sax"

   \bar " "           a b  % c
   \bar "|" c d e f g a b  % c'
   \bar "|" c d e f g a b  % c''
   \bar "|" c              % c'''
}
sopranoWords = \new Lyrics \lyricmode {
                                      a      b
   c'     d'     e'     f'     g'     a'     b'
   c''    d''    e''    f''    g''    a''    b''
   c'''
}

\score
{
   \new PianoStaff
   {
     \cadenzaOn
     \autochange
     <<
       \acousticGrandVoice
       \acousticGrandWords
     >>

     \bar "" \break
     <<
       \sopranoVoice
       \sopranoWords
     >>
     \cadenzaOff
   }
}

\score
{
   \new PianoStaff
   {
     \acousticGrandVoice
     r r r r
     \sopranoVoice
   }
   \midi { \tempo 4 = 144 }
}
