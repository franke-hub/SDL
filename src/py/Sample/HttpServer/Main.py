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
##       HTTP server
##
## Last change date-
##       2018/01/01
##
## Usage-
##       ./Main.py
##
##############################################################################
import cgi
import os
import queue
import sys
import http.server
import socket
import socketserver
import threading

_PORT = 7777
_VERBOSE = 0

def encode(s):
    return s.encode() + b'\r\n'

class HTTPThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

        self.httpd = None
        self.inpQ = queue.Queue()
        self.outQ = queue.Queue()
        self.outQ.put("initialize")
        self.Daemon = True
        self.start()
        self.outQ.join()

    def run(self):
        handler = HTTPRequestHandler
        self.httpd = socketserver.TCPServer(("",_PORT), handler)
        self.outQ.get() ## "initialize""
        self.outQ.task_done()

        print("Now serving: %s:%d" % (socket.gethostname(), _PORT))
        self.httpd.serve_forever()

    def inp(self, string):
        self.inpQ.put(string)

    def get(self):
        string = self.inpQ.get()
        self.inpQ.task_done()
        return string

    def put(self, string):
        self.outQ.put(string)
        self.outQ.join()

    def quit(self):
        if self.httpd:
            self.httpd.shutdown()
            self.httpd = None

class HTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if _VERBOSE:
            print("do_GET")
            print("Command:", self.command)
            print("Path:", self.path)
            print("Version:", self.request_version)
            print("Headers:...\n", str(self.headers) + "...Headers")

            if _VERBOSE > 1:
                for heading in self.headers:
                    print(heading, "=", self.headers[heading])

        if self.path.lower() == "/quit": self.path = "/quit.html"
        f = self.send_head()
        if f:
            try:
                self.copyfile(f,self.wfile)
            finally:
                f.close()

        if self.path.lower() == "/quit.html":
            if True:
                thread = threading.current_thread()
                try:
                    thread.inp("quit")
                except:
                    os.kill(os.getpid(), 9) ## Fallback method

            elif True:
                quit() ## Not effective
            elif True:
                assert False,"/quit" ## Not effective
            elif True:
                httpd.shutdown() ## Deadlock
            elif True:
                os.abort() ## Too drastic, causes dump
            else:
                os.kill(os.getpid(), 9) ## SUCCESS!!

    def do_POST(self):
        if _VERBOSE: print("do_POST")

        # Extract the form data
        form = cgi.FieldStorage(
            fp=self.rfile,
            headers=self.headers,
            environ={'REQUEST_METHOD':'POST',
                     'CONTENT_TYPE':self.headers['Content-Type'],
                     })

        # Begin the response
        self.send_response(200)

        body = b''
        host, port = self.client_address
        body += encode('Client: {}:{}'.format(host, port))
        body += encode('User-agent: {}'.format(self.headers['user-agent']))
        body += encode('Path: {}'.format(self.path))
        body += encode('Form data:')

        # Echo back information about what was posted in the form
        for field in list(form.keys()):
            field_item = form[field]
            if field_item.filename:
                # The field contains an uploaded file
                file_data = field_item.file.read()
                file_len = len(file_data)
                del file_data
                body += encode('\tUploaded {} as "{}" ({} bytes)\n'
                              .format(field, field_item.filename, file_len))
            else:
                # Regular form value
                body += encode('\t{}={}'.format(field, form[field].value))

        self.send_header('Content-Length', str(len(body)))
        self.send_header('Content-Type', 'text')
        self.end_headers()
        self.wfile.write(body)
        return

    def send_head(self):
        if _VERBOSE: print("send_head")
        return http.server.SimpleHTTPRequestHandler.send_head(self)

    def set_thread(self, thread):
        self._thread = thread

##############################################################################
## Mainline code
##############################################################################
if __name__ == "__main__":
    thread = HTTPThread()
    while True:
        inp = thread.get()
        if inp == "quit":
            break

    thread.quit()
    print(".... Done ....")

