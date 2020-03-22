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
%%       GardenSong.ly
%%
%% Purpose-
%%       Garden song, Frank Eskesen
%%
%% Last change date-
%%       2020/01/17
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
   title = "Garden Song"
   composer = "Frank Eskesen"
%% poet = "Frank Eskesen"
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% showLastLength = R1*8
softBreak = { \break }

%% The Singer's Lyrics
sLyric = \new Lyrics \lyricmode {
   \set associatedVoice = #"melody"
   \barNumberCheck #1
   \bar "|."
}

%% The Singer's voice
sVoice = \new Voice = "melody" \relative c' {
   \barNumberCheck #1
   \bar "|."
}

signature = {
   \key c \major
   \override Staff.TimeSignature #'style = #'()
   \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
   \time 4/4
}

%% Piano Right Hand
pRH = {
   \barNumberCheck #1
   \relative c' { f8 c c c f c a' c, } |
   \relative c' { e8 c c c e c g' c, } |
   \relative c' { d8 b b b d b g' b, } |
   \relative c' { c4. e8 g4 c,4 } |
   \softBreak

   \relative c' { f4. f8 a g f a } |
   \relative c' { g'2 e } |
   \relative c' { d4. d8 g d b g' } |
   \relative c' { c2 e } |

   \relative c' { f4. f8 a g f a } |
   \relative c' { g'2 e } |
   \relative c' { d4. d8 g d b g' } |
   \relative c' { c1 } |

   \bar "|."
}

%% Piano Left Hand
pLH = {
   \barNumberCheck #1
   \relative c  { c8 a' f a c, a' f a } |
   \relative c  { c8 g' e g c, g' e g } |
   \relative c  { b8 g' d g b, g' d g } |
   \relative c  { c8 g' e g c, g' e g } |

   \relative c  { c8 a' f a c, a' f a } |
   \relative c  { c8 g' e g c, g' e g } |
   \relative c  { b8 g' d g b, g' d g } |
   \relative c  { c8 g' e g c, g' e g } |

   \relative c  { c8 a' f a c, a' f a } |
   \relative c  { c8 g' e g c, g' e g } |
   \relative c  { b8 g' d g b, g' d g } |
   \relative c  { c8 g' e g c,2 } |

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
