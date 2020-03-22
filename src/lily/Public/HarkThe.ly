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
%%       This copyright applies only to this Lily representation, not to the
%%       score by Felix Mendelssohn or lyrics by Charles Wesley.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Title-
%%       HarkThe.ly
%%
%% Purpose-
%%       Hark! The Herald Angels Sing, Mendelssohn
%%
%% Last change date-
%%       2020/01/17
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
   title = "Hark! the Herald Angels Sing"
   composer = "Felix Mendelssohn (1809-1847)"
   poet = "Charles Wesley (1707-1788)"
   arranger = "William H. Cummings (1831-1915)"
   copyright = "Public Domain"
}

signature = {
    \key g \major
    \override Staff.TimeSignature #'style = #'()
    \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
    \time 4/4
}

%% softBreak = { \break }
softBreak = { \break }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% sInstrument = \set Staff.midiInstrument = "soprano sax"
%% aInstrument = \set Staff.midiInstrument = "alto sax"
%% tInstrument = \set Staff.midiInstrument = "tenor sax"
%% bInstrument = \set Staff.midiInstrument = "baritone sax"

%% For voice separation
sInstrument = \set Staff.midiInstrument = "acoustic grand"
bInstrument = \set Staff.midiInstrument = "voice oohs"
tInstrument = \set Staff.midiInstrument = "clarinet"
aInstrument = \set Staff.midiInstrument = "harmonica"

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sVoice = \relative {
   % Hark! the herald...
   \relative d' {
     d4 g g4. fis8 |                % 01
     g4 b b ( a ) |
     d d d4. ( c8 ) |
     b4  a b2 |
   }
   \softBreak

   % Peace on Earth...
   \relative d' {
     d4 g g4. fis8 |                % 05
     g4 b b ( a ) |
     d a a4. fis8 |
     fis4 e d2 |
   }
   \softBreak

   % Joyful all ye nations...
   \relative d' {                   % 09
     d4 d d g |
     c b b ( a ) |
     d d d g, |
     c b b ( a ) |
   }
   \softBreak

   % With th'angelic host...
   \relative d' {
     e4 e e e |                     % 13
     e e e2 |                       % 14
     a4 b8 ( c ) d4. g,8 |          % 15
     g4 a b2 |                      % 16
   }
   \softBreak

   % Hark! the herald...
   \relative d' {
     e4. e8 e4 e |                  % 17
     e e e2 |                       % 18
     a4 b8 ( c ) d4. g,8 |
     g4 a g2\fermata |
   }
   \softBreak
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
aVoice = \relative {
   % Hark! the herald...
   \relative d' {
     d4 d d4. d8 |                  % 01
     d4 g g ( fis ) |
     g4 fis e a |
     g  fis g2 |
   }
   \softBreak

   % Peace on Earth...
   \relative d' {
     d4 d d4. d8 |                  % 05
     b4 g' g2 |
     fis4 e d4. d8 |
     d4 cis d2 |
   }
   \softBreak

   % Joyful all ye nations...
   \relative d' {                   % 09
     d4 d d g |
     a g g ( fis ) |
     d d d g |
     a g g ( fis ) |
   }
   \softBreak

   % With th'angelic host...
   \relative e' {
     e4 e e d |                     % 13
     c b c2 |                       % 14
     fis4 fis g4. d8 |              % 15
     d4 fis g2 |                    % 16
   }
   \softBreak

   % Hark! the herald...
   \relative e' {
     e4.  e8 e4 d |                 % 17
     c b c2 |                       % 18
     fis4 fis g4. d8 |
     d4 d d2 |
   }
   \softBreak
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tVoice = \relative {
   % Hark! the herald...
   \relative b {
     b4 b b4. a8 |                  % 01
     b4 d d2 |
     d4 d c e |
     d  d d2 |
   }
   \softBreak

   % Peace on Earth...
   \relative b {
     b4 b b4. a8 |                  % 05
     g4 d' e2 |
     a,4 g fis4. a8 |
     b4 g fis2 |
   }
   \softBreak

   % Joyful all ye nations...
   \relative d {                    % 09
     d'4 d d d |
     d d d2 |
     d4 d d d |
     d d d2 |
   }
   \softBreak

   % With th'angelic host...
   \relative c' {
     c4  c c b |                    % 13
     a gis a2 |
     d4 d d4. b8 |                  % 15 tenor
     b4 d d2 |
   }
   \softBreak

   % Hark! the herald...
   \relative c' {
     c4. c8 c4 b |                  % 17
     a gis a2 |
     d4 d d4. b8 |
     b4 c b2 |
   }
   \softBreak
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
bVoice = \relative {
   % Hark! the herald...
   \relative g {
     g4 g g d |                     % 01
     b g d'2 |
     b4 b c c |
     d d g2 |
   }
   \softBreak

   % Peace on Earth...
   \relative g {
     g4 g g d |                     % 05
     e4 d cis4. ( a8 ) |
     b4 cis d fis, |
     g a d2 |
   }
   \softBreak

   % Joyful all ye nations...
   \relative d {                    % 09
     d'4 d d b |
     fis g d2 |
     d'4 d d b |
     fis g d2 |
   }
   \softBreak

   % With th'angelic host...
   \relative c {
     c4 c c c |                     % 13
     c e a2 |                       % 14
     c4 c b4. g8 |                  % 15
     d4 d g2 |
   }
   \softBreak

   % Hark! the herald...
   \relative c {
     c4. c8 c4 c |                  % 17
     c e a2 |
     c4 c b4. g8 |
     d4 d g,2 |
   }
   \softBreak
}

theLyrics = \new Lyrics \lyricmode {
   Hark!4 the her -- ald | an -- gels sing2 |
   "\"Glo"4 -- ry to the | new -- born King!2 |
   \softBreak

   Peace4 on earth, and | mer -- cy mild2 |
   God4 and sin -- ners | rec -- on -- "ciled.\""2 |
   \softBreak

  Joy4 -- ful, all ye | na -- tions, rise,2 |
  Join4 the tri -- umph | of the skies;2 |
   \softBreak

  With4 th'an -- gel -- ic | host pro -- claim,2 |
  "\"Christ"4 is born4. in8 | Beth4 -- le -- "hem.\""2 |
   \softBreak

   Hark!4. the8 her4 -- ald an -- gels sing,2 |
   "\"Glo"4 -- ry to4. the8 | new4 -- born "King!\""2 |
   \softBreak
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% PS/PDF output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
<<
   \new Staff
   {
     \signature
     \clef treble
     <<
       \sVoice
       \\
       \aVoice
     >>
   }
   \theLyrics
   \new Staff
   {
     \signature
     \clef bass
     <<
       \tVoice
       \\
       \bVoice
     >>
   }
>>
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
\score
{
%% \unfoldRepeats
   {
   <<
     \new Staff
     {
       \signature
       \clef treble
       <<
         \sInstrument
         \sVoice
       >>
     }
     \new Staff
     {
       \signature
       \clef treble
       <<
         \aInstrument
         \aVoice
       >>
     }
     \new Staff
     {
       \signature
       \clef bass
       <<
         \tInstrument
         \tVoice
       >>
     }
     \new Staff
     {
       \signature
       \clef bass
       <<
         \bInstrument
         \bVoice
       >>
     }
   >>
   }
   \midi
   {
     \tempo 4 = 144
   }
}
