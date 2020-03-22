%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%%       Copyright (c) 2007-2020 Frank Eskesen.
%%
%%       This file is free content, distributed under cc by-sa version 3.0,
%%       with attribution required.
%%       (See accompanying file LICENSE.BY_SA-3.0 or the original contained
%%       within https://creativecommons.org/licenses/by-sa/3.0/us/legalcode)
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Title-
%%       One-2-3-One.ly
%%
%% Purpose-
%%       1, 2, 3 in one.
%%
%% Last change date-
%%       2020/01/17
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
   title = "One Two Three, One"
   composer = "Frank Eskesen"
%  poet = "Frank Eskesen"
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% showLastLength = R1*8
softBreak = { \break }

signature = {
   \key g \major
   \override Staff.TimeSignature #'style = #'()
   \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
   \time 3/8
}

%% Piano Right Hand
pRH = {
   \barNumberCheck #1
   \relative c' { <g' b d>8 <g b d> <g b d> } |
   \relative c' { <fis a d>8 <fis a d> <fis a d> } |
   \relative c' { <g' b d>8 <g b d> <g b d> } |
   \relative c' { <fis a d>4. } |

   \softBreak s4. \softBreak s
   \softBreak s4. \softBreak s
   \bar "|."
}

%% Piano Left Hand
pLH = {
   \barNumberCheck #1
   s4. s s s s s s s

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
     \new Staff
     {
       \signature
       \clef treble
       \pRH
     }
     \new Staff
     {
       \signature
       \clef bass
       \pLH
     }
   >>
   }
}
\paper
{
   between-system-padding = #1
   ragged-bottom = ##f
   ragged-last-bottom = ##f
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% MIDI output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sInstrument = \set Staff.midiInstrument = "voice oohs"
sInstrument = \set Staff.midiInstrument = "synth voice"
sInstrument = \set Staff.midiInstrument = "acoustic grand"
sInstrument = \set Staff.midiInstrument = "choir aahs"
\score
{
   \unfoldRepeats
   {
     \new PianoStaff
     {
     <<
       \new Staff
       {
         \signature
         \pRH
       }
       \new Staff
       {
         \signature
         \pLH
       }
     >>
     }
   }
   \midi
   {
     \tempo 4 = 112
   }
}
