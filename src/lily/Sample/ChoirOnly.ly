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
%%       ChoirOnly.ly
%%
%% Purpose-
%%       Choir only layout, needs work.
%%
%% Last change date-
%%       2007/01/01
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\version "2.8.8"
global = {
   \key c \major
   \time 4/4
}

sopMusic = \relative c'' {
   c4 c c8[( b)] c4
}
sopWords = \lyricmode {
   hi hi hi hi
}

altoMusic = \relative c' {
   e4 f d e
}
altoWords =\lyricmode {
   ha ha ha ha
}

tenorMusic = \relative c' {
   g4 a f g
}
tenorWords = \lyricmode {
   hu hu hu hu
}

bassMusic = \relative c {
   c4 c g c
}
bassWords = \lyricmode {
   ho ho ho ho
}

\score {
   \new ChoirStaff <<
      \new Lyrics = sopranos { s1 }
      \new Staff = women <<
         \new Voice =
           "sopranos" { \voiceOne << \global \sopMusic >> }
         \new Voice =
           "altos" { \voiceTwo << \global \altoMusic >> }
      >>
      \new Lyrics = "altos" { s1 }
      \new Lyrics = "tenors" { s1 }
      \new Staff = men <<
         \clef bass
         \new Voice =
           "tenors" { \voiceOne <<\global \tenorMusic >> }
         \new Voice =
           "basses" { \voiceTwo <<\global \bassMusic >> }
      >>
      \new Lyrics = basses { s1 }

      \context Lyrics = sopranos \lyricsto sopranos \sopWords
      \context Lyrics = altos \lyricsto altos \altoWords
      \context Lyrics = tenors \lyricsto tenors \tenorWords
      \context Lyrics = basses \lyricsto basses \bassWords
   >>

   \layout {
      \context {
         % a little smaller so lyrics
         % can be closer to the staff
         \Staff
         \override VerticalAxisGroup #'minimum-Y-extent = #'(-3 . 3)
      }
   }
}
