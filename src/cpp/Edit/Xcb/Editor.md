# Editor documentation

Copyright (C) 2021 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

Last change date: 2021/03/03

------------------------------------------------------------------------------

The Editor is a WYSIWYG editor that uses XCB functionality to control the
screen, keyboard, and mouse.
Since it's based on XCB, it only operates under Cygwin or Linux.
Some of the XCB functionality is offloaded into the GUI library, part of the
code distribution package containing.
The Editor also uses the PUB library.

It's derived from ~/src/cpp/Edit, also a WYSIWYG editor.
This editor uses the screen, keyboard and other functions provided in the
COM library, but does not use the mouse.
The COM library uses Windows(TM) functions and Linux ncurses for screen and
keyboard control.

(*WYSIWYG*: What you see is what you get.)

------------------------------------------------------------------------------


## The editor command line
The parameters are the list of files to be edited.

## General usage notes
The TAB key moves the cursor.
Ctrl-TAB inserts a '\t' into the file.

When any line is changed, it is saved with trailing blanks removed.
If the last line has no delimiter and is changed, its delimiter is changed to
'\n' or "\r\n" depending upon the file mode.

Copy and move operations (Alt-C and Alt-M) are divided in two[1],
either copy or cut, then paste.
The paste operation can be repeated using Ctrl-V.
This implies that, for move, UNDO is also two operations.
Once created, a copy list is only replaced. It is never deleted.

## Editor clip (Internal clipboard)
Currently the internal clipboard and the system clipboard are maintained
separately.
The editor DOES NOT USE the system clipboard (yet.)

------------------------------------------------------------------------------

## Editor command guide (Command names are not case sensitive)
<dl>
<dt>BOT:   </dt><dd>Move the cursor to the last editor line (column 0)</dd>
<dt>C:     </dt><dd>(Change) example: `C /find string/replace string/`
(Find search begins at *current* character)</dd>
<dt>D:     </dt><dd>Debug \{option\}</dd>
<dt>DEBUG: </dt><dd>(Alias of D)</dd>
<dt>E:     </dt><dd>Edit another set of files
(Only the file name can contain wildcards. The path name cannot.)</dd>
<dt>EDIT:  </dt><dd>(Alias of E)</dd>
<dt>EXIT:  </dt><dd>Exit (safely.)
If no files are changed, close the Editor.</dd>
<dt>FILE:  </dt><dd>Save and close the current file</dd>
<dt>L:     </dt><dd>(Locate) example: `L /find string/`
(Find search begins at *next* character)</dd>
<dt>QUIT:  </dt><dd>(Unconditionally) close the current file</dd>
<dt>SAVE:  </dt><dd>Save the current file, leaving it active
(The UNDO/REDO lists are deleted. There is no UN-SAVE.)</dd>
<dt>SET:   </dt>
<dd>Set an option (Example: `set mixed on`)

<dl>
<dt>MIXED: </dt><dd>Set locate mixed case mode (ON or off)</dd>
<dt>PRIOR: </dt><dd>Set locate prior mode (ON or off)</dd>
<dt>WRAP:  </dt><dd>Set locate wrap mode (ON or off)</dd>
<dt>MODE:  </dt><dd>Change all file line delimiters (DOS or UNIX)</dd>
</dl>
</dd>
<dt>SORT:  </dt><dd>Sort the file list (by name)</dd>
<dt>TOP:   </dt><dd>Move the cursor to the first editor line (column 0)</dd>
</dl>

------------------------------------------------------------------------------

