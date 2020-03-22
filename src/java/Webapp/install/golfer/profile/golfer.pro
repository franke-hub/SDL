##############################################################################
##
##       Copyright (C) 2008-2020 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
##
## Filename-
##       golfer.pro
##
## Purpose-
##       Golfer properties file.
##
## Last change date-
##       2020/01/16
##
##############################################################################

##############################################################################
# Property: status-url
#  Content: The URL for the status Servlet
#  Default: None
#    Notes: Not used -- for demonstration purposes only.
status-url=http://localhost:8080/golfer/Status

##############################################################################
# Property: GolferServlet.database-path
#  Content: The path to the GolferServlet database
#  Default: .
database-path=/home/data/web/database/golfer

##############################################################################
# Property: GolferServlet.database-name
#  Content: The name of the GolferServlet database
#  Default: GOLFER.DB
database-name=GOLFER.DB

##############################################################################
# Property: GolferServlet.database-port
#  Content: The port used for remote database access
#  Default: 65025 (Defined in DbCommon.PORT)
# database-port=65025

##############################################################################
# Property: GolferServlet.verbose
#  Content: Debugging verbosity
#  Default: 0
verbose=9

