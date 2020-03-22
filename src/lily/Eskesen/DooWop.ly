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
%%       DooWop.ly
%%
%% Purpose-
%%       Doo wop beat.
%%
%% Last change date-
%%       2007/01/01
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
   title = "Doo Wop"
   composer = "Frank Eskesen"
%  poet = "Frank Eskesen"
%% Similar to "Talk about the boy from New York City"
%% Written first, but not published.
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% showLastLength = R1*8
softBreak = { \break }

signature = {
   \key c \major
%  \override Staff.TimeSignature #'style = #'()
   \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
}

cRH = {
   \relative c' { <c e g>4 <c e g> <c f a> <c f a> } |
   \relative c' { <c g' bes>4 <c g' bes> <f a>8 <c> <f a> <c> } |
   \relative c' { <c e g>4 <c e g> <c f a> <c f a> } |
   \relative c' { <c g' bes>4 <c g' bes> <f a>8 <c> <f a> <c> } |
}

cLH = {
   \relative c  { <c e g>2 <c f a> } |
   \relative c  { <c g' bes>2 <c f a> } |
   \relative c  { <c e g>2 <c f a> } |
   \relative c  { <c g' bes>2 <c f a> } |
}

%% Piano Right Hand
pRH = {
   \barNumberCheck #1
   \cRH
   \softBreak
   \transpose c d \cRH

   \softBreak s1 \softBreak s
   \softBreak s1 \softBreak s
   \bar "|."
}

%% Piano Left Hand
pLH = {
   \barNumberCheck #1
   \cLH
   \transpose c d \cLH

   s1 s s s

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
     \tempo 4 = 144
   }
}
