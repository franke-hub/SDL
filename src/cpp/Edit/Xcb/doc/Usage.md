# Editor documentation

Copyright (C) 2022 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

Last change date: 2022/04/11

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
Each parameter is the name of a file to be edited.

## General usage notes
The TAB key tabs the cursor. Shift-TAB reverse-tabs the cursor.

Use Alt-\,Backspace to type a '\b' (0x08) character.
Use Alt-\,TAB to type a '\t' (0x09) character.
Use Alt-\,ESC to type a '\e' (0x1B) character.

When any line is changed, it is saved with trailing blanks removed.
If the last line has no delimiter and is changed, its delimiter is changed to
'\n' or "\r\n" depending upon the file mode.

Move operations are divided in two parts[1]: cut, then paste.
The paste operation can be repeated using Ctrl-V.
This implies that, for move, UNDO is also two operations.

## Editor clip (Internal clipboard)
Currently the internal clipboard and the system clipboard are maintained
separately.

The editor DOES NOT USE the system clipboard (yet.)
Using the system clipboard is on the to-do list, but isn't a high priority.

------------------------------------------------------------------------------

## Editor command guide (Command names are not case sensitive)
<dl>
<dt>BOT:   </dt><dd>Move the cursor to the last editor line (column 0)</dd>
<dt>C:     </dt><dd>(Change) example: `C /find string/replace string/`
(Searching begins at the *current* character)</dd>
<dt>D:     </dt><dd>Debug \{option\}</dd>
<dt>DEBUG: </dt><dd>(Alias of D)</dd>
<dt>E:     </dt><dd>Edit another set of files
(Only the file name can contain wildcards. The path name cannot.)</dd>
<dt>EDIT:  </dt><dd>(Alias of E)</dd>
<dt>EXIT:  </dt><dd>Exit (safely.)
If no files are changed, close the Editor.</dd>
<dt>FILE:  </dt><dd>Save and close the current file</dd>
<dt>L:     </dt><dd>(Locate) example: `L /find string/`
(Searching begins at the *next* character)</dd>
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
<dt>Alt-\: </dt><dd>Escape the next character, including BS, TAB, and ESC.</dd>
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

-------- Footnotes -----------------------------------------------------------

[1]: Why split move into cut and paste?
The quick answer is that it simplicifies REDO and UNDO logic.
Move can move lines or blocks from one file to another.
Since REDO and UNDO are file-specific operations, it would be impossible to
REDO or UNDO a file to file move after one of the files was closed.