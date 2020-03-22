%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%%       Copyright (C) 2007-2020 Frank Eskesen.
%%
%%       This file is free content, distributed under cc by-sa version 3.0,
%%       with attribution required.
%%       (See accompanying file LICENSE.BY_SA-3.0 or the original contained
%%       within https://creativecommons.org/licenses/by-sa/3.0/us/legalcode)
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Title-
%%       AMinor01.ly
%%
%% Purpose-
%%       Untitled in A Minor
%%
%% Last change date-
%%       2020/01/17
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
   title = "Untitled in A minor"
   composer = "Frank Eskesen"
%  poet = "Frank Eskesen"
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% showLastLength = R1*8
softBreak = { \break }

signature = {
   \key a \minor
   \override Staff.TimeSignature #'style = #'()
   \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
   \time 6/4
}

%% Piano Right Hand
pRH = {
   \barNumberCheck #1
   \relative c' { c8 a c a c a d4 a8 a4. } |
   \relative c' { e8 d c d e c d4 a8 a4. } |
   \relative c' { c8 b a c b a g b a2 } |
%% \softBreak

%% Not 100% happy with rhythm here.
   \relative c' { <d fis a>4 <d fis a>8 <d fis a> b' a g fis <d g>2 } |
   \relative c' { c8 d e d2 g,8 e' d c b a1. } |

%% \softBreak s1. \softBreak s
%% \softBreak s1. \softBreak s
   \bar "|."
}

%% Piano Left Hand
pLH = {
   \barNumberCheck #1
   \relative c { <a c e>2. <a d fis>2. } |
   \relative c { <a c e>2. <a d fis>2. } |
   \relative c { <a c e>2 <g b e> <a c e> } |
%% \softBreak

   \relative c { <a d fis>1 <a c e>2~ } |
   \relative c { <a c e>4. <g b e>1 r8 } |
   \relative c { <a c e>1. } |

%% \softBreak s1. \softBreak s
%% \softBreak s1. \softBreak s
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
   ragged-bottom = ##t
   ragged-last-bottom = ##t
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
