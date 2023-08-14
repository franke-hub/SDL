#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016-2021 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       HttpServer.py
##
## Purpose-
##       AIML HTTP server, command['http-server']
##
## Last change date-
##       2021/04/03
##
## Implementation notes-
##       VERBOSITY 0: No logging
##       VERBOSITY 1: Request type logging
##       VERBOSITY 2: Detailed request/response logging
##
##############################################################################
from __future__ import print_function

import os
import sys
import threading
import time
import urllib.parse

if sys.version_info[0] < 3:
    import SimpleHTTPServer
    import SocketServer
    _SimpleHTTPRequestHandler = SimpleHttpServer.SimpleHTTPRequestHandler
    _TCPServer = SocketServer.TCPServer
else:
    import http.server
    import socketserver
    _SimpleHTTPRequestHandler = http.server.SimpleHTTPRequestHandler
    _TCPServer = socketserver.TCPServer

from lib.Command import command
from lib.Control import control
from lib.Debug import *
from lib.Dispatch import TAB, UOW, WDW

import Common
from Config import PROGRAM, VERBOSE, VERSION

##############################################################################
## Define available imports
##############################################################################
__all__ = None                      ## Nothing exported

##############################################################################
## Control defaults
##############################################################################
_DEFAULT_SESSION = "Default/Server" ## The default sessionID
_PORT = 8080                        ## The default server port
_USE_DAEMON = True                  ## Use daemon _HttpServerThread?
_VERBOSE = VERBOSE                  ## Verbosity, larger is noisier

##############################################################################
## Internal variables
##############################################################################
_lock   = threading.Lock()          ## State change lock
_thread = None                      ## The _HttpServerThread, when running

##############################################################################
## Default web pages
##############################################################################
_403PAGE = ( '<HTML><HEAD><TITLE>NOT AUTHORIZED</TITLE></HEAD>'
           , '<BODY><H1 align="center">NOT AUTHORIZED</H1>'
           , 'Your improper request has been logged'
           , '</BODY></HTML>' )

_404PAGE = ( '<HTML><HEAD><TITLE>NOT FOUND</TITLE></HEAD>'
           , '<BODY><H1 align="center">NOT FOUND</H1>'
           , 'The page you requested could not be found.'
           , '</BODY></HTML>' )

_WEBPAGE = ( '<HTML><HEAD><TITLE>'+PROGRAM+'/'+VERSION+'</TITLE></HEAD>'
           , '<META http-equiv="Expires" content="0">'
           , '<META http-equiv="CACHE-CONTROL" content="NO-CACHE">'
           , '<BODY><H1 align="center">Talk to me</H1>'
           , '<FORM method="POST" action="" autocomplete="off">'
           , '<INPUT name=data size=200 autofocus=autofocus>'
           , '<INPUT type=submit style="height: 0; width: 0; padding: 0; border: none">'
           , '</FORM>'
           , '</BODY></HTML>' )

##############################################################################
## Internal functions
##############################################################################
_debugf = Common._debugf
_tracef = Common._tracef

def _logger(*args, **kwargs):
    line = " ".join(str(arg) for arg in args) + "\n"
    log = open("log/http-server.log", "a")
    log.write(line)
    log.close()

def _server(*args, **kwargs):
    debugf("HTTP server:", *args, **kwargs)