## Editor key bindings
<dl>
<dt>Alt-B: </dt><dd>Mark block  (Column)</dd>
<dt>Alt-C: </dt><dd>Copy mark   (lines or block)
(This replaces internal Editor clipboard)</dd>
<dt>Alt-D: </dt><dd>Delete mark (lines or block)
(This replaces internal Editor clipboard)</dd>
<dt>Alt-I: </dt><dd>Insert line</dd>
<dt>Alt-J: </dt><dd>Join the cursor and next lines</dd>
<dt>Alt-L: </dt><dd>Mark line</dd>
<dt>Alt-M: </dt><dd>Move mark   (lines or block)
(This replaces internal Editor clipboard)</dd>
<dt>Alt-Q: </dt><dd>Quit (safely.) Close the file if it's unchanged.</dd>
<dt>Alt-S: </dt><dd>Split the current line (at the cursor column)</dd>
<dt>Alt-U: </dt><dd>Undo mark</dd>
<br>
<dt>Ctrl-C: </dt><dd>Copy mark   (First half of Alt-C or Alt-M)</dd>
<dt>Ctrl-V: </dt><dd>Paste copy  (Second half of Alt-C or Alt-M)</dd>
<dt>Ctrl-X: </dt><dd>Delete mark (Same as Alt-D)</dd>
<br>
<dt>ESC:   </dt><dd>Switch between command and data mode</dd>
<dt>F1:    </dt><dd>Help message (to stdout)</dd>
<dt>F2:    </dt><dd>(No operation)</dd>
<dt>F3:    </dt><dd>Quit (safely.) Close the file if it's unchanged.</dd>
<dt>F4:    </dt><dd>Move focus to next changed file</dd>
<dt>F5:    </dt><dd>Repeat locate</dd>
<dt>F6:    </dt><dd>Repeat change</dd>
<dt>F7:    </dt><dd>Move focus to previous file</dd>
<dt>F8:    </dt><dd>Move focus to next file</dd>
<dt>F9:    </dt><dd>(No operation)</dd>
<dt>F10:   </dt><dd>Move current line to top of screen</dd>
<dt>F11:   </dt><dd>UNDO (Note: Cannot UNDO a save operation)</dd>
<dt>F12:   </dt><dd>REDO (Note: Change after UNDO deletes REDO list)</dd>
</dl>

#### Synonyms

* Alt-D and Ctrl-X

* Alt-Q and F3

------------------------------------------------------------------------------

## Implementation notes

#### The no-delimiter line
Only the last line in the file can be the no delimiter line.
This occurs when the last character in the file is not a newline.

This line remains without a delimiter unless it is split, joined, or lines are
added after it. If any of these occur, the line is changed so that it
has a text file delimiter.
If the no delimiter line is simply modified, the modified line remains the
no delimiter line.

