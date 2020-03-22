#!/bin/python3
##############################################################################
##
##       Copyright (C) 2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Main.py
##
## Purpose-
##       HTTP client
##
## Last change date-
##       2018/01/01
##
## Usage-
##       ./Main.py
##
##############################################################################
import sys
import time
import urllib.request, urllib.error, urllib.parse

_COUNTER = 10
_WEBPAGE = "http://bigblue:7777/"
_VERBOSE = 0

##############################################################################
## Speed test
##############################################################################
def _speed_test():
    url = _WEBPAGE
    if len(sys.argv) > 2: url = url + sys.argv[2]
    count = _COUNTER
    start = time.time()
    for i in range(count):
        file = urllib.request.urlopen(url)
        for line in file:
            pass

    elapsed = time.time() - start
    print("%d URLs, %.2f seconds" % (count, elapsed))
    print("%.2f URLs/second" % (count / elapsed))

##############################################################################
## Mainline code
##############################################################################
def _main():
    url = _WEBPAGE
    if len(sys.argv) > 1:
        url = sys.argv[1]
        if url == "speed-test":
            _speed_test()
            return

    if url == "shutdown": url = _WEBPAGE + "shutdown.html"
    if url == "quit": url = _WEBPAGE + "quit"
    if not url.startswith("http:"):
        url = "http://" + url

    request = urllib.request.Request(url)
    # request.add_header("accept", "text/plain; q=0.5, text/html")
    request.add_header("user-agent", "Python-Brian/0.0.1")
    request.add_header("X-something", "X-value")

    try:
        file = urllib.request.urlopen(request)
    except urllib.error.URLError as e:
        print(e, url)
        return
    except Exception as e:
        print("Exception:", e)
        raise

    if _VERBOSE:
        print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        print("URL:", url)
        print(file.headers)

    ##########################################################################
    ## Select decoder
    method = 0
    if method == 0:
        ## OK, uses default
        def decode(line): return str(line,encoding='iso-8859-1')
    elif method == 1:
        ## OK, explicit decoding [python2]
        def decode(line): return line.decode('utf-8')
    elif method == 2:
        ## NG: This converts "sample\r\n" into "b'sample\\r\\n'"
        def decode(line): return str(line) ## Always ends with "'"
    else:
        ## NG, throws exception on line.endswith() statement
        def decode(line): return line

    for line in file:
        line = decode(line)

        suffix = ""
        while line.endswith(("\n", "\r")):
            if line.endswith("\n"):
                suffix = "\\n" + suffix
            else:
                suffix = "\\r" + suffix
            line = line[:-1]

        if _VERBOSE > 1:
            line += suffix

        print(line)

    print(".... Done ....")

if __name__ == "__main__":
    _main()

