##############################################################################
#
# Filename-
#        testreader.pro
#
# Purpose-
#        Reader properties file.
#
##############################################################################

##############################################################################
# Property: ControlServlet::debug
# Property: MainLogger::debug
#  Content: Debugging verbosity
#  Default: 0
debug=99

##############################################################################
# Property: NewsReader::database-path
#  Content: The home directory for all NewsReader databases
#  Default: .
# database-path=/home/data/web/database/reader
database-path=data

##############################################################################
# Property: MainLogger::logfile
#  Content: Logfile name
#  Default: logfile.out
# logfile=nul:

##############################################################################
# Property: MainLogger::max-article-count
#  Content: The maximum number of articles to be read
#  Default: 0 (no limit)
max-article-count=500

##############################################################################
# Property: NewsReader::news-groups-file
#  Content: The name of the news group database
#  Default: groups.all
news-groups-file=groups.all

##############################################################################
# Property: NewsReader::news-server-file
#  Content: The name of the news server database
#  Default: article.db
news-server-file=article.db

##############################################################################
# Property: NewsReader::news-server-name
#  Content: The name of the news server
#  Default: 127.0.0.1

##############################################################################
# Property: NewsReader::news-server-port
#  Content: The port of the news server
#  Default: 119
news-server-port=65025

##############################################################################
# Property: NewsReader::refresh-interval
#  Content: The number of seconds between database updates
#  Default: 1800 (30 minutes)
refresh-interval=1800
refresh-interval=30

##############################################################################
# Property: MainReader::runtime
#  Content: The number of seconds to operate the NewsReader
#  Default: 30
runtime=28800
runtime=30

##############################################################################
# Property: NewsReader::subs-groups-file
#  Content: The name of the news group subscription database
#  Default: groups.sub
subs-groups-file=groups.sub

##############################################################################
# Property: NewsReader::subs-groups-zero
#  Content: Boolean: Should the subgroups be reset?
#  Default: false
subs-groups-zero=true