#### Undo/Redo operation
All operations that change a file can be un-done and re-done.
As a special case, UNDO for a line being changed cannot be re-done.
(Maybe this should change. It's easy enough.)

UNDO/REDO logic is complicated by the fact that there are two parts to
REDO and UNDO: Both the file itself changes and the line mark changes.
The file changes are internally recorded in REDO/UNDO lists.
Mark changes are not recorded, so the mark state changes separately from the
file change.
This means that when a file UNDO or REDO operation occurs, we need to *infer*
what mark changes are required.

#### Undo/Redo operation mark considertions
There are four line changing operations: COMMIT, INSERT, SPLIT, and JOIN.
COMMIT and INSERT are complicated slightly because the no-delimiter line might
also need to be changed.[2]

For line REDO or UNDO operations, inserted lines are marked if either

1. They are within a mark. (mark_head and mark_tail do not change.)
2. Any removed lines were marked. (mark_head and/or mark_tail change.)

They are unmarked otherwise.

There are four group changing operations: CUT-LINE, PASTE-LINE, CUT-COLS, and
PASTE-COLS.
PASTE-LINE is complicated slightly because the no-delimiter line might need to
be changed too.

For CUT REDO and PASTE UNDO operations,
if any of the removed lines are marked the entire mark is removed.
Otherwise, it's unchanged.

For PASTE REDO and CUT UNDO operations,
any existing mark is removed and the PASTE lines become the new mark.

To differentiate CUT and PASTE from line operations, the REDO rh_col field
is alway made positive. The lh_col field is -1 for CUT/PASTE line operations.

To differentiate CUT/PASTE redo from CUT/PASTE undo, the REDO rh_col and
lh_col fields are inverted for undo. The redo logic checks this.

#### Memory leaks
Once text for a file has been allocated, it's not released until the Editor
program exits. This is by design.

First, files are loaded in one operation. They are then parsed into lines,
replacing the ending newline character with the NUL ('\0') character.
No space or time is wasted in malloc/free (or new/delete) overhead.
All allocated text is immutable. This allows pointers to text to be freely
copied (e.g. in REDO and UNDO lists) without reference counters.

Second, if reference counters (or strings) were used to control deallocation,
these counters would themselves take up large amounts of space. In a text
editor, there are usually a lot more unchanged than changed lines.
Allocation overhead in reference counting and boundary alignment will usually
far exceed the space lost because deallocated text isn't reclaimed.

------------------------------------------------------------------------------

## Debugging/Maintenance information

<h4>
Some Cygwin(TM) features not available or not working.
(They operate properly on Linux.)
</h4>

* Config::backtrace()
No output

* setlocale(LC_NUMERIC, ""); printf("%'d\n", 123456789);
Missing commas.

#### For internal trace type information use: "rgrep trace\("
*rgrep*, recursive grep, is a script found in the *bat* subdirectory provided
as a part of the distribution.

When issued in the source tree, internal trace calls are listed.
(These change too frequently to be documented here.)

------------------------------------------------------------------------------

#### EdMark.cpp undo/redo logic working notes.
My normal development method is to just do it.
Sometimes, though, a modification turns out to be more difficult than expected.
This occurred when trying to update EdMark.cpp to coordinate mark undo/redo
with EdFile.cpp's file undo/redo.
The file undo/redo logic is simpler in that the undo/redo list information
always matches the state of the file itself.
The mark undo/redo logic is more complicated because the mark state does not
necessarily match the undo/redo file state, and
trying to "just do" the synchronization wasn't working.
The first approach tried wasn't working consistently. Neither was the second.

A more thoughtful approach was needed, and this documents by example how to
use visualization to clarify thinking.
These are the working notes used trying to figure out what to do.
It's a list of all the redo/undo operations and their desired outcomes.

     COMMIT: (Want: only mark if already entirely within mark)

     INSERT: (Want: only mark if already entirely within mark)
       UNDO is DELETE operation, like cut but does not always remove mark.
       Info: REDO cannot change mark head/tail, but UNDO might.
       Normal case:
         Remove None; Insert empty line.
         REDO: REM(Null) => INS("\0") (Special case)
         UNDO: INS("\0") => REM(Null) (Special case)
       Special case: NO-DELIM line
         Remove NO-DELIM; Insert empty line.
         REDO: REM(NoDL) => INS(*TWO) (Special case, delimiter change)
         UNDO: INS(*TWO) => REM(NoDL) (Special case)
       ?? HOWTO DIFFERENTIATE INSERT EMPTY LINE WITH PASTE EMPTY LINE ??
       ?? How about a different empty line for Insert ??
       ?? How about an EdLine ins/split/join flag ??
       ?? How about using the EdRedo columns somehow ??

     JOIN:   (Want: joined line marked if first line marked.)
       Handle: mark, possible head/tail change
       NO-DELIM: not relevant
       Remove two lines; Insert one line
       REDO: REM(*TWO) => INS(*ONE)
       UNDO: INS(*ONE) => REM(*TWO)

     SPLIT:  (Want: Both lines marked if line marked)
       Handle: mark, possible head/tail change
       NO-DELIM: not relevant.
       Remove one line; Insert two lines

     CUT LINES:
       No insert, only remove
       REDO: REM(Mark) => INS(Null) (No mark remains)
       UNDO: INS(Null) => REM(----) (SET and replace mark with REM)

     PASTE LINES:
       No remove, only insert
       ((Unconditionally replaces mark.))
       Special case: if after NO-DELIM line. It's removed.
       REDO: REM(Null) => INS(Mark) (SET and replace mark with INS)
       UNDO: INS(----) => REM(Null) (Remove  mark)

     CUT BLOCK:
       ?? Can the mark state in the logical remove be used as an indicator
       to determine whether a REDO/UNDO is a CUT or a PASTE ??
       Logic: REDO:
           Use INS to determine whether mark exists or not
           Set REM to invert INS state
       Logic: UNDO:
           Use REM to determine whether mark exists or not
           Set INS to invert REM state
       Handle: file, cursor changed.
       COLUMNS APPLY TO REM
       REDO: REM(----) => INS(*CLR) [SET all REM] (No mark remains)
       UNDO: INS(----) => REM(*SET) [CLR all INS] (Replace mark)

     PASTE BLOCK:
       Handle: file, cursor changed.
       COLUMNS APPLY TO INS
       REDO: REM(----) => INS(*SET) [CLR all REM] (Replace mark)
       UNDO: INS(----) => REM(*CLR) [SET all INS] (No mark remains)

Old insights:

* REDO/UNDO are inverse operations. Only one implementation needed.

* INSERT/DELETE handled differently from PASTE/CUT

New insights:

* Line operations COMMIT, INSERT, DELETE, JOIN, and SPLIT handled similarly
whether or not they are REDO or UNDO.

* CUT/PASTE handled similarly.

Debugging additions:

* Needed to differentiate CUT/PASTE redo and undo operations better.
(Without the column inversion in handle_undo, handle_redo couldn't
differentiate between paste redo and undo.)

* Move block left needs to adjust cursor column when starting right of mark.

-------- Footnotes -----------------------------------------------------------

[1]: Why split move into cut and paste?
The quick answer is that it simplicifies REDO and UNDO logic.
Move can move lines or blocks from one file to another.
Since REDO and UNDO are file-specific operations, it would be impossible to
REDO or UNDO a file to file move after one of the files was closed.

[2]: Split and join operations may modify the no-delimiter
line as part of their normal operation. It's not a special case.

