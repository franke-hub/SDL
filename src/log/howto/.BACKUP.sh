#!/bin/bash
##
## Title-
##       /home/data/.BACKUP.sh
##
## Purpose-
##       Run backup to remote machine
##
## Usage-
##       .BACKUP.sh [OPTIONS...] hostname
##
## Local backup-
##         Windows: rdserver; /E/Backups/data rdclient
##         MyLinux: backup.raid data
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
