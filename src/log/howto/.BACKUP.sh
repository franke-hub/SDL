#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2020-2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/license/mit)
##
## SPDX-License-Identifier: MIT
##----------------------------------------------------------------------------
##
## Title-
##       ~/src/log/howto/.BACKUP.sh
##
## Purpose-
##       SAMPLE: Run backup to remote machine
##
## Last change date-
##       2025/08/31
##
## Usage-
##       [cd /home/data/SDL]
##       .BACKUP.sh [OPTIONS...] hostname
##
## Local backup-
##       cd /home/data/ ; rdserver &
##       cd /E/Backups/data ; rdclient {-E}
##       killer rdserver
##
## Remote backup-
##       cd /home/data/
##       backup.raid {-del} {hostname}
##
## Implementation notes-
##       NOT VERIFIED
##
##############################################################################

##############################################################################
# Extract the parameters
opt=`backup opt $*`
rem=`backup rem $*`
set -- $rem

##############################################################################
# Verify parameters
if [[ -z "$1" ]] ; then
  echo "Missing destination host parameter"
  exit 1
fi

if [[ ! -z "$2" ]] ; then
  shift
  echo "Extra parameters: $*"
  exit 1
fi

##############################################################################
# Perform backup
set -x
rsync $opt /home/data/ $1:/home/data
