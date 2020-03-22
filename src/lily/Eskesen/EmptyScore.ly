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
%%       EmptyScore.ly
%%
%% Purpose-
%%       Empty score. *** NOT UP TO DATE ***
%%
%% Last change date-
%%       2007/01/01
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
%  title = "Empty Score"
%  composer = "Frank Eskesen"
%  poet = "Frank Eskesen"
}

%% showLastLength = R1*4
softBreak = { \break }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Singer's Lyrics
sLyric = \new Lyrics \lyricmode {
   \set associatedVoice = #"melody"
   \barNumberCheck #1

   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
signature = {
   \key c \major
%% \override Staff.TimeSignature #'style = #'()
%% \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
%% \time 4/4
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Singer's voice
sVoice = \new Voice = "melody" {
   \barNumberCheck #1

   \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Piano Right Hand
pRH = {
   \barNumberCheck #1
   s1
   \break s1
   \break s1
   \break s1
   \break s1
   \break s1

   \break s1
   \break s1
   \break s1
   \break s1
   \break s1
   \break s1

%% \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Piano Left Hand
pLH = \relative c  {
   \barNumberCheck #1
   s1
   \break s1
   \break s1
   \break s1
   \break s1
   \break s1

   \break s1
   \break s1
   \break s1
   \break s1
   \break s1
   \break s1

%% \bar "|."
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% PS/PDF output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
   \new GrandStaff
   {
   <<
%{
     \new Staff
     {
     <<
       \signature
       \clef treble
       <<
         \sVoice
         \sLyric
       >>
     >>
     }
%}

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
   >>
   }
}
\paper
{
   between-system-padding = #1
   ragged-bottom = ##t
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
     \new GrandStaff
     {
     <<
       \new Staff
       {
         \signature
         \sInstrument
         \sVoice
       }
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
     >>
     }
   }
   \midi
   {
     \tempo 4 = 112  %% Normal tempo
   }
}
