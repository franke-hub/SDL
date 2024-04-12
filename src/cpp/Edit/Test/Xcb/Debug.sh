#!/bin/bash
uuid=e743e3ac-6816-4878-81a2-b47c9bbc2d37
~/bin/fsdump   ~/.config/uuid/$uuid/trace.out >~/.config/uuid/$uuid/debug.log
~/bin/edit.old ~/.config/uuid/$uuid/debug.*