##############################################################################
## The HTTP request handler: Created for each page served
##############################################################################
class _HttpRequestHandler(_SimpleHTTPRequestHandler):
    global _thread

    def do_HTML(self,code,list_):
        text = "<br>\n".join(line for line in list_) + "\n"
        self.send_response(code)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-Length", len(text))
        self.send_header("Last-Modified", self.date_time_string())
        self.end_headers()
        self.wfile.write(text.encode('utf-8'))
        if _VERBOSE > 1:
            _logger("\n" + text)

    def do_TEXT(self, text):
        self.send_response(200)
        self.send_header("Content-type", "text")
        self.send_header("Content-Length", len(text))
        self.send_header("Last-Modified", self.date_time_string())
        self.end_headers()
        self.wfile.write(text.encode('utf-8'))
        if _VERBOSE > 1:
            _logger("\n" + text)

    ##########################################################################
    ## For debugging: (Change XX_GET to do_GET, do_GET to GET_do)
    def XX_GET(self):
        try:
            self.GET_do()
        except Exception as x:
            Debug.handle_exception()

    ##########################################################################
    ## Handle GET request: Serve web page, icon, or QUIT
    def do_GET(self):
        if _VERBOSE > 1:
            _logger(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
            _logger("%s %s %s" %
                    (self.command, self.path, self.request_version))
            for heading in self.headers:
                _logger(heading + ":", self.headers[heading])

        if self.path == "/" or self.path == "/index.html":
            self.do_HTML(200, _WEBPAGE)
        elif self.path == "/favicon.ico":
            self.do_HTML(404, _404PAGE)
        else:
            self.do_HTML(403,_403PAGE)

    ##########################################################################
    ## We can get either FORM data or simple text
    def do_POST(self):
        content_length = self.headers.get('Content-Length', '0')
        try:
            content_length = int(content_length)
        except:
            content_length = 0

        if _VERBOSE > 1:
            _logger(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
            _logger("%s %s %s" %
                    (self.command, self.path, self.request_version))
            for heading in self.headers:
                _logger(heading + ":", self.headers[heading])

        messinp = ''
        if content_length > 0:
            messinp = self.rfile.read(content_length).decode('UTF-8')
            if _VERBOSE > 1:
                _logger("\n" + messinp)
            if self.path == '/' and messinp.startswith('data='):
                messinp = messinp[5:]
            messinp = urllib.parse.unquote_plus(messinp)

        ######################################################################
        ## Extract the sessionID
        session = _DEFAULT_SESSION
        if self.path != '/':
            session = self.path[1:]

        ######################################################################
        ## Transmit the message
        messout = Common.aiml_transmit(session, messinp)

        ######################################################################
        ## Generate HTML response, refreshing the web page
        if self.path == '/':        ## If servicing a web page
            list_ = list(_WEBPAGE[:-1])
            list_ += [">>>> " + messinp, messout]
            list_ += [_WEBPAGE[-1]]
            self.do_HTML(200,list_)
            return

        ######################################################################
        ## Generate TEXT response
        self.do_TEXT(messout)

    def log_message(self, format, *args):
        if _VERBOSE > 0:
            _logger("%s - - [%s] %s" %
                    (self.client_address[0],
                     self.log_date_time_string(),
                     format % args))

    def send_header(self, keyword, value):
        super(_SimpleHTTPRequestHandler, self).send_header(keyword, value)
        if _VERBOSE > 1:
            _logger(keyword + ":", value)

##############################################################################
## _HttpRemoveThread: Terminates the _HttpServerThread
##############################################################################
class _HttpRemoveThread(threading.Thread):
    def __init__(self, _thread):
        super(_HttpRemoveThread, self).__init__()
        self.name = "HttpRemove"
        self.daemon = True
        self._thread = _thread

        self._event = threading.Event()
        self.start()
        self._event.wait()
        del self._event

    def run(self):
        self._event.set()

        _debugf("%s.shutdown" % self._thread.name)
        self._thread._httpd.shutdown()
        if not self._thread.daemon:
            self._thread.join(3.0)
            while self._thread.is_alive():
                _debugf("%s.shutdown pending" % self._thread.name)
                self._thread.join(15.0)

##############################################################################
## _HttpServerThread: Operates the _TCPServer
##############################################################################
class _HttpServerThread(threading.Thread):
    def __init__(self):
        super(_HttpServerThread, self).__init__()
        self.name = "HttpServer"
        if _USE_DAEMON:
            self.daemon = True

        self._is_running = False
        self._httpd = None

        self._event = threading.Event()
        self.start()
        self._event.wait()
        del self._event

    def _del_thread(self):
        with _lock:
            was_running = self._is_running
            if was_running:
                self._is_running = False
                Common.del_thread(self)
        return was_running

    def run(self):
        global _lock, _thread

        _thread = self
        self._event.set()
        self._is_running = True
        Common.add_thread(self)

        try:
            self._httpd = _TCPServer(("",_PORT), _HttpRequestHandler)
            _server("Running, port:", _PORT)
            _logger("%.3f"%time.time(), time.asctime(),
                    "HTTP server: Running, port:", _PORT)

            self._httpd.serve_forever()

            _server("Stopped, port:", _PORT)
        except Exception as x:
            _server("Exception: ", x)

        _thread = None
        self._del_thread()

    def stop(self):
        if self._del_thread():
            _HttpRemoveThread(_thread)

##############################################################################
## __Command class: (http-server command)
##############################################################################
def _usage(argv, mess):
    debugf(argv[0] + ":", mess)

class __Command:
    @staticmethod
    def run(argv):
        global _PORT, _thread, _VERBOSE
        argc = len(argv)
        if argc > 1:
            if argv[1].upper() == 'PORT':
                if argc == 3:
                    if _thread:
                        _server("running, cannot set port")
                    else:
                        _PORT = int(argv[2])
                        _server("port:", _PORT)
                else:
                    _usage(argv, "port option requires port number (only)")
            elif argv[1].upper() == 'START':
                running = _thread
                if running:
                    _server("already running")
                else:
                    _HttpServerThread() ## (Sets _thread before return)
                    time.sleep(0.125)
            elif argv[1].upper() == 'STOP':
                running = _thread
                if running and running._is_running:
                    running.stop()
                    time.sleep(0.125)
                else:
                    _server("already stopped")
            elif argv[1].upper() == 'VERBOSE':
                if argc == 3:
                    _VERBOSE = int(argv[2])
                    _server("VERBOSE:", _VERBOSE)
                else:
                    _usage(argv, "verbose option requires number (only)")
            else:
                _usage(argv, "Invalid option: " + argv[1])
                debugf("Valid options: port, start, stop, verbose")
        else:
            state = "Stopped"
            if _thread:
                state = "Running"

            _server(state+", port:", str(_PORT)+", verbose:", _VERBOSE)

        return 0

command['http-server'] = __Command

##############################################################################
## Startup: start the HTTP server
##############################################################################
if True:
    _HttpServerThread()             ## Start the http-server thread
    time.sleep(0.125)               ## (Hack: Wait for Running message)
