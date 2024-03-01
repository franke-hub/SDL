##############################################################################
##
##       Copyright (C) 2019 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Global.py
##
## Purpose-
##       Global data container.
##
## Last change date-
##       2019/09/04
##
##############################################################################

##############################################################################
##
## Class-
##       Global
##
## Purpose-
##       Global information
##
## Implementation notes-
##       The default attributes control library tests.
##       Applications may use this class for other purposes.
##
##############################################################################
class Global:
    HCDM    = False                 ## Hard Core Debug Mode?
    ## USAGE: When True, extensive debugging logging added.

    TESTING = None                  ## Specific test control
    ## USAGE: Include TESTING in a list possibile tests normally omitted.

    VERBOSE = 1                     ## Output verbosity control
    ## USAGE: The greater the abs(VERBOSE) > N, the more output that occurs.
    ##   Values < 0 indicate more tests in a group.
