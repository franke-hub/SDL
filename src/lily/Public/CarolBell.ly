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
%%       score by Mykola Dymtrovych Leontovych.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Title-
%%       CarolBell.ly
%%
%% Purpose-
%%       Carol of the Bells, Mykola Dymtrovych Leontovych.
%%
%% Last change date-
%%       2020/02/01
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
\header {
   title = "Carol of the Bells"
   composer = "Mykola Dmytrovych Leontovych (1877-1921)"
%% poet =
}

signature = {
   \key bes \major
   \override Staff.VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
   \time 3/4
}

sLyrics = \new Lyrics \lyricmode { %% The Singer's Lyrics (UNUSED)
   % Ching ching-a-ling ...
   % 4repeats
}

%% softBreak = { \break }
softBreak = {}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The Voices
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% sInstrument = \set Staff.midiInstrument = "soprano sax"
%% aInstrument = \set Staff.midiInstrument = "alto sax"
%% tInstrument = \set Staff.midiInstrument = "tenor sax"
%% bInstrument = \set Staff.midiInstrument = "baritone sax"
%% For separation of voices
sInstrument = \set Staff.midiInstrument = "acoustic grand"
bInstrument = \set Staff.midiInstrument = "voice oohs"
tInstrument = \set Staff.midiInstrument = "clarinet"
aInstrument = \set Staff.midiInstrument = "harmonica"

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sVoice = {
   \autoBeamOff
   \relative c'' { bes4--\pp a8-.^\markup { \italic (staccato) } bes-. g4-. } | % 01
   \relative c'' { bes4-- a8-. bes-. g4-. } |                % 02
   \relative c'' { bes4-- a8-. bes-. g4-. } |                % 03
   \relative c'' { bes4-- a8-. bes-. g4-. } |                % 04

   \repeat volta 2
   {
     % 12 repeats
     \relative c'' { bes4--\pp a8-. bes-. g4-. } |           % 05
     \relative c'' { bes4-- a8-. bes-. g4-. } |              % 06
     \softBreak
     \relative c'' { bes4^\markup { \italic simile } a8 bes g4 } | % 07
     \relative c'' { bes4 a8 bes g4 } |                      % 08
     \relative c'' { bes4\p a8 bes g4 } |                    % 09
     \relative c'' { bes4 a8 bes g4 } |                      % 10
     \relative c'' { bes4 a8 bes g4 } |                      % 11
     \relative c'' { bes4 a8 bes g4 } |                      % 12
     \relative c'' { bes4\mf a8 bes g4 } |                   % 13
     \relative c'' { bes4 a8 bes g4 } |                      % 14
     \relative c'' { bes4 a8 bes g4 } |                      % 15
     \relative c'' { bes4 a8 bes g4 } |                      % 16

     % 4 repeats
     \relative c'' { d'4\f c8 d bes4 } |                     % 17
     \relative c'' { d4 c8 d bes4 } |                        % 18
     \softBreak
     \relative c'' { d4 c8 d bes4 } |                        % 19
     \relative c'' { d4 c8 d bes4 } |                        % 20

     %
%%   \barNumberCheck #21
%%   \barNumberCheck #53
     \relative c'' { g'4\ff g8 g f[( ees )] } |              % 21
     \relative c'' { d4 d8 d c[( bes )] } |                  % 22
     \relative c'' { c4 c8 c c[( d )] } |                    % 23
     \relative c'' { g,4 g8 g g4 } |                         % 24

     % Merry, merry, ...
     \softBreak
     \relative c'' { d8\f e fis g a bes } |                  % 25
     \relative c'' { c8 d c4 bes } |                         % 26

     \relative c'' { d,8 e fis g a bes } |                   % 27
     \relative c'' { c8 d c4 bes } |                         % 28

     % 4 repeats
     \relative c'' { bes4-- a8-. bes-. g4-. } |              % 29
     \relative c'' { bes4-- a8-. bes-. g4-. } |              % 30
     \softBreak
     \relative c'' { bes4-- a8-. bes-. g4-. } |              % 31
     \relative c'' { bes4-- a8-. bes-. g4-. } |              % 32
   }

   \alternative
   {
     {
       \barNumberCheck #33
       \relative c'' { bes4\pp a8 bes g4 } |                % 33
       \relative c'' { bes4 a8 bes g4 } |                   % 34
       \relative c'' { bes4 a8 bes g4 } |                   % 35
       \relative c'' { bes4 a8 bes g4 } |                   % 36
     }
     {
%%     \barNumberCheck #37 %% incorrect
%%     \barNumberCheck #65
       \relative c'' { g2.~ \softBreak | g2.~ | g2.~ | g2.\ppp\fermata } | % 65
       \relative c'' { d'4\mf c8 d\fermata ees,4~ | ees2. } | % 69
%%     \barNumberCheck #43 %% incorrect
%%     \barNumberCheck #71
     }
   }
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
aVoice = {
   \autoBeamOff
   { r2. | r2. | r2. | r2. } |                             % 01

   \repeat volta 2
   {
     \relative c'' { g2.\pp | f | ees | d } |              % 05
     \relative c'  { g2.\p  | f | ees | d } |              % 09

     \relative c'  { g4--\mf g8-. g8-. g4-. } |            % 13
     \relative c'' { g4-- g8-. g8-. g4-. } |               % 14
     \relative c'' { g4^\markup { \italic simile } g8 g8 g4 } | % 15
     \relative c'' { g4 g8 g8 g4 } |                       % 16

     \relative c'' { bes4\f a8 bes g4 } |                  % 17
     \relative c'' { bes4 a8 bes g4 } |                    % 18
     \relative c'' { bes4 a8 bes g4 } |                    % 19
     \relative c'' { bes4 a8 bes g4 } |                    % 20

%%   \barNumberCheck #21
%%   \barNumberCheck #53
     \relative c'' { g4\ff g8 g8 g4 } |                    % 21
     \relative c'' { g4 g8 g8 g4 } |                       % 22
     \relative c'' { g4 g8 g8 g4 } |                       % 23
     \relative c'' { g4 g8 g8 g4 } |                       % 24

     \relative c'' { d2.\f | e4( fis) g } |                % 25
     \relative c'' { d2. | e4( fis) g } |                  % 27
     \relative c'' { d2. | c2. | f2. | ees2. } |           % 29
   }

   \alternative
   {
     {
       \barNumberCheck #33
       \relative c'  {  d2.~ | d2.~ | d2.  | r2. }  |      % 33
     }
     {
%%     \barNumberCheck #37
       \relative c'  {  d2.~ | d2.~ | d2.~ | d2.\ppp\fermata } % 37
       \relative c'  {  bes'4\mf a8 bes\fermata g4~ | g2. } | % 41
%%     \barNumberCheck #43
     }
   }
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tVoice = {
   \autoBeamOff
   { r2. | r2. | r2. | r2. } |                             % 01

   \repeat volta 2
   {
     \relative c'' {  r2. | r2. | r2. | r2. } |            % 05
     \relative c'' { g'2.\p | f | ees | bes } |            % 09

     \relative c'' { c4--\mf c8-. c8-. c4-. } |            % 13
     \relative c'' { d4-- d8-. d8-. d4-. } |               % 14
     \relative c'' { ees4^\markup { \italic simile } ees8 ees8 ees4 } | % 15
     \relative c'' { d4 d8 d8 d4 } |                       % 16

     \relative c'' { d2.\f | e2. } |                       % 17
     \relative c'' { f4( ees) d } |                        % 19
     \relative c'' { g8[( f)] ees4 d } |                   % 20

%%   \barNumberCheck #21
%%   \barNumberCheck #53
     \relative c'' { d4\ff ees8 ees d[( c)] }              % 21
     \relative c'' { d4 d8 d d4 } |                        % 22
     \relative c'' { ees4 ees8 ees f[( ees )] }            % 23
     \relative c'' { d4 d8 d d4 } |                        % 24

     \relative c'' { bes4\ff a8 bes g4 } |                 % 25
     \relative c'' { bes4 a8 bes g4 } |                    % 26
     \relative c'' { bes4 a8 bes g4 } |                    % 27
     \relative c'' { bes4 a8 bes g4 } |                    % 28
     \relative c'' { g2. | g2. | g2. | g2. } |             % 29
   }

   \alternative
   {
     {
       \barNumberCheck #33
       \relative c'' { g2.~ | g2.~ | g2.  | r2. }  |       % 33
     }
     {
%%     \barNumberCheck #37
       \relative c'' { bes4 a8 bes g4 } |                  % 37
       \relative c'' { bes4 a8 bes g4 } |                  % 38
       \relative c'' { bes4 a8 bes g4 } |                  % 39
       \relative c'' { bes4 a8 bes g4\fermata } |          % 40
       \relative c'' { r2. | r2. } |                       % 41
%%     \barNumberCheck #43
     }
   }
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
bVoice = {
   \autoBeamOff
   { r2. | r2. | r2. | r2. } |                             % 01

   \repeat volta 2
   {
     \relative c   {  r2. | r2. | r2. | r2. } |            % 05
     \relative c   {  r2. | r2. | r2. | r2. } |            % 09

     \relative c   { ees4--\mf ees8-. ees-. ees4-. } |     % 13
     \relative c   { g4-- g8-. g8-. g4-. } |               % 14
     \relative c'  { c4^\markup { \italic simile } c8 c8 c4 } | % 15
     \relative c'  { g4 g8 g8 g4 } |                       % 16

     \relative c'  { g4\f g8 g8 g4 } |                     % 17
     \relative c'  { g4 g8 g8 g4 } |                       % 18
     \relative c'  { g4 g8 g8 g4 } |                       % 19
     \relative c'  { g4 g8 g8 g4 } |                       % 20

%%   \barNumberCheck #21
%%   \barNumberCheck #53
     \relative c'  { bes4\ff a8 bes g4 } |                 % 21
     \relative c'  { bes4 a8 bes g4 } |                    % 22
     \relative c'  { bes4 a8 bes g4 } |                    % 23
     \relative c'  { bes4 a8 bes g4 } |                    % 24

     \relative c'  { d2.\ff | d2. |  d2. | d2( e4) } |     % 25
     \relative c   { f2. | ees2. | d2. | c2. } |           % 29
   }

   \alternative
   {
     {
       \barNumberCheck #33
       \relative c   { g2.~ | g2.~ | g2.\ppp  | r2. }  |   % 33
     }
     {
%%     \barNumberCheck #37
       \relative c   { g2.~ | g2.~ | g2.~ | g2.\ppp\fermata } | % 37
       \relative c   { r2. | g2.\f\fermata } |             % 41
%%     \barNumberCheck #43
     }
   }
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% PS/PDF output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\score
{
   \new GrandStaff
   <<
     \new Staff
     {
       \signature
       \clef treble
       \sVoice
     }
     \new Staff
     {
       \signature
       \clef treble
       \aVoice
     }

     \new Staff
     {
       \signature
       \clef treble
       \tVoice
     }

     \new Staff
     {
       \signature
       \clef bass
       \bVoice
     }
   >>
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
\score
{
   \new GrandStaff
   {
   \unfoldRepeats
   <<
     \new Staff
     {
       \signature \clef treble  %% This is needed on ONE Staff only
       <<
         \sInstrument
         \sVoice
       >>
     }
     \new Staff
     {
       <<
         \aInstrument
         \aVoice
       >>
     }
     \new Staff
     {
       <<
         \tInstrument
         \tVoice
       >>
     }
     \new Staff
     {
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
%{ COMMENT >>>> %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% <<<< COMMENT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %}
