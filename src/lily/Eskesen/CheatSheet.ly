%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%%       Copyright (C) 2020 Frank Eskesen.
%%
%%       This file is free content, distributed under cc by-sa version 3.0,
%%       with attribution required.
%%       (See accompanying file LICENSE.BY_SA-3.0 or the original contained
%%       within https://creativecommons.org/licenses/by-sa/3.0/us/legalcode)
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Title-
%%       CheatSheet.ly
%%
%% Purpose-
%%       Score listing all notes.
%%
%% Last change date-
%%       2020/02/05
%%
%% Implementation TODOs-
%%       Add spacing between systems.
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.18.8"
\header {
   title = "Note Cheat Sheet"
%  composer = "Frank Eskesen"
%  poet = "Frank Eskesen"
}

signature = {
   \key c \major
   \override Staff.TimeSignature #'style = #'()
   \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
   \time 4/4
}

instrument = \set Staff.midiInstrument = "acoustic grand"

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Pseudo lyrics
topLyrics = \new Lyrics \lyricmode {
   \skip 1 A1      B       C       D       E
   \skip 1 F       \skip 1 \skip 1 G
   \skip 4
   f4      g       a       b       c'      d'      e'
   f_'     g'      a'      b'      c''     d''     e''
   f_''    g''     a''     b''     c'''    d'''    e'''
   \skip 4 \skip 4
}

midLyrics = \new Lyrics \lyricmode {
   \skip 1 A1      B       C       D       E
   F       F       \skip 1 G       G
   -4
   f       g       A       B       C       D       E
   F       G       A       B       C       D       E
   F       G       A       B       C       D       E
   f_'     g'
}

botLyrics = \new Lyrics \lyricmode {
   \skip 1 A1              B       C       D       E
   F       \skip 1 \skip 1 G       \skip 1
   \skip 4 \skip 4 \skip 4
   a,,4    b,,     c,      d,      e,      f,      g,
   a,      b,      c       d       e       f       g
   a       b       c'      d'      e'      f_'     g'
%% \skip 4 \skip 4 \skip 4 \skip 4 \skip 4 \skip 4 \skip 4
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Treble clef
rangeT = \relative c {
   \relative c''' {
     r1
     { <a  a, a,> }
     { <b  b, b,> }
     { <c  c, c,> }
     { <d  d, d,> }
     { <e  e, e,> }
   \break
     r
     { <f, f, f,> }
     r
     r
     { <g  g, g,> }
   }

%% s
   \break
   \relative c {
     r4
     { <f> }
     { <g> }
     { <a> }
     { <b> }
     { <c> }
     { <d> }
     { <e> }
     { <f> }
     { <g> }
     { <a> }
     { <b> }
     { <c> }
     { <d> }
     { <e> }
     { <f> }
     { <g> }
     { <a> }
     { <b> }
     { <c> }
     { <d> }
     { <e> }
     r
     r
   }

   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Bass clef
rangeB = \relative c {
   \relative c, {
     r1
     { <a  a' a'> }
     { <b  b' b'> }
     { <c  c' c'> }
     { <d  d' d'> }
     { <e  e' e'> }
   \break
     { <f  f' f'> }
     r
     r
     { <g  g' g'> }
     r
   }

%% s
   \break
   \relative c, {
     r4
     r
     r
     { <a> }
     { <b> }
     { <c> }
     { <d> }
     { <e> }
     { <f> }
     { <g> }
     { <a> }
     { <b> }
     { <c> }
     { <d> }
     { <e> }
     { <f> }
     { <g> }
     { <a> }
     { <b> }
     { <c> }
     { <d> }
     { <e> }
     { <f> }
     { <g> }
   }

   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% PS/PDF output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
   \new PianoStaff
   {
     <<
       \topLyrics
       \new Staff
       {
         \signature
         \clef treble
         \rangeT
       }

       \midLyrics

       \new Staff
       {
         \signature
         \clef bass
         \rangeB
       }
       \botLyrics
     >>
   }
}
\paper
{
   between-system-padding = #1
   ragged-bottom = ##t
   ragged-last-bottom = ##t
   ragged-last = ##f
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% MIDI output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
   \new PianoStaff
   {
     <<
       \new Staff
       {
         \signature
         \instrument
         \rangeT
       }
       \new Staff
       {
         \signature
         \instrument
         \rangeB
       }
     >>
   }
   \midi
   {
     \tempo 4 = 52
   }
}
